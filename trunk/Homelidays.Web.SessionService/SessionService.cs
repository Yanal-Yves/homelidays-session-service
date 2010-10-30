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
using System.Linq;
using System.Text;
using System.Web;

namespace Homelidays.Web.SessionService
{
    /// <summary>
    /// Session service main class. This class is used by any handler or Page that need the session service
    /// </summary>
    public class SessionService
    {
        /// <summary>
        /// An instance of the data access layer
        /// </summary>
        private static ISessionPersistence sessionPersistence = new SessionPersistence();

        /// <summary>
        /// Initialize the Session State
        /// </summary>
        /// <param name="context">Http context related</param>
        /// <returns>Sessionstate initialized new or deserialized from base</returns>
        public static SessionState InitializeSession(HttpContext context)
        {
            HttpCookie cookie = context.Request.Cookies[AppSettingsManager.CookieName];
            SessionState session;
            if (cookie == null)
            {
                session = new SessionState();
#if HOMELIDAYS_SESSION_ONSTART
#endif
            }
            else
            {
                string key = context.Server.UrlDecode(cookie[AppSettingsManager.CookieKey]).ToLower().Trim();
                session = sessionPersistence.LoadSession(key);
                if (session == null)
                { // peut se produire si un admin vide les session en base et que la session d'un utilisateur n'est pas terminée
                    session = new SessionState();
#if HOMELIDAYS_SESSION_ONSTART
#endif
                }
            }

            return session;
        }

        /// <summary>
        ///  Create a new session cookie
        /// </summary>
        /// <param name="context">the http Context to create the cookie</param>
        /// <param name="timeOut">The session timeout to add into the cookie</param>
        /// <param name="sessionId">The session Id to add into the cookie</param>
        /// <returns>the cookie created</returns>
        public static HttpCookie SetCookie(HttpContext context, int timeOut, string sessionId)
        {
            // Création du cookie
            HttpCookie cookie;
            cookie = new HttpCookie(AppSettingsManager.CookieName);

            // Ajout du LastAccessDate
            DateTime now = DateTime.Now.ToUniversalTime();
            string last_accessed_date = now.ToString("yyyy-MM-ddTHH:mmZ");
            string url_enc_last_accessed_date = UrlEncode(last_accessed_date);
            cookie.Values.Add(AppSettingsManager.CookieKeyLastAccessed, url_enc_last_accessed_date);

            // Ajout du TimeOut
            cookie.Values.Add(AppSettingsManager.CookieKeyTimeOut, timeOut.ToString());

            // Ajout du SessionId
            string session_id = sessionId;
            cookie.Values.Add(AppSettingsManager.CookieKey, session_id);

            // Ajout du cookie à la réponse http
            context.Response.Cookies.Add(cookie);
            return cookie;
        }

        /// <summary>
        /// Enocde input string like ASP is doing for '-' and ':'. This is used for encoding last accessed date
        /// </summary>
        /// <param name="lastAccessedDate">the string to encode</param>
        /// <returns>The encoded string</returns>
        private static string UrlEncode(string lastAccessedDate)
        {
            string url_enc_last_accessed_date = lastAccessedDate.Replace("-", "%2D");
            url_enc_last_accessed_date = url_enc_last_accessed_date.Replace(":", "%3A");
            return url_enc_last_accessed_date;
        }

        /// <summary>
        /// Persist session in data accesslayer
        /// </summary>
        /// <param name="context">Httpcontext of the caller</param>
        /// <param name="session">Sessionstate to save</param>
        public static void PersistSession(HttpContext context, SessionState session)
        {
            HttpCookie cookie = context.Request.Cookies[AppSettingsManager.CookieName];
            string session_id;
            if (cookie != null)
            {
                session_id = context.Server.UrlDecode(cookie[AppSettingsManager.CookieKey]).ToLower().Trim();
            }
            else
            {
                session_id = sessionPersistence.GenerateSessionId();
            }

            // génère le cookie
            cookie = SetCookie(context, session.TimeOut, session_id);

            // persistence en base de données
            sessionPersistence.SaveSession(session_id, session, session.TimeOut);
        }

        /// <summary>
        /// Abandon session delete the line in the table
        /// </summary>
        /// <param name="context">Httpcontext of the caller</param>
        public static void Abandon(HttpContext context)
        {
            SessionPersistence persist = new SessionPersistence();
            HttpCookie cookie = context.Request.Cookies[AppSettingsManager.CookieName];
            string session_id = context.Server.UrlDecode(cookie[AppSettingsManager.CookieKey]).ToLower().Trim();
            persist.DeleteSession(session_id);
        }
    }
}
