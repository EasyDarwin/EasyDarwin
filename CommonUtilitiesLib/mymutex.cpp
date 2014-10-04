/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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
 */
/*
    File:       mymutex.c

    Contains:   xxx put contents here xxx

    Written by: Greg Vaughan

    Writers:

        (GV)    Greg Vaughan
        (CNR)   Christopher Ryan

    Change History (most recent first):

         <7>      9/5/00    CNR     Use mach_port_destroy instead of mach_port_deallocate in
                                    MMDisposeMutex.
         <6>     7/24/00    GV      changed Carbon to CarbonCore for header include
         <5>     1/11/00    CNR     Fix for IncrementAtomic/DecrementAtomic return value.
         <4>      1/6/00    GBV     got rid of libatomic
         <3>     12/8/99    CNR     Use OSAssert.h
         <2>    10/27/99    GBV     update for beaker

    To Do:
*/

#include "mymutex.h"
#include <stdlib.h>
#include "SafeStdLib.h"
#if __MacOSX__

#ifndef __CORESERVICES__
#include <CoreServices/CoreServices.h>
#endif

#endif

#include "MyAssert.h"

struct MyMutex
{
    SInt32              fCount;
    pthread_t           fHolder;
    UInt32       		fMutexLock;
    mach_port_t         fWaitPort;
    SInt32              fNumWaiting;
};

typedef struct MyMutex MyMutex;

MyMutex* MMAllocateMutex();
void MMDisposeMutex(MyMutex* theMutex);
void MMGrab(MyMutex* theMutex);
int MMTryGrab(MyMutex* theMutex);
void MMRelease(MyMutex* theMutex);
pthread_t MMGetFirstWaitingThread(MyMutex* theMutex, int* listWasEmpty);
int MMAlreadyHaveControl(MyMutex* theMutex, pthread_t thread);
int MMTryAndGetControl(MyMutex* theMutex, pthread_t thread);
void MMReleaseControl(MyMutex* theMutex);
void MMStripOffWaitingThread(MyMutex* theMutex);
void MMAddToWaitList(MyMutex* theMutex, pthread_t thread);
void MMBlockThread(MyMutex* theMutex);
void MMUnblockThread(MyMutex* theMutex);

mymutex_t mymutex_alloc()
{
    return (mymutex_t)MMAllocateMutex();
}

void mymutex_free(mymutex_t theMutex_t)
{
    MMDisposeMutex((MyMutex*)theMutex_t);
}

void mymutex_lock(mymutex_t theMutex_t)
{
    MMGrab((MyMutex*)theMutex_t);
}

int mymutex_try_lock(mymutex_t theMutex_t)
{
    return MMTryGrab((MyMutex*)theMutex_t);
}

void mymutex_unlock(mymutex_t theMutex_t)
{
    MMRelease((MyMutex*)theMutex_t);
}

SInt32 sNumMutexes = 0;

MyMutex* MMAllocateMutex()
{
    kern_return_t ret;
    MyMutex* newMutex = (MyMutex*)malloc(sizeof(MyMutex));
    if (newMutex == NULL)
    {
        Assert(newMutex != NULL);
        return NULL;
    }
        
    newMutex->fCount = 0;
    newMutex->fHolder = 0;
    newMutex->fNumWaiting = 0;
    newMutex->fMutexLock = 0;
    ret = mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &newMutex->fWaitPort);
    if (ret != KERN_SUCCESS)
    {
        AssertV(ret == 0, ret);
        free(newMutex);
        return NULL;
    }
    ret = mach_port_insert_right(mach_task_self(), newMutex->fWaitPort, newMutex->fWaitPort, MACH_MSG_TYPE_MAKE_SEND);
    if (ret != KERN_SUCCESS)
    {
        AssertV(ret == 0, ret);
        free(newMutex);
        return NULL;
    }
    AssertV(ret == 0, ret);
    IncrementAtomic(&sNumMutexes);
    return newMutex;
}

void MMDisposeMutex(MyMutex* theMutex)
{
    int err = noErr;
    err = mach_port_destroy(mach_task_self(), theMutex->fWaitPort);
    DecrementAtomic(&sNumMutexes);
    AssertV(err == noErr, err);
    free(theMutex);
}

void MMGrab(MyMutex* theMutex)
{
    pthread_t thread = pthread_self();
    
    if (theMutex->fHolder != thread) 
    {
        int waiting = IncrementAtomic(&theMutex->fNumWaiting) + 1;
         
        if ((waiting > 1) || !CompareAndSwap(0, 1, &theMutex->fMutexLock))
        {
            do
            {
                // suspend ourselves until something happens
                MMBlockThread(theMutex);
            } while (!CompareAndSwap(0, 1, &theMutex->fMutexLock));
        }

        DecrementAtomic(&theMutex->fNumWaiting);

        // we just got control, so reset fCount
        theMutex->fCount = 0; // gets incremented below...
        theMutex->fHolder = thread;
    }

    // we have control now, so increment the count
    ++theMutex->fCount;
}

int MMTryGrab(MyMutex* theMutex)
{
    pthread_t thread = pthread_self();
    int haveControl;
    
    haveControl = (theMutex->fHolder == thread);
    if (!haveControl)
        haveControl = CompareAndSwap(0, 1, &theMutex->fMutexLock);

    if (haveControl)
    {
        theMutex->fHolder = thread;
        ++theMutex->fCount;
    }
        
    return haveControl;
}

void MMRelease(MyMutex* theMutex)
{
    pthread_t thread = pthread_self();
    if (theMutex->fHolder != thread)
        return;
    
    if (!--theMutex->fCount) 
    {
        theMutex->fHolder = NULL;
        theMutex->fMutexLock = 0;   // let someone else deal with it
        if (theMutex->fNumWaiting > 0)
            MMUnblockThread(theMutex);
    }
}

typedef struct {
    mach_msg_header_t header;
    mach_msg_trailer_t trailer;
} mHeader;

void MMBlockThread(MyMutex* theMutex)
{
    kern_return_t ret;
    mHeader msg;

    memset(&msg, 0, sizeof(msg));
    ret = mach_msg(&msg.header,MACH_RCV_MSG,0,sizeof(msg),
                                theMutex->fWaitPort,MACH_MSG_TIMEOUT_NONE,MACH_PORT_NULL);
    AssertV(ret == 0, ret);
}

void MMUnblockThread(MyMutex* theMutex)
{
    kern_return_t ret;
    mHeader msg;

    memset(&msg, 0, sizeof(msg));
    msg.header.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_COPY_SEND, 0);
    msg.header.msgh_size = sizeof msg - sizeof msg.trailer;
    msg.header.msgh_local_port = MACH_PORT_NULL; 
    msg.header.msgh_remote_port = theMutex->fWaitPort;
    msg.header.msgh_id = 0;
    ret = mach_msg(&msg.header,MACH_SEND_MSG,msg.header.msgh_size,0,MACH_PORT_NULL,MACH_MSG_TIMEOUT_NONE,
                            MACH_PORT_NULL);
    AssertV(ret == 0, ret);
}

