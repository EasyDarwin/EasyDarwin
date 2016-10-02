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
	 File:       RTPBandwidthTracker.cpp

	 Contains:   Implementation of class decribed in .h file

 */

#include "RTPBandwidthTracker.h"
#include "MyAssert.h"
#include "OS.h"

void RTPBandwidthTracker::SetWindowSize(SInt32 clientWindowSize)
{
	//
	// Currently we only allow this info to be set once
	if (fClientWindow > 0)
		return;

	// call SetWindowSize once the clients buffer size is known
	// since this occurs before the stream starts to send

	fClientWindow = clientWindowSize;
	fLastCongestionAdjust = 0;

#if RTP_PACKET_RESENDER_DEBUGGING   
	//€ test to see what happens w/o slow start at beginning
	//if ( initSlowStart )
	//  qtss_printf( "ack list initializing with slow start.\n" );
	//else
	//  qtss_printf( "ack list initializing at full speed.\n" );
#endif

	if (fUseSlowStart)
	{
		fSlowStartThreshold = clientWindowSize * 3 / 4;

		//
		// This is a change to the standard TCP slow start algorithm. What
		// we found was that on high bitrate high latency networks (a DSL connection, perhaps),
		// it took just too long for the ACKs to come in and for the window size to
		// grow enough. So we cheat a bit.
		fCongestionWindow = clientWindowSize / 2;
		//fCongestionWindow = kMaximumSegmentSize;
	}
	else
	{
		fSlowStartThreshold = clientWindowSize;
		fCongestionWindow = clientWindowSize;
	}

	if (fSlowStartThreshold < kMaximumSegmentSize)
		fSlowStartThreshold = kMaximumSegmentSize;
}

void RTPBandwidthTracker::EmptyWindow(UInt32 bytesIncreased, bool updateBytesInList)
{
	if (bytesIncreased == 0)
		return;

	Assert(fClientWindow > 0 && fCongestionWindow > 0);

	if (fBytesInList < bytesIncreased)
		bytesIncreased = fBytesInList;

	if (updateBytesInList)
		fBytesInList -= bytesIncreased;

	// this assert hits
	Assert(fBytesInList < ((UInt32)fClientWindow + 2000)); //mainly just to catch fBytesInList wrapping below 0

	// update the congestion window by the number of bytes just acknowledged.

	if (fCongestionWindow >= fSlowStartThreshold)
	{
		// when we hit the slow start threshold, only increase the 
		// window for each window full of acks.
		fCongestionWindow += bytesIncreased * bytesIncreased / fCongestionWindow;
	}
	else
		//
		// This is a change to the standard TCP slow start algorithm. What
		// we found was that on high bitrate high latency networks (a DSL connection, perhaps),
		// it took just too long for the ACKs to come in and for the window size to
		// grow enough. So we cheat a bit.
		fCongestionWindow += bytesIncreased;


	if (fCongestionWindow > fClientWindow)
		fCongestionWindow = fClientWindow;

	//  qtss_printf("Window = %d, %d left\n", fCongestionWindow, fCongestionWindow-fBytesInList);
}

void RTPBandwidthTracker::AdjustWindowForRetransmit()
{
	// this assert hits
	Assert(fBytesInList < ((UInt32)fClientWindow + 2000)); //mainly just to catch fBytesInList wrapping below 0

	// slow start says that we should reduce the new ss threshold to 1/2
	// of where started getting errors ( the current congestion window size )

	// so, we get a burst of re-tx becuase our RTO was mis-estimated
	// it doesn't seem like we should lower the threshold for each one.
	// it seems like we should just lower it when we first enter
	// the re-transmit "state" 
//  if ( !fIsRetransmitting )
//      fSlowStartThreshold = fCongestionWindow/2;

	// make sure that it is at least 1 packet
	if (fSlowStartThreshold < kMaximumSegmentSize)
		fSlowStartThreshold = kMaximumSegmentSize;

	// start the full window segemnt counter over again.
	fSlowStartByteCount = 0;

	// tcp resets to one (max segment size) mss, but i'm experimenting a bit
	// with not being so brutal.

	//curAckList->fCongestionWindow = kMaximumSegmentSize;

//  fCongestionWindow = kMaximumSegmentSize;
//  fCongestionWindow = fCongestionWindow / 2;  // half the congestion window size
	SInt64 theTime = OS::Milliseconds();
	if (theTime - fLastCongestionAdjust > 250)
	{
		fSlowStartThreshold = fCongestionWindow * 3 / 4;
		fCongestionWindow = fCongestionWindow / 2;
		fLastCongestionAdjust = theTime;
	}

	/*
		if ( fSlowStartThreshold < fCongestionWindow )
			fCongestionWindow = fSlowStartThreshold/2;
		else
			fCongestionWindow = fCongestionWindow /2;
	*/

	if (fCongestionWindow < kMaximumSegmentSize)
		fCongestionWindow = kMaximumSegmentSize;

	// qtss_printf("Congestion window now %d\n", fCongestionWindow);
	fIsRetransmitting = true;
}

void RTPBandwidthTracker::AddToRTTEstimate(SInt32 rttSampleMSecs)
{
	//  qtss_printf("%d ", rttSampleMSecs);
	//  static int count = 0;
	//  if ((count++ % 10) == 0) qtss_printf("\n");

		// this assert hits
	Assert(fBytesInList < ((UInt32)fClientWindow + 2000)); //mainly just to catch fBytesInList wrapping below 0

	if (fRunningAverageMSecs == 0)
		fRunningAverageMSecs = rttSampleMSecs * 8;  // init avg to cur sample, scaled by 2**3 

	SInt32 delta = rttSampleMSecs - fRunningAverageMSecs / 8; // scale average back to get cur delta from sample

	// add 1/8 the delta back to the smooth running average
	fRunningAverageMSecs = fRunningAverageMSecs + delta; // same as: rt avg = rt avg + delta / 8, but scaled

	if (delta < 0)
		delta = -1 * delta;   // absolute value 

	/*

		fRunningMeanDevationMSecs is kept scaled by 4


		so this is the same as

		fRunningMeanDevationMSecs = fRunningMeanDevationMSecs + ( |delta| - fRunningMeanDevationMSecs ) /4;
	*/

	fRunningMeanDevationMSecs += delta - fRunningMeanDevationMSecs / 4;


	fUnadjustedRTO = fCurRetransmitTimeout = fRunningAverageMSecs / 8 + fRunningMeanDevationMSecs;

	// rto should not be too low..
	if (fCurRetransmitTimeout < kMinRetransmitIntervalMSecs)
		fCurRetransmitTimeout = kMinRetransmitIntervalMSecs;

	// or too high...
	if (fCurRetransmitTimeout > kMaxRetransmitIntervalMSecs)
		fCurRetransmitTimeout = kMaxRetransmitIntervalMSecs;
	//  qtss_printf("CurTimeout == %d\n", fCurRetransmitTimeout);
}

void RTPBandwidthTracker::UpdateStats()
{
	fNumStatsSamples++;

	if (fMaxCongestionWindowSize < fCongestionWindow)
		fMaxCongestionWindowSize = fCongestionWindow;
	if (fMinCongestionWindowSize > fCongestionWindow)
		fMinCongestionWindowSize = fCongestionWindow;

	if (fMaxRTO < fUnadjustedRTO)
		fMaxRTO = fUnadjustedRTO;
	if (fMinRTO > fUnadjustedRTO)
		fMinRTO = fUnadjustedRTO;

	fTotalCongestionWindowSize += fCongestionWindow;
	fTotalRTO += fUnadjustedRTO;
}

void RTPBandwidthTracker::UpdateAckTimeout(UInt32 bitsSentInInterval, SInt64 intervalLengthInMsec)
{
	//
	// First figure out how long it will take us to fill up our window, based on
	// the movie's current bit rate
	UInt32 unadjustedTimeout = 0;
	if (bitsSentInInterval > 0)
		unadjustedTimeout = (UInt32)((intervalLengthInMsec * fCongestionWindow) / bitsSentInInterval);

	//
	// If we wait that long, that's too long because we need to actually wait for the ack to arrive.
	// So, subtract 1/2 the rto - the last ack timeout
	UInt32 rto = (UInt32)fUnadjustedRTO;
	if (rto < fAckTimeout)
		rto = fAckTimeout;
	UInt32 adjustment = (rto - fAckTimeout) / 2;
	//qtss_printf("UnadjustedTimeout = %"   _U32BITARG_   ". rto: %" _S32BITARG_ ". Last ack timeout: %"   _U32BITARG_   ". Adjustment = %"   _U32BITARG_   ".", unadjustedTimeout, fUnadjustedRTO, fAckTimeout, adjustment);
	if (adjustment > unadjustedTimeout)
		adjustment = unadjustedTimeout;
	fAckTimeout = unadjustedTimeout - adjustment;

	//qtss_printf("AckTimeout: %"   _U32BITARG_   "\n",fAckTimeout);
	if (fAckTimeout > kMaxAckTimeout)
		fAckTimeout = kMaxAckTimeout;
	else if (fAckTimeout < kMinAckTimeout)
		fAckTimeout = kMinAckTimeout;
}
