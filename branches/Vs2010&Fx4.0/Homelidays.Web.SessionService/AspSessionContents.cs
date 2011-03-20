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

using System;
using System.Collections.Generic;

namespace Homelidays.Web.SessionService
{
    /// <summary>
    /// The session state collection
    /// </summary>
    public class AspSessionContents : Dictionary<string, object>, IAspSessionContents
    {
        /// <summary>
        /// Session Timeout
        /// </summary>
        private int timeOut;

        /// <summary>
        /// Initializes a new instance of the AspSessionContents class.
        /// This constructor is defined explicitly because we want to be sure to have it
        /// even if in the future we add other constructors
        /// </summary>
        public AspSessionContents()
            : base()
        {
        }

        /// <summary>
        /// Gets or sets the session timeout
        /// </summary>
        public int TimeOut
        {
            get
            {
                if (this.timeOut == 0)
                {
                    this.timeOut = AppSettingsManager.SessionTimeout;
                }

                return this.timeOut;
            }

            set
            {
                this.timeOut = value;
            }
        }

        /// <summary>
        /// Indexer of the Session
        /// </summary>
        /// <param name="key">key of the wanted session variable</param>
        /// <returns>the session variable value</returns>
        public new object this[string key]
        {
            get
            {
                if (key == null)
                {
                    return null;
                }

                if (this.ContainsKey(key) || this.ContainsKey(key.ToLower()))
                {
                    return base[key.ToLower()];
                }
                else
                {
                    return null;
                }
            }

            set
            {
                base[key.ToLower()] = value;
            }
        }

        /// <summary>
        /// Add the specified key and value to the session
        /// </summary>
        /// <param name="key">The key of the element to add</param>
        /// <param name="value">The value of the element to add. The value can be null for reference type</param>
        public new void Add(string key, object value)
        {
            base.Add(key.ToLower(), value);
        }
    }
}
