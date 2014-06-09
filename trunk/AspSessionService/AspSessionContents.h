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

// AspSessionContents.h : Declaration of the CAspSessionContents

#pragma once
#include "ContainerType.h"
#include "resource.h"       // main symbols
#include "AspSessionService_i.h"
#include "VCUE_Copy.h"
#include "VCUE_Collection.h"
#include "AspSession.h"
#include "scrrun.tlh"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

typedef IEnumVARIANT EnumeratorInterface;
typedef IAspSessionContents CollectionInterface;

// Typically the copy policy can be calculated from the typedefs defined above:
// typedef VCUE::GenericCopy<ExposedType, ContainerType::value_type> CopyType;
// However, we may want to use a different class, as in this case:
typedef VCUE::MapCopy<ContainerType, VARIANT> CopyType;
// (The advantage of MapCopy is that we don't need to provide implementations 
//  of GenericCopy for all the different pairs of key and value types)

// doc : http://msdn.microsoft.com/en-us/library/td6kz9x0(VS.80).aspx
typedef CComEnumOnSTL<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, CopyType, ContainerType > EnumeratorType;

/// CAspSessionContents proivdes the functionnality of the Session.Content collection
class ATL_NO_VTABLE CAspSessionContents :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CAspSessionContents, &CLSID_AspSessionContents>,
	public IDispatchImpl<IAspSessionContents, &IID_IAspSessionContents, &LIBID_AspSessionServiceLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
private:
	/// The Session.Contents collection
	ContainerType dico;

    /// Create a copy of the bstrString parameter and put all characters of a BSTR to lower case.
    /// @return the lowered string.
	BSTR BstrToLower(
		BSTR bstrString ///< [in] the bstr to lower case
	);

	/// Get the session id as a Guid usable for DB access
	/// @return S_OK if succeed any other value else
	HRESULT GetSessionId(
		_variant_t& vtSessionId ///< [in] The session Id as a Guid
	);

public:
	CAspSessionContents();

	~CAspSessionContents();

	DECLARE_REGISTRY_RESOURCEID(IDR_ASPSESSIONCONTENTS)

	BEGIN_COM_MAP(CAspSessionContents)
		//COM_INTERFACE_ENTRY(IAspSessionContents)
		COM_INTERFACE_ENTRY2(IDispatch, IAspSessionContents)
		COM_INTERFACE_ENTRY(IAspSessionContents)
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

	/// Serialise the content collection into the database
	HRESULT PersistSession();

	/// A pointer to the CAspSession instance that has created the current instance of AspSessionContents
	void* m_piAspSession;

	// IAspSessionContents Methods
	STDMETHOD(get__NewEnum)(IUnknown** ppEnumReturn);
	STDMETHOD(get_Item)(BSTR Key, VARIANT* Value);
	STDMETHOD(put_Item)(BSTR Key, VARIANT Value);
	STDMETHOD(putref_Item)(BSTR Key, IDispatch* Value);
	STDMETHOD(get_Key)(BSTR Key, BSTR* KeyValue);
	STDMETHOD(get_Count)(int* cStrRet);
	STDMETHOD(Remove)(BSTR Key);
	STDMETHOD(RemoveAll)();

	/// Indicate whether the Contents collection is already deserialized
	bool m_bIsSessionInitialized;

	/// Deserialize the dico from the database only the first call to InitializeComponent
	/// @return le code d'erreur (S_OK si tout c'est correctement déroulé)
	HRESULT InitializeComponent();
};

OBJECT_ENTRY_AUTO(__uuidof(AspSessionContents), CAspSessionContents)