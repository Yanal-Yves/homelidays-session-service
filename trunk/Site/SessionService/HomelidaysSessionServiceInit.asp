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
	Dim Session
	Set Session = Server.CreateObject("AspSessionService.AspSession")

	' Vérification de la présence de 'LastAccessed' dans le cookie Yacht
	Dim DateLastCookieAccess : DateLastCookieAccess = Request.Cookies("Yacht")("LastAccessed") ' The date stored by the session service should not be more than 23 characters

	If DateLastCookieAccess <> "" Then
		' Les champs MinutesSinceLastAccess et HoursSinceLastAccess correspondent aux minutes et heures depuis le dernier accès au cookie
		Dim MinutesSinceLastAccess
		Dim HoursSinceLastAccess

		' Parsing de la date récupérée dans le cookie
		DateLastCookieAccessParsed = ParseDateLastAccess(DateLastCookieAccess)

		' Récupération de la date actuelle en temps UTC
		DateUTC = GetSystemUtcTime()

		' On renseigne les champs MinutesSinceLastAccess et HoursSinceLastAccess grâce à la méthode suivante
		TimeSpan DateLastCookieAccessParsed, DateUTC

		' Get the timeout from the cookie
		Dim SessionTimeOut : SessionTimeOut = Request.Cookies("Yacht")("TimeOut") ' Le timeout est un int donc au max du max 10 digits

		If SessionTimeOut <> "" Then
			Dim SlidingUpperBound : SlidingUpperBound = SessionTimeOut + (SessionTimeOut / 2)
			Dim SlidingLowerBound : SlidingLowerBound = SessionTimeOut / 2

			' Pour raffraichir la date dans le cookie et la base il faut que le temps de session active soit entre 10 et 30 minutes
			If HoursSinceLastAccess = 0 Then
				If MinutesSinceLastAccess > SlidingLowerBound And MinutesSinceLastAccess < SlidingUpperBound Then
					' Ici volontairement on fait appel à la Session pour que le SessionService remette à jour la date du cookie et que la BDD soit mise à jour
					Dim valeurTemp : valeurTemp = Session("SessionId")
				End If
			End If
		End If
	End If

	' Méthode qui renvoie la date actuelle en format GMT
	Function GetSystemUtcTime()
		Dim oShell
		set oShell = CreateObject("WScript.Shell")
		Dim registry_path : registry_path = "HKEY_LOCAL_MACHINE\System\CurrentControlSet\Control\TimeZoneInformation\ActiveTimeBias"
		Dim offsetMin : offsetMin = oShell.RegRead(registry_path)
		Dim date_time_now : date_time_now = now()
		Dim date_time_now_utc : date_time_now_utc = dateadd("n", offsetMin, date_time_now)

		GetSystemUtcTime = date_time_now_utc
	End Function

	' Méthode qui renseigne le nombre de minutes et le nombre d'heures entre les 2 dates données en param
	Function TimeSpan(dt1, dt2)
		If (isDate(dt1) And IsDate(dt2)) = false Then
			Exit Function
		End If

		seconds = DateDiff("S", dt1, dt2)

		' Il faut obligatoirement que dt2 soit supérieur à dt1
		If Abs(seconds) <> seconds then
			 Exit Function
		End If

		minutes = seconds \ 60
		HoursSinceLastAccess = minutes \ 60
		MinutesSinceLastAccess = minutes mod 60
	End Function

	' Fonction qui parse la date dans le cookie en un format date standard
	Function ParseDateLastAccess(dateFromCookie)
		'On parse la date de cookie qui a cette forme : 2010-08-27T12:42Z pour la transformer en 2010-08-27 12:42:00
		Dim lastAccessDay : lastAccessDay = Left(dateFromCookie,10)
		Dim lastAccessHour : lastAccessHour = Mid(dateFromCookie,12,2)
		Dim lastAccessMinute : lastAccessMinute = Mid(dateFromCookie,15,2)

		ParseDateLastAccess = lastAccessDay & " " & lastAccessHour & ":" & lastAccessMinute  & ":00"
	End Function
%>