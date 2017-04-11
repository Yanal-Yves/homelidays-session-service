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
#include <ATLComTime.h> // for COleDateTime 
#include "ContainerType.h" // for ContainerType
#include "scrrun.tlh"

/// Number of digit that a long can use + 1 for the ending \0
#define NB_DIGIT_LONG 11

#define BASE_TEN 10

/// AspSessionSerializer allows to serialize the Session.Content collection
class AspSessionSerializer
{
public:
	AspSessionSerializer(void);
	~AspSessionSerializer(void);

	/// Serialize a collection of type ContainerType to XML
	HRESULT SerializeSession(
		ContainerType& dico, ///< [in] A reference to the dico to serialize
		CString& serializeDico ///< [out] the resulting serialize dico
		);

private:
	/// Seralise number type
	/// @return the resutlt code
	HRESULT SerializeValueTypeInternal(
		const VARIANT* pvarSource, ///< [in] Variant source de type nombre (entier ou flotant)
		const CString typeName, ///< [in] type of the VARIANT to add in the xml
		CString& xml, ///< [in, out] the xml of serialized content collection
		LONG* plCoord, ///< [in] In case the serialization occurs within a table this parameter allow to serialize
									 /// the coordinate of an element. If NULL is provided the Coord attribute is not written
		USHORT usDim ///< [in] number of the element in the plCoord table. i.e. the number of dimension of the table
		);

	/// Serialize the provided coordinate
	/// @return the result code. S_OK in case of success
	HRESULT SerializeCoordinate(
		LONG* plCoord, ///< [in] the coordiantes to serialize
		USHORT usDim, ///< [in] number of the element in the plCoord table. i.e. the number of dimension of the table
		CString& strCoord ///<  [in, out] the result of the serialization
		);

	/// Append The Coord attribute to the provided XML string
	/// @return the result code
	HRESULT AppendCoordAttribute(
		LONG* plCoord, ///< [in] In case the serialization occurs within a table this parameter allows to serialize
									 /// the coordinate of an element. If NULL is provided the Coord attribute is not written
		CString& xml, ///< [in, out] the xml of serialized content collection 
		USHORT usDim ///< [in] number of elements in the plCoord table. i.e. the number of dimension of the table
		);

	/// Serialize a value type
	/// @return the result code
	HRESULT SerializeValueType(
		CString& key, ///< [in] the key of the element being seriailized
		_variant_t& value, ///< [in] the value of the element being serialized
		CString& xml, ///< [in, out] the xml of serialized content collection 
		LONG* plCoord, ///< [in] In case the serialization occurs within a table this parameter allow to serialize
									 /// the coordinate of an element. If NULL is provided the Coord attribute is not written
		USHORT usDim ///< [in] number of elements in the plCoord table. i.e. the number of dimension of the table
	);

	/// Sérialize a SAFEARRAY
	/// @return the result code
	HRESULT SerializeSourceArrayItem(
		CString& key, ///< [in] The key of the SAFEARRAY in the content collection
		SAFEARRAY& safeArray, ///< [in] The SAFEARRAY to serialize
		CString& xml, ///< [in, out] the xml of serialized content collection
		LONG* plTableCoord, ///< [in] the coordinate in the containing safe array in case of a table of table
		USHORT usDim ///< [in] the dimention of the containing safe array
		);

	HRESULT GenerateXmlItemNode(ContainerType::iterator& dicoIterator, CString& xml);

	/// Get the nex Coordinate
	/// @return S_OK if plCoord point to the next element.
	///         S_FALSE if plCoord is out of bounds or last element has been reached.
	HRESULT GetNextCoord(
		LONG* plCoord, ///[in, out] pointer to current Coordinate 
		SAFEARRAY* psaArray ///[in] SAFEARAY of which we need to have the next coordinate
		);

	/// Sérialize a Scripting.Dictionary
	/// @return the result code
	HRESULT SerializeScriptingDictionary(
		CString& key, ///< [in] The key of the Scripting.Dictionary in the content collection
		Scripting::IDictionary& scriptingDictionary, ///< [in] The Scripting.Dictionary to serialize
		CString& xml, ///< [in, out] the xml of serialized content collection
		LONG* plCoord, ///< [in] In case the serialization occurs within a table this parameter allow to serialize
								 /// the coordinate of an element. If NULL is provided the Coord attribute is not written
		USHORT usDim ///< [in] number of elements in the plCoord table. i.e. the number of dimension of the table
		);

	/// Sérialize Nothing
	HRESULT SerializeNothing(
		CString& key, ///< [in] The key of the Scripting.Dictionary in the content collection
		CString& xml, ///< [in, out] the xml of serialized content collection
		LONG* plCoord, ///< [in] In case the serialization occurs within a table this parameter allow to serialize
								 /// the coordinate of an element. If NULL is provided the Coord attribute is not written
		USHORT usDim ///< [in] number of elements in the plCoord table. i.e. the number of dimension of the table
		);
};
