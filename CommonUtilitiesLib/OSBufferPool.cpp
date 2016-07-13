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
	 File:       OSBufferPool.cpp

	 Contains:   Fast access to fixed size buffers.

	 Written By: Denis Serenyi

 */

#include "OSBufferPool.h"
#include "OSMemory.h"
#ifdef _WIN32
#include <new>
#endif

void*   OSBufferPool::Get()
{
	OSMutexLocker locker(&fMutex);
	if (fQueue.GetLength() == 0)
	{
		fTotNumBuffers++;
		char* theNewBuf = NEW char[fBufSize + sizeof(OSQueueElem)];

		//
		// We need to construct a Queue Element, but we don't actually need
		// to use it in this function, so to avoid a compiler warning just
		// don't assign the result to anything.
		(void)new (theNewBuf) OSQueueElem(theNewBuf + sizeof(OSQueueElem));

		return theNewBuf + sizeof(OSQueueElem);
	}
	return fQueue.DeQueue()->GetEnclosingObject();
}

void OSBufferPool::Put(void* inBuffer)
{
	OSMutexLocker locker(&fMutex);
	fQueue.EnQueue((OSQueueElem*)((char*)inBuffer - sizeof(OSQueueElem)));
}
