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
        call WriteBeginTestSection("Cr&eacute;ation d'un tableau a 1 dimensions : Tab1[2]")
        Dim Tab1(2)
        Tab1(0) = "prout"
        Tab1(1) = "prout2"
        AspSessionService("Tab2") = Tab1
        WriteEndTestSection()
    
        call WriteBeginTestSection("Cr&eacute;ation d'un tableau a 2 dimensions : Tab2[4][3]")
        Dim Tab2(3,2,1)
        Tab2(0,0,0) = "A"
        Tab2(0,1,0) = "B"
        Tab2(0,2,0) = "C"
        Tab2(1,0,0) = "D"
        Tab2(1,1,0) = "E"
        Tab2(1,2,0) = "F"
        Tab2(2,0,0) = "G"
        Tab2(2,1,0) = "H"
        Tab2(2,2,0) = Tab1
        AspSessionService("Tab2") = Tab2
        WriteEndTestSection()
    
     %>
</body>
</html>