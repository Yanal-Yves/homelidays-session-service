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
using Homelidays.Web.SessionService;
using MbUnit.Framework;

namespace Homelidays.Web.SessionService.Tests
{
    /// <summary>
    /// Test class for PartitionResolver
    /// </summary>
    [TestFixture]
    public class PartitionResolverTest
    {
        /// <summary>
        /// test the creation of an instance of partitionresolver
        /// </summary>
        [Test]
        public void PartitionResolverCtorTest()
        {
            PartitionResolver pr = new PartitionResolver();
        }

        /// <summary>
        /// test the resolvepartition
        /// </summary>
        [Test]
        public void PartitionResolverResolvePartitionTest()
        {
            PartitionResolver pr = new PartitionResolver();
            SessionPersistence persist = new SessionPersistence();
            pr.ResolvePartition(persist.GenerateSessionId());
        }
    }
}
