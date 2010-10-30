﻿/* Copyright (c) 2010 HomeAway, Inc.
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

using System;
using System.Collections;
using System.Reflection;

namespace Homelidays.Web.SessionService
{
    /// <summary>
    /// The session state collection interface
    /// </summary>
    public interface ISessionState
    {
        /// <summary>
        /// Gets or sets the timeout of the Session
        /// </summary>
        int TimeOut
        {
            get;
            set;
        }
    }
}
