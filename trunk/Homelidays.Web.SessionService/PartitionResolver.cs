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
using System.Linq;
using System.Xml.Linq;

namespace Homelidays.Web.SessionService
{
    /// <summary>
    /// The PartionResolver class determines which partition (database) to use for each sessionid
    /// </summary>
    public class PartitionResolver
    {
        /// <summary>
        /// An array of connection strings
        /// </summary>
        private static string[] partitionsConnectionStrings;

        /// <summary>
        /// indicates if the configuration is loaded and if the tables are created
        /// </summary>
        private static bool isConfLoaded = false;

        /// <summary>
        /// Initializes a new instance of the PartitionResolver class
        /// </summary>
        public PartitionResolver()
        {
            this.LoadConf();
        }

        /// <summary>
        /// Charge la configuration depuis un fichier xml
        /// </summary>
        private void LoadConf()
        {
            if (!isConfLoaded)
            {
                string currentDir = FileUtility.GetDirectory();
                XDocument doc = XDocument.Load(Path.Combine(currentDir, "AspSessionServiceConfig.xml"));
                if (doc != null)
                {
                    var query = from conf in doc.Element("Partitions").Elements("Partition")
                                select conf.Element("ConnectionString").Value
                                + "User Id=" + conf.Element("Login").Value
                                + ";Password=" + conf.Element("Password").Value;

                    partitionsConnectionStrings = query.ToArray();
                }
                else
                {
                    Loggers.Logger.Error("Unable to load conf");
                }

                SessionPersistence.CreateTables(partitionsConnectionStrings[0]);
                isConfLoaded = true;
            }
        }

        /// <summary>
        /// reset la configuration du partitionresolver
        /// </summary>
        public void ResetConf()
        {
            isConfLoaded = false;
            this.LoadConf();
        }

        /// <summary>
        /// Get the connection string correspond to the provided sessionId
        /// </summary>
        /// <param name="sessionId">The session id</param>
        /// <returns>The connection string to use with the provided session id</returns>
        public string ResolvePartition(object sessionId)
        {
            string session_id = (string)sessionId;

            // hash the incoming session ID into one of the available partitions
            // TODO comprendre la ligne en dessous et sassurer que la répartition est linéaire
            // TODO : géré le retrait d'un serveur de session à chaud !
            // int partitionID = Math.Abs(session_id.GetHashCode()) % this.partitionsConnectionStrings.Length;
            int partitionID = 0;
            return partitionsConnectionStrings[partitionID];
        }
    }
}
