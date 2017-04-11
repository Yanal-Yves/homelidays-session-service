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
#include "scrrun.tlh"

/// Automation Type handle by Session Servce
enum AutomationType
{
	NotAutomation = 0, ///< Not an automation object 
	ScriptingDictionary = 1, ///< A Scripting.Dictionary object
	Nothing = 2 ///< Nothing vb type
};

/// Helper class to handle automation object
class DispatchUtility
{
public:
	/// Get the Automation type of the IDispatch object provided
	/// @return The Automation type of the disp object
	static AutomationType GetAutomationType(
		IDispatch* pDisp // The Automation component to test
		);
};
