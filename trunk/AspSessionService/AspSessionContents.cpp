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

// AspSessionContents.cpp : Implementation of CAspSessionContents

#include "stdafx.h"
#include "AspSessionContents.h"
#include "comutil.h"
#include "AspSessionSerializer.h"
#include "AspSessionDeserializer.h"
#include "AspSessionPersistence.h"
#include "Logger.h"

/// Instanciate a CAspSessionContents class
CAspSessionContents::CAspSessionContents()
{
	m_bIsSessionInitialized = false;
}

/// Destructor of the CAspSessionContents class
CAspSessionContents::~CAspSessionContents()
{
	m_bIsSessionInitialized = false;
}

/// Deserialize the dico from the database only the first call to InitializeComponent
/// @return le code d'erreur (S_OK si tout c'est correctement déroulé)
HRESULT CAspSessionContents::InitializeComponent()
{
	HRESULT hr = S_OK;
	if (m_bIsSessionInitialized == false)
	{ // if the dico is not deserialized
		// On flag la session ASP initialisée
		m_bIsSessionInitialized = true;

		BSTR bstrSessionId = NULL;

		hr = ((IAspSession*)m_piAspSession)->get_SessionID(&bstrSessionId);
		if (FAILED(hr))
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionContent InitializeComponent get_SessionID  \r\n");
			if (bstrSessionId != NULL)
			{
				SysFreeString(bstrSessionId);
			}

			return hr;
		}

		CString cstrSessionId = bstrSessionId;
		SysFreeString(bstrSessionId);

		// The session id is a Guid without the dashes => restore them
		cstrSessionId.Insert(8, L'-');
		cstrSessionId.Insert(13, L'-');
		cstrSessionId.Insert(18, L'-');
		cstrSessionId.Insert(23, L'-');

		// Récupération de la session from the storage
		_variant_t last_access;
		_variant_t data;
		_variant_t vtTimeOut;

		_variant_t vtSessionId = cstrSessionId;
		AspSessionPersistence SessPersistence;
		hr = SessPersistence.GetSession(vtSessionId, last_access, vtTimeOut, data);
		if (FAILED(hr))
		{
			Logging::Logger::GetCurrent()->WriteInfo(
				L"Error AspSessionContent InitializeComponent unable to get Session from persistence \r\n",
				((CAspSession*)m_piAspSession)->m_piRequest
				);
			return hr;
		}

		// S'il y a une session en base, on régle le timeout actuel avec le timeout en base
		if (vtTimeOut.vt!=VT_EMPTY)
		{
			long l_timeout;
			l_timeout = vtTimeOut;
			hr = ((IAspSession*)m_piAspSession)->put_Timeout(l_timeout);
		}

		// Désérialisation
		if (data.vt == VT_BSTR)
		{
			AspSessionDeserializer deserializer(dico, data);
			hr = deserializer.DeserializeSession();
			if (FAILED(hr))
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionContent InitializeComponent unable to deserialize session \r\n");

				return hr;
			}
		}
		else if (data.vt == VT_EMPTY)
		{ // VT_EMTPY possible si la session n'existe pas en base de données
#ifdef HOMELIDAYS_SESSION_ONSTART
#endif
		}
		else // if ((data.vt != VT_EMPTY) && (data.vt != VT_BSTR))
		{ // Tout autre cas n'est pas normal
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionContent InitializeComponent session est vide \r\n");
			return E_FAIL;
		}
	}

	return hr;
}

/// Get an enumerator of the CAspSessionContents class
STDMETHODIMP CAspSessionContents::get__NewEnum(
	IUnknown** ppEnumReturn /**< [out] an enumerator to the Content collection */
	)
{
	HRESULT hr = S_OK;
	hr = InitializeComponent();
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionContent get__NewEnum unable to intialize component \r\n");

		return hr;
	}

	// http://msdn.microsoft.com/en-us/library/7k0az35b(VS.80).aspx
	return VCUE::CreateSTLEnumerator<EnumeratorType>(ppEnumReturn, this, dico);
}

/// Get an item corresponding to the provided key
STDMETHODIMP CAspSessionContents::get_Item(
	BSTR Key, ///< [in] the key of the wanted item
	VARIANT* Value ///<[out] the item corresponding to the provided key
	)
{
	if (Value == NULL)
	{
		return E_POINTER;
	}

	BstrToLower(Key);

	HRESULT hr = S_OK;
	hr = InitializeComponent();
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionContent get_Item unable to intialize component \r\n");
		return hr;
	}

	ContainerType::iterator it = dico.find(Key);

	// If item not found, just return
	if (it == dico.end())
	{
		*Value = variant_t().Detach();
	}
	else
	{
		*Value = _variant_t(it->second).Detach();
	}

	// If item was found, copy the variant to the out param
	return S_OK;
}

/// Put an item to the contents collection with the provided key
STDMETHODIMP CAspSessionContents::put_Item(
	BSTR Key, ///< [in] the key where to store the item in the collection
	VARIANT Value ///< [in] the value to store in the content collection
	)
{
	BstrToLower(Key);

	HRESULT hr = S_OK;
	hr = InitializeComponent();
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionContent put_Item unable to intialize component \r\n");
		return hr;
	}

	if (V_ISARRAY(&Value))
	{ // Array of Variant (ex : tableau ASP stocké en Session)
		SAFEARRAY* sourceArray = NULL;
		VARIANT variantArray;
		if (V_ISBYREF(&Value))
		{
			sourceArray = *(Value.pparray);
			variantArray.vt = Value.vt ^ VT_BYREF;
		}
		else
		{ // On rentre dans ce cas si, par exemple, on stocke en Session le tableau des Clefs d'un Scripting.Dictionary
			sourceArray = V_ARRAY(&Value);
			variantArray.vt = Value.vt;
		}

		variantArray.parray = sourceArray;
		dico[Key] = variantArray;
	}
	else
	{
		IDispatch* pdispValue = NULL;
		VARIANT* pValue = &Value;

		if (pValue->vt == VT_DISPATCH)
		{ // COM Component : use default property
			pdispValue = pValue->pdispVal;
			_variant_t vResult;
			DISPPARAMS dispParams;
			_variant_t vDispArg;
			dispParams.rgvarg = &vDispArg;
			dispParams.rgdispidNamedArgs = 0;
			dispParams.cArgs = 0;
			dispParams.cNamedArgs = 0;
			hr = pdispValue->Invoke(
				DISPID_VALUE, // dispIdMember : DISPID_VALUE is the id of the default value
				IID_NULL, // riid : Reserved for future use. Must be IID_NULL.
				LOCALE_SYSTEM_DEFAULT, // lcid : Applications that do not support multiple national languages can ignore this parameter.
				DISPATCH_PROPERTYGET, // wFlags : Flags describing the context of the Invoke call, include:
				&dispParams, // pDispParams : Pointer to a DISPPARAMS structure
				&vResult, // Pointer to the location where the result is to be stored
				NULL,
				0);

			dico[Key] = vResult;
		}
		else
		{ //Value type (string, Double, integer, etc...)
			dico[Key] = Value;
		}
	}

	return S_OK;
}

/// Put a COM object to the contents collection with the provided key
STDMETHODIMP CAspSessionContents::putref_Item(
	BSTR Key, ///< [in] the key where to store the object in the content collection
	IDispatch *Value ///< [in] the object to store in the content collection
	)
{
	// The COM object Value will be released by the ASP runtime

	HRESULT hr;

	BstrToLower(Key);

	hr = InitializeComponent();
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionContent putref_Item unable to intialize component \r\n");
		return hr;
	}

	if (Value == 0x0)
	{ // Value is Nothing
		VARIANT vNothing;
		VariantInit(&vNothing);
		vNothing.vt = VT_DISPATCH;
		vNothing.pdispVal = NULL;

		dico[Key] = vNothing;

		return hr;
	}
	else
	{ // Scripting.Dictionary
		IDispatch* pIDispatch = Value;
		Scripting::IDictionary * pIDictionary;

		hr = pIDispatch->QueryInterface(__uuidof(Scripting::IDictionary), (void **)&pIDictionary);

		if (FAILED(hr))
		{ // E_NOTIMPL : Type d'objet COM non pris en charge par le session service
			return E_NOTIMPL;
		}

		// Type Scripting.Dictionary
		pIDictionary->Release(); // Release the QueryInterface call.

		VARIANT vDictionary;
		VariantInit(&vDictionary);
		vDictionary.vt = VT_DISPATCH;
		vDictionary.pdispVal = Value;

		dico[Key] = vDictionary;

		return hr; // According to the doc should be S_OK
	}
}

/// Get the key corresponding to the provided key
STDMETHODIMP CAspSessionContents::get_Key(
	BSTR Key, ///< [in] The desired key
	BSTR* KeyValue ///< [out] the found key corresponding to the desired key
	)
{
	if (KeyValue == NULL)
	{
		return E_POINTER;
	}

	BstrToLower(Key);

	HRESULT hr = S_OK;
	hr = InitializeComponent();
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionContent get_Key unable to intialize component \r\n");
		return hr;
	}

	ContainerType::iterator it = dico.find(Key);

	// If item not found, return Empty VARIANT
	if (it == dico.end())
	{
		*KeyValue = _bstr_t();
	}
	else
	{
		*KeyValue = Key;
	}

	return S_OK;
}

/// Get the number of element in the content collection
STDMETHODIMP CAspSessionContents::get_Count(
	int* cStrRet ///< [out] the number of element in the content collection
	)
{
	HRESULT hr = S_OK;
	hr = InitializeComponent();
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionContent get_Count unable to intialize component \r\n");
		return hr;
	}

	*cStrRet = (int)dico.size();

	return S_OK;
}

/// Remove the item in content collection corresponding to the specified key
STDMETHODIMP CAspSessionContents::Remove(
	BSTR Key ///< [in] the key of the element to remove in the content collection
	)
{
	HRESULT hr = S_OK;
	hr = InitializeComponent();
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionContent Remove unable to intialize component \r\n");
		return hr;
	}

	ContainerType::iterator it = dico.find(Key);

	if (it != dico.end())
	{ // If item was found, remove it from the map
		dico.erase(it);
	}

	// (Could just use erase on the key, but it's not clear what erase does if the item isn't present.
	//  Our method is safer and allows us to inform the client of failure.)
	return S_OK;
}

/// Remove all the item of the content collection
STDMETHODIMP CAspSessionContents::RemoveAll()
{
	HRESULT hr = S_OK;
	hr = InitializeComponent();
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionContent Remove All unable to intialize component \r\n");
		return hr;
	}

	dico.clear();
	
	return S_OK;
}

/// Get the session id as a Guid usable for DB access
/// @return S_OK if succeed any other value else
HRESULT CAspSessionContents::GetSessionId(
	_variant_t& vtSessionId ///< [in] The session Id as a Guid
	)
{
	HRESULT hr;
	// On récupère le SessionId
	BSTR bstrSessionId = NULL;
	hr = ((IAspSession*)m_piAspSession)->get_SessionID(&bstrSessionId);
	if (SUCCEEDED(hr))
	{		
		CString cstrSessionId = bstrSessionId;
		SysFreeString(bstrSessionId);

		// The session id is a Guid without the dashes => restore them
		cstrSessionId.Insert(8, L'-');
		cstrSessionId.Insert(13, L'-');
		cstrSessionId.Insert(18, L'-');
		cstrSessionId.Insert(23, L'-');

		vtSessionId = cstrSessionId;
	}
	else
	{
		if (bstrSessionId != NULL)
		{
			SysFreeString(bstrSessionId);
		}

		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionContent PersistSession unable to get sessionid \r\n");
	}

	return hr;
}

/// Serialise the content collection into the database
HRESULT CAspSessionContents::PersistSession()
{
	HRESULT hr;
	// Get the session id
	_variant_t vtSessionId;
	GetSessionId(vtSessionId);
	
	// Instanciate the persistance object
	AspSessionPersistence SessPersistence;

	if(this->m_bIsSessionInitialized)
	{ // Session has been initialized => serialize and store session.
		AspSessionSerializer serializer;
		CString xml;

		// Sérialization de la session
		hr = serializer.SerializeSession(dico, xml);
		if (FAILED(hr))
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionContent PersistSession unable to intialize component \r\n");
			return hr;
		}

		_variant_t vtSessionTimeOut;
		long lSessionTimeOut;

		// On récupère le TimeOut
		hr = ((IAspSession*)m_piAspSession)->get_Timeout(&lSessionTimeOut);
		vtSessionTimeOut = lSessionTimeOut;

		// On sauvegarde la session en base de données
		_variant_t vtxml = xml;
		hr = SessPersistence.SetSession(vtSessionId, vtSessionTimeOut, vtxml);
		if (FAILED(hr))
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionContent PersistSession unable to setsession \r\n");
			return hr;
		}
	}
	else
	{ // Session has not been initialized => refresh the LastAccessDate
		hr = SessPersistence.RefreshSession(vtSessionId);
	}

	return hr;
}

/// Put all characters of a BSTR to lower case
void CAspSessionContents::BstrToLower(
	BSTR bstrString ///< [in] the bstr to lower case
	)
{
	UINT uSize = SysStringLen(bstrString);

	for (UINT i = 0 ; i < uSize ; i++)
	{
		bstrString[i] = towlower(bstrString[i]);
	}
}
