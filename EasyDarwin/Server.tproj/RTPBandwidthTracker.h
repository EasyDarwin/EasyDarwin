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
	 File:       RTPBandwidthTracker.h

	 Contains:   Uses Karns Algorithm to measure round trip times. This also
				 tracks the current window size based on input from the caller.

	 ref:

	 Improving Round-Trip Time Estimates in Reliable Transport Protocols, ACM SIGCOMM, ???

	 Congestion Avoidance and Control - Van Jacobson, November 1988 -- Preoceedings of SIGCOMM '88

	 Internetworking with TCP/IP Comer and Stevens, Chap 14, prentice hall 1991
 */

#ifndef __RTP_BANDWIDTH_TRACKER_H__
#define __RTP_BANDWIDTH_TRACKER_H__

#include "OSHeaders.h"

class RTPBandwidthTracker
{
public:
	RTPBandwidthTracker(bool inUseSlowStart)
		: fRunningAverageMSecs(0),
		fRunningMeanDevationMSecs(0),
		fCurRetransmitTimeout(kMinRetransmitIntervalMSecs),
		fUnadjustedRTO(kMinRetransmitIntervalMSecs),
		fCongestionWindow(kMaximumSegmentSize),
		fSlowStartThreshold(0),
		fSlowStartByteCount(0),
		fClientWindow(0),
		fBytesInList(0),
		fAckTimeout(kMinAckTimeout),
		fUseSlowStart(inUseSlowStart),
		fMaxCongestionWindowSize(0),
		fMinCongestionWindowSize(1000000),
		fMaxRTO(0),
		fMinRTO(24000),
		fTotalCongestionWindowSize(0),
		fTotalRTO(0),
		fNumStatsSamples(0)
	{}

	~RTPBandwidthTracker() {}

	//
	// Initialization - give the client's window size.
	void    SetWindowSize(SInt32 clientWindowSize);

	//
	// Each RTT sample you get, let the tracker know what it is
	// so it can keep a good running average.
	void AddToRTTEstimate(SInt32 rttSampleMSecs);

	//
	// Before sending new data, let the tracker know
	// how much data you are sending so it can adjust the window.
	void FillWindow(UInt32 inNumBytes)
	{
		fBytesInList += inNumBytes; fIsRetransmitting = false;
	}

	//
	// When data is acked, let the tracker know how much
	// data was acked so it can adjust the window
	void EmptyWindow(UInt32 inNumBytes, bool updateBytesInList = true);

	//
	// When retransmitting a packet, call this function so
	// the tracker can adjust the window sizes and back off.
	void AdjustWindowForRetransmit();

	//
	// ACCESSORS
	const bool ReadyForAckProcessing() { return (fClientWindow > 0 && fCongestionWindow > 0); } // see RTPBandwidthTracker::EmptyWindow for requirements
	const bool IsFlowControlled() { return ((SInt32)fBytesInList >= fCongestionWindow); }
	const SInt32 ClientWindowSize() { return fClientWindow; }
	const UInt32 BytesInList() { return fBytesInList; }
	const SInt32 CongestionWindow() { return fCongestionWindow; }
	const SInt32 SlowStartThreshold() { return fSlowStartThreshold; }
	const SInt32 RunningAverageMSecs() { return fRunningAverageMSecs / 8; }  // fRunningAverageMSecs is stored scaled up 8x
	const SInt32 RunningMeanDevationMSecs() { return fRunningMeanDevationMSecs / 4; } // fRunningMeanDevationMSecs is stored scaled up 4x
	const SInt32 CurRetransmitTimeout() { return fCurRetransmitTimeout; }
	const SInt32 GetCurrentBandwidthInBps()
	{
		return (fUnadjustedRTO > 0) ? (fCongestionWindow * 1000) / fUnadjustedRTO : 0;
	}
	inline const UInt32 RecommendedClientAckTimeout() { return fAckTimeout; }
	void UpdateAckTimeout(UInt32 bitsSentInInterval, SInt64 intervalLengthInMsec);
	void UpdateStats();

	//
	// Stats
	SInt32              GetMaxCongestionWindowSize() { return fMaxCongestionWindowSize; }
	SInt32              GetMinCongestionWindowSize() { return fMinCongestionWindowSize; }
	SInt32              GetAvgCongestionWindowSize() { return (SInt32)(fTotalCongestionWindowSize / (SInt64)fNumStatsSamples); }
	SInt32              GetMaxRTO() { return fMaxRTO; }
	SInt32              GetMinRTO() { return fMinRTO; }
	SInt32              GetAvgRTO() { return (SInt32)(fTotalRTO / (SInt64)fNumStatsSamples); }

	enum
	{
		kMaximumSegmentSize = 1466,  // enet - just a guess!

		//
		// Our algorithm for telling the client what the ack timeout
		// is currently not too sophisticated. This could probably be made
		// better. During slow start, we just use 20, and afterwards, just use 100
		kMinAckTimeout = 20,
		kMaxAckTimeout = 100
	};

private:

	//
	// For computing the round-trip estimate using Karn's algorithm
	SInt32  fRunningAverageMSecs;
	SInt32  fRunningMeanDevationMSecs;
	SInt32  fCurRetransmitTimeout;
	SInt32  fUnadjustedRTO;

	//
	// Tracking our window sizes
	SInt64              fLastCongestionAdjust;
	SInt32              fCongestionWindow;      // implentation of VJ congestion avoidance
	SInt32              fSlowStartThreshold;    // point at which we stop adding to the window for each ack, and add to the window for each window full of acks
	SInt32              fSlowStartByteCount;            // counts window a full of acks when past ss thresh
	SInt32              fClientWindow;          // max window size based on client UDP buffer
	UInt32              fBytesInList;               // how many unacked bytes on this stream
	UInt32              fAckTimeout;

	bool              fUseSlowStart;
	bool              fIsRetransmitting;      // are we in the re-transmit 'state' ( started resending, but have yet to send 'new' data

	//
	// Stats
	SInt32              fMaxCongestionWindowSize;
	SInt32              fMinCongestionWindowSize;
	SInt32              fMaxRTO;
	SInt32              fMinRTO;
	SInt64              fTotalCongestionWindowSize;
	SInt64              fTotalRTO;
	SInt32              fNumStatsSamples;

	enum
	{
		kMinRetransmitIntervalMSecs = 600,
		kMaxRetransmitIntervalMSecs = 24000
	};
};

#endif // __RTP_BANDWIDTH_TRACKER_H__
