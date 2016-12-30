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
	 File:       RTPPacketResender.cpp

	 Contains:   RTPPacketResender class to buffer and track re-transmits of RTP packets.


 */

#include <stdio.h>

#include "RTPPacketResender.h"
#include "RTPStream.h"
#include "OSMutex.h"

#if RTP_PACKET_RESENDER_DEBUGGING
#include "QTSSRollingLog.h"
#include <stdarg.h>

class MyAckListLog : public QTSSRollingLog
{
public:

	MyAckListLog(char * logFName) : QTSSRollingLog() { this->SetTaskName("MyAckListLog"); ::strcpy(fLogFName, logFName); }
	virtual ~MyAckListLog() {}

	virtual char* GetLogName()
	{
		char *logFileNameStr = new char[80];

		::strcpy(logFileNameStr, fLogFName);
		return logFileNameStr;
	}

	virtual char* GetLogDir()
	{
		char *logDirStr = new char[80];

		::strcpy(logDirStr, DEFAULTPATHS_LOG_DIR);
		return logDirStr;
	}

	virtual UInt32 GetRollIntervalInDays() { return 0; }
	virtual UInt32 GetMaxLogBytes() { return 0; }

	char    fLogFName[128];

};
#endif

static const UInt32 kPacketArrayIncreaseInterval = 32;// must be multiple of 2
static const UInt32 kInitialPacketArraySize = 64;// must be multiple of kPacketArrayIncreaseInterval (Turns out this is as big as we typically need)
//static const UInt32 kMaxPacketArraySize = 512;// must be multiple of kPacketArrayIncreaseInterval it would have to be a 3 mbit or more

static const UInt32 kMaxDataBufferSize = 1600;
OSBufferPool RTPPacketResender::sBufferPool(kMaxDataBufferSize);
std::atomic_uint RTPPacketResender::sNumWastedBytes{ 0 };

RTPPacketResender::RTPPacketResender()
	: fBandwidthTracker(NULL),
	fSocket(NULL),
	fDestAddr(0),
	fDestPort(0),
	fMaxPacketsInList(0),
	fPacketsInList(0),
	fNumResends(0),
	fNumExpired(0),
	fNumAcksForMissingPackets(0),
	fNumSent(0),
	fPacketArray(NULL),
	fPacketArraySize(kInitialPacketArraySize),
	fPacketArrayMask(0),
	fHighestSeqNum(0),
	fLastUsed(0),
	fPacketQMutex()
{
	fPacketArray = (RTPResenderEntry*)new char[sizeof(RTPResenderEntry) * fPacketArraySize];
	::memset(fPacketArray, 0, sizeof(RTPResenderEntry) * fPacketArraySize);

}

RTPPacketResender::~RTPPacketResender()
{
	for (UInt32 x = 0; x < fPacketArraySize; x++)
	{
		if (fPacketArray[x].fPacketSize > 0)
			//atomic_sub(&sNumWastedBytes, kMaxDataBufferSize - fPacketArray[x].fPacketSize);
			sNumWastedBytes.fetch_sub(kMaxDataBufferSize - fPacketArray[x].fPacketSize);
		if (fPacketArray[x].fPacketData != NULL)
		{
			if (fPacketArray[x].fIsSpecialBuffer)
				delete[](char*)fPacketArray[x].fPacketData;
			else
				sBufferPool.Put(fPacketArray[x].fPacketData);
		}
	}

	delete[] fPacketArray;


}

#if RTP_PACKET_RESENDER_DEBUGGING
void RTPPacketResender::logprintf(const char * format, ...)
{
	/*
		WARNING - the logger is not multiple task thread safe.
		its OK when we run just one thread for all of the
		sending tasks though.

		each logger for a given session will open up access
		to the same log file.  with one thread we're serialized
		on writing to the file, so it works.
	*/

	va_list argptr;
	char    buff[1024];


	va_start(argptr, format);

	vsprintf(buff, format, argptr);

	va_end(argptr);

	if (fLogger)
	{
		fLogger->WriteToLog(buff, false);
		qtss_printf(buff);
	}
}


void RTPPacketResender::SetDebugInfo(UInt32 trackID, UInt16 remoteRTCPPort, UInt32 curPacketDelay)
{
	fTrackID = trackID;
	fRemoteRTCPPort = remoteRTCPPort;
	fCurrentPacketDelay = curPacketDelay;
}

void RTPPacketResender::SetLog(StrPtrLen *logname)
{
	/*
		WARNING - see logprintf()
	*/

	char    logFName[128];

	memcpy(logFName, logname->Ptr, logname->Len);
	logFName[logname->Len] = 0;

	if (fLogger)
		delete fLogger;

	fLogger = new MyAckListLog(logFName);

	fLogger->EnableLog();
}

void RTPPacketResender::LogClose(SInt64 inTimeSpentInFlowControl)
{
	this->logprintf("Flow control duration msec: %" _64BITARG_ "d. Max outstanding packets: %d\n", inTimeSpentInFlowControl, this->GetMaxPacketsInList());

}


UInt32 RTPPacketResender::SpillGuts(UInt32 inBytesSentThisInterval)
{
	if (fInfoDisplayTimer.DurationInMilliseconds() > 1000)
	{
		//fDisplayCount++;

		// spill our guts on the state of KRR
		char *isFlowed = "open";

		if (fBandwidthTracker->IsFlowControlled())
			isFlowed = "flowed";

		SInt64  kiloBitperSecond = (((SInt64)inBytesSentThisInterval * (SInt64)1000 * (SInt64)8) / fInfoDisplayTimer.DurationInMilliseconds()) / (SInt64)1024;

		//fStreamCumDuration += fInfoDisplayTimer.DurationInMilliseconds();
		fInfoDisplayTimer.Reset();

		//this->logprintf( "\n[%li] info for track %li, cur bytes %li, cur kbit/s %li\n", /*(SInt32)fStreamCumDuration,*/ fTrackID, (SInt32)inBytesSentThisInterval, (SInt32)kiloBitperSecond);
		this->logprintf("\nx info for track %li, cur bytes %li, cur kbit/s %li\n", /*(SInt32)fStreamCumDuration,*/ fTrackID, (SInt32)inBytesSentThisInterval, (SInt32)kiloBitperSecond);
		this->logprintf("stream is %s, bytes pending ack %li, cwnd %li, ssth %li, wind %li \n", isFlowed, fBandwidthTracker->BytesInList(), fBandwidthTracker->CongestionWindow(), fBandwidthTracker->SlowStartThreshold(), fBandwidthTracker->ClientWindowSize());
		this->logprintf("stats- resends:  %li, expired:  %li, dupe acks: %li, sent: %li\n", fNumResends, fNumExpired, fNumAcksForMissingPackets, fNumSent);
		this->logprintf("delays- cur:  %li, srtt: %li , dev: %li, rto: %li, bw: %li\n\n", fCurrentPacketDelay, fBandwidthTracker->RunningAverageMSecs(), fBandwidthTracker->RunningMeanDevationMSecs(), fBandwidthTracker->CurRetransmitTimeout(), fBandwidthTracker->GetCurrentBandwidthInBps());


		inBytesSentThisInterval = 0;
	}
	return inBytesSentThisInterval;
}

#endif


void RTPPacketResender::SetDestination(UDPSocket* inOutputSocket, UInt32 inDestAddr, UInt16 inDestPort)
{
	fSocket = inOutputSocket;
	fDestAddr = inDestAddr;
	fDestPort = inDestPort;
}

RTPResenderEntry*   RTPPacketResender::GetEmptyEntry(UInt16 inSeqNum, UInt32 inPacketSize)
{

	RTPResenderEntry* theEntry = NULL;

	for (UInt32 packetIndex = 0; packetIndex < fPacketsInList; packetIndex++) // see if packet is already in the array
	{
		if (inSeqNum == fPacketArray[packetIndex].fSeqNum)
		{
			return NULL;
		}
	}

	if (fPacketsInList == fPacketArraySize) // allocate a new array
	{
		fPacketArraySize += kPacketArrayIncreaseInterval;
		RTPResenderEntry* tempArray = (RTPResenderEntry*)new char[sizeof(RTPResenderEntry) * fPacketArraySize];
		::memset(tempArray, 0, sizeof(RTPResenderEntry) * fPacketArraySize);
		::memcpy(tempArray, fPacketArray, sizeof(RTPResenderEntry) * fPacketsInList);
		delete[] fPacketArray;
		fPacketArray = tempArray;
		//qtss_printf("NewArray size=%" _S32BITARG_ " packetsInList=%" _S32BITARG_ "\n",fPacketArraySize, fPacketsInList);
	}

	if (fPacketsInList < fPacketArraySize) // have an open spot
	{
		theEntry = &fPacketArray[fPacketsInList];
		fPacketsInList++;

		if (fPacketsInList < fPacketArraySize)
			fLastUsed = fPacketsInList;
		else
			fLastUsed = fPacketArraySize;
	}
	else
	{
		// nothing open so re-use 
		if (fLastUsed < fPacketArraySize - 1)
			fLastUsed++;
		else
			fLastUsed = 0;

		//qtss_printf("array is full = %"   _U32BITARG_   " reusing index=%"   _U32BITARG_   "\n",fPacketsInList,fLastUsed); 
		theEntry = &fPacketArray[fLastUsed];
		RemovePacket(fLastUsed, false); // delete packet in place don't fill we will use the spot
	}

	//
	// Check to see if this packet is too big for the buffer. If it is, then
	// we need to specially allocate a special buffer
	if (inPacketSize > kMaxDataBufferSize)
	{
		//sBufferPool.Put(theEntry->fPacketData);
		theEntry->fIsSpecialBuffer = true;
		theEntry->fPacketData = new char[inPacketSize];
	}
	else// It is not special, it's from the buffer pool
	{
		theEntry->fIsSpecialBuffer = false;
		theEntry->fPacketData = sBufferPool.Get();
	}



	return theEntry;
}


void RTPPacketResender::ClearOutstandingPackets()
{
	//OSMutexLocker packetQLocker(&fPacketQMutex);
	//for (UInt16 packetIndex = 0; packetIndex < fPacketArraySize; packetIndex++) //Testing purposes
	for (UInt16 packetIndex = 0; packetIndex < fPacketsInList; packetIndex++)
	{
		this->RemovePacket(packetIndex, false);// don't move packets delete in place
		Assert(fPacketArray[packetIndex].fPacketSize == 0);
	}
	if (fBandwidthTracker != NULL)
		fBandwidthTracker->EmptyWindow(fBandwidthTracker->BytesInList()); //clean it out
	fPacketsInList = 0; // deleting in place doesn't decrement

	Assert(fPacketsInList == 0);
}

void RTPPacketResender::AddPacket(void * inRTPPacket, UInt32 packetSize, SInt32 ageLimit)
{
	//OSMutexLocker packetQLocker(&fPacketQMutex);
	// the caller needs to adjust the overall age limit by reducing it
	// by the current packet lateness.

	// we compute a re-transmit timeout based on the Karns RTT esmitate

	UInt16* theSeqNumP = (UInt16*)inRTPPacket;
	UInt16 theSeqNum = ntohs(theSeqNumP[1]);

	if (ageLimit > 0)
	{
		RTPResenderEntry* theEntry = this->GetEmptyEntry(theSeqNum, packetSize);

		//
		// This may happen if this sequence number has already been added.
		// That may happen if we have repeat packets in the stream.
		if (theEntry == NULL || theEntry->fPacketSize > 0)
			return;

		//
		// Reset all the information in the RTPResenderEntry
		::memcpy(theEntry->fPacketData, inRTPPacket, packetSize);
		theEntry->fPacketSize = packetSize;
		theEntry->fAddedTime = OS::Milliseconds();
		theEntry->fOrigRetransTimeout = fBandwidthTracker->CurRetransmitTimeout();
		theEntry->fExpireTime = theEntry->fAddedTime + ageLimit;
		theEntry->fNumResends = 0;
		theEntry->fSeqNum = theSeqNum;

		//
		// Track the number of wasted bytes we have
		//atomic_add(&sNumWastedBytes, kMaxDataBufferSize - packetSize);
		sNumWastedBytes.fetch_add(kMaxDataBufferSize - packetSize);

		//PLDoubleLinkedListNode<RTPResenderEntry> * listNode = new PLDoubleLinkedListNode<RTPResenderEntry>( new RTPResenderEntry(inRTPPacket, packetSize, ageLimit, fRTTEstimator.CurRetransmitTimeout() ) );
		//fAckList.AddNodeToTail(listNode);
		fBandwidthTracker->FillWindow(packetSize);
	}
	else
	{
#if RTP_PACKET_RESENDER_DEBUGGING   
		this->logprintf("packet too old to add: seq# %li, age limit %li, cur late %li, track id %li\n", (SInt32)ntohs(*((UInt16*)(((char*)inRTPPacket) + 2))), (SInt32)ageLimit, fCurrentPacketDelay, fTrackID);
#endif
		fNumExpired++;
	}
	fNumSent++;
}

void RTPPacketResender::AckPacket(UInt16 inSeqNum, SInt64& inCurTimeInMsec)
{
	//OSMutexLocker packetQLocker(&fPacketQMutex);

	SInt32 foundIndex = -1;
	for (UInt32 packetIndex = 0; packetIndex < fPacketsInList; packetIndex++)
	{
		if (inSeqNum == fPacketArray[packetIndex].fSeqNum)
		{
			foundIndex = packetIndex;
			break;
		}
	}

	RTPResenderEntry* theEntry = NULL;
	if (foundIndex != -1)
		theEntry = &fPacketArray[foundIndex];


	if (theEntry == NULL || theEntry->fPacketSize == 0)
	{   /*  we got an ack for a packet that has already expired or
			for a packet whose re-transmit crossed with it's original ack

		*/
#if RTP_PACKET_RESENDER_DEBUGGING   
		this->logprintf("acked packet not found: %li, track id %li, OS::MSecs %li\n"
			, (SInt32)inSeqNum, fTrackID, (SInt32)OS::Milliseconds()
		);
#endif
		fNumAcksForMissingPackets++;
		//qtss_printf("Ack for missing packet: %d\n", inSeqNum);

		 // hmm.. we -should not have- closed down the window in this case
		 // so reopen it a bit as we normally would.
		 // ?? who know's what it really was, just use kMaximumSegmentSize
		fBandwidthTracker->EmptyWindow(RTPBandwidthTracker::kMaximumSegmentSize, false);

		// when we don't add an estimate from re-transmitted segments we're actually *underestimating* 
		// both the variation and srtt since we're throwing away ALL estimates above the current RTO!
		// therefore it's difficult for us to rapidly adapt to increases in RTT, as well as RTT that
		// are higher than our original RTO estimate.

		// for duplicate acks, use 1.5x the cur RTO as the RTT sample
		//fRTTEstimator.AddToEstimate( fRTTEstimator.CurRetransmitTimeout() * 3 / 2 );
		/// this results in some very very big RTO's since the dupes come in batches of maybe 10 or more!

//      qtss_printf("Got ack for expired packet %d\n", inSeqNum);
	}
	else
	{

#if RTP_PACKET_RESENDER_DEBUGGING
		Assert(inSeqNum == theEntry->fSeqNum);
		this->logprintf("Ack for packet: %li, track id %li, OS::MSecs %qd\n"
			, (SInt32)inSeqNum, fTrackID, OS::Milliseconds()
		);
#endif      
		fBandwidthTracker->EmptyWindow(theEntry->fPacketSize);
		if (theEntry->fNumResends == 0)
		{
			// add RTT sample...        
			// only use rtt from packets acked after their initial send, do not use
			// estimates gatherered from re-trasnmitted packets.
			//fRTTEstimator.AddToEstimate( theEntry->fPacketRTTDuration.DurationInMilliseconds() );
			fBandwidthTracker->AddToRTTEstimate((SInt32)(inCurTimeInMsec - theEntry->fAddedTime));

			//          qtss_printf("Got ack for packet %d RTT = %qd\n", inSeqNum, inCurTimeInMsec - theEntry->fAddedTime);
		}
		else
		{
#if RTP_PACKET_RESENDER_DEBUGGING
			this->logprintf("re-tx'd packet acked.  ack num : %li, pack seq #: %li, num resends %li, track id %li, size %li, OS::MSecs %qd\n" \
				, (SInt32)inSeqNum, (SInt32)ntohs(*((UInt16*)(((char*)theEntry->fPacketData) + 2))), (SInt32)theEntry->fNumResends
				, (SInt32)fTrackID, theEntry->fPacketSize, OS::Milliseconds());
#endif
		}
		this->RemovePacket(foundIndex);
	}
}

void RTPPacketResender::RemovePacket(UInt32 packetIndex, bool reuseIndex)
{
	//OSMutexLocker packetQLocker(&fPacketQMutex);

	Assert(packetIndex < fPacketArraySize);
	if (packetIndex >= fPacketArraySize)
		return;

	if (fPacketsInList == 0)
		return;

	RTPResenderEntry* theEntry = &fPacketArray[packetIndex];
	if (theEntry->fPacketSize == 0)
		return;

	//
	// Track the number of wasted bytes we have
	//atomic_sub(&sNumWastedBytes, kMaxDataBufferSize - theEntry->fPacketSize);
	sNumWastedBytes.fetch_sub(kMaxDataBufferSize - theEntry->fPacketSize);

	Assert(theEntry->fPacketSize > 0);

	//
	// Update our list information
	Assert(fPacketsInList > 0);

	if (theEntry->fIsSpecialBuffer)
	{
		delete[](char*)theEntry->fPacketData;
	}
	else if (theEntry->fPacketData != NULL)
		sBufferPool.Put(theEntry->fPacketData);


	if (reuseIndex) // we are re-using the space so keep array contiguous
	{
		fPacketArray[packetIndex] = fPacketArray[fPacketsInList - 1];
		::memset(&fPacketArray[fPacketsInList - 1], 0, sizeof(RTPResenderEntry));
		fPacketsInList--;

	}
	else    // the array is full
	{
		fBandwidthTracker->EmptyWindow(theEntry->fPacketSize, false); // keep window available
		::memset(theEntry, 0, sizeof(RTPResenderEntry));
	}

}

void RTPPacketResender::ResendDueEntries()
{
	if (fPacketsInList <= 0)
		return;

	//OSMutexLocker packetQLocker(&fPacketQMutex);
	//
	SInt32 numResends = 0;
	RTPResenderEntry* theEntry = NULL;
	SInt64 curTime = OS::Milliseconds();
	for (SInt32 packetIndex = fPacketsInList - 1; packetIndex >= 0; packetIndex--) // walk backwards because remove packet moves array members forward
	{
		theEntry = &fPacketArray[packetIndex];

		if (theEntry->fPacketSize == 0)
			continue;

		if ((curTime - theEntry->fAddedTime) > fBandwidthTracker->CurRetransmitTimeout())
		{
			// Change:  Only expire packets after they were due to be resent. This gives the client
			// a chance to ack them and improves congestion avoidance and RTT calculation
			if (curTime > theEntry->fExpireTime)
			{
#if RTP_PACKET_RESENDER_DEBUGGING   
				unsigned char version;
				version = *((char*)theEntry->fPacketData);
				version &= 0x84;    // grab most sig 2 bits
				version = version >> 6; // shift by 6 bits
				this->logprintf("expired:  seq number %li, track id %li (port: %li), vers # %li, pack seq # %li, size: %li, OS::Msecs: %qd\n", \
					(SInt32)ntohs(*((UInt16*)(((char*)theEntry->fPacketData) + 2))), fTrackID, (SInt32)ntohs(fDestPort), \
					(SInt32)version, (SInt32)ntohs(*((UInt16*)(((char*)theEntry->fPacketData) + 2))), theEntry->fPacketSize, OS::Milliseconds());
#endif
				//
				// This packet is expired
				fNumExpired++;
				//qtss_printf("Packet expired: %d\n", ((UInt16*)thePacket)[1]);
				fBandwidthTracker->EmptyWindow(theEntry->fPacketSize);
				this->RemovePacket(packetIndex);
				//              qtss_printf("Expired packet %d\n", theEntry->fSeqNum);
				continue;
			}

			// Resend this packet
			fSocket->SendTo(fDestAddr, fDestPort, theEntry->fPacketData, theEntry->fPacketSize);
			//qtss_printf("Packet resent: %d\n", ((UInt16*)theEntry->fPacketData)[1]);

			theEntry->fNumResends++;
#if RTP_PACKET_RESENDER_DEBUGGING   
			this->logprintf("re-sent: %li RTO %li, track id %li (port %li), size: %li, OS::Ms %qd\n", (SInt32)ntohs(*((UInt16*)(((char*)theEntry->fPacketData) + 2))), curTime - theEntry->fAddedTime, \
				fTrackID, (SInt32)ntohs(fDestPort) \
				, theEntry->fPacketSize, OS::Milliseconds());
#endif      

			fNumResends++;

			numResends++;
			//qtss_printf("resend loop numResends=%" _S32BITARG_ " packet theEntry->fNumResends=%" _S32BITARG_ " stream fNumResends=\n",numResends,theEntry->fNumResends++, fNumResends);

			// ok -- lets try this.. add 1.5x of the INITIAL duration since the last send to the rto estimator
			// since we won't get an ack on this packet
			// this should keep us from exponentially increasing due o a one time increase
			// in the actuall rtt, only AddToEstimate on the first resend ( assume that it's a dupe )
			// if it's not a dupe, but rather an actual loss, the subseqnuent actuals wil bring down the average quickly

			if (theEntry->fNumResends == 1)
				fBandwidthTracker->AddToRTTEstimate((SInt32)((theEntry->fOrigRetransTimeout * 3) / 2));

			//          qtss_printf("Retransmitted packet %d\n", theEntry->fSeqNum);
			theEntry->fAddedTime = curTime;
			fBandwidthTracker->AdjustWindowForRetransmit();
			continue;
		}

	}
}
void RTPPacketResender::RemovePacket(RTPResenderEntry* inEntry) { Assert(0); }
