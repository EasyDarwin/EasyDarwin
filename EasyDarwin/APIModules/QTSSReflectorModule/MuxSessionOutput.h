/*
	Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
	File:       MuxSessionOutput.h
	Contains:   For Mux HLS or MP4
*/

#ifndef __MUX_SESSION_OUTPUT_H__
#define __MUX_SESSION_OUTPUT_H__

#include "ReflectorOutput.h"
#include "ReflectorSession.h"
#include "QTSS.h"

class MuxSessionOutput : public ReflectorOutput
{
public:

	// Adds some dictionary attributes
	static void Register();

	MuxSessionOutput(QTSS_ClientSessionObject inRTPSession, ReflectorSession* inReflectorSession,
		QTSS_Object serverPrefs, QTSS_AttributeID inCookieAddrID);
	virtual ~MuxSessionOutput() {}

	ReflectorSession* GetReflectorSession() { return fReflectorSession; }
	void InitializeStreams();

	// This writes the packet out to the proper QTSS_RTPStreamObject.
	// If this function returns QTSS_WouldBlock, timeToSendThisPacketAgain will
	// be set to # of msec in which the packet can be sent, or -1 if unknown
	virtual QTSS_Error  WritePacket(StrPtrLen* inPacketData, void* inStreamCookie, UInt32 inFlags, SInt64 packetLatenessInMSec, SInt64* timeToSendThisPacketAgain, UInt64* packetIDPtr, SInt64* arrivalTimeMSec, Bool16 firstPacket);
	virtual void TearDown();

	SInt64  GetReflectorSessionInitTime() { return fReflectorSession->GetInitTimeMS(); }

	virtual Bool16  IsUDP();

	virtual Bool16  IsPlaying();

	void SetBufferDelay(UInt32 delay) { fBufferDelayMSecs = delay; }

private:

	QTSS_ClientSessionObject fClientSession;
	ReflectorSession*       fReflectorSession;
	QTSS_AttributeID        fCookieAttrID;
	UInt32                  fBufferDelayMSecs;
	SInt64                  fBaseArrivalTime;
	Bool16                  fIsUDP;
	Bool16                  fTransportInitialized;
	Bool16                  fMustSynch;
	Bool16                  fPreFilter;

	UInt16 GetPacketSeqNumber(StrPtrLen* inPacket);
	void SetPacketSeqNumber(StrPtrLen* inPacket, UInt16 inSeqNumber);
	Bool16  FilterPacket(QTSS_RTPStreamObject *theStreamPtr, StrPtrLen* inPacket);

	UInt32 GetPacketRTPTime(StrPtrLen* packetStrPtr);
	inline  Bool16 PacketMatchesStream(void* inStreamCookie, QTSS_RTPStreamObject *theStreamPtr);
	Bool16 PacketReadyToSend(QTSS_RTPStreamObject *theStreamPtr, SInt64 *currentTimePtr, UInt32 inFlags, UInt64* packetIDPtr, SInt64* timeToSendThisPacketAgainPtr);
	Bool16 PacketAlreadySent(QTSS_RTPStreamObject *theStreamPtr, UInt32 inFlags, UInt64* packetIDPtr);
};


Bool16 MuxSessionOutput::PacketMatchesStream(void* inStreamCookie, QTSS_RTPStreamObject *theStreamPtr)
{
	void** theStreamCookie = NULL;
	UInt32 theLen = 0;
	(void)QTSS_GetValuePtr(*theStreamPtr, fCookieAttrID, 0, (void**)&theStreamCookie, &theLen);

	if ((theStreamCookie != NULL) && (*theStreamCookie == inStreamCookie))
		return true;

	return false;
}
#endif //__MUX_REFLECTOR_OUTPUT_H__
