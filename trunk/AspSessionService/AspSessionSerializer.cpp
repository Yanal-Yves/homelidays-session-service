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
#include "AspSessionSerializer.h"
#include "Logger.h"
#include "DispatchUtility.h"

/// Create a new instance of AspSessionSerializer class
AspSessionSerializer::AspSessionSerializer(void)
{
}

/// Destructor of the AspSessionSerializer class
AspSessionSerializer::~AspSessionSerializer(void)
{
}

/// Serialize a collection of type ContainerType to XML
HRESULT AspSessionSerializer::SerializeSession(
	ContainerType& dico, ///< [in] A reference to the dico to serialize
	CString& serializeDico ///< [out] the resulting serialize dico
	)
{
	HRESULT hr;

	// On sérialize en XML la collection
	serializeDico = L"<?xml version=\"1.0\"?><Session>";
	
	ContainerType::iterator dico_iter;
	for ( dico_iter = dico.begin( ) ; dico_iter != dico.end( ); dico_iter++ )
	{ // On parcours la collection
		hr = GenerateXmlItemNode(dico_iter, serializeDico);
		if (hr != S_OK)
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeSession Unable to generatexml itme node \r\n");
			return hr;
		}
	}

	serializeDico += L"</Session>";

	hr = S_OK;

	return hr;
}

/// Get the nex Coordinate
/// @return S_OK if plCoord point to the next element.
///         S_FALSE if plCoord is out of bounds or last element has been reached.
HRESULT AspSessionSerializer::GetNextCoord(
	LONG* plCoord, ///[in, out] pointer to current Coordinate
	SAFEARRAY* psaArray ///[in] SAFEARAY of which we need to have the next coordinate
	)
{
	HRESULT hr = E_FAIL; // return code
	SAFEARRAYBOUND* psaBounds = psaArray->rgsabound; // Bounds of the safe array
	USHORT usDim = psaArray->cDims; // Dimmension of the safe array
	LONG lMaxIndex = usDim -1;

	if (usDim < 1)
	{ // the dimension of a safe array should be >=1
		Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer GetNextCoord the dimension of a safe array is <1 \r\n");
		hr = E_INVALIDARG;
	}
	else
	{
		for (USHORT i = 0 ; i < usDim ; i++)
		{ // For each dimension
			const long lMaxValue = 0x7FFFFFFF; // max value handled by a signed long
			if ((plCoord[i] < 0) || (psaBounds[lMaxIndex-i].cElements > lMaxValue))
			{ // plCoord should not be negative and number of elements in the safearray should be handle by a signed long
				// in order to be used by Microsoft Array Manaipulation function (ex : SafeArrayGetElement)
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer GetNextCoord plCoord is negative or number of elements in the safearray should are handle by a signed long \r\n");
				hr = E_FAIL;
			}
			else
			{
				if (plCoord[i] < (LONG)psaBounds[lMaxIndex-i].cElements - 1)
				{
					plCoord[i]++;
					hr = S_OK;
					break;
				}
				else
				{
					plCoord[i] = psaBounds[lMaxIndex-i].lLbound;
					hr = S_FALSE;
				}
			}
		}
	}

	if (hr == E_FAIL)
	{ // this should not happen because in all case the above code should set hr
		Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer GetNextCoord  this should not happen because in all case the above code should set hr \r\n");
	}

	return hr;
}

/// Sérialize a SAFEARRAY
/// Attention quand on déclare un tableau Dim Tab1(2) en ASP, ça cré un tableau de 3 élément, 2 étant l'index max
/// @return the result code
HRESULT AspSessionSerializer::SerializeSourceArrayItem(
	CString& key, ///< [in] The key of the SAFEARRAY in the content collection
	SAFEARRAY& safeArray, ///< [in] The SAFEARRAY to serialize
	CString& xml, ///< [in, out] the xml of serialized content collection
	LONG* plTableCoord, ///< [in] the coordinate in the containing safe array in case of a table of table
	USHORT usDim ///< [in] the dimention of the containing safe array
	)
{
	HRESULT hr;
	const USHORT dim = safeArray.cDims; // Dimension of the table to serialize

	// On ouvre la balise Table :on marque dans l'XML que l'on est dans un tabeau
	CString strDim; // Serialized dimention of the table to serialize
	strDim.Format(L"%d", dim);
	xml += L"<Table Key=\"" + key + L"\" TypeName=\"Table\" Dimension=\"" + CString(strDim) + L"\" Bounds=\"";
	
	// Serialize the bounds of the table to serialize into the XML
	for (LONG i = 0 ; i < safeArray.cDims ; i++)
	{ // For each dimension of the safe array
		SAFEARRAYBOUND rgsaBounds = safeArray.rgsabound[i]; // Get the dimensional informations

		// Convert the number of elements into a string
		WCHAR pwszElement[NB_DIGIT_LONG];
		_itow_s(rgsaBounds.cElements, pwszElement, NB_DIGIT_LONG, BASE_TEN);
		CString bound = pwszElement;
		
		// Add the number of elements for this dimension
		if (i < safeArray.cDims - 1)
		{ // If it is not the last dimension add a ',' char
			bound += L",";
		}

		xml += bound;
	}

	xml	+= L"\"";

	hr = AppendCoordAttribute(plTableCoord, xml, usDim);
	if (hr != S_OK)
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeSourceArrayItem AppendCoordAttribute failed \r\n");
		return hr;
	}

	xml += L">";

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Serailize each element in the array

	hr = S_OK;

	// an iterator in the safe array to serialize
	LONG* plCoord = new LONG[dim]; // Coordinate in the sub table
	memset(plCoord, 0, dim * sizeof(LONG)); // first coodinate is (0, 0, ..., 0)

	while (hr != S_FALSE)
	{ // While the last element has not been reached
		variant_t variantValue;
		hr = SafeArrayGetElement(&safeArray, plCoord, &variantValue); // Get the element
		if (hr != S_OK)
		{ // We should get an element. We should not call SafeArrayGetElement if we reach the end of the table
			delete [] plCoord;
			Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeSourceArrayItem We should get an element. We should not call SafeArrayGetElement if we reach the end of the table \r\n");
			return hr;
		}

		if (V_ISARRAY(&variantValue))
		{ // Tableau de tableau
			SAFEARRAY* sourceArray = NULL;
			if (V_ISBYREF(&variantValue))
			{
				sourceArray = *(variantValue.pparray);
			}
			else
			{
				sourceArray = V_ARRAY(&variantValue);
			}

			hr = AspSessionSerializer::SerializeSourceArrayItem(key, *sourceArray, xml, plCoord, dim);
			if (hr != S_OK)
			{
				delete [] plCoord;
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeSourceArrayItem SerializeSourceArrayItem failed \r\n");
				return hr;
			}
		}
		else if (variantValue.vt == VT_DISPATCH)
		{ // Automation object
			AutomationType obj_type = DispatchUtility::GetAutomationType(variantValue.pdispVal);
			switch (obj_type)
			{
			case ScriptingDictionary:
				{
					Scripting::IDictionary* pScriptDico;
					hr = variantValue.pdispVal->QueryInterface(__uuidof(Scripting::IDictionary), (void**)&pScriptDico);
					if (FAILED(hr))
					{
						delete [] plCoord;
						Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeSourceArrayItem SerializeScriptingDictionary failed \r\n");
						return hr;
					}

					hr = SerializeScriptingDictionary(key, *pScriptDico, xml, plCoord, dim);
					if (hr != S_OK)
					{
						pScriptDico->Release();
						delete [] plCoord;
						Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeSourceArrayItem SerializeScriptingDictionary failed \r\n");
						return hr;
					}

					pScriptDico->Release();
				} break;
			case Nothing:
				{
					SerializeNothing(key, xml, plCoord, dim);
				} break;
			case NotAutomation:
			default:
				{
					hr = E_NOTIMPL;
				} break;
			}
		}
		else
		{ // ValueType
			hr = SerializeValueType(key, variantValue, xml, plCoord, dim);
			if (hr != S_OK)
			{
				delete [] plCoord;
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeSourceArrayItem SerializeValueType failed \r\n");
				return hr;
			}
		}

		hr = GetNextCoord(plCoord, &safeArray);
		if (hr != S_OK && hr != S_FALSE)
		{
			delete [] plCoord;
			Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeSourceArrayItem GetNextCoord failed \r\n");
			return hr;
		}
	}

	delete [] plCoord;

	if (hr == S_FALSE)
	{ // At the end of the loop hr == S_FALSE meaning that we reach the of the table => let's return S_OK
		hr = S_OK;
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	// End the Table element
	xml += L"</Table>";
	return hr;
}

/// Seralise number type
/// @return the resutlt code
HRESULT AspSessionSerializer::SerializeValueTypeInternal(
	const VARIANT* pvarSource, ///< [in] Variant source de type nombre (entier ou flotant)
	const CString typeName, ///< [in] type of the VARIANT to add in the xml
	CString& xml, ///< [in, out] the xml of serialized content collection
	LONG* plCoord, ///< [in] In case the serialization occurs within a table this parameter allow to serialize
								 /// the coordinate of an element. If NULL is provided the Coord attribute is not written
	USHORT usDim ///< [in] number of the element in the plCoord table. i.e. the number of dimension of the table
	)
{
	HRESULT hr;

	VARIANT dest;
	VariantInit(&dest);
	hr = VariantChangeType(
		&dest,
		pvarSource,
		0,
		VT_BSTR);
	if (hr != S_OK)
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeValueTypeInternal VariantInit failed \r\n");
		return hr;
	}

	CString strString = dest.bstrVal;
	hr = VariantClear(&dest);
	if (hr != S_OK)
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeValueTypeInternal VariantClear failed \r\n");
		return hr;
	}

	xml += typeName + L"\"";

	hr = AppendCoordAttribute(plCoord, xml, usDim);
	if (FAILED(hr))
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeValueTypeInternal AppendCoordAttribute failed \r\n");
		return hr;
	}

	xml += L">" + strString + + L"</Item>";

	return hr;
}

/// Serialize the provided coordinate
/// @return the result code. S_OK in case of success
HRESULT AspSessionSerializer::SerializeCoordinate(
	LONG* plCoord, ///< [in] the coordiantes to serialize
	USHORT usDim, ///< [in] number of the element in the plCoord table. i.e. the number of dimension of the table
	CString& strCoord ///< [in, out] the result of the serialization
	)
{
	for (LONG i = 0 ; i < usDim ; i++)
	{ // For each dimension
		LONG ulCurrentCoord = plCoord[i];
		errno_t err;
		WCHAR pwszCoord[NB_DIGIT_LONG];
		err = _ltow_s(ulCurrentCoord, pwszCoord, NB_DIGIT_LONG, BASE_TEN);
		if (err != 0)
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeCoordinate Erroer Unexpected \r\n");
			return E_UNEXPECTED;
		}

		strCoord += pwszCoord;

		if (i < usDim -1)
		{
			strCoord += L",";
		}
	}

	return S_OK;
}

/// Append The Coord attribute to the provided XML string
/// @return the result code
HRESULT AspSessionSerializer::AppendCoordAttribute(
	LONG* plCoord, ///< [in] In case the serialization occurs within a table this parameter allows to serialize
								 /// the coordinate of an element. If NULL is provided the Coord attribute is not written
	CString& xml, ///< [in, out] the xml of serialized content collection
	USHORT usDim ///< [in] number of elements in the plCoord table. i.e. the number of dimension of the table
	)
{
	HRESULT hr = S_OK;

	if (plCoord != NULL)
	{
		CString strCoord;
		hr = SerializeCoordinate(plCoord, usDim, strCoord);
		if (hr != S_OK)
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer AppendCoordAttribute SerializeCoordinate failed \r\n");
			return hr;
		}

		xml += L" Coord=\"" + strCoord + L"\"";
	}

	return hr;
}

/// Serialize a value type
/// @return the result code
HRESULT AspSessionSerializer::SerializeValueType(
	CString& key, ///< [in] the key of the element being seriailized
	_variant_t& value, ///< [in] the value of the element being serialized
	CString& xml, ///< [in, out] the xml of serialized content collection
	LONG* plCoord, ///< [in] In case the serialization occurs within a table this parameter allow to serialize
								 /// the coordinate of an element. If NULL is provided the Coord attribute is not written
	USHORT usDim ///< [in] number of elements in the plCoord table. i.e. the number of dimension of the table
	)
{
	HRESULT hr;
	xml += L"<Item Key=\"" + key + L"\" TypeName=\"";
	CString typeName;

	switch (value.vt)
	{
	case VT_EMPTY:
		{ // Empty
			xml += L"Empty\"";
			hr = AppendCoordAttribute(plCoord, xml, usDim);
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeValueType VT_EMPTY AppendCoordAttribute failed \r\n");
				return hr;
			}

			xml+= L" />";
		}break;
	case VT_DATE:
		{ // Date
			COleDateTime cDate = value.date;
			xml += L"Date\"";
			hr = AppendCoordAttribute(plCoord, xml, usDim);
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeValueType VT_DATE AppendCoordAttribute failed \r\n");
				return hr;
			}

			CString sDate = cDate.Format(L"%Y-%m-%d %H:%M:%S");
			xml += L">" + sDate + L"</Item>";
		} break;
	case VT_I1:
		{ // One byte signed integer
			typeName = L"SingedByte";
			hr = SerializeValueTypeInternal(&value, typeName, xml, plCoord, usDim);
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeValueType VT_I1 SerializeValueTypeInternal failed \r\n");
				return hr;
			}
		} break;
	case VT_I2:
		{ // Two bytes signed integer
			typeName = L"SingedShort";
			hr = SerializeValueTypeInternal(&value, typeName, xml, plCoord, usDim);
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeValueType VT_I2 SerializeValueTypeInternal failed \r\n");
				return hr;
			}
		} break;
	case VT_I4:
		{ // Four byte signed integer
			typeName = L"SingedInteger";
			hr = SerializeValueTypeInternal(&value, typeName, xml, plCoord, usDim);
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeValueType VT_I4 SerializeValueTypeInternal failed \r\n");
				return hr;
			}
		} break;
	case VT_UI1:
		{ // One byte unsigned integer
			typeName = L"UnsingedByte";
			hr = SerializeValueTypeInternal(&value, typeName, xml, plCoord, usDim);
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeValueType VT_UI1 SerializeValueTypeInternal failed \r\n");
				return hr;
			}
		} break;
	case VT_UI2:
		{ // Two byte unsigned integer
			typeName = L"UnsingedShort";
			hr = SerializeValueTypeInternal(&value, typeName, xml, plCoord, usDim);
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeValueType VT_UI2 SerializeValueTypeInternal failed \r\n");
				return hr;
			}
		} break;
	case VT_UI4:
		{ // Four byte unsigned integer
			typeName = L"UnsingedInteger";
			hr = SerializeValueTypeInternal(&value, typeName, xml, plCoord, usDim);
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeValueType VT_UI4 SerializeValueTypeInternal failed \r\n");
				return hr;
			}
		} break;
	case VT_INT:
		{ // Signed machine int
			typeName = L"SingedMachineInteger";
			hr = SerializeValueTypeInternal(&value, typeName, xml, plCoord, usDim);
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeValueType VT_INT SerializeValueTypeInternal failed \r\n");
				return hr;
			}
		} break;
	case VT_UINT:
		{ // Unsigned machine int
			typeName = L"UnsingedMachineInteger";
			hr = SerializeValueTypeInternal(&value, typeName, xml, plCoord, usDim);
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeValueType VT_UINT SerializeValueTypeInternal failed \r\n");
				return hr;
			}
		} break;
	case VT_DECIMAL:
		{ // 16 byte fixed point
			typeName = L"Decimal";
			hr = SerializeValueTypeInternal(&value, typeName, xml, plCoord, usDim);
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeValueType VT_DECIMAL SerializeValueTypeInternal failed \r\n");
				return hr;
			}
		} break;
	case VT_R4:
		{ // 4 byte real
			typeName = L"Real4";
			hr = SerializeValueTypeInternal(&value, typeName, xml, plCoord, usDim);
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeValueType VT_R4 SerializeValueTypeInternal failed \r\n");
				return hr;
			}
		} break;
	case VT_R8:
		{ // 8 byte real
			typeName = L"Real8";
			hr = SerializeValueTypeInternal(&value, typeName, xml, plCoord, usDim);
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeValueType VT_R8 SerializeValueTypeInternal failed \r\n");
				return hr;
			}
		} break;
	case VT_BOOL:
		{ // True=-1, False=0
			typeName = L"Bool";
			hr = SerializeValueTypeInternal(&value, typeName, xml, plCoord, usDim);
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeValueType VT_BOOL SerializeValueTypeInternal failed \r\n");
				return hr;
			}
		} break;
	case VT_BSTR:
		{ // String
			CString strString = value.bstrVal;
			xml += L"String\"";
			hr = AppendCoordAttribute(plCoord, xml, usDim);
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeValueType VT_BSTR AppendCoordAttribute failed \r\n");
				return hr;
			}
			xml += L"><![CDATA[" + strString + + L"]]></Item>";
		} break;
	case VT_CY:
		{ // Currency
			typeName = L"Currency";
			hr = SerializeValueTypeInternal(&value, typeName, xml, plCoord, usDim);
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeValueType VT_CY SerializeValueTypeInternal failed \r\n");
				return hr;
			}
		} break;
	case VT_NULL:
		{ // Null
			typeName = L"Null\"";
			xml += typeName;
			hr = AppendCoordAttribute(plCoord, xml, usDim);
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeValueType VT_NULL AppendCoordAttribute failed \r\n");
				return hr;
			}

			xml += L"></Item>";

		} break;
	default:
		{
			hr = E_UNEXPECTED;
			Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeValueType default Unexpected error \r\n");
			return hr;
		} break;
	}

	return hr;
}

/// Generate the Item node corresponding to the key into the xml
HRESULT AspSessionSerializer::GenerateXmlItemNode(
	ContainerType::iterator& dicoIterator, ///< [in] a reference to the iterator to serialize
	CString& xml ///< [in, out] the xml to append
	)
{
	HRESULT hr;

	CString key = dicoIterator->first;
	int var_type = dicoIterator->second.vt;

	if (V_ISARRAY(&dicoIterator->second))
	{ // Array of Variant (ex : tableau ASP stocké en Session)
		SAFEARRAY* sourceArray = NULL;
		if (V_ISBYREF(&dicoIterator->second))
		{
			sourceArray = *(dicoIterator->second.pparray);
		}
		else
		{
			sourceArray = V_ARRAY(&dicoIterator->second);
		}

		// Attention quand on déclare un tableau Dim Tab1(2) en ASP, ça cré un tableau de 3 élément, 2 étant l'index max
		hr = SerializeSourceArrayItem(key, *sourceArray, xml, NULL, 0);
		if (hr != S_OK)
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer GenerateXmlItemNode SerializeSourceArrayItem failed \r\n");
			return hr;
		}
	}
	else if (var_type == VT_DISPATCH)
	{ // COM component
		IDispatch* pDisp = dicoIterator->second.pdispVal;
		
		if (pDisp == NULL)
		{
			hr = SerializeNothing(key, xml, NULL, 0);
			if (hr != S_OK)
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer GenerateXmlItemNode SerializeNothing failed \r\n");
				return hr;
			}
		}
		else
		{ // Scripting Dictionary
			Scripting::IDictionary* pScriptDico;

			hr = pDisp->QueryInterface(__uuidof(Scripting::IDictionary), (void**)&pScriptDico);

			if (FAILED(hr))
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer GenerateXmlItemNode QueryInterface failed \r\n");
				return hr;
			}

			hr = SerializeScriptingDictionary(key, *pScriptDico, xml, NULL, 0);
			if (FAILED(hr))
			{
				pScriptDico->Release();
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer GenerateXmlItemNode SerializeScriptingDictionary failed \r\n");
				return hr;
			}

			 pScriptDico->Release();
		}
	}
	else
	{ // Variant (string, Double, integer, etc...)
		_variant_t value = dicoIterator->second;
		hr = SerializeValueType(key, value, xml, NULL, 0);
		if (FAILED(hr))
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer GenerateXmlItemNode SerializeValueType failed \r\n");
			return hr;
		}
	}

	return hr;
}

/// Sérialize a Scripting.Dictionary
/// @return the result code
HRESULT AspSessionSerializer::SerializeScriptingDictionary(
	CString& key, ///< [in] The key of the Scripting.Dictionary in the content collection
	Scripting::IDictionary& scriptingDictionary, ///< [in] The SAFEARRAY to serialize
	CString& xml, ///< [in, out] the xml of serialized content collection
	LONG* plCoord, ///< [in] In case the serialization occurs within a table this parameter allow to serialize
								 /// the coordinate of an element. If NULL is provided the Coord attribute is not written
	USHORT usDim ///< [in] number of elements in the plCoord table. i.e. the number of dimension of the table
	)
{
	HRESULT hr;

	xml += L"<Dictionary Key=\"" + key + L"\" TypeName=\"Dictionary\"";

	hr = AppendCoordAttribute(plCoord, xml, usDim);
	if (hr != S_OK)
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeScriptingDictionary AppendCoordAttribute failed \r\n");
		return hr;
	}

	xml	+= L">";

	IEnumVARIANTPtr spEnum = scriptingDictionary._NewEnum();
	const ULONG nBatchSize = 1; // nBatchSize is the number of items that we request in each call to IEnumVARIANT::Next.
	ULONG nReturned = 0; // nReturned will store the number of items returned by a call to IEnumVARIANT::Next
	
	// arrVariant is the array used to hold the returned keys. No VariantInit here else spEnum->Next failed
	VARIANT arrVariant[nBatchSize];
	VariantInit(&arrVariant[0]);

	int i = 0;
	do
	{ // For each element in the Scripting.Dictionary
		i++;
		hr = spEnum->Next(nBatchSize, &arrVariant[0], &nReturned);
		if (FAILED(hr))
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeScriptingDictionary Next failed \r\n");
			return hr;
		}

		if (hr == S_OK)
		{
			_variant_t variantValue = scriptingDictionary.GetItem(&arrVariant[0]);
			CString item_key = arrVariant[0].bstrVal;
			
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Serialize depending on the variantValue

			if (V_ISARRAY(&variantValue))
			{ // Tableau
				SAFEARRAY* sourceArray = NULL;
				if (V_ISBYREF(&variantValue))
				{
					sourceArray = *(variantValue.pparray);
				}
				else
				{
					sourceArray = V_ARRAY(&variantValue);
				}

				hr = AspSessionSerializer::SerializeSourceArrayItem(item_key, *sourceArray, xml, NULL, 0);
				if (hr != S_OK)
				{
					Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeScriptingDictionary SerializeSourceArrayItem failed \r\n");
					return hr;
				}
			}
			else if (variantValue.vt == VT_DISPATCH)
			{ // Automation object
				AutomationType obj_type = DispatchUtility::GetAutomationType(variantValue.pdispVal);
				switch (obj_type)
				{
				case ScriptingDictionary:
					{
						Scripting::IDictionary* pScriptDico;
						hr = variantValue.pdispVal->QueryInterface(__uuidof(Scripting::IDictionary), (void**)&pScriptDico);
						if (FAILED(hr))
						{
							Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeScriptingDictionary SerializeScriptingDictionary failed \r\n");
							return hr;
						}

						hr = SerializeScriptingDictionary(item_key, *pScriptDico, xml, NULL, 0);
						if (hr != S_OK)
						{
							pScriptDico->Release();
							Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeScriptingDictionary SerializeScriptingDictionary failed \r\n");
							return hr;
						}

						pScriptDico->Release();
					} break;
				case Nothing:
				{
					SerializeNothing(key, xml, NULL, 0);
				} break;
				case NotAutomation:
				default:
					{
						hr = E_NOTIMPL;
					} break;
				}
			}
			else
			{ // ValueType
				hr = SerializeValueType(item_key, variantValue, xml, NULL, 0);
				if (hr != S_OK)
				{
					Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeSourceArrayItem SerializeValueType failed \r\n");
					return hr;
				}
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			hr = VariantClear(&arrVariant[0]);
			if (FAILED(hr))
			{
				Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeScriptingDictionary VariantClear failed \r\n");
				return hr;
			}
		}
	} while (hr != S_FALSE); // S_FALSE indicates end of collection

	xml += L"</Dictionary>";

	if (hr == S_FALSE)
	{ // the previous do while loop should end with S_FALSE
		hr = S_OK; // SerializeScriptingDictionary will finish with an S_OK
	}

	return hr;
}

/// Sérialize Nothing
HRESULT AspSessionSerializer::SerializeNothing(
	CString& key, ///< [in] The key of the Scripting.Dictionary in the content collection
	CString& xml, ///< [in, out] the xml of serialized content collection
	LONG* plCoord, ///< [in] In case the serialization occurs within a table this parameter allow to serialize
							 /// the coordinate of an element. If NULL is provided the Coord attribute is not written
	USHORT usDim ///< [in] number of elements in the plCoord table. i.e. the number of dimension of the table
	)
{
	HRESULT hr = S_OK;

	xml += L"<Dictionary Key=\"" + key + L"\" TypeName=\"Nothing\"";

	hr = AppendCoordAttribute(plCoord, xml, usDim);
	if (hr != S_OK)
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"Error AspSessionSerializer SerializeScriptingDictionary AppendCoordAttribute failed \r\n");
		return hr;
	}

	xml	+= L" />";

	return hr;
}
