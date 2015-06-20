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
	Copyleft (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       SDPSourceInfo.h

    Contains:   This object takes input SDP data, and uses it to support the SourceInfo
                API.

	Works only for QTSS

*/

#ifndef __SDP_SOURCE_INFO_H__
#define __SDP_SOURCE_INFO_H__

#include "StrPtrLen.h"
#include "SourceInfo.h"
#include "StringParser.h"

class SDPSourceInfo : public SourceInfo
{
    public:
    
        // Uses the SDP Data to build up the StreamInfo structures
        SDPSourceInfo(char* sdpData, UInt32 sdpLen) { Parse(sdpData, sdpLen); }
        SDPSourceInfo() {}
        virtual ~SDPSourceInfo();
        
        // Parses out the SDP file provided, sets up the StreamInfo structures
        void    Parse(char* sdpData, UInt32 sdpLen);

        // This function uses the Parsed SDP file, and strips out all the network information,
        // producing an SDP file that appears to be local.
        virtual char*   GetLocalSDP(UInt32* newSDPLen);

        // Returns the SDP data
        StrPtrLen*  GetSDPData()    { return &fSDPData; }
        
        // Utility routines
        
        // Assuming the parser is currently pointing at the beginning of an dotted-
        // decimal IP address, this consumes it (stopping at inStopChar), and returns
        // the IP address (host ordered) as a UInt32
        static UInt32 GetIPAddr(StringParser* inParser, char inStopChar);
      
    private:

        enum
        {
            kDefaultTTL = 15    //UInt16
        };
        StrPtrLen   fSDPData;
};
#endif // __SDP_SOURCE_INFO_H__

