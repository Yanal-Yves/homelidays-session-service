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

// dllmain.h : Declaration of module class.

/// Atl module class of the Session service.
/// It provides code for the standard exports for a dynamic-link library (DLL) and provides functions used by all DLL projects.
/// This specialization of CAtlModuleT class includes support for registration.
class CAspSessionServiceModule : public CAtlDllModuleT< CAspSessionServiceModule >
{
public :
	DECLARE_LIBID(LIBID_AspSessionServiceLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_ASPSESSIONSERVICE, "{42D49BCB-8A96-4064-838B-F0A089A1E704}")
};

extern class CAspSessionServiceModule _AtlModule;
