/*
	Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
 /*
	 File:       MuxSessionOutput.cpp
	 Contains:   Implementation of object in .h file
 */

#include "MuxSessionOutput.h"
#include "ReflectorStream.h"
#include <errno.h>

#if DEBUG 
#define MUX_SESSION_DEBUGGING 0
#else
#define MUX_SESSION_DEBUGGING 0
#endif

 // ATTRIBUTES
static QTSS_AttributeID     sStreamPacketCountAttr = qtssIllegalAttrID;


static QTSS_AttributeID     sNextSeqNumAttr = qtssIllegalAttrID;
static QTSS_AttributeID     sSeqNumOffsetAttr = qtssIllegalAttrID;
static QTSS_AttributeID     sLastQualityChangeAttr = qtssIllegalAttrID;
static QTSS_AttributeID     sLastRTPPacketIDAttr = qtssIllegalAttrID;
static QTSS_AttributeID     sLastRTCPPacketIDAttr = qtssIllegalAttrID;

static QTSS_AttributeID     sFirstRTCPCurrentTimeAttr = qtssIllegalAttrID;
static QTSS_AttributeID     sFirstRTCPArrivalTimeAttr = qtssIllegalAttrID;
static QTSS_AttributeID     sFirstRTCPTimeStampAttr = qtssIllegalAttrID;

static QTSS_AttributeID     sFirstRTPCurrentTimeAttr = qtssIllegalAttrID;
static QTSS_AttributeID     sFirstRTPArrivalTimeAttr = qtssIllegalAttrID;
static QTSS_AttributeID     sFirstRTPTimeStampAttr = qtssIllegalAttrID;

static QTSS_AttributeID     sBaseRTPTimeStampAttr = qtssIllegalAttrID;

static QTSS_AttributeID     sBaseArrivalTimeStampAttr = qtssIllegalAttrID;
static QTSS_AttributeID     sStreamSSRCAttr = qtssIllegalAttrID;

static QTSS_AttributeID     sStreamByteCountAttr = qtssIllegalAttrID;

static QTSS_AttributeID     sLastRTPTimeStampAttr = qtssIllegalAttrID;

static QTSS_AttributeID     sLastRTCPTransmitAttr = qtssIllegalAttrID;

MuxSessionOutput::MuxSessionOutput(QTSS_ClientSessionObject inClientSession, ReflectorSession* inReflectorSession,
	QTSS_Object serverPrefs, QTSS_AttributeID inCookieAddrID)
	: fClientSession(inClientSession),
	fReflectorSession(inReflectorSession),
	fCookieAttrID(inCookieAddrID),
	fBufferDelayMSecs(ReflectorStream::sOverBufferInMsec),
	fBaseArrivalTime(0),
	fIsUDP(false),
	fTransportInitialized(false),
	fMustSynch(true),
	fPreFilter(true)
{
	// create a bookmark for each stream we'll reflect
	this->InititializeBookmarks(inReflectorSession->GetNumStreams());

}

void MuxSessionOutput::Register()
{
	// Add some attributes to QTSS_RTPStream dictionary 
	static char*        sNextSeqNum = "qtssNextSeqNum";
	static char*        sSeqNumOffset = "qtssSeqNumOffset";
	static char*        sLastQualityChange = "qtssLastQualityChange";

	static char*        sLastRTPPacketID = "qtssReflectorStreamLastRTPPacketID";
	static char*        sLastRTCPPacketID = "qtssReflectorStreamLastRTCPPacketID";

	static char*        sFirstRTCPArrivalTime = "qtssReflectorStreamStartRTCPArrivalTime";
	static char*        sFirstRTCPTimeStamp = "qtssReflectorStreamStartRTCPTimeStamp";
	static char*        sFirstRTCPCurrentTime = "qtssReflectorStreamStartRTCPCurrent";

	static char*        sFirstRTPArrivalTime = "qtssReflectorStreamStartRTPArrivalTime";
	static char*        sFirstRTPTimeStamp = "qtssReflectorStreamStartRTPTimeStamp";
	static char*        sFirstRTPCurrentTime = "qtssReflectorStreamStartRTPCurrent";

	static char*        sBaseRTPTimeStamp = "qtssReflectorStreamBaseRTPTimeStamp";
	static char*        sBaseArrivalTimeStamp = "qtssReflectorStreamBaseArrivalTime";

	static char*        sLastRTPTimeStamp = "qtssReflectorStreamLastRTPTimeStamp";
	static char*        sLastRTCPTransmit = "qtssReflectorStreamLastRTCPTransmit";

	static char*        sStreamSSRC = "qtssReflectorStreamSSRC";
	static char*        sStreamPacketCount = "qtssReflectorStreamPacketCount";
	static char*        sStreamByteCount = "qtssReflectorStreamByteCount";

	(void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sLastRTCPTransmit, NULL, qtssAttrDataTypeUInt16);
	(void)QTSS_IDForAttr(qtssRTPStreamObjectType, sLastRTCPTransmit, &sLastRTCPTransmitAttr);

	(void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sNextSeqNum, NULL, qtssAttrDataTypeUInt16);
	(void)QTSS_IDForAttr(qtssRTPStreamObjectType, sNextSeqNum, &sNextSeqNumAttr);

	(void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sSeqNumOffset, NULL, qtssAttrDataTypeUInt16);
	(void)QTSS_IDForAttr(qtssRTPStreamObjectType, sSeqNumOffset, &sSeqNumOffsetAttr);

	(void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sLastQualityChange, NULL, qtssAttrDataTypeSInt64);
	(void)QTSS_IDForAttr(qtssRTPStreamObjectType, sLastQualityChange, &sLastQualityChangeAttr);

	(void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sLastRTPPacketID, NULL, qtssAttrDataTypeUInt64);
	(void)QTSS_IDForAttr(qtssRTPStreamObjectType, sLastRTPPacketID, &sLastRTPPacketIDAttr);

	(void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sLastRTCPPacketID, NULL, qtssAttrDataTypeUInt64);
	(void)QTSS_IDForAttr(qtssRTPStreamObjectType, sLastRTCPPacketID, &sLastRTCPPacketIDAttr);

	(void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sLastRTPTimeStamp, NULL, qtssAttrDataTypeUInt32);
	(void)QTSS_IDForAttr(qtssRTPStreamObjectType, sLastRTPTimeStamp, &sLastRTPTimeStampAttr);

	(void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sFirstRTCPArrivalTime, NULL, qtssAttrDataTypeSInt64);
	(void)QTSS_IDForAttr(qtssRTPStreamObjectType, sFirstRTCPArrivalTime, &sFirstRTCPArrivalTimeAttr);

	(void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sFirstRTCPTimeStamp, NULL, qtssAttrDataTypeUInt32);
	(void)QTSS_IDForAttr(qtssRTPStreamObjectType, sFirstRTCPTimeStamp, &sFirstRTCPTimeStampAttr);

	(void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sFirstRTCPCurrentTime, NULL, qtssAttrDataTypeUInt32);
	(void)QTSS_IDForAttr(qtssRTPStreamObjectType, sFirstRTCPCurrentTime, &sFirstRTCPCurrentTimeAttr);

	(void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sFirstRTPCurrentTime, NULL, qtssAttrDataTypeSInt64);
	(void)QTSS_IDForAttr(qtssRTPStreamObjectType, sFirstRTPCurrentTime, &sFirstRTPCurrentTimeAttr);

	(void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sFirstRTPArrivalTime, NULL, qtssAttrDataTypeSInt64);
	(void)QTSS_IDForAttr(qtssRTPStreamObjectType, sFirstRTPArrivalTime, &sFirstRTPArrivalTimeAttr);

	(void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sFirstRTPTimeStamp, NULL, qtssAttrDataTypeUInt32);
	(void)QTSS_IDForAttr(qtssRTPStreamObjectType, sFirstRTPTimeStamp, &sFirstRTPTimeStampAttr);

	(void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sBaseRTPTimeStamp, NULL, qtssAttrDataTypeUInt32);
	(void)QTSS_IDForAttr(qtssRTPStreamObjectType, sBaseRTPTimeStamp, &sBaseRTPTimeStampAttr);

	(void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sBaseArrivalTimeStamp, NULL, qtssAttrDataTypeVoidPointer);
	(void)QTSS_IDForAttr(qtssRTPStreamObjectType, sBaseArrivalTimeStamp, &sBaseArrivalTimeStampAttr);

	(void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sStreamSSRC, NULL, qtssAttrDataTypeVoidPointer);
	(void)QTSS_IDForAttr(qtssRTPStreamObjectType, sStreamSSRC, &sStreamSSRCAttr);

	(void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sStreamPacketCount, NULL, qtssAttrDataTypeUInt32);
	(void)QTSS_IDForAttr(qtssRTPStreamObjectType, sStreamPacketCount, &sStreamPacketCountAttr);

	(void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sStreamByteCount, NULL, qtssAttrDataTypeUInt32);
	(void)QTSS_IDForAttr(qtssRTPStreamObjectType, sStreamByteCount, &sStreamByteCountAttr);

}

bool MuxSessionOutput::IsPlaying()
{
	QTSS_RTPSessionState*   theState = NULL;
	UInt32                  theLen = 0;

	if (!fClientSession)
		return false;

	(void)QTSS_GetValuePtr(fClientSession, qtssCliSesState, 0, (void**)&theState, &theLen);
	if (theLen == 0 || theState == NULL || *theState != qtssPlayingState)
		return false;


	return true;
}

void MuxSessionOutput::InitializeStreams()
{

	UInt32                  theLen = 0;
	QTSS_RTPStreamObject*   theStreamPtr = NULL;
	UInt32                  packetCountInitValue = 0;

	for (SInt16 z = 0; QTSS_GetValuePtr(fClientSession, qtssCliSesStreamObjects, z, (void**)&theStreamPtr, &theLen) == QTSS_NoErr; z++)
	{
		(void)QTSS_SetValue(*theStreamPtr, sStreamPacketCountAttr, 0, &packetCountInitValue, sizeof(UInt32));
	}

}



bool MuxSessionOutput::IsUDP()
{
	if (fTransportInitialized)
		return fIsUDP;


	QTSS_RTPSessionState*   theState = NULL;
	UInt32                  theLen = 0;
	(void)QTSS_GetValuePtr(fClientSession, qtssCliSesState, 0, (void**)&theState, &theLen);
	if (*theState != qtssPlayingState)
		return true;

	QTSS_RTPStreamObject *theStreamPtr = NULL;
	QTSS_RTPTransportType *theTransportTypePtr = NULL;
	for (SInt16 z = 0; QTSS_GetValuePtr(fClientSession, qtssCliSesStreamObjects, z, (void**)&theStreamPtr, &theLen) == QTSS_NoErr; z++)
	{
		(void)QTSS_GetValuePtr(*theStreamPtr, qtssRTPStrTransportType, 0, (void**)&theTransportTypePtr, &theLen);
		if (theTransportTypePtr && *theTransportTypePtr == qtssRTPTransportTypeUDP)
		{
			fIsUDP = true;
			break; // treat entire session UDP
		}
		else
		{
			fIsUDP = false;
		}
	}

	//if (fIsUDP) printf("MuxSessionOutput::MuxSessionOutput Standard UDP client\n");
	 //else printf("MuxSessionOutput::MuxSessionOutput Buffered Client\n");

	fTransportInitialized = true;
	return fIsUDP;
}


bool  MuxSessionOutput::FilterPacket(QTSS_RTPStreamObject *theStreamPtr, StrPtrLen* inPacket)
{

	UInt32* packetCountPtr = NULL;
	UInt32 theLen = 0;

	//see if we started sending and if so then just keep sending (reset on a play)
	QTSS_Error writeErr = QTSS_GetValuePtr(*theStreamPtr, sStreamPacketCountAttr, 0, (void**)&packetCountPtr, &theLen);
	if (writeErr == QTSS_NoErr && theLen > 0 && *packetCountPtr > 0)
		return false;

	Assert(theStreamPtr);
	Assert(inPacket);

	UInt16 seqnum = this->GetPacketSeqNumber(inPacket);
	UInt16 firstSeqNum = 0;
	theLen = sizeof(firstSeqNum);

	if (QTSS_NoErr != QTSS_GetValue(*theStreamPtr, qtssRTPStrFirstSeqNumber, 0, &firstSeqNum, &theLen))
		return true;

	if (seqnum < firstSeqNum)
	{
		//printf("MuxSessionOutput::FilterPacket don't send packet = %u < first=%lu\n", seqnum, firstSeqNum);
		return true;
	}

	//printf("MuxSessionOutput::FilterPacket found first packet = %u \n", firstSeqNum);

	fPreFilter = false;
	return fPreFilter;
}


bool  MuxSessionOutput::PacketAlreadySent(QTSS_RTPStreamObject *theStreamPtr, UInt32 inFlags, UInt64* packetIDPtr)
{
	Assert(theStreamPtr);
	Assert(packetIDPtr);

	UInt32 theLen = 0;
	UInt64 *lastPacketIDPtr = NULL;
	bool packetSent = false;

	if (inFlags & qtssWriteFlagsIsRTP)
	{
		if ((QTSS_NoErr == QTSS_GetValuePtr(*theStreamPtr, sLastRTPPacketIDAttr, 0, (void**)&lastPacketIDPtr, &theLen))
			&& (*packetIDPtr <= *lastPacketIDPtr)
			)
		{
			//printf("MuxSessionOutput::WritePacket Don't send RTP packet id =%qu\n", *packetIDPtr);
			packetSent = true;
		}

	}
	else if (inFlags & qtssWriteFlagsIsRTCP)
	{
		if (QTSS_NoErr == QTSS_GetValuePtr(*theStreamPtr, sLastRTCPPacketIDAttr, 0, (void**)&lastPacketIDPtr, &theLen)
			&& (*packetIDPtr <= *lastPacketIDPtr)
			)
		{
			//printf("MuxSessionOutput::WritePacket Don't send RTCP packet id =%qu last packet sent id =%qu\n", *packetIDPtr,*lastPacketIDPtr);
			packetSent = true;
		}
	}

	return packetSent;
}

bool  MuxSessionOutput::PacketReadyToSend(QTSS_RTPStreamObject *theStreamPtr, SInt64 *currentTimePtr, UInt32 inFlags, UInt64* packetIDPtr, SInt64* timeToSendThisPacketAgainPtr)
{
	return true;
}

QTSS_Error  MuxSessionOutput::WritePacket(StrPtrLen* inPacket, void* inStreamCookie, UInt32 inFlags, SInt64 packetLatenessInMSec, SInt64* timeToSendThisPacketAgain, UInt64* packetIDPtr, SInt64* arrivalTimeMSecPtr, bool firstPacket)
{
	QTSS_RTPSessionState*   theState = NULL;
	UInt32                  theLen = 0;
	QTSS_Error              writeErr = QTSS_NoErr;
	SInt64                  currentTime = OS::Milliseconds();

	if (inPacket == NULL || inPacket->Len == 0)
		return QTSS_NoErr;


	(void)QTSS_GetValuePtr(fClientSession, qtssCliSesState, 0, (void**)&theState, &theLen);
	if (theLen == 0 || theState == NULL || *theState != qtssPlayingState)
	{   //qtss_printf("QTSS_WouldBlock *theState=%d qtssPlayingState=%d\n", *theState , qtssPlayingState);
		return QTSS_WouldBlock;
	}

	//make sure all RTP streams with this ID see this packet
	QTSS_RTPStreamObject *theStreamPtr = NULL;

	for (UInt32 z = 0; QTSS_GetValuePtr(fClientSession, qtssCliSesStreamObjects, z, (void**)&theStreamPtr, &theLen) == QTSS_NoErr; z++)
	{
		if (this->PacketMatchesStream(inStreamCookie, theStreamPtr))
		{
			if ((inFlags & qtssWriteFlagsIsRTP) && this->FilterPacket(theStreamPtr, inPacket))
				return  QTSS_NoErr; // keep looking at packets

			if (this->PacketAlreadySent(theStreamPtr, inFlags, packetIDPtr))
				return QTSS_NoErr; // keep looking at packets

			if (!this->PacketReadyToSend(theStreamPtr, &currentTime, inFlags, packetIDPtr, timeToSendThisPacketAgain))
			{   //qtss_printf("QTSS_WouldBlock\n");
				return QTSS_WouldBlock; // stop not ready to send packets now
			}


			// TrackPackets below is for re-writing the rtcps we don't use it right now-- shouldn't need to    
			// (void) this->TrackPackets(theStreamPtr, inPacket, &currentTime,inFlags,  &packetLatenessInMSec, timeToSendThisPacketAgain, packetIDPtr,arrivalTimeMSecPtr);

			QTSS_PacketStruct thePacket;
			thePacket.packetData = inPacket->Ptr;
			SInt64 delayMSecs = fBufferDelayMSecs - (currentTime - *arrivalTimeMSecPtr);
			thePacket.packetTransmitTime = (currentTime - packetLatenessInMSec);
			if (fBufferDelayMSecs > 0)
				thePacket.packetTransmitTime += delayMSecs; // add buffer time where oldest buffered packet as now == 0 and newest is entire buffer time in the future.

			writeErr = QTSS_Write(*theStreamPtr, &thePacket, inPacket->Len, NULL, inFlags | qtssWriteFlagsWriteBurstBegin);
			if (writeErr == QTSS_WouldBlock)
			{
				//qtss_printf("QTSS_Write == QTSS_WouldBlock\n");
			   //
			   // We are flow controlled. See if we know when flow control will be lifted and report that
				*timeToSendThisPacketAgain = thePacket.suggestedWakeupTime;

				if (firstPacket)
				{
					fBufferDelayMSecs = (currentTime - *arrivalTimeMSecPtr);
					//qtss_printf("firstPacket fBufferDelayMSecs =%lu \n", fBufferDelayMSecs);
				}
			}
			else
			{
				fLastIntervalMilliSec = currentTime - fLastPacketTransmitTime;
				if (fLastIntervalMilliSec > 100) //reset interval maybe first packet or it has been blocked for awhile
					fLastIntervalMilliSec = 5;
				fLastPacketTransmitTime = currentTime;

				if (inFlags & qtssWriteFlagsIsRTP)
				{
					(void)QTSS_SetValue(*theStreamPtr, sLastRTPPacketIDAttr, 0, packetIDPtr, sizeof(UInt64));
				}
				else if (inFlags & qtssWriteFlagsIsRTCP)
				{
					(void)QTSS_SetValue(*theStreamPtr, sLastRTCPPacketIDAttr, 0, packetIDPtr, sizeof(UInt64));
					(void)QTSS_SetValue(*theStreamPtr, sLastRTCPTransmitAttr, 0, &currentTime, sizeof(UInt64));
				}



				{ // increment packet counts
					UInt32* packetCountPtr = NULL;
					UInt32 theLen = 0;

					(void)QTSS_GetValuePtr(*theStreamPtr, sStreamPacketCountAttr, 0, (void**)&packetCountPtr, &theLen);
					if (theLen > 0)
					{
						*packetCountPtr += 1;
						//printf("SET sStreamPacketCountAttr =%lu\n", *packetCountPtr);
					}
				}
			}
		}

		if (writeErr != QTSS_NoErr)
			break;
	}

	return writeErr;
}

UInt16 MuxSessionOutput::GetPacketSeqNumber(StrPtrLen* inPacket)
{
	if (inPacket->Len < 4)
		return 0;

	//The RTP seq number is the second short of the packet
	UInt16* seqNumPtr = (UInt16*)inPacket->Ptr;
	return ntohs(seqNumPtr[1]);
}

void MuxSessionOutput::SetPacketSeqNumber(StrPtrLen* inPacket, UInt16 inSeqNumber)
{
	if (inPacket->Len < 4)
		return;

	//The RTP seq number is the second short of the packet
	UInt16* seqNumPtr = (UInt16*)inPacket->Ptr;
	seqNumPtr[1] = htons(inSeqNumber);
}

void MuxSessionOutput::TearDown()
{
	QTSS_CliSesTeardownReason reason = qtssCliSesTearDownBroadcastEnded;
	(void)QTSS_SetValue(fClientSession, qtssCliTeardownReason, 0, &reason, sizeof(reason));
	(void)QTSS_Teardown(fClientSession);
}


