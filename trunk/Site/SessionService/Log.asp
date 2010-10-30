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
' Start a new test
' @param SectionName nom de la section de test
Sub WriteBeginTestSection(sectionName)
    Response.Write("<fieldset>")
    Response.Write("<h2>" & sectionName & "</h2>")
    Response.Write("<ul>")
End Sub

' Log information to display. This should be called after a WriteBeginTestSection and before a WriteEndTestSection
' @param message the message to display
Sub WriteTestLog(message)
    Response.Write("<li>")
    Response.Write(message)
    Response.Write("</li>")
End Sub

' End a test
' @param SectionName nom de la section de test
Sub WriteEndTestSection
    Response.Write("</ul>")
    Response.Write("</fieldset>")
End Sub

' Verifies that two objects, expected and actual, are equal.
' @param expected : The expected value
' @param actual The actual value
' @param failedMessage A message to display if the test failled
' @param successMessage A message to display if the test succeed
' @return : true if expected and actual are equal, false else
function Assert_AreEqual(expected, actual, failedMessage, successMessage)
    if (expected = actual) then
        Assert_AreEqual = true
    else
        Response.Write(message)
        Assert_AreEqual = false
    end if
End function
 %>