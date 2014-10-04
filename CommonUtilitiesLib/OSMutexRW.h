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
    File:       OSMutexRW.h

    Contains:   

    

*/

#ifndef _OSMUTEXRW_H_
#define _OSMUTEXRW_H_

#include <stdlib.h>
#include "SafeStdLib.h"
#include "OSHeaders.h"
#include "OSThread.h"
#include "MyAssert.h"
#include "OSMutex.h"
#include "OSQueue.h"

#define DEBUGMUTEXRW 0

class OSMutexRW
{
    public:
        
        OSMutexRW(): fState(0), fWriteWaiters(0),fReadWaiters(0),fActiveReaders(0) {} ;

        void LockRead();
        void LockWrite();
        void Unlock();
        
        // Returns 0 on success, EBUSY on failure
        int TryLockWrite();
        int TryLockRead();
    
    private:
        enum {eMaxWait = 0x0FFFFFFF, eMultiThreadCondition = true, };
        enum {eActiveWriterState = -1, eNoWriterState = 0 };

        OSMutex             fInternalLock;   // the internal lock         
        OSCond              fReadersCond;    // the waiting readers             
        OSCond              fWritersCond;    // the waiting writers             
        int                 fState;          // -1:writer,0:free,>0:readers 
        int                 fWriteWaiters;   // number of waiting writers   
        int                 fReadWaiters;    // number of waiting readers
        int                 fActiveReaders;  // number of active readers = fState >= 0;

        inline void AdjustState(int i) {  fState += i; };
        inline void AdjustWriteWaiters(int i) { fWriteWaiters += i; };
        inline void AdjustReadWaiters(int i) {  fReadWaiters += i; };
        inline void SetState(int i) { fState = i; };
        inline void SetWriteWaiters(int i) {  fWriteWaiters = i; };
        inline void SetReadWaiters(int i) { fReadWaiters = i; };
        
        inline void AddWriteWaiter() { AdjustWriteWaiters(1); };
        inline void RemoveWriteWaiter() {  AdjustWriteWaiters(-1); };
        
        inline void AddReadWaiter() { AdjustReadWaiters(1); };
        inline void RemoveReadWaiter() {  AdjustReadWaiters(-1); };
        
        inline void AddActiveReader() { AdjustState(1); };
        inline void RemoveActiveReader() {  AdjustState(-1); };
        
        
        inline Bool16 WaitingWriters()  {return (Bool16) (fWriteWaiters > 0) ; }
        inline Bool16 WaitingReaders()  {return (Bool16) (fReadWaiters > 0) ;}
        inline Bool16 Active()          {return (Bool16) (fState != 0) ;}
        inline Bool16 ActiveReaders()   {return (Bool16) (fState > 0) ;}
        inline Bool16 ActiveWriter()    {return (Bool16) (fState < 0) ;} // only one
    
    #if DEBUGMUTEXRW
        static int fCount, fMaxCount;
        static OSMutex sCountMutex;
        void CountConflict(int i); 
    #endif

};

class   OSMutexReadWriteLocker
{
    public:
        OSMutexReadWriteLocker(OSMutexRW *inMutexPtr): fRWMutexPtr(inMutexPtr) {};
        ~OSMutexReadWriteLocker() { if (fRWMutexPtr != NULL) fRWMutexPtr->Unlock(); }


        void UnLock() { if (fRWMutexPtr != NULL) fRWMutexPtr->Unlock(); }
        void SetMutex(OSMutexRW *mutexPtr) {fRWMutexPtr = mutexPtr;}
        OSMutexRW*  fRWMutexPtr;
};
    
class   OSMutexReadLocker: public OSMutexReadWriteLocker
{
    public:

        OSMutexReadLocker(OSMutexRW *inMutexPtr) : OSMutexReadWriteLocker(inMutexPtr) { if (OSMutexReadWriteLocker::fRWMutexPtr != NULL) OSMutexReadWriteLocker::fRWMutexPtr->LockRead(); }
        void Lock()         { if (OSMutexReadWriteLocker::fRWMutexPtr != NULL) OSMutexReadWriteLocker::fRWMutexPtr->LockRead(); }       
};

class   OSMutexWriteLocker: public OSMutexReadWriteLocker
{
    public:

        OSMutexWriteLocker(OSMutexRW *inMutexPtr) : OSMutexReadWriteLocker(inMutexPtr) { if (OSMutexReadWriteLocker::fRWMutexPtr != NULL) OSMutexReadWriteLocker::fRWMutexPtr->LockWrite(); }       
        void Lock()         { if (OSMutexReadWriteLocker::fRWMutexPtr != NULL) OSMutexReadWriteLocker::fRWMutexPtr->LockWrite(); }

};



#endif //_OSMUTEX_H_
