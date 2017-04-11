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

namespace Homelidays.Web.SessionService
{
    /// <summary>
    /// Class to be like the asp currency classe
    /// </summary>
    public struct Currency
    {  
        /// <summary>
        /// private decimal value of the currency
        /// </summary>
        private decimal amount;

        /// <summary>
        /// Initializes a new instance of the Currency struct with the value of the decimal inside
        /// </summary>
        /// <param name="value">the decimal inside</param>
        public Currency(decimal value)
        {
            this.amount = value;
        } 

        /// <summary>
        /// Gets or sets a decimal value of the currency
        /// </summary>
        public decimal Amount
        {
            get { return this.amount; }
            set { this.amount = value; }
        }

        /// <summary>
        /// Get a string representing the amount of money
        /// </summary>
        /// <returns>the amount of money</returns>
        public override string ToString()
        {
            return this.amount.ToString();
        }
    }
}
