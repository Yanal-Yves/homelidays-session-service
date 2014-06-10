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
using System.IO;
using System.Net;
using System.Threading;

namespace LoadTester
{
    /// <summary>
    /// Load test de la page de test du session service
    /// </summary>
    class Program
    {
        /// <summary>
        /// A value indicating whether to preserver the cookie between two request (one cookie per thread)
        /// If false the storage will increase a lot : the storage should handle 20 minutes of test in its memory.
        /// Usually we prefer doing this test with the same client two avoid "memory leaking" in the storage.
        /// Using the same NbThread clients prevent from checking memory leaks in the session id generation code.
        /// </summary>
        const bool PreserveCookie = true;

        /// <summary>
        /// Number of threads used by the test program
        /// </summary>
        const int NbThread = 20;

        /// <summary>
        /// Tables of cookies. Each thread has one cookie. Used only if PreserveCookie is true
        /// </summary>
        static readonly CookieCollection[] Cookies = new CookieCollection[NbThread];

        /// <summary>
        /// Compte le nombre d'itération de tous les threads 
        /// </summary>
        static int nbRequest;

        /// <summary>
        /// Compte le nombre d'itération de tous les threads pour le test sur la session IIS
        /// </summary>
        static int nbRequestSessAsp;

        /// <summary>
        /// Fait le même job que DoWork plus un comptage d'itération avec affichage
        /// </summary>
        static void DisplayNumberOfRequest()
        {
            for (;;)
            {
                Thread.Sleep(10000);
                Console.WriteLine("Number of HTTP requests handled by AspSessionService:  " + nbRequest);
            }
// ReSharper disable FunctionNeverReturns
        }
// ReSharper restore FunctionNeverReturns

        /// <summary>
        /// Appel la page de test du service de session avec erreur 500 et sans erreur
        /// </summary>
        /// <param name="threadNumber">the number of the thread</param>
        static void CallTest(object threadNumber)
        {
            var threadNumberInt = (int)threadNumber;
            for (;;)
            {
                string url = "http://localhost/SessionService/Test.asp";
                RequestUrl(url, true, false, threadNumberInt);
                url += "?Error=1";
                RequestUrl(url, false, false, threadNumberInt);
                url = "http://localhost/SessionService/TestNoSessionAccess.asp";
                RequestUrl(url, true, false, threadNumberInt);
                url = "http://localhost/SessionService/TestReload.asp";
                RequestUrl(url, true, false, threadNumberInt);
                Interlocked.Add(ref nbRequest, 1);
            }
// ReSharper disable FunctionNeverReturns
        }
// ReSharper restore FunctionNeverReturns

        /// <summary>
        /// Fait le même job que DoWork plus un comptage d'itération avec affichage
        /// </summary>
        static void DisplayNumberOfRequestOldSess()
        {
            for (;;)
            {
                Thread.Sleep(10000);
                Console.WriteLine("IIS Session : " + nbRequestSessAsp);
            }
// ReSharper disable FunctionNeverReturns
        }
// ReSharper restore FunctionNeverReturns

        /// <summary>
        /// Appel la page de test du service de session avec erreur 500 et sans erreur.
        /// </summary>
        /// <param name="threadNumber">the number of the thread</param>
        static void CallTestIisSession(object threadNumber)
        {
            var threadNumberInt = (int)threadNumber;
            for (;;)
            {
                string url = "http://localhost/SessionService/TestIisSession.asp";
                RequestUrl(url, true, false, threadNumberInt);
                url += "?Error=1";
                RequestUrl(url, false, false, threadNumberInt);
                Interlocked.Add(ref nbRequestSessAsp, 1);
            }
// ReSharper disable FunctionNeverReturns
        }
// ReSharper restore FunctionNeverReturns

        /// <summary>
        /// Appel la page de test du service de session avec erreur 500 et sans erreur
        /// </summary>
        /// <param name="threadNumber">the number of the thread</param>
        static void CallTestScriptingDictionary(object threadNumber)
        {
            var threadNumberInt = (int)threadNumber;
            for (;;)
            {
                const string Url = "http://localhost/SessionService/TestScriptingDictionary.asp";
                RequestUrl(Url, true, false, threadNumberInt);
                Interlocked.Add(ref nbRequest, 1);
            }
// ReSharper disable FunctionNeverReturns
        }
// ReSharper restore FunctionNeverReturns

        /// <summary>
        /// Request the Session Service test page
        /// </summary>
        /// <param name="url">Url to call</param>
        /// <param name="stopIfInternalError">Stop the application in case of web server internal error</param>
        /// <param name="displayInfo">Verbose mode used by CallTestIisSession</param>
        /// <param name="threadNumber">the number of the current thread</param>
        private static void RequestUrl(string url, bool stopIfInternalError, bool displayInfo, int threadNumber)
        {
            var request = (HttpWebRequest)WebRequest.Create(url);
            request.CookieContainer = new CookieContainer();

// ReSharper disable ConditionIsAlwaysTrueOrFalse
            if (PreserveCookie)
// ReSharper restore ConditionIsAlwaysTrueOrFalse
            {
                if (Cookies[threadNumber] != null && Cookies[threadNumber].Count > 0)
                { // Set the cookie
                    request.CookieContainer.Add(Cookies[threadNumber]);
                }
            }

            try
            {
                if (displayInfo)
                {
                    Console.WriteLine("avant requête");
                }

                using (var response = (HttpWebResponse)request.GetResponse())
                {
                    if (response.StatusCode != HttpStatusCode.OK)
                    {
                        throw new Exception("Code de retour != 200");
                    }

// ReSharper disable AssignNullToNotNullAttribute
                    var sr = new StreamReader(response.GetResponseStream());
// ReSharper restore AssignNullToNotNullAttribute
                    sr.ReadToEnd();

                    if (response.Cookies.Count > 0)
                    {
                        Cookies[threadNumber] = response.Cookies;
                    }
                }

                if (displayInfo)
                {
                    Console.WriteLine("après requête");
                }
            }
            catch (WebException ex)
            {
                if (stopIfInternalError)
                {
                    Console.WriteLine(ex.Response.ToString());
                    Stream s = ex.Response.GetResponseStream();
// ReSharper disable AssignNullToNotNullAttribute
                    var sr = new StreamReader(s);
// ReSharper restore AssignNullToNotNullAttribute
                    var strResponse = sr.ReadToEnd();
                    Console.WriteLine(strResponse);
                    throw new Exception("500: Interal Error", ex);
                }

                if (displayInfo)
                {
                    Console.WriteLine("après requête");
                }
            }
        }

        /// <summary>
        /// Test entry point
        /// </summary>
        static void Main()
        {
            var thread2 = new Thread(DisplayNumberOfRequest);
            thread2.Start();

            const TestType TestType = TestType.ValueType;

            switch (TestType)
            {
#pragma warning disable 162
// ReSharper disable HeuristicUnreachableCode
                case TestType.ValueType:
                    {
                        for (int i = 0; i < NbThread; i++)
                        {
                            var thread = new Thread(CallTest);
                            thread.Start(i);
                        }
                    }

                    break;
                case TestType.ScriptingDictionary:
                    {
                        for (int i = 0; i < NbThread; i++)
                        {
                            var thread = new Thread(CallTestScriptingDictionary);
                            thread.Start(i);
                        }
                    }

                    break;
                case TestType.ValueTypeAndIisSession:
                    {
                        for (int i = 0; i < NbThread; i++)
                        {
                            var thread = new Thread(CallTest);
                            thread.Start(i);
                        }

                        var thread3 = new Thread(DisplayNumberOfRequestOldSess);
                        thread3.Start();

                        for (int i = 0; i < NbThread; i++)
                        {
                            var thread4 = new Thread(CallTestIisSession);
                            thread4.Start(i);
                        }
                    }

                    break;
                default:
                    {
                        throw new Exception("This should not happend");
                    }
// ReSharper restore HeuristicUnreachableCode
#pragma warning restore 162
            }

            Console.ReadLine();
        }
    }
}
