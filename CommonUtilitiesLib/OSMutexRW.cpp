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
	 File:       OSMutex.cpp

	 Contains:



 */

#include "OSMutexRW.h"
#include "OSMutex.h"
#include "OSCond.h"

#include <stdlib.h>

#if DEBUGMUTEXRW
int OSMutexRW::fCount = 0;
int OSMutexRW::fMaxCount = 0;
#endif

#if DEBUGMUTEXRW
void OSMutexRW::CountConflict(int i)
{
	fCount += i;
	if (i == -1) qtss_printf("Num Conflicts: %d\n", fMaxCount);
	if (fCount > fMaxCount)
		fMaxCount = fCount;

}
#endif

void OSMutexRW::LockRead()
{
	OSMutexLocker locker(&fInternalLock);
#if DEBUGMUTEXRW
	if (fState != 0)
	{
		qtss_printf("LockRead(conflict) fState = %d active readers = %d, waiting writers = %d, waiting readers=%d\n", fState, fActiveReaders, fWriteWaiters, fReadWaiters);
		CountConflict(1);
	}

#endif

	addReadWaiter();
	while (activeWriter() // active writer so wait
		|| waitingWriters() // reader must wait for write waiters
		)
	{
		fReadersCond.Wait(&fInternalLock, OSMutexRW::eMaxWait);
	}

	removeReadWaiter();
	addActiveReader(); // add 1 to active readers
	fActiveReaders = fState;

#if DEBUGMUTEXRW
	//  qtss_printf("LockRead(conflict) fState = %d active readers = %d, waiting writers = %d, waiting readers=%d\n",fState,  fActiveReaders, fWriteWaiters, fReadWaiters);

#endif
}

void OSMutexRW::LockWrite()
{
	OSMutexLocker locker(&fInternalLock);
	addWriteWaiter();       //  1 writer queued            
#if DEBUGMUTEXRW

	if (Active())
	{
		qtss_printf("LockWrite(conflict) state = %d active readers = %d, waiting writers = %d, waiting readers=%d\n", fState, fActiveReaders, fWriteWaiters, fReadWaiters);
		CountConflict(1);
	}

	qtss_printf("LockWrite 'waiting' fState = %d locked active readers = %d, waiting writers = %d, waiting readers=%d\n", fState, fActiveReaders, fReadWaiters, fWriteWaiters);
#endif

	while (activeReaders())  // active readers
	{
		fWritersCond.Wait(&fInternalLock, OSMutexRW::eMaxWait);
	}

	removeWriteWaiter(); // remove from waiting writers
	setState(OSMutexRW::eActiveWriterState);    // this is the active writer    
	fActiveReaders = fState;
#if DEBUGMUTEXRW
	//  qtss_printf("LockWrite 'locked' fState = %d locked active readers = %d, waiting writers = %d, waiting readers=%d\n",fState, fActiveReaders, fReadWaiters, fWriteWaiters);
#endif

}

void OSMutexRW::Unlock()
{
	OSMutexLocker locker(&fInternalLock);
#if DEBUGMUTEXRW
	//  qtss_printf("Unlock active readers = %d, waiting writers = %d, waiting readers=%d\n", fActiveReaders, fReadWaiters, fWriteWaiters);

#endif

	if (activeWriter())
	{
		setState(OSMutexRW::eNoWriterState); // this was the active writer 
		if (waitingWriters()) // there are waiting writers
		{
			fWritersCond.Signal();
		}
		else
		{
			fReadersCond.Broadcast();
		}
#if DEBUGMUTEXRW
		qtss_printf("Unlock(writer) active readers = %d, waiting writers = %d, waiting readers=%d\n", fActiveReaders, fReadWaiters, fWriteWaiters);
#endif
	}
	else
	{
		removeActiveReader(); // this was a reader
		if (!activeReaders()) // no active readers
		{
			setState(OSMutexRW::eNoWriterState); // this was the active writer now no actives threads
			fWritersCond.Signal();
		}
	}
	fActiveReaders = fState;

}

// Returns true on successful grab of the lock, false on failure
int OSMutexRW::TryLockWrite()
{
	int    status = EBUSY;
	OSMutexLocker locker(&fInternalLock);

	if (!active() && !waitingWriters()) // no writers, no readers, no waiting writers
	{
		this->LockWrite();
		status = 0;
	}

	return status;
}

int OSMutexRW::TryLockRead()
{
	int    status = EBUSY;
	OSMutexLocker locker(&fInternalLock);

	if (!activeWriter() && !waitingWriters()) // no current writers but other readers ok
	{
		this->LockRead();
		status = 0;
	}

	return status;
}



