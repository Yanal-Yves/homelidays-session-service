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

// AspSession.cpp : Implementation of CAspSession

#include "stdafx.h"
#include "AspSession.h"
#include "comutil.h"
#include "Logger.h"
#include "AspSessionPersistence.h"
#include <atlstr.h>
#include "StringUtility.h"

/// Instanciate a CAspSession class
CAspSession::CAspSession()
{
	m_bOnStartPageCalled = FALSE;
	m_bAbandonSession = FALSE;
	m_lTimeOut = 20; // valeur par défaut du timeout de session
	m_bstrSessionId = NULL;
}

/// Destructor of the CAspSession class
CAspSession::~CAspSession()
{
	if (m_bstrSessionId != NULL)
	{
		SysFreeString(m_bstrSessionId);
	}
}

/// Get ASP intrinsic objects
/// @return S_OK in case of succees anything else in case of faillure
HRESULT CAspSession::GetASPObject(
	LPCWSTR objectName, ///<[in] Name of the ASP intrinsic objects (ex : Response, Request)
	REFIID iid, ///<[in] Interface Identifier requested
	void **ppObj ///<[out] a pointer to the wanted ASP intrinsic objects
)
{
	HRESULT hr;

	// Get the Properties interface
    CComPtr<IGetContextProperties> pProps;
	hr = m_spObjectContext->QueryInterface(IID_IGetContextProperties, (void**) &pProps);
    if (FAILED(hr)) 
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSession GetASPObject QueryInterface for IGetContextProperties\r\n");
	}
	else
	{ // Get the ASP object
		CComBSTR bsObject(objectName);
		CComVariant vtObject;
		hr = pProps->GetProperty(bsObject, &vtObject);
		if (FAILED(hr)) 
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSession GetASPObject IGetContextProperties::GetProperty\r\n");
		}
		else
		{ // Make VT_DISPATCH (should be no change)
			hr = vtObject.ChangeType(VT_DISPATCH);
			if (FAILED(hr)) 
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSession GetASPObject The object does not have a dispatch interface\r\n");
			}
			else
			{ // Query for the requested interface
				hr = vtObject.pdispVal->QueryInterface(iid, ppObj);
				if (FAILED(hr)) 
				{
					Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSession GetASPObject The Response object does not support the requested interface\r\n");
				}
			}
		}
	}

	return S_OK;
}

/// OnStartPage event.
/// The ASP runtime call this method while creating the CAspSession COM object with Server.CreateObject.
STDMETHODIMP CAspSession::OnStartPage(
	IUnknown* pUnk ///< [in] a pointer to the IUnknown interface of a COM component provided by the ASP runtime
	)
{
	if (pUnk == NULL)
	{
		return E_POINTER;
	}

	HRESULT hr;

	// ms-help://MS.VSCC.v90/MS.MSDNQTR.v90.en/iissdk/html/4699c7c4-7f90-405a-be8b-336903aef0e3.htm
	// Get the object context
	hr = GetObjectContext(&m_spObjectContext);
	if (FAILED(hr))
	{
		return hr;
	}

	// Get Request Object Pointer
	hr = GetASPObject(L"Request", __uuidof(IRequest), (void **)&m_piRequest);
	if (FAILED(hr))
	{
		m_spObjectContext.Release();
		return hr;
	}

	// Get Response Object Pointer
	GetASPObject(L"Response", __uuidof(IResponse), (void **)&m_piResponse);
	if (FAILED(hr))
	{
		m_piRequest->Release();
		m_spObjectContext.Release();
		return hr;
	}
	
	// On crée la collection de sesion en mémoire pour la durée de vie de la page
	hr = CAspSessionContents::CreateInstance(&Contents);
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSession sur création de la collection de sesion en mémoire pour la durée de vie de la page\r\n");
		m_piResponse->Release();
		m_piRequest->Release();
		m_spObjectContext.Release();
	}
	else
	{
		// On passe à la collection une référence sur l'AspSession qui l'a instancié pour que
		// Session.Contents ait accès au membre SessoinId.
		// Le composant COM CAspSessionContents sera toujours dans le même processus que CAspSession.
		// De plus la durrée de vie de CAspSessionContents est toujours inférieure à celle CAspSession.
		// On peut donc se passer d'une interface et d'un appel à QError AspSession OnStartPage get_SessionID failedueryInterface
		((CAspSessionContents*)this->Contents)->m_piAspSession = this;

		// Call the get_SessionID to make sure the cookie is written before any user code.
		// This is usefull when user calls Response.Flush before any call to session
		BSTR bstr_session_id = NULL; // Dummy variable, only used to call get_SessionID
		hr = this->get_SessionID(&bstr_session_id);
		if (FAILED(hr))
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSession OnStartPage get_SessionID failed\r\n");
			Contents->Release();	
			m_piResponse->Release();
			m_piRequest->Release();
			m_spObjectContext.Release();
		}
		else
		{
			m_bOnStartPageCalled = TRUE;
		}

		if(bstr_session_id != NULL)
		{
			SysFreeString(bstr_session_id);
		}
	}

	return hr;
}

/// OnEndPage event called by the ASP runtime when releasing the CAspSession COM object
STDMETHODIMP CAspSession::OnEndPage()
{
	HRESULT hr;

	// Sauvegarde la session en base de données.
	if (m_bAbandonSession == FALSE)
	{ // No call to Session.Abandon => Persist the session
		hr = ((CAspSessionContents*)this->Contents)->PersistSession();
		if (FAILED(hr))
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSession OnEndPage PersistSession\r\n");
		}
	}
	else
	{ // Delete Session if Session.abandon has been called into the lifecycle of the page
		BSTR bstrSessionId = NULL;
		hr = get_SessionID(&bstrSessionId);
		if (FAILED(hr))
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSession OnEndPage unable to get sessionid\r\n");
		}
		else
		{
			CString cstrSessionId = bstrSessionId;

			// The session id is a Guid without the dashes => restore them
			cstrSessionId.Insert(8, L'-');
			cstrSessionId.Insert(13, L'-');
			cstrSessionId.Insert(18, L'-');
			cstrSessionId.Insert(23, L'-');

			_variant_t vtSessionId = cstrSessionId;
			AspSessionPersistence SessPersistence;
			hr = SessPersistence.DeleteSession(vtSessionId);
			if (FAILED(hr))
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSession OnEndPage unable to deletesession\r\n");
			}
			else
			{
				m_bAbandonSession = FALSE;
			}
		}

		if (bstrSessionId != NULL)
		{
			SysFreeString(bstrSessionId);
		}
	}

	// Release resources
	m_piRequest->Release();
	m_piResponse->Release();
	m_spObjectContext.Release();
	Contents->Release();
	((CAspSessionContents*)this->Contents)->m_bIsSessionInitialized = false;
	m_bOnStartPageCalled = FALSE;

	return hr;
}

/// Get the Session.Contents collection
STDMETHODIMP CAspSession::get_Contents(
	IAspSessionContents** pVal ///< [out] a pointer to the Session.Contents Collection
	)
{
	if (pVal == NULL)
	{
		return E_POINTER;
	}

	// On renvoit une référence au client ASP donc on l'indique au composant par un AddRef
	Contents->AddRef();
	*pVal = Contents;

	return S_OK;
}

/// Get a value in the Session.Contents collection according to the provided key
STDMETHODIMP CAspSession::get_Value(
	BSTR bstrValue, ///< [in] the key of the wanted value
	VARIANT* pVal ///< [out] a pointer to the wanted value
	)
{
	HRESULT hr;

	if (pVal == NULL)
	{
		return E_POINTER;
	}

	hr = Contents->get_Item(bstrValue, pVal);

	return hr;
}

/// Add a value into the Contents.Collection according to the provided key.
/// This method is called by ASP runtime for Set MyVar = MyComObject
STDMETHODIMP CAspSession::putref_Value(BSTR bstrValue, VARIANT newVal)
{
	HRESULT hr;

	if (newVal.vt == VT_DISPATCH)
	{
		hr = Contents->putref_Item(bstrValue, newVal.pdispVal);
	}
	else
	{ // Shouold not happend
		hr = E_NOTIMPL;
	}

	return hr;
}

/// Add a value into the Contents.Collection according to the provided key
STDMETHODIMP CAspSession::put_Value(
	BSTR bstrValue, ///< [in] the key of the value to add
	VARIANT newVal ///< [in] the value to add in the Session.Contents collection
	)
{
	HRESULT hr;

	hr = Contents->put_Item(bstrValue, newVal);

	return hr;
}

/// Read a cookie. This code is inspired by Microsoft at http://support.microsoft.com/kb/241492/en-us
HRESULT CAspSession::ReadCookie(
	BSTR bstrCookieName, ///< [in] name of the cookie to read
	BSTR bstrCookieKey, ///< [in] key of the value to add in the cookie dictionary
	BSTR* bstrCookieValue ///< [out] value of the cookie read
	)
{
	if (bstrCookieName == NULL || bstrCookieKey == NULL)
	{
		return E_POINTER;
	}

	HRESULT hr;
	CComPtr<IRequestDictionary> spDict; // First step to get cookie collection
	CComVariant vtCookieDict; // ptr to dispinterface IReadCookie
	CComVariant vtCookieName = bstrCookieName; // For the name of the cookie
	CComVariant vtCookieKey = bstrCookieKey; // For the cookie key
	CComVariant vtCookieVal; // For the cookie value
	
	// First you have to get the IRequestDictionary then call get_Item() to get the actual IReadCookie object.
	hr = this->m_piRequest->get_Cookies(&spDict);

	if (SUCCEEDED(hr))
	{
		// Request access to the cookie named "KBTESTCOOKIES" specifically.
		// vtCookieDict will return with the ptr to the IReadCookie object
		hr = spDict->get_Item(vtCookieName, &vtCookieDict);

		if (SUCCEEDED(hr))
		{
			// Get an interface to to IReadCookie;
			CComPtr<IReadCookie> spReadCookie; // Cookie collection
			hr = vtCookieDict.pdispVal->QueryInterface(__uuidof(IReadCookie), (void**)&spReadCookie);
			if (SUCCEEDED(hr))
			{
				hr = spReadCookie->get_Item(vtCookieKey, &vtCookieVal);

				if (SUCCEEDED(hr))
				{
					_bstr_t _bstr_cookie_value = vtCookieVal; // extract BSTR from the VARIANT
					*bstrCookieValue = _bstr_cookie_value.Detach();
				}
				else
				{
					Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSession ReadCookie spReadCookie->get_Item\r\n"); 
				}
			}
			else
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSession ReadCookie vtCookieDict.pdispVal->QueryInterface\r\n");
			}
		}
		else
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSession ReadCookie pDict->get_Item\r\n");
		}
	}
	else
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSession  ReadCookie  m_piRequest->get_Cookies\r\n");
	}

	return hr;
}

/// Write a cookie. This code is inspired by Microsoft at http://support.microsoft.com/kb/240191/en-us
/// Attention : code provide by MS leak if called twice in the same asp page life cycle with the same bstrCookieName and bstrCookieKey to avoid memory leak
HRESULT CAspSession::WriteCookie(
	BSTR bstrCookieName, ///< [in] name of the cookie to write
	BSTR bstrCookieKey, ///< [in] key of the value to add in the cookie dictionary
	BSTR bstrCookieValue ///< [in] value of the cookie to write in the cookie dictionary
	)
{
	if (bstrCookieName == NULL || bstrCookieKey == NULL || bstrCookieValue == NULL)
	{
		return E_POINTER;
	}

	HRESULT hr;
	CComPtr<IRequestDictionary> spDict; // First step to get cookie collection
	CComVariant vtCookieDict; // ptr to dispinterface IWriteCookie
	CComVariant vtCookieName = bstrCookieName; // For the name of the cookie
	CComVariant vtCookieKey = bstrCookieKey; // For the cookie key

	// First you have to get the IRequestDictionary then call get_Item() to get the actual IWriteCookie object.
	hr = m_piResponse->get_Cookies(&spDict);

	if (SUCCEEDED(hr))
	{
		// Request access to the cookie named "KBTESTCOOKIES" specifically.
		// vtCookieDict will return with the ptr to the IWriteCookie object
		hr = spDict->get_Item(vtCookieName, &vtCookieDict);

		if (SUCCEEDED(hr))
		{
			// Get an interface to to IWriteCookie;
			CComPtr<IWriteCookie> pWriteCookie; // Cookie collection
			hr = vtCookieDict.pdispVal->QueryInterface(__uuidof(IWriteCookie), (void**)&pWriteCookie);
			if (SUCCEEDED(hr))
			{
				// Fill the Response buffer with the cookie data and we're done
				// Attention : this is leaking if called twice with the same vtCookieKey for the same cookie (as provided by MS)
				hr = pWriteCookie->put_Item(vtCookieKey, bstrCookieValue);
				if (FAILED(hr))
				{
					Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSession WriteCookie pWriteCookie->put_Item\r\n");
				}
			}
			else
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSession WriteCookie vtCookieDict.pdispVal->QueryInterface\r\n");
			}
		}
		else
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSession WriteCookie spDict->get_Item\r\n", this->m_piRequest);
			
			CString msg;
			msg += L"\tvtCookieName type is: [";
			CComVariant var_vt = (int)vtCookieName.vt;
			CString str_vt(var_vt);
			msg += str_vt + L"]\r\n";
			msg += L"CookieName is: [";
			msg += vtCookieName;
			msg += L"]\r\n";
			if ((short)vtCookieName.bstrVal[5] == 0)
			{
				msg += L"CookieName : le dernier caractrère est un zéro\r\n";
			}
			else
			{
				msg += L"CookieName : le dernier caractrère n'est pas un zéro\r\n";
			}
			
			int* pBstrSize = (int*)(vtCookieName.bstrVal)-1;
			if (*pBstrSize != 10)
			{
				msg += L"CookieName : BSTR size is not equal to 10\r\n";
			}
			else
			{
				msg += L"CookieName : BSTR size is equal to 10\r\n";
			}
			
			if(vtCookieDict.vt == VT_EMPTY)
			{
				msg += L"vtCookieDict is Empty\r\n";
			}
			else
			{
				msg += L"vtCookieDict is not Empty\r\n";
			}

			CComVariant var_hr = hr;
			CString str_hr(var_hr);
			msg += L"HR = [" + str_hr + L"]\r\n";

			Logging::Logger::GetCurrent()->WriteInfo(msg);
		}
	}
	else
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSession  WriteCookie  m_piRequest->get_Cookies\r\n");
	}

	return hr;
}

/// Generate a new SessionId
/// @return S_OK in case of success.
HRESULT CAspSession::GenerateSessionId(
	BSTR* pVal ///< [out] the generated session id
)
{
	HRESULT hr;

	// Use a GUID as a session id
	GUID guid;

	hr = CoCreateGuid(&guid);
	if (FAILED(hr))
	{
		return hr;
	}

	// Convert the guid struct as a _bstr_t
	// The right pattern should be "%X-%X-%X-%X%X-%X%X%X%X%X%X" but we remove "-" to avoid URL encoding translation
	// (replace "-" by "%2D") and because .NET Guid constructor allows to pass a string without "-".
	_bstr_t bstr_guid;
	StringUtility::GuidToBstrWithoutDashes(guid, bstr_guid);

	// Detach the _bstr_t to provide the Bstr the caller
	*pVal = bstr_guid.Detach();

	return hr;
}

/// Set the session cookie
HRESULT CAspSession::SetSessionCookie()
{
	// Ajouut du SessionId dans le cookie
	_bstr_t bstrCookieName = L"Yacht"; // For the cookie name
	_bstr_t bstrCookieKey = L"SessionId"; // For the key

	HRESULT hr;
	hr = WriteCookie(bstrCookieName, bstrCookieKey, this->m_bstrSessionId);
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSession SetCookie WriteCookie SessionId\r\n");
		return hr;
	}

	return hr;
}

/// If found, get the session id from the cookie else set a new session id to the cookie
HRESULT CAspSession::GetSessionId(
	BSTR * pVal ///< [out] The session id.
	)
{
	BSTR bstrCookieName; // For the cookie name
	BSTR bstrCookieKey; // For the key

	// Initialize the temp vars
	bstrCookieName = SysAllocString(L"Yacht");
	bstrCookieKey  = SysAllocString(L"SessionId");

	HRESULT hr;

	hr = this->ReadCookie(bstrCookieName, bstrCookieKey, pVal);
	if (wcslen(*pVal) == 0)
	{ // If the SessionId could not be found on the cookie => create it
		SysFreeString(*pVal); // Free the allocated memory done by the the ReadCookie method
		
		hr = this->GenerateSessionId(pVal);
		if (FAILED(hr))
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSession  GetCookie  SetCookie\r\n");
		}
		else
		{ // Write the cookie to the browser
			hr = this->SetSessionCookie();
			if (FAILED(hr))
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionContent PersistSession unable to SetCookie\r\n");
				return hr;
			}
		}
	}

	SysFreeString(bstrCookieName);
	SysFreeString(bstrCookieKey);

	return hr;
}

/// Returns a Session ID for this user.
STDMETHODIMP CAspSession::get_SessionID(
	BSTR* pVal ///< [out] the SessionID for this user
	)
{
	HRESULT hr;

	if (m_bstrSessionId == NULL)
	{
		hr = this->GetSessionId(&m_bstrSessionId);
		if (FAILED(hr))
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSession get_SessionID GetCookie\r\n");
			return E_FAIL;
		}
	}

	*pVal = SysAllocString(m_bstrSessionId);

	return S_OK;
}

/// Abandon the Session. Remove at end of page life cycle the session into the storage
/// @return S_OK in case of success any other value else
STDMETHODIMP CAspSession::Abandon(void)
{
	// No need to initialize session service in this case as we want to delete session at the end of the page life cycle
	m_bAbandonSession = TRUE;

	return S_OK;
}

/// Get the current session timeout
/// @return S_OK in case of success any other value else
STDMETHODIMP CAspSession::get_Timeout(
	long* plTimeout ///< [out] current session timeout
)
{
	HRESULT hr = ((CAspSessionContents*)this->Contents)->InitializeComponent();
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError CAspSession Abandon unable to intialize component\r\n");
		return hr;
	}

	*plTimeout = m_lTimeOut;

	return hr;
}

/// Set the timeout for the current session
/// @return S_OK in case of success any other value else
STDMETHODIMP CAspSession::put_Timeout(
	long plTimeout ///< [in] new session timeout to set
)
{
	HRESULT hr = ((CAspSessionContents*)this->Contents)->InitializeComponent();
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError CAspSession Abandon unable to intialize component\r\n");
		return hr;
	}

	m_lTimeOut = plTimeout;

	return hr;
}

/// Reload the session from the storage
/// @return S_OK in case of success any other value else
STDMETHODIMP CAspSession::Reload(void)
{
	// Remove the content of the session in memory
	HRESULT hr = ((CAspSessionContents*)this->Contents)->RemoveAll();
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError CAspSession Reload unable to RemoveAll\r\n");
		return hr;
	}

	// Mark the session as uninitialized
	((CAspSessionContents*)this->Contents)->m_bIsSessionInitialized = false;

	// Initialize again the Session
	hr = ((CAspSessionContents*)this->Contents)->InitializeComponent();
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError CAspSession Reload unable to intialize component\r\n");
		return hr;
	}

	return S_OK;
}
