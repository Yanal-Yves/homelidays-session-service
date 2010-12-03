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

using System.Collections;
using System.Collections.Generic;
using System.Web;

namespace Homelidays.Web.SessionService
{
    /// <summary>
    /// Session service main class. This class is used by any handler or Page that need the session service
    /// </summary>
    public class AspSession : IEnumerable<KeyValuePair<string, object>>
    {
        /// <summary>
        /// An instance of the data access layer
        /// </summary>
        private AspSessionPersistence sessionPersistence = new AspSessionPersistence();

        /// <summary>
        /// A value indicating whether the session is initialized
        /// </summary>
        private bool isSessionInitialized = false;

        /// <summary>
        /// ASP Session.Contents dictionnary.
        /// </summary>
        private AspSessionContents contents;

        /// <summary>
        /// The current HTTP context
        /// </summary>
        private HttpContext context;

        /// <summary>
        /// Cached SessionId
        /// </summary>
        private string sessionId;

        /// <summary>
        /// Initializes a new instance of the AspSession class.
        /// </summary>
        /// <param name="context">The current HTTP context of the thread instanciating the Session service</param>
        public AspSession(HttpContext context)
        {
            this.context = context;
        }

        /// <summary>
        /// Gets the session id. If found the sesison id comes from the cookie else it generates a new session id, store it into the session cookie.
        /// </summary>
        public string SessionId
        {
            get
            {
                HttpCookie cookie = this.context.Request.Cookies[AppSettingsManager.CookieName];
                if (cookie != null)
                { // If we cann't find the session cookie => generate a session id and create the cookie
                    this.sessionId = this.context.Server.UrlDecode(cookie[AppSettingsManager.CookieKey]).ToLower().Trim();
                }
                else
                {
                    // Génération d'un nouveau session id
                    this.sessionId = this.sessionPersistence.GenerateSessionId();

                    // On écrit le cookie
                    this.SetSessionCookie();
                }

                return this.sessionId;
            }
        }

        /// <summary>
        /// Gets the ASP Session.Contents dictionnary.
        /// </summary>
        public AspSessionContents Contents
        {
            get
            {
                if (!this.isSessionInitialized)
                {
                    // TODO : que se passe-t-il si on est au unload ?
                    this.InitializeSession();
                    this.isSessionInitialized = true;
                }

                return this.contents;
            }
        }

        /// <summary>
        /// Indexer of the Session
        /// </summary>
        /// <param name="key">key of the wanted session variable</param>
        /// <returns>the session variable value</returns>
        public object this[string key]
        {
            get
            {
                return this.Contents[key];
            }

            set
            {
                this.Contents[key] = value;
            }
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
        ///  Create a new session cookie
        /// </summary>
        /// <returns>the cookie created</returns>
        private HttpCookie SetSessionCookie()
        {
            // Création du cookie
            HttpCookie cookie;
            cookie = new HttpCookie(AppSettingsManager.CookieName);

            // Ajout du SessionId
            // Ici on doit utiliser la variable privée car l'appel à SetSessionCookie est provoqué par this.SessionId
            string session_id = this.sessionId;
            cookie.Values.Add(AppSettingsManager.CookieKey, session_id);

            // Ajout du cookie à la réponse http
            this.context.Response.Cookies.Add(cookie);
            return cookie;
        }

        /// <summary>
        /// Initialize the Session State
        /// </summary>
        private void InitializeSession()
        {
            var session_id = this.SessionId;
            AspSessionContents session;
            session = this.sessionPersistence.LoadSession(session_id);
            if (session == null)
            {
                session = new AspSessionContents();
#if HOMELIDAYS_SESSION_ONSTART
#endif
            }

            this.contents = session;
        }

        /// <summary>
        /// Persist session in data accesslayer
        /// </summary>
        public void PersistSession()
        {
            if (this.isSessionInitialized)
            { // Persistence en base de données
                this.sessionPersistence.SaveSession(this.SessionId, this.contents, this.contents.TimeOut);

                this.isSessionInitialized = false;
            }
            else
            { // Refresh the last accessed date
                this.sessionPersistence.RefreshSession(this.SessionId);
            }
        }

        /// <summary>
        /// Abandon session delete the line in the table
        /// </summary>
        /// <param name="context">Httpcontext of the caller</param>
        public void Abandon(HttpContext context)
        {
            this.sessionPersistence.DeleteSession(this.SessionId);
            this.isSessionInitialized = false;
        }

        #region IEnumerable<KeyValuePair<string,object>> Members

        /// <summary>
        /// Provides an enumeartor in order to enumerate the session collection.
        /// </summary>
        /// <returns>An enumerator of the session collection</returns>
        public IEnumerator<KeyValuePair<string, object>> GetEnumerator()
        {
            return this.Contents.GetEnumerator();
        }

        #endregion

        #region IEnumerable Members

        /// <summary>
        /// Provides an enumeartor in order to enumerate the session collection.
        /// </summary>
        /// <returns>An enumerator of the session collection</returns>
        IEnumerator IEnumerable.GetEnumerator()
        {
            return this.Contents.GetEnumerator();
        }

        #endregion
    }
}
