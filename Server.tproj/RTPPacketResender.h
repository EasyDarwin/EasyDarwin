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
    File:       RTPPacketResender.h

    Contains:   RTPPacketResender class to buffer and track re-transmits of RTP packets.
    
    the ctor copies the packet data, sets a timer for the packet's age limit and
    another timer for it's possible re-transmission.
    A duration timer is started to measure the RTT based on the client's ack.
    
*/

#ifndef __RTP_PACKET_RESENDER_H__
#define __RTP_PACKET_RESENDER_H__

#include "PLDoubleLinkedList.h"

#include "RTPBandwidthTracker.h"
#include "DssStopwatch.h"
#include "UDPSocket.h"
#include "OSMemory.h"
#include "OSBufferPool.h"
#include "OSMutex.h"

#define RTP_PACKET_RESENDER_DEBUGGING 0

class MyAckListLog;

class RTPResenderEntry
{
    public:
        
        void*               fPacketData;
        UInt32              fPacketSize;
        Bool16              fIsSpecialBuffer;
        SInt64              fExpireTime;
        SInt64              fAddedTime;
        SInt64              fOrigRetransTimeout;
        UInt32              fNumResends;
        UInt16              fSeqNum;
#if RTP_PACKET_RESENDER_DEBUGGING
        UInt32              fPacketArraySizeWhenAdded;
#endif
};


class RTPPacketResender
{
    public:
        
        RTPPacketResender();
        ~RTPPacketResender();
        
        //
        // These must be called before using the object
        void                SetDestination(UDPSocket* inOutputSocket, UInt32 inDestAddr, UInt16 inDestPort);
        void                SetBandwidthTracker(RTPBandwidthTracker* inTracker) { fBandwidthTracker = inTracker; }
        
        //
        // AddPacket adds a new packet to the resend queue. This will not send the packet.
        // AddPacket itself is not thread safe.
        void                AddPacket( void * rtpPacket, UInt32 packetSize, SInt32 ageLimitInMsec );
        
        //
        // Acks a packet. Also not thread safe.
        void                AckPacket( UInt16 sequenceNumber, SInt64& inCurTimeInMsec );

        //
        // Resends outstanding packets in the queue. Guess what. Not thread safe.
        void                ResendDueEntries();
        
        //
        // Clear outstanding packets - if we no longer care about any of the
        // outstanding, unacked packets
        void                ClearOutstandingPackets();

        //
        // ACCESSORS
        Bool16              IsFlowControlled()      { return fBandwidthTracker->IsFlowControlled(); }
        SInt32              GetMaxPacketsInList()   { return fMaxPacketsInList; }
        SInt32              GetNumPacketsInList()   { return fPacketsInList; }
        SInt32              GetNumResends()         { return fNumResends; }
        
        static UInt32       GetNumRetransmitBuffers() { return sBufferPool.GetTotalNumBuffers(); }
        static UInt32       GetWastedBufferBytes() { return sNumWastedBytes; }

#if RTP_PACKET_RESENDER_DEBUGGING
        void                SetDebugInfo(UInt32 trackID, UInt16 remoteRTCPPort, UInt32 curPacketDelay);
        void                SetLog( StrPtrLen *logname );
        UInt32              SpillGuts(UInt32 inBytesSentThisInterval);
        void                LogClose(SInt64 inTimeSpentInFlowControl);
        void                logprintf( const char * format, ... );

#else
        void                SetLog( StrPtrLen * /*logname*/) {}
#endif
        
    private:
    
        // Tracking the capacity of the network
        RTPBandwidthTracker* fBandwidthTracker;
        
        // Who to send to
        UDPSocket*          fSocket;
        UInt32              fDestAddr;
        UInt16              fDestPort;

        UInt32              fMaxPacketsInList;
        UInt32              fPacketsInList;
        UInt32              fNumResends;                // how many total retransmitted packets
        UInt32              fNumExpired;                // how many total packets dropped
        UInt32              fNumAcksForMissingPackets;  // how many acks received in the case where the packet was not in the list
        UInt32              fNumSent;                   // how many packets sent

#if RTP_PACKET_RESENDER_DEBUGGING
        MyAckListLog        *fLogger;
        
        UInt32              fTrackID;
        UInt16              fRemoteRTCPPort;
        UInt32              fCurrentPacketDelay;
        DssDurationTimer    fInfoDisplayTimer;
#endif
        
        RTPResenderEntry*   fPacketArray;
        UInt16              fStartSeqNum;
        UInt32              fPacketArraySize;
        UInt32              fPacketArrayMask;
        UInt16              fHighestSeqNum;
        UInt32              fLastUsed;
        OSMutex             fPacketQMutex;

        RTPResenderEntry*   GetEntryByIndex(UInt16 inIndex);
        RTPResenderEntry*   GetEntryBySeqNum(UInt16 inSeqNum);

        RTPResenderEntry*   GetEmptyEntry(UInt16 inSeqNum, UInt32 inPacketSize);
        void ReallocatePacketArray();
        void RemovePacket(UInt32 packetIndex, Bool16 reuse=true);
        void RemovePacket(RTPResenderEntry* inEntry);

        static OSBufferPool sBufferPool;
        static unsigned int sNumWastedBytes;
        
        void            UpdateCongestionWindow(SInt32 bytesToOpenBy );
};

#endif //__RTP_PACKET_RESENDER_H__
