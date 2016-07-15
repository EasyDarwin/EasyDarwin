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
	 File:       ReflectorStream.h

	 Contains:   This object supports reflecting an RTP multicast stream to N
				 RTPStreams. It spaces out the packet send times in order to
				 maximize the randomness of the sending pattern and smooth
				 the stream.
 */

#ifndef _REFLECTOR_STREAM_H_
#define _REFLECTOR_STREAM_H_

#include "QTSS.h"

#include "IdleTask.h"
#include "SourceInfo.h"

#include "UDPSocket.h"
#include "UDPSocketPool.h"
#include "UDPDemuxer.h"
#include "SequenceNumberMap.h"

#include "OSMutex.h"
#include "OSQueue.h"
#include "OSRef.h"

#include "RTCPSRPacket.h"
#include "ReflectorOutput.h"
#include "atomic.h"

 /*fantasy add this*/
#include "keyframecache.h"

//This will add some printfs that are useful for checking the thinning
#define REFLECTOR_THINNING_DEBUGGING 0 
#define MAX_CACHE_SIZE  1024*1024*2

//Define to use new potential workaround for NAT problems
#define NAT_WORKAROUND 1

typedef struct FU_Indicator_tag
{
	unsigned char F : 1;
	unsigned char nRI : 2;
	unsigned char type : 5;//
}FAU_Indicator;

typedef struct FU_Head_tag
{
	unsigned char nalu_type : 5;//little 5 bit
	unsigned char r : 1;
	unsigned char e : 1;
	unsigned char s : 1;//high bit    
}FU_Head;


class ReflectorPacket;
class ReflectorSender;
class ReflectorStream;
class RTPSessionOutput;
class ReflectorSession;

class ReflectorPacket
{
public:

	ReflectorPacket() : fQueueElem() { fQueueElem.SetEnclosingObject(this); this->Reset(); }
	void Reset() { // make packet ready to reuse fQueueElem is always in use
		fBucketsSeenThisPacket = 0;
		fTimeArrived = 0;
		//fQueueElem -- should be set to this
		fPacketPtr.Set(fPacketData, 0);
		fIsRTCP = false;
		fStreamCountID = 0;
		fNeededByOutput = false;
	}

	~ReflectorPacket() {}

	void    SetPacketData(char *data, UInt32 len)
	{
		Assert(kMaxReflectorPacketSize > len);

		if (len > kMaxReflectorPacketSize)
			len = kMaxReflectorPacketSize;

		if (len > 0)
			memcpy(this->fPacketPtr.Ptr, data, len);
		this->fPacketPtr.Len = len;
	}

	Bool16  IsRTCP() { return fIsRTCP; }
	inline  UInt32  GetPacketRTPTime();
	inline  UInt16  GetPacketRTPSeqNum();
	inline  UInt32  GetSSRC(Bool16 isRTCP);
	inline  SInt64  GetPacketNTPTime();

private:

	enum
	{
		kMaxReflectorPacketSize = 2060    //jm 5/02 increased from 2048 by 12 bytes for test bytes appended to packets
	};

	UInt32      fBucketsSeenThisPacket;
	SInt64      fTimeArrived;
	OSQueueElem fQueueElem;
	char        fPacketData[kMaxReflectorPacketSize];
	StrPtrLen   fPacketPtr;
	Bool16      fIsRTCP;
	Bool16      fNeededByOutput; // is this packet still needed for output?
	UInt64      fStreamCountID;

	friend class ReflectorSender;
	friend class ReflectorSocket;
	friend class RTPSessionOutput;


};

UInt32 ReflectorPacket::GetSSRC(Bool16 isRTCP)
{
	if (fPacketPtr.Ptr == NULL || fPacketPtr.Len < 8)
		return 0;

	UInt32* theSsrcPtr = (UInt32*)fPacketPtr.Ptr;
	if (isRTCP)// RTCP 
		return ntohl(theSsrcPtr[1]);

	if (fPacketPtr.Len < 12)
		return 0;

	return ntohl(theSsrcPtr[2]);  // RTP SSRC
}

UInt32 ReflectorPacket::GetPacketRTPTime()
{

	UInt32 timestamp = 0;
	if (!fIsRTCP)
	{
		//The RTP timestamp number is the second long of the packet
		if (fPacketPtr.Ptr == NULL || fPacketPtr.Len < 8)
			return 0;
		timestamp = ntohl(((UInt32*)fPacketPtr.Ptr)[1]);
	}
	else
	{
		if (fPacketPtr.Ptr == NULL || fPacketPtr.Len < 20)
			return 0;
		timestamp = ntohl(((UInt32*)fPacketPtr.Ptr)[4]);
	}
	return timestamp;
}

UInt16 ReflectorPacket::GetPacketRTPSeqNum()
{
	Assert(!fIsRTCP); // not a supported type

	if (fPacketPtr.Ptr == NULL || fPacketPtr.Len < 4 || fIsRTCP)
		return 0;

	UInt16 sequence = ntohs(((UInt16*)fPacketPtr.Ptr)[1]); //The RTP sequenc number is the second short of the packet
	return sequence;
}


SInt64  ReflectorPacket::GetPacketNTPTime()
{
	Assert(fIsRTCP); // not a supported type
	if (fPacketPtr.Ptr == NULL || fPacketPtr.Len < 16 || !fIsRTCP)
		return 0;

	UInt32* theReport = (UInt32*)fPacketPtr.Ptr;
	theReport += 2;
	SInt64 ntp = 0;
	::memcpy(&ntp, theReport, sizeof(SInt64));

	return OS::Time1900Fixed64Secs_To_TimeMilli(OS::NetworkToHostSInt64(ntp));


}


//Custom UDP socket classes for doing reflector packet retrieval, socket management
class ReflectorSocket : public IdleTask, public UDPSocket
{
public:

	ReflectorSocket();
	virtual ~ReflectorSocket();
	void    AddBroadcasterSession(QTSS_ClientSessionObject inSession) { OSMutexLocker locker(this->GetDemuxer()->GetMutex()); fBroadcasterClientSession = inSession; }
	void    RemoveBroadcasterSession(QTSS_ClientSessionObject inSession) { OSMutexLocker locker(this->GetDemuxer()->GetMutex()); if (inSession == fBroadcasterClientSession) fBroadcasterClientSession = NULL; }
	void    AddSender(ReflectorSender* inSender);
	void    RemoveSender(ReflectorSender* inStreamElem);
	Bool16  HasSender() { return (this->GetDemuxer()->GetHashTable()->GetNumEntries() > 0); }
	Bool16  ProcessPacket(const SInt64& inMilliseconds, ReflectorPacket* thePacket, UInt32 theRemoteAddr, UInt16 theRemotePort);
	ReflectorPacket*    GetPacket();
	virtual SInt64      Run();
	void    SetSSRCFilter(Bool16 state, UInt32 timeoutSecs) { fFilterSSRCs = state; fTimeoutSecs = timeoutSecs; }
private:

	//virtual SInt64        Run();
	void    GetIncomingData(const SInt64& inMilliseconds);
	void    FilterInvalidSSRCs(ReflectorPacket* thePacket, Bool16 isRTCP);

	//Number of packets to allocate when the socket is first created
	enum
	{
		kNumPreallocatedPackets = 20,   //UInt32
		kRefreshBroadcastSessionIntervalMilliSecs = 10000,
		kSSRCTimeOut = 30000 // milliseconds before clearing the SSRC if no new ssrcs have come in
	};
	QTSS_ClientSessionObject    fBroadcasterClientSession;
	SInt64                      fLastBroadcasterTimeOutRefresh;
	// Queue of available ReflectorPackets
	OSQueue fFreeQueue;
	// Queue of senders
	OSQueue fSenderQueue;
	SInt64  fSleepTime;

	UInt32  fValidSSRC;
	SInt64  fLastValidSSRCTime;
	Bool16  fFilterSSRCs;
	UInt32  fTimeoutSecs;

	Bool16  fHasReceiveTime;
	UInt64  fFirstReceiveTime;
	SInt64  fFirstArrivalTime;
	UInt32  fCurrentSSRC;

};


class ReflectorSocketPool : public UDPSocketPool
{
public:

	ReflectorSocketPool() {}
	virtual ~ReflectorSocketPool() {}

	virtual UDPSocketPair*  ConstructUDPSocketPair();
	virtual void            DestructUDPSocketPair(UDPSocketPair *inPair);
	virtual void            SetUDPSocketOptions(UDPSocketPair* inPair);
	void                    DestructUDPSocket(ReflectorSocket* socket);


};

class ReflectorSender : public UDPDemuxerTask
{
public:
	ReflectorSender(ReflectorStream* inStream, UInt32 inWriteFlag);
	virtual ~ReflectorSender();
	// Queue of senders
	OSQueue fSenderQueue;
	SInt64  fSleepTime;

	//Used for adjusting sequence numbers in light of thinning
	UInt16      GetPacketSeqNumber(const StrPtrLen& inPacket);
	void        SetPacketSeqNumber(const StrPtrLen& inPacket, UInt16 inSeqNumber);
	Bool16      PacketShouldBeThinned(QTSS_RTPStreamObject inStream, const StrPtrLen& inPacket);

	//We want to make sure that ReflectPackets only gets invoked when there
	//is actually work to do, because it is an expensive function
	Bool16      ShouldReflectNow(const SInt64& inCurrentTime, SInt64* ioWakeupTime);

	//This function gets data from the multicast source and reflects.
	//Returns the time at which it next needs to be invoked
	void        ReflectPackets(SInt64* ioWakeupTime, OSQueue* inFreeQueue);

	//this is the old way of doing reflect packets. It is only here until the relay code can be cleaned up.
	void        ReflectRelayPackets(SInt64* ioWakeupTime, OSQueue* inFreeQueue);

	OSQueueElem*    SendPacketsToOutput(ReflectorOutput* theOutput, OSQueueElem* currentPacket, SInt64 currentTime, SInt64  bucketDelay, Bool16 firstPacket);

	UInt32      GetOldestPacketRTPTime(Bool16 *foundPtr);
	UInt16      GetFirstPacketRTPSeqNum(Bool16 *foundPtr);
	Bool16      GetFirstPacketInfo(UInt16* outSeqNumPtr, UInt32* outRTPTimePtr, SInt64* outArrivalTimePtr);

	OSQueueElem*GetClientBufferNextPacketTime(UInt32 inRTPTime);
	Bool16      GetFirstRTPTimePacket(UInt16* outSeqNumPtr, UInt32* outRTPTimePtr, SInt64* outArrivalTimePtr);

	void        RemoveOldPackets(OSQueue* inFreeQueue);
	OSQueueElem* GetClientBufferStartPacketOffset(SInt64 offsetMsec, Bool16 needKeyFrameFirstPacket = false);
	OSQueueElem* GetClientBufferStartPacket() { return this->GetClientBufferStartPacketOffset(0); };

	// ->geyijyn@20150427
	// 关键帧索引及丢帧方案
	OSQueueElem* NeedRelocateBookMark(OSQueueElem* currentElem);
	OSQueueElem* GetNewestKeyFrameFirstPacket(OSQueueElem* currentElem, SInt64 offsetMsec);
	Bool16 IsKeyFrameFirstPacket(ReflectorPacket* thePacket);
	Bool16 IsFrameFirstPacket(ReflectorPacket* thePacket);
	Bool16 IsFrameLastPacket(ReflectorPacket* thePacket);

	ReflectorStream*    fStream;
	UInt32              fWriteFlag;

	OSQueue         fPacketQueue;
	OSQueueElem*    fFirstNewPacketInQueue;
	OSQueueElem*    fFirstPacketInQueueForNewOutput;
	OSQueueElem*	fKeyFrameStartPacketElementPointer;//最新关键帧指针

	//these serve as an optimization, keeping track of when this
	//sender needs to run so it doesn't run unnecessarily

	inline void SetNextTimeToRun(SInt64 nextTime)
	{
		fNextTimeToRun = nextTime;
		//qtss_printf("SetNextTimeToRun =%"_64BITARG_"d\n", fNextTimeToRun);
	}

	Bool16      fHasNewPackets;
	SInt64      fNextTimeToRun;

	//how often to send RRs to the source
	enum
	{
		kRRInterval = 5000      //SInt64 (every 5 seconds)
	};

	SInt64      fLastRRTime;
	OSQueueElem fSocketQueueElem;

	friend class ReflectorSocket;
	friend class ReflectorStream;
};

class ReflectorStream
{
public:

	enum
	{
		// A ReflectorStream is uniquely identified by the
		// destination IP address & destination port of the broadcast.
		// This ID simply contains that information.
		//
		// A unicast broadcast can also be identified by source IP address. If
		// you are attempting to demux by source IP, this ID will not guarentee
		// uniqueness and special care should be used.
		kStreamIDSize = sizeof(UInt32) + sizeof(UInt16)
	};

	// Uses a StreamInfo to generate a unique ID
	static void GenerateSourceID(SourceInfo::StreamInfo* inInfo, char* ioBuffer);

	ReflectorStream(SourceInfo::StreamInfo* inInfo);
	~ReflectorStream();

	//
	// SETUP
	//
	// Call Register from the Register role, as this object has some QTSS API
	// attributes to setup
	static void Register();
	static void Initialize(QTSS_ModulePrefsObject inPrefs);

	//
	// MODIFIERS

	// Call this to initialize the reflector sockets. Uses the QTSS_RTSPRequestObject
	// if provided to report any errors that occur 
	// Passes the QTSS_ClientSessionObject to the socket so the socket can update the session if needed.
	QTSS_Error BindSockets(QTSS_StandardRTSP_Params* inParams, UInt32 inReflectorSessionFlags, Bool16 filterState, UInt32 timeout);

	// This stream reflects packets from the broadcast to specific ReflectorOutputs.
	// You attach outputs to ReflectorStreams this way. You can force the ReflectorStream
	// to put this output into a certain bucket by passing in a certain bucket index.
	// Pass in -1 if you don't care. AddOutput returns the bucket index this output was
	// placed into, or -1 on an error.

	SInt32  AddOutput(ReflectorOutput* inOutput, SInt32 putInThisBucket);

	// Removes the specified output from this ReflectorStream.
	void    RemoveOutput(ReflectorOutput* inOutput); // Removes this output from all tracks

	void	TearDownAllOutputs(); // causes a tear down and then a remove

	// If the incoming data is RTSP interleaved, packets for this stream are identified
	// by channel numbers
	void	SetRTPChannelNum(SInt16 inChannel) { fRTPChannel = inChannel; }
	void	SetRTCPChannelNum(SInt16 inChannel) { fRTCPChannel = inChannel; }
	void	PushPacket(char *packet, UInt32 packetLen, Bool16 isRTCP);

	//
	// ACCESSORS
	UInt32                  GetBitRate() { return fCurrentBitRate; }
	SourceInfo::StreamInfo* GetStreamInfo() { return &fStreamInfo; }
	OSMutex*                GetMutex() { return &fBucketMutex; }
	void*                   GetStreamCookie() { return this; }
	SInt16                  GetRTPChannel() { return fRTPChannel; }
	SInt16                  GetRTCPChannel() { return fRTCPChannel; }
	UDPSocketPair*          GetSocketPair() { return fSockets; }
	ReflectorSender*        GetRTPSender() { return &fRTPSender; }
	ReflectorSender*        GetRTCPSender() { return &fRTCPSender; }

	void                    SetHasFirstRTCP(Bool16 hasPacket) { fHasFirstRTCPPacket = hasPacket; }
	Bool16                  HasFirstRTCP() { return fHasFirstRTCPPacket; }

	void                    SetFirst_RTCP_RTP_Time(UInt32 time) { fFirst_RTCP_RTP_Time = time; }
	UInt32                  GetFirst_RTCP_RTP_Time() { return fFirst_RTCP_RTP_Time; }

	void                    SetFirst_RTCP_Arrival_Time(SInt64 time) { fFirst_RTCP_Arrival_Time = time; }
	SInt64                  GetFirst_RTCP_Arrival_Time() { return fFirst_RTCP_Arrival_Time; }


	void                    SetHasFirstRTP(Bool16 hasPacket) { fHasFirstRTPPacket = hasPacket; }
	Bool16                  HasFirstRTP() { return fHasFirstRTPPacket; }

	UInt32                  GetBufferDelay() { return ReflectorStream::sOverBufferInMsec; }
	UInt32                  GetTimeScale() { return fStreamInfo.fTimeScale; }
	UInt64                  fPacketCount;

	void                    SetEnableBuffer(Bool16 enableBuffer) { fEnableBuffer = enableBuffer; }
	Bool16                  BufferEnabled() { return fEnableBuffer; }
	inline  void                    UpdateBitRate(SInt64 currentTime);
	static UInt32           sOverBufferInMsec;

	void                    IncEyeCount() { OSMutexLocker locker(&fBucketMutex); fEyeCount++; }
	void                    DecEyeCount() { OSMutexLocker locker(&fBucketMutex); fEyeCount--; }
	UInt32                  GetEyeCount() { OSMutexLocker locker(&fBucketMutex); return fEyeCount; }

	void					SetMyReflectorSession(ReflectorSession* reflector) { fMyReflectorSession = reflector; }
	ReflectorSession*		GetMyReflectorSession() { return fMyReflectorSession; }

private:

	//Sends an RTCP receiver report to the broadcast source
	void    SendReceiverReport();
	void    AllocateBucketArray(UInt32 inNumBuckets);
	SInt32  FindBucket();

	// Reflector sockets, retrieved from the socket pool
	UDPSocketPair*      fSockets;

	QTSS_RTPTransportType fTransportType;

	ReflectorSender     fRTPSender;
	ReflectorSender     fRTCPSender;
	SequenceNumberMap   fSequenceNumberMap; //for removing duplicate packets

	// All the necessary info about this stream
	SourceInfo::StreamInfo  fStreamInfo;

	enum
	{
		kReceiverReportSize = 16,               //UInt32
		kAppSize = 36,                          //UInt32
		kMinNumBuckets = 16,                    //UInt32
		kBitRateAvgIntervalInMilSecs = 30000 // time between bitrate averages
	};

	// BUCKET ARRAY
	//ReflectorOutputs are kept in a 2-dimensional array, "Buckets"
	typedef ReflectorOutput** Bucket;
	Bucket*     fOutputArray;

	UInt32      fNumBuckets;        //Number of buckets currently
	UInt32      fNumElements;       //Number of reflector outputs in the array

	//Bucket array can't be modified while we are sending packets.
	OSMutex     fBucketMutex;

	// RTCP RR information

	char        fReceiverReportBuffer[kReceiverReportSize + kAppSize +
		RTCPSRPacket::kMaxCNameLen];
	UInt32*     fEyeLocation;//place in the buffer to write the eye information
	UInt32      fReceiverReportSize;

	// This is the destination address & port for RTCP
	// receiver reports.
	UInt32      fDestRTCPAddr;
	UInt16      fDestRTCPPort;

	// Used for calculating average bit rate
	UInt32              fCurrentBitRate;
	SInt64              fLastBitRateSample;
	unsigned int        fBytesSentInThisInterval;// unsigned int because we need to atomic_add 

	// If incoming data is RTSP interleaved
	SInt16              fRTPChannel; //These will be -1 if not set to anything
	SInt16              fRTCPChannel;

	Bool16              fHasFirstRTCPPacket;
	Bool16              fHasFirstRTPPacket;

	Bool16              fEnableBuffer;
	UInt32              fEyeCount;

	UInt32              fFirst_RTCP_RTP_Time;
	SInt64              fFirst_RTCP_Arrival_Time;

	ReflectorSession*	fMyReflectorSession;

	static UInt32       sBucketSize;
	static UInt32       sMaxPacketAgeMSec;
	static UInt32       sMaxFuturePacketSec;

	static UInt32       sMaxFuturePacketMSec;
	static UInt32       sOverBufferInSec;
	static UInt32       sBucketDelayInMsec;
	static Bool16       sUsePacketReceiveTime;
	static UInt32       sFirstPacketOffsetMsec;

	static UInt32       sRelocatePacketAgeMSec;

	friend class ReflectorSocket;
	friend class ReflectorSender;

public:
	CKeyFrameCache*		pkeyFrameCache;
};


void    ReflectorStream::UpdateBitRate(SInt64 currentTime)
{
	if ((fLastBitRateSample + ReflectorStream::kBitRateAvgIntervalInMilSecs) < currentTime)
	{
		unsigned int intervalBytes = fBytesSentInThisInterval;
		(void)atomic_sub(&fBytesSentInThisInterval, intervalBytes);

		// Multiply by 1000 to convert from milliseconds to seconds, and by 8 to convert from bytes to bits
		Float32 bps = (Float32)(intervalBytes * 8) / (Float32)(currentTime - fLastBitRateSample);
		bps *= 1000;
		fCurrentBitRate = (UInt32)bps;

		// Don't check again for awhile!
		fLastBitRateSample = currentTime;
	}
}
#endif //_REFLECTOR_SESSION_H_

