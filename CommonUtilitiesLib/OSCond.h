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
	 File:       OSCond.h

	 Contains:   A simple condition variable abstraction




 */

#ifndef _OSCOND_H_
#define _OSCOND_H_

#ifndef __Win32__
#if __PTHREADS_MUTEXES__
#include <pthread.h>
#else
#include "mycondition.h"
#endif
#endif

#include "OSMutex.h"
#include "MyAssert.h"

class OSCond
{
public:

	OSCond();
	~OSCond();

	inline void     Signal();
	inline void     Wait(OSMutex* inMutex, SInt32 inTimeoutInMilSecs = 0);
	inline void     Broadcast();

private:

#ifdef __Win32__
	HANDLE              fCondition;
	UInt32              fWaitCount;
#elif __PTHREADS_MUTEXES__
	pthread_cond_t      fCondition;
	void                TimedWait(OSMutex* inMutex, SInt32 inTimeoutInMilSecs);
#else
	mycondition_t       fCondition;
#endif
};

inline void OSCond::Wait(OSMutex* inMutex, SInt32 inTimeoutInMilSecs)
{
#ifdef __Win32__
	DWORD theTimeout = INFINITE;
	if (inTimeoutInMilSecs > 0)
		theTimeout = inTimeoutInMilSecs;
	inMutex->Unlock();
	fWaitCount++;
	DWORD theErr = ::WaitForSingleObject(fCondition, theTimeout);
	fWaitCount--;
	Assert((theErr == WAIT_OBJECT_0) || (theErr == WAIT_TIMEOUT));
	inMutex->Lock();
#elif __PTHREADS_MUTEXES__
	this->TimedWait(inMutex, inTimeoutInMilSecs);
#else
	Assert(fCondition != NULL);
	mycondition_wait(fCondition, inMutex->fMutex, inTimeoutInMilSecs);
#endif
}

inline void OSCond::Signal()
{
#ifdef __Win32__
	BOOL theErr = ::SetEvent(fCondition);
	Assert(theErr == TRUE);
#elif __PTHREADS_MUTEXES__
	pthread_cond_signal(&fCondition);
#else
	Assert(fCondition != NULL);
	mycondition_signal(fCondition);
#endif
}

inline void OSCond::Broadcast()
{
#ifdef __Win32__
	//
	// There doesn't seem like any more elegant way to
	// implement Broadcast using events in Win32.
	// This will work, it may generate spurious wakeups,
	// but condition variables are allowed to generate
	// spurious wakeups
	UInt32 waitCount = fWaitCount;
	for (UInt32 x = 0; x < waitCount; x++)
	{
		BOOL theErr = ::SetEvent(fCondition);
		Assert(theErr == TRUE);
	}
#elif __PTHREADS_MUTEXES__
	pthread_cond_broadcast(&fCondition);
#else
	Assert(fCondition != NULL);
	mycondition_broadcast(fCondition);
#endif
}

#endif //_OSCOND_H_
