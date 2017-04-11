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
#include "StringUtility.h"

StringUtility::StringUtility(void)
{
}

StringUtility::~StringUtility(void)
{
}

/// Convert a guid into a bstr without dashes
/// @return the converted GUID as a _bstr_t without dashes
void StringUtility::GuidToBstrWithoutDashes(
	GUID& guid, ///< [in] The GUID to convert to bstr
	_bstr_t& bstrGuid ///< [out] the GUID as a _bstr_t without dashes
)
{
	TCHAR tchar_guid[GUID_NUMBER_DIGIT + 1]; // number of number digit + 1 for the ending char

	// Put the GUID into a String
	wsprintf(tchar_guid,
		_T("%8.X%4.X%4.X%2.X%2.X%2.X%2.X%2.X%2.X%2.X%2.X"),
		guid.Data1,
		guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);

	// If there are leading 0 in the guid the wsprintf will add space (ex: 035 will be _35 where _ is a space)
	for (int i=0 ; i < GUID_NUMBER_DIGIT ; i++)
	{ // replace spaces by 0
		if (tchar_guid[i] == ASCII_SPACE_CHAR) // space
		{
			tchar_guid[i] = ASCII_0_CHAR; // 0
		}
	}

	bstrGuid = tchar_guid;
}