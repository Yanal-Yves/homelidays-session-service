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
using System.Data.SqlClient;
using System.IO;
using System.Linq;
using System.Text;
using System.Xml;
using System.Xml.Linq;

namespace Homelidays.Web.SessionService
{
    /// <summary>
    /// The session state service data access layer
    /// </summary>
    public class AspSessionPersistence
    {
        /// <summary>
        /// Requête de lecture de la session.
        /// Attention un changement de l'ordre des paramètres nécessite une mise à jour du code de cette classe 
        /// (ex : reader.GetDateTime(0))
        /// </summary>
        private const string SelectStatement = "SELECT /*6123A967-9D20-41a6-BED0-64A82D69D783*/ LastAccessed, Data,SessionTimeOut FROM dbo.Session WHERE SessionId=@Id and LastAccessed > DATEADD(minute, -SessionTimeOut, GETUTCDATE())";

        /// <summary>
        /// Request that just touch the session state by updating the last access date.
        /// </summary>
        private const string UpdateLastAccessedStatement = "UPDATE /*07614B4A-4224-492e-88DE-656DB7F876F3*/ [Session] SET LastAccessed = GETUTCDATE() WHERE SessionId=@Id";

        /// <summary>
        /// Request that insert a new session state into the database. The SQL statement is not taged as it is always used with the <see cref="DeleteStatement" /> SQL
        /// </summary>
        private const string InsertStatement = "INSERT INTO dbo.Session(SessionId,LastAccessed,SessionTimeOut,Data) VALUES (@Id,GETUTCDATE(),@SessionTimeOut,@Data);";

        /// <summary>
        /// delete a session state into the database
        /// </summary>
        private const string DeleteStatement = "DELETE /*4080907D-4E44-41e6-A036-4B7CB41146D5*/ FROM dbo.Session WHERE SessionId=@Id;";

        /// <summary>
        /// Initializes a new instance of the AspSessionPersistence class.
        /// </summary>
        public AspSessionPersistence()
        {
        }

        /// <summary>
        /// Serialize the type of the object
        /// </summary>
        /// <param name="obj">the object to serializee</param>
        /// <returns>type name of the object</returns>
        private string SerializeTypeName(object obj)
        {
            string myString = string.Empty;

            if (obj == null)
            {
                myString = "Null";
                return myString;
            }

            var type = obj.GetType().FullName;

            switch (type)
            {
                case "System.Byte":
                    myString = "UnsingedByte";
                    break;
                case "System.SByte":
                    myString = "SingedByte";
                    break;
                case "System.Int16":
                    myString = "SingedShort";
                    break;
                case "System.Int32":
                    myString = "SingedInteger";
                    break;
                case "System.UInt16":
                    myString = "UnsingedShort";
                    break;
                case "System.UInt32":
                    myString = "UnsingedInteger";
                    break;
                case "System.Decimal":
                    myString = "Decimal";
                    break;
                case "Homelidays.Web.SessionService.Currency":
                    myString = "Currency";
                    break;
                case "System.Single":
                    myString = "Real4";
                    break;
                case "System.Double":
                    myString = "Real8";
                    break;
                case "System.Boolean":
                    myString = "Bool";
                    break;
                case "System.DateTime":
                    myString = "Date";
                    break;
                case "Homelidays.Web.SessionService.AspEmpty":
                    myString = "Empty";
                    break;
                case "System.String":
                    myString = "String";
                    break;
                default:
                    Loggers.Logger.Error("Try to serialize unknown type name");
                    break;
            }

            return myString;
        }

        /// <summary>
        /// Serialize an object
        /// </summary>
        /// <param name="obj">the object to serialize</param>
        /// <returns>the string containing the serialized object</returns>
        private object SerializeObject(object obj)
        {
            string myString = string.Empty;
            if (obj == null)
            {
                return myString;
            }

            var type = obj.GetType().FullName;
            switch (type)
            {
                case "Homelidays.Web.SessionService.AspEmpty":
                    myString = string.Empty;
                    break;
                case "System.String":
                    XCData myXcData = new XCData(obj.ToString());
                    return myXcData;
                case "System.DateTime":
                    myString = string.Format("{0:u}", ((DateTime)obj));
                    myString = myString.Remove(myString.Length - 1);
                    break;
                case "Homelidays.Web.SessionService.Currency":
                    myString = ((Currency)obj).Amount.ToString().Replace('.', ',');
                    break;
                case "System.Boolean":
                    myString = Convert.ToInt32(obj).ToString();
                    break;
                case "System.Decimal":
                case "System.Single":
                case "System.Double":
                    myString = obj.ToString().Replace('.', ',');
                    break;
                default:
                    myString = obj.ToString();
                    break;
            }

            return myString;
        }

        /// <summary>
        /// Serialize an item or a table
        /// </summary>
        /// <param name="key">name of the item</param>
        /// <param name="value">the item to serialize</param>
        /// <returns>The Xelement corresponding to the serialized item </returns>
        private XElement SerializeItemOrTable(string key, object value)
        {
            if (value != null && value.GetType().BaseType == System.Type.GetType("System.Array"))
            {
                return this.SerializeTable(key, (Array)value);
            }
            else
            {
                return this.SerializeItem(key, value);
            }
        }

        /// <summary>
        /// Serialize the bounds of the table
        /// </summary>
        /// <param name="value">the array to serialize</param>
        /// <returns>string corresponding to the serialized table</returns>
        private string SerializeBounds(Array value)
        {
            string bounds = string.Empty;
            for (int i = value.Rank; i > 0; i--)
            {
                if (i != value.Rank)
                {
                    bounds += ",";
                }

                bounds += value.GetLength(i - 1);
            }

            return bounds;
        }

        /// <summary>
        /// Serialize insisde elements of the table
        /// </summary>
        /// <param name="key">name of the table</param>
        /// <param name="value">the table to serialize</param>
        /// <returns>the Xelement corresponding to the serialized table</returns>
        private XElement[] SerializeElementsOfTable(string key, Array value)
        {
            int[] bounds = new int[value.Rank];
            XElement[] elementsOfTable = new XElement[value.Length];
            for (int j = 0; j < value.Length; j++)
            {
                elementsOfTable[j] = this.SerializeItemOrTable(key, value.GetValue(bounds));
                var coordSerialized = bounds.Select(b => b.ToString()).Aggregate((bout, bout2) => bout.ToString() + "," + bout2.ToString());
                elementsOfTable[j].SetAttributeValue("Coord", coordSerialized);
                bounds = this.GetCoord(bounds, value, value.Rank);
            }

            return elementsOfTable;
        }

        /// <summary>
        /// Get the coord in the table to serialize
        /// </summary>
        /// <param name="bounds">bounds of the table</param>
        /// <param name="value">the array from which to extract the coordinates</param>
        /// <param name="dim">number of dimension</param>
        /// <returns>A table containing the coord</returns>
        private int[] GetCoord(int[] bounds, Array value, int dim)
        {
            for (int i = dim - 1; i > -1; i--)
            {
                if (bounds[i] < value.GetUpperBound(i))
                {
                    bounds[i]++;
                    return bounds;
                }
                else
                {
                    bounds[i] = 0;
                }
            }

            return null;
        }

        /// <summary>
        /// Serialize a table in xml
        /// </summary>
        /// <param name="key">name of the table</param>
        /// <param name="value">the table to serialize</param>
        /// <returns>the Xelement corresponding to the serialized item</returns>
        private XElement SerializeTable(string key, Array value)
        {
            return new XElement(
                "Table",
                new XAttribute("Key", key),
                new XAttribute("TypeName", "Table"),
                new XAttribute("Dimension", value.Rank),
                new XAttribute("Bounds", this.SerializeBounds(value)),
                this.SerializeElementsOfTable(key, value));
        }

        /// <summary>
        /// Serialize an item
        /// </summary>
        /// <param name="key">name of the item</param>
        /// <param name="value">value of the item</param>
        /// <returns>the Xelement corresponding to the Serialized item</returns>
        private XElement SerializeItem(string key, object value)
        {
            return new XElement(
                "Item",
                new XAttribute("Key", key),
                new XAttribute("TypeName", this.SerializeTypeName(value)),
                this.SerializeObject(value));
        }

        /// <summary>
        /// Serialize the SessionState Collection in order to save it in the database
        /// </summary>
        /// <param name="session">the session state to serailize to a string</param>
        /// <returns>a string of an formatted xml</returns>
        public string Serialize(AspSessionContents session)
        {
            if (session == null)
            {
                return null;
            }

            var xquery = from sess in session
                         select this.SerializeItemOrTable(sess.Key, sess.Value);

            StringBuilder sb = new StringBuilder();
            XmlWriterSettings xws = new XmlWriterSettings();
            xws.OmitXmlDeclaration = false;
            xws.Indent = false;

            using (XmlWriter xw = XmlWriter.Create(sb, xws))
            {
                XDocument doc = new XDocument(new XElement("Session", xquery));
                doc.WriteTo(xw);
            }

            return sb.ToString();
        }

        /// <summary>
        /// Deserialize un xml en objet sessionstate
        /// </summary>
        /// <param name="xml">xml a déserializer</param>
        /// <returns>retourne un objet sessionstate</returns>
        public AspSessionContents Deserialize(string xml)
        {
            if (xml == null)
            {
                return null;
            }

            AspSessionContents session = new AspSessionContents();

            XDocument xdoc = XDocument.Parse(xml);
            var queryforItem = from item in xdoc.Element("Session").Elements()
                               select new
                               {
                                   key = item.Attribute("Key").Value,
                                   value = this.DeserializeItem(item)
                               };
            foreach (var item in queryforItem)
            {
                session.Add(item.key, item.value);
            }

            return session;
        }

        /// <summary>
        /// Deserialize un item commence par vérifier si c'est une table ou un item
        /// </summary>
        /// <param name="element">Xelement qui contient l'item</param>
        /// <returns>renvoie un object ou null si c ni un item ni un table</returns>
        private object DeserializeItem(XElement element)
        {
            if (element.Name == "Item")
            {
                return this.DeserializeFromType(element);
            }

            if (element.Name == "Table")
            {
                return this.DeserializeTable(element);
            }
            else
            {
                return null;
            }
        }

        /// <summary>
        /// Deserialize une table
        /// </summary>
        /// <param name="element">Xelement qui contient la table</param>
        /// <returns>renvoie un array</returns>
        private object DeserializeTable(XElement element)
        {
            int dim = int.Parse(element.Attribute("Dimension").Value);
            var sizes = new int[dim];
            var bounds = element.Attribute("Bounds").Value.Split(new char[] { ',' });
            if (sizes.Length == bounds.Length)
            {
                for (int i = 0; i < sizes.Length; i++)
                {
                    sizes[i] = int.Parse(bounds[sizes.Length - 1 - i]);
                }
            }
            else
            {
                Loggers.Logger.Error("Unable to deserialize table bounds different from sizes");
            }

            var my_tab = System.Array.CreateInstance(System.Type.GetType("System.Object"), sizes);
            var queryForItem = from item in element.Elements()
                               select new
                               {
                                   Xelem = item,
                                   Object = this.DeserializeItem(item)
                               };
            foreach (var item in queryForItem)
            {
                var coord = item.Xelem.Attribute("Coord").Value.Split(new char[] { ',' }).Select(elem => int.Parse(elem)).ToArray();
                my_tab.SetValue(item.Object, coord);
            }

            return my_tab;
        }

        /// <summary>
        /// deserialize un element xml en type associé
        /// </summary>
        /// <param name="element">element xml</param>
        /// <returns>The deserialized object</returns>
        private object DeserializeFromType(XElement element)
        {
            string type = element.Attribute("TypeName").Value;
            object myObject;
            switch (type)
            {
                case "SingedByte":
                    myObject = SByte.Parse(element.Value);
                    break;
                case "UnsingedByte":
                    myObject = Byte.Parse(element.Value);
                    break;
                case "SingedShort":
                    myObject = short.Parse(element.Value);
                    break;
                case "SingedInteger":
                    myObject = Int32.Parse(element.Value);
                    break;
                case "UnsingedShort":
                    myObject = ushort.Parse(element.Value);
                    break;
                case "UnsingedInteger":
                    myObject = UInt32.Parse(element.Value);
                    break;
                case "SingedMachineInteger":
                    myObject = int.Parse(element.Value);
                    break;
                case "UnsingedMachineInteger":
                    myObject = uint.Parse(element.Value);
                    break;
                case "Decimal":
                    myObject = decimal.Parse(element.Value);
                    break;
                case "Real4":
                    myObject = float.Parse(element.Value);
                    break;
                case "Real8":
                    myObject = double.Parse(element.Value);
                    break;
                case "Bool":// Le bool.parse ne marche que sur true ou false. dans la base ici on aura une valeur numérique
                    myObject = System.Convert.ToBoolean(int.Parse(element.Value));
                    break;
                case "Currency":
                    myObject = new Currency(decimal.Parse(element.Value));
                    break;
                case "Date":
                    myObject = DateTime.Parse(element.Value);
                    break;
                case "Null":
                    myObject = null;
                    break;
                case "Empty":
                    myObject = new AspEmpty();
                    break;
                case "String":
                    myObject = element.Value;
                    break;
                default:
                    myObject = null;
                    Loggers.Logger.Error("try to deserialize unknown type");
                    break;
            }

            return myObject;
        }

        /// <summary>
        /// Création des tables de persistence dans la chaine de connexion fournie
        /// </summary>
        /// <param name="cnx_str">chaine de connexion ou créer la table </param>
        public static void CreateTables(string cnx_str)
        {
            string dir = FileUtility.GetDirectory();
            string sqlScript;
            using (TextReader tr = new StreamReader(Path.Combine(dir, "Createtables.sql")))
            {
                sqlScript = tr.ReadToEnd();
                tr.Close();
            }

            using (SqlConnection conn = new SqlConnection(cnx_str))
            {
                using (SqlCommand loadCmd = new SqlCommand())
                {
                    loadCmd.CommandText = sqlScript;
                    loadCmd.Connection = conn;
                    try
                    {
                        conn.Open();
                        loadCmd.ExecuteNonQuery();
                        conn.Close();
                    }
                    catch (Exception e)
                    {
                        Loggers.Logger.Error("Create tables failed : " + e.Message);
                        throw;
                    }
                }
            }
        }

        /// <summary>
        /// Load the session state corresponding to a session id from the database
        /// </summary>
        /// <param name="sessionId">the session id</param>
        /// <returns>the requested session state or a new one if it does not exist in the database or it is expired</returns>
        public AspSessionContents LoadSession(string sessionId)
        {
            string cnx_str;
            var resolver = new PartitionResolver();
            cnx_str = resolver.ResolvePartition(sessionId);
            AspSessionContents session = null;

            using (SqlConnection conn = new SqlConnection(cnx_str))
            {
                using (SqlCommand loadCmd = new SqlCommand())
                {
                    loadCmd.CommandText = SelectStatement;
                    loadCmd.Connection = conn;
                    loadCmd.Parameters.AddWithValue("@Id", new Guid(sessionId));
                    try
                    {
                        conn.Open();
                        using (SqlDataReader reader = loadCmd.ExecuteReader())
                        {
                            if (reader.Read())
                            { // Session has not expired => deserialize it
                                session = this.Deserialize(reader["Data"].ToString());
                                session.TimeOut = int.Parse(reader["SessionTimeOut"].ToString());
                            }
                        }
                    }
                    catch (Exception e)
                    {
                        Loggers.Logger.Error("Load session failed : " + e.Message);
                        resolver.ResetConf();
                        throw;
                    }
                }
            }

            return session;
        }

        /// <summary>
        /// Save the provided session state into the database
        /// </summary>
        /// <param name="sessionId">the session id of the session state to save</param>
        /// <param name="session">the session state to save</param>
        /// <param name="sessionTimeOut">Timeout of the session</param>
        public void SaveSession(string sessionId, AspSessionContents session, int sessionTimeOut)
        {
            string cnx_str;
            var resolver = new PartitionResolver();
            cnx_str = resolver.ResolvePartition(sessionId);

            using (SqlConnection conn = new SqlConnection(cnx_str))
            {
                using (SqlCommand saveCmd = new SqlCommand())
                {
                    saveCmd.Connection = conn;
                    saveCmd.CommandText = "BEGIN TRAN;";
                    saveCmd.CommandText += DeleteStatement;
                    saveCmd.CommandText += InsertStatement;
                    saveCmd.CommandText += "COMMIT";

                    saveCmd.Parameters.AddWithValue("@SessionTimeOut", sessionTimeOut);
                    saveCmd.Parameters.AddWithValue("@Data", this.Serialize(session));

                    saveCmd.Parameters.AddWithValue("@ID", new Guid(sessionId));
                    try
                    {
                        conn.Open();
                        saveCmd.ExecuteNonQuery();
                    }
                    catch (Exception e)
                    {
                        Loggers.Logger.Error("Save Session failed : " + e.Message);
                        resolver.ResetConf();
                        throw;
                    }
                }
            }
        }

        /// <summary>
        /// delete the provided session state into the database
        /// </summary>
        /// <param name="sessionId">the session id of the session state to save</param>
        public void DeleteSession(string sessionId)
        {
            string cnx_str;
            var resolver = new PartitionResolver();
            cnx_str = resolver.ResolvePartition(sessionId);

            using (SqlConnection conn = new SqlConnection(cnx_str))
            {
                using (SqlCommand saveCmd = new SqlCommand())
                {
                    saveCmd.Connection = conn;
                    saveCmd.CommandText = DeleteStatement;
                    saveCmd.Parameters.AddWithValue("@ID", new Guid(sessionId));
                    try
                    {
                        conn.Open();
                        saveCmd.ExecuteNonQuery();
                    }
                    catch (Exception e)
                    {
                        Loggers.Logger.Error("Delete Session failed : " + e.Message);
                        resolver.ResetConf();
                        throw;
                    }
                }
            }
        }

        /// <summary>
        /// Refresh the session into the storage
        /// </summary>
        /// <param name="sessionId">The Id of the session to refresh</param>
        public void RefreshSession(string sessionId)
        {
            string cnx_str;
            var resolver = new PartitionResolver();
            cnx_str = resolver.ResolvePartition(sessionId);

            using (SqlConnection conn = new SqlConnection(cnx_str))
            {
                using (SqlCommand saveCmd = new SqlCommand())
                {
                    saveCmd.Connection = conn;
                    saveCmd.CommandText = UpdateLastAccessedStatement;

                    saveCmd.Parameters.AddWithValue("@ID", new Guid(sessionId));
                    try
                    {
                        conn.Open();
                        saveCmd.ExecuteNonQuery();
                    }
                    catch (Exception e)
                    {
                        Loggers.Logger.Error("Update Session failed : " + e.Message);
                        resolver.ResetConf();
                        throw;
                    }
                }
            }
        }

        /// <summary>
        /// Generate a new session id
        /// </summary>
        /// <returns>the generated session id</returns>
        public string GenerateSessionId()
        {
            return Guid.NewGuid().ToString().Replace("-", string.Empty);
        }
    }
}