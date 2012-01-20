<%
Dim AspSessionService
Set AspSessionService = Server.CreateObject("AspSessionService.AspSession")
AspSessionService("keyChildPage") = "Set in the child page"
%>
