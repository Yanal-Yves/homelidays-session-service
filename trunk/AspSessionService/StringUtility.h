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

#include "comutil.h"

#define GUID_NUMBER_DIGIT 32 // number of number digit in a guid
#define ASCII_SPACE_CHAR 0x20
#define ASCII_0_CHAR 0x30

class StringUtility
{
public:
	StringUtility(void);
	~StringUtility(void);

	/// Convert a guid into a bstr without dashes
	/// @return the converted GUID as a _bstr_t without dashes
	static void GuidToBstrWithoutDashes(
		GUID& guid, ///< [in] The GUID to convert to bstr
		_bstr_t& bstrGuid ///< [out] the GUID as a _bstr_t without dashes
	);
};
