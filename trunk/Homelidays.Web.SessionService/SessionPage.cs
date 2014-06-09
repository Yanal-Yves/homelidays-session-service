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

namespace Homelidays.Web.SessionService
{
    /// <summary>
    /// An aspx page that enables the Yacht scalable session state store
    /// </summary>
    public class SessionPage : System.Web.UI.Page
    {
        /// <summary>
        /// The ASP Session object
        /// </summary>
        private AspSession aspSession;

        /// <summary>
        /// Initializes a new instance of the SessionPage class.
        /// </summary>
        public SessionPage()
        {
            this.Unload += new EventHandler(this.SessionPageUnload);
        }

        /// <summary>
        /// Gets the ASP session object
        /// </summary>
        public AspSession AspSession
        {
            get
            {
                if (this.aspSession == null)
                {
                    this.aspSession = new AspSession(this.Context);
                }

                return this.aspSession;
            }
        }

        /// <summary>
        /// a l'unload de la page on fait  la persistence en base
        /// </summary>
        /// <param name="sender">objet sender de l'event</param>
        /// <param name="e">Event args</param>
        public void SessionPageUnload(object sender, EventArgs e)
        {
            this.AspSession.PersistSession();
        }
    }
}
