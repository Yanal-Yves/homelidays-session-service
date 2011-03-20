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
#include <iostream>
#include <fstream>
#include <string>
#include <atlstr.h>
#include "stdafx.h"

/// Utility Class with helper methods for file use
class FileUtility
{
public:
	/// Retrieves the fully-qualified path for the directory where the AspSessionService.dll file is stored.
	static HRESULT GetModuleDirectory(
		CString& cstrModulePath ///< [out] The folder path of AspSessionService.dll
		);
};