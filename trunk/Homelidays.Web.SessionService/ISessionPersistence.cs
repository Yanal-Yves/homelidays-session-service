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

namespace Homelidays.Web.SessionService
{
    /// <summary>
    /// This is the session state service data access layer interface
    /// </summary>
    public interface ISessionPersistence
    {
        /// <summary>
        /// Load the session from the storage
        /// </summary>
        /// <param name="sessionId">Session state id</param>
        /// <returns>The session state corresponding to the provided session id</returns>
        SessionState LoadSession(string sessionId);

        /// <summary>
        /// Save the session to the storage
        /// </summary>
        /// <param name="sessionId">Session state id</param>
        /// <param name="session">Session state to save</param>
        /// <param name="sessionTimeOut">Timeout of the session</param>
        void SaveSession(string sessionId, SessionState session, int sessionTimeOut);

        /// <summary>
        /// Generate a new session id
        /// </summary>
        /// <returns>the generated session id</returns>
        string GenerateSessionId();
    }
}
