<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
    <title>Test the reload function</title>
</head>
<body>
	<!--#include virtual="/SessionService/Init.asp"-->
	<%
		WriteBeginTestSection("Reload Session")
		AspSessionService("DataNotToBeStored") = "toto"
		AspSessionService.Reload()
		AspSessionService("DataToBeStored") = "toto2"
		WriteEndTestSection()
	%>
</body>
</html>
