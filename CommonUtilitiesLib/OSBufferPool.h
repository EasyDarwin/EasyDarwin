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
    File:       OSBufferPool.h

    Contains:   Fast access to fixed size buffers.
    
    Written By: Denis Serenyi
    
*/

#ifndef __OS_BUFFER_POOL_H__
#define __OS_BUFFER_POOL_H__

#include "OSQueue.h"
#include "OSMutex.h"

class OSBufferPool
{
    public:
    
        OSBufferPool(UInt32 inBufferSize) : fBufSize(inBufferSize), fTotNumBuffers(0) {}
        
        //
        // This object currently *does not* clean up for itself when
        // you destruct it!
        ~OSBufferPool() {}
        
        //
        // ACCESSORS
        UInt32  GetTotalNumBuffers() { return fTotNumBuffers; }
        UInt32  GetNumAvailableBuffers() { return fQueue.GetLength(); }
        
        //
        // All these functions are thread-safe
        
        //
        // Gets a buffer out of the pool. This buffer must be replaced
        // by calling Put when you are done with it.
        void*   Get();
        
        //
        // Returns a buffer retreived by Get back to the pool.
        void    Put(void* inBuffer);
    
    private:
    
        OSMutex fMutex;
        OSQueue fQueue;
        UInt32  fBufSize;
        UInt32  fTotNumBuffers;
};

#endif //__OS_BUFFER_POOL_H__
