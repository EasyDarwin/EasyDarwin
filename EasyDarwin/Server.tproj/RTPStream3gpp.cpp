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
    File:       RTPStream3gpp.cpp

    Contains:   Implementation of RTPStream3gpp class. 
*/

#include <stdlib.h>
#include "SafeStdLib.h"
#include "OS.h"
#include "RTPStream.h"
#include "RTPStream3gpp.h"
#include "RTPSessionInterface.h"
#include "RTSPRequest3GPP.h"
#include "RTCPAPPNADUPacket.h"

#if DEBUG
    #define RTP_STREAM_3GPP_DEBUG 1
#else
    #define RTP_STREAM_3GPP_DEBUG 0
#endif

#if RTP_STREAM_3GPP_DEBUG
    #define DEBUG_PRINTF(s) qtss_printf s
#else
    #define DEBUG_PRINTF(s) if (GetDebugPrintfs()) qtss_printf s
#endif

#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif /* MIN */
#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif	/* MAX */

QTSSAttrInfoDict::AttrInfo  RTPStream3GPP::sAttributes[] = 
{   /*fields:   fAttrName, fFuncPtr, fAttrDataType, fAttrPermission */
    /* 0 */ { "qtss3GPPStreamEnabled",                    NULL,       qtssAttrDataTypeBool16,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 1 */ { "qtss3GPPStreamRateAdaptBufferBytes",       NULL,       qtssAttrDataTypeUInt32,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 2 */ { "qtss3GPPStreamRateAdaptTimeMilli",         NULL,       qtssAttrDataTypeUInt32,  qtssAttrModeRead | qtssAttrModePreempSafe }

};


void    RTPStream3GPP::Initialize()
{
    for (int x = 0; x < qtss3GPPStreamNumParams; x++)
        QTSSDictionaryMap::GetMap(QTSSDictionaryMap::k3GPPStreamDictIndex)->
            SetAttribute(x, sAttributes[x].fAttrName, sAttributes[x].fFuncPtr,
                sAttributes[x].fAttrDataType, sAttributes[x].fAttrPermission);
}


RTPStream3GPP::RTPStream3GPP(RTPStream &owner, Bool16 enabled)
:   QTSSDictionary(QTSSDictionaryMap::GetMap(QTSSDictionaryMap::k3GPPStreamDictIndex), NULL),
		fRTPStream(owner),
        fEnabled (enabled), 
        fHasRateAdaptData(false), 
        fBufferSize(0), 
        fTargetBufferingDelay(0),
        fMovieBitRate(0),
		fStartDoingAdaptation(false),
		fNumRTCPWithoutPacketLoss(kUInt32_Max / 4),
		fNumLargeRTT(0),
		fNumSmallRTT(0),
		fCurRTT(0),
        fLastReportedFreeBufferSpace(0),
        fLastReportedBufferingDelay(0),
		fLastLocalTime(0),
		fPlayTimeOffset(0),
		fAdjustSize(kNoChange),
		fAdjustTime(kNoChange),
		fLastSeqNum(0),
		fDebugPrintfs(QTSServerInterface::GetServer()->GetPrefs()->Get3GPPDebugPrintfs())
{
    this->SetVal(qtss3GPPStreamEnabled, &fEnabled, sizeof(fEnabled));
    this->SetVal(qtss3GPPStreamRateAdaptBufferBytes, &fBufferSize, sizeof(fBufferSize));
    this->SetVal(qtss3GPPStreamRateAdaptTimeMilli, &fTargetBufferingDelay, sizeof(fTargetBufferingDelay));
    fNaduList.Initialize(kNaduListSize);
}

void RTPStream3GPP::AddNadu(UInt8* inPacketBuffer, UInt32 inPacketLength, UInt32 highestSeqNum)
{
	if (!RateAdaptationEnabled())
		return;
		
	UInt32 id;
	fNaduList.AddReport(inPacketBuffer, inPacketLength, &id);
    fLastReportedFreeBufferSpace = fNaduList.LastReportedFreeBuffSizeBytes();
	UInt32 lastReportedPlayoutDelay = fNaduList.LastReportedTimeDelayMilli();
	
	//Overwrites the high 16 bits of the highest sequence number with what we think it should be since some clients report the wrong high 16 bits.
	if (highestSeqNum != 0)
		highestSeqNum = ExtendSeqNum(static_cast<UInt16>(highestSeqNum), fLastSeqNum);
	
	UInt32 nextSeqNum = ExtendSeqNum(fNaduList.GetLastReportedNSN(), highestSeqNum);   //Extend the sequence number based on the highest sequence number
	
	if (nextSeqNum <= highestSeqNum)
	{
		SInt64 nextPacketTime = 0;
		SInt64 highestPacketTime = 0;
		
		//Go through the list of sequence numbers to time mapping and find the two sequence numbers, and kick off all the sequence numbers
		//less than next sequence number since they are not needed anymore.
		for(UInt32 i = 0; i < fSeqNumTimeMapping.size();)
		{
			if (fSeqNumTimeMapping[i].fSeqNum < nextSeqNum)
				fSeqNumTimeMapping.swap_erase(i);
			else
			{
				if (fSeqNumTimeMapping[i].fSeqNum == nextSeqNum)
					nextPacketTime = fSeqNumTimeMapping[i].fTime;
				if (fSeqNumTimeMapping[i].fSeqNum == highestSeqNum)
					highestPacketTime = fSeqNumTimeMapping[i].fTime;
				++i;
				//<jm> not sure about this but it means if highestSeqNum is the same as nextSeqNum, maybe they are both 0 as in nothing to report.
				if (nextPacketTime != 0 && highestPacketTime != 0)
				    break;
			}
		}
		
		if (nextPacketTime == 0 || highestPacketTime == 0)
			fLastReportedBufferingDelay = kUInt32_Max;
		else
			fLastReportedBufferingDelay = static_cast<UInt32>(highestPacketTime - nextPacketTime);
	}
	else if (nextSeqNum == highestSeqNum + 1) //The client buffer is empty!
	{
	    fLastReportedBufferingDelay = 0;
    }
	else //error!
		fLastReportedBufferingDelay = kUInt32_Max;
	
	//Add the playout delay to the buffering delay
	if (fLastReportedBufferingDelay == kUInt32_Max)
	{
		if (highestSeqNum != 0)
			DEBUG_PRINTF((
				"RTPStream3GPP::AddNadu sequence number error: seq=%hu, nextSeqNum=%"_U32BITARG_", highestSeqNum=%"_U32BITARG_"\n",
				fNaduList.GetLastReportedNSN(), nextSeqNum, highestSeqNum));
	}
	else if (lastReportedPlayoutDelay != RTCPNaduPacket::kReservedPlayoutDelayValue) //Is the playout delay available?
		fLastReportedBufferingDelay += lastReportedPlayoutDelay;

}

void RTPStream3GPP::SetRTT(UInt32 minRTT, UInt32 curRTT)
{
    int scaleFactor = 10;
    curRTT /= scaleFactor;
    DEBUG_PRINTF(( "RTPStream3GPP::SetRTT curRTT ==%"_U32BITARG_" scaling by %d\n", curRTT,scaleFactor));
    
	double ratio = curRTT / static_cast<double>(minRTT);
	if (ratio < 1.6)
	{
		fNumSmallRTT++;
		fNumLargeRTT = MIN(0, fNumLargeRTT);
	}
	else if (ratio >= 4.0)
	{
		fNumLargeRTT++;
		fNumSmallRTT = MIN(0, fNumSmallRTT);
	}
	else
	{
		fNumLargeRTT = MIN(0, fNumLargeRTT);
		fNumSmallRTT = MIN(0, fNumSmallRTT);
	}
	fCurRTT = curRTT;
}

//Does nothing if we are not doing 3GPP-rate-adaptation.
void RTPStream3GPP::AddSeqNumTimeMapping(UInt16 theSeqNum, SInt64 timeStamp)
{
	if (!RateAdaptationEnabled())
		return;

	//We need to extend the sequence number to 32 bits
	fLastSeqNum = (fLastSeqNum == 0) ? theSeqNum : ExtendSeqNum(theSeqNum, fLastSeqNum);
	fSeqNumTimeMapping.push_back(SeqNumTimePair(fLastSeqNum, timeStamp));
}

//Extends seqNum to full 32 bits by picking the highest 16 bits such that the sequence number is closest to the reference.
UInt32 RTPStream3GPP::ExtendSeqNum(UInt16 seqNum, UInt32 refSeqNum)
{
	//the highest 16 bits is either 1 less, 1 more, or the same as the highest sequence number.
	UInt32 highBits = refSeqNum >> 16;
	UInt32 seqNum32 = seqNum | (highBits << 16);
	UInt32 diff = MIN(seqNum32 - refSeqNum, refSeqNum - seqNum32);
	
	UInt32 nextSeqNum = seqNum | ((highBits + 1) << 16);
	if (nextSeqNum - refSeqNum < diff)
	{
		diff = nextSeqNum - refSeqNum;
		seqNum32 = nextSeqNum;
	}

	nextSeqNum = seqNum | ((highBits - 1) << 16);
	if (refSeqNum - nextSeqNum < diff)
		seqNum32 = nextSeqNum;

	return seqNum32;
}

void RTPStream3GPP::UpdateTimeAndQuality(SInt64 curTime)
{
	fAdjustTime = kNoChange;
	fAdjustSize = kNoChange;

    if (!RateAdaptationEnabled())
        return;
        
        
    Bool16 limitTargetDelay = true;
    UInt32 maxTargetDelayMilli = 5000;
    
     UInt32 adjustedFreeBuffer = fLastReportedFreeBufferSpace;
     UInt32 adjustTargetDelay = fTargetBufferingDelay;
     
    if (limitTargetDelay)
    {
        if (adjustTargetDelay > maxTargetDelayMilli) //make this constant a pref
            adjustTargetDelay = maxTargetDelayMilli;
    }
    
    if (fBufferSize > RTCPNaduPacket::kMaximumReportableFreeBufferSpace)
        fBufferSize = RTCPNaduPacket::kMaximumReportableFreeBufferSpace;

    UInt32 maxUsableBufferSizeBytes = fBufferSize;
    UInt32 extraBufferMultipleInSeconds = 2; // use up to 3 times the requested target delay in bytes
    UInt32 maxUsableDelaySecs = extraBufferMultipleInSeconds * (adjustTargetDelay/1000);
    
    UInt32 movieByteRate = fMovieBitRate >> 3; // bits to bytes
    UInt32 bufferUsage = fBufferSize - fLastReportedFreeBufferSpace; 
    UInt32 bufferDelay = fLastReportedBufferingDelay;
    
    if (bufferUsage > fBufferSize)  //there is more reported free buffer than the maximum -- not good for our ratios but a good situation at the client i.e. no buffer overrun here.
    {
        bufferUsage = fBufferSize / 2; // Have to pick something so use 50%.
    }

    DEBUG_PRINTF(("reported buffer size = %"_U32BITARG_" reported Free Buffer=%"_U32BITARG_"  current calculated bufferUsage= %"_U32BITARG_"\n", fBufferSize, fLastReportedFreeBufferSpace,bufferUsage));
	DEBUG_PRINTF(("Avg Movie BitRate = %lu original target delay=%lu adjusted TargetDelay =%ld \n",fMovieBitRate,fTargetBufferingDelay, adjustTargetDelay));
	
	if (qtssVideoPayloadType == fRTPStream.GetPayLoadType() && fMovieBitRate > 0 && adjustTargetDelay > 0) //limit how much we use
	{   
	    
	    maxUsableBufferSizeBytes = maxUsableDelaySecs * movieByteRate; //buffer time * bit rate for movie is bigger than any single stream buffer

        if (maxUsableBufferSizeBytes > fBufferSize) // reported size is smaller than our buffer target
        {     maxUsableBufferSizeBytes = fBufferSize;  
              if (maxUsableBufferSizeBytes < movieByteRate) //hope for the best
                maxUsableBufferSizeBytes = movieByteRate;
              UInt32 newTargetDelay = (maxUsableBufferSizeBytes / movieByteRate) * 1000;
              if (newTargetDelay < adjustTargetDelay)
                    adjustTargetDelay = newTargetDelay;
        }

        if (adjustedFreeBuffer > maxUsableBufferSizeBytes)
            adjustedFreeBuffer = maxUsableBufferSizeBytes ;
        
        UInt32 freeBytes = fBufferSize - bufferUsage;
        if (freeBytes > fBufferSize) 
            bufferUsage = maxUsableBufferSizeBytes / 2;
         
	    DEBUG_PRINTF(("ADJUSTING buffer usage and target delay: maxUsableBufferSizeBytes =%lu adjustedFreeBuffer=%lu bufferUsage=%lu adjusted TargetDelay=%lu\n",maxUsableBufferSizeBytes, adjustedFreeBuffer, bufferUsage, adjustTargetDelay));
	}
        

    DEBUG_PRINTF(("Calculated maxUsableBufferSize =%"_U32BITARG_" reported fBufferSize=%"_U32BITARG_"  reported buffer delay=%"_U32BITARG_" current calculated bufferUsage= %"_U32BITARG_"\n", maxUsableBufferSizeBytes, fBufferSize,bufferDelay, bufferUsage));


	
    //bufferDelay should really be the network delay because if buffer delay were really large that would be ok
    // it is supposed to be -1 if not supported or a real value. Some clients send 0 incorrectly.
    //if buffer delay is small that should mean the buffer is empty and a under-run failure occurred.
    if (bufferDelay == 0)
        bufferDelay = kUInt32_Max;

    
	double bufferUsageRatio = static_cast<double>(bufferUsage) / maxUsableBufferSizeBytes;
	double bufferDelayRatio = static_cast<double>(bufferDelay) / adjustTargetDelay;
    DEBUG_PRINTF(("bufferUsageRatio =%f bufferDelayRatio=%f\n", bufferUsageRatio, bufferDelayRatio));
	
	if(!fStartDoingAdaptation)
	{
		//This is used to prevent QTSS from thinning in the beginning of the stream, when the buffering delay and usage are expected to be low
		//Rate adaptation will start when EITHER of the two low watermarks for thinning have passed, OR the media has been playing for the target buffering delay.
        //The ideal situation for the current code is 2x or more buffer size to target time. So target time converted to bytes should be 50% or less the buffer size to avoid overrun
       
		//this one is agressive and works well with Nokia when all is good and there is extra bandwidth so it makes a good network look good
        if (bufferUsageRatio >= 0.7) //start active rate adapt when client is 70% full
            fStartDoingAdaptation = true;
        else if (curTime - fRTPStream.GetSession().GetFirstPlayTime() >= 15000) // but don't wait longer than 15 seconds
            fStartDoingAdaptation = true;
        else //neither criteria was met. //speed up while waiting for the buffer to fill.
        {   fAdjustTime = kAdjustUpUp;
        }

      if (fStartDoingAdaptation)
        {		fNumLargeRTT = 0;
                fNumSmallRTT = -3; //Delay the first rate increase
        }
	}

    if (fStartDoingAdaptation)
    {
          SInt32 currentQualityLevel = fRTPStream.GetQualityLevel();
 
 // new code works good for Nokia N93 on wifi and ok for slow links (needs some more comparison testing against non rate adapt code and against build 520 or earlier) 

        if (bufferDelay != kUInt32_Max) //not supported
        {
            DEBUG_PRINTF(("rate adapt is using delay ratio and buffer size\n"));
            //The buffering delay information is available.
            
            //should I speed up or slow down?  A Delay Ratio of 100% is a target not a minimum and not a maximum.
            if (bufferDelayRatio < 2.0) //allow up to 200%
                fAdjustTime = kAdjustUp;
            else 
                fAdjustTime = kAdjustDown;
                
            if (bufferUsageRatio >= 0.7) //if you are in danger of buffer-overflowing because the buffer size is too small for the movie, also slow
                fAdjustTime = kAdjustDownDown;
            else if (bufferUsageRatio < 0.5 && bufferDelayRatio > 2.5) // stop pushing.
                fAdjustTime = kAdjustDownDown;
            else if (bufferUsageRatio < 0.5 && bufferDelayRatio > 2.0) // stop pushing.
                fAdjustTime = kAdjustDown;
            else if (bufferUsageRatio < 0.5 && bufferDelayRatio > 0.5) // try to push up hard.
                fAdjustTime = kAdjustUpUp;
            //should I thin or thicken?
            
            if (bufferUsageRatio  < 0.2 && bufferDelayRatio > 2.5) // avoid underflow since the bandwidth is low.
            {	fAdjustSize = kAdjustDown;
                DEBUG_PRINTF(("fAdjustSize=kAdjustDown 1\n"));
            }
            else if (bufferUsageRatio <= 0.1 ) //try thickening
            {	fAdjustSize = kAdjustUp;
                DEBUG_PRINTF(("fAdjustSize=kAdjustUp 1\n"));
            }
            else if (bufferUsageRatio <= 0.3 && bufferDelayRatio < 1.0) //still in danger of underflow
            {	fAdjustSize = kAdjustDown;
                DEBUG_PRINTF(("fAdjustSize=kAdjustDown 2\n"));
            }
            else if (bufferUsageRatio < 0.7 ) //no longer in danger of underflow; ok to thick
            {	fAdjustSize = kAdjustUp;
                DEBUG_PRINTF(("fAdjustSize=kAdjustUp 2\n"));
            }
            else 
                fAdjustSize = kNoChange;
        }
        else
        {
            DEBUG_PRINTF(("rate adapt is using only buffer size\n"));
    
            //The buffering delay is not available; we make thin/slow decisions based on just the buffer usage alone
            if (bufferUsageRatio  > 0.9) //need to slow and thin to avoid overflow
            {
                fAdjustSize = kAdjustDown;
                fAdjustTime = kAdjustDown;
            }
            if (bufferUsageRatio > 0.8) //need to slow and thin to avoid overflow
            {
                fAdjustSize = kNoChange;
                fAdjustTime = kAdjustDown;
            }
            else if (bufferUsageRatio > 0.7) //need to slow and thin to avoid overflow
            {
                fAdjustSize = kAdjustUp;
                fAdjustTime = kAdjustDown;
            }
            else if (bufferUsageRatio >= 0.5) //OK to start thickening
            {
                fAdjustSize = kAdjustUp;
                fAdjustTime = kAdjustDown;
            }
            else if (bufferUsageRatio > 0.4) //OK to start thickening 
            {
                fAdjustSize = kAdjustUp;
                fAdjustTime = kAdjustUp;
            }
            else if (bufferUsageRatio > 0.3) //need to speed up to avoid underflow; not enough bandwidth
            {
                fAdjustSize = kNoChange;
                fAdjustTime = kAdjustUpUp;
            }
            else if (bufferUsageRatio > 0.2) //need to speed up and thin to avoid underflow; not enough bandwidth
            {
                fAdjustSize = kAdjustDown;
                fAdjustTime = kAdjustUpUp;
            }
            else //below 20%  //need to speed up and thin to avoid underflow; not enough bandwidth
            {
                fAdjustSize = kAdjustDown;
                fAdjustTime = kAdjustUp;
            }
        }
    }

	if(fNumRTCPWithoutPacketLoss == 0) //RTCP have reported packet loss --> thinning
	{	
	    if (fCurRTT <= 10)
	    {
             DEBUG_PRINTF(("RTPStream3GPP::UpdateTimeAndQuality fast network packet loss slowing down fNumRTCPWithoutPacketLoss=%"_S32BITARG_"\n",fNumRTCPWithoutPacketLoss));
             fAdjustTime = kAdjustDown; //slow down could be random packet loss. 
	    }
	    else
	    {
            DEBUG_PRINTF(("RTPStream3GPP::UpdateTimeAndQuality slow network packet loss decrease quality fNumRTCPWithoutPacketLoss=%"_S32BITARG_"\n",fNumRTCPWithoutPacketLoss));
            fAdjustSize = kAdjustDown; //most likely out of bandwidth so reduce quality.
            fAdjustTime = kAdjustUpUp; //don't let the buffer drain while reducing quality.
	    }
	}
	else if (fNumRTCPWithoutPacketLoss <= 2) //If I get packet loss, then do not increase the rate for 2 RTCP cycles
		fAdjustSize = MIN(kNoChange, fAdjustSize);
	fNumRTCPWithoutPacketLoss++;
	
	//Set the quality based on the thinning value
    if (fAdjustSize == kAdjustUp)  // increase bit rate gradually
        fRTPStream.SetQualityLevel(fRTPStream.GetQualityLevel() - 1);
    else if (fAdjustSize == kAdjustDown) // thin down aggressively
        fRTPStream.HalveQualityLevel();

    SInt32 levelTest = ( (fRTPStream.GetNumQualityLevels() - fRTPStream.GetQualityLevel()) /2 )+ 1;	
    DEBUG_PRINTF(("RTPStream3GPP::UpdateTimeAndQuality  update threshold=%ld\n", levelTest));

	//Adjust the maximum quality if the router is getting congested(1 consecutive large RTT ratios)
	if (fNumLargeRTT >= 4)
	{
		fNumLargeRTT = 0;	//separate consecutive maximum quality lowering by at least 1 RTCP cycles.
		fRTPStream.SetMaxQualityLevelLimit(fRTPStream.GetMaxQualityLevelLimit() + 1);
		DEBUG_PRINTF(("RTPStream3GPP::UpdateTimeAndQuality fNumLargeRTT(%"_S32BITARG_") >= 4 maximum quality level decreased: %"_S32BITARG_"\n",fNumLargeRTT, fRTPStream.GetMaxQualityLevelLimit()));
	}
	else if (fNumSmallRTT >= levelTest) //Router is not congested(4 consecutive small RTT ratios); can start thickening.
	{
		fNumSmallRTT = 0;  //separate consecutive maximum quality raising by at least x RTCP cycles.
		fRTPStream.SetMaxQualityLevelLimit(fRTPStream.GetMaxQualityLevelLimit() - 1);
		DEBUG_PRINTF(("RTPStream3GPP::UpdateTimeAndQuality fNumSmallRTT (%"_S32BITARG_") >= levelTest(%"_S32BITARG_") maximum quality level increased: %"_S32BITARG_"\n",fNumSmallRTT,  levelTest, fRTPStream.GetMaxQualityLevelLimit()));
	}

	char *payload = "?";
	if (GetDebugPrintfs()) 
	{
		UInt8 payloadType = fRTPStream.GetPayLoadType();
		if (qtssVideoPayloadType == payloadType) 
			payload="video";
		else if (qtssAudioPayloadType == payloadType)
			payload="audio";
	}
	
	if (bufferDelay != kUInt32_Max)
		DEBUG_PRINTF((
					"RTPStream3GPP::UpdateTimeAndQuality type=%s quality=%"_S32BITARG_",  qualitylimit=%"_S32BITARG_", fAdjustTime=%i bufferUsage=%"_U32BITARG_"(%.0f%%), "
					"bufferDelay=%"_U32BITARG_"(%.0f%%)\n",
					payload,
					fRTPStream.GetQualityLevel(), fRTPStream.GetMaxQualityLevelLimit(),
					fAdjustTime,
					bufferUsage, bufferUsageRatio * 100,
					bufferDelay, bufferDelayRatio * 100
		));
	else
		DEBUG_PRINTF((
					"RTPStream3GPP::UpdateTimeAndQuality type=%s quality=%"_S32BITARG_", qualitylimit=%"_S32BITARG_", fAdjustTime=%i bufferUsage=%"_U32BITARG_"(%.0f%%), "
					"bufferDelay=?\n",
					payload,
					fRTPStream.GetQualityLevel(), fRTPStream.GetMaxQualityLevelLimit(),
					fAdjustTime,
					bufferUsage, bufferUsageRatio * 100
		));
		
		
}

void   RTPStream3GPP::SetRateAdaptationData(RateAdapationStreamDataFields* rateDataPtr)
{
    if (NULL == rateDataPtr || !fEnabled)
        return;
 
    this->SetBufferBytes(rateDataPtr->GetBufferSizeBytes());
    this->SetBufferTime(rateDataPtr->GetTargetTimeMilliSec());
    fHasRateAdaptData = true;
	
	if (fTargetBufferingDelay < 1000)
		fTargetBufferingDelay = 1000;
		
	if (fBufferSize == 0)
		fBufferSize = 1;
	
    //rateDataPtr->PrintData();
}

//Returns the adjusted transmit time to take into account of speed increases/decreases
SInt64 RTPStream3GPP::GetAdjustedTransmitTime(SInt64 packetTransmitTime, SInt64 theTime)
{
	if (!RateAdaptationEnabled())
		return packetTransmitTime;
		
	if (fLastLocalTime == 0)
	{
		//can this happen? In case the file module tries to send a packet while the session is paused.
		if (fRTPStream.GetSession().GetSessionState() == qtssPausedState)
			return packetTransmitTime + fPlayTimeOffset;
			
		//First packet after starting to play or after a pause.
		DEBUG_PRINTF(("RTPStream3GPP::GetAdjustedTransmitTime: initial quality=%"_U32BITARG_"\n", fRTPStream.GetQualityLevel()));
		fLastLocalTime = theTime;
		fPlayTimeOffset = 0;
		return packetTransmitTime;
	}
	
    SInt32 elapsedTime = theTime - fLastLocalTime;
	if (elapsedTime >= 50) //Don't adjust the time over very small increments
	{
		fLastLocalTime = theTime;
        if (fAdjustTime == kAdjustUpUp) // go faster subtract time (by 50%)
			fPlayTimeOffset -= elapsedTime / 2;
		else if (fAdjustTime == kAdjustUp) // go a bit faster by 10%
			fPlayTimeOffset -= elapsedTime / 10;
        else if (fAdjustTime == kAdjustDown) // go slower add time (by 20%)
			fPlayTimeOffset += elapsedTime / 5;
        else if (fAdjustTime == kAdjustDownDown) // go slower add time (by 50%)
			fPlayTimeOffset += elapsedTime / 2;
	}
	return packetTransmitTime + fPlayTimeOffset;
}

