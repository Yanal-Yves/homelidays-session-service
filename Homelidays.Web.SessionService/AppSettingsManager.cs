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
    /// Helper class that get data from appSettings and default values
    /// </summary>
    public static class AppSettingsManager
    {
        /// <summary>
        /// Gets the session expiration timeout
        /// </summary>
        public static int SessionTimeout
        {
            get
            {
                // Idéalement, il faudrait mettre cette valeur en AppSetting. 
                // Mais il faudrait aussi le faire en C++ ce qui est plus dure à faire qu'en .net.
                // => On laisse la valeur en dure dans le code pour l'instant
                return 20; // ATTENTION : Modifier ceci nécessite de changer l'équivalent dans le code C++
            }
        }

        /// <summary>
        /// Gets the cookie name that store the session id
        /// </summary>
        public static string CookieName
        {
            get
            {
                // Attention : changer le nom du cookie ici oblige à le changer dans AspSessionService
                // Le composant COM C++ n'ayant pas conscience d'appSetting on ne rend volontairement pas 
                // le nom du cookie paramétrable via appSettings
                return "Yacht";
            }
        }

        /// <summary>
        /// Gets the key of the cookie that hold the SessionId
        /// </summary>
        public static string CookieKey
        {
            get
            {
                return "SessionId";
            }
        }

        /// <summary>
        /// Gets the key of the cookie that hold the LastAccessed date of the Session
        /// </summary>
        public static string CookieKeyLastAccessed
        {
            get
            {
                return "LastAccessed";
            }
        }

        /// <summary>
        /// Gets the key of the cookie that hold the TimeOut date of the Session
        /// </summary>
        public static string CookieKeyTimeOut
        {
            get
            {
                return "TimeOut";
            }
        }
    }
}
