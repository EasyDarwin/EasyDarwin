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
//
// QTAtom_hinf:
//   The 'hinf' QTAtom class.


// -------------------------------------
// Includes
//
#include <stdio.h>
#include <time.h>

#include "QTFile.h"

#include "QTAtom.h"
#include "QTAtom_hinf.h"



// -------------------------------------
// Macros
//
#define DEBUG_PRINT(s) if(fDebug) qtss_printf s
#define DEEP_DEBUG_PRINT(s) if(fDeepDebug) qtss_printf s



// -------------------------------------
// Constants
//
const char *    hinfAtom_TotalRTPBytes64        = ":trpy";
const char *    hinfAtom_TotalRTPBytes32        = ":totl";
const char *    hinfAtom_TotalRTPPackets64      = ":nump";
const char *    hinfAtom_TotalRTPPackets32      = ":npck";
const char *    hinfAtom_TotalPayloadBytes64    = ":tpyl";
const char *    hinfAtom_TotalPayloadBytes32    = ":tpay";
const char *    hinfAtom_MaxDataRate64          = ":maxr";
const char *    hinfAtom_TotalMediaBytes64      = ":dmed";
const char *    hinfAtom_TotalImmedBytes64      = ":dimm";
const char *    hinfAtom_TotalRepeatBytes64     = ":drep";
const char *    hinfAtom_MinTransTime32         = ":tmin";
const char *    hinfAtom_MaxTransTime32         = ":tmax";
const char *    hinfAtom_MaxPacketSize32        = ":pmax";
const char *    hinfAtom_MaxPacketDuration32    = ":dmax";
const char *    hinfAtom_PayloadType            = ":payt";
/*
'trpy' 8 bytes The total number of bytes that will be sent,
including 12-byte RTP headers, but not including
any network headers.
'totl' 4 bytes 4-byte version of 'trpy'
'nump' 8 bytes The total number of network packets that will be
sent (if the application knows there is a 28-byte
network header, it can multiply 28 by this number
and add it to the Ôtrpy?value to get the true
number of bytes sent.
'npck' 4 bytes 4-byte version of 'nump'
'tpyl' 8 bytes The total number of bytes that will be sent, not
including 12-byte RTP headers.
'tpay' 4 bytes 4-byte version of 'tpyl'
'maxr' 8 bytes The maximum data rate. This atom contains two
numbers: g, followed by m (both 32-bit values). g
is the granularity, in milliseconds. m is the
maximum data rate given that granularity. For
example, if g is 1 second, then m is the maximum
data rate over any 1 second. There may be
multiple 'maxr' atoms, with different values for g.
The maximum data rate calculation does not
include any network headers (but does include
12-byte RTP headers).
'dmed' 8 bytes The number of bytes from the media track to be
sent.
'dimm' 8 bytes The number of bytes of immediate data to be sent.
'drep' 8 bytes The number of bytes of repeated data to be sent.
'tmin' 4 bytes The smallest relative transmission time, in
milliseconds.
'tmax' 4 bytes The largest relative transmission time, in
milliseconds.

'pmax' 4 bytes The largest packet, in bytes; includes 12-byte RTP
header.
'dmax' 4 bytes The largest packet duration, in milliseconds.
'payt' variable The payload type, which includes payload
number (32-bits) followed by rtpmap payload
string (Pascal string).
*/

// -------------------------------------
// Constructors and destructors
//
QTAtom_hinf::QTAtom_hinf(QTFile * File, QTFile::AtomTOCEntry * TOCEntry, Bool16 Debug, Bool16 DeepDebug)
    : QTAtom(File, TOCEntry, Debug, DeepDebug),
    fTotalRTPBytes32(0), //totl
    fTotalRTPBytes64(0), //trpy
    fTotalRTPPackets32(0), //nump
    fTotalRTPPackets64(0), //npck
    fTotalPayLoadBytes32(0), //tpay
    fTotalPayLoadBytes64(0), //tpyl
    fMaxDataRate64(0), //maxr
    fTotalMediaBytes64(0), //dmed
    fTotalImmediateBytes64(0), //dimm   
    fTotalRepeatBytes64(0), //drep
    fMinTransTime32(0), //tmin
    fMaxTransTime32(0), //tmax
    fMaxPacketSizeBytes32(0), //pmax
    fMaxPacketDuration32(0), //dmax
    fPayloadID(0)//payt
{
    fPayloadStr[0] = 0;//payt
}

QTAtom_hinf::~QTAtom_hinf(void)
{
}



// -------------------------------------
// Initialization functions
//
Bool16 QTAtom_hinf::Initialize(void)
{
    //
    // Parse this atom's sub-atoms.
    ReadSubAtomInt32(hinfAtom_TotalRTPBytes32, &fTotalRTPBytes32);
    ReadSubAtomInt32(hinfAtom_TotalRTPPackets32, &fTotalRTPPackets32);
    ReadSubAtomInt32(hinfAtom_TotalPayloadBytes32, &fTotalPayLoadBytes32);

    
    ReadSubAtomInt64(hinfAtom_TotalRTPBytes64, &fTotalRTPBytes64);
    ReadSubAtomInt64(hinfAtom_TotalRTPPackets64, &fTotalRTPPackets64);
    ReadSubAtomInt64(hinfAtom_TotalPayloadBytes64, &fTotalPayLoadBytes64);
    
    ReadSubAtomInt64(hinfAtom_MaxDataRate64, &fMaxDataRate64);
    ReadSubAtomInt64(hinfAtom_TotalMediaBytes64, &fTotalMediaBytes64);
    ReadSubAtomInt64(hinfAtom_TotalImmedBytes64, &fTotalImmediateBytes64);
    ReadSubAtomInt64(hinfAtom_TotalRepeatBytes64, &fTotalRepeatBytes64);
    
    
    ReadSubAtomInt32(hinfAtom_MinTransTime32, &fMinTransTime32);
    ReadSubAtomInt32(hinfAtom_MaxTransTime32, &fMaxTransTime32);
    ReadSubAtomInt32(hinfAtom_MaxPacketSize32, &fMaxPacketSizeBytes32);
    ReadSubAtomInt32(hinfAtom_MaxPacketDuration32, &fMaxPacketDuration32);
    
    ReadSubAtomInt32(hinfAtom_PayloadType, &fPayloadID);
    if (fPayloadID != 0)
    {   SInt8 len = 0;
        ReadSubAtomBytes(hinfAtom_PayloadType, (char*)fPayloadStr, 5);
        len = fPayloadStr[4];
        if (len > 0)
        {   ReadSubAtomBytes(hinfAtom_PayloadType, (char*)fPayloadStr, len+5);
            ::memmove(fPayloadStr,&fPayloadStr[5],len);
            fPayloadStr[len] = 0;
        }
    }
    //
    // This atom has been successfully read in.
    return true;
}


// -------------------------------------
// Debugging functions
//
void QTAtom_hinf::DumpAtom(void)
{
    DEBUG_PRINT(("QTAtom_hinf::DumpAtom - Dumping atom.\n"));
    DEBUG_PRINT(("QTAtom_hinf::DumpAtom - ..Total RTP bytes: %" _64BITARG_ "u\n", this->GetTotalRTPBytes()));
#ifndef __Win32__
    DEBUG_PRINT(("QTAtom_hinf::DumpAtom - ....Average bitrate: %.2f Kbps\n", ((this->GetTotalRTPBytes() << 3) / fFile->GetDurationInSeconds()) / 1024));
#endif
    DEBUG_PRINT(("QTAtom_hinf::DumpAtom - ..Total RTP packets: %" _64BITARG_ "u\n", this->GetTotalRTPPackets()));
    DEBUG_PRINT(("QTAtom_hinf::DumpAtom - ....Average packet size: %" _64BITARG_ "u\n", this->GetTotalRTPBytes() / this->GetTotalRTPPackets()));
    
    DEBUG_PRINT(("QTAtom_hinf::DumpAtom - ..Total Payload bytes: %" _64BITARG_ "u\n", this->GetTotalPayLoadBytes()));
    
    DEBUG_PRINT(("QTAtom_hinf::DumpAtom - ..Maximum Data Rate: %" _64BITARG_ "u\n", this->GetMaxDataRate()));
    DEBUG_PRINT(("QTAtom_hinf::DumpAtom - ..Total Media Track bytes: %" _64BITARG_ "u\n", this->GetTotalMediaBytes()));
    DEBUG_PRINT(("QTAtom_hinf::DumpAtom - ..Total Repeat Packet bytes: %" _64BITARG_ "u\n", this->GetRepeatBytes()));
    DEBUG_PRINT(("QTAtom_hinf::DumpAtom - ..Total Immediate Bytes: %" _64BITARG_ "u\n", this->GetTotalImmediateBytes()));

    DEBUG_PRINT(("QTAtom_hinf::DumpAtom - ..Minimum Transmission Time: %"   _U32BITARG_   "\n", this->GetMinTransTime()));
    DEBUG_PRINT(("QTAtom_hinf::DumpAtom - ..Maximum Transmission Time: %"   _U32BITARG_   "\n", this->GetMaxTransTime()));
    DEBUG_PRINT(("QTAtom_hinf::DumpAtom - ..Maximum Packet Size bytes: %"   _U32BITARG_   "\n", this->GetMaxPacketSizeBytes()));
    DEBUG_PRINT(("QTAtom_hinf::DumpAtom - ..Maximum Maximum Packet Duration: %"   _U32BITARG_   "\n", this->GetMaxPacketDuration()));
    
    DEBUG_PRINT(("QTAtom_hinf::DumpAtom - ..Payload ID: %"   _U32BITARG_   "\n", this->GetPayLoadID()));
    DEBUG_PRINT(("QTAtom_hinf::DumpAtom - ..Payload string: %s\n", this->GetPayLoadStr()));

}
