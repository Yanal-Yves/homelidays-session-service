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
using MbUnit.Framework;

namespace Homelidays.Web.SessionService.Tests
{
    /// <summary>
    /// Test class for AppsettingsManager 
    /// </summary>
    [TestFixture]
    public class AppSettingManagerTest
    {
        /// <summary>
        /// Test for sessiontimeout
        /// </summary>
        [Test]
        public void SessiontimeOutTest()
        {
            Assert.IsNotNull(AppSettingsManager.SessionTimeout);
        }

        /// <summary>
        /// test for cookiename
        /// </summary>
        [Test]
        public void CookieNameTest()
        {
            Assert.IsNotNull(AppSettingsManager.CookieName);
        }

        /// <summary>
        /// test for cookiekey
        /// </summary>
        [Test]
        public void CookieKeyTest()
        {
            Assert.IsNotNull(AppSettingsManager.CookieKey);
        }
    }
}
