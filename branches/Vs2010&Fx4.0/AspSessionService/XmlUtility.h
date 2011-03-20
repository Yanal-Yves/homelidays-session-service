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
#include <xmllite.h>
#include <atlsafe.h>

/// Utility Class with helper methods for Xml use 
class XmlUtility
{
public:
	/// Get a stream and a reader to the serialized content collection
	static HRESULT XmlUtility::GetReader(
		_variant_t& serializeDico, ///< [in] An XML (BSTR) serialized content collection.
		CComPtr<IXmlReader>& pReader, ///< [out] the reader to the serialized dico
		CComPtr<IStream>& stream ///< [out] the stream used by the reader
		);
};