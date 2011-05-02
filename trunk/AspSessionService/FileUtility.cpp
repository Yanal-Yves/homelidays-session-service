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
#include "StdAfx.h"
#include "FileUtility.h"
#include "Logger.h"

/// Retrieves the fully-qualified path for the directory where the AspSessionService.dll file is stored.
HRESULT FileUtility::GetModuleDirectory(
	CString& cstrModulePath ///< [out] The folder path of AspSessionService.dll
	)
{
#ifdef __AspSessionServiceTestConsolApplication
	cstrModulePath = L"";
	return S_OK;
#else
	HRESULT hr=S_OK;
	WCHAR wcPath[_MAX_PATH];
	// Get the HMODULE of AspSessionService.dll
	HMODULE hLibrary = GetModuleHandle(L"AspSessionService.dll");
	if (hLibrary == NULL)
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError  FileUtility::GetModuleDirectory GetModuleHandle failed (dll not loaded ?) \r\n");
		return E_FAIL;
	}

	// Get the fully-qualified path of AspSessionService.dll
	DWORD dw = GetModuleFileName(hLibrary, wcPath, _MAX_PATH);
	if (dw == 0)
	{
		Logging::Logger::GetCurrent()->WriteInfo(L"\tError  FileUtility::GetModuleDirectory GetModuleFileName failed \r\n");
		return E_FAIL;
	}

	// Remove the AspSessionService.dll word to get the containing folder
	cstrModulePath = wcPath;
	cstrModulePath.Replace(L"AspSessionService.dll", L"");
	return hr;
#endif
}
