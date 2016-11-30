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
	 File:       OSMutex.h

	 Contains:   Platform - independent mutex header. The implementation of this object
				 is platform - specific. Each platform must define an independent
				 OSMutex.h & OSMutex.cpp file.

				 This file is for Mac OS X Server only



 */

#ifndef _OSMUTEX_H_
#define _OSMUTEX_H_

#include <stdlib.h>
#include "SafeStdLib.h"
#ifndef __Win32__
#include <sys/errno.h>
#if __PTHREADS_MUTEXES__
#if __MacOSX__
#ifndef _POSIX_PTHREAD_H
#include <pthread.h>
#endif
#else
#include <pthread.h>
#endif
#include <unistd.h>

#else
#include "mymutex.h"
#endif
#endif

#include "OSHeaders.h"
#include "OSThread.h"
#include "MyAssert.h"

class OSCond;

class OSMutex
{
public:

	OSMutex();
	~OSMutex();

	inline void Lock();
	inline void Unlock();

	// Returns true on successful grab of the lock, false on failure
	inline Bool16 TryLock();

private:

#ifdef __Win32__
	CRITICAL_SECTION fMutex;

	DWORD       fHolder;
	UInt32      fHolderCount;

#elif !__PTHREADS_MUTEXES__
	mymutex_t fMutex;
#else
	pthread_mutex_t fMutex;
	// These two platforms don't implement pthreads recursive mutexes, so
	// we have to do it manually
	pthread_t   fHolder;
	UInt32      fHolderCount;
#endif

#if __PTHREADS_MUTEXES__ || __Win32__       
	void        RecursiveLock();
	void        RecursiveUnlock();
	Bool16      RecursiveTryLock();
#endif
	friend class OSCond;
};

class OSMutexLocker
{
public:

	OSMutexLocker(OSMutex* inMutexP) : fMutex(inMutexP) { if (fMutex != NULL) fMutex->Lock(); }
	~OSMutexLocker() { if (fMutex != NULL) fMutex->Unlock(); }

	void Lock() { if (fMutex != NULL) fMutex->Lock(); }
	void Unlock() { if (fMutex != NULL) fMutex->Unlock(); }

private:
	OSMutex*    fMutex;

};

void OSMutex::Lock()
{
#if __PTHREADS_MUTEXES__ || __Win32__
	this->RecursiveLock();
#else
	mymutex_lock(fMutex);
#endif //!__PTHREADS__
}

void OSMutex::Unlock()
{
#if __PTHREADS_MUTEXES__ || __Win32__
	this->RecursiveUnlock();
#else
	mymutex_unlock(fMutex);
#endif //!__PTHREADS__
}

Bool16 OSMutex::TryLock()
{
#if __PTHREADS_MUTEXES__ || __Win32__
	return this->RecursiveTryLock();
#else
	return (Bool16)mymutex_try_lock(fMutex);
#endif //!__PTHREADS__
}

#endif //_OSMUTEX_H_
