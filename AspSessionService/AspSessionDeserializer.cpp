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
#include "AspSessionDeserializer.h"
#include "XmlUtility.h"
#include "Logger.h"
#include "atlsafe.h"

/// Create a new instance of the AspSessionDeserializer class
AspSessionDeserializer::AspSessionDeserializer(
	ContainerType& dico, ///< [in] The session content collection.
	_variant_t& serializedDico ///< [in] An XML (BSTR) serialized content collection.
	)
{
	this->Dico = &dico;
	this->SerializedDico = &serializedDico;
}

/// Destructor of the AspSessionDeserializer class
AspSessionDeserializer::~AspSessionDeserializer(void)
{
}

/// Deserialize an XML serialized content collection to a in memory content collection
HRESULT AspSessionDeserializer::DeserializeSession()
{
	// Get a reader to the XML
	HRESULT hr=S_OK;
	hr = XmlUtility::GetReader(this->SerializedDico, this->SpReader, this->SpStream);
	if (hr != S_OK)
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer DeserializeSession unable to get reader to the Xml \r\n");
		return hr;
	}

	// Build the dico
	hr = BuildDico();
	if (hr != S_OK)
	{ // Libération des ressources
		this->SpReader.Release();
		this->SpStream.Release();
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer DeserializeSession unable to build dico\r\n");
		return hr;
	}

	// Libération des ressources
	this->SpReader.Release();
	this->SpStream.Release();

	return hr;
}

/// Element handler of the build dico method
/// @return S_OK in case of success, any other value if operatio failled
HRESULT AspSessionDeserializer::BuildDico_ElementHandler(
	_bstr_t& bstrKey, ///< [out] The key string extracted from the XML
	_bstr_t& bstrType ///< [out] The type string extracted from the XML
	)
{
	HRESULT hr;

	_bstr_t bstrElement;
	const WCHAR* pwszElementTmp;
	UINT uNbChar;
	hr = this->SpReader->GetLocalName(&pwszElementTmp, &uNbChar);
	if (hr != S_OK)
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer BuildDico unable to getlocalname \r\n");
		return hr;
	}

	// Make a deep copy in order to continue parsing without loosing the Key
	bstrElement = pwszElementTmp;

	CString strElement = pwszElementTmp;
	if (strElement != L"Session")
	{ // Seul l'élément Session n'a pas de key et de type
		hr = ExtractKeyAndTypeFromItemAttributes(bstrKey, bstrType);
		if (hr != S_OK)
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer DeserializeSession unable to ExtractKeyAndTypeFromItemAttributes \r\n");
			return hr;
		}

		CString type = bstrType;
		if (type == L"Null")
		{
			VARIANT source;
			VariantInit(&source);
			source.vt=VT_NULL;
			(*this->Dico)[bstrKey] = source;
		}

		if (type == L"Table")
		{
			unsigned short usDim;
			SAFEARRAYBOUND* psabBounds;

			// Get the dimension and bounds from Item node
			ExtractDimAndBoundsFromItemAttributes(&usDim, &psabBounds);

			// Create the appropriate SAFEARAY
			CComSafeArray<VARIANT> pSar = CComSafeArray<VARIANT>(psabBounds, usDim);

			// Release resources
			delete [] psabBounds;
			
			// Create the variant to add into the dico
			VARIANT variantArray;
			variantArray.vt =  VT_VARIANT | VT_ARRAY;
			variantArray.parray = *pSar.GetSafeArrayPtr();

			hr = ExtractDataFromCollection(variantArray);
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer DeserializeSession unable to ExtractDataFromCollection \r\n");

				return hr;
			}

			(*this->Dico)[bstrKey] = variantArray; // Fait une copie des structure mémoire de variantArray
		}

		if (type == L"Dictionary")
		{
			// Create the appropriated Scripting::IDictionaryPtr
			Scripting::IDictionaryPtr spDictionary;
			hr = spDictionary.CreateInstance(__uuidof(Scripting::Dictionary));
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer BuildDico spDictionary.CreateInstance failled\r\n");
				return hr;
			}

			// Put the dictionary into a Variant
			VARIANT vDictionary;
			VariantInit(&vDictionary);
			vDictionary.vt = VT_DISPATCH;
			vDictionary.pdispVal = spDictionary;

			hr = ExtractDataFromCollection(vDictionary);
			if (hr != S_OK)
			{
				spDictionary->Release();
				Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer BuildDico ExtractDataFromDictionary failled\r\n");
				return hr;
			}

			(*this->Dico)[bstrKey] = vDictionary;
		}

		if (type == L"Nothing")
		{
			VARIANT vNothing;
			VariantInit(&vNothing);
			vNothing.vt = VT_DISPATCH;
			vNothing.pdispVal = NULL;

			(*this->Dico)[bstrKey] = vNothing;
		}
	}

	return hr;
}

HRESULT AspSessionDeserializer::BuildDico()
{
	HRESULT hr;
	XmlNodeType nodeType;

	// bstrKey and bstrType are initialized in a "case" statement of the while loop and used in an other "case" statement.
	_bstr_t bstrKey;
	_bstr_t bstrType;
	_bstr_t bstrValue;

	while (S_OK == (hr = this->SpReader->Read(&nodeType)))
	{ // Read until there is no more nodes
		 switch (nodeType)
		 {
			 case XmlNodeType_Element:
				{
					BuildDico_ElementHandler(bstrKey, bstrType);
				} break;
			case XmlNodeType_Text:
				{
					// Extract the value from the XML
					const WCHAR* pwszValueTmp;
					hr = this->SpReader->GetValue(&pwszValueTmp, NULL);
					if (hr != S_OK)
					{
						Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer DeserializeSession unable to  Extract the value from the XML \r\n");
						return hr;
					}

					if (pwszValueTmp != NULL)
					{
						bstrValue = pwszValueTmp;
					}
					else
					{ // Should not happen
						hr = E_FAIL;
						Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer DeserializeSession Should not happen pwszValueTmp == NULL \r\n");
						return hr;
					}
					
					// Creation of a VARIANT initialize with the value and type extracted
					_variant_t varToAdd;
					hr = CreateVariantFromValueAndType(bstrValue, bstrType, varToAdd);
					if (hr != S_OK)
					{
						Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer DeserializeSession unable to  Createa VARIANT initialize with the value and type extracted \r\n");
						return hr;
					}
					
					// Add the item to the dico
					(*this->Dico)[bstrKey] = varToAdd;
				} break;
			case XmlNodeType_CDATA:
				{ // Only String are handled in CDATA
					// Extract the value from the XML
					const WCHAR* pwszValueTmp;
					hr = this->SpReader->GetValue(&pwszValueTmp, NULL);
					if (hr != S_OK)
					{
						Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer DeserializeSession unable to get value of string \r\n");
						return hr;
					}

					_variant_t varToAdd;
					if (pwszValueTmp != NULL)
					{
						varToAdd = pwszValueTmp;
					}
					else
					{ // Should not happen
						hr = E_FAIL;
						Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer DeserializeSession Should not happen pwszKey == NULL \r\n");
						return hr;
					}

					// Add the item to the dico
					(*this->Dico)[bstrKey] = varToAdd;
				} break;
		 }
	} // while (S_OK == (hr = this->SpReader->Read(&nodeType)))
	
	return S_OK;
}

/// Create a variant withe the provided type and value
/// @return S_OK in case of success, any other value if operation failled
HRESULT AspSessionDeserializer::CreateVariantFromValueEnumAndType(
	const _bstr_t& bstrValue, /// [in] value to add in the created variant
	VARENUM vtType, ///< [in] type of the value
	_variant_t& dest ///< [in, out] An empty variant that will contain the resulting variant to add into the safe array
	)
{
	HRESULT hr;

	// Create a string variant
	VARIANT source;
	VariantInit(&source);
	source.vt = VT_BSTR;
	source.bstrVal = bstrValue;

	// Convert it into the wanted type
	hr = VariantChangeType(
		&dest,
		&source,
		0,
		(VARTYPE) vtType); // Cast allowed because value defined for VARENUM are define within 16 bits

	if (hr != S_OK)
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer CreateVariantFromValueEnumAndType unable to Convert it into the wanted type \r\n");
		
		return hr;
	}

	return hr;
}

/// Create a variant with the provided value and type
/// @return S_FALSE in case of failure, else S_OK
HRESULT AspSessionDeserializer::CreateVariantFromValueAndType(
	const _bstr_t& bstrValue, ///< [in] the value to init the variant with
	const _bstr_t& bstrType, ///<[in] the type of value to init the varaint with
	_variant_t& variantValue ///< [in, out] the safe array containing the result of the deserialization
	)
{
	HRESULT hr;

	CString pstrType = bstrType;
	
	if (pstrType == L"Empty")
	{
		hr = CreateVariantFromValueEnumAndType(bstrValue, VT_EMPTY, variantValue);
		if (hr != S_OK)
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer CreateVariantFromValueAndType unable to CreateVariantFromValueEnumAndType type empty \r\n");
			return hr;
		}
	}
	else if (pstrType == L"SingedByte")
	{
		hr = CreateVariantFromValueEnumAndType(bstrValue, VT_I1, variantValue);
		if (hr != S_OK)
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer CreateVariantFromValueAndType unable to CreateVariantFromValueEnumAndType type signedbyte \r\n");

			return hr;
		}
	}
	else if (pstrType == L"SingedShort")
	{
		hr = CreateVariantFromValueEnumAndType(bstrValue, VT_I2, variantValue);
		if (hr != S_OK)
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer CreateVariantFromValueAndType unable to CreateVariantFromValueEnumAndType type SingedShort \r\n");

			return hr;
		}
	}
	else if (pstrType == L"SingedInteger")
	{
		hr = CreateVariantFromValueEnumAndType(bstrValue, VT_I4, variantValue);
		if (hr != S_OK)
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer CreateVariantFromValueAndType unable to CreateVariantFromValueEnumAndType type SingedInteger \r\n");

			return hr;
		}
	}
	else if (pstrType == L"UnsingedByte")
	{
		hr = CreateVariantFromValueEnumAndType(bstrValue, VT_UI1, variantValue);
		if (hr != S_OK)
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer CreateVariantFromValueAndType unable to CreateVariantFromValueEnumAndType type UnsingedByte \r\n");

			return hr;
		}
	}
	else if (pstrType == L"UnsingedShort")
	{
		hr = CreateVariantFromValueEnumAndType(bstrValue, VT_UI2, variantValue);
		if (hr != S_OK)
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer CreateVariantFromValueAndType unable to CreateVariantFromValueEnumAndType type UnsingedShort \r\n");

			return hr;
		}
	}
	else if (pstrType == L"UnsingedInteger")
	{
		hr = CreateVariantFromValueEnumAndType(bstrValue, VT_UI4, variantValue);
		if (hr != S_OK)
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer CreateVariantFromValueAndType unable to CreateVariantFromValueEnumAndType type UnsingedInteger \r\n");

			return hr;
		}
	}
	else if (pstrType == L"SingedMachineInteger")
	{
		hr = CreateVariantFromValueEnumAndType(bstrValue, VT_INT, variantValue);
		if (hr != S_OK)
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer CreateVariantFromValueAndType unable to CreateVariantFromValueEnumAndType type SingedMachineInteger \r\n");

			return hr;
		}
	}
	else if (pstrType == L"UnsingedMachineInteger")
	{
		hr = CreateVariantFromValueEnumAndType(bstrValue, VT_UINT, variantValue);
		if (hr != S_OK)
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer CreateVariantFromValueAndType unable to CreateVariantFromValueEnumAndType type UnsingedMachineInteger \r\n");

			return hr;
		}
	}
	else if (pstrType == L"Decimal")
	{
		hr = CreateVariantFromValueEnumAndType(bstrValue, VT_DECIMAL, variantValue);
		if (hr != S_OK)
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer CreateVariantFromValueAndType unable to CreateVariantFromValueEnumAndType type Decimal \r\n");

			return hr;
		}
	}
	else if (pstrType == L"Real4")
	{
		hr = CreateVariantFromValueEnumAndType(bstrValue, VT_R4, variantValue);
		if (hr != S_OK)
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer CreateVariantFromValueAndType unable to CreateVariantFromValueEnumAndType type Real4 \r\n");

			return hr;
		}
	}
	else if (pstrType == L"Real8")
	{
		hr = CreateVariantFromValueEnumAndType(bstrValue, VT_R8, variantValue);
		if (hr != S_OK)
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer CreateVariantFromValueAndType unable to CreateVariantFromValueEnumAndType type Real8 \r\n");

			return hr;
		}
	}
	else if (pstrType == L"Bool")
	{
		hr = CreateVariantFromValueEnumAndType(bstrValue, VT_BOOL, variantValue);
		if (hr != S_OK)
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer CreateVariantFromValueAndType unable to CreateVariantFromValueEnumAndType type Bool \r\n");

			return hr;
		}
	}
	else if (pstrType == L"Currency")
	{
		hr = CreateVariantFromValueEnumAndType(bstrValue, VT_CY, variantValue);
		if (hr != S_OK)
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer CreateVariantFromValueAndType unable to CreateVariantFromValueEnumAndType type Currency \r\n");

			return hr;
		}
	}
	else if (pstrType == L"Date")
	{
		hr = CreateVariantFromValueEnumAndType(bstrValue, VT_DATE, variantValue);
		if (hr != S_OK)
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer CreateVariantFromValueAndType unable to CreateVariantFromValueEnumAndType type Date \r\n");

			return hr;
		}
	}
	else if (pstrType == L"Null")
	{
		hr = CreateVariantFromValueEnumAndType(bstrValue, VT_NULL, variantValue);
		if (hr != S_OK)
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer CreateVariantFromValueAndType unable to CreateVariantFromValueEnumAndType type Null \r\n");

			return hr;
		}
	}
	else
	{
		hr = E_UNEXPECTED;
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer CreateVariantFromValueAndType type Unexpected \r\n");

		return hr;
	}

	return hr;
}

/// Deserialize safeArray
/// @return S_FALSE in case of failure, else S_OK
HRESULT AspSessionDeserializer::ExtractDataFromCollection(
	VARIANT& variantArray ///< [in, out] the safe array containing the result of the deserialization
	)
{
	HRESULT hr;
	XmlNodeType nodeType;
	_bstr_t bstrKey;
	_bstr_t bstrType;
	_bstr_t bstrValue;
	_bstr_t bstrCoord; // bstrCoord is initialized in a "case" statement of the while loop and used in an other

	while (S_OK == (hr = this->SpReader->Read(&nodeType)))
	{ // Read until there is no more nodes
		switch (nodeType)
		{
		case XmlNodeType_EndElement:
			{
				const WCHAR* pwszEndElement;
				hr = this->SpReader->GetLocalName(&pwszEndElement, NULL);
				if (hr != S_OK)
				{
					Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDataFromTable unable to GetLocalName \r\n");
					return hr;
				}

				CString strEndElement = pwszEndElement;
				if (strEndElement == L"Table" || strEndElement == L"Dictionary")
				{ // On a fini de charger la table
					return hr;
				}
			} break;
		case XmlNodeType_Element:
			{
				hr = ExtractKeyAndTypeFromItemAttributes(bstrKey, bstrType);
				if (hr != S_OK)
				{
					Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDataFromCollection unable to ExtractKeyAndTypeFromItemAttributes \r\n");
					return hr;
				}

				CString type = bstrType;
				if (type == L"Table")
				{
					unsigned short usDim;
					SAFEARRAYBOUND* psabBounds;

					// Get the dimension and bounds from Item node
					hr = ExtractDimAndBoundsFromItemAttributes(&usDim, &psabBounds);
					if (hr != S_OK)
					{
						delete [] psabBounds;
						Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDataFromCollection unable to Get the dimension and bounds from Item node \r\n");
						return hr;
					}

					// Create the appropriated SAFEARAY
					CComSafeArray<VARIANT> pSar = CComSafeArray<VARIANT>(psabBounds, usDim);
					delete [] psabBounds;

					// Put the SAFEARRAY into a Variant
					variant_t variantArraySub;
					variantArraySub.vt =  VT_VARIANT | VT_ARRAY;
					variantArraySub.parray = *(pSar.GetSafeArrayPtr());

					// Extract coordonate from xml as a string
					hr = ExtractCoordinatesFromItemAttributes(bstrCoord);
					if (FAILED(hr))
					{
						Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDataFromCollection unable to Extract coordonate from xml as a string \r\n");
						return hr;
					}

					// Extract data of the subarray
					hr = ExtractDataFromCollection(variantArraySub);
					if (FAILED(hr))
					{
						Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDataFromCollection unable to Extract data of the subarray \r\n");
						return hr;
					}

					// Add the sub array into the array
					hr = AddVariantToContainer(bstrCoord, bstrKey, &variantArray, &variantArraySub);
					if (hr != S_OK)
					{
						Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDataFromCollection AddVariantToSafeArray (type : table) failled\r\n");
						return hr;
					}
				}
				else if (type == L"Dictionary")
				{
					// Create the appropriated Scripting::IDictionaryPtr
					Scripting::IDictionaryPtr spDictionary;
					hr = spDictionary.CreateInstance(__uuidof(Scripting::Dictionary));
					if (hr != S_OK)
					{
						Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDataFromCollection spDictionary.CreateInstance failled \r\n");
						return hr;
					}

					// Put the dictionary into a Variant
					variant_t vDictionary;
					vDictionary.vt = VT_DISPATCH;
					vDictionary.pdispVal = spDictionary;
					spDictionary->AddRef(); // As the destructor of spDictionary will release the dictionary, we call AddRef to keep the instance of the dictionary

					// Extract coordonate from xml as a string
					hr = ExtractCoordinatesFromItemAttributes(bstrCoord);
					if (FAILED(hr))
					{
						Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDataFromCollection unable to Extract coordonate from xml as a string \r\n");
						return hr;
					}

					// Extract data of the Dictionary
					hr = ExtractDataFromCollection(vDictionary);
					if (FAILED(hr))
					{
						Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDataFromCollection unable to Extract data of the subarray \r\n");
						return hr;
					}

					// Add the sub array into the array
					hr = AddVariantToContainer(bstrCoord, bstrKey, &variantArray, &vDictionary);
					if (hr != S_OK)
					{
						spDictionary->Release();
						Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDataFromCollection AddVariantToSafeArray (type : table) failled\r\n");
						return hr;
					}
				}
				else
				{
					// Extract the coordinate from the XML
					hr = ExtractCoordinatesFromItemAttributes(bstrCoord);
					if (FAILED(hr))
					{
						Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDataFromCollection unable to  Extract the coordinate from the XML\r\n");
						return hr;
					}

				}
			} break;
		case XmlNodeType_Text:
			{
				// Extract the value from the XML
				const WCHAR* pwszValueTmp;
				hr = this->SpReader->GetValue(&pwszValueTmp, NULL);
				if (hr != S_OK)
				{
					Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDataFromCollection unable to Extract the value from the XML\r\n");
					return hr;
				}
				
				// Make a deep copy
				if (pwszValueTmp != NULL)
				{
					bstrValue = pwszValueTmp;
				}
				else
				{ // Should not happen
					hr = E_FAIL;
					Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDataFromCollection Should not happen pwszValueTmp == NULL \r\n");
					return hr;
				}

				// Creation of a VARIANT initialized with the value and type extracted
				_variant_t varToAdd;
				hr = CreateVariantFromValueAndType(bstrValue, bstrType, varToAdd);
				if (hr != S_OK)
				{
					Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDataFromCollection  unable to  Createa VARIANT initialize with the value and type extracted \r\n");
					return hr;
				}

				// Add the variant to the container
				hr = AddVariantToContainer(bstrCoord, bstrKey, &variantArray, &varToAdd);
				if (hr != S_OK)
				{
					Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDataFromCollection AddVariantToContainer(XmlNodeType_Text) failled\r\n");
					return hr;
				}
			} break;
		case XmlNodeType_CDATA:
			{ // Only String are handled in CDATA
				// Get the value to add into the safe array
				const WCHAR* pwszValueTmp;
				hr = this->SpReader->GetValue(&pwszValueTmp, NULL);
				if (hr != S_OK)
				{
					Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDataFromCollection unable to Get the value to add into the safe array\r\n");

					return hr;
				}

				// Creation of a VARIANT initialized with the value and type extracted
				_variant_t varToAdd;
				if (pwszValueTmp != NULL)
				{
					varToAdd = pwszValueTmp;
				}
				else
				{ // Should not happen
					hr = E_FAIL;
					Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDataFromCollection case XmlNodeType_CDATA Should not happen pwszValueTmp == NULL \r\n");
					return hr;
				}

				// Add the variant to the container
				hr = AddVariantToContainer(bstrCoord, bstrKey, &variantArray, &varToAdd);
				if (hr != S_OK)
				{
					Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDataFromCollection AddVariantToContainer(XmlNodeType_Text) failled\r\n");
					return hr;
				}
			} break;
		} // while (S_OK == (hr = this->SpReader->Read(&nodeType)))
	}

	return hr;
}

/// Add a variant into a container
/// @return S_OK if success, DISP_E_BADINDEX if the specified index was invalid if container is a safe Array,
/// E_INVALIDARG if one of the arguments is invalid,
/// E_OUTOFMEMORY if Memory could not be allocated for the element and other in case of failure
HRESULT AspSessionDeserializer::AddVariantToContainer(
		const WCHAR* pwszCoord, ///< [in] The coordiante of the variant to add. (e.g. L"2,1") in case the container is a SafeArray
		const WCHAR* pwszKey, ///< [in] The key where to add the variant in case of Scripting.Dictionary
		VARIANT* pvContainer, ///< [in] The Container in which the variant is added
		VARIANT* pVariantToAdd ///< [in] the variant to add into the safe array
	)
{
	HRESULT hr;

	if (pvContainer->vt == VT_DISPATCH)
	{
		Scripting::IDictionaryPtr spDictionary;
		spDictionary = (Scripting::IDictionaryPtr) pvContainer->pdispVal;
		variant_t vKey = pwszKey;

		hr = spDictionary->Add(&vKey, pVariantToAdd);
		if (FAILED(hr))
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tAspSessionDeserializer AddVariantToContainer spDictionary->Add failled\r\n");
		}
	}
	else
	{ // Add the variant to the safe array
		if (pwszCoord != NULL)
		{
			hr = AddVariantToSafeArray(pwszCoord, pvContainer->parray, pVariantToAdd);
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer AddVariantToContainer AddVariantToSafeArray failled\r\n");
			}
		}
		else
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer AddVariantToContainer pwszCoord is NULL\r\n");
			hr = E_FAIL;
		}
	}

	return hr;
}

/// Extract data from attributes of the Item element
/// @return S_FALSE if failled, else S_OK
HRESULT AspSessionDeserializer::ExtractKeyAndTypeFromItemAttributes(
	_bstr_t& bstrKey, ///< [out] The extracted key
	_bstr_t& bstrType ///< [out] The extracted type
	)
{
	HRESULT hr;

	// Key : On se positionne sur le premier attribut
	hr = this->SpReader->MoveToFirstAttribute();
	if (hr != S_OK)
	{ // There should be a first argument
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractKeyAndTypeFromItemAttributes There should be a first argument \r\n");
		return hr;
	}

	// Get the Key from the XML
	const WCHAR* pwszKeyTmp;
	UINT uNbChar = 0;
	hr = this->SpReader->GetValue(&pwszKeyTmp, &uNbChar);
	if (hr != S_OK)
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractKeyAndTypeFromItemAttributes Unable to GetValue of first argument \r\n");
		return hr;
	}

	// Make a deep copy in order to continue parsing without loosing the Key
	bstrKey = pwszKeyTmp;

	// Type : on se positionne sur le deuxième attribut
	hr = this->SpReader->MoveToNextAttribute();
	if (hr != S_OK)
	{ // There should be a second argument
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractKeyAndTypeFromItemAttributes There should be a second argument \r\n");
		return hr;
	}

	// Get the type from the XML
	const WCHAR* pwszTypeTmp;
	hr = this->SpReader->GetValue(&pwszTypeTmp, &uNbChar);
	if (hr != S_OK)
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractKeyAndTypeFromItemAttributes Unable to getvalue of second argument\r\n");
		return hr;
	}

	// Make a deep copy in order to continue parsing without loosing the Key
	bstrType = pwszTypeTmp;

	return hr;
}

/// Extract safe array bounds form item attribute. /!\ Do not call this method if the current Item is not a Table
/// @return S_FALSE in case of error, else S_OK
HRESULT AspSessionDeserializer::ExtractDimAndBoundsFromItemAttributes(
		unsigned short* pusDimension, ///< [out] the extracted dimension
		SAFEARRAYBOUND** psabBounds ///< [out] the extracted bounds
		)
{
	HRESULT hr;

	// Extract Dimension from the XML as a string
	hr = this->SpReader->MoveToNextAttribute();
	if (hr != S_OK)
	{ // There should be a third argument
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDimAndBoundsFromItemAttributes There should be a third argument \r\n");
		return hr;
	}

	const WCHAR * pwszDim;
	hr = this->SpReader->GetValue(&pwszDim, NULL);
	if (hr != S_OK)
	{
		return hr;
	}

	// Convert dimension an unsigned short
	// Dimension is unsigned short => the serialization should have produce an unsigned short
	*pusDimension = (unsigned short)_wtoi(pwszDim);

	// Extract the Bounds from the XML as a string
	hr = this->SpReader->MoveToNextAttribute();
	if (hr != S_OK)
	{ // There should be a third argument
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDimAndBoundsFromItemAttributes There should be a third argument \r\n");
		return hr;
	}

	LPCWSTR pwszBounds;
	hr = this->SpReader->GetValue(&pwszBounds, NULL);
	if (hr != S_OK)
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDimAndBoundsFromItemAttributes unable to get value of  third argument \r\n");
		return hr;
	}

	// Split the string ',' is the delimiter
	CString* arrStringArray = new CString[*pusDimension];
	hr = StringSplit(pwszBounds, *pusDimension, arrStringArray);
	if (hr != S_OK)
	{
		delete [] arrStringArray;
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractDimAndBoundsFromItemAttributes unable to split string \r\n");
		return hr;
	}

	// Create the SAFEARRAYBOUND table => the caller should delete it
	__try
	{
		*psabBounds = new SAFEARRAYBOUND[*pusDimension];

		LONG lMaxIndex = (*pusDimension)-1;
		for (LONG i = lMaxIndex ; i>= 0 ; i--)
		{
			int count = _wtoi(arrStringArray[i]);

			(*psabBounds)[lMaxIndex-i].cElements = count; // set the number of elements
			(*psabBounds)[lMaxIndex-i].lLbound = 0; // set the lower bound (lowest index)
		}
	}
	__finally
	{ // even if an unexpected error occurs release memory
		delete [] arrStringArray;
	}

	return hr;
}

/// Extract coordinate from data from attributes of the Item element.
// /!\ Do not call this method if the current Item within a Table.
/// @return S_FALSE in case of failure, else S_OK
HRESULT AspSessionDeserializer::ExtractCoordinatesFromItemAttributes(
	_bstr_t& bstrCoord ///< [out] The extracted coordinate of the item in the table
	)
{
	HRESULT hr;

	// Move to next attribute that should be the Coord attribute
	hr = this->SpReader->MoveToNextAttribute();
	if (SUCCEEDED(hr))
	{ // hr is S_OK for a table and S_FALSE for a Dictionary => both case succeed
		if (hr == S_OK)
		{ // there is a third attribute => should be Coord => Get the coordinate as a string
			const WCHAR* pwszCoordTmp = NULL;
			hr = this->SpReader->GetValue(&pwszCoordTmp, NULL);
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractCoordinatesFromItemAttributes unable to getvalue\r\n");
			}

			bstrCoord = pwszCoordTmp;
		}
	}
	else
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer ExtractCoordinatesFromItemAttributes MoveToNextAttribute failled\r\n");
	}

	return hr;
}

/// Split a string. The delimiter is ','
/// @return S_FALSE in case of failure, else S_OK
HRESULT AspSessionDeserializer::StringSplit(
	const WCHAR* pwszValue, ///< [in] input string to split
	const unsigned short usDimension, ///< [in] dimension of the safearray
	CString* arrStringArray ///< [out] pointer to a table of pointer to splited strings
	)
{
	HRESULT hr = S_OK;

	CString str(pwszValue);
	int curPos = 0;
	CString resToken;

	resToken = str.Tokenize(L",", curPos);

	for (int i = 0; i < usDimension; i++)
	{
		arrStringArray[i] = resToken;
		resToken = str.Tokenize(L",", curPos);
	}

	return hr;
}

HRESULT AspSessionDeserializer::GetCoordinateFromStringCoordinate(
	const WCHAR* pwszCoord, ///< [in] the coordinate as a string to split
	const unsigned short usDimension, ///< [in] dimension of the safearray
	LONG** plCoordinate ///< [out] A table of LONG representing the coordinate
	)
{
	HRESULT hr;

	// Split the string ',' is the delimiter
	CString* arrStringArray = new CString[usDimension];
	hr = StringSplit(pwszCoord, usDimension, arrStringArray);
	if (hr != S_OK)
	{
		delete [] arrStringArray;
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer GetCoordinateFromStringCoordinate unable to split string \r\n");
		return hr;
	}

	// Get the coordinate as table of LONG => the caller should delete it
	*plCoordinate =  new LONG[usDimension]; // this should be release by the caller

	__try
	{
		for (int i = 0 ; i<usDimension ; i++)
		{
			int iCoord = _wtoi(arrStringArray[i]);

			(*plCoordinate)[i] = iCoord;
		}
	}
	__finally
	{ // even if an unexpected error occurs release memory
		delete [] arrStringArray;
	}

	return hr;
}

/// Add a variant into a safe array
/// @return S_OK if success, DISP_E_BADINDEX if the specified index was invalid,
/// E_INVALIDARG if one of the arguments is invalid,
/// E_OUTOFMEMORY if Memory could not be allocated for the element.
HRESULT AspSessionDeserializer::AddVariantToSafeArray(
	const WCHAR* pwszCoord, ///< [in] The coordiante of the variant to add. (e.g. L"2,1")
	SAFEARRAY* pArray, ///< [in] The safe array in which the variant is added
	VARIANT* pVariantToAdd ///< [in] the variant to add into the safe array
	)
{
	HRESULT hr;
	
	// Get the dimension of the table;
	USHORT usDim = pArray->cDims;

	// Get the coordinate as a table of LONG
	LONG* pulCoordinate;
	hr = GetCoordinateFromStringCoordinate(pwszCoord, usDim, &pulCoordinate);
	if (hr != S_OK)
	{
		delete [] pulCoordinate; // Allocated by GetCoordinateFromStringCoordinate
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer AddVariantToSafeArray unable to  Get the coordinate as a table of LONG \r\n");
		return hr;
	}

	// Add the sub array into the array
	hr = SafeArrayPutElement(pArray, pulCoordinate, pVariantToAdd);
	if (hr != S_OK)
	{
		delete [] pulCoordinate; // Allocated by GetCoordinateFromStringCoordinate
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError AspSessionDeserializer AddVariantToSafeArray unable to Add the sub array into the array \r\n");
		return hr;
	}

	delete [] pulCoordinate; // Allocated by GetCoordinateFromStringCoordinate

	return hr;
}
