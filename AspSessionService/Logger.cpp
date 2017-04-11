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

#include "StdAfx.h"
#include "Logger.h"
#include "FileUtility.h"
#include <atlstr.h>
#include <time.h>
#include <STDDEF.H>
#include "StringUtility.h"

namespace Logging
{
	Logger* Logger::m_sLogger = NULL;

	/// Instantiate a Logger class. This is a log utility class that writes messages to a file.
	Logger::Logger(void)
	{
		Initialize();
	}

	void Logger::Initialize()
	{
		CString cstrModulePath;
		FileUtility::GetModuleDirectory(cstrModulePath);
		cstrModulePath.Append(L"SessionServiceLog - ");
		wchar_t buffer[LOGGING_BUFFER_SIZE];
		this->GetCurrentDateTimeAsString(buffer);
		buffer[LOGGING_BUFFER_SIZE-3] = L'\0'; // replace ending \r by \0
		buffer[LOGGING_BUFFER_SIZE-2] = L'\0'; // replace ending \r by \0
		cstrModulePath.Append(buffer);
		cstrModulePath.Append(L".log");

		/// Local path of the log file.
		LPCWSTR LOG_FILE = cstrModulePath;
		
		m_hLogFile = CreateFile(
			LOG_FILE, // lpFileName
			GENERIC_READ | GENERIC_WRITE, // dwDesiredAccess
			FILE_SHARE_READ | FILE_SHARE_WRITE, // dwShareMode
			NULL, // lpSecurityAttributes
			OPEN_ALWAYS, // dwCreationDisposition
			FILE_ATTRIBUTE_NORMAL, // dwFlagsAndAttributes
			NULL); // hTemplateFile

		if (m_hLogFile != INVALID_HANDLE_VALUE)
		{ // fichier ouvert => on supprime son contenu.
			BOOL ret;
			ret = SetEndOfFile(m_hLogFile);
		}
	}

	/// Destructor of the Logger class.
	Logger::~Logger(void)
	{
		if (m_hLogFile != INVALID_HANDLE_VALUE)
		{
			SetEndOfFile(m_hLogFile); // if it fails we do not care because the application is shutding down.

			CloseHandle(m_hLogFile); // if it fails we do not care because the application is shutding down.
		}
	}

	/// Get an instance of the singleton instance of the Logger Class.
	Logger* Logger::GetCurrent()
	{
		if (Logger::m_sLogger == NULL)
		{
			Logger::m_sLogger = new Logger();
		}

		return Logger::m_sLogger;
	}

	/// Write an Info message in the log file.
	void Logger::WriteInfo(
		const wchar_t* pMessage ///< [in] The message to wirte.
		)
	{
		WriteInternal(pMessage); // If write fails, just ignore it
	}

	/// Write an Info message in the log file.
	void Logger::WriteInfo(
		const wchar_t* pMessage, ///< [in] The message to wirte.
		IRequest* request ///< [in] The current request.
		)
	{
		CString msg = pMessage;

		// Add the information from the last COM Error message into the message.
		_bstr_t com_msg;
		HRESULT hr = GetLastComError(com_msg);
		if (hr == S_OK)
		{ // A COM error has been found.
			msg.Append(com_msg);
		}

		// Add the url into the message
		_bstr_t bstr_url;
		GetCurrentUrl(request, bstr_url);
		msg += L"URL : [";
		msg.Append(bstr_url);
		msg += L"]\r\n";

		WriteInternal(msg); // If write fails, just ignore it
	}

	/// Write an Debug message in the log file.
	void Logger::WriteDebug(
		const wchar_t* pMessage ///< [in] The message to wirte.
		)
	{
#ifdef DEBUG
		WriteInternal(pMessage); // If write fails, just ignore it.
#else
		pMessage; // C4100 : 'pMessage' : unreferenced formal parameter
#endif
	}

	/// Write a message in the log file.
	void Logger::WriteInternal(
		const wchar_t* pMessage ///< [in] The message to wirte.
		)
	{
		DWORD dNumberOfByteWritten;
		BOOL bWriteResult = FALSE;

		// Write the DateTime at the begining of the log.
		wchar_t buffer[LOGGING_BUFFER_SIZE];
		this->GetCurrentDateTimeAsString(buffer);
		bWriteResult = WriteFile(
		m_hLogFile,
		&buffer,
		(DWORD)(LOGGING_BUFFER_SIZE-1) * sizeof(wchar_t),
		&dNumberOfByteWritten,
		NULL);

		// Write the body of the message.
		DWORD message_size = (DWORD)wcslen(pMessage) * sizeof(wchar_t);
		bWriteResult = WriteFile(
			m_hLogFile,
			pMessage,
			message_size,
			&dNumberOfByteWritten,
			NULL);

		// Write the information from the last COM Error message.
		_bstr_t com_msg;
		HRESULT hr = GetLastComError(com_msg);
		if (hr == S_OK)
		{ // A COM error has been found.
			BSTR bstr_com_msg;
			bstr_com_msg = com_msg.GetBSTR();
			message_size = com_msg.length() * sizeof(wchar_t);
			bWriteResult = WriteFile(
				m_hLogFile,
				bstr_com_msg,
				message_size,
				&dNumberOfByteWritten,
				NULL);
		}

		// Write a message separator.
		wchar_t message2[] = L"------------------------\r\n\r\n";
		DWORD message_size2 = (DWORD)wcslen(message2) * sizeof(wchar_t);

		bWriteResult = WriteFile(
			m_hLogFile,
			message2,
			message_size2,
			&dNumberOfByteWritten,
			NULL);
	}

	/// Get the current date & time as a string. The string contains an ending L"\r\n\0".
	void Logger::GetCurrentDateTimeAsString(
		wchar_t* pwcTime ///< [in, out] a buffer of 27 wchar_t where the function will write the current date time as a string.
		)
	{
		time_t rawtime;
	  struct tm timeinfo;
		time(&rawtime);

		localtime_s(&timeinfo, &rawtime);

		size_t sBufferSize = LOGGING_BUFFER_SIZE;
		_wasctime_s(pwcTime, sBufferSize, &timeinfo);

		// Replace : by - so we can use the string in a file name.
		for (int i = 0; i<LOGGING_BUFFER_SIZE ; i++)
		{
			if (pwcTime[i] == ':')
			{
				pwcTime[i] = '-';
			}
		}

		pwcTime[LOGGING_BUFFER_SIZE-3] = L'\r';
		pwcTime[LOGGING_BUFFER_SIZE-2] = L'\n';
		pwcTime[LOGGING_BUFFER_SIZE-1] = L'\0';
	}

	// Get the last COM error.
	// @return S_OK if an error has been found and written into bstrComMessage, S_FALSE if no COM error occured.
	HRESULT Logger::GetLastComError(
		_bstr_t& bstrComMessage // A bstr that recieves the last COM error message.
	)
	{
		HRESULT hr;

		// Get the IErrorInfo.
		IErrorInfo* pErrInfo;
		GetErrorInfo(0, &pErrInfo);
		if (pErrInfo != NULL)
		{
			// Get the guid.
			GUID guid_ierror;
			pErrInfo->GetGUID(&guid_ierror);
			_bstr_t bstr_guid_ierror;
			StringUtility::GuidToBstrWithoutDashes(guid_ierror, bstr_guid_ierror);
			bstrComMessage = L"GUID of the interface that defined the error :[" + bstr_guid_ierror + L"]\r\n";

			if (guid_ierror != GUID_NULL)
			{
				// Get the source.
				BSTR bstr_source;
				pErrInfo->GetSource(&bstr_source);
				bstrComMessage += L"ProgID for the class or application that raised the error:[";
				bstrComMessage += bstr_source;
				bstrComMessage += L"]\r\n";

				BSTR bstr_desc;
				pErrInfo->GetDescription(&bstr_desc);
				bstrComMessage += L"Error description:[";
				bstrComMessage += bstr_desc;
				bstrComMessage += L"]\r\n";

				BSTR bstr_help_file;
				pErrInfo->GetHelpFile(&bstr_help_file);
				bstrComMessage += L"Fully qualified path of the Help file:[";
				bstrComMessage += bstr_help_file;
				bstrComMessage += L"]\r\n";
			}

			hr = S_OK;
		}
		else
		{
			hr = S_FALSE;
		}

		return hr;
	}

	/// Get the current running url from the current request.
	void Logger::GetCurrentUrl(
		IRequest* request, ///< [in] the current request.
		_bstr_t& bstrRawRequest ///< [out] the full current url.
		)
	{
		HRESULT hr;
		bstrRawRequest = L"";

		if (request != NULL)
		{
			CComPtr<IRequestDictionary> pRequestDico;
			hr = request->get_ServerVariables(&pRequestDico);
			if(SUCCEEDED(hr))
			{
				CComVariant var_http_host = L"HTTP_HOST";
				CComVariant var_http_host_value;
				hr = pRequestDico->get_Item(var_http_host, &var_http_host_value);
				bstrRawRequest = var_http_host_value;

				CComVariant var_url = L"URL";
				CComVariant var_url_value;
				hr = pRequestDico->get_Item(var_url, &var_url_value);
				_bstr_t bstr_url_value = var_url_value;
				bstrRawRequest += bstr_url_value;

				CComVariant var_query_string = L"QUERY_STRING";
				CComVariant var_query_string_value;
				hr = pRequestDico->get_Item(var_query_string, &var_query_string_value);
				_bstr_t bstr_query_string_value = var_query_string_value;

				if (bstr_query_string_value.length() != 0)
				{ // bstr_query_string_value is not empty string
					bstrRawRequest += L"?";
					bstrRawRequest += bstr_query_string_value;
				}
			}
		}
	}
}
