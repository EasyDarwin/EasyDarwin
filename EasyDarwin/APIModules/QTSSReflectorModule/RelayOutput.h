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
	 File:       RelayOutput.h

	 Contains:   An implementation of the ReflectorOutput abstract base class,
				 that just writes data out to UDP sockets.

 */

#ifndef __RELAY_OUTPUT_H__
#define __RELAY_OUTPUT_H__

#include "ReflectorOutput.h"
#include "RelaySession.h"
#include "SourceInfo.h"
#include "RTSPClient.h"
#include "ClientSocket.h"

#include "OSQueue.h"
#include "OSMutex.h"

class RTSPOutputInfo;
class RelayAnnouncer;

class RelayOutput : public ReflectorOutput
{
public:

	// Call Register in the Relay Module's Register Role
	static void Register();

	RelayOutput(SourceInfo* inInfo, UInt32 inWhichOutput, RelaySession* inSession, bool isRTSPSourceInfo);
	virtual ~RelayOutput();

	// Returns true if this output matches one of the Outputs in the SourceInfo.
	// Also marks the proper SourceInfo::OutputInfo "fAlreadySetup" flag as true 
	bool  Equal(SourceInfo* inInfo);

	// Call this to setup this object's output socket
	OS_Error BindSocket();

	// Writes the packet directly to a UDP socket
	virtual QTSS_Error  WritePacket(StrPtrLen* inPacket, void* inStreamCookie, UInt32 inFlags, SInt64 packetLatenessInMSec, SInt64* timeToSendThisPacketAgain, UInt64* packetIDPtr, SInt64* arrivalTime, bool firstPacket);

	virtual bool              IsUDP() { return true; }

	virtual bool              IsPlaying() { if (!fValid || fDoingAnnounce) return false; return true; }

	// ACCESSORS

	RelaySession*   GetRelaySession() { return fRelaySession; }
	StrPtrLen*          GetOutputInfoHTML() { return &fOutputInfoHTML; }

	UInt32              GetCurPacketsPerSecond() { return fPacketsPerSecond; }
	UInt32              GetCurBitsPerSecond() { return fBitsPerSecond; }
	UInt64&             GetTotalPacketsSent() { return fTotalPacketsSent; }
	UInt64&             GetTotalBytesSent() { return fTotalBytesSent; }
	bool              IsValid() { return fValid; }

	// Use these functions to iterate over all RelayOutputs
	static OSMutex* GetQueueMutex() { return &sQueueMutex; }
	static OSQueue* GetOutputQueue() { return &sRelayOutputQueue; }
	void TearDown() {};

	SInt64 RunAnnounce();

private:

	void SetupRelayOutputObject(RTSPOutputInfo* inRTSPInfo);

	class RelayAnnouncer : public Task
	{
	public:
		RelayAnnouncer(RelayOutput* output) : fOutput(output) { this->SetTaskName("RelayAnnouncer"); }

		virtual SInt64 Run();
		RelayOutput* fOutput;
	};

	enum
	{
		kMaxHTMLSize = 255, // Note, this may be too short and we don't protect!
		kStatsIntervalInMilSecs = 10000 // Update "current" statistics every 10 seconds
	};

	RelaySession* fRelaySession;

	// Relay streams all share this one socket for writing.
	UDPSocket   fOutputSocket;
	UInt32      fNumStreams;
	SourceInfo::OutputInfo fOutputInfo;
	void**      fStreamCookieArray;//Each stream has a cookie
	UInt32*     fTrackIDArray;

	OSQueueElem fQueueElem;

	char        fHTMLBuf[kMaxHTMLSize];
	StrPtrLen   fOutputInfoHTML;
	ResizeableStringFormatter fFormatter;

	// Statistics
	UInt32      fPacketsPerSecond;
	UInt32      fBitsPerSecond;

	SInt64      fLastUpdateTime;
	UInt64      fTotalPacketsSent;
	UInt64      fTotalBytesSent;
	UInt64      fLastPackets;
	UInt64      fLastBytes;

	TCPClientSocket* fClientSocket;
	EasyDarwin::RTSPClient* fClient;
	bool      fDoingAnnounce;
	bool      fValid;
	char*       fOutgoingSDP;
	RelayAnnouncer* fAnnounceTask;

	RTSPOutputInfo* fRTSPOutputInfo;

	enum    // anounce states
	{
		kSendingAnnounce = 0,
		kSendingSetup = 1,
		kSendingPlay = 2,
		kDone = 3
	};
	UInt32          fAnnounceState;
	UInt32          fCurrentSetup;

	// Queue of all current RelayReflectorOutput objects, for use in the        
	static OSQueue  sRelayOutputQueue;
	static OSMutex  sQueueMutex;

	QTSS_Object                 fRelaySessionObject;
	QTSS_Object                 fRelayOutputObject;

	// attributes of the qtssRelayOutputObjectType
	static QTSS_ObjectType          qtssRelayOutputObjectType;

	static QTSS_AttributeID         sOutputType;
	static QTSS_AttributeID         sOutputDestAddr;
	static QTSS_AttributeID         sOutputLocalAddr;
	static QTSS_AttributeID         sOutputUDPPorts;
	static QTSS_AttributeID         sOutputRTSPPort;
	static QTSS_AttributeID         sOutputURL;
	static QTSS_AttributeID         sOutputTTL;
	static QTSS_AttributeID         sOutputCurPacketsPerSec;
	static QTSS_AttributeID         sOutputCurBitsPerSec;
	static QTSS_AttributeID         sOutputTotalPacketsSent;
	static QTSS_AttributeID         sOutputTotalBytesSent;

};


#endif //__RELAY_OUTPUT_H__
