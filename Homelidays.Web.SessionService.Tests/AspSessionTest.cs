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
    public class AspSessionTest
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
            AspSession asp_session = new AspSession(context);
            ReflectionUtility.CallNonPublicMethod(asp_session, "InitializeSession", null); // equivaut à asp_session.InitializeSession();
            Assert.IsNotNull(asp_session.Contents);
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
            var asp_session = new AspSession(context);
            string session_id = Guid.NewGuid().ToString().Replace("-", string.Empty);
            ReflectionUtility.SetNonPublicField(asp_session, "sessionId", session_id);
            HttpCookie cookie = (HttpCookie)ReflectionUtility.CallNonPublicMethod(asp_session, "SetSessionCookie", null);
            Assert.IsNotNull(cookie);
            var session_id_cookie_str = cookie["SessionId"];
            Assert.AreEqual(session_id, session_id_cookie_str);
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
            AspSessionPersistence persist = new AspSessionPersistence();
            string key = persist.GenerateSessionId();
            HttpCookie cookie = new HttpCookie(AppSettingsManager.CookieName);
            cookie.Values.Add(AppSettingsManager.CookieKey, key);
            request.Cookies.Add(cookie);
            Stream str = Stream.Null;
            StreamWriter sw = new StreamWriter(str);
            HttpResponse response = new HttpResponse(sw);
            HttpContext context = new HttpContext(request, response);

            // Create AspSession
            AspSessionPersistenceTest pt = new AspSessionPersistenceTest();
            AspSession asp_session = new AspSession(context);
            pt.CreateSessionState(asp_session.Contents);
            asp_session.PersistSession();
            var sessloaded = persist.LoadSession(key);
            foreach (var item in asp_session.Contents)
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
            AspSessionPersistence persist = new AspSessionPersistence();
            string key = persist.GenerateSessionId();
            HttpCookie cookie = new HttpCookie(AppSettingsManager.CookieName);
            cookie.Values.Add(AppSettingsManager.CookieKey, key);
            request.Cookies.Add(cookie);
            Stream str = Stream.Null;
            StreamWriter sw = new StreamWriter(str);
            HttpResponse response = new HttpResponse(sw);
            HttpContext context = new HttpContext(request, response);
            
            // Create AspSession
            AspSessionPersistenceTest pt = new AspSessionPersistenceTest();
            AspSession asp_session = new AspSession(context);
            pt.CreateSessionState(asp_session.Contents);
            
            // persist context in base
            asp_session.PersistSession();
            
            // test abandon
            asp_session.Abandon(context);
            var sessloaded = persist.LoadSession(key);
            Assert.IsNull(sessloaded);
        }
    }
}
