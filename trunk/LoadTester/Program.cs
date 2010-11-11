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
    public class Program
    {
        /// <summary>
        /// A value indicating whether to preserver the cookie between two request (one cookie per thread)
        /// If false the storage will increase a lot : the storage should handle 20 minutes of test in its memory.
        /// Usually we prefer doing this test with the same client two avoid "memory leaking" in the storage.
        /// Using the same NbThread clients prevent from checking memory leaks in the session id generation code.
        /// </summary>
        private const bool PreserveCookie = true;

        /// <summary>
        /// Number of threads used by the test program
        /// </summary>
        private const int NbThread = 20;

        /// <summary>
        /// Compte le nombre d'itération de tous les threads 
        /// </summary>
        private static int numberOfRequest = 0;

        /// <summary>
        /// Compte le nombre d'itération de tous les threads pour le test sur la session IIS
        /// </summary>
        private static int numberOfRequestSessAsp = 0;

        /// <summary>
        /// Tables of cookies. Each thread has one cookie. Used only if PreserveCookie is true
        /// </summary>
        private static CookieCollection[] cookies = new CookieCollection[NbThread];

        /// <summary>
        /// Fait le même job que DoWork plus un comptage d'itération avec affichage
        /// </summary>
        private static void DisplayNumberOfRequest()
        {
            for (;;)
            {
                Thread.Sleep(10000);
                Console.WriteLine("Number of HTTP requests handled by AspSessionService:  " + numberOfRequest);
            }
        }

        /// <summary>
        /// Appel la page de test du service de session avec erreur 500 et sans erreur
        /// </summary>
        /// <param name="threadNumber">the number of the thread</param>
        private static void CallTest(object threadNumber)
        {
            int thread_number = (int)threadNumber;
            for (;;)
            {
                string url = "http://sessionservice.my.homelidays.com/SessionService/Test.asp";
                RequestUrl(url, true, false, thread_number);
                url += "?Error=1";
                RequestUrl(url, false, false, thread_number);
                Interlocked.Add(ref numberOfRequest, 1);
            }
        }

        /// <summary>
        /// Fait le même job que DoWork plus un comptage d'itération avec affichage
        /// </summary>
        private static void DisplayNumberOfRequestOldSess()
        {
            for (;;)
            {
                Thread.Sleep(10000);
                Console.WriteLine("IIS Session : " + numberOfRequestSessAsp);
            }
        }

        /// <summary>
        /// Appel la page de test du service de session avec erreur 500 et sans erreur.
        /// </summary>
        /// <param name="threadNumber">the number of the thread</param>
        private static void CallTestIisSession(object threadNumber)
        {
            int thread_number = (int)threadNumber;
            for (;;)
            {
                string url = "http://sessionservice2.my.homelidays.com/SessionService/TestIisSession.asp";
                RequestUrl(url, true, false, thread_number);
                url += "?Error=1";
                RequestUrl(url, false, false, thread_number);
                Interlocked.Add(ref numberOfRequestSessAsp, 1);
            }
        }

        /// <summary>
        /// Appel la page de test du service de session avec erreur 500 et sans erreur
        /// </summary>
        /// <param name="threadNumber">the number of the thread</param>
        private static void CallTestScriptingDictionary(object threadNumber)
        {
            int thread_number = (int)threadNumber;
            for (;;)
            {
                string url = "http://sessionservice.my.homelidays.com/SessionService/TestScriptingDictionary.asp";
                RequestUrl(url, true, false, thread_number);
                Interlocked.Add(ref numberOfRequest, 1);
            }
        }

        /// <summary>
        /// Request the Session Service test page
        /// </summary>
        /// <param name="url">Url to call</param>
        /// <param name="stopIfInternalError">Stop the application in case of web server internal error</param>
        /// <param name="displayInfo">Verbose mode used by CallTestIisSession</param>
        /// <param name="threadNumber">the number of the current thread</param>
        private static void RequestUrl(string url, bool stopIfInternalError, bool displayInfo, int threadNumber)
        {
            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(url);
            string result;
            request.CookieContainer = new CookieContainer();

            if (PreserveCookie)
            {
                if (Program.cookies[threadNumber] != null && Program.cookies[threadNumber].Count > 0)
                { // Set the cookie
                    request.CookieContainer.Add(Program.cookies[threadNumber]);
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

                    StreamReader sr = new StreamReader(response.GetResponseStream());
                    result = sr.ReadToEnd();

                    if (response.Cookies.Count > 0)
                    {
                        Program.cookies[threadNumber] = response.Cookies;
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
                    StreamReader sr = new StreamReader(s);
                    var str_response = sr.ReadToEnd();
                    Console.WriteLine(str_response);
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
        /// <param name="args">Application argument : none</param>
        private static void Main(string[] args)
        {
            Thread thread2 = new Thread(DisplayNumberOfRequest);
            thread2.Start();

            TestType test_type = TestType.ValueType;

            switch (test_type)
            {
                case TestType.ValueType:
                    {
                        for (int i = 0; i < Program.NbThread; i++)
                        {
                            Thread thread = new Thread(CallTest);
                            thread.Start(i);
                        }
                    }

                    break;
                case TestType.ScriptingDictionary:
                    {
                        for (int i = 0; i < Program.NbThread; i++)
                        {
                            Thread thread = new Thread(CallTestScriptingDictionary);
                            thread.Start(i);
                        }
                    }

                    break;
                case TestType.ValueTypeAndIisSession:
                    {
                        for (int i = 0; i < Program.NbThread; i++)
                        {
                            Thread thread = new Thread(CallTest);
                            thread.Start(i);
                        }

                        Thread thread3 = new Thread(DisplayNumberOfRequestOldSess);
                        thread3.Start();

                        for (int i = 0; i < Program.NbThread; i++)
                        {
                            Thread thread4 = new Thread(CallTestIisSession);
                            thread4.Start(i);
                        }
                    }

                    break;
                default:
                    {
                        throw new Exception("This should not happend");
                    }
            }

            Console.ReadLine();
        }
    }
}
