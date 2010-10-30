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
#include "DispatchUtility.h"
#include "Logger.h"

/// Get the Automation type of the IDispatch object provided
/// @return The Automation type of the disp object
AutomationType DispatchUtility::GetAutomationType(
	IDispatch* pDisp // The Automation component to test
	)
{
	HRESULT hr;
	AutomationType type;
	
	if (pDisp == NULL)
	{
		type = Nothing;
	}
	else
	{
		// As The session service handle only the Scripting.Dictionary => minimum of work done here
		Scripting::IDictionary* pScriptDico;
		hr = pDisp->QueryInterface(__uuidof(Scripting::IDictionary), (void**)&pScriptDico);
		if (hr == S_OK)
		{
			type = ScriptingDictionary;
		}
		else
		{
			Logging::Logger::GetCurrent()->WriteInfo(L"Error DispatchUtility GetAutomationType failed \r\n");
			type = NotAutomation;
		}

		pScriptDico->Release();
	}
	
	return type;
}
