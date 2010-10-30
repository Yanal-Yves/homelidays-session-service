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
<%
Dim arrayRedim() 
Dim arrayRedimPreserve() 
Dim Dico

response.Write ("DEBUT REDIM PRESERVE : "&now()&"<BR>")
for i=0 to 70000
	redim preserve arrayRedimPreserve(i+10)
	i = i+1
	for j=1 to 10
		arrayRedimPreserve(j) = "texte"&j
	next
next
response.Write ("FIN REDIM PRESERVE : "&now()&"<BR>")

response.Write ("DEBUT REDIM : "&now()&"<BR>")
for i=0 to 70000
	redim arrayRedim(i+10)
	i = i+1
	for j=1 to 10
		arrayRedim(j) = "texte"&j
	next
next
response.Write ("FIN REDIM : "&now()&"<BR>")
response.Flush

response.Flush
response.Write ("DEBUT DICO NEW : "&now()&"<BR>")
for i=0 to 70000
	set Dico = Server.CreateObject("Scripting.Dictionary")
	call Dico.Add(i,"texte"&i)
next
response.Write ("FIN DICO NEW : "&now()&"<BR>")

response.Flush
response.Write ("DEBUT DICO ADD : "&now()&"<BR>")
set Dico = Server.CreateObject("Scripting.Dictionary")
for i=0 to 70000
	call Dico.Add(i,"texte"&i)
next
response.Write ("FIN DICO ADD : "&now()&"<BR>")

response.Write ("DEBUT REDIM PRESERVE : "&now()&"<BR>")
redim preserve arrayRedimPreserve(300000)
for i=0 to 300000
	arrayRedimPreserve(i) = "texte"&i
	i = i+1
next
response.Write ("FIN REDIM PRESERVE : "&now()&"<BR>")

response.Write ("DEBUT REDIM PRESERVE : "&now()&"<BR>")
for i=0 to 300000
	redim preserve arrayRedimPreserve(i)
	arrayRedimPreserve(i) = "texte"&i
	i = i+1
next
response.Write ("FIN REDIM PRESERVE : "&now()&"<BR>")

%>

