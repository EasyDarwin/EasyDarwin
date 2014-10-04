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
    File:       RTCPAPPPacket.cpp

    Contains:   RTCPAPPPacket de-packetizing classes

    
*/


#include "RTCPAPPPacket.h"
#include "MyAssert.h"
#include "OS.h"
#include "OSMemory.h"
#include "ResizeableStringFormatter.h"


RTCPAPPPacket::RTCPAPPPacket(Bool16 debug) : 
    fRTCPAPPDataBuffer(NULL),
    fAPPDataBufferSize(0),
    mDumpArray(NULL), 
    mDumpArrayStrDeleter(NULL),
    fDumpReport(),
    fDebug(debug)
{
    if (fDebug)
    {
       mDumpArray = NEW char[kmDumpArraySize];
       mDumpArray[0] = '\0';
       mDumpArrayStrDeleter.Set(mDumpArray);
    }

}

void RTCPAPPPacket::Dump()//Override
{

    
    RTCPPacket::Dump();
    fDumpReport.PutTerminator();
    qtss_printf("%s\n", fDumpReport.GetBufPtr());
    fDumpReport.Reset();
}


Bool16 RTCPAPPPacket::ParseAPPPacketHeader(UInt8* inPacketBuffer, UInt32 inPacketLength)
{   
    if (inPacketLength < kRTCPPacketSizeInBytes + kRTCPAPPHeaderSizeInBytes)
        return false;
        
   return true;    
}

Bool16 RTCPAPPPacket::ParseAPPPacket(UInt8* inPacketBuffer, UInt32 inPacketLength)
{
    if (false == this->ParsePacket(inPacketBuffer, inPacketLength) ) // base class
        return false;
                      
    return this->ParseAPPPacketHeader(inPacketBuffer, inPacketLength);

}



