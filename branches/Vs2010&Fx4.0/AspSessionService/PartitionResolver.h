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
#include "stdafx.h"
#include "Partition.h"

/// Singleton class that allows to find the partition (SQL Server connection string) used for a SessionId
class PartitionResolver
{
private:
	/// Critical section used to initialize the database
	static CRITICAL_SECTION CriticalSection;

	/// Max number of partition handled by the session service
	static const USHORT NbMaxSessionServer=64;

	/// Table of partitions. In this first version only the first partition is used
	static Partition lpPartitions[PartitionResolver::NbMaxSessionServer];

	/// PartitionResolver is a Singleton lpSingleton is the static reference that allow access to the singleton object
	static PartitionResolver* lpSingleton;

	///Charge la configuration du fichier
	HRESULT LoadConf();

public:
	/// Get the current instance of the PartitionResolver
	/// As PartitionResolver is a singleton, the constructor should be called once first
	/// @return S_OK in case of success, E_FAIL if the constructor has not been called once
	static HRESULT GetSingleton(PartitionResolver** ppSingleton);

	/// Get the partition according to the provided SessionId
	/// @return S_OK in case of succes, E_FAIL if no more persistance is available
	static HRESULT GetPartitionn(
		_variant_t vtGuid, ///< [in] The session Id
		Partition& partition /// [in, out] The partition to use to store the session corresponding to the session id
		);

	/// Reset the current applied configuration and reload configuration from the xml file
	static HRESULT ResetConf();

	/// Initializez a new instance of the PartitionResolver singleton class.
	PartitionResolver(void);

	/// Destructor of the PartitionResolver class.
	~PartitionResolver(void);
};
