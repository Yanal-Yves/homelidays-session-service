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

// dllmain.cpp : Implementation of DllMain.

#include "stdafx.h"
#include "resource.h"
#include "AspSessionService_i.h"
#include "dllmain.h"
#include "PartitionResolver.h"

CAspSessionServiceModule _AtlModule;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	hInstance;

	if (dwReason == DLL_PROCESS_ATTACH)
	{ // Instanciate the PartitionResolver singleton
		new PartitionResolver();
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		PartitionResolver* pPartitionResolver;
		PartitionResolver::GetSingleton(&pPartitionResolver);
		delete pPartitionResolver;
	}

	return _AtlModule.DllMain(dwReason, lpReserved);
}
