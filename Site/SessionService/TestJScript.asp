<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
    <title></title>
</head>
<body>
	<%@ Language= "Javascript" %> 
	<% 
		var Session = Server.CreateObject("AspSessionService.AspSession");

		Session("TEST") = "TEST_UPPER_CASE"
		Session("test") = "test_lower_case"
		Response.Write("1) In the following line you should see 'TEST' in upper case. If not that probably means that we have a regression of <a href=\"https://sessionservice.codeplex.com/workitem/2165\">issue 2165</a> <BR \><BR \>")
		Response.Write("TEST")
		Response.Write("<BR \><BR \> 2) Bellow you should see that Session(\"TEST\") and Session(\"test\") have the same value<BR \>")
		Response.Write("<BR \> Session(\"TEST\"): ")
		Response.Write(Session("TEST"))
		Response.Write("<BR \> Session(\"test\"): ")
		Response.Write(Session("test"))
	%> 
</body>
</html>
