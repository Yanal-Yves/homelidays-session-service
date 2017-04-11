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
#include "PartitionResolver.h"
#include "XmlUtility.h"
#include "AspSessionPersistence.h"
#include "FileUtility.h"
#include "Logger.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// Initialize lpSingleton static member
PartitionResolver* PartitionResolver::lpSingleton = NULL;

// Initialize lpPartitions static member
Partition PartitionResolver::lpPartitions[PartitionResolver::NbMaxSessionServer];

/// Get the current instance of the PartitionResolver
/// As PartitionResolver is a singleton, the constructor should be called once first
/// @return S_OK in case of success, E_FAIL if the constructor has not been called once
HRESULT PartitionResolver::GetSingleton(PartitionResolver** ppSingleton)
{
	if (PartitionResolver::lpSingleton == NULL)
	{ // The constructor should be called before the call of this method
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError PartitionResolver::GetSingleton The constructor should be called before the call of this method \r\n");
		return E_FAIL;
	}

	*ppSingleton = PartitionResolver::lpSingleton;

	return S_OK;
}

/// Get the partition according to the provided SessionId
/// @return S_OK in case of succes, E_FAIL if no more persistance is available
HRESULT PartitionResolver::GetPartitionn(
	_variant_t vtGuid, ///< [in] The session Id
	Partition& bstrPartition /// [in, out] The partition to use to store the session corresponding to vtGuid
	)
{
	HRESULT hr;

	// Get the partition corresponding to the provided session id
	PartitionResolver* resolver = NULL;
	hr = PartitionResolver::GetSingleton(&resolver);
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError PartitionResolver GetPartitionn GetSingleton failed \r\n");
		return hr;
	}

	// Request ownership of the critical section.
	EnterCriticalSection(&PartitionResolver::CriticalSection);

	bstrPartition = resolver->lpPartitions[0];
	if(!bstrPartition.bIsInitialized)
	{ // Si le Partition Resolver n'est pas initialisé
		// Création des tables à l'intérieur de la partition
		hr=AspSessionPersistence::CreateTables(bstrPartition);
		if(hr== S_OK)
		{
			PartitionResolver::lpPartitions[0].bIsInitialized = true;
		}
		else
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError PartitionResolver GetPartitionn CreateTables failed \r\n");
		}
	}

	// Release ownership of the critical section.
	LeaveCriticalSection(&PartitionResolver::CriticalSection);

	return hr;
}

/// Reset the current applied configuration and reload configuration from the xml file
HRESULT PartitionResolver::ResetConf()
 {
	HRESULT hr=S_OK;

	// Get the partition corresponding to the provided session id
	PartitionResolver* resolver = NULL;
	hr = PartitionResolver::GetSingleton(&resolver);
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError PartitionResolver ResetConf GetSingleton failed \r\n");
		return hr;
	}

	hr=resolver->LoadConf();
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError PartitionResolver GetPartitionn LoadConf failed \r\n");
		return hr;
	}

	return hr;
 }

///charge la configuration depuis 1 fichier Xml
HRESULT PartitionResolver::LoadConf()
{
	HRESULT hr=S_OK;
	CString cstrModulePath;
 	hr=FileUtility::GetModuleDirectory(cstrModulePath);

	if (hr != S_OK)
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError PartitionResolver LoadConf Unable to getmoduledirectory \r\n");
		return hr;
	}

	cstrModulePath.Append(L"AspSessionServiceConfig.xml");
	
	string s, line;
	ifstream in(cstrModulePath); //open the file
	if(!in.bad())
	{
		while(getline(in, line))
			s += line;
	}
	else
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError PartitionResolver LoadConf Unable to open file AspSessionServiceConfig.xml \r\n");
	}

	in.close();

	_variant_t vtXml;
	vtXml.SetString(s.data()); //on convertit le string en variant

	CComPtr<IStream> stream;
	CComPtr<IXmlReader> pReader;
	hr =  XmlUtility::GetReader(vtXml, pReader, stream);
	if (hr != S_OK)
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError PartitionResolver LoadConf Unable to getreader for xml\r\n");
		return hr;
	}

	XmlNodeType nodeType;
	int i=0;
	CString strElement;
	while (S_OK == (hr = pReader->Read(&nodeType)))
	{
		switch (nodeType)
		 {
			 case XmlNodeType_Element:
				{
					const WCHAR* pwszElement;
					hr = pReader->GetLocalName(&pwszElement, NULL);
					if (hr != S_OK)
					{
						pReader.Release();
						stream.Release();
						Logging::Logger::GetCurrent()->WriteInfo(L"\tError PartitionResolver LoadConf Unable to GetLocalName \r\n");
						return hr;
					}

					strElement = pwszElement;
				}break;
			case XmlNodeType_Text:
				{
					// Extract the value from the XML
					const WCHAR* pwszValue;
					hr = pReader->GetValue(&pwszValue, NULL);
					if (hr != S_OK)
					{
						pReader.Release();
						stream.Release();
						Logging::Logger::GetCurrent()->WriteInfo(L"\tError PartitionResolver LoadConf Unable to Extract the value from the XML \r\n");
						return hr;
					}
					
					if(strElement=="ConnectionString")
					{
						PartitionResolver::lpPartitions[i].bstrPartition = pwszValue;
					}

					if(strElement=="Login")
					{
						PartitionResolver::lpPartitions[i].bstrLogin = pwszValue;
					}

					if(strElement=="Password")
					{
						PartitionResolver::lpPartitions[i].bstrPassword = pwszValue;
					}
				}break;
			case XmlNodeType_EndElement:
				{
					const WCHAR* pwszElement;
					hr = pReader->GetLocalName(&pwszElement, NULL);
					strElement = pwszElement;
					if (hr != S_OK)
					{
						pReader.Release();
						stream.Release();
						Logging::Logger::GetCurrent()->WriteInfo(L"\tError PartitionResolver LoadConf Unable to GetLocalName\r\n");
						return hr;
					}

					if(strElement=="Partition") // on incrémente pour la prochaine partition
					{
						PartitionResolver::lpPartitions[i].bIsInitialized=false;
						i++;
					}
				}break;
		}
	}

	//libération des ressources
	pReader.Release();
	stream.Release();
	return S_OK;
}

/// Critical section used to initialize the database
CRITICAL_SECTION PartitionResolver::CriticalSection;

/// Initializez a new instance of the PartitionResolver singleton class.
PartitionResolver::PartitionResolver(void)
{
	// Save the current instance in order to create a singleton
	PartitionResolver::lpSingleton = this;

	// Init the critical section
	InitializeCriticalSection(&PartitionResolver::CriticalSection);

	HRESULT hr;
	hr = LoadConf();
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError PartitionResolver Constructor LoadConf failed \r\n");
	}
}

/// Destructor of the PartitionResolver class.
PartitionResolver::~PartitionResolver(void)
{
	// Release the system resources that were allocated when the critical section object was initialized
	DeleteCriticalSection(&PartitionResolver::CriticalSection);
}
