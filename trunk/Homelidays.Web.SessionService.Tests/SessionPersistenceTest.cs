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
using MbUnit.Framework;

namespace Homelidays.Web.SessionService.Tests
{
    /// <summary>
    /// This is a test class for SessionPersistence class
    /// </summary>
    [TestFixture]
    public class SessionPersistenceTest
    {
        /// <summary>
        /// Serialized session expected into the database
        /// </summary>
        private string sessionXml = "<?xml version=\"1.0\" encoding=\"utf-16\"?><Session><Table Key=\"keyarray\" TypeName=\"Table\" Dimension=\"1\" Bounds=\"3\"><Item Key=\"keyarray\" TypeName=\"String\" Coord=\"0\"><![CDATA[Yanal]]></Item><Item Key=\"keyarray\" TypeName=\"String\" Coord=\"1\"><![CDATA[Joe]]></Item><Item Key=\"keyarray\" TypeName=\"String\" Coord=\"2\"><![CDATA[Simon]]></Item></Table><Item Key=\"keybool\" TypeName=\"Bool\">1</Item><Item Key=\"keysignedbyte\" TypeName=\"SingedByte\">-12</Item><Item Key=\"keybyte\" TypeName=\"UnsingedByte\">12</Item><Item Key=\"keycur\" TypeName=\"Currency\">543</Item><Item Key=\"keydate\" TypeName=\"Date\">1962-10-19 00:00:00</Item><Item Key=\"keydbl\" TypeName=\"Real8\">3,23454</Item><Item Key=\"keyushort\" TypeName=\"UnsingedShort\">13</Item><Item Key=\"keyint\" TypeName=\"SingedShort\">12</Item><Item Key=\"keyuint\" TypeName=\"UnsingedInteger\">2434</Item><Item Key=\"keylng\" TypeName=\"SingedInteger\">25427</Item><Item Key=\"keynull\" TypeName=\"Null\"></Item><Item Key=\"keysng\" TypeName=\"Real4\">75,34211</Item><Item Key=\"keydecimal\" TypeName=\"Decimal\">234,456</Item><Item Key=\"keytime\" TypeName=\"Date\">1899-12-30 04:35:47</Item><Item Key=\"keyempty\" TypeName=\"Empty\"></Item></Session>";

        /// <summary>
        /// Create a session state for the tests
        /// </summary>
        /// <returns>the test sesssion state</returns>
        public SessionState CreateSessionState()
        {
            SessionState st = new SessionState();

            // Tableau
            object[] array = new object[3];
            array[0] = "Yanal";
            array[1] = "Joe";
            array[2] = "Simon";
            st.Add("keyarray", array);

            // Bool
            st.Add("keybool", true);

            // SingedByte
            sbyte sbyt = -12;
            st.Add("KeySignedByte", sbyt);

            // Unsigned byte
            byte byt = 12;
            st.Add("keybyte", byt);
            st.Add("keycur", new Currency(543));
            st.Add("keydate", new DateTime(1962, 10, 19));

            // double
            double doub = 3.23454;
            st.Add("keydbl", doub);

            // UnsingedShort"
            st["KeyUshort"] = (ushort)13;

            // Short
            short myshort = 12;
            st.Add("keyint", myshort);

            // UnsingedInteger
            st["KeyUInt"] = (uint)2434;

            // SingedInteger
            int myint = 25427;
            st.Add("keylng", myint);
            st.Add("keynull", null);

            // Single
            float mysing = 75.34211F;
            st.Add("keysng", mysing);

            // Decimal
            st["KeyDecimal"] = (decimal)234.456;

            // Date
            st.Add("keytime", new DateTime(1899, 12, 30, 04, 35, 47));

            // Empty
            st["keyempty"] = new AspEmpty();

            return st;
        }

        /// <summary>
        /// Test of SessionPersistence.Serialize
        /// </summary>
        [Test]
        public void SerializeTest()
        {
            SessionPersistence persist = new SessionPersistence();
            var st = this.CreateSessionState();
            var xml = persist.Serialize(st);
            Assert.AreEqual(xml, this.sessionXml);
        }

        /// <summary>
        /// Test of SessionPersistence.DeSerialize
        /// </summary>
        [Test]
        public void DeSerializeTest()
        {
            SessionPersistence persist = new SessionPersistence();
            var sess = persist.Deserialize(this.sessionXml);
            var st = this.CreateSessionState();
            foreach (var item in sess)
            {
                object session_value = st[item.Key]; // st[item.Key].GetType() == item.value.GetType()
                Assert.AreEqual(item.Value, session_value);
            }
        }

        /// <summary>
        /// Test of SessionPersistence.SaveSession
        /// </summary>
        [Test]
        public void SaveSessionTest()
        {
            PartitionResolver pr = new PartitionResolver();
            SessionPersistence persist = new SessionPersistence();
            SessionState st = this.CreateSessionState();
            var sessionid = persist.GenerateSessionId();
            persist.SaveSession(sessionid, st, 20);
            var sessionSaved = persist.LoadSession(sessionid);
            foreach (var item in sessionSaved)
            {
                Assert.AreEqual(item.Value, st[item.Key]);
            }
        }

        /// <summary>
        /// Test of SessionPersistence.LoadSession
        /// </summary>
        [Test]
        public void LoadSessionTest()
        {
            // cette méthode est déja testé dans le savesessiontest
        }

        /// <summary>
        /// Test of SessionPersistence.CreateTable
        /// </summary>
        [Test]
        public void CreateTableTest()
        {
            SessionPersistence.CreateTables(ConfigurationManager.ConnectionStrings["CnxForCreateTable"].ConnectionString);
        }

        /// <summary>
        /// Test of SessionPersistence.GenerateNewSessionId
        /// </summary>
        [Test]
        public void GenerateNewSessionIdTest()
        {
            SessionPersistence persist = new SessionPersistence();
            persist.GenerateSessionId();
        }
    }
}
