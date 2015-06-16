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
    File:       RTPOverbufferWindow.cpp

    Contains:   Implementation of the class
    
    Written By: Denis Serenyi

*/

#include "RTPOverbufferWindow.h"
#include "OSMemory.h"
#include "MyAssert.h"

RTPOverbufferWindow::RTPOverbufferWindow(UInt32 inSendInterval, UInt32 inInitialWindowSize, UInt32 inMaxSendAheadTimeInSec,
									Float32 inOverbufferRate)
:   fWindowSize(inInitialWindowSize),
    fBytesSentSinceLastReport(0),
    fSendInterval(inSendInterval),
    fBytesDuringLastSecond(0),
    fLastSecondStart(-1),
    fBytesDuringPreviousSecond(0),
    fPreviousSecondStart(-1),
    fBytesDuringBucket(0),
    fBucketBegin(0),
    fBucketTimeAhead(0),
    fPreviousBucketTimeAhead(0),
    fMaxSendAheadTime(inMaxSendAheadTimeInSec * 1000),
    fWriteBurstBeginning(false),
	fOverbufferingEnabled(true),
	fOverbufferRate(inOverbufferRate),
	fSendAheadDurationInMsec(1000),
	fOverbufferWindowBegin(-1)
{
    if (fSendInterval == 0)
    {
        fOverbufferingEnabled = false;
        fSendInterval = 200;
    }
	
	if(fOverbufferRate < 1.0)
		fOverbufferRate = 1.0;

}

SInt64 RTPOverbufferWindow::CheckTransmitTime(const SInt64& inTransmitTime, const SInt64& inCurrentTime, SInt32 inPacketSize)
{
    // if this is the beginning of a bucket interval, roll over figures from last time.
    // accumulate statistics over the period of a second
    if (inCurrentTime - fBucketBegin > fSendInterval)
    {
        fPreviousBucketBegin = fBucketBegin;
        fBucketBegin = inCurrentTime;
		if (fPreviousBucketBegin == 0)
			fPreviousBucketBegin = fBucketBegin - fSendInterval;
        fBytesDuringBucket = 0;
        if (inCurrentTime - fLastSecondStart > 1000)
        {
            fBytesDuringPreviousSecond = fBytesDuringLastSecond;
            fBytesDuringLastSecond = 0;
            fPreviousSecondStart = fLastSecondStart;
            fLastSecondStart = inCurrentTime;
        }
        
        fPreviousBucketTimeAhead = fBucketTimeAhead;
    }
    
	if (fOverbufferWindowBegin == -1)
		fOverbufferWindowBegin = inCurrentTime;
	
	if ((inTransmitTime <= inCurrentTime + fSendInterval) || 
		(fOverbufferingEnabled && (inTransmitTime <= inCurrentTime + fSendInterval + fSendAheadDurationInMsec)))
    {
        //
        // If this happens, this packet needs to be sent regardless of overbuffering
        return -1;
    }
    
    if (!fOverbufferingEnabled || (fWindowSize == 0))
        return inTransmitTime;
        
    // if the client is running low on memory, wait a while for it to be freed up
    // there's nothing magic bout these numbers, we're just trying to be conservative
    if ((fWindowSize != -1) && (inPacketSize * 5 > fWindowSize - fBytesSentSinceLastReport))
    {
        return inCurrentTime + (fSendInterval * 5);  // client reports don't come that often
    }
    
    // if we're far enough ahead, then wait until it's time to send more packets
    if (inTransmitTime - inCurrentTime > fMaxSendAheadTime)
        return inTransmitTime - fMaxSendAheadTime + fSendInterval;
        
    // during the first second just send packets normally
//    if (fPreviousSecondStart == -1)
//        return inCurrentTime + fSendInterval;
    
    // now figure if we want to send this packet during this bucket.  We have two limitations.
    // First we scale up bitrate slowly, so we should only try and send a little more than we
    // sent recently (averaged over a second or two).  However, we always try and send at
    // least the current bitrate and never more than double.
//    SInt32 currentBitRate = fBytesDuringBucket * 1000 / (inCurrentTime - fPreviousBucketBegin);
//  SInt32 averageBitRate = (fBytesDuringPreviousSecond + fBytesDuringLastSecond) * 1000 / (inCurrentTime - fPreviousSecondStart);
//    SInt32 averageBitRate = fBytesDuringPreviousSecond * 1000 / (fLastSecondStart - fPreviousSecondStart);
	fBucketTimeAhead = inTransmitTime - inCurrentTime;
//	printf("Current br = %d, average br = %d (cta = %qd, pta = %qd)\n", currentBitRate, averageBitRate, currentTimeAhead, fPreviousBucketTimeAhead);
    
    // always try and stay as far ahead as we were before
	if (fBucketTimeAhead < fPreviousBucketTimeAhead)
        return -1;
        
    // but don't send at more that double the bitrate (for any given time we should only get further
    // ahead by that amount of time)
	//printf("cta - pta = %qd, ct - pbb = %qd\n", fBucketTimeAhead - fPreviousBucketTimeAhead, SInt64((inCurrentTime - fPreviousBucketBegin) * (fOverbufferRate - 1.0)));
	if (fBucketTimeAhead - fPreviousBucketTimeAhead > ((inCurrentTime - fPreviousBucketBegin) * (fOverbufferRate - 1.0)))
	{ 
		fBucketTimeAhead = fPreviousBucketTimeAhead + SInt64((inCurrentTime - fPreviousBucketBegin) * (fOverbufferRate - 1.0));
		return inCurrentTime + fSendInterval;		// this will get us to the next bucket
	}
        
	// don't send more than 10% over the average bitrate for the previous second
//    if (currentBitRate > averageBitRate * 11 / 10)
//        return inCurrentTime + fSendInterval;       // this will get us to the next bucket
    
    return -1;  // send this packet
}

void RTPOverbufferWindow::ResetOverBufferWindow()
{
    fBytesDuringLastSecond = 0;
    fLastSecondStart = -1;
    fBytesDuringPreviousSecond = 0;
    fPreviousSecondStart = -1;
    fBytesDuringBucket = 0;
    fBucketBegin = 0;
    fBucketTimeAhead = 0;
    fPreviousBucketTimeAhead = 0;
	fOverbufferWindowBegin = -1;
}

void RTPOverbufferWindow::AddPacketToWindow(SInt32 inPacketSize)
{
    fBytesDuringBucket += inPacketSize;
    fBytesDuringLastSecond += inPacketSize;
    fBytesSentSinceLastReport += inPacketSize;
}

void RTPOverbufferWindow::EmptyOutWindow(const SInt64& inCurrentTime)
{
    // no longer needed
}

void RTPOverbufferWindow::SetWindowSize(UInt32 inWindowSizeInBytes)
{
    fWindowSize = inWindowSizeInBytes;
    fBytesSentSinceLastReport = 0;
}
