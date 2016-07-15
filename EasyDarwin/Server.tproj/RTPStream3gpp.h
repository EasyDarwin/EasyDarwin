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
	 File:       RTPStream3gpp.h

	 Contains:  Handles the rate-adaptation algorithm
 */

#ifndef __RTPSTREAM3GPP_H__
#define __RTPSTREAM3GPP_H__

#include "QTSSDictionary.h"
#include "RTCPAPPNADUPacket.h"
#include "RTSPRequest3GPP.h"
#include "SVector.h"

class RTPStream;

class RTPStream3GPP : public QTSSDictionary
{
public:

	// Initializes dictionary resources
	static void Initialize();

	//
	// CONSTRUCTOR / DESTRUCTOR

	RTPStream3GPP(RTPStream &owner, Bool16 enabled);
	virtual ~RTPStream3GPP() {}

	//
	//ACCESS FUNCTIONS
	void		AddNadu(UInt8* inPacketBuffer, UInt32 inPacketLength, UInt32 highestSeqNum);
	void		AddSeqNumTimeMapping(UInt16 theSeqNum, SInt64 timeStamp);		//Add a sequence number, timestamp pair; needed for figuring out the buffering delay
	Bool16      Enabled() { return fEnabled; }
	Bool16      HasRateAdaptation() { return fHasRateAdaptData; }
	Bool16		RateAdaptationEnabled() { return Enabled() && HasRateAdaptation(); }

	void       SetBufferBytes(UInt32 inBytes) { fBufferSize = inBytes; }
	void       SetBufferTime(UInt32 inMilliSecs) { fTargetBufferingDelay = inMilliSecs; }
	void       SetRateAdaptationData(RateAdapationStreamDataFields* rateDataPtr);
	void       SetBitRateData(UInt32 movieBitRate) { fMovieBitRate = movieBitRate; }

	//Will modify the containing RTPStream's QualityLevel
	void       UpdateTimeAndQuality(SInt64 curTime);							//Call this function after every RTCP

	SInt64     GetAdjustedTransmitTime(SInt64 packetTransmitTime, SInt64 theTime);

	//This needs to be called so that the next call to GetAdjustedTransmitTime will return the right value.
	void		Pause() { fLastLocalTime = 0; fSeqNumTimeMapping.clear(); }

	//Called when there is a packet loss to slow down quality adjustment increase upon the next RTCP
	void		SetPacketLoss(Float32 percentage) { if (percentage > .10) fNumRTCPWithoutPacketLoss = 0; }
	void		SetRTT(UInt32 minRTT, UInt32 curRTT);

	Bool16       GetDebugPrintfs() { return fDebugPrintfs; }
	//public members
	NaduList    fNaduList;

private:
	static UInt32 ExtendSeqNum(UInt16 seqNum, UInt32 refSeqNum);

	RTPStream&	fRTPStream;				//The RTPStream object that contains this RTPStream3GPP object
	Bool16      fEnabled;
	Bool16      fHasRateAdaptData;
	UInt32      fBufferSize;			//Buffer size as declared in 3GPP-Adaptation
	UInt32      fTargetBufferingDelay;  //Target buffering delay as declared in 3GPP-Adaptation
	UInt32      fMovieBitRate; //in bits per second;
	Bool16		fStartDoingAdaptation;

	SInt32		fNumRTCPWithoutPacketLoss;  //Number of consecutive RTCP's without any reported packet loss
	SInt32		fNumLargeRTT;				//Number of consecutive RTCP's with a large RTT
	SInt32		fNumSmallRTT;				//Number of consecutive RTCP's with a small RTT
	SInt32		fCurRTT;
	//units are in bytes and milliseconds
	UInt32      fLastReportedFreeBufferSpace;
	UInt32      fLastReportedBufferingDelay;			//set to kUInt32_Max if this value is not available.

	//Used to do play rate increase/decrease
	SInt64		fLastLocalTime; //set to 0 whenver there is a pause
	SInt64		fPlayTimeOffset;

	enum { kNaduListSize = 3 };


	enum AdjustMent {
		kAdjustDownDown = -2,
		kAdjustDown = -1,
		kNoChange = 0,
		kAdjustUp = 1,
		kAdjustUpUp = 2
	};
	AdjustMent		fAdjustSize;
	AdjustMent		fAdjustTime;

	//Dictionary support
	static QTSSAttrInfoDict::AttrInfo   sAttributes[];

	//Used to map sequence number to timestamp; needed to figure out the client's buffering delay
	struct SeqNumTimePair
	{
		SeqNumTimePair(UInt32 seqNum = 0, SInt64 time = 0) : fSeqNum(seqNum), fTime(time) {}
		UInt32 fSeqNum;
		SInt64 fTime;
	};
	SVector<SeqNumTimePair> fSeqNumTimeMapping;
	UInt32		fLastSeqNum;
	Bool16      fDebugPrintfs;
};
#endif // __RTPSTREAM3GPP_H__
