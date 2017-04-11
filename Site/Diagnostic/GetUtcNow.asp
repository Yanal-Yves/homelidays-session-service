<%@ Language="VBScript"%>
<%
 'Copyright (c) 2010 HomeAway, Inc.
 ' All rights reserved.  http://sessionservice.codeplex.com
 '
 ' Licensed under the Apache License, Version 2.0 (the "License");
 ' you may not use this file except in compliance with the License.
 ' You may obtain a copy of the License at
 '
 '      http://www.apache.org/licenses/LICENSE-2.0
 '
 ' Unless required by applicable law or agreed to in writing, software
 ' distributed under the License is distributed on an "AS IS" BASIS,
 ' WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 ' See the License for the specific language governing permissions and
 ' limitations under the License.
%>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
    <title>
    </title>
</head>
<body>
<div>
<% 
	' Retrieves the current system date and time. The system time is expressed in Coordinated Universal Time (UTC).
	' Returns the current system date and time in UTC.
	Function GetSystemUtcTime()
		Dim oShell
		set oShell = CreateObject("WScript.Shell")
		Dim registry_path : registry_path = "HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\TimeZoneInformation\ActiveTimeBias"
		Dim offsetMin : offsetMin = oShell.RegRead(registry_path)
		Dim date_time_now : date_time_now = now()
		Dim date_time_now_utc : date_time_now_utc = dateadd("n", offsetMin, date_time_now)

		GetSystemUtcTime = date_time_now_utc
	End Function

    Response.Write("Registry Key where to read the timezone offset: ")
    Response.Write("HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\TimeZoneInformation\ActiveTimeBias <br />")
    Dim utc_now : utc_now = GetSystemUtcTime
    Response.Write("Current = " & now() & "<br />UTC = " & utc_now)
%>
</div>
</body>
</html>