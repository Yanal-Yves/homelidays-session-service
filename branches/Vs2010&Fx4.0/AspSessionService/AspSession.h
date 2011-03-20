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

// AspSession.h : Declaration of the CAspSession

#pragma once

#include "resource.h" // main symbols
#include <asptlb.h> // Active Server Pages Definitions
#include <comsvcs.h>
#include "AspSessionContents.h"
#include "AspSessionService_i.h"

/// CAspSession provides the functionnality of a centralizd session service for ASP. When an ASP page first call the Session object the session is (if it exists) deserialized from the database.
class ATL_NO_VTABLE CAspSession :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CAspSession, &CLSID_AspSession>,
	public IDispatchImpl<IAspSession, &IID_IAspSession, &LIBID_AspSessionServiceLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
	/// ASP Session.Contents dictionnary.
	IAspSessionContents *Contents;

	/// Request Object
	IRequest* m_piRequest;

	/// Instanciate a CAspSession class
	CAspSession();

	/// Destructor of the CAspSession class
	~CAspSession();

	DECLARE_REGISTRY_RESOURCEID(IDR_ASPSESSION)

	BEGIN_COM_MAP(CAspSession)
		COM_INTERFACE_ENTRY(IAspSession)
		COM_INTERFACE_ENTRY(IDispatch)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	/// Default generated ATL FinalConstruct method. This method always return S_OK.
	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	/// Default generated ATL FinalRelease method. This method does nothing.
	void FinalRelease()
	{
	}

	// Active Server Pages Methods
	STDMETHOD(OnStartPage)(IUnknown* IUnk);
	STDMETHOD(OnEndPage)();

	// IAspSession
	STDMETHOD(get_Contents)(IAspSessionContents** pVal);
	STDMETHOD(get_Value)(BSTR bstrValue, VARIANT* pVal);
	STDMETHOD(putref_Value)(BSTR bstrValue, VARIANT newVal);
	STDMETHOD(put_Value)(BSTR bstrValue, VARIANT newVal);
	STDMETHOD(get_SessionID)(BSTR* pVal);
	STDMETHOD(Abandon)(void);
	STDMETHOD(get_Timeout)(long* plTimeout);
	STDMETHOD(put_Timeout)(long plTimeout);

private:
	/// Context object
	CComPtr<IObjectContext> m_spObjectContext;

	/// Response Object
	IResponse* m_piResponse;

	/// OnStartPage successful?
	BOOL m_bOnStartPageCalled;

	/// A value indicating whether AbandonSession has been called during the page life cycle
	BOOL m_bAbandonSession;

	/// The current timeout of the Session
	long m_lTimeOut;

	/// Get ASP intrinsic objects
	/// @return S_OK in case of succees anything else in case of faillure
	HRESULT CAspSession::GetASPObject(
		LPCWSTR objectName, ///<[in] Name of the ASP intrinsic objects (ex : Response, Request)
		REFIID iid, ///<[in] Interface Identifier requested
		void **ppObj ///<[out] a pointer to the wanted ASP intrinsic objects
	);

	/// If found, get the session id from the cookie else set a new session id to the cookie
	HRESULT CAspSession::GetSessionId(
		BSTR * pVal ///< [out] the session id
	);

	/// Set the session cookie
	HRESULT SetSessionCookie();

	/// Generate a new SessionId
	/// @return S_OK in case of success. 
	HRESULT CAspSession::GenerateSessionId(
		BSTR* pVal ///< [out] the generated session id 
	);

	/// Write a cookie. This code is provided by Microsoft at http://support.microsoft.com/kb/240191/en-us
	/// Attention : be sure to not call this method twice in the same asp page life cycle with the same bstrCookieName and bstrCookieKey to avoid memory leak
	HRESULT CAspSession::WriteCookie(
		BSTR bstrCookieName, ///< [in] name of the cookie to write
		BSTR bstrCookieKey, ///< [in] key of the value to add in the cookie dictionary
		BSTR bstrCookieValue ///< [in] value of the cookie to write in the cookie dictionary
		);

	/// Read a cookie. This code is provided by Microsoft at http://support.microsoft.com/kb/241492/en-us
	HRESULT CAspSession::ReadCookie(
		BSTR bstrCookieName, ///< [in] name of the cookie to read
		BSTR bstrCookieKey, ///< [in] key of the value to add in the cookie dictionary
		BSTR* bstrCookieValue ///< [out] value of the cookie read
		);

	/// Cached SessionId
	BSTR m_bstrSessionId;
};

OBJECT_ENTRY_AUTO(__uuidof(AspSession), CAspSession)
