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
    File:       SequenceNumberMap.h

    Contains:   Data structure for keeping track of duplicate sequence numbers.
                Useful for removing duplicate packets from an RTP stream.
                    

    

*/

#ifndef _SEQUENCE_NUMBER_MAP_H_
#define _SEQUENCE_NUMBER_MAP_H_

#include "OSHeaders.h"

#define SEQUENCENUMBERMAPTESTING 1

class SequenceNumberMap
{
    public:
    
        enum
        {
            kDefaultSlidingWindowSize = 256
        };
        
        SequenceNumberMap(UInt32 inSlidingWindowSize = kDefaultSlidingWindowSize);
        ~SequenceNumberMap() { delete [] fSlidingWindow; }
        
        // Returns whether this sequence number was already added or not.
        Bool16  AddSequenceNumber(UInt16 inSeqNumber);
        
#if SEQUENCENUMBERMAPTESTING
        static void Test();
#endif
        
    private:
        
        Bool16*         fSlidingWindow;

        const SInt32    fWindowSize;
        const SInt32    fNegativeWindowSize;

        UInt16          fHighestSeqIndex;
        UInt16          fHighestSeqNumber;
};


#endif
