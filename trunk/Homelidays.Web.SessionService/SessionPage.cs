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
using System.Configuration;
using System.Web;

namespace Homelidays.Web.SessionService
{
    /// <summary>
    /// An aspx page that enables the Yacht scalable session state store
    /// </summary>
    public class SessionPage : System.Web.UI.Page
    {
        /// <summary>
        /// The session state
        /// </summary>
        private SessionState session = null;

        /// <summary>
        /// A value indicating whether the session is initialized
        /// </summary>
        private bool isSessionInitialized = false;

        /// <summary>
        /// Gets or sets the session state
        /// </summary>
        public SessionState AspSession
        {
            get
            {
                if (!this.isSessionInitialized)
                {
                    // TODO : que se passe-t-il si on est au unload ?
                    this.session = SessionService.InitializeSession(Context);
                    this.Unload += new EventHandler(this.SessionPageUnload);
                    this.isSessionInitialized = true;
                }

                return this.session;
            }

            set
            {
                this.session = value;
            }
        }

        /// <summary>
        /// Abandon Session delete the line in the table
        /// </summary>
        public void AbandonSession()
        {
            SessionService.Abandon(Context);
            this.isSessionInitialized = false;
        }

        /// <summary>
        /// a l'unload de la page on fait  la persistence en base
        /// </summary>
        /// <param name="sender">objet sender de l'event</param>
        /// <param name="e">Event args</param>
        public void SessionPageUnload(object sender, EventArgs e)
        {
            if (this.isSessionInitialized)
            {
                SessionService.PersistSession(this.Context, this.session);
                this.isSessionInitialized = false;
            }
        }

        /// <summary>
        /// OnLoad de la HomelidaysPage
        /// </summary>
        /// <param name="e">Evenement du OnLoad</param>
        protected override void OnLoad(EventArgs e)
        {
            // Rafraichissement de la date de dernier accès du cookie Yacht + rafraichissement de la date dans la BDD
            this.RefreshCookieYachtLastTimeAccess();

            base.OnLoad(e);
        }

        /// <summary>
        /// Parse a W3C formated string to a DateTime
        /// </summary>
        /// <param name="dateTimeStringW3C">W3C formated string</param>
        /// <returns>The parsed datetime corresponding to the provided W3C formated string</returns>
        private static DateTime ParseW3C(string dateTimeStringW3C)
        {
            int year = int.Parse(dateTimeStringW3C.Substring(0, 4));
            int month = int.Parse(dateTimeStringW3C.Substring(5, 2));
            int day = int.Parse(dateTimeStringW3C.Substring(8, 2));
            int hour = int.Parse(dateTimeStringW3C.Substring(11, 2));
            int minute = int.Parse(dateTimeStringW3C.Substring(14, 2));
            DateTime date_time = new DateTime(year, month, day, hour, minute, 0);

            return date_time;
        }

        /// <summary>
        /// Rafraichissement de la date de dernier accès du cookie Yacht + rafraichissement de la date dans la BDD
        /// Equivalent du bout de code dans le 010-Include/AspSessionService.asp
        /// </summary>
        private void RefreshCookieYachtLastTimeAccess()
        {
            if (Request.Cookies != null && Request.Cookies["Yacht"] != null)
            {
                string lastAccess = Request.Cookies["Yacht"]["LastAccessed"];

                if (!string.IsNullOrEmpty(lastAccess))
                {
                    TimeSpan elapsedTimeSinceLastAccess = this.GetElapsedTimeSinceSessionLastAccess(lastAccess);

                    // On récupère le timeout
                    string time_out_str = Request.Cookies["Yacht"]["TimeOut"];

                    if (!string.IsNullOrEmpty(time_out_str))
                    {
                        int time_out = int.Parse(time_out_str);

                        // On récupère la borne suppérieur et la borne inférieur de la fenêtre dans laquelle on rafraichie la session.
                        int slidingUpperBound = time_out + (time_out / 2);
                        int slidingLowerBound = time_out / 2;

                        // Pour raffraichir la date dans le cookie et la base il faut que le temps de session active 
                        // soit entre 10 et 30 minutes pour une session à 20 minute par défaut
                        if (elapsedTimeSinceLastAccess.TotalMinutes > slidingLowerBound
                            && elapsedTimeSinceLastAccess.TotalMinutes < slidingUpperBound)
                        {
                            // Ici volontairement on fait appel à la Session pour que le SessionService remette à jour la 
                            // date du cookie et que la BDD soit mise à jour
                            string valeurTemp = this.AspSession["dummy"].ToString();
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Get the elapsed time interval since the last session access date and now.
        /// </summary>
        /// <param name="lastAccess">W3C formated string indicating the last accessed date.</param>
        /// <returns>The time interval since the last accessed date and now</returns>
        private TimeSpan GetElapsedTimeSinceSessionLastAccess(string lastAccess)
        {
            string lastAccessDecoded = Server.UrlDecode(lastAccess);

            // Parsing de la date récupérée dans le cookie qui est en temps UTC
            // on n'utilise pas le server url decode car il transforme l'heure UTC en heure du serveur
            DateTime dateLastCookieAccess = ParseW3C(lastAccessDecoded);

            // DateTime actuel en UTC
            DateTime currentTime = DateTime.Now.ToUniversalTime();

            TimeSpan elapsedTimeSinceLastAccess = currentTime - dateLastCookieAccess;

            return elapsedTimeSinceLastAccess;
        }
    }
}
