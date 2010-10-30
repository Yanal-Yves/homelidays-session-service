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
<!--#include virtual="/SessionService/Log.asp"-->
<%
Response.Write("<b>D&eacute;but du Test</b><br /><br />")

' On instancie le composant COM
call WriteBeginTestSection("Instanciation de AspSessionService.AspSession")
%>
<!--#include virtual="/SessionService/AspSessionServiceInit.asp"-->
<%
call WriteTestLog("Instanciation termin&eacute;e")
call WriteEndTestSection()
%>