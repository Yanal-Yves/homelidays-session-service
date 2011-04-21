/* Copyright (c) 2010 HomeAway, Inc.
 * All rights reserved.  http://sessionservice.codeplex.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include "StdAfx.h"
#include "AspSessionPersistence.h"
#include "PartitionResolver.h"
#include "FileUtility.h"
#include "Logger.h"
#include <iostream>
#include <fstream>
#include <string>
#include <atlstr.h>

using namespace std;

inline void TESTHR(HRESULT x)
{
	if (FAILED(x))
	{
		_com_issue_error(x);
	}
};

/// Log Provider Errors from Connection object.
void AspSessionPersistence::LogProviderError(
	ADODB::_ConnectionPtr pConnection, // Current connection objects
	_bstr_t bstrCommandText // The SQL command text that failled
	)
{
	ADODB::ErrorPtr pErr = NULL; // pErr is a record object in the Connection's Error collection.
	long nCount = 0;
	long i = 0;

	if (pConnection->Errors->Count > 0)
	{
		nCount = pConnection->Errors->Count;

		for (i = 0 ; i < nCount ; i++)
		{
			pErr = pConnection->Errors->GetItem(i);
			
			Logging::Logger::GetCurrent()->WriteInfo("Ado Error : " + pErr->Description + " Query: " + bstrCommandText +"\r\n");
			printf(
			"Error number: %x\n Error Description: %s\n Query: %s\n",
			pErr->Number,
			(LPCSTR) pErr->Description,
			(LPCSTR) bstrCommandText);
		}
	}
}

/// Create tables dynamically in the database if they do not exist
HRESULT AspSessionPersistence::CreateTables(
	Partition _partition ///< [in] Partition (SQL Connexion string) where to create the tables
	)
{
	HRESULT hr=S_OK;
	CString cstrModulePath;
	hr = FileUtility::GetModuleDirectory(cstrModulePath);
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"Error on retrieving module directory\r\n");
	}

	cstrModulePath.Append(L"Createtables.sql");
	//On récupére le script de création de table depuis un fichier
	string s, line;
	
	ifstream in(cstrModulePath); //open the file
	if (in.bad())
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"File access error : CreateTables.sql  \r\n");
	}
	else
	{
		while (getline(in, line))
		{
			s += line+" \r\n";
		}
	}

	in.close();

	_bstr_t bstrCommandText=s.c_str();
	// Create ADO Objects
	ADODB::_ConnectionPtr connection;
	ADODB::_CommandPtr command;
	ADODB::_RecordsetPtr recordset;
	try
	{
		// Create Connection Object.
		hr = connection.CreateInstance(__uuidof(ADODB::Connection));
		TESTHR(hr);
		connection->CursorLocation = ADODB::adUseClient;
		hr = connection->Open(
			_partition.bstrPartition,
			_partition.bstrLogin,
			_partition.bstrPassword,
			ADODB::adConnectUnspecified);
		TESTHR(hr);
		
		// Create Command Object.
		hr = command.CreateInstance(__uuidof(ADODB::Command));
		TESTHR(hr);

		command->ActiveConnection = connection;
		command->CommandType = ADODB::adCmdText;
		command->CommandText = bstrCommandText;

		// Execute the request
		_variant_t vtEmpty (DISP_E_PARAMNOTFOUND, VT_ERROR);
		_variant_t recordAffected;
		recordset = command->Execute(&recordAffected, &vtEmpty, ADODB::adCmdText);
	}
	catch (_com_error &e)
	{
		LogProviderError(connection, bstrCommandText);
		_bstr_t bstrError(e.Error());
		_bstr_t bstrErrorMessage(e.ErrorMessage());
		_bstr_t bstrSource(e.Source());
		_bstr_t bstrDescription(e.Description());
		// Printf pour l'application de test le LogProviderError ci-dessus permet l'écriture dans le log
		printf(
			"\n Error : %s \n ErrorMessage : %s \n Source : %s \n Description : %s \n",
			(LPCSTR)bstrError,
			(LPCSTR)bstrErrorMessage,
			(LPCSTR)bstrSource,
			(LPCSTR)bstrDescription);
		hr = E_FAIL;
	}
	catch (...)
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"SessionPersistence.CreateTable Unknown error \r\n");
		printf("\n UNKNOWN ERROR \n");
	}

	// Release ADO objects
	if (recordset)
	{
		if (recordset->State == ADODB::adStateOpen)
		{
			HRESULT hr2;
			hr2 = recordset->Close();
			if (FAILED(hr2))
			{
				if (hr == S_OK)
				{ // if it is the first error in the method
					hr = hr2;
				}
				Logging::Logger::GetCurrent()->WriteInfo(L"Close AdoRecordset error \r\n");

			}
		}

		recordset.Release();
	}

	if (command)
	{
		command.Release();
	}
	
	// On détruit la connexion ADO
	if (connection)
	{
		if (connection->State == ADODB::adStateOpen)
		{
			HRESULT hr2;
			hr2 = connection->Close();
			if (FAILED(hr2))
			{
				if (hr == S_OK)
				{ // if it is the first error in the method
					hr = hr2;
				}
				Logging::Logger::GetCurrent()->WriteInfo(L"Close AdoConnection error \r\n");
				
			}
		}

		connection.Release();
	}
	return hr;
}
/// Delete the session from the storage Happens where session.abandon is called
/// @return S_OK in case of success, any other value if operation failled
HRESULT AspSessionPersistence::DeleteSession(
	_variant_t vtGuid ///< [in] Session Id (GUID as a string)
	)
{
	HRESULT hr=S_OK;

	Partition partition;
	hr = PartitionResolver::GetPartitionn(vtGuid, partition);
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"Partition Resolver error. unable to get partition \r\n");
		return hr;
	}

	// Create ADO Objects
	ADODB::_ConnectionPtr connection;
	ADODB::_CommandPtr command;
	ADODB::_ParameterPtr parameter;
	ADODB::_RecordsetPtr recordset;

	_bstr_t bstrCommandText;
	bstrCommandText = L"DELETE /*20100722-190014-YFA*/ FROM dbo.Session WHERE SessionId=?;";

	try
	{
		// Create Connection Object.
		hr = connection.CreateInstance(__uuidof(ADODB::Connection));
		TESTHR(hr);
		
		connection->CursorLocation = ADODB::adUseClient;
		hr = connection->Open(
			partition.bstrPartition,
			partition.bstrLogin,
			partition.bstrPassword,
			ADODB::adConnectUnspecified);
		TESTHR(hr);
		
		// Create Command Object.
		hr = command.CreateInstance(__uuidof(ADODB::Command));
		TESTHR(hr);

		command->ActiveConnection = connection;
		command->CommandType = ADODB::adCmdText;
		command->CommandText = bstrCommandText;
		
		// Add the parameter
		parameter = command->CreateParameter(
			_bstr_t(L"SessionId"),
			ADODB::adVarChar,
			ADODB::adParamInput,
			AspSessionPersistence::GuidNumberOfDigits,
			vtGuid);

		hr = command->Parameters->Append(parameter); // Add Parameters
		TESTHR(hr);

		// Execute the request
		_variant_t vtEmpty(DISP_E_PARAMNOTFOUND, VT_ERROR); // Open Recordset Object
		recordset = command->Execute(&vtEmpty, &vtEmpty, ADODB::adCmdText);
	}
	catch (_com_error &e)
	{
		LogProviderError(connection, bstrCommandText);
		_bstr_t bstrError(e.Error());
		_bstr_t bstrErrorMessage(e.ErrorMessage());
		_bstr_t bstrSource(e.Source());
		_bstr_t bstrDescription(e.Description());
		// Printf pour l'application de test le LogProviderError ci-dessus permet l'écriture dans le log
		printf(
			"\n Error : %s \n ErrorMessage : %s \n Source : %s \n Description : %s \n",
			(LPCSTR)bstrError,
			(LPCSTR)bstrErrorMessage,
			(LPCSTR)bstrSource,
			(LPCSTR)bstrDescription);
		hr = E_FAIL;
		PartitionResolver::ResetConf();
	}
	catch (...)
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"SessionPersistence.DeleteSession Unknown error \r\n");
		printf("\n UNKNOWN ERROR \n");
	}

	// Release ADO objects
	if (recordset)
	{
		if (recordset->State == ADODB::adStateOpen)
		{
			HRESULT hr2;
			hr2 = recordset->Close();
			if (FAILED(hr2))
			{
				if (hr == S_OK)
				{ // if it is the first error in the method
					hr = hr2;
				}

				Logging::Logger::GetCurrent()->WriteInfo(L"Close AdoRecordset error \r\n");
			}
		}

		recordset.Release();
	}

	if (parameter)
	{
		parameter.Release();
	}

	if (command)
	{
		command.Release();
	}
	
	// On détruit la connexion ADO
	if (connection)
	{
		if (connection->State == ADODB::adStateOpen)
		{
			HRESULT hr2;
			hr2 = connection->Close();
			if (FAILED(hr2))
			{
				if (hr == S_OK)
				{ // if it is the first error in the method
					hr = hr2;
				}

				Logging::Logger::GetCurrent()->WriteInfo(L"Close AdoConnection error \r\n");
			}
		}

		connection.Release();
	}

	return hr;

}

/// Get the session from the storage
/// @return S_OK in case of success, any other value if operation failled
HRESULT AspSessionPersistence::GetSession(
	_variant_t vtGuid, ///< [in] Session Id (GUID as a string)
	_variant_t &LastAccessed, ///< [out] Last accessed date
	_variant_t &SessionTimeout, /// [out] Session time out
	_variant_t &Data ///< [out] Session data
	)
{
	HRESULT hr=S_OK;

	Partition partition;
	hr = PartitionResolver::GetPartitionn(vtGuid, partition);
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"Partition Resolver error. unable to get partition \r\n");
		return hr;
	}
	
	// Create ADO Objects
	ADODB::_ConnectionPtr connection;
	ADODB::_CommandPtr command;
	ADODB::_ParameterPtr parameter;
	ADODB::_RecordsetPtr recordset;

	_bstr_t bstrCommandText =
		L"SELECT /*20100722-190015-YFA*/ LastAccessed, Data,SessionTimeOut FROM dbo.Session WHERE SessionId=? and LastAccessed > DATEADD(minute, -SessionTimeOut, GETUTCDATE())";

	try
	{
		// Create Connection Object.
		hr = connection.CreateInstance(__uuidof(ADODB::Connection));
		TESTHR(hr);
		
		connection->CursorLocation = ADODB::adUseClient;
		hr = connection->Open(
			partition.bstrPartition,
			partition.bstrLogin,
			partition.bstrPassword,
			ADODB::adConnectUnspecified);
		TESTHR(hr);
		
		// Create Command Object.
		hr = command.CreateInstance(__uuidof(ADODB::Command));
		TESTHR(hr);

		command->ActiveConnection = connection;
		command->CommandType = ADODB::adCmdText;
		command->CommandText = bstrCommandText;
		
		// Add the parameter
		parameter = command->CreateParameter(
			_bstr_t(L"SessionId"),
			ADODB::adVarChar,
			ADODB::adParamInput,
			AspSessionPersistence::GuidNumberOfDigits,
			vtGuid);

		hr = command->Parameters->Append(parameter); // Add Parameters
		TESTHR(hr);

		// Execute the request
		_variant_t vtEmpty(DISP_E_PARAMNOTFOUND, VT_ERROR); // Open Recordset Object
		recordset = command->Execute(&vtEmpty, &vtEmpty, ADODB::adCmdText);
		
		while (recordset->ADOEOF == false)
		{
			LastAccessed = recordset->Fields->GetItem(L"LastAccessed")->GetValue();
			Data = recordset->Fields->GetItem(L"Data")->GetValue();
			SessionTimeout =  recordset->Fields->GetItem(L"SessionTimeOut")->GetValue();
			recordset->MoveNext();
		};
	}
	catch (_com_error &e)
	{
		LogProviderError(connection, bstrCommandText);
		_bstr_t bstrError(e.Error());
		_bstr_t bstrErrorMessage(e.ErrorMessage());
		_bstr_t bstrSource(e.Source());
		_bstr_t bstrDescription(e.Description());
		// Printf pour l'application de test le LogProviderError ci-dessus permet l'écriture dans le log
		printf(
			"\n Error : %s \n ErrorMessage : %s \n Source : %s \n Description : %s \n",
			(LPCSTR)bstrError,
			(LPCSTR)bstrErrorMessage,
			(LPCSTR)bstrSource,
			(LPCSTR)bstrDescription);
		hr = E_FAIL;
		PartitionResolver::ResetConf();
	}
	catch (...)
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"SessionPersistence.GetSession Unknown error \r\n");
		printf("\n UNKNOWN ERROR \n");
	}

	// Release ADO objects
	if (recordset)
	{
		if (recordset->State == ADODB::adStateOpen)
		{
			HRESULT hr2;
			hr2 = recordset->Close();
			if (FAILED(hr2))
			{
				if (hr == S_OK)
				{ // if it is the first error in the method
					hr = hr2;
				}

				Logging::Logger::GetCurrent()->WriteInfo(L"Close AdoRecordset error \r\n");
			}
		}

		recordset.Release();
	}

	if (parameter)
	{
		parameter.Release();
	}

	if (command)
	{
		command.Release();
	}
	
	// On détruit la connexion ADO
	if (connection)
	{
		if (connection->State == ADODB::adStateOpen)
		{
			HRESULT hr2;
			hr2 = connection->Close();
			if (FAILED(hr2))
			{
				if (hr == S_OK)
				{ // if it is the first error in the method
					hr = hr2;
				}

				Logging::Logger::GetCurrent()->WriteInfo(L"Close AdoConnection error \r\n");
			}
		}

		connection.Release();
	}

	return hr;
}

/// Set the session into the storage
/// @return S_OK in case of success, any other value if operation failled
HRESULT AspSessionPersistence::SetSession(
	_variant_t vtGuid, ///< [in] Session Id (GUID as a string)
	_variant_t SessionTimeOut, //[in] Session Time out
	_variant_t Data ///< [in] Session data
	)
{
	HRESULT hr;
	
	Partition partition;
	hr = PartitionResolver::GetPartitionn(vtGuid, partition);
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"Partition Resolver error. Unable to get partition \r\n");
		return hr;
	}

	// Create ADO Object.
	ADODB::_ConnectionPtr connection;
	ADODB::_CommandPtr command;
	ADODB::_ParameterPtr parameter;
	ADODB::_ParameterPtr parameter2;
	ADODB::_RecordsetPtr recordset;

	_bstr_t bstrCommandText;
	bstrCommandText = L"BEGIN TRAN;DELETE /*20100722-190016-YFA*/ FROM dbo.Session WHERE SessionId=?;";
	bstrCommandText += L"INSERT INTO dbo.Session(SessionId,LastAccessed,SessionTimeOut,Data) VALUES (?,GETUTCDATE(),?,?);COMMIT";

	try
	{
		// Create Connection Object.
		hr = connection.CreateInstance(__uuidof(ADODB::Connection));
		TESTHR(hr);

		connection->CursorLocation = ADODB::adUseClient;
		hr = connection->Open(
			partition.bstrPartition,
			partition.bstrLogin,
			partition.bstrPassword,
			ADODB::adConnectUnspecified);
		TESTHR(hr);

		// Create Command Object.
		hr = command.CreateInstance(__uuidof(ADODB::Command));
		TESTHR(hr);

		command->ActiveConnection = connection;
		command->CommandType = ADODB::adCmdText;
		command->CommandText = bstrCommandText;
		
		// Add the "SessionId" for delete request in parameter
		parameter = command->CreateParameter(
			_bstr_t(L"SessionIdDelete"),
			ADODB::adVarWChar,
			ADODB::adParamInput,
			AspSessionPersistence::GuidNumberOfDigits,
			vtGuid);
		hr = command->Parameters->Append(parameter);
		TESTHR(hr);

		// Add the "SessionId" for insert request in parameter
		parameter = command->CreateParameter(
			_bstr_t(L"SessionIdInsert"),
			ADODB::adVarWChar,
			ADODB::adParamInput,
			AspSessionPersistence::GuidNumberOfDigits,
			vtGuid);
		hr = command->Parameters->Append(parameter);
		TESTHR(hr);
		
		// Add the "Sessiontimeout" for insert request in parameter
		parameter = command->CreateParameter(
			_bstr_t(L"SessionTimeOut"),
			ADODB::adInteger,
			ADODB::adParamInput,
			5,
			SessionTimeOut);
		hr = command->Parameters->Append(parameter);
		TESTHR(hr);

		// Add the "Data" out parameter
		parameter2 = command->CreateParameter(_bstr_t(L"Data"), ADODB::adVarWChar, ADODB::adParamInput, -1, Data);
		hr = command->Parameters->Append(parameter2);
		TESTHR(hr);

		// Execute the request
		_variant_t vtEmpty (DISP_E_PARAMNOTFOUND, VT_ERROR);
		_variant_t recordAffected;
		recordset = command->Execute(&recordAffected, &vtEmpty, ADODB::adCmdText);
	}
	catch (_com_error &e)
	{
		LogProviderError(connection, bstrCommandText);
		_bstr_t bstrError(e.Error());
		_bstr_t bstrErrorMessage(e.ErrorMessage());
		_bstr_t bstrSource(e.Source());
		_bstr_t bstrDescription(e.Description());
		// Printf pour l'application de test le LogProviderError ci-dessus permet l'écriture dans le log
		printf(
			"\n Error : %s \n ErrorMessage : %s \n Source : %s \n Description : %s \n",
			(LPCSTR)bstrError,
			(LPCSTR)bstrErrorMessage,
			(LPCSTR)bstrSource,
			(LPCSTR)bstrDescription);
		hr = E_FAIL;
		PartitionResolver::ResetConf();
	}
	catch (...)
	{
		printf("\n UNKNOWN ERROR \n");
		Logging::Logger::GetCurrent()->WriteInfo(L"SessionPersistence.SetSession Unknown error \r\n");
	}

	// Release ADO objects
	if (recordset)
	{
		if (recordset->State == ADODB::adStateOpen)
		{
			HRESULT hr2;
			hr2 = recordset->Close();
			if (FAILED(hr2))
			{
				if (hr == S_OK)
				{ // if it is the first error in the method
					hr = hr2;
				}

				Logging::Logger::GetCurrent()->WriteInfo(L"Close AdoRecordset error \r\n");
			}
		}

		recordset.Release();
	}

	if (parameter2)
	{
		parameter2.Release();
	}

	if (parameter)
	{
		parameter.Release();
	}

	if (command)
	{
		command.Release();
	}
	
	// On détruit la connexion ADO
	if (connection)
	{
		if (connection->State == ADODB::adStateOpen)
		{
			HRESULT hr2;
			hr2 = connection->Close();
			if (FAILED(hr2))
			{
				if (hr == S_OK)
				{ // if it is the first error in the method
					hr = hr2;
				}

				Logging::Logger::GetCurrent()->WriteInfo(L"Close AdoConnection error \r\n");
			}
		}

		connection.Release();
	}

	return hr;
}

/// Refresh the session into the storage
/// @return S_OK in case of success, any other value if operation failled
HRESULT AspSessionPersistence::RefreshSession(
	_variant_t vtGuid ///< [in] Session Id (GUID as a string) of the session to refresh
	)
{
	HRESULT hr;

	Partition partition;
	hr = PartitionResolver::GetPartitionn(vtGuid, partition);
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"Partition Resolver error. unable to get partition \r\n");
		return hr;
	}

	// Create ADO Object.
	ADODB::_ConnectionPtr connection;
	ADODB::_CommandPtr command;
	ADODB::_ParameterPtr parameter;
	ADODB::_RecordsetPtr recordset;

	_bstr_t bstrCommandText;
	bstrCommandText = L"UPDATE /*20101123-194816-YFA*/ dbo.[Session] SET LastAccessed = GETUTCDATE() WHERE SessionId=?";
	
	try
	{
		// Create Connection Object.
		hr = connection.CreateInstance(__uuidof(ADODB::Connection));
		TESTHR(hr);

		connection->CursorLocation = ADODB::adUseClient;
		hr = connection->Open(
			partition.bstrPartition,
			partition.bstrLogin,
			partition.bstrPassword,
			ADODB::adConnectUnspecified);
		TESTHR(hr);

		// Create Command Object.
		hr = command.CreateInstance(__uuidof(ADODB::Command));
		TESTHR(hr);

		command->ActiveConnection = connection;
		command->CommandType = ADODB::adCmdText;
		command->CommandText = bstrCommandText;
		
		// Add the "SessionId" parameter
		parameter = command->CreateParameter(
			_bstr_t(L"SessionId"),
			ADODB::adVarWChar,
			ADODB::adParamInput,
			AspSessionPersistence::GuidNumberOfDigits,
			vtGuid);
		hr = command->Parameters->Append(parameter);
		TESTHR(hr);
		
		// Execute the request
		_variant_t vtEmpty (DISP_E_PARAMNOTFOUND, VT_ERROR);
		_variant_t recordAffected;
		recordset = command->Execute(&recordAffected, &vtEmpty, ADODB::adCmdText);
	}
	catch (_com_error &e)
	{
		LogProviderError(connection, bstrCommandText);
		_bstr_t bstrError(e.Error());
		_bstr_t bstrErrorMessage(e.ErrorMessage());
		_bstr_t bstrSource(e.Source());
		_bstr_t bstrDescription(e.Description());
		// Printf pour l'application de test le LogProviderError ci-dessus permet l'écriture dans le log
		printf(
			"\n Error : %s \n ErrorMessage : %s \n Source : %s \n Description : %s \n",
			(LPCSTR)bstrError,
			(LPCSTR)bstrErrorMessage,
			(LPCSTR)bstrSource,
			(LPCSTR)bstrDescription);
		hr = E_FAIL;
		PartitionResolver::ResetConf();
	}
	catch (...)
	{
		printf("\n UNKNOWN ERROR \n");
		Logging::Logger::GetCurrent()->WriteInfo(L"SessionPersistence.RefreshSession Unknown error \r\n");
	}

	// Release ADO objects
	if (recordset)
	{
		if (recordset->State == ADODB::adStateOpen)
		{
			HRESULT hr2;
			hr2 = recordset->Close();
			if (FAILED(hr2))
			{
				if (hr == S_OK)
				{ // if it is the first error in the method
					hr = hr2;
				}

				Logging::Logger::GetCurrent()->WriteInfo(L"Close AdoRecordset error \r\n");
			}
		}

		recordset.Release();
	}

	if (parameter)
	{
		parameter.Release();
	}

	if (command)
	{
		command.Release();
	}
	
	// On détruit la connexion ADO
	if (connection)
	{
		if (connection->State == ADODB::adStateOpen)
		{
			HRESULT hr2;
			hr2 = connection->Close();
			if (FAILED(hr2))
			{
				if (hr == S_OK)
				{ // if it is the first error in the method
					hr = hr2;
				}

				Logging::Logger::GetCurrent()->WriteInfo(L"Close AdoConnection error \r\n");
			}
		}

		connection.Release();
	}

	return hr;
}