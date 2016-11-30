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
	 File:       RelayOutput.cpp

	 Contains:   Implementation of object described in .h file



 */

#include "RelayOutput.h"

#include "OSMemory.h"
#include "SocketUtils.h"
#include "RTSPSourceInfo.h"

static StrPtrLen    sUDPDestStr("udp_destination");
static StrPtrLen    sAnnouncedDestStr("announced_destination");

// STATIC DATA
OSQueue RelayOutput::sRelayOutputQueue;
OSMutex RelayOutput::sQueueMutex;

QTSS_ObjectType             RelayOutput::qtssRelayOutputObjectType;

QTSS_AttributeID            RelayOutput::sOutputType = qtssIllegalAttrID;
QTSS_AttributeID            RelayOutput::sOutputDestAddr = qtssIllegalAttrID;
QTSS_AttributeID            RelayOutput::sOutputLocalAddr = qtssIllegalAttrID;
QTSS_AttributeID            RelayOutput::sOutputUDPPorts = qtssIllegalAttrID;
QTSS_AttributeID            RelayOutput::sOutputRTSPPort = qtssIllegalAttrID;
QTSS_AttributeID            RelayOutput::sOutputURL = qtssIllegalAttrID;
QTSS_AttributeID            RelayOutput::sOutputTTL = qtssIllegalAttrID;
QTSS_AttributeID            RelayOutput::sOutputCurPacketsPerSec = qtssIllegalAttrID;
QTSS_AttributeID            RelayOutput::sOutputCurBitsPerSec = qtssIllegalAttrID;
QTSS_AttributeID            RelayOutput::sOutputTotalPacketsSent = qtssIllegalAttrID;
QTSS_AttributeID            RelayOutput::sOutputTotalBytesSent = qtssIllegalAttrID;

static char*                    sOutputTypeName = "output_type";
static char*                    sOutputDestAddrName = "output_dest_addr";
static char*                    sOutputLocalAddrName = "output_local_addr";
static char*                    sOutputUDPPortsName = "output_udp_ports";
static char*                    sOutputRTSPPortName = "output_rtsp_port";
static char*                    sOutputURLName = "output_url";
static char*                    sOutputTTLName = "output_ttl";

static char*                    sOutputCurPacketsPerSecName = "output_cur_packetspersec";
static char*                    sOutputCurBitsPerSecName = "output_cur_bitspersec";
static char*                    sOutputTotalPacketsSentName = "output_total_packets_sent";
static char*                    sOutputTotalBytesSentName = "output_total_bytes_sent";

void RelayOutput::Register()
{
	// Create the relay output object type
	(void)QTSS_CreateObjectType(&qtssRelayOutputObjectType);

	// Add the static attributes to the qtssRelayOutputObjectType object
	(void)QTSS_AddStaticAttribute(qtssRelayOutputObjectType, sOutputTypeName, NULL, qtssAttrDataTypeCharArray);
	(void)QTSS_IDForAttr(qtssRelayOutputObjectType, sOutputTypeName, &sOutputType);         // dest type

	(void)QTSS_AddStaticAttribute(qtssRelayOutputObjectType, sOutputDestAddrName, NULL, qtssAttrDataTypeCharArray);
	(void)QTSS_IDForAttr(qtssRelayOutputObjectType, sOutputDestAddrName, &sOutputDestAddr);     // dest addr

	(void)QTSS_AddStaticAttribute(qtssRelayOutputObjectType, sOutputLocalAddrName, NULL, qtssAttrDataTypeCharArray);
	(void)QTSS_IDForAttr(qtssRelayOutputObjectType, sOutputLocalAddrName, &sOutputLocalAddr);   // interface addr

	(void)QTSS_AddStaticAttribute(qtssRelayOutputObjectType, sOutputUDPPortsName, NULL, qtssAttrDataTypeUInt16);
	(void)QTSS_IDForAttr(qtssRelayOutputObjectType, sOutputUDPPortsName, &sOutputUDPPorts); // udp ports

	(void)QTSS_AddStaticAttribute(qtssRelayOutputObjectType, sOutputRTSPPortName, NULL, qtssAttrDataTypeUInt16);
	(void)QTSS_IDForAttr(qtssRelayOutputObjectType, sOutputRTSPPortName, &sOutputRTSPPort); // rtsp port

	(void)QTSS_AddStaticAttribute(qtssRelayOutputObjectType, sOutputURLName, NULL, qtssAttrDataTypeCharArray);
	(void)QTSS_IDForAttr(qtssRelayOutputObjectType, sOutputURLName, &sOutputURL);               // url

	(void)QTSS_AddStaticAttribute(qtssRelayOutputObjectType, sOutputTTLName, NULL, qtssAttrDataTypeUInt16);
	(void)QTSS_IDForAttr(qtssRelayOutputObjectType, sOutputTTLName, &sOutputTTL);               // ttl

	(void)QTSS_AddStaticAttribute(qtssRelayOutputObjectType, sOutputCurPacketsPerSecName, NULL, qtssAttrDataTypeUInt32);
	(void)QTSS_IDForAttr(qtssRelayOutputObjectType, sOutputCurPacketsPerSecName, &sOutputCurPacketsPerSec);// cur packets/sec

	(void)QTSS_AddStaticAttribute(qtssRelayOutputObjectType, sOutputCurBitsPerSecName, NULL, qtssAttrDataTypeUInt32);
	(void)QTSS_IDForAttr(qtssRelayOutputObjectType, sOutputCurBitsPerSecName, &sOutputCurBitsPerSec);   // cur bits/sec

	(void)QTSS_AddStaticAttribute(qtssRelayOutputObjectType, sOutputTotalPacketsSentName, NULL, qtssAttrDataTypeUInt64);
	(void)QTSS_IDForAttr(qtssRelayOutputObjectType, sOutputTotalPacketsSentName, &sOutputTotalPacketsSent);// total packets

	(void)QTSS_AddStaticAttribute(qtssRelayOutputObjectType, sOutputTotalBytesSentName, NULL, qtssAttrDataTypeUInt64);
	(void)QTSS_IDForAttr(qtssRelayOutputObjectType, sOutputTotalBytesSentName, &sOutputTotalBytesSent); // total bytes

}

RelayOutput::RelayOutput(SourceInfo* inInfo, UInt32 inWhichOutput, RelaySession* inRelaySession, Bool16 isRTSPSourceInfo)
	: fRelaySession(inRelaySession),
	fOutputSocket(NULL, Socket::kNonBlockingSocketType),
	fNumStreams(inRelaySession->GetSourceInfo()->GetNumStreams()),  // use the reflector session's source info
	fOutputInfo(*inInfo->GetOutputInfo(inWhichOutput)),
	fQueueElem(),
	fFormatter(&fHTMLBuf[0], kMaxHTMLSize),
	fPacketsPerSecond(0),
	fBitsPerSecond(0),
	fLastUpdateTime(0),
	fTotalPacketsSent(0),
	fTotalBytesSent(0),
	fLastPackets(0),
	fLastBytes(0),
	fClientSocket(NULL),
	fClient(NULL),
	fDoingAnnounce(false),
	fValid(true),
	fOutgoingSDP(NULL),
	fAnnounceTask(NULL),
	fRTSPOutputInfo(NULL)
{
	Assert(fNumStreams > 0);

	fQueueElem.SetEnclosingObject(this);
	fStreamCookieArray = NEW void*[fNumStreams];
	fTrackIDArray = NEW UInt32[fNumStreams];
	fOutputInfo.fPortArray = NEW UInt16[fNumStreams];//copy constructor doesn't do this
	::memset(fOutputInfo.fPortArray, 0, fNumStreams * sizeof(UInt16));

	// create a bookmark for each stream we'll reflect
	this->InititializeBookmarks(inRelaySession->GetNumStreams());

	// Copy out all the track IDs for each stream
	for (UInt32 x = 0; x < fNumStreams; x++)
	{
		fTrackIDArray[x] = inRelaySession->GetSourceInfo()->GetStreamInfo(x)->fTrackID;
		fStreamCookieArray[x] = inRelaySession->GetStreamCookie(fTrackIDArray[x]);
	}

	// Copy the contents of the output port array

	if (inInfo->GetOutputInfo(inWhichOutput)->fPortArray != NULL)
	{
		UInt32 copySize = fNumStreams;
		if (fOutputInfo.fNumPorts < fNumStreams)
			copySize = fOutputInfo.fNumPorts;
		::memcpy(fOutputInfo.fPortArray, inInfo->GetOutputInfo(inWhichOutput)->fPortArray, copySize * sizeof(UInt16));
	}
	else if (fOutputInfo.fBasePort != 0)
	{
		for (UInt32 y = 0; y < fNumStreams; y++)
			fOutputInfo.fPortArray[y] = (UInt16)(fOutputInfo.fBasePort + (y * 2));
	}

	OS_Error err = BindSocket();
	if (err != OS_NoErr)
	{
		fValid = false;
		return;
	}

	RTSPOutputInfo* rtspInfo = NULL;
	if (isRTSPSourceInfo)
	{
		// in the case of the announce source info, the passed in source info will have the new
		// output information, but only the session's source info will have the sdp and url.
		RTSPSourceInfo* rtspSourceInfo = (RTSPSourceInfo *)(inInfo);
		Assert(rtspSourceInfo != NULL);
		RTSPSourceInfo* sessionSourceInfo = (RTSPSourceInfo *)(inRelaySession->GetSourceInfo());
		Assert(sessionSourceInfo != NULL);

		rtspInfo = rtspSourceInfo->GetRTSPOutputInfo(inWhichOutput);
		if (rtspInfo->fIsAnnounced)
		{
			fRTSPOutputInfo = NEW RTSPOutputInfo();
			fRTSPOutputInfo->Copy(*rtspInfo);

			fDoingAnnounce = true;
			// set up rtsp socket and client
			fClientSocket = new TCPClientSocket(Socket::kNonBlockingSocketType);
			fClient = new RTSPClient(fClientSocket, false, RelaySession::sRelayUserAgent);

			// set up the outgoing socket
			fClientSocket->Set(fOutputInfo.fDestAddr, rtspInfo->fAnnouncePort);
			int sndBufSize = 32 * 1024;
			int rcvBufSize = 1024;
			fClientSocket->SetOptions(sndBufSize, rcvBufSize);

			// set up the client object
			StrPtrLen url;
			if ((rtspInfo->fDestURl != NULL) && (strlen(rtspInfo->fDestURl) > 0))
				url.Set(rtspInfo->fDestURl);
			else
				url.Set(sessionSourceInfo->GetSourceURL());

			fClient->Set(url);

			fClient->SetTransportMode(RTSPClient::kPushMode);
			fClient->SetName(rtspInfo->fUserName);
			fClient->SetPassword(rtspInfo->fPassword);

			UInt32 len;
			fOutgoingSDP = sessionSourceInfo->GetAnnounceSDP(fOutputInfo.fDestAddr, &len);
			fAnnounceState = kSendingAnnounce;
			fCurrentSetup = 0;
			fAnnounceTask = new RelayAnnouncer(this);    // this will now go and run the async announce
			fAnnounceTask->Signal(Task::kStartEvent);
		}
	}

	// Write the Output HTML
	// Looks like: Relaying to: 229.49.52.102, Ports: 16898 16900 Time to live: 15
	static StrPtrLen sHTMLStart("Relaying to: ");
	static StrPtrLen sPorts(", Ports: ");
	static StrPtrLen sTimeToLive(" Time to live: ");
	static StrPtrLen sHTMLEnd("<BR>");

	// First, format the destination addr as a dotted decimal string
	char theIPAddrBuf[20];
	StrPtrLen theIPAddr(theIPAddrBuf, 20);
	struct in_addr theAddr;
	theAddr.s_addr = htonl(fOutputInfo.fDestAddr);
	SocketUtils::ConvertAddrToString(theAddr, &theIPAddr);

	// Begin writing the HTML
	fFormatter.Put(sHTMLStart);
	fFormatter.Put(theIPAddr);
	fFormatter.Put(sPorts);

	for (UInt32 y = 0; y < fNumStreams; y++)
	{
		// Write all the destination ports
		fFormatter.Put(fOutputInfo.fPortArray[y]);
		fFormatter.PutSpace();
	}

	if (SocketUtils::IsMulticastIPAddr(inInfo->GetOutputInfo(inWhichOutput)->fDestAddr))
	{
		// Put the time to live if this is a multicast destination
		fFormatter.Put(sTimeToLive);
		fFormatter.Put(fOutputInfo.fTimeToLive);
	}
	fFormatter.Put(sHTMLEnd);

	// Setup the StrPtrLen to point to the right stuff
	fOutputInfoHTML.Ptr = fFormatter.GetBufPtr();
	fOutputInfoHTML.Len = fFormatter.GetCurrentOffset();

	OSMutexLocker locker(&sQueueMutex);
	sRelayOutputQueue.EnQueue(&fQueueElem);

	SetupRelayOutputObject(rtspInfo);
}

RelayOutput::~RelayOutput()
{
	OSMutexLocker locker(&sQueueMutex);
	sRelayOutputQueue.Remove(&fQueueElem);

	if (fClientSocket)
		delete fClientSocket;
	if (fClient)
		delete fClient;

	delete[] fStreamCookieArray;

	delete fOutgoingSDP;
	fOutgoingSDP = NULL;

	if (fAnnounceTask != NULL)
		fAnnounceTask->fOutput = NULL;

	if (fRTSPOutputInfo != NULL)
		delete fRTSPOutputInfo;

	QTSS_Object outputObject;
	UInt32 len = sizeof(QTSS_Object);

	for (int x = 0; QTSS_GetValue(fRelaySessionObject, RelaySession::sRelayOutputObject, x, &outputObject, &len) == QTSS_NoErr; x++)
	{
		Assert(outputObject != NULL);
		Assert(len == sizeof(QTSS_Object));

		if (outputObject == fRelayOutputObject)
		{
			(void)QTSS_RemoveValue(fRelaySessionObject, RelaySession::sRelayOutputObject, x);
			break;
		}
	}
}


OS_Error RelayOutput::BindSocket()
{
	OS_Error theErr = fOutputSocket.Open();
	if (theErr != OS_NoErr)
		return theErr;

	// We don't care what local port we bind to
	theErr = fOutputSocket.Bind(fOutputInfo.fLocalAddr, 0);
	if (theErr != OS_NoErr)
		return theErr;

	// Set the ttl to be the proper value
	return fOutputSocket.SetTtl(fOutputInfo.fTimeToLive);
}

Bool16  RelayOutput::Equal(SourceInfo* inInfo)
{
	// First check if the Source Info matches this RelaySession
	if (!fRelaySession->Equal(inInfo))
		return false;
	for (UInt32 x = 0; x < inInfo->GetNumOutputs(); x++)
	{
		if (inInfo->GetOutputInfo(x)->Equal(fOutputInfo))
		{
			RTSPOutputInfo* rtspOutputInfo = NULL;
			if (inInfo->IsRTSPSourceInfo())
			{
				rtspOutputInfo = ((RTSPSourceInfo*)inInfo)->GetRTSPOutputInfo(x);
				if (!rtspOutputInfo->fIsAnnounced)
					rtspOutputInfo = NULL;
			}

			if (fRTSPOutputInfo != NULL) // announced output
			{
				if (!fRTSPOutputInfo->Equal(rtspOutputInfo)) // doesn't match the output
					continue;
			}
			else if (rtspOutputInfo != NULL)
				continue;

			// This is a rather special purpose function... here we set this
			// flag marking this particular output as a duplicate, because
			// we know it is equal to this object.
			// (This is used in QTSSReflectorModule.cpp:RereadRelayPrefs)
			inInfo->GetOutputInfo(x)->fAlreadySetup = true;
			return true;
		}
	}
	return false;
}

QTSS_Error  RelayOutput::WritePacket(StrPtrLen* inPacket, void* inStreamCookie, UInt32 inFlags, SInt64 /*packetLatenessInMSec*/, SInt64* /*timeToSendThisPacketAgain*/, UInt64* packetIDPtr, SInt64* /*arrivalTimeMSec*/, Bool16 /*firstPacket */)
{

	if (!fValid || fDoingAnnounce)
		return OS_NoErr;    // Not done setting up or we had an error setting up

	// we don't use packetLateness becuase relays don't need to worry about TCP  flow control induced transmit delay

	// Look for the matching streamID
	for (UInt32 x = 0; x < fNumStreams; x++)
	{
		if (inStreamCookie == fStreamCookieArray[x])
		{
			UInt16 theDestPort = fOutputInfo.fPortArray[x];
			Assert((theDestPort & 1) == 0); //this should always be an RTP port (even)
			if (inFlags & qtssWriteFlagsIsRTCP)
				theDestPort++;

			(void)fOutputSocket.SendTo(fOutputInfo.fDestAddr, theDestPort,
				inPacket->Ptr, inPacket->Len);

			// Update our totals
			fTotalPacketsSent++;
			fTotalBytesSent += inPacket->Len;
			break;
		}
	}

	// If it is time to recalculate statistics, do so
	SInt64 curTime = OS::Milliseconds();
	if ((fLastUpdateTime + kStatsIntervalInMilSecs) < curTime)
	{
		// Update packets per second
		Float64 packetsPerSec = (Float64)((SInt64)fTotalPacketsSent - (SInt64)fLastPackets);
		packetsPerSec *= 1000;
		packetsPerSec /= (Float64)(curTime - fLastUpdateTime);
		fPacketsPerSecond = (UInt32)packetsPerSec;

		// Update bits per second. Win32 doesn't implement UInt64 -> Float64.
		Float64 bitsPerSec = (Float64)((SInt64)fTotalBytesSent - (SInt64)fLastBytes);
		bitsPerSec *= 1000 * 8;//convert from seconds to milsecs, bytes to bits
		bitsPerSec /= (Float64)(curTime - fLastUpdateTime);
		fBitsPerSecond = (UInt32)bitsPerSec;

		fLastUpdateTime = curTime;
		fLastPackets = fTotalPacketsSent;
		fLastBytes = fTotalBytesSent;
	}

	return QTSS_NoErr;
}

SInt64 RelayOutput::RelayAnnouncer::Run()
{
	OSMutexLocker locker(RelayOutput::GetQueueMutex());
	SInt64 result = -1;
	if (fOutput != NULL)
		result = fOutput->RunAnnounce();

	return result;
}

SInt64 RelayOutput::RunAnnounce()
{
	OS_Error err = OS_NoErr;
	SInt64 result = 1000;

	if (fAnnounceState == kSendingAnnounce)
	{
		if (fOutgoingSDP == NULL || ::strlen(fOutgoingSDP) == 0)
			err = ENOTCONN;
		else
		{
			err = fClient->SendAnnounce(fOutgoingSDP);
			if (err == OS_NoErr)
			{
				delete fOutgoingSDP;
				fOutgoingSDP = NULL;
				if (fClient->GetStatus() == 200)
					fAnnounceState = kSendingSetup;
				else
					err = ENOTCONN;
			}
		}
	}

	while ((fAnnounceState == kSendingSetup) && (err == OS_NoErr))
	{
		err = fClient->SendUDPSetup(fTrackIDArray[fCurrentSetup], 10000);
		if (err == OS_NoErr)
		{
			if (fClient->GetStatus() == 200)
			{
				fOutputInfo.fPortArray[fCurrentSetup] = fClient->GetServerPort();   // this got set from the Setup reply
				fCurrentSetup++;
				if (fCurrentSetup == fNumStreams)
					fAnnounceState = kSendingPlay;
			}
			else
				err = ENOTCONN;
		}
	}

	if (fAnnounceState == kSendingPlay)
	{
		err = fClient->SendPlay(0);
		if (err == OS_NoErr)
		{
			if (fClient->GetStatus() == 200)
				fAnnounceState = kDone;
			else
				err = ENOTCONN;
		}
	}

	if (fAnnounceState == kDone)
	{
		for (UInt32 index = 0; index < fNumStreams; index++)    // source udp ports
		{
			UInt16 udpPort = fOutputInfo.fPortArray[index];
			err = QTSS_SetValue(fRelayOutputObject, sOutputUDPPorts, index, &udpPort, sizeof(udpPort));
			Assert(err == QTSS_NoErr);
		}

		fDoingAnnounce = false;
		result = -1;    // let the task die
		fAnnounceTask = NULL;
	}

	if ((err == EINPROGRESS) || (err == EAGAIN))
	{
		// Request an async event
		fClientSocket->GetSocket()->SetTask(fAnnounceTask);
		fClientSocket->GetSocket()->RequestEvent(fClientSocket->GetEventMask());
	}
	else if (err != OS_NoErr)
	{
		// We encountered some fatal error with the socket. Record this as a connection failure
		fValid = false;
		result = -1;    // let the task die
		fAnnounceTask = NULL;
	}

	if ((-1 == result) && (NULL != fClientSocket) && (NULL != fClientSocket->GetSocket()))
		fClientSocket->GetSocket()->SetTask(NULL); //remove fAnnounceTask from event handling code. The task can be safely deleted after detaching from the socket.

	return result;
}

void RelayOutput::SetupRelayOutputObject(RTSPOutputInfo* inRTSPInfo)
{
	fRelaySessionObject = fRelaySession->GetRelaySessionObject();
	UInt32 outIndex = 0;

	QTSS_Error theErr = QTSS_LockObject(fRelaySessionObject);
	Assert(theErr == QTSS_NoErr);

	theErr = QTSS_CreateObjectValue(fRelaySessionObject, RelaySession::sRelayOutputObject, qtssRelayOutputObjectType, &outIndex, &fRelayOutputObject);
	Assert(theErr == QTSS_NoErr);

	if ((inRTSPInfo == NULL) || !inRTSPInfo->fIsAnnounced)          // output type
	{
		theErr = QTSS_SetValue(fRelayOutputObject, sOutputType, 0, (void*)sUDPDestStr.Ptr, sUDPDestStr.Len);
		Assert(theErr == QTSS_NoErr);
	}
	else
	{
		theErr = QTSS_SetValue(fRelayOutputObject, sOutputType, 0, (void*)sAnnouncedDestStr.Ptr, sAnnouncedDestStr.Len); // output type
		Assert(theErr == QTSS_NoErr);

		theErr = QTSS_SetValue(fRelayOutputObject, sOutputRTSPPort, 0, &inRTSPInfo->fAnnouncePort, sizeof(inRTSPInfo->fAnnouncePort)); // rtsp port
		Assert(theErr == QTSS_NoErr);

		theErr = QTSS_SetValue(fRelayOutputObject, sOutputURL, 0, (fClient->GetURL())->Ptr, (fClient->GetURL())->Len); // output url
		Assert(theErr == QTSS_NoErr);
	}

	char theIPAddrBuf[20];
	StrPtrLen theIPAddr(theIPAddrBuf, 20);

	struct in_addr theDestAddr;     // output destination address
	theDestAddr.s_addr = htonl(fOutputInfo.fDestAddr);
	SocketUtils::ConvertAddrToString(theDestAddr, &theIPAddr);

	theErr = QTSS_SetValue(fRelayOutputObject, sOutputDestAddr, 0, (void*)theIPAddr.Ptr, theIPAddr.Len);
	Assert(theErr == QTSS_NoErr);

	struct in_addr theLocalAddr;        // output local address
	theLocalAddr.s_addr = htonl(fOutputInfo.fLocalAddr);
	SocketUtils::ConvertAddrToString(theLocalAddr, &theIPAddr);

	theErr = QTSS_SetValue(fRelayOutputObject, sOutputLocalAddr, 0, (void*)theIPAddr.Ptr, theIPAddr.Len);
	Assert(theErr == QTSS_NoErr);


	for (UInt32 index = 0; index < fNumStreams; index++)    // output udp ports
	{
		UInt16 udpPort = fOutputInfo.fPortArray[index];
		theErr = QTSS_SetValue(fRelayOutputObject, sOutputUDPPorts, index, &udpPort, sizeof(udpPort));
		Assert(theErr == QTSS_NoErr);
	}

	theErr = QTSS_SetValue(fRelayOutputObject, sOutputTTL, 0, &(fOutputInfo.fTimeToLive), sizeof(fOutputInfo.fTimeToLive));
	Assert(theErr == QTSS_NoErr);

	theErr = QTSS_SetValuePtr(fRelayOutputObject, sOutputCurPacketsPerSec, &fPacketsPerSecond, sizeof(fPacketsPerSecond));
	Assert(theErr == QTSS_NoErr);

	theErr = QTSS_SetValuePtr(fRelayOutputObject, sOutputCurBitsPerSec, &fBitsPerSecond, sizeof(fBitsPerSecond));
	Assert(theErr == QTSS_NoErr);

	theErr = QTSS_SetValuePtr(fRelayOutputObject, sOutputTotalPacketsSent, &fTotalPacketsSent, sizeof(fTotalPacketsSent));
	Assert(theErr == QTSS_NoErr);

	theErr = QTSS_SetValuePtr(fRelayOutputObject, sOutputTotalBytesSent, &fTotalBytesSent, sizeof(fTotalBytesSent));
	Assert(theErr == QTSS_NoErr);

	theErr = QTSS_UnlockObject(fRelaySessionObject);
	Assert(theErr == QTSS_NoErr);

}
