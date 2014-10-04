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
    This file defines a good file format to use for storing RTP streams.
    It is optimized for linear reads through the file, but also allows for
    seeking using a "Block Map", similar to a QuickTime sample table but more
    compressed.
    
    RTP PACKET FILE FORMAT DEFINITION:

    1 RTPFileHeader 
    __ SDP DATA __
    1 RTPFileTrackInfo per track
    __ BLOCK MAP__
    
    packet, packet, packet
    
    Each packet is:
    
    1 RTPFilePacket, followed by data
    
    No partial packets allowed between blocks, blocks should finish with a pad packet

*/ 


#ifndef __RTP_FILE_DEFS__
#define __RTP_FILE_DEFS__

#include "OSHeaders.h"

#define RTP_FILE_CURRENT_VERSION 0

typedef struct RTPFileHeader
{
    UInt32  fVersion;           // Version 0
    Float64 fMovieDuration;
    UInt32  fSDPLen;
    UInt32  fNumTracks;
    UInt32  fMaxTrackID;
    UInt32  fBlockMapSize;
    UInt32  fDataStartPos;
    
} RTPFileHeader;

typedef struct RTPFileTrackInfo
{
    UInt32  fID;
    UInt32  fTimescale;
    UInt64  fBytesInTrack;
    Float64 fDuration;
    
} RTPFileTrackInfo;

typedef struct RTPFilePacket
{
    UInt16 fTrackID;
    UInt16 fPacketLength;
    Float64 fTransmitTime;
    
} RTPFilePacket;

static const UInt32 kBlockSize = 32768;
static const UInt32 kBlockMask = 0xFFF8000;
static const UInt16 kPaddingBit = 0x8000;

#endif //__RTP_FILE_DEFS__
