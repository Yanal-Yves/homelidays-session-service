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
#include "Partition.h"
#ifdef _M_X64
#import "C:\Program files\Common Files\System\Ado\msado15.dll" rename("EOF", "ADOEOF")
#else 
#import "C:\Program Files (x86)\Common Files\System\Ado\msado15.dll" rename("EOF", "ADOEOF")
#endif

/// AspSessionPersistence allows to save and retrieve the serialize Session.Content collection
class AspSessionPersistence
{
private:
	static const int GuidNumberOfDigits = 36;

	/// Log Provider Errors from Connection object.
	static void LogProviderError(
		_com_error &e, ///< [in] Current exception
		ADODB::_ConnectionPtr pConnection, ///< [in] Current connection objects
		_bstr_t bstrCommandText ///< [in] The SQL command text that failled
		);

public:
	/// Get the session from the storage
	/// @return S_OK in case of success, any other value if operation failled
	HRESULT GetSession(
		_variant_t Guid, ///< [in] Session Id (GUID as a string)
		_variant_t &LastAccessed, ///< [out] Last accessed date
		_variant_t &SessionTimeOut, ///< [out] Session Time out
		_variant_t &Data ///< [out] Session data
		);

	/// Set the session into the storage
	/// @return S_OK in case of success, any other value if operation failled
	HRESULT SetSession(
		_variant_t Guid, ///< [in] Session Id (GUID as a string)
		_variant_t SessionTimeOut, ///< [in] Session Time out
		_variant_t Data ///< [in] Session data
		);

	/// Delete the session into the storage, Happens where session.abandon is called
	HRESULT DeleteSession(
		_variant_t Guid ///< [in] Session Id (GUID as a string)
		);

	/// Refresh the session into the storage
	/// @return S_OK in case of success, any other value if operation failled
	HRESULT RefreshSession(
		_variant_t vtGuid ///< [in] Session Id (GUID as a string) of the session to refresh
		);

	/// Create tables dynamically in the database if they do not exist
	static HRESULT CreateTables(
		Partition _partition ///< [in] Partition (SQL Connexion string) where to create the tables
		);
};
