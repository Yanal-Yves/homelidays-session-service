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
#include "XmlUtility.h"
#include "Logger.h"

/// Get a stream and a reader to the serialized content collection
HRESULT XmlUtility::GetReader(
	_variant_t& serializeDico, ///< [in] An XML (BSTR) serialized content collection.
	CComPtr<IXmlReader>& pReader, ///< [out] the reader to the serialized dico
	CComPtr<IStream>& stream ///< [out] the stream used by the reader
	)
{
	HRESULT hr;

	// Open a memory stream to the serialized dico
	// &(*stream) in order to use the copy constructor that allow us to convert from CComPtr<IStream> to LPSTREAM
	hr = CreateStreamOnHGlobal(0, TRUE, &stream);
	if (hr != S_OK)
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError XmlUtility::GetReader CreateStreamOnHGlobal failed\r\n");
		return hr;
	}

	UINT length;
	ULONG nbWrittenByte;
	length = ::SysStringByteLen(serializeDico.bstrVal);
	length += sizeof(OLECHAR); // Add the terminating null character length that is not included by SysStringByteLen
	stream->Write(serializeDico.bstrVal, length, &nbWrittenByte);
	// Set the current position of the stream at the begining
	LARGE_INTEGER dlibMove; // offset from the begining of the stream
	dlibMove.QuadPart = 0; // no offset
	hr = stream->Seek(dlibMove, STREAM_SEEK_SET, NULL);
	if (hr != S_OK)
	{ // Libération des ressources
		stream.Release();
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError XmlUtility::GetReader stream->Seek failed\r\n");
		return hr;
	}

	hr = CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, NULL);
	if (hr != S_OK)
	{ // Libération des ressources
		stream.Release();
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError XmlUtility::GetReader CreateXmlReader failed\r\n");
		return hr;
	}

	hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit);
	if (hr != S_OK)
	{ // Libération des ressources
		pReader.Release();
		stream.Release();
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError XmlUtility::GetReader pReader->SetProperty failed\r\n");
		return hr;
	}

	hr = pReader->SetInput(stream);
	if (hr != S_OK)
	{ // Libération des ressources
		pReader.Release();
		stream.Release();
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError XmlUtility::GetReader pReader->SetInput failed\r\n");
		return hr;
	}

	return hr;
}