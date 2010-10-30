/* Copyright (c) 2010 HomeAway, Inc.
 * All rights reserved.  http://sessionservice.codeplex.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "stdafx.h"
#include "..\AspSessionService\Logger.h"
#include "..\AspSessionService\AspSessionSerializer.h"
#include "..\AspSessionService\AspSessionDeserializer.h"
#include "..\AspSessionService\PartitionResolver.h"
#include "..\AspSessionService\AspSessionPersistence.h"
#include "..\AspSessionService\Logger.h"
#include "comutil.h"
#include <map> // for map

using namespace Logging;
#if _DEBUG
int const THREAD_NB = 1; ///< Number of thread to start for the test
#else
int const THREAD_NB = 100; ///< Number of thread to start for the test
#endif

DWORD ThreadId[THREAD_NB]; ///< Thread ids
HANDLE hThread[THREAD_NB]; ///< Thread handles

/// La méthode LoggerTest lance plusieurs thread qui execute tous cette méthode
DWORD WINAPI LoggerTestThreadProc(
	LPVOID param ///< [in] le numéro d'ordre de lancement du thread
	)
{
	int const NB_WCHAR = 30; // Taille du message que l'on écrit dans le fichier
	wchar_t msg[NB_WCHAR]; 
	DWORD * pthread_num = (DWORD*)param;
	swprintf_s(msg, NB_WCHAR, L"NB thread : %d \r\n", *pthread_num); // /!\ Changement de msg => changement de NB_WCHAR
	free(pthread_num);

	for (;;)
	{
		Logging::Logger *log;
		log = Logging::Logger::GetCurrent();
		if (log != NULL)
		{
			log->WriteInfo(msg);
			log->WriteInfo(msg, NULL); // Not easy to have a IRequest Com object for testing
			log->WriteDebug(msg);
		}
		else
		{
			perror("_tmain - Logging::Logger::GetCurrent() failled");
		}
	}

	return 0;
}

/// Test la class logger
int LoggerTest()
{
	for (DWORD i = 0 ; i < THREAD_NB ; i++)
	{ // Create test threads
		DWORD* pThreadId = &(ThreadId[i]);
		DWORD* lpParam = new DWORD(i);
		hThread[i] = CreateThread(NULL, 0, LoggerTestThreadProc, lpParam, CREATE_SUSPENDED, pThreadId);
		if (hThread == NULL)
		{
			perror("LoggerTest - CreateThread failled");
			return 1;
		}
		else
		{
			ResumeThread(hThread[i]);
		}
	}

	for (DWORD i = 0 ; i < THREAD_NB ; i++)
	{ // Wait for each thread to finish
		WaitForSingleObject(hThread[i], INFINITE);
	}

	for (DWORD i = 0 ; i < THREAD_NB ; i++)
	{ // Close every single handle used for threads
		CloseHandle(hThread[i]);
	}

	return 0;
}

/// La méthode LoggerTest lance plusieurs thread qui execute tous cette méthode
DWORD WINAPI SerializerTestThreadProc(
	LPVOID param ///< [in] le numéro d'ordre de lancement du thread
	)
{
	ContainerType dico;

	// Empty
	_variant_t emptyVariant;
	_bstr_t bstrKey = L"KeyEmpty";
	emptyVariant.vt = VT_EMPTY;
	dico[bstrKey] = emptyVariant;

	// Integer
	_variant_t lVariant;
	bstrKey = L"KeyInteger";
	lVariant.vt = VT_I4;
	lVariant.lVal = (long) 12;
	dico[bstrKey] = lVariant;

	// Date
	_variant_t dVariant;
	bstrKey = L"KeyDate";
	dVariant.vt = VT_DATE;
	dVariant.date = 2.55;
	dico[bstrKey] = dVariant;

	// String 
	_variant_t sVariant = L"A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.A very long string.";
	bstrKey = L"KeyString";
	dico[bstrKey] = sVariant;

	// Double
	_variant_t doubleVariant;
	bstrKey = L"KeyDouble";
	doubleVariant.vt = VT_R8;
	doubleVariant.dblVal = 3.342;
	dico[bstrKey] = doubleVariant;

	// 1 dimension Table
	SAFEARRAYBOUND psabBounds[1];
	psabBounds[0].cElements = 2; // 2 elements
	psabBounds[0].lLbound = 0; // first element index is 0 in the safearray
	USHORT usDim = 1; // number of dimensions
	CComSafeArray<VARIANT> pSar = CComSafeArray<VARIANT>(psabBounds,usDim);
	_variant_t tableVariant;
	tableVariant.vt = VT_VARIANT | VT_ARRAY;
	tableVariant.parray = *pSar.GetSafeArrayPtr();
	bstrKey = L"KeyTable";
	dico[bstrKey] = tableVariant;

	for (;;)
	{
		CString serializedDico;
		AspSessionSerializer serializer;
		HRESULT hr;
		hr = serializer.SerializeSession(dico, serializedDico);
		if (hr != S_OK)
		{
			throw L"serializer.SerializeSession failed";
		}
	}

	return 0;
}

/// Test the serialization
/// @return 0 if the test succeed, 1 if it failed
int SerializerTest()
{
	for (DWORD i = 0 ; i < THREAD_NB ; i++)
	{ // Create test threads
		DWORD* pThreadId = &(ThreadId[i]);
		DWORD* lpParam = new DWORD(i);
		hThread[i] = CreateThread(NULL, 0, SerializerTestThreadProc, lpParam, CREATE_SUSPENDED, pThreadId);
		if (hThread == NULL)
		{
			perror("SerializerTest - CreateThread failled");
			return 1;
		}
		else
		{
			ResumeThread(hThread[i]);
		}
	}

	for (DWORD i = 0 ; i < THREAD_NB ; i++)
	{ // Wait for each thread to finish
		WaitForSingleObject(hThread[i], INFINITE);
	}

	for (DWORD i = 0 ; i < THREAD_NB ; i++)
	{ // Close every single handle used for threads
		CloseHandle(hThread[i]);
	}

	return 0;
}

/// La méthode LoggerTest lance plusieurs thread qui execute tous cette méthode
DWORD WINAPI DeserializerTestThreadProc(
	LPVOID param ///< [in] le numéro d'ordre de lancement du thread
	)
{
	ContainerType dico;

	for (;;)
	{
		_variant_t deserializedDico = 
			L"<?xml version=\"1.0\"?>\
			<Session>\
			  <Item Key=\"Key3\" TypeName=\"Real8\">3,23454</Item>\
			  <Table Key=\"Tab2\" TypeName=\"Table\" Dimension=\"2\" Bounds=\"4,3\">\
					<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"0,0\"><![CDATA[A]]></Item>\
					<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"1,0\"><![CDATA[E]]></Item>\
					<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"2,0\"><![CDATA[I]]></Item>\
					<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"0,1\"><![CDATA[B]]></Item>\
					<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"1,1\"><![CDATA[F]]></Item>\
					<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"2,1\"><![CDATA[]]></Item>\
					<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"0,2\"><![CDATA[C]]></Item>\
					<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"1,2\"><![CDATA[G]]></Item>\
					<Table Key=\"Tab2\" TypeName=\"Table\" Dimension=\"1\" Bounds=\"4\" Coord=\"2,2\">\
						<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"0\"><![CDATA[Yanal]]></Item>\
						<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"1\"><![CDATA[Joe]]></Item>\
						<Item Key=\"Tab2\" TypeName=\"SingedShort\" Coord=\"2\">16</Item>\
						<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"3\"><![CDATA[Simon]]></Item>\
					</Table>\
					<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"0,3\"><![CDATA[D]]></Item>\
					<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"1,3\"><![CDATA[H]]></Item>\
					<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"2,3\"><![CDATA[]]></Item>\
			  </Table>\
			</Session>";
		AspSessionDeserializer deserializer(dico, deserializedDico);
		deserializer.DeserializeSession();

		// Test double
		variant_t key3_variant = dico["Key3"];
		double test_double = 3.23454;
		if (key3_variant.dblVal != test_double)
		{
			throw new CString(L"Key 3 n'a pas pu être relu correctement");
		}

		// Test tableau mutli-dimension
		variant_t tab2_variant = dico["Tab2"];
		USHORT us_dim = tab2_variant.parray->cDims;
		if (us_dim != 2)
		{
			throw new CString(L"Tab2 devrait être de dimension 2");
		}

		LONG plCoord[2] = {2, 0};
		variant_t variant_value;
		SafeArrayGetElement(tab2_variant.parray, plCoord, &variant_value);
		CString str_value = variant_value.bstrVal;
		if (str_value != L"I")
		{
			throw new CString(L"Tab2[2, 0] devrait être I");
		}

		LONG plCoord2[2] = {2, 2};
		variant_t variant_tab;
		SafeArrayGetElement(tab2_variant.parray, plCoord2, &variant_tab);
		LONG plCoord3[1] = {1};
		variant_t variant_value2;
		SafeArrayGetElement(variant_tab.parray, plCoord3, &variant_value2);
		CString str_value2 = variant_value2.bstrVal;
		if(str_value2 != "Joe")
		{
			throw new CString(L"Tab2[2, 2][1] devrait être Joe");
		}

		dico.clear();
	}

	return 0;
}

/// Test de la désrialization
int DeserializerTest()
{
	for (DWORD i = 0 ; i < THREAD_NB ; i++)
	{ // Create test threads
		DWORD* pThreadId = &(ThreadId[i]);
		DWORD* lpParam = new DWORD(i);
		hThread[i] = CreateThread(NULL, 0, DeserializerTestThreadProc, lpParam, CREATE_SUSPENDED, pThreadId);
		if (hThread == NULL)
		{
			perror("SerializerTest - CreateThread failled");
			return 1;
		}
		else
		{
			ResumeThread(hThread[i]);
		}
	}

	for (DWORD i = 0 ; i < THREAD_NB ; i++)
	{ // Wait for each thread to finish
		WaitForSingleObject(hThread[i], INFINITE);
	}

	for (DWORD i = 0 ; i < THREAD_NB ; i++)
	{ // Close every single handle used for threads
		CloseHandle(hThread[i]);
	}

	return 0;
}

/// La méthode DeserializeSerialiseTest lance plusieurs thread qui execute tous cette méthode
DWORD WINAPI DeserializerSerializerTestThreadProc(
	LPVOID param ///< [in] le numéro d'ordre de lancement du thread
	)
{
	ContainerType dico;

	for (;;)
	{
		_variant_t deserializedDico = 
			L"<?xml version=\"1.0\"?>\
<Session>\
<Item Key=\"Key3\" TypeName=\"Real8\">3,23454</Item>\
<Table Key=\"Tab2\" TypeName=\"Table\" Dimension=\"2\" Bounds=\"4,3\">\
<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"0,0\"><![CDATA[A]]></Item>\
<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"1,0\"><![CDATA[E]]></Item>\
<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"2,0\"><![CDATA[I]]></Item>\
<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"0,1\"><![CDATA[B]]></Item>\
<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"1,1\"><![CDATA[F]]></Item>\
<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"2,1\"><![CDATA[]]></Item>\
<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"0,2\"><![CDATA[C]]></Item>\
<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"1,2\"><![CDATA[G]]></Item>\
<Table Key=\"Tab2\" TypeName=\"Table\" Dimension=\"1\" Bounds=\"4\" Coord=\"2,2\">\
<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"0\"><![CDATA[Yanal]]></Item>\
<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"1\"><![CDATA[Joe]]></Item>\
<Item Key=\"Tab2\" TypeName=\"SingedShort\" Coord=\"2\">16</Item>\
<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"3\"><![CDATA[Simon]]></Item>\
</Table>\
<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"0,3\"><![CDATA[D]]></Item>\
<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"1,3\"><![CDATA[H]]></Item>\
<Item Key=\"Tab2\" TypeName=\"String\" Coord=\"2,3\"><![CDATA[]]></Item>\
</Table>\
<Item Key=\"bon_commande\" TypeName=\"String\">\
<![CDATA[ <b><span class=\"bleuc\"> \
 <translate>BON DE COMMANDE, valable 30 jours</translate>  \
 </span></b> \
 <br>\
 <b><span class=\"bleuc\"> \
 <translate>Offre locative</translate> n° 271668\
 </span></b> \
 <br><br>\
<center>\
<table border=\"0\" cellPadding=\"1\" cellSpacing=\"1\" width=\"95%\" bordercolor=LightSkyBlue> \
	<tr>\
		<td width=\"50%\">\
			<span class=\"violetbg\">\
			<b> <translate>Référence :</translate> C-1008-1386175</b>\
			</span>\
		</td >\
		<td>\
			<span class=\"violetbg\">\
			<b><translate>Le</translate> 04/08/2010</b>\
			</span>\
		</td>\
	</tr>\
	<tr>\
		<td>\
			 \
		</td>\
		<td>\
			 \
		</td>\
	</tr>\
	<tr>\
		<td>\
			<span class=\"bleu\">\
			<b>HOMELIDAYS</b>\
			</span>\
		</td>	\
		<td>\
			<span class=\"bleu\">\
			<b>Monsieur FARGIALLA Yanal-Yves</b> \
			</span>\
		</td>\
	</tr>\
	<tr>\
		<td>\
			<span class=\"bleu\">\
			<translate>Service propriétaires</translate>\
			</span>\
		</td>\
		<td>\
			<span class=\"bleu\">\
			 \
			</span>\
		</td>\
	</tr>\
	<tr>\
		<td>\
			<span class=\"bleu\">\
			47 bis rue des Vinaigriers\
			</span>\
		</td>\
		<td>\
			<span class=\"bleu\">\
			37 rue de Sablonville\
			</span>\
		</td>\
	</tr>\
	<tr>\
		<td>\
			<span class=\"bleu\">\
			75010 PARIS\
			</span>\
		</td>	\
		<td>\
			<span class=\"bleu\">\
78510 Triel Sur Seine\
			</span>\
		</td>\
	</tr>\
	<tr>\
		<td>\
			<span class=\"bleu\">\
			FRANCE\
			</span>\
		</td>\
		<td>\
			<span class=\"bleu\">\
FRANCE\
			</span>\
		</td>\
	</tr>\
	<tr>\
		<td>\
			<span class=\"bleu\">\
			<TRANSLATE>N° de TVA intracommunautaire :</TRANSLATE>\
			</span>\
		</td>\
		<td>\
		</td>\
	</tr>\
	<tr>\
		<td>\
			<span class=\"bleu\">\
			<b>FR60432287209</b>\
			</span>\
		</td>\
		<td>\
		</td>\
	</tr>\
	<tr>\
		<td>\
			 \
		</td>\
		<td>\
			 \
		</td>\
	</tr>\
	<tr>\
		<td>\
			<span class=\"bleu\">\
			<a href=\"http://www.homelidays.com/FR-Locations-Vacances/340_Contact/formulaireContact.asp\">Contact service clientèle</a>\
			</span>\
		</td>\
		<td>\
			<span class=\"bleu\">\
			<translate>E-mail :</translate> yanal_yves_fargialla@hotmail.com\
			</span>\
		</td>\
	</tr>\
	<tr>\
		<td>\
			<span class=\"bleu\">\
			<translate>Tél :</translate> <translate>+33 (0)1 70 75 34 00</translate>\
			</span>\
		</td>\
		<td>\
			<span class=\"bleu\">\
			<translate>Tél :</translate> +33139276106\
			</span>\
		</td>\
	</tr>\
	<tr>\
		<td>\
			<span class=\"bleu\">\
			<translate>Fax :</translate> +33 (0)1 40 36 59 33\
			</span>\
		</td>\
		<td>\
			<span class=\"bleu\">\
			<translate>Portable :</translate> \
			</span>\
		</td>\
	</tr>\
</table>\
</center>\
<br><br>\
<form action=\"\" id=\"RecapCommande\" method=\"post\" name=\"RecapCommande\" >\
<input type=\"hidden\" name=\"PageAppelante\" value=\"AdForm-RecCmd.asp\">\
<input type=\"hidden\" name=\"PK\" value='271668'>\
<input type=\"hidden\" name=\"Fk_loueur\" value='3791515'>\
<input type=\"hidden\" name=\"Date\" value='04/08/2010'>\
<input type=\"hidden\" id=\"PageARenvoyer\" name=\"PageARenvoyer\" value=\"AdForm-Recording.asp\">\
<input type=\"hidden\" id=\"PageARenvoyerBAD\" name=\"PageARenvoyerBAD\" value=\"AdForm-FormulaSelection.asp\">\
<input type=\"hidden\" name=\"origine\" value=\"\">\
<input type=\"hidden\" name=\"statut\" value=\"006\">	\
<input type=\"hidden\" name=\"NoModule\" value=0>	\
<input type=\"hidden\" name=\"SourceUrl\" value=\"http://my.homelidays.com/Telesales/Auth/Telesales/PropertySearch.aspx?PtyId=271668\">	\
<span class=\"violetbg\"> \
<b><translate>Détail de votre commande :</translate></b></span> \
<br><br> \
<center>\
<table border=\"1\" cellPadding=\"1\" cellSpacing=\"1\" width=\"95%\" bordercolor=\"LightSkyBlue\">\
	<tr>\
		<th>\
			<span class=\"bleu\">\
			<translate>Descriptif</translate>\
		</th>\
		<th>\
			<span class=\"bleu\">\
			<translate>Prix unitaire</translate>  \
		</th>\
		<th>\
			<span class=\"bleu\">\
			<translate>Quantité</translate>  \
		</th>\
		<th>\
			<span class=\"bleu\">\
			<translate>Prix TTC</translate>\
		</th>\
	</tr>\
		<input type=\"hidden\" name=\"SERVICE.FORMULE\" value=\"PACK_DECOUVERTE_10\">\
				<tr> \
					<td> \
						<span class=\"bleu\"> \
						 \
Formule Découverte - 4 mois\
						</span>\
					</td>\
					<td align=\"right\">\
						<span class=\"bleu\">\
149.00 €\
						 \
						</span>\
					</td>	\
					<td align=\"right\">\
						<span class=\"bleu\">\
1\
						  \
						</span>\
					</td>\
					<td align=\"right\">\
						<span class=\"bleu\">\
149.00 €\
						  \
						</span>\
					</td>\
				</tr>	\
	<input type=\"hidden\" name=\"intitule\" value=\"C-1008-1386175\">\
	<tr>\
		<td colspan=\"3\" align=\"right\">	\
			<span class=\"violetbg\">\
			<b><translate>TOTAL TTC</translate></b>\
			</span>\
		</td>\
		<td align=\"right\">\
			<font color=\"red\" size=\"3\">\
			<b>\
\
			</b> \
			</font>\
			<input type=\"hidden\" name=\"totalGeneral\" value=\"\">\
		</td>\
	</tr>	\
	<tr>\
		<td colspan=\"3\" align=\"right\">	\
			<span class=\"bleu\">\
			<translate>Dont TVA</translate> (19,6 %) \
			</span>\
		</td>\
		<td align=\"right\">\
			<span class=\"bleu\">\
0.00 €\
			 \
			</span>			\
		</td>		\
	</tr>	\
	<tr>\
		<td colspan=\"3\" align=\"right\">\
			<span class=\"bleu\">\
			<translate>Total HT</translate> \
			</span>\
		</td>\
		<td align=\"right\">\
			<span class=\"bleu\">\
0.00 €\
			 	\
			</span>\
		</td>	\
	</tr>	\
</table>\
</center>\
]]>\
</Item>\
</Session>";
		HRESULT hr;
		AspSessionDeserializer deserializer(dico, deserializedDico);
		hr = deserializer.DeserializeSession();
		if(FAILED(hr))
		{
			perror("Deserialiation failled");
			throw new CString(L"Deserialiation failled");
		}

		CString strSerializedDico;
		AspSessionSerializer serializer;
		hr = serializer.SerializeSession(dico, strSerializedDico);
		if(FAILED(hr))
		{
			perror("Serialiation failled");
			throw new CString(L"Serialiation failled");
		}

		CString strExpected = deserializedDico.bstrVal;
		if (strSerializedDico != strExpected)
		{
			perror("DeserializerSerializerTestThreadProc : L'XML sérialisé est différent de l'XML fourni au désérialisateur");
			throw new CString(L"DeserializerSerializerTestThreadProc : L'XML sérialisé est différent de l'XML fourni au désérialisateur");
		}

		dico.clear();
	}
}

/// Test de la désrialization et sérialisation.
/// Au cours de ce test on désérialize un fichier XML, puis on sérialise à nouveau la collection,
/// enfin on vérifie que l'XML sérialisé est le même que l'XML de départ.
int DeserializerSerialiserTest()
{
	for (DWORD i = 0 ; i < THREAD_NB ; i++)
	{ // Create test threads
		DWORD* pThreadId = &(ThreadId[i]);
		DWORD* lpParam = new DWORD(i);
		hThread[i] = CreateThread(NULL, 0, DeserializerSerializerTestThreadProc, lpParam, CREATE_SUSPENDED, pThreadId);
		if (hThread == NULL)
		{
			perror("SerializerTest - CreateThread failled");
			return 1;
		}
		else
		{
			ResumeThread(hThread[i]);
		}
	}

	for (DWORD i = 0 ; i < THREAD_NB ; i++)
	{ // Wait for each thread to finish
		WaitForSingleObject(hThread[i], INFINITE);
	}

	for (DWORD i = 0 ; i < THREAD_NB ; i++)
	{ // Close every single handle used for threads
		CloseHandle(hThread[i]);
	}

	return 0;
}

/// La méthode PartitionResolverTest lance plusieurs thread qui execute tous cette méthode
DWORD WINAPI PartitionResolverTestThreadProc(
	LPVOID param ///< [in] le numéro d'ordre de lancement du thread
	)
{
	HRESULT hr;
	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	_variant_t vtGuid = L"690C3A0C-CBF0-4427-A6B4-0F23B3352DA0";
	Partition partition;

	for(;;)
	{
		PartitionResolver::GetPartitionn(vtGuid, partition);
	}

	// Uninitialize COM
	CoUninitialize();

	return 0;
}

/// Test du PartitionResolver.
int PartitionResolverTest()
{
	// initialize the singleton object
	PartitionResolver partitionResolver;

	for (DWORD i = 0 ; i < THREAD_NB ; i++)
	{ // Create test threads
		DWORD* pThreadId = &(ThreadId[i]);
		DWORD* lpParam = new DWORD(i);
		hThread[i] = CreateThread(NULL, 0, PartitionResolverTestThreadProc, lpParam, CREATE_SUSPENDED, pThreadId);
		if (hThread == NULL)
		{
			perror("PartitionResolver - CreateThread failled");
			return 1;
		}
		else
		{
			ResumeThread(hThread[i]);
		}
	}

	for (DWORD i = 0 ; i < THREAD_NB ; i++)
	{ // Wait for each thread to finish
		WaitForSingleObject(hThread[i], INFINITE);
	}

	for (DWORD i = 0 ; i < THREAD_NB ; i++)
	{ // Close every single handle used for threads
		CloseHandle(hThread[i]);
	}

	return 0;
}

DWORD WINAPI SessionPersistenceTestThreadProc(
	LPVOID param ///< [in] le numéro d'ordre de lancement du thread
	)
{
	//comme on est pas dans le contexte de IIS on doit initialiser Com
	HRESULT hr;
	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	_variant_t vtGuid = L"690C3A0C-CBF0-4427-A6B4-0F23B3352DA0";
	_variant_t LastAccessed;
	_variant_t Data= L"<TEST>value</TEST>";
	_variant_t Timeout = 20;
	AspSessionPersistence SessPersistence;

	for(;;)
	{
		hr= SessPersistence.SetSession(vtGuid, Timeout, Data);
		hr = SessPersistence.GetSession(vtGuid, LastAccessed, Timeout, Data);
		hr= SessPersistence.DeleteSession(vtGuid);
	}

	// Uninitialize COM
	CoUninitialize();
	
	return 0;
}


/// Test de la persistence de la session dans la base de données
int SessionPersistenceTest()
{
	// initialize the singleton object
	PartitionResolver partitionResolver;

	for (DWORD i = 0 ; i < THREAD_NB ; i++)
	{ // Create test threads
		DWORD* pThreadId = &(ThreadId[i]);
		DWORD* lpParam = new DWORD(i);
		hThread[i] = CreateThread(NULL, 0, SessionPersistenceTestThreadProc, lpParam, CREATE_SUSPENDED, pThreadId);
		if (hThread == NULL)
		{
			perror("PartitionResolver - CreateThread failled");
			return 1;
		}
		else
		{
			ResumeThread(hThread[i]);
		}
	}

	for (DWORD i = 0 ; i < THREAD_NB ; i++)
	{ // Wait for each thread to finish
		WaitForSingleObject(hThread[i], INFINITE);
	}

	for (DWORD i = 0 ; i < THREAD_NB ; i++)
	{ // Close every single handle used for threads
		CloseHandle(hThread[i]);
	}

	return 0;
}

/// Entry point of the test
int _tmain(
	int argc, ///< [in] number of argument passed to the application
	_TCHAR* argv[] ///< [in] table of argument passed to the application
)
{
	// LoggerTest();
	// SerializerTest();
	// DeserializerTest();
	// DeserializerSerialiserTest();
	// PartitionResolverTest();
	SessionPersistenceTest();
	return 0;
}