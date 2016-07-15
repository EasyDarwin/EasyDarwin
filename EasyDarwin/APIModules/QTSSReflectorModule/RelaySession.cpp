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
	 File:       RelaySession.cpp

	 Contains:   Implementation of object defined in RelaySession.h.



 */

#include "RelaySession.h"
#include "QTSSModuleUtils.h"
#include "SocketUtils.h"
#include "revision.h"
#include "RTSPSourceInfo.h"

static StrPtrLen    sUDPSourceStr("udp_source");
static StrPtrLen    sRTSPSourceStr("rtsp_source");
static StrPtrLen    sAnnouncedSourceStr("announced_source");
static StrPtrLen    sEmptyStr("");

static char*        sRelaySessionObjectName = "relay_session";
static char*        sRelayNameName = "relay_name";
static char*        sSourceTypeName = "source_type";
static char*        sSourceIPAddrName = "source_ip_addr";
static char*        sSourceInIPAddrName = "source_in_ip_addr";
static char*        sSourceUDPPortsName = "source_udp_ports";
static char*        sSourceRTSPPortName = "source_rtsp_port";
static char*        sSourceURLName = "source_url";
static char*        sSourceUsernameName = "source_username";
static char*        sSourcePasswordName = "source_password";
static char*        sSourceTTLName = "source_ttl";
static char*        sRelayOutputObjectName = "relay_output";

QTSS_Object     RelaySession::relayModuleAttributesObject;
QTSS_ObjectType     RelaySession::qtssRelaySessionObjectType;

QTSS_AttributeID    RelaySession::sRelaySessionObjectID = qtssIllegalAttrID;
QTSS_AttributeID    RelaySession::sRelayName = qtssIllegalAttrID;
QTSS_AttributeID    RelaySession::sSourceType = qtssIllegalAttrID;
QTSS_AttributeID    RelaySession::sSourceIPAddr = qtssIllegalAttrID;
QTSS_AttributeID    RelaySession::sSourceInIPAddr = qtssIllegalAttrID;
QTSS_AttributeID    RelaySession::sSourceUDPPorts = qtssIllegalAttrID;
QTSS_AttributeID    RelaySession::sSourceRTSPPort = qtssIllegalAttrID;
QTSS_AttributeID    RelaySession::sSourceURL = qtssIllegalAttrID;
QTSS_AttributeID    RelaySession::sSourceUsername = qtssIllegalAttrID;
QTSS_AttributeID    RelaySession::sSourcePassword = qtssIllegalAttrID;
QTSS_AttributeID    RelaySession::sSourceTTL = qtssIllegalAttrID;
QTSS_AttributeID    RelaySession::sRelayOutputObject = qtssIllegalAttrID;

char            RelaySession::sRelayUserAgent[20] = "";

void RelaySession::Register()
{
	qtssRelaySessionObjectType = 0;

	// create relay session object type
	(void)QTSS_CreateObjectType(&qtssRelaySessionObjectType);

	// Add the static attributes to the qtssRelaySessionObjectType object
	(void)QTSS_AddStaticAttribute(qtssRelaySessionObjectType, sRelayNameName, NULL, qtssAttrDataTypeCharArray);
	(void)QTSS_IDForAttr(qtssRelaySessionObjectType, sRelayNameName, &sRelayName);              // relay name

	(void)QTSS_AddStaticAttribute(qtssRelaySessionObjectType, sSourceTypeName, NULL, qtssAttrDataTypeCharArray);
	(void)QTSS_IDForAttr(qtssRelaySessionObjectType, sSourceTypeName, &sSourceType);                // source type

	(void)QTSS_AddStaticAttribute(qtssRelaySessionObjectType, sSourceIPAddrName, NULL, qtssAttrDataTypeCharArray);
	(void)QTSS_IDForAttr(qtssRelaySessionObjectType, sSourceIPAddrName, &sSourceIPAddr);            // source addr

	(void)QTSS_AddStaticAttribute(qtssRelaySessionObjectType, sSourceInIPAddrName, NULL, qtssAttrDataTypeCharArray);
	(void)QTSS_IDForAttr(qtssRelaySessionObjectType, sSourceInIPAddrName, &sSourceInIPAddr);        // interface addr

	(void)QTSS_AddStaticAttribute(qtssRelaySessionObjectType, sSourceUDPPortsName, NULL, qtssAttrDataTypeUInt16);
	(void)QTSS_IDForAttr(qtssRelaySessionObjectType, sSourceUDPPortsName, &sSourceUDPPorts);        // udp ports

	(void)QTSS_AddStaticAttribute(qtssRelaySessionObjectType, sSourceRTSPPortName, NULL, qtssAttrDataTypeUInt16);
	(void)QTSS_IDForAttr(qtssRelaySessionObjectType, sSourceRTSPPortName, &sSourceRTSPPort);        // rtsp port

	(void)QTSS_AddStaticAttribute(qtssRelaySessionObjectType, sSourceURLName, NULL, qtssAttrDataTypeCharArray);
	(void)QTSS_IDForAttr(qtssRelaySessionObjectType, sSourceURLName, &sSourceURL);                  // url

	(void)QTSS_AddStaticAttribute(qtssRelaySessionObjectType, sSourceUsernameName, NULL, qtssAttrDataTypeCharArray);
	(void)QTSS_IDForAttr(qtssRelaySessionObjectType, sSourceUsernameName, &sSourceUsername);    // username

	(void)QTSS_AddStaticAttribute(qtssRelaySessionObjectType, sSourcePasswordName, NULL, qtssAttrDataTypeCharArray);
	(void)QTSS_IDForAttr(qtssRelaySessionObjectType, sSourcePasswordName, &sSourcePassword);        // password

	(void)QTSS_AddStaticAttribute(qtssRelaySessionObjectType, sSourceTTLName, NULL, qtssAttrDataTypeUInt16);
	(void)QTSS_IDForAttr(qtssRelaySessionObjectType, sSourceTTLName, &sSourceTTL);                  // ttl

	(void)QTSS_AddStaticAttribute(qtssRelaySessionObjectType, sRelayOutputObjectName, NULL, qtssAttrDataTypeQTSS_Object);
	(void)QTSS_IDForAttr(qtssRelaySessionObjectType, sRelayOutputObjectName, &sRelayOutputObject);  // relay output

	//char* strEnd        = NULL;
	char* relayStr = "QTSS_Relay/";
	//kVersionString is changed now -- it doesn't contain any spaces or the build number
	//strEnd = strchr(kVersionString, ' ');
	//Assert(strEnd != NULL);
#ifndef __Win32__
	//qtss_snprintf(sRelayUserAgent, ::strlen(relayStr) + (strEnd - kVersionString) + 1, "%s/%s", relayStr, kVersionString);
	qtss_snprintf(sRelayUserAgent, ::strlen(relayStr) + ::strlen(kVersionString) + 1, "%s%s", relayStr, kVersionString);
#else
	//_snprintf(sRelayUserAgent, ::strlen(relayStr) + (strEnd - kVersionString) + 1, "%s/%s", relayStr, kVersionString);
	_snprintf(sRelayUserAgent, ::strlen(relayStr) + ::strlen(kVersionString) + 1, "%s%s", relayStr, kVersionString);
#endif        
}

void RelaySession::Initialize(QTSS_Object inRelayModuleAttributesObject)
{
	ReflectorSession::Initialize();

	if (inRelayModuleAttributesObject != NULL)
	{
		relayModuleAttributesObject = inRelayModuleAttributesObject;
		sRelaySessionObjectID = QTSSModuleUtils::CreateAttribute(inRelayModuleAttributesObject, sRelaySessionObjectName, qtssAttrDataTypeQTSS_Object, NULL, 0);
	}
}

QTSS_Error RelaySession::SetupRelaySession(SourceInfo* inInfo)
{
	QTSS_Error theErr = this->SetupReflectorSession(inInfo, NULL);

	if (theErr != QTSS_NoErr)
		return theErr;

	// create the reflector session object for this session
	UInt32 outIndex = 0;
	fRelaySessionObject = NULL;

	theErr = QTSS_LockObject(relayModuleAttributesObject);
	Assert(theErr == QTSS_NoErr);

	theErr = QTSS_CreateObjectValue(relayModuleAttributesObject, sRelaySessionObjectID, qtssRelaySessionObjectType, &outIndex, &fRelaySessionObject);
	Assert(theErr == QTSS_NoErr);

	if (fRelaySessionObject == NULL)
		return theErr;

	// set the values for all the static attributes in this session

	char* relayName = inInfo->Name();       // name of the relay
	if (relayName != NULL)
		theErr = QTSS_SetValue(fRelaySessionObject, sRelayName, 0, (void*)relayName, ::strlen(relayName));
	else
		theErr = QTSS_SetValue(fRelaySessionObject, sRelayName, 0, (void*)sEmptyStr.Ptr, sEmptyStr.Len);
	Assert(theErr == QTSS_NoErr);


	StrPtrLen sourceStr;            // type of source

	if (inInfo->IsRTSPSourceInfo())
	{
		if (((RTSPSourceInfo*)inInfo)->IsAnnounce())
			sourceStr.Set(sAnnouncedSourceStr.Ptr, sAnnouncedSourceStr.Len);
		else
			sourceStr.Set(sRTSPSourceStr.Ptr, sRTSPSourceStr.Len);
	}
	else
		sourceStr.Set(sUDPSourceStr.Ptr, sUDPSourceStr.Len);

	theErr = QTSS_SetValue(fRelaySessionObject, sSourceType, 0, (void*)sourceStr.Ptr, sourceStr.Len);
	Assert(theErr == QTSS_NoErr);

	char theIPAddrBuf[20];
	StrPtrLen theIPAddr(theIPAddrBuf, 20);

	struct in_addr theSrcAddr;      // source ip address
	theSrcAddr.s_addr = htonl(inInfo->GetStreamInfo(0)->fSrcIPAddr);
	SocketUtils::ConvertAddrToString(theSrcAddr, &theIPAddr);

	theErr = QTSS_SetValue(fRelaySessionObject, sSourceIPAddr, 0, (void*)theIPAddr.Ptr, theIPAddr.Len);
	Assert(theErr == QTSS_NoErr);

	struct in_addr theDestAddr;     // dest (of source) ip address
	theDestAddr.s_addr = htonl(inInfo->GetStreamInfo(0)->fDestIPAddr);
	SocketUtils::ConvertAddrToString(theDestAddr, &theIPAddr);

	theErr = QTSS_SetValue(fRelaySessionObject, sSourceInIPAddr, 0, (void*)theIPAddr.Ptr, theIPAddr.Len);
	Assert(theErr == QTSS_NoErr);


	for (UInt32 index = 0; index < (inInfo->GetNumStreams()); index++)  // source udp ports
	{
		UInt16 udpPort = inInfo->GetStreamInfo(index)->fPort;
		theErr = QTSS_SetValue(fRelaySessionObject, sSourceUDPPorts, index, &udpPort, sizeof(udpPort));
		Assert(theErr == QTSS_NoErr);
	}

	if (inInfo->IsRTSPSourceInfo())
	{
		RTSPSourceInfo* rtspInfo = (RTSPSourceInfo*)inInfo;
		if (!rtspInfo->IsAnnounce())
		{
			UInt16 rtspPort = (UInt16)rtspInfo->GetHostPort();
			char* username = rtspInfo->GetUsername();
			char* password = rtspInfo->GetPassword();
			theErr = QTSS_SetValue(fRelaySessionObject, sSourceRTSPPort, 0, &rtspPort, sizeof(rtspPort));  // source rtsp port
			Assert(theErr == QTSS_NoErr);

			theErr = QTSS_SetValue(fRelaySessionObject, sSourceUsername, 0, username, sizeof(username));   // source username
			Assert(theErr == QTSS_NoErr);

			theErr = QTSS_SetValue(fRelaySessionObject, sSourcePassword, 0, password, sizeof(password));   // source password  
			Assert(theErr == QTSS_NoErr);
		}

		char* url = rtspInfo->GetSourceURL();
		theErr = QTSS_SetValue(fRelaySessionObject, sSourceURL, 0, url, ::strlen(url));
		Assert(theErr == QTSS_NoErr);                   // source url
	}

	UInt16 ttl = inInfo->GetStreamInfo(0)->fTimeToLive;
	theErr = QTSS_SetValue(fRelaySessionObject, sSourceTTL, 0, &ttl, sizeof(ttl)); // source ttl
	Assert(theErr == QTSS_NoErr);

	theErr = QTSS_UnlockObject(relayModuleAttributesObject);
	Assert(theErr == QTSS_NoErr);

	return QTSS_NoErr;
}

RelaySession::~RelaySession()
{
	QTSS_Object sessionObject;
	UInt32 len = sizeof(QTSS_Object);

	for (int x = 0; QTSS_GetValue(relayModuleAttributesObject, sRelaySessionObjectID, x, &sessionObject, &len) == QTSS_NoErr; x++)
	{
		Assert(sessionObject != NULL);
		Assert(len == sizeof(QTSS_Object));

		if (sessionObject == fRelaySessionObject)
		{
			(void)QTSS_RemoveValue(relayModuleAttributesObject, sRelaySessionObjectID, x);
			break;
		}
	}
}







