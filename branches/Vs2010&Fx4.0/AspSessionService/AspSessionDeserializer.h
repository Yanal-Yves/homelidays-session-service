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

#include <comutil.h>
#include <ATLComTime.h> // for COleDateTime and CString
#include "ContainerType.h" // for ContainerType
#include <xmllite.h>
#include <atlsafe.h>
#include "scrrun.tlh"

typedef std::map<_bstr_t, _variant_t> ContainerType;

/// AspSessionSerializer allows to de-serialize the Session.Content collection
class AspSessionDeserializer
{
public:
	/// Create a new instance of the AspSessionDeserializer class
	AspSessionDeserializer(
			ContainerType& dico, ///< [in] The session content collection.
			_variant_t& serializedDico ///< [in] An XML (BSTR) serialized content collection.
			);

	/// Destructor of the AspSessionDeserializer class
	~AspSessionDeserializer(void);

	HRESULT DeserializeSession();
private:
	/// The dictionry where to deserialize
	ContainerType* Dico;

	/// An XML (as a variant BSTR) serialized content collections
	_variant_t SerializedDico;

	/// Smart pointer to a reader to the serializedDico
	CComPtr<IXmlReader> SpReader;
	
	/// Smart pointer to the stream used by the spReader
	CComPtr<IStream> SpStream;

	/// Element handler of the build dico method
	/// @return S_OK in case of success, any other value if operatio failled
	HRESULT AspSessionDeserializer::BuildDico_ElementHandler(
		_bstr_t& bstrKey, ///< [out] a pointer to a pointer of a copy of the key string extracted from the XML
		_bstr_t& bstrType ///< [out] a pointer to a pointer of a copy of the type string extracted from the XML
		);

	HRESULT AspSessionDeserializer::BuildDico();

	/// Create a variant withe the provided type and value
	/// @return S_OK in case of success, any other value if operation failled
	HRESULT AspSessionDeserializer::CreateVariantFromValueEnumAndType(
		const _bstr_t& pwszValue, ///< [in] value to add in the created variant
		VARENUM vtType, ///< [in] type of the value
		_variant_t& dest ///< [in, out] An empty variant that will contain the resulting variant to add into the safe array
		);

	/// Create a variant with the provided value and type
	/// @return S_OK in case of success, any other value if operation failled
	HRESULT AspSessionDeserializer::CreateVariantFromValueAndType(
		const _bstr_t& pwszValue, ///< [in] the value to init the variant with
		const _bstr_t& pwszType, ///<[in] the type of value to init the varaint with 
		_variant_t& variantValue ///< [in, out] the safe array containing the result of the deserialization
		);

	/// Deserialize safeArray
	/// @return S_FALSE in case of failure, else S_OK
	HRESULT AspSessionDeserializer::ExtractDataFromCollection(
		VARIANT& variantArray ///< [in, out] the safe array containing the result of the deserialization
		);

	/// Extract data from attributes of the Item element
	/// @return S_FALSE if failled, else S_OK
	HRESULT AspSessionDeserializer::ExtractKeyAndTypeFromItemAttributes(
		_bstr_t& bstrKey, ///< [out] The extracted key
		_bstr_t& bstrType ///< [out] The extracted type
		);

	/// Extract safe array bounds form item attribute. /!\ Do not call this method if the current Item is not a Table
	/// @return S_FALSE in case of error, else S_OK
	HRESULT AspSessionDeserializer::ExtractDimAndBoundsFromItemAttributes(
		unsigned short* pusDimension, ///< [in, out] the extracted dimension
		SAFEARRAYBOUND** psabBounds ///< [out] the extracted bounds
		);

	/// Extract coordinate from data from attributes of the Item element  /!\ Do not call this method if the current Item within a Table
	/// @return S_FALSE in case of failure, else S_OK
	HRESULT AspSessionDeserializer::ExtractCoordinatesFromItemAttributes(
		_bstr_t& bstrCoord ///< [out] The extracted coordinate of the item in the table
		);

	/// Split a string. The delimiter is ','
	/// @return S_FALSE in case of failure, else S_OK
	HRESULT AspSessionDeserializer::StringSplit(
		const WCHAR* pwszValue, ///< [in] input string to split
		const unsigned short usDimension, ///< [in] dimension of the safearray
		CString* arrStringArray ///< [out] pointer to a table of pointer to splited strings
		);

	HRESULT AspSessionDeserializer::GetCoordinateFromStringCoordinate(
		const WCHAR* pwszCoord, ///< [in] the coordinate as a string to split
		const unsigned short usDimension, ///< [in] dimension of the safearray
		LONG** plCoordinate ///< [out] A table of LONG representing the coordinate
		);

	/// Add a variant into a safe array
	/// @return S_OK if success, DISP_E_BADINDEX if the specified index was invalid,
	/// E_INVALIDARG if one of the arguments is invalid, 
	/// E_OUTOFMEMORY if Memory could not be allocated for the element. 
	HRESULT AspSessionDeserializer::AddVariantToSafeArray(
		const WCHAR* pwszCoord, ///< [in] The coordiante of the variant to add. (e.g. L"2,1")
		SAFEARRAY* pArray, ///< [in] The safe array in which the variant is added
		VARIANT* pVariantToAdd ///< [in] the variant to add into the safe array
		);

	/// Add a variant into a container
	/// @return S_OK if success, DISP_E_BADINDEX if the specified index was invalid if container is a safe Array,
	/// E_INVALIDARG if one of the arguments is invalid, 
	/// E_OUTOFMEMORY if Memory could not be allocated for the element and other in case of failure
	HRESULT AddVariantToContainer(
		const WCHAR* pwszCoord, ///< [in] The coordiante of the variant to add. (e.g. L"2,1")	
		const WCHAR* pwszkey, ///< [in] The key where to add the variant in case of Scripting.Dictionary
		VARIANT* pvContainer, ///< [in] The Container in which the variant is added
		VARIANT* pVariantToAdd ///< [in] the variant to add into the safe array
		);
};
