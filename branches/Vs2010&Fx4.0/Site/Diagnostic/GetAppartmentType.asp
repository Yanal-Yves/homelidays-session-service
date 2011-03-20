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
<%@ Language="VBScript"%>
<%
 Response.Write("bonjour")
 
 dim AspSession
 set AspSession = server.CreateObject("YachtWebSessionServiceAsp.AspSession")
 dim nb_ref 
 nb_ref = AspSession.GetNumberOfRef()
 
 dim aptType
 dim aptQualifier
 
 aptType = AspSession.GetAppartmentType()
 aptQualifier = AspSession.GetAppartmentQualifier()
 
  response.Write("nombre de référence : " & nb_ref & "<br />")
  'response.Write("GetAppartmentType : " & aptType & "<br />")
  'response.Write("GetAppartmentQualifier : " & aptQualifier & "<br />")
  
 'Session(1) = "qslfkj"
  'Session("toto") = "prout"
  
 'set AspSession = nothing

 %>