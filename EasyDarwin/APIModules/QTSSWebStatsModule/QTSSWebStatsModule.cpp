/*
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 1999-2008 Apple Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 *
 */
 /*
	 File:       QTSSWebStatsModule.cpp

	 Contains:   Implements web stats module





 */

#include <time.h>

#include <stdio.h>      /* for qtss_printf */
#include <stdlib.h>     /* for getloadavg & other useful stuff */

#include "QTSSWebStatsModule.h"
#include "OSArrayObjectDeleter.h"
#include "StringParser.h"
#include "StrPtrLen.h"
#include "QTSSModuleUtils.h"

 // STATIC DATA

static char* sResponseHeader = "HTTP/1.0 200 OK\r\nServer: QTSS/3.0\r\nConnection: Close\r\nContent-Type: text/html\r\n\r\n";

static QTSS_ServerObject        sServer = NULL;
static QTSS_ModulePrefsObject   sAccessLogPrefs = NULL;
static QTSS_ModulePrefsObject   sReflectorPrefs = NULL;
static QTSS_ModulePrefsObject   sSvrControlPrefs = NULL;
static QTSS_ModulePrefsObject   sPrefs = NULL;
static QTSS_PrefsObject         sServerPrefs = NULL;

static Bool16                   sFalse = false;
static time_t                   sStartupTime = 0;

static char*	sDefaultURL = "";
static StrPtrLen sDefaultURLStr;
static char*	sDefaultURLPrefName = "web_stats_url";


static QTSS_Error   QTSSWebStatsModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error   Register(QTSS_Register_Params* inParams);
static QTSS_Error Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error FilterRequest(QTSS_Filter_Params* inParams);
static void SendStats(QTSS_StreamRef inStream, UInt32  refreshInterval, Bool16 displayHelp, StrPtrLen* fieldList);
static char*    GetPrefAsString(QTSS_ModulePrefsObject inPrefsObject, char* inPrefName);


// FUNCTION IMPLEMENTATIONS

QTSS_Error QTSSWebStatsModule_Main(void* inPrivateArgs)
{
	return _stublibrary_main(inPrivateArgs, QTSSWebStatsModuleDispatch);
}


QTSS_Error  QTSSWebStatsModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
	switch (inRole)
	{
	case QTSS_Register_Role:
		return Register(&inParams->regParams);
	case QTSS_Initialize_Role:
		return Initialize(&inParams->initParams);
	case QTSS_RTSPFilter_Role:
		return FilterRequest(&inParams->rtspFilterParams);
	}
	return QTSS_NoErr;
}


QTSS_Error Register(QTSS_Register_Params* inParams)
{
	// Do role & attribute setup
	(void)QTSS_AddRole(QTSS_Initialize_Role);
	(void)QTSS_AddRole(QTSS_RTSPFilter_Role);

	// Tell the server our name!
	static char* sModuleName = "QTSSWebStatsModule";
	::strcpy(inParams->outModuleName, sModuleName);

	return QTSS_NoErr;
}


QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{
	// Setup module utils
	QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);

	sAccessLogPrefs = QTSSModuleUtils::GetModulePrefsObject(QTSSModuleUtils::GetModuleObjectByName("QTSSAccessLogModule"));
	sReflectorPrefs = QTSSModuleUtils::GetModulePrefsObject(QTSSModuleUtils::GetModuleObjectByName("QTSSReflectorModule"));
	sPrefs = QTSSModuleUtils::GetModulePrefsObject(inParams->inModule);

	// This module may not be present, so be careful...
	QTSS_ModuleObject theSvrControlModule = QTSSModuleUtils::GetModuleObjectByName("QTSSSvrControlModule");
	if (theSvrControlModule != NULL)
		sSvrControlPrefs = QTSSModuleUtils::GetModulePrefsObject(theSvrControlModule);
	sServer = inParams->inServer;
	sServerPrefs = inParams->inPrefs;

	sStartupTime = ::time(NULL);

	sDefaultURLStr.Delete();
	sDefaultURLStr.Set(QTSSModuleUtils::GetStringAttribute(sPrefs, sDefaultURLPrefName, sDefaultURL));

	return QTSS_NoErr;
}

QTSS_Error FilterRequest(QTSS_Filter_Params* inParams)
{
	UInt8 sParamStopMask[] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //0-9      //stop unless a '\t', ' ', or '&'
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //10-19  
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //20-29
		0, 0, 0, 0, 0, 0, 0, 0, 1, 0, //30-39   
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //40-49
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //50-59
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //60-69
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //70-79
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //80-89
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //90-99
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //100-109
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //110-119
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //120-129
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //130-139
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //140-149
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //150-159
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //160-169
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //170-179
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //180-189
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //190-199
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //200-209
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //210-219
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //220-229
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //230-239
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //240-249
		0, 0, 0, 0, 0, 1             //250-255
	};


	//check to see if we should handle this request. Invokation is triggered
	//by a "GET /" request

	QTSS_RTSPRequestObject theRequest = inParams->inRTSPRequest;

	StrPtrLen theFullRequest;
	(void)QTSS_GetValuePtr(theRequest, qtssRTSPReqFullRequest, 0, (void**)&theFullRequest.Ptr, &theFullRequest.Len);

	StringParser fullRequest(&theFullRequest);

	StrPtrLen   strPtr;
	StrPtrLen   paramName;
	StrPtrLen   fieldsList;

	fullRequest.ConsumeWord(&strPtr);
	if (strPtr.Equal(StrPtrLen("GET")))   //it's a "Get" request
	{
		fullRequest.ConsumeWhitespace();
		if (fullRequest.Expect('/'))
		{
			UInt32  refreshInterval = 0;
			Bool16  displayHelp = false;


			OSCharArrayDeleter theWebStatsURL(GetPrefAsString(sPrefs, sDefaultURLPrefName));
			StrPtrLen theWebStatsURLPtr(theWebStatsURL.GetObject());

			// If there isn't any web stats URL, we can just return at this point
			if (theWebStatsURLPtr.Len == 0)
				return QTSS_NoErr;

			fullRequest.ConsumeUntil(&strPtr, StringParser::sEOLWhitespaceQueryMask);
			if (strPtr.Len != 0 && strPtr.Equal(theWebStatsURLPtr))   //it's a "stats" request
			{
				if (fullRequest.Expect('?'))
				{
					do {
						fullRequest.ConsumeWord(&paramName);

						if (paramName.Len != 0)
						{

							if (paramName.Equal(StrPtrLen("refresh", strlen("refresh"))))
							{
								if (fullRequest.Expect('='))
									refreshInterval = fullRequest.ConsumeInteger(NULL);
							}
							else if (paramName.Equal(StrPtrLen("help", strlen("help"))))
							{
								displayHelp = true;
							}
							else if (paramName.Equal(StrPtrLen("fields", strlen("fields"))))
							{
								if (fullRequest.Expect('='))
									fullRequest.ConsumeUntil(&fieldsList, (UInt8*)sParamStopMask);

							}
						}
					} while (paramName.Len != 0 && fullRequest.Expect('&'));
				}

				// Before sending a response, set keep alive to off for this connection
				(void)QTSS_SetValue(theRequest, qtssRTSPReqRespKeepAlive, 0, &sFalse, sizeof(sFalse));
				SendStats(inParams->inRTSPRequest, refreshInterval, displayHelp, (fieldsList.Len != 0) ? &fieldsList : NULL);
			}
		}
	}
	return QTSS_NoErr;
}


void SendStats(QTSS_StreamRef inStream, UInt32  refreshInterval, Bool16 displayHelp, StrPtrLen* fieldList)
{
	struct FieldIndex {
		char*   fieldName;
		int fieldIndex;
	};

	const FieldIndex kFieldIndexes[] = {
									{"title", 1},
									{"dnsname", 2},
									{"curtime", 3},
									{"", 4},
									{"serververs", 5},
									{"serverbornon", 6},
									{"serverstatus", 7},
									{"", 8},
									{"", 9},
									{"", 10},
									{"", 11},
									{"", 12},
									{"", 13},
									{"currtp", 14},
									{"currtsp", 15},
									{"currtsphttp", 16},
									{"curthru", 17},
									{"curpkts", 18},
									{"totbytes", 19},
									{"totconns", 20},
									{"", 21},
									{"connlimit", 22},
									{"thrulimit", 23},
									{"moviedir", 24},
									{"rtspip", 25},
									{"rtspport", 26},
									{"rtsptimeout", 27},
									{"rtptimeout", 28},
									{"secstobuffer", 29},
									{"", 30},
									{"accesslog", 31},
									{"accesslogdir",32},
									{"accesslogname", 33},
									{"accessrollsize", 34},
									{"accessrollinterval", 35},
									{"", 36},
									{"errorlog", 37},
									{"errorlogdir", 38},
									{"errorlogname", 39},
									{"errorrollsize", 40},
									{"errorrollinterval", 41},
									{"errorloglevel", 42},
									{"", 43},
									{"assertbreak", 44},
									{"autostart", 45},
									{"totbytesupdateinterval", 46},
									{"reflectordelay", 47},
									{"reflectorbucketsize", 48},
									{"historyinterval", 49},
									{"outoffiledesc", 50},
									{"numudpsockets", 51},
									{"apiversion", 52},
									{"numreliableudpbuffers", 53},
									{"reliableudpwastedbytes", 54},
									{"numtaskthreads", 55}
	};
	const int kMaxFieldNum = 55;
	static char* kEmptyStr = "?";
	char* thePrefStr = kEmptyStr;

	char buffer[1024];

	(void)QTSS_Write(inStream, sResponseHeader, ::strlen(sResponseHeader), NULL, 0);

	if (refreshInterval > 0)
	{
		qtss_sprintf(buffer, "<META HTTP-EQUIV=Refresh CONTENT=%"   _U32BITARG_   ">\n", refreshInterval);
		(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
	}

	//qtss_sprintf(buffer, "<body text=\"#000000\" bgcolor=\"#C0C0C0\" link=\"#0000FF\" vlink=\"#551A8B\" alink=\"#0000FF\">\n");
	//(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);

	char *theHTML = "<HTML><BODY>\n";

	(void)QTSS_Write(inStream, theHTML, ::strlen(theHTML), NULL, 0);

	if (displayHelp)
	{

#ifndef __MacOSX__
		static StrPtrLen sHelpLine1("<P><b>Streaming Server Statistics Help</b></P>\n");
#else
		static StrPtrLen sHelpLine1("<P><b>QuickTime Streaming Server Statistics Help</b></P>\n");
#endif

		static StrPtrLen sHelpLine2("<P>Example:</P>\n");
		static StrPtrLen sHelpLine3("<BLOCKQUOTE><P>http://server/statsURL?help&amp;refresh=15&amp;fields=curtime,cpuload </P>\n");

		static StrPtrLen sHelpLine4("\"?\" means that there are options being attached to the stats request.<BR>\n");
		static StrPtrLen sHelpLine5("\"&amp;\" separates multiple stats options<BR>\n<BR>\n");

		static StrPtrLen sHelpLine6("<P>The three possible parameters to stats are:</P>\n");
		static StrPtrLen sHelpLine7("<P>\"help\"  -- shows the help information you're reading right now.</P>\n");
		static StrPtrLen sHelpLine8("<P>\"refresh=&#91;n&#93;\"  -- tells the browser to automatically update the page every &#91;n&#93; seconds.</P>\n");
		static StrPtrLen sHelpLine9("<P>\"fields=&#91;fieldList&#93;\"  -- show only the fields specified in comma delimited &#91;fieldList&#93;</P>\n");
		static StrPtrLen sHelpLine10("<BLOCKQUOTE>The following fields are available for use with the \"fields\" option:</P><BLOCKQUOTE><DL>\n");

		(void)QTSS_Write(inStream, sHelpLine1.Ptr, sHelpLine1.Len, NULL, 0);
		(void)QTSS_Write(inStream, sHelpLine2.Ptr, sHelpLine2.Len, NULL, 0);
		(void)QTSS_Write(inStream, sHelpLine3.Ptr, sHelpLine3.Len, NULL, 0);

		(void)QTSS_Write(inStream, sHelpLine4.Ptr, sHelpLine4.Len, NULL, 0);
		(void)QTSS_Write(inStream, sHelpLine5.Ptr, sHelpLine5.Len, NULL, 0);

		(void)QTSS_Write(inStream, sHelpLine6.Ptr, sHelpLine6.Len, NULL, 0);
		(void)QTSS_Write(inStream, sHelpLine7.Ptr, sHelpLine7.Len, NULL, 0);
		(void)QTSS_Write(inStream, sHelpLine8.Ptr, sHelpLine8.Len, NULL, 0);
		(void)QTSS_Write(inStream, sHelpLine9.Ptr, sHelpLine9.Len, NULL, 0);
		(void)QTSS_Write(inStream, sHelpLine10.Ptr, sHelpLine10.Len, NULL, 0);

		for (short i = 0; i < kMaxFieldNum; i++)
		{
			qtss_sprintf(buffer, "<DT><I>%s</I></DT>\n", kFieldIndexes[i].fieldName);
			(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
		}

		static StrPtrLen sHelpLine11("</DL></BLOCKQUOTE></BLOCKQUOTE></BLOCKQUOTE><BR><P><HR>");
		(void)QTSS_Write(inStream, sHelpLine11.Ptr, sHelpLine11.Len, NULL, 0);
	}

	StringParser fieldNamesParser(fieldList);
	StrPtrLen fieldName;
	int fieldNum = 0;
	do
	{


		if (fieldList != NULL)
		{
			fieldNum = 0;

			fieldNamesParser.ConsumeWord(&fieldName);

			for (short i = 0; i < kMaxFieldNum; i++)
			{
				if (fieldName.Equal(StrPtrLen(kFieldIndexes[i].fieldName, ::strlen(kFieldIndexes[i].fieldName))))
				{
					fieldNum = kFieldIndexes[i].fieldIndex;
					break;
				}
			}
		}
		else
		{
			fieldNum++;
			if (fieldNum > kMaxFieldNum)
				fieldNum = 0;
		}

		UInt32 theLen = 0;

		switch (fieldNum)
		{
		case 1:
			{
#if __MacOSX__
				static StrPtrLen sStatsLine1("<TITLE>QuickTime Streaming Server Stats</TITLE><BR>\n");
				(void)QTSS_Write(inStream, sStatsLine1.Ptr, sStatsLine1.Len, NULL, 0);
#else
				static StrPtrLen sStatsLine1("<TITLE>Streaming Server Stats</TITLE><BR>\n");
				(void)QTSS_Write(inStream, sStatsLine1.Ptr, sStatsLine1.Len, NULL, 0);
#endif

#if __MacOSX__
				static StrPtrLen sStatsLine2("<center><h1>QuickTime Streaming Server Statistics</h1></center>\n");
				(void)QTSS_Write(inStream, sStatsLine2.Ptr, sStatsLine2.Len, NULL, 0);
#else
				static StrPtrLen sStatsLine2("<center><h1>Streaming Server Statistics</h1></center>\n");
				(void)QTSS_Write(inStream, sStatsLine2.Ptr, sStatsLine2.Len, NULL, 0);
#endif
			}
			break;

		case 2:
			{
				StrPtrLen theDNS;
				(void)QTSS_GetValuePtr(sServer, qtssSvrDefaultDNSName, 0, (void**)&theDNS.Ptr, &theDNS.Len);

				if (theDNS.Ptr == NULL)
				{   // no DNS, try for the IP address only.
					(void)QTSS_GetValuePtr(sServer, qtssSvrDefaultIPAddr, 0, (void**)&theDNS.Ptr, &theDNS.Len);

				}

				if (theDNS.Ptr != NULL)
				{
					qtss_sprintf(buffer, "<b>DNS Name (default): </b> %s<BR>\n", theDNS.Ptr);
					(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
				}
			}
			break;

		case 3:
			{
				char uptimebuffer[1024];
				time_t curTime = ::time(NULL);
				qtss_sprintf(uptimebuffer, "<b>Current Time: </b> %s<BR>\n", qtss_ctime(&curTime, buffer, sizeof(buffer)));
				(void)QTSS_Write(inStream, uptimebuffer, ::strlen(uptimebuffer), NULL, 0);

				time_t upTime = curTime - sStartupTime;
#define kDaySeconds  (24 * 60 * 60)
#define kHourSeconds  (60 * 60)
#define kMinuteSeconds 60

				UInt32 upTimeDays = upTime / kDaySeconds;
				UInt32 upTimeHours = (upTime % kDaySeconds) / kHourSeconds;
				UInt32 upTimeMinutes = (upTime % kHourSeconds) / kMinuteSeconds;
				UInt32 upTimeSeconds = (upTime % kMinuteSeconds);
				qtss_snprintf(uptimebuffer, sizeof(uptimebuffer), "<b>Up Time Total Seconds: </b> %"   _U32BITARG_   "<BR>\n", upTime);
				uptimebuffer[sizeof(uptimebuffer) - 1] = 0;
				(void)QTSS_Write(inStream, uptimebuffer, ::strlen(uptimebuffer), NULL, 0);

				qtss_snprintf(uptimebuffer, sizeof(uptimebuffer), "<b>Up Time: </b> %"   _U32BITARG_   " days %"   _U32BITARG_   " hours %"   _U32BITARG_   " minutes %"   _U32BITARG_   " seconds <BR>\n", upTimeDays, upTimeHours, upTimeMinutes, upTimeSeconds);
				uptimebuffer[sizeof(uptimebuffer) - 1] = 0;
				(void)QTSS_Write(inStream, uptimebuffer, ::strlen(uptimebuffer), NULL, 0);
			}
			break;

		case 4:
			{
				(void)QTSS_Write(inStream, "<P><HR>", ::strlen("<P><HR>"), NULL, 0);
			}
			break;

		case 5:
			{
				StrPtrLen theVersion;
				(void)QTSS_GetValuePtr(sServer, qtssSvrRTSPServerHeader, 0, (void**)&theVersion.Ptr, &theVersion.Len);
				Assert(theVersion.Ptr != NULL);
				if (theVersion.Len > 7) //Skip the "Server:" text
					theVersion.Ptr += 7;
				qtss_sprintf(buffer, "<b>Server Version: </b>%s<BR>\n", theVersion.Ptr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 6:
			{
				StrPtrLen theBuildDate;
				(void)QTSS_GetValuePtr(sServer, qtssSvrServerBuildDate, 0, (void**)&theBuildDate.Ptr, &theBuildDate.Len);
				Assert(theBuildDate.Ptr != NULL);
				qtss_sprintf(buffer, "<b>Server Build Date: </b> %s<BR>\n", theBuildDate.Ptr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 7:
			{
				char statusBuffer[1024];
				const char* states[] = { "Starting Up",
											"Running",
											"Refusing Connections",
											"Fatal Error",
											"Shutting Down"
				};
				QTSS_ServerState theState = qtssRunningState;
				theLen = sizeof(theState);
				(void)QTSS_GetValue(sServer, qtssSvrState, 0, &theState, &theLen);

				if (theState == qtssRunningState)
				{
					qtss_snprintf(statusBuffer, sizeof(statusBuffer), "<b>Status: </b> %s since %s<BR>", states[theState], qtss_ctime(&sStartupTime, buffer, sizeof(buffer)));
				}
				else
					qtss_snprintf(statusBuffer, sizeof(statusBuffer), "<b>Status: </b> %s<BR>", states[theState]);
				(void)QTSS_Write(inStream, statusBuffer, ::strlen(statusBuffer), NULL, 0);
			}
			break;

		case 8:
			{
				(void)QTSS_Write(inStream, "<P><HR>", ::strlen("<P><HR>"), NULL, 0);
			}
			break;

		case 9:
			{

				//NOOP
			}
			break;

		case 10:
			{

				//NOOP
			}
			break;

		case 11:
			{

				//NOOP
			}
			break;

		case 12:
			{
				/*
					struct vm_statistics vmStats = {};
					if (vm_statistics (current_task (), &vmStats) != KERN_SUCCESS)
						memset (&stats, '\0', sizeof (vmStats)) ;
				*/
				(void)QTSS_Write(inStream, "<P><HR>", ::strlen("<P><HR>"), NULL, 0);
			}
			break;

		case 13:
			{
				(void)QTSS_Write(inStream, "<P><HR>", ::strlen("<P><HR>"), NULL, 0);
			}
			break;

			//**********************************




		case 14:
			{
				(void)QTSS_GetValueAsString(sServer, qtssRTPSvrCurConn, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>Current RTP Connections: </b> %s<BR>\n", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 15:
			{
				(void)QTSS_GetValueAsString(sServer, qtssRTSPCurrentSessionCount, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>Current RTSP Connections: </b> %s<BR>\n", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 16:
			{
				(void)QTSS_GetValueAsString(sServer, qtssRTSPHTTPCurrentSessionCount, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>Current RTSP over HTTP Connections: </b> %s<BR>\n", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 17:
			{
				UInt32 curBandwidth = 0;
				theLen = sizeof(curBandwidth);
				(void)QTSS_GetValue(sServer, qtssRTPSvrCurBandwidth, 0, &curBandwidth, &theLen);

				qtss_sprintf(buffer, "<b>Current Throughput: </b> %"   _U32BITARG_   " kbits<BR>\n", curBandwidth / 1024);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 18:
			{
				(void)QTSS_GetValueAsString(sServer, qtssRTPSvrCurPackets, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>Current Packets Per Second: </b> %s <BR>\n", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 19:
			{
				(void)QTSS_GetValueAsString(sServer, qtssRTPSvrTotalBytes, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>Total Bytes Served: </b> %s<BR>\n", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 20:
			{
				(void)QTSS_GetValueAsString(sServer, qtssRTPSvrTotalConn, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>Total Connections: </b> %s<BR>", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 21:
			{
				(void)QTSS_Write(inStream, "<P><HR>", ::strlen("<P><HR>"), NULL, 0);
			}
			break;

			//**************************************
		case 22:
			{
				(void)QTSS_GetValueAsString(sServerPrefs, qtssPrefsMaximumConnections, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>Maximum Connections: </b> %s<BR>\n", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 23:
			{
				(void)QTSS_GetValueAsString(sServerPrefs, qtssPrefsMaximumBandwidth, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>Maximum Throughput: </b> %s Kbits<BR>\n", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 24:
			{
				(void)QTSS_GetValueAsString(sServerPrefs, qtssPrefsMovieFolder, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>Movie Folder Path: </b> %s<BR>\n", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 25:
			{
				(void)QTSS_GetValueAsString(sServerPrefs, qtssPrefsRTSPIPAddr, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>RTSP IP Address: </b> %s<BR>\n", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 26:
			{
				static StrPtrLen sRTSPPortsStart("<b>RTSP Ports: </b> ");
				(void)QTSS_Write(inStream, sRTSPPortsStart.Ptr, sRTSPPortsStart.Len, NULL, 0);

				StrPtrLen thePort;
				for (UInt32 theIndex = 0; true; theIndex++)
				{
					QTSS_Error theErr = QTSS_GetValuePtr(sServer, qtssSvrRTSPPorts, theIndex, (void**)&thePort.Ptr, &thePort.Len);
					if (theErr != QTSS_NoErr)
						break;

					Assert(thePort.Ptr != NULL);
					char temp[20];
					qtss_sprintf(temp, "%u ", *(UInt16*)thePort.Ptr);
					(void)QTSS_Write(inStream, temp, ::strlen(temp), NULL, 0);
				}

				static StrPtrLen sRTSPPortsEnd("<BR>\n");
				(void)QTSS_Write(inStream, sRTSPPortsEnd.Ptr, sRTSPPortsEnd.Len, NULL, 0);
			}
			break;

		case 27:
			{
				(void)QTSS_GetValueAsString(sServerPrefs, qtssPrefsRTSPTimeout, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>RTP Timeout: </b> %s<BR>\n", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 28:
			{
				(void)QTSS_GetValueAsString(sServerPrefs, qtssPrefsRTPTimeout, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>RTP Timeout: </b> %s<BR>\n", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 29:
			{
				(void)QTSS_Write(inStream, "<P><HR>", ::strlen("<P><HR>"), NULL, 0);
			}
			break;

		case 30:
			{
				(void)QTSS_Write(inStream, "<P><HR>", ::strlen("<P><HR>"), NULL, 0);
			}
			break;

		case 31:
			{
				if (sAccessLogPrefs != NULL)
				{
					thePrefStr = GetPrefAsString(sAccessLogPrefs, "request_logging");
					qtss_sprintf(buffer, "<b>Access Logging: </b> %s<BR>\n", thePrefStr);
					(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
				}
			}
			break;

		case 32:
			{
				if (sAccessLogPrefs != NULL)
				{
					thePrefStr = GetPrefAsString(sAccessLogPrefs, "request_logfile_dir");
					qtss_sprintf(buffer, "<b>Access Log Directory: </b> %s<BR>\n", thePrefStr);
					(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
				}
			}
			break;

		case 33:
			{
				if (sAccessLogPrefs != NULL)
				{
					thePrefStr = GetPrefAsString(sAccessLogPrefs, "request_logfile_name");
					qtss_sprintf(buffer, "<b>Access Log Name: </b> %s<BR>\n", thePrefStr);
					(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
				}
			}
			break;

		case 34:
			{
				if (sAccessLogPrefs != NULL)
				{
					thePrefStr = GetPrefAsString(sAccessLogPrefs, "request_logfile_size");
					qtss_sprintf(buffer, "<b>Access Log Roll Size: </b> %s<BR>\n", thePrefStr);
					(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
				}
			}
			break;

		case 35:
			{
				if (sAccessLogPrefs != NULL)
				{
					thePrefStr = GetPrefAsString(sAccessLogPrefs, "request_logfile_interval");
					qtss_sprintf(buffer, "<b>Access Log Roll Interval (days): </b> %s<BR>", thePrefStr);
					(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
				}
			}
			break;

		case 36:
			{
				(void)QTSS_Write(inStream, "<P><HR>", ::strlen("<P><HR>"), NULL, 0);
			}
			break;

		case 37:
			{
				(void)QTSS_GetValueAsString(sServerPrefs, qtssPrefsErrorLogEnabled, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>Error Logging: </b> %s<BR>\n", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 38:
			{
				(void)QTSS_GetValueAsString(sServerPrefs, qtssPrefsErrorLogDir, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>Error Log Directory: </b> %s<BR>\n", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 39:
			{
				(void)QTSS_GetValueAsString(sServerPrefs, qtssPrefsErrorLogName, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>Error Log Name: </b> %s<BR>\n", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 40:
			{
				(void)QTSS_GetValueAsString(sServerPrefs, qtssPrefsMaxErrorLogSize, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>Error Log Roll Size: </b> %s<BR>\n", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 41:
			{
				(void)QTSS_GetValueAsString(sServerPrefs, qtssPrefsErrorRollInterval, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>Error Log Roll Interval (days): </b> %s<BR>\n", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 42:
			{
				(void)QTSS_GetValueAsString(sServerPrefs, qtssPrefsErrorLogVerbosity, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>Error Log Verbosity: </b> %s<BR>", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 43:
			{
				(void)QTSS_Write(inStream, "<P><HR>", ::strlen("<P><HR>"), NULL, 0);
			}
			break;

		case 44:
			{
				(void)QTSS_GetValueAsString(sServerPrefs, qtssPrefsBreakOnAssert, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>Break On Assert: </b> %s<BR>\n", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 45:
			{
				(void)QTSS_GetValueAsString(sServerPrefs, qtssPrefsAutoRestart, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>AutoStart: </b> %s<BR>\n", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 46:
			{
				(void)QTSS_GetValueAsString(sServerPrefs, qtssPrefsTotalBytesUpdate, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>Total Bytes Update Interval: </b> %s<BR>\n", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 47:
			{
				if (sReflectorPrefs != NULL)
				{
					thePrefStr = GetPrefAsString(sReflectorPrefs, "reflector_delay");
					qtss_sprintf(buffer, "<b>Reflector Delay Time: </b> %s<BR>\n", thePrefStr);
					(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
				}
			}
			break;

		case 48:
			{
				if (sReflectorPrefs != NULL)
				{
					thePrefStr = GetPrefAsString(sReflectorPrefs, "reflector_bucket_size");
					qtss_sprintf(buffer, "<b>Reflector Bucket Size: </b> %s<BR>\n", thePrefStr);
					(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
				}
			}
			break;

		case 49:
			{
				if (sSvrControlPrefs != NULL)
				{
					thePrefStr = GetPrefAsString(sSvrControlPrefs, "history_update_interval");
					qtss_sprintf(buffer, "<b>History Update Interval: </b> %s<BR>\n", thePrefStr);
					(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
				}
			}
			break;

		case 50:
			{
				Bool16 isOutOfDescriptors = false;
				theLen = sizeof(isOutOfDescriptors);
				(void)QTSS_GetValue(sServer, qtssSvrIsOutOfDescriptors, 0, &isOutOfDescriptors, &theLen);

				qtss_sprintf(buffer, "<b>Out of file descriptors: </b> %d<BR>\n", isOutOfDescriptors);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 51:
			{
				(void)QTSS_GetValueAsString(sServer, qtssRTPSvrNumUDPSockets, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>Number of UDP sockets: </b> %s<BR>\n", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 52:
			{
				UInt32 apiVersion = 0;
				UInt32 size = sizeof(UInt32);
				(void)QTSS_GetValue(sServer, qtssServerAPIVersion, 0, &apiVersion, &size);
				qtss_sprintf(buffer, "<b>API version: </b> %d.%d<BR>\n", (int)((UInt32)(apiVersion & (UInt32)0xFFFF0000L) >> 16), (int)(apiVersion &(UInt32)0x0000FFFFL));
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 53:
			{
				UInt32 reliableUDPBuffers = 0;
				UInt32 blahSize = sizeof(reliableUDPBuffers);
				(void)QTSS_GetValue(sServer, qtssSvrNumReliableUDPBuffers, 0, &reliableUDPBuffers, &blahSize);
				qtss_sprintf(buffer, "<b>Num Reliable UDP Retransmit Buffers: </b> %"   _U32BITARG_   "<BR>\n", reliableUDPBuffers);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 54:
			{
				UInt32 wastedBufSpace = 0;
				UInt32 blahSize2 = sizeof(wastedBufSpace);
				(void)QTSS_GetValue(sServer, qtssSvrReliableUDPWastageInBytes, 0, &wastedBufSpace, &blahSize2);
				qtss_sprintf(buffer, "<b>Amount of buffer space being wasted in UDP Retrans buffers: </b> %"   _U32BITARG_   "<BR>\n", wastedBufSpace);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;

		case 55:
			{
				(void)QTSS_GetValueAsString(sServer, qtssSvrNumThreads, 0, &thePrefStr);
				qtss_sprintf(buffer, "<b>Number of Task Threads: </b> %s<BR>\n", thePrefStr);
				(void)QTSS_Write(inStream, buffer, ::strlen(buffer), NULL, 0);
			}
			break;



		default:
			break;


		} //switch fieldNum


		if (fieldList != NULL && !fieldNamesParser.Expect(','))
			fieldNum = 0;

		if (thePrefStr != kEmptyStr)
			delete[] thePrefStr;

		thePrefStr = kEmptyStr;

	} while (fieldNum != 0);

	theHTML = "</BODY></HTML>\n";
	(void)QTSS_Write(inStream, theHTML, ::strlen(theHTML), NULL, 0);
}






char*   GetPrefAsString(QTSS_ModulePrefsObject inPrefsObject, char* inPrefName)
{
	static StrPtrLen                sEmpty("");

	//
	// Get the attribute ID of this pref.
	QTSS_AttributeID theID = qtssIllegalAttrID;

	if (inPrefsObject != NULL)
		theID = QTSSModuleUtils::GetAttrID(inPrefsObject, inPrefName);

	char* theString = NULL;

	if (inPrefsObject != NULL)
		(void)QTSS_GetValueAsString(inPrefsObject, theID, 0, &theString);

	if (theString == NULL)
		theString = sEmpty.GetAsCString();

	return theString;
}
