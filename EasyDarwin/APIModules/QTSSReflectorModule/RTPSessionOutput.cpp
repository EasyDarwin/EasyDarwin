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
    File:       RTSPReflectorOutput.cpp

    Contains:   Implementation of object in .h file
                    

*/


#include "RTPSessionOutput.h"
#include "ReflectorStream.h"

#include <errno.h>


#if DEBUG 
#define RTP_SESSION_DEBUGGING 0
#else
#define RTP_SESSION_DEBUGGING 0
#endif

// ATTRIBUTES
static QTSS_AttributeID     sStreamPacketCountAttr          = qtssIllegalAttrID;


static QTSS_AttributeID     sNextSeqNumAttr             = qtssIllegalAttrID;
static QTSS_AttributeID     sSeqNumOffsetAttr           = qtssIllegalAttrID;
static QTSS_AttributeID     sLastQualityChangeAttr      = qtssIllegalAttrID;
static QTSS_AttributeID     sLastRTPPacketIDAttr        = qtssIllegalAttrID;
static QTSS_AttributeID     sLastRTCPPacketIDAttr       = qtssIllegalAttrID;

static QTSS_AttributeID     sFirstRTCPCurrentTimeAttr       = qtssIllegalAttrID;
static QTSS_AttributeID     sFirstRTCPArrivalTimeAttr       = qtssIllegalAttrID;
static QTSS_AttributeID     sFirstRTCPTimeStampAttr         = qtssIllegalAttrID;

static QTSS_AttributeID     sFirstRTPCurrentTimeAttr        = qtssIllegalAttrID;
static QTSS_AttributeID     sFirstRTPArrivalTimeAttr        = qtssIllegalAttrID;
static QTSS_AttributeID     sFirstRTPTimeStampAttr          = qtssIllegalAttrID;

static QTSS_AttributeID     sBaseRTPTimeStampAttr           = qtssIllegalAttrID;

static QTSS_AttributeID     sBaseArrivalTimeStampAttr       = qtssIllegalAttrID;
static QTSS_AttributeID     sStreamSSRCAttr                 = qtssIllegalAttrID;

static QTSS_AttributeID     sStreamByteCountAttr            = qtssIllegalAttrID;

static QTSS_AttributeID     sLastRTPTimeStampAttr           = qtssIllegalAttrID;

static QTSS_AttributeID     sLastRTCPTransmitAttr           = qtssIllegalAttrID;

RTPSessionOutput::RTPSessionOutput(QTSS_ClientSessionObject inClientSession, ReflectorSession* inReflectorSession,
                                    QTSS_Object serverPrefs, QTSS_AttributeID inCookieAddrID)
:   fClientSession(inClientSession),
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
    this->InititializeBookmarks( inReflectorSession->GetNumStreams() );
   
}

void RTPSessionOutput::Register()
{
    // Add some attributes to QTSS_RTPStream dictionary 
    static char*        sNextSeqNum             = "qtssNextSeqNum";
    static char*        sSeqNumOffset           = "qtssSeqNumOffset";
    static char*        sLastQualityChange      = "qtssLastQualityChange";
    
    static char*        sLastRTPPacketID        = "qtssReflectorStreamLastRTPPacketID";
    static char*        sLastRTCPPacketID       = "qtssReflectorStreamLastRTCPPacketID";


    static char*        sFirstRTCPArrivalTime   = "qtssReflectorStreamStartRTCPArrivalTime";
    static char*        sFirstRTCPTimeStamp     = "qtssReflectorStreamStartRTCPTimeStamp";
    static char*        sFirstRTCPCurrentTime   = "qtssReflectorStreamStartRTCPCurrent";

    static char*        sFirstRTPArrivalTime    = "qtssReflectorStreamStartRTPArrivalTime";
    static char*        sFirstRTPTimeStamp      = "qtssReflectorStreamStartRTPTimeStamp";
    static char*        sFirstRTPCurrentTime    = "qtssReflectorStreamStartRTPCurrent";

    static char*        sBaseRTPTimeStamp       = "qtssReflectorStreamBaseRTPTimeStamp";
    static char*        sBaseArrivalTimeStamp   = "qtssReflectorStreamBaseArrivalTime";

    static char*        sLastRTPTimeStamp       = "qtssReflectorStreamLastRTPTimeStamp";
    static char*        sLastRTCPTransmit       = "qtssReflectorStreamLastRTCPTransmit";

    static char*        sStreamSSRC             = "qtssReflectorStreamSSRC";
    static char*        sStreamPacketCount      = "qtssReflectorStreamPacketCount";
    static char*        sStreamByteCount        = "qtssReflectorStreamByteCount";


 
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

Bool16 RTPSessionOutput::IsPlaying()
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

void RTPSessionOutput::InitializeStreams()
{ 
        
    UInt32                  theLen = 0;
    QTSS_RTPStreamObject*   theStreamPtr = NULL;
    UInt32                  packetCountInitValue = 0;
    
    for (SInt16 z = 0; QTSS_GetValuePtr(fClientSession, qtssCliSesStreamObjects, z, (void**)&theStreamPtr, &theLen) == QTSS_NoErr; z++)
    {
        (void) QTSS_SetValue(*theStreamPtr, sStreamPacketCountAttr, 0, &packetCountInitValue, sizeof(UInt32));
    }

}



Bool16 RTPSessionOutput::IsUDP()
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
        (void) QTSS_GetValuePtr(*theStreamPtr, qtssRTPStrTransportType, 0, (void**) &theTransportTypePtr, &theLen);
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
   
  //if (fIsUDP) printf("RTPSessionOutput::RTPSessionOutput Standard UDP client\n");
   //else printf("RTPSessionOutput::RTPSessionOutput Buffered Client\n");
   
   fTransportInitialized = true;
   return fIsUDP;
}


Bool16  RTPSessionOutput::FilterPacket(QTSS_RTPStreamObject *theStreamPtr, StrPtrLen* inPacket)
{ 
    
    UInt32* packetCountPtr = NULL;
    UInt32 theLen = 0;
    
    //see if we started sending and if so then just keep sending (reset on a play)
    QTSS_Error writeErr = QTSS_GetValuePtr(*theStreamPtr, sStreamPacketCountAttr, 0,(void**) &packetCountPtr,&theLen);        
    if (writeErr == QTSS_NoErr && theLen > 0 && *packetCountPtr > 0)
        return false;

    Assert(theStreamPtr);
    Assert(inPacket);
    
    UInt16 seqnum = this->GetPacketSeqNumber(inPacket);
    UInt16 firstSeqNum = 0;            
    theLen = sizeof(firstSeqNum); 

    if (    QTSS_NoErr != QTSS_GetValue(*theStreamPtr, qtssRTPStrFirstSeqNumber, 0, &firstSeqNum, &theLen)  )
         return true;

    if ( seqnum < firstSeqNum ) 
    {   
        //printf("RTPSessionOutput::FilterPacket don't send packet = %u < first=%lu\n", seqnum, firstSeqNum);
        return true;
    }
     
    //printf("RTPSessionOutput::FilterPacket found first packet = %u \n", firstSeqNum);

    fPreFilter = false;    
    return fPreFilter;
}


Bool16  RTPSessionOutput::PacketAlreadySent(QTSS_RTPStreamObject *theStreamPtr, UInt32 inFlags, UInt64* packetIDPtr)
{ 
    Assert(theStreamPtr);
    Assert(packetIDPtr);
    
    UInt32 theLen = 0;
    UInt64 *lastPacketIDPtr = NULL;
    Bool16 packetSent = false;
    
    if (inFlags & qtssWriteFlagsIsRTP) 
    {
        if ( (QTSS_NoErr == QTSS_GetValuePtr(*theStreamPtr, sLastRTPPacketIDAttr, 0, (void**)&lastPacketIDPtr, &theLen))
            && (*packetIDPtr <= *lastPacketIDPtr)
            )
        {    
            //printf("RTPSessionOutput::WritePacket Don't send RTP packet id =%qu\n", *packetIDPtr);
            packetSent = true;
        }
        
    } else if (inFlags & qtssWriteFlagsIsRTCP)
    {  
        if (    QTSS_NoErr == QTSS_GetValuePtr(*theStreamPtr, sLastRTCPPacketIDAttr, 0, (void**)&lastPacketIDPtr, &theLen)
            && (*packetIDPtr <= *lastPacketIDPtr)
           )
        {   
            //printf("RTPSessionOutput::WritePacket Don't send RTCP packet id =%qu last packet sent id =%qu\n", *packetIDPtr,*lastPacketIDPtr);
            packetSent = true;
        }
    }

    return packetSent;
}

Bool16  RTPSessionOutput::PacketReadyToSend(QTSS_RTPStreamObject *theStreamPtr,SInt64 *currentTimePtr, UInt32 inFlags, UInt64* packetIDPtr, SInt64* timeToSendThisPacketAgainPtr)
{ 
    return true;
    
}
  
QTSS_Error  RTPSessionOutput::TrackRTCPBaseTime(QTSS_RTPStreamObject *theStreamPtr, StrPtrLen* inPacketStrPtr, SInt64 *currentTimePtr, UInt32 inFlags, SInt64* packetLatenessInMSec, SInt64* timeToSendThisPacketAgain, UInt64* packetIDPtr, SInt64* arrivalTimeMSecPtr)
{
    Bool16 haveBaseTime = false;
    Bool16 haveAllFirstRTPs = true;
    
    UInt32 streamTimeScale = 0;
    UInt32  theLen =  sizeof(streamTimeScale);
    QTSS_Error writeErr = QTSS_GetValue(*theStreamPtr, qtssRTPStrTimescale, 0, (void *) &streamTimeScale, &theLen);
    Assert(writeErr == QTSS_NoErr);
         
    UInt32 baseTimeStamp = 0;
    theLen = sizeof(baseTimeStamp);
    if (!fMustSynch || QTSS_NoErr ==  QTSS_GetValue(*theStreamPtr, sBaseRTPTimeStampAttr, 0, (void*)&baseTimeStamp, &theLen) ) // we need a starting stream time that is synched 
    {
        haveBaseTime = true;
    }
    else
    {
        UInt64 earliestArrivalTime = ~(UInt64) 0; //max value
        UInt32 firstStreamTime = 0;
        SInt64 firstStreamArrivalTime = 0;
        QTSS_RTPStreamObject *findStream = NULL;
        
        if (fMustSynch || QTSS_NoErr != QTSS_GetValuePtr(*theStreamPtr, sBaseArrivalTimeStampAttr, 0, (void**)&fBaseArrivalTime, &theLen)  )
        {   // we don't have a base arrival time for the session see if we can set one now.
        
            for (SInt32 z = 0; QTSS_GetValuePtr(fClientSession, qtssCliSesStreamObjects, z, (void**)&findStream, &theLen) == QTSS_NoErr; z++)
            {
                SInt64* firstArrivalTimePtr = NULL;
                if (QTSS_NoErr != QTSS_GetValuePtr(*findStream, sFirstRTPArrivalTimeAttr, 0, (void**)&firstArrivalTimePtr, &theLen))
                {// no packet on this stream yet 
                    haveAllFirstRTPs = false; // not enough info to calc a base time
                    break;
                }
                else
                { // we have an arrival time see if it is the first for all streams
                   if ( (UInt64) *firstArrivalTimePtr < earliestArrivalTime )
                    {
                        earliestArrivalTime = *firstArrivalTimePtr;
                    }
                }

            }
        
            if (haveAllFirstRTPs) // we can now create a base arrival time and base stream time from that
            {
                
                writeErr = QTSS_SetValue(*theStreamPtr, sBaseArrivalTimeStampAttr, 0, &earliestArrivalTime, sizeof(SInt64));
                Assert(writeErr == QTSS_NoErr);
                fBaseArrivalTime = (SInt64) earliestArrivalTime;
            }
        }
        
        if (haveAllFirstRTPs)//sBaseRTPTimeStamp
        {   // we don't have a base stream time but we have a base session time so calculate the base stream time.
            theLen = sizeof(firstStreamTime);
            if (QTSS_NoErr != QTSS_GetValue(*theStreamPtr, sFirstRTPTimeStampAttr, 0, (void*)&firstStreamTime, &theLen))
                return QTSS_NoErr;
              
            theLen = sizeof(firstStreamArrivalTime);  
            if (QTSS_NoErr != QTSS_GetValue(*theStreamPtr, sFirstRTPArrivalTimeAttr, 0, (void*)&firstStreamArrivalTime, &theLen))
                return QTSS_NoErr;

            SInt64 arrivalTimeDiffMSecs = (firstStreamArrivalTime - fBaseArrivalTime);// + fBufferDelayMSecs;//add the buffer delay !! not sure about faster than real time arrival times....
            UInt32 timeDiffStreamTime = (UInt32)( ( (Float64) arrivalTimeDiffMSecs/(Float64) 1000.0) * (Float64) streamTimeScale );
            baseTimeStamp = firstStreamTime - timeDiffStreamTime;
            if (QTSS_NoErr == QTSS_SetValue(*theStreamPtr, sBaseRTPTimeStampAttr, 0, (void*)&baseTimeStamp, sizeof(baseTimeStamp)))
                haveBaseTime = true;               
             
            (void) QTSS_SetValue(*theStreamPtr, qtssRTPStrFirstTimestamp, 0, &baseTimeStamp, sizeof(baseTimeStamp));   
            
            fMustSynch = false;
            //printf("fBaseArrivalTime =%qd baseTimeStamp %"_U32BITARG_" streamStartTime=%qd diff =%qd\n", fBaseArrivalTime, baseTimeStamp, firstStreamArrivalTime, arrivalTimeDiffMSecs);
        }
    }

    return writeErr;

}

QTSS_Error  RTPSessionOutput::RewriteRTCP(QTSS_RTPStreamObject *theStreamPtr, StrPtrLen* inPacketStrPtr, SInt64 *currentTimePtr, UInt32 inFlags, SInt64* packetLatenessInMSec, SInt64* timeToSendThisPacketAgain, UInt64* packetIDPtr, SInt64* arrivalTimeMSecPtr)
{   
    UInt32  theLen;
    
    SInt64 firstRTPCurrentTime = 0;
    theLen = sizeof(firstRTPCurrentTime);  
    QTSS_GetValue(*theStreamPtr, sFirstRTPCurrentTimeAttr, 0, (void*)&firstRTPCurrentTime, &theLen);

    SInt64 firstRTPArrivalTime = 0;
    theLen = sizeof(firstRTPArrivalTime);  
    QTSS_GetValue(*theStreamPtr, sFirstRTPArrivalTimeAttr, 0, (void*)&firstRTPArrivalTime, &theLen);


    UInt32 rtpTime = 0;
    theLen = sizeof(rtpTime); 
    QTSS_GetValue(*theStreamPtr, sFirstRTPTimeStampAttr, 0, (void*)&rtpTime, &theLen);
 
        
    UInt32* theReport = (UInt32*) inPacketStrPtr->Ptr; 
    theReport+=2; // point to the NTP time stamp
    SInt64* theNTPTimestampP = (SInt64*)theReport;      
    *theNTPTimestampP = OS::HostToNetworkSInt64(OS::TimeMilli_To_1900Fixed64Secs(*currentTimePtr)); // time now

    UInt32 baseTimeStamp = 0;
    theLen = sizeof(baseTimeStamp);
    (void) QTSS_GetValue(*theStreamPtr, sBaseRTPTimeStampAttr, 0, (void*)&baseTimeStamp, &theLen); // we need a starting stream time that is synched 

    UInt32 streamTimeScale = 0;
    theLen =  sizeof(streamTimeScale);
    QTSS_GetValue(*theStreamPtr, qtssRTPStrTimescale, 0, (void *) &streamTimeScale, &theLen);

    SInt64 packetOffset = *currentTimePtr - fBaseArrivalTime; // real time that has passed
    packetOffset -=  (firstRTPCurrentTime - firstRTPArrivalTime); // less the initial buffer delay for this stream
    if (packetOffset < 0)
        packetOffset = 0;

    Float64 rtpTimeFromStart = (Float64) packetOffset / (Float64) 1000.0;
    UInt32 rtpTimeFromStartInScale =  (UInt32) (Float64) ((Float64) streamTimeScale * rtpTimeFromStart);
    //printf("rtptime offset time =%f in scale =%"_U32BITARG_"\n", rtpTimeFromStart, rtpTimeFromStartInScale );

    theReport += 2; // point to the rtp time stamp of "now" synched and scaled in stream time
    *theReport = htonl(baseTimeStamp + rtpTimeFromStartInScale); 
    
    theLen = sizeof(UInt32);                   
    UInt32 packetCount = 0;
    (void) QTSS_GetValue(*theStreamPtr, sStreamPacketCountAttr, 0, &packetCount,&theLen);
    theReport += 1; // point to the rtp packets sent
    *theReport = htonl(ntohl(*theReport) * 2); 
        
    UInt32 byteCount = 0;
    (void) QTSS_GetValue(*theStreamPtr, sStreamByteCountAttr, 0, &byteCount,&theLen);
    theReport += 1; // point to the rtp payload bytes sent
    *theReport = htonl(ntohl(*theReport) * 2); 
        
    return QTSS_NoErr;
}

QTSS_Error  RTPSessionOutput::TrackRTCPPackets(QTSS_RTPStreamObject *theStreamPtr, StrPtrLen* inPacketStrPtr, SInt64 *currentTimePtr, UInt32 inFlags, SInt64* packetLatenessInMSec, SInt64* timeToSendThisPacketAgain, UInt64* packetIDPtr, SInt64* arrivalTimeMSecPtr)
{
    QTSS_Error writeErr = QTSS_NoErr;

    Assert(inFlags & qtssWriteFlagsIsRTCP);

    if (!(inFlags & qtssWriteFlagsIsRTCP))
        return -1;
 
    this->TrackRTCPBaseTime(theStreamPtr,inPacketStrPtr,currentTimePtr,inFlags, packetLatenessInMSec,timeToSendThisPacketAgain, packetIDPtr,arrivalTimeMSecPtr);

    this->RewriteRTCP(theStreamPtr,inPacketStrPtr,currentTimePtr,inFlags, packetLatenessInMSec,timeToSendThisPacketAgain, packetIDPtr,arrivalTimeMSecPtr);

 
    return writeErr;
}

QTSS_Error  RTPSessionOutput::TrackRTPPackets(QTSS_RTPStreamObject *theStreamPtr, StrPtrLen* inPacketStrPtr, SInt64 *currentTimePtr, UInt32 inFlags, SInt64* packetLatenessInMSec, SInt64* timeToSendThisPacketAgain, UInt64* packetIDPtr, SInt64* arrivalTimeMSecPtr)
{
    QTSS_Error writeErr = QTSS_NoErr;

    Assert(inFlags & qtssWriteFlagsIsRTP);

    if (!(inFlags & qtssWriteFlagsIsRTP))
        return QTSS_NoErr;
    
    ReflectorPacket packetContainer;
    packetContainer.SetPacketData(inPacketStrPtr->Ptr, inPacketStrPtr->Len);
    packetContainer.fIsRTCP = false;
    SInt64 *theTimePtr = NULL;
    UInt32 theLen = 0;
    
    if (QTSS_NoErr != QTSS_GetValuePtr(*theStreamPtr, sFirstRTPArrivalTimeAttr, 0, (void**)&theTimePtr, &theLen))
    {                   
        UInt32 theSSRC = packetContainer.GetSSRC(packetContainer.fIsRTCP);
        (void) QTSS_SetValue(*theStreamPtr, sStreamSSRCAttr, 0, &theSSRC, sizeof(theSSRC));
        
        UInt32 rtpTime = packetContainer.GetPacketRTPTime();
        writeErr = QTSS_SetValue(*theStreamPtr, sFirstRTPTimeStampAttr, 0, &rtpTime, sizeof(rtpTime));
        Assert(writeErr == QTSS_NoErr);
        
        writeErr = QTSS_SetValue(*theStreamPtr, sFirstRTPArrivalTimeAttr, 0, arrivalTimeMSecPtr, sizeof(SInt64));
        Assert(writeErr == QTSS_NoErr);
        
        writeErr = QTSS_SetValue(*theStreamPtr, sFirstRTPCurrentTimeAttr, 0, currentTimePtr, sizeof(SInt64));
        Assert(writeErr == QTSS_NoErr);

        UInt32 initValue = 0;
        writeErr = QTSS_SetValue(*theStreamPtr, sStreamByteCountAttr, 0, &initValue, sizeof(UInt32));
        Assert(writeErr == QTSS_NoErr);
        
        //printf("first rtp on stream stream=%"_U32BITARG_" ssrc=%"_U32BITARG_" rtpTime=%"_U32BITARG_" arrivalTimeMSecPtr=%qd currentTime=%qd\n",(UInt32) theStreamPtr, theSSRC, rtpTime, *arrivalTimeMSecPtr, *currentTimePtr);
        
    }
    else
    {
        UInt32* packetCountPtr = NULL;
        UInt32* byteCountPtr = NULL;
        UInt32 theLen = 0;
                
        writeErr = QTSS_GetValuePtr(*theStreamPtr, sStreamByteCountAttr, 0, (void**) &byteCountPtr,&theLen);
        if (writeErr == QTSS_NoErr && theLen > 0)
            *byteCountPtr += inPacketStrPtr->Len - 12;// 12 header bytes
            
            
       UInt32* theSSRCPtr = 0;
        (void) QTSS_GetValuePtr(*theStreamPtr, sStreamSSRCAttr, 0, (void**)&theSSRCPtr, &theLen);
        if (*theSSRCPtr != packetContainer.GetSSRC(packetContainer.fIsRTCP))
        {    

           
           (void) QTSS_RemoveValue(*theStreamPtr,sFirstRTPArrivalTimeAttr,0);
           (void) QTSS_RemoveValue(*theStreamPtr,sFirstRTPTimeStampAttr,0);
           (void) QTSS_RemoveValue(*theStreamPtr,sFirstRTPCurrentTimeAttr,0);
           (void) QTSS_RemoveValue(*theStreamPtr,sStreamPacketCountAttr,0);
           (void) QTSS_RemoveValue(*theStreamPtr,sStreamByteCountAttr,0);
           fMustSynch = true;
           
           //printf("found different ssrc =%"_U32BITARG_" packetssrc=%"_U32BITARG_"\n",*theSSRCPtr, packetContainer.GetSSRC(packetContainer.fIsRTCP));
           
        }
        
              
            
    } 
    
    return writeErr; 

}

QTSS_Error  RTPSessionOutput::TrackPackets(QTSS_RTPStreamObject *theStreamPtr, StrPtrLen* inPacketStrPtr, SInt64 *currentTimePtr, UInt32 inFlags, SInt64* packetLatenessInMSec, SInt64* timeToSendThisPacketAgain, UInt64* packetIDPtr, SInt64* arrivalTimeMSecPtr)
{
    if (this->IsUDP())
        return QTSS_NoErr;

    if (inFlags & qtssWriteFlagsIsRTCP)
        (void) this->TrackRTCPPackets(theStreamPtr, inPacketStrPtr, currentTimePtr,inFlags,  packetLatenessInMSec, timeToSendThisPacketAgain, packetIDPtr,arrivalTimeMSecPtr);
    else if (inFlags & qtssWriteFlagsIsRTP)
        (void) this->TrackRTPPackets(theStreamPtr, inPacketStrPtr, currentTimePtr,inFlags,  packetLatenessInMSec, timeToSendThisPacketAgain, packetIDPtr,arrivalTimeMSecPtr);
           
    return QTSS_NoErr;
}


QTSS_Error  RTPSessionOutput::WritePacket(StrPtrLen* inPacket, void* inStreamCookie, UInt32 inFlags, SInt64 packetLatenessInMSec, SInt64* timeToSendThisPacketAgain, UInt64* packetIDPtr, SInt64* arrivalTimeMSecPtr, Bool16 firstPacket)
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
            if (  (inFlags & qtssWriteFlagsIsRTP) && this->FilterPacket(theStreamPtr, inPacket) )
                return  QTSS_NoErr; // keep looking at packets
                
            if (this->PacketAlreadySent(theStreamPtr,inFlags, packetIDPtr)) 
                return QTSS_NoErr; // keep looking at packets
                
            if (!this->PacketReadyToSend(theStreamPtr,&currentTime, inFlags, packetIDPtr, timeToSendThisPacketAgain)) 
            {   //qtss_printf("QTSS_WouldBlock\n");
                return QTSS_WouldBlock; // stop not ready to send packets now
            }
                                          
    
       // TrackPackets below is for re-writing the rtcps we don't use it right now-- shouldn't need to    
       // (void) this->TrackPackets(theStreamPtr, inPacket, &currentTime,inFlags,  &packetLatenessInMSec, timeToSendThisPacketAgain, packetIDPtr,arrivalTimeMSecPtr);

            QTSS_PacketStruct thePacket;
            thePacket.packetData = inPacket->Ptr;
            SInt64 delayMSecs = fBufferDelayMSecs - (currentTime - *arrivalTimeMSecPtr);
            thePacket.packetTransmitTime = (currentTime - packetLatenessInMSec);
            if (fBufferDelayMSecs > 0 ) 
                thePacket.packetTransmitTime += delayMSecs; // add buffer time where oldest buffered packet as now == 0 and newest is entire buffer time in the future.
 
            writeErr = QTSS_Write(*theStreamPtr, &thePacket, inPacket->Len, NULL, inFlags | qtssWriteFlagsWriteBurstBegin); 
            if (writeErr == QTSS_WouldBlock)
            {  
                 //qtss_printf("QTSS_Write == QTSS_WouldBlock\n");
                //
                // We are flow controlled. See if we know when flow control will be lifted and report that
                *timeToSendThisPacketAgain = thePacket.suggestedWakeupTime;
                 
                if (firstPacket)
                {   fBufferDelayMSecs = (currentTime - *arrivalTimeMSecPtr);
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
                    (void) QTSS_SetValue (*theStreamPtr, sLastRTPPacketIDAttr, 0, packetIDPtr, sizeof(UInt64));
                }
                else if (inFlags & qtssWriteFlagsIsRTCP)
                {
                    (void) QTSS_SetValue (*theStreamPtr, sLastRTCPPacketIDAttr, 0, packetIDPtr, sizeof(UInt64));                            
                    (void) QTSS_SetValue (*theStreamPtr, sLastRTCPTransmitAttr, 0, &currentTime, sizeof(UInt64));
                }
                       
                       
                       
               { // increment packet counts
                    UInt32* packetCountPtr = NULL;
                    UInt32 theLen = 0;
                    
                    (void) QTSS_GetValuePtr(*theStreamPtr, sStreamPacketCountAttr, 0,(void**) &packetCountPtr,&theLen);
                    if (theLen > 0)
                    {   *packetCountPtr += 1;
                        //printf("SET sStreamPacketCountAttr =%lu\n", *packetCountPtr);
                    }
               }
           }
        }

        if ( writeErr != QTSS_NoErr )
            break;
    }
        
    return writeErr;
}

UInt16 RTPSessionOutput::GetPacketSeqNumber(StrPtrLen* inPacket)
{
    if (inPacket->Len < 4)
        return 0;
    
    //The RTP seq number is the second short of the packet
    UInt16* seqNumPtr = (UInt16*)inPacket->Ptr;
    return ntohs(seqNumPtr[1]);
}

void RTPSessionOutput::SetPacketSeqNumber(StrPtrLen* inPacket, UInt16 inSeqNumber)
{
    if (inPacket->Len < 4)
        return;

    //The RTP seq number is the second short of the packet
    UInt16* seqNumPtr = (UInt16*)inPacket->Ptr;
    seqNumPtr[1] = htons(inSeqNumber);
}

// this routine is not used
Bool16 RTPSessionOutput::PacketShouldBeThinned(QTSS_RTPStreamObject inStream, StrPtrLen* inPacket)
{
    return false; // function is disabled.
    
    static UInt16 sZero = 0;
    //This function determines whether the packet should be dropped.
    //It also adjusts the sequence number if necessary

    if (inPacket->Len < 4)
        return false;
    
    UInt16 curSeqNum = this->GetPacketSeqNumber(inPacket);
    UInt32* curQualityLevel = NULL;
    UInt16* nextSeqNum = NULL;
    UInt16* theSeqNumOffset = NULL;
    SInt64* lastChangeTime = NULL;
    
    UInt32 theLen = 0;
    (void)QTSS_GetValuePtr(inStream, qtssRTPStrQualityLevel, 0, (void**)&curQualityLevel, &theLen);
    if ((curQualityLevel == NULL) || (theLen != sizeof(UInt32)))
        return false;
    (void)QTSS_GetValuePtr(inStream, sNextSeqNumAttr, 0, (void**)&nextSeqNum, &theLen);
    if ((nextSeqNum == NULL) || (theLen != sizeof(UInt16)))
    {
        nextSeqNum = &sZero;
        (void)QTSS_SetValue(inStream, sNextSeqNumAttr, 0, nextSeqNum, sizeof(UInt16));
    }
    (void)QTSS_GetValuePtr(inStream, sSeqNumOffsetAttr, 0, (void**)&theSeqNumOffset, &theLen);
    if ((theSeqNumOffset == NULL) || (theLen != sizeof(UInt16)))
    {
        theSeqNumOffset = &sZero;
        (void)QTSS_SetValue(inStream, sSeqNumOffsetAttr, 0, theSeqNumOffset, sizeof(UInt16));
    }
    UInt16 newSeqNumOffset = *theSeqNumOffset;
    
    (void)QTSS_GetValuePtr(inStream, sLastQualityChangeAttr, 0, (void**)&lastChangeTime, &theLen);
    if ((lastChangeTime == NULL) || (theLen != sizeof(SInt64)))
    {   static SInt64 startTime = 0;
        lastChangeTime = &startTime;
        (void)QTSS_SetValue(inStream, sLastQualityChangeAttr, 0, lastChangeTime, sizeof(SInt64));
    }
    
    SInt64 timeNow = OS::Milliseconds();
    if (*lastChangeTime == 0 || *curQualityLevel == 0) 
        *lastChangeTime =timeNow;
    
    if (*curQualityLevel > 0 && ((*lastChangeTime + 30000) < timeNow) ) // 30 seconds between reductions
    {   *curQualityLevel -= 1; // reduce quality value.  If we quality doesn't change then we may have hit some steady state which we can't get out of without thinning or increasing the quality
        *lastChangeTime =timeNow; 
        //qtss_printf("RTPSessionOutput set quality to %"_U32BITARG_"\n",*curQualityLevel);
    }

    //Check to see if we need to drop to audio only
    if ((*curQualityLevel >= ReflectorSession::kAudioOnlyQuality) &&
        (*nextSeqNum == 0))
    {
#if REFLECTOR_THINNING_DEBUGGING || RTP_SESSION_DEBUGGING
        qtss_printf(" *** Reflector Dropping to audio only *** \n");
#endif
        //All we need to do in this case is mark the sequence number of the first dropped packet
        (void)QTSS_SetValue(inStream, sNextSeqNumAttr, 0, &curSeqNum, sizeof(UInt16));  
         *lastChangeTime =timeNow;  
    }
    
    
    //Check to see if we can reinstate video
    if ((*curQualityLevel == ReflectorSession::kNormalQuality) && (*nextSeqNum != 0))
    {
        //Compute the offset amount for each subsequent sequence number. This offset will
        //alter the sequence numbers so that they increment normally (providing the illusion to the
        //client that there are no missing packets)
        newSeqNumOffset = (*theSeqNumOffset) + (curSeqNum - (*nextSeqNum));
        (void)QTSS_SetValue(inStream, sSeqNumOffsetAttr, 0, &newSeqNumOffset, sizeof(UInt16));
        (void)QTSS_SetValue(inStream, sNextSeqNumAttr, 0, &sZero, sizeof(UInt16));
    }
    
    //tell the caller whether to drop this packet or not.
    if (*curQualityLevel >= ReflectorSession::kAudioOnlyQuality)
        return true;
    else
    {
        //Adjust the sequence number of the current packet based on the offset, if any
        curSeqNum -= newSeqNumOffset;
        this->SetPacketSeqNumber(inPacket, curSeqNum);
        return false;
    }
}

void RTPSessionOutput::TearDown()
{
    QTSS_CliSesTeardownReason reason = qtssCliSesTearDownBroadcastEnded;
    (void)QTSS_SetValue(fClientSession, qtssCliTeardownReason, 0, &reason, sizeof(reason));     
    (void)QTSS_Teardown(fClientSession);
}


