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
using System.IO;
using System.Linq;
using System.Text;
using System.Web;
using MbUnit.Framework;

namespace Homelidays.Web.SessionService.Tests
{
    /// <summary>
    /// this is a test class for Session Service
    /// </summary>
    [TestFixture]
    public class SessionServiceTest
    {
        /// <summary>
        /// Test du initialize Session
        /// </summary>
        [Test]
        public void InitializeSessionTest()
        {
            // création des objets réponses Request et Response pour le HtppContext
            HttpRequest request = new HttpRequest("myFileName", "http://urlQuelconque", "BBQ=OUI");
            Encoding requestEncoding = Encoding.GetEncoding("ISO-8859-1");
            request.ContentEncoding = requestEncoding;
            Stream str = Stream.Null;
            StreamWriter sw = new StreamWriter(str);
            HttpResponse response = new HttpResponse(sw);
            HttpContext context = new HttpContext(request, response);
            var session = SessionService.InitializeSession(context);
            Assert.IsNotNull(session);
        }

        /// <summary>
        /// test du CreateNewSessionCookie
        /// </summary>
        [Test]
        public void CreateNewSessionCookieTest()
        {
            // création des objets réponses Request et Response pour le HtppContext
            HttpRequest request = new HttpRequest("myFileName", "http://urlQuelconque", "BBQ=OUI");
            Encoding requestEncoding = Encoding.GetEncoding("ISO-8859-1");
            request.ContentEncoding = requestEncoding;
            Stream str = Stream.Null;
            StreamWriter sw = new StreamWriter(str);
            HttpResponse response = new HttpResponse(sw);
            HttpContext context = new HttpContext(request, response);
            string session_id = Guid.NewGuid().ToString().Replace("-", string.Empty);
            var cookie = SessionService.SetCookie(context, 20, session_id);
            Assert.IsNotNull(cookie);
        }

        /// <summary>
        /// test du PersistSession
        /// </summary>
        [Test]
        public void PersistSessionTest()
        {
            // création des objets réponses Request et Response pour le HtppContext
            HttpRequest request = new HttpRequest("myFileName", "http://urlQuelconque", "BBQ=OUI");
            Encoding requestEncoding = Encoding.GetEncoding("ISO-8859-1");
            request.ContentEncoding = requestEncoding;

            // generate cookie
            SessionPersistence persist = new SessionPersistence();
            string key = persist.GenerateSessionId();
            HttpCookie cookie = new HttpCookie(AppSettingsManager.CookieName);
            cookie.Values.Add(AppSettingsManager.CookieKey, key);
            request.Cookies.Add(cookie);
            Stream str = Stream.Null;
            StreamWriter sw = new StreamWriter(str);
            HttpResponse response = new HttpResponse(sw);
            HttpContext context = new HttpContext(request, response);

            // Create SessionState
            SessionPersistenceTest pt = new SessionPersistenceTest();
            SessionState sess = pt.CreateSessionState();
            SessionService.PersistSession(context, sess);
            var sessloaded = persist.LoadSession(key);
            foreach (var item in sess)
            {
                Assert.AreEqual(item.Value, sessloaded[item.Key]);
            }
        }

        /// <summary>
        /// test du AbandonSession
        /// </summary>
        [Test]
        public void AbandonSessionTest()
        {
            // création des objets réponses Request et Response pour le HtppContext
            HttpRequest request = new HttpRequest("myFileName", "http://urlQuelconque", "BBQ=OUI");
            Encoding requestEncoding = Encoding.GetEncoding("ISO-8859-1");
            request.ContentEncoding = requestEncoding;

            // generate cookie
            SessionPersistence persist = new SessionPersistence();
            string key = persist.GenerateSessionId();
            HttpCookie cookie = new HttpCookie(AppSettingsManager.CookieName);
            cookie.Values.Add(AppSettingsManager.CookieKey, key);
            request.Cookies.Add(cookie);
            Stream str = Stream.Null;
            StreamWriter sw = new StreamWriter(str);
            HttpResponse response = new HttpResponse(sw);
            HttpContext context = new HttpContext(request, response);
            
            // Create SessionState
            SessionPersistenceTest pt = new SessionPersistenceTest();
            SessionState sess = pt.CreateSessionState();
            
            // persist context in base
            SessionService.PersistSession(context, sess);
            
            // test abandon
            SessionService.Abandon(context);
            var sessloaded = persist.LoadSession(key);
            Assert.IsNull(sessloaded);
        }
    }
}
