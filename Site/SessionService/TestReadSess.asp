<%
 'Copyright (c) 2010 HomeAway, Inc.
 ' All rights reserved.  http://sessionservice.codeplex.com
 '
 ' Licensed under the Apache License, Version 2.0 (the "License");
 ' you may not use this file except in compliance with the License.
 ' You may obtain a copy of the License at
 '
 '	  http://www.apache.org/licenses/LICENSE-2.0
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
	<title>Page de test de l'AspSessionService</title>
	<style type="text/css">
		body{
			font-size:12px;
		}
		h2{
			margin: 0;
			font-size:12px;
		}
	</style>
</head>
<body>
	<!--#include virtual="/SessionService/Init.asp"-->
	<%
		Response.Write("Liste du contenu de AspSessionService : <br/>")
		
		WriteBeginTestSection("Liste du contenu de AspSessionService.Contents (AspSessionService n'est pas enum&eacute;rable)")
		For Each i in AspSessionService.Contents
			WriteTestLog(i)
			If( VarType(AspSessionService.Contents(i))<8192) Then
				WriteTestLog(AspSessionService.Contents(i))
		  Else
				For Each j in AspSessionService.Contents(i)
					If( VarType(j)<8192) Then
						WriteTestLog(j)
						WriteTestLog(VarType(j))
					Else
						For Each k in j   
							If( VarType(k)<8192) Then
								WriteTestLog(k)
								WriteTestLog(VarType(k))
							Else
								For Each l in k
									WriteTestLog(l)
									WriteTestLog(VarType(l))
								Next
								WriteTestLog(VarType(k))
							End If
						Next
						WriteTestLog(VarType(j))
					End If   
				Next
			End If
		  WriteTestLog(VarType(AspSessionService.Contents(i)))
		Next 
		WriteEndTestSection()

		Response.Write("<br /><b>Fin du Test</b>")
	%>
</body>
</html>