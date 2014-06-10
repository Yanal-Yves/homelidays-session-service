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
	<!--#include virtual="/SessionService/Init.asp"-->
	<%
		Call WriteBeginTestSection("Test de Scripting.Dictionary en Session")
		Set ScriptDico = CreateObject("Scripting.Dictionary")
		ScriptDico.Add "Key1", 12
		Set AspSessionService.Contents("ScriptingDico") = ScriptDico
		call WriteTestLog("Set AspSessionService.Contents(""ScriptingDico"") = ScriptDico")
		Set AspSessionService("ScriptingDico2") = ScriptDico
		call WriteTestLog("Set AspSessionService(""ScriptingDico2"") = ScriptDico")
		WriteEndTestSection()

		Call WriteBeginTestSection("Test de Scripting.Dictionary dans un tableau une dimension en Session")
		Set ScriptDico = CreateObject("Scripting.Dictionary")
		ScriptDico.Add "Key1", 12
		ScriptDico.Add "Key2", "StringValue"
		Dim Personnes(1)
		Set Personnes(0)= ScriptDico
		Personnes(1)="Joe"
		call WriteTestLog("AspSessionService(""Key1"") = ""Test""")
		AspSessionService("Key1") = "Test"
		call WriteTestLog("AspSessionService(""Key1"") = ""Personnes"" (Perssonnes est un tableau)")
		AspSessionService("TableOfScriptingDico1") = Personnes
		Set ScriptDico = Nothing
		WriteEndTestSection()

		Call WriteBeginTestSection("Test de Scripting.Dictionary dans un tableau a deux dimensions en Session")
		Set ScriptDico = CreateObject("Scripting.Dictionary")
		ScriptDico.Add "Key1", 12
		ScriptDico.Add "Key2", "StringValue"
		Dim Personnes2(1, 2)
		Set Personnes2(1, 0)= ScriptDico
		Personnes2(0, 1)="Joe"
		call WriteTestLog("AspSessionService(""TableOfScriptingDico2"") = ""Personnes2"" (Perssonnes est un tableau)")
		AspSessionService("TableOfScriptingDico2") = Personnes2
		Set ScriptDico = Nothing
		WriteEndTestSection()

		Call WriteBeginTestSection("Test d'un tableau dans un Scripting.Dictionary")
		Dim Personnes3(1, 2)
		Personnes3(1, 0) = 13
		Personnes3(0, 1) = "Joe"
		Set ScriptDico = CreateObject("Scripting.Dictionary")
		ScriptDico.Add "Key1", 12
		ScriptDico.Add "Key2", "StringValue"
		ScriptDico.Add "Personnes2", Personnes2
		ScriptDico.Add "Personnes3", Personnes3
		call WriteTestLog("AspSessionService(""ScriptingDicoOfTable"") = ""ScriptDico"" (ScriptDico est un Scripting Dictionary)")
		Set AspSessionService("ScriptingDicoOfTable") = ScriptDico
		WriteEndTestSection()

		Call WriteBeginTestSection("Test de Nothing dans la session")
		call WriteTestLog("AspSessionService(""KeyNothing"") = nothing")
		Set AspSessionService.Contents("KeyNothing") = nothing
		WriteEndTestSection()

		Call WriteBeginTestSection("Test de Nothing dans la session")
		Set Pco = Server.CreateObject("Scripting.Dictionary")
    	Pco.CompareMode = VBBinaryCompare
    	Pco.item("PcoKey1") = "PcoValue1"
		Pco.item("PcoKey2") = "PcoValue2"
		call WriteTestLog("AspSessionService(""PcoKeys"") = Pco.Keys (tableau de clef d'un Scripting.Dictionary)")
		call WriteTestLog("AspSessionService(""PcoValues"") = Pco.Items (tableau de valeurs d'un Scripting.Dictionary)")
		AspSessionService("PcoKeys") = Pco.Keys
		AspSessionService("PcoValues") = Pco.Items
		call WriteTestLog("AspSessionService(""PcoValues"")(0) : " & AspSessionService("PcoValues")(0))
		WriteEndTestSection()

		Response.Write("<br /><b>Fin du Test</b>")
	%>
</body>
</html>