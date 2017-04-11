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

/// The Partition class store all the information needed to access a partition
class Partition
{
public:
	/// The partition string of this partition that allow the partition service to access the partition
	_bstr_t bstrPartition;

	/// The partition login
	_bstr_t bstrLogin;

	/// The partition password
	_bstr_t bstrPassword;

	/// Indicate whether the partition is active or not. Changing this value allows to dynamically add or remove a
	/// partition server in the service
	bool bIsActive;

	/// Indicate whether the partition is initialized (ie. the tables are created).
	bool bIsInitialized;
	
	/// Initialize a new instance of the Partition class.
	Partition(void);

	/// Destructor of the partition class
	~Partition(void);
};
