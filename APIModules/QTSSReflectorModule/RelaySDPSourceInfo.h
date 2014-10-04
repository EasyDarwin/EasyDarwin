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
    File:       RelaySDPSourceInfo.h

    Contains:   This object takes input SDP data, and uses it to support the SourceInfo
                API. It looks for the x-qt-relay lines put in by some broadcasters,
                and uses that information to construct OutputInfo objects.


*/

#ifndef __RELAY_SDP_SOURCE_INFO_H__
#define __RELAY_SDP_SOURCE_INFO_H__

#include "StrPtrLen.h"
#include "SourceInfo.h"

class RelaySDPSourceInfo : public SourceInfo
{
    public:
    
        // Reads in the SDP data from this file, and builds up the SourceInfo structures
        RelaySDPSourceInfo(StrPtrLen* inSDPData) { Parse(inSDPData); }
        virtual ~RelaySDPSourceInfo();
        
    private:
        
        void    Parse(StrPtrLen* inSDPData);
};
#endif // __SDP_SOURCE_INFO_H__


