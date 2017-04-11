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

#pragma once

#include <STDDEF.H>
#include <comutil.h>
#include <asptlb.h>

#define LOGGING_BUFFER_SIZE 27 // 26 for _wasctime_s (see doc) and one more to replace \n by \r\n

namespace Logging
{
	/// Utility class that provide a log service. This class is a singleton class and it is thread safe.
	class Logger
	{
	private:
		static const LPCWSTR LOG_FILE;
		
		/// Handle to the log file
		HANDLE m_hLogFile;

		/// Instantiate a Logger class. This is a log utility class that writes messages to a file
		Logger(void);

		/// Destructor of the Logger class
		~Logger(void);

		/// Write a message in the log file
		void WriteInternal(
			const wchar_t* pMessage ///< [in] The message to wirte */
			);

		/// Get the current date & time as a string. The string contains an ending L"\r\n\0"
		void GetCurrentDateTimeAsString(
			wchar_t* pwcTime ///< [in, out] a buffer of 27 wchar_t where the function will write the current date time as a string
		);

		void Initialize();

		// Get the last COM error
		// @return S_OK if an error has been found and written into bstrComMessage, S_FALSE if no COM error occured
		HRESULT GetLastComError(
			_bstr_t& bstrComMessage // A bstr that recieve the last COM error message
		);

		/// Get the current running url from the current request.
		void GetCurrentUrl(
		IRequest* request, ///< [in] the current request.
		_bstr_t& bstrRawRequest ///< [out] the full current url.
		);

	public:
		/// singleton instance of the logger
		static Logger* m_sLogger;

		/// Get an instance of the singleton instance of the Logger Class
		static Logger* GetCurrent();

		/// Write an Info message in the log file
		void WriteInfo(
			const wchar_t* pMessage ///< [in] The message to wirte
			);

		/// Write an Info message in the log file
		void WriteInfo(
			const wchar_t* pMessage, ///< [in] The message to wirte
			IRequest* request ///< [in] the current request
		);

		/// Write an Debug message in the log file
		void WriteDebug(
			const wchar_t* pMessage ///< [in] The message to wirte
			);
	};
}