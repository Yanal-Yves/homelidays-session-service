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
	<title></title>
</head>
<body>
<%
dim a : a = 54

function Fct(b)
	b = 66
	fuc = 0
end function

Sub SubRout(c)
	c = 66
end Sub

Sub SubRout2(c, d)
	c = 66
	d= 66
end Sub

Response.Write("Fct est une fonction qui affecte 66 a son unique param&egrave;tre et renvoi toujrous 0.<br />")
Response.Write("SubRout est une Sub qui affecte 66 a son unique param&egrave;tre.<br />")
Response.Write("SubRout2 est une Sub qui affecte 66 a ses deux param&egrave;tre.<br /><br />")

Response.Write("Valeur de l'entier a : " & a & "<br />")

Response.Write("Fct(a) 'Passage par valeur" & "<br />")
Fct(a)
Response.Write("Valeur de a : " & a & "<br /><br />")

Response.Write("a = 54" & "<br />")
a = 54
Response.Write("Fct a 'Passage par r&eacute;f&eacute;rence" & "<br />")
Fct a
Response.Write("Valeur de a : " & a & "<br /><br />")

Response.Write("a = 54" & "<br />")
a = 54
Response.Write("Call Fct(a) 'Passage par r&eacute;f&eacute;rence" & "<br />")
Call Fct(a)
Response.Write("Valeur de a : " & a & "<br /><br />")

Response.Write("a = 54" & "<br />")
a = 54
Response.Write("Call SubRout(a) 'Passage par r&eacute;f&eacute;rence" & "<br />")
Call SubRout(a)
Response.Write("Valeur de a : " & a & "<br /><br />")

Response.Write("a = 54" & "<br />")
a = 54
Response.Write("SubRout(a) 'Passage par valeur" & "<br />")
SubRout(a)
Response.Write("Valeur de a : " & a & "<br /><br />")

Response.Write("a = 54" & "<br />")
a = 54
Response.Write("SubRout a 'Passage par r&eacute;f&eacute;rence" & "<br />")
SubRout a
Response.Write("Valeur de a : " & a & "<br /><br />")

Response.Write("Attention : SubRout2(a, a) ne marche pas : Cannot use parentheses when calling a Sub. ")
Response.Write("Dans SubRout(a) : les parenth&egrave;ses signifient ""passer par valeur"" et non ""voici la liste de param&egrave;tres"". ")
Response.Write("Il faut faire SubRout2(a), (a)")
%>
</body>
</html>