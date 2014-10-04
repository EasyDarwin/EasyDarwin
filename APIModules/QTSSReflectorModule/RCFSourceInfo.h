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
    File:       RCFSourceInfo.h

    Contains:   This object takes input RCF data, and uses it to support the SourceInfo
                API.

    

*/

#ifndef __RCF_SOURCE_INFO_H__
#define __RCF_SOURCE_INFO_H__

#include "StrPtrLen.h"
#include "SourceInfo.h"
#include "XMLParser.h"
#include "StringParser.h"

class RCFSourceInfo : public SourceInfo
{
    public:
    
        // Uses the SDP Data to build up the StreamInfo structures
        RCFSourceInfo() : fName(NULL) {}
        RCFSourceInfo(XMLTag* relayTag) : fName(NULL) { Parse(relayTag); }
        RCFSourceInfo(const RCFSourceInfo& copy):SourceInfo(copy) { this->SetName(copy.fName); }
        virtual ~RCFSourceInfo();
        
        // Parses out the SDP file provided, sets up the StreamInfo structures
        void    Parse(XMLTag* relayTag);
        
        // Parses relay_destination lines and builds OutputInfo structs
        void    ParseRelayDestinations(XMLTag* relayTag);
        
                void    SetName(const char* inName);
                char*   Name() { return fName; }
                
    protected:
        virtual void ParseDestination(XMLTag* destTag, UInt32 index);
        virtual void ParseAnnouncedDestination(XMLTag* destTag, UInt32 index);
        virtual void AllocateOutputArray(UInt32 numOutputs);
                
                char*   fName;

};
#endif // __SDP_SOURCE_INFO_H__


