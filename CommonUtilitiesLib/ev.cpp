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
    File:       ev.cpp

    Contains:   POSIX select implementation of MacOS X event queue functions.
*/

#if !MACOSXEVENTQUEUE

#define EV_DEBUGGING 0 //Enables a lot of printfs

#if SET_SELECT_SIZE
	#ifndef FD_SETSIZE
		#define FD_SETSIZE SET_SELECT_SIZE
	#endif 
#endif

#define FD_SETSIZE	1024

    #include <sys/time.h>
    #include <sys/types.h>

#ifndef __MACOS__
#ifndef __hpux__
    #include <sys/select.h>
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/errno.h>

#include "ev.h"
#include "OS.h"
#include "OSHeaders.h"
#include "MyAssert.h"
#include "OSThread.h"
#include "OSMutex.h"

static fd_set   sReadSet;
static fd_set   sWriteSet;
static fd_set   sReturnedReadSet;
static fd_set   sReturnedWriteSet;
static void**   sCookieArray = NULL;
static int*     sFDsToCloseArray = NULL;
static int sPipes[2];

static int sCurrentFDPos = 0;
static int sMaxFDPos = 0;
static bool sInReadSet = true;
static int sNumFDsBackFromSelect = 0;
static UInt32 sNumFDsProcessed = 0;
static OSMutex sMaxFDPosMutex;


static bool selecthasdata();
static int constructeventreq(struct eventreq* req, int fd, int event);


void select_startevents()
{
    FD_ZERO(&sReadSet);
    FD_ZERO(&sWriteSet);
    FD_ZERO(&sReturnedReadSet);
    FD_ZERO(&sReturnedWriteSet);

    //qtss_printf("FD_SETSIZE=%d sizeof(fd_set) * 8 ==%ld\n", FD_SETSIZE, sizeof(fd_set) * 8);
    //We need to associate cookies (void*)'s with our file descriptors.
    //We do so by storing cookies in this cookie array. Because an fd_set is
    //a big array of bits, we should have as many entries in the array as
    //there are bits in the fd set  
    sCookieArray = new void*[sizeof(fd_set) * 8];
    ::memset(sCookieArray, 0, sizeof(void *) * sizeof(fd_set) * 8);
    
    //We need to close all fds from the select thread. Once an fd is passed into
    //removeevent, its added to this array so it may be deleted from the select thread
    sFDsToCloseArray = new int[sizeof(fd_set) * 8];
    for (int i = 0; i < (int) (sizeof(fd_set) * 8); i++)
        sFDsToCloseArray[i] = -1;
    
    //We need to wakeup select when the masks have changed. In order to do this,
    //we create a pipe that gets written to from modwatch, and read when select returns
    int theErr = ::pipe((int*)&sPipes);
    Assert(theErr == 0);
    
    //Add the read end of the pipe to the read mask
    FD_SET(sPipes[0], &sReadSet);
    sMaxFDPos = sPipes[0];
}

int select_removeevent(int which)
{

    {
        //Manipulating sMaxFDPos is not pre-emptive safe, so we have to wrap it in a mutex
        //I believe this is the only variable that is not preemptive safe....
        OSMutexLocker locker(&sMaxFDPosMutex);
        
    //Clear this fd out of both sets
        FD_CLR(which, &sWriteSet);
        FD_CLR(which, &sReadSet);
        
        FD_CLR(which, &sReturnedReadSet);
        FD_CLR(which, &sReturnedWriteSet);
    
        sCookieArray[which] = NULL; // Clear out the cookie
        
        if (which == sMaxFDPos)
        {
            //We've just deleted the highest numbered fd in our set,
            //so we need to recompute what the highest one is.
            while (!FD_ISSET(sMaxFDPos, &sReadSet) && !FD_ISSET(sMaxFDPos, &sWriteSet) &&
                (sMaxFDPos > 0))
                {
#if EV_DEBUGGING                  
                     qtss_printf("removeevent: reset MaxFDPos = %d to %d\n", sMaxFDPos , sMaxFDPos -1);
#endif                
                    sMaxFDPos--;
                }
        }

        //We also need to keep the mutex locked during any manipulation of the
        //sFDsToCloseArray, because it's definitely not preemptive safe.
            
        //put this fd into the fd's to close array, so that when select wakes up, it will
        //close the fd
        UInt32 theIndex = 0;
        while ((sFDsToCloseArray[theIndex] != -1) && (theIndex < sizeof(fd_set) * 8))
            theIndex++;
        Assert(sFDsToCloseArray[theIndex] == -1);
        sFDsToCloseArray[theIndex] = which;
#if EV_DEBUGGING
    qtss_printf("removeevent: Disabled %d \n", which);
#endif
    }
    
    //write to the pipe so that select wakes up and registers the new mask
    int theErr = ::write(sPipes[1], "p", 1);
    Assert(theErr == 1);

    return 0;
}

int select_watchevent(struct eventreq *req, int which)
{
    return select_modwatch(req, which);
}

int select_modwatch(struct eventreq *req, int which)
{
    {
        //Manipulating sMaxFDPos is not pre-emptive safe, so we have to wrap it in a mutex
        //I believe this is the only variable that is not preemptive safe....
        OSMutexLocker locker(&sMaxFDPosMutex);

        //Add or remove this fd from the specified sets
        if (which & EV_RE)
        {
    #if EV_DEBUGGING
            qtss_printf("modwatch: Enabling %d in readset\n", req->er_handle);
    #endif
            FD_SET(req->er_handle, &sReadSet);
        }
        else
        {
    #if EV_DEBUGGING
            qtss_printf("modwatch: Disbling %d in readset\n", req->er_handle);
    #endif
            FD_CLR(req->er_handle, &sReadSet);
        }
        if (which & EV_WR)
        {
    #if EV_DEBUGGING
            qtss_printf("modwatch: Enabling %d in writeset\n", req->er_handle);
    #endif
            FD_SET(req->er_handle, &sWriteSet);
        }
        else
        {
    #if EV_DEBUGGING
            qtss_printf("modwatch: Disabling %d in writeset\n", req->er_handle);
    #endif
            FD_CLR(req->er_handle, &sWriteSet);
        }

        if (req->er_handle > sMaxFDPos)
            sMaxFDPos = req->er_handle;

#if EV_DEBUGGING
        qtss_printf("modwatch: MaxFDPos=%d\n", sMaxFDPos);
#endif
        //
        // Also, modifying the cookie is not preemptive safe. This must be
        // done atomically wrt setting the fd in the set. Otherwise, it is
        // possible to have a NULL cookie on a fd.
        Assert(req->er_handle < (int)(sizeof(fd_set) * 8));
		if(req->er_handle >= (int)(sizeof(fd_set) * 8))
		{
			char msg[200] = { 0 };
			sprintf(msg,"req->er_handle=%d, sizeof(fd_set)=%d", req->er_handle, (int)(sizeof(fd_set) * 8));
			AssertE(false, msg);
		}
        Assert(req->er_data != NULL);
        sCookieArray[req->er_handle] = req->er_data;
    }
    
    //write to the pipe so that select wakes up and registers the new mask
    int theErr = ::write(sPipes[1], "p", 1);
    Assert(theErr == 1);

    return 0;
}

int constructeventreq(struct eventreq* req, int fd, int event)
{
    Assert(fd < (int)(sizeof(fd_set) * 8));
    if (fd >=(int)(sizeof(fd_set) * 8) )
    {
        #if EV_DEBUGGING
                qtss_printf("constructeventreq: invalid fd=%d\n", fd);
        #endif
        return 0;
    }        
    req->er_handle = fd;
    req->er_eventbits = event;
    req->er_data = sCookieArray[fd];
    sCurrentFDPos++;
    sNumFDsProcessed++;
    
    //don't want events on this fd until modwatch is called.
    FD_CLR(fd, &sWriteSet);
    FD_CLR(fd, &sReadSet);
    
    return 0;
}

int select_waitevent(struct eventreq *req, void* /*onlyForMacOSX*/)
{
    //Check to see if we still have some select descriptors to process
    int theFDsProcessed = (int)sNumFDsProcessed;
    bool isSet = false;
    
    if (theFDsProcessed < sNumFDsBackFromSelect)
    {
        if (sInReadSet)
        {
            OSMutexLocker locker(&sMaxFDPosMutex);
#if EV_DEBUGGING
            qtss_printf("waitevent: Looping through readset starting at %d\n", sCurrentFDPos);
#endif
            while((!(isSet = FD_ISSET(sCurrentFDPos, &sReturnedReadSet))) && (sCurrentFDPos < sMaxFDPos)) 
                sCurrentFDPos++;        

            if (isSet)
            {   
#if EV_DEBUGGING
                qtss_printf("waitevent: Found an fd: %d in readset max=%d\n", sCurrentFDPos, sMaxFDPos);
#endif
                FD_CLR(sCurrentFDPos, &sReturnedReadSet);
                return constructeventreq(req, sCurrentFDPos, EV_RE);
            }
            else
            {
#if EV_DEBUGGING
                qtss_printf("waitevent: Stopping traverse of readset at %d\n", sCurrentFDPos);
#endif
                sInReadSet = false;
                sCurrentFDPos = 0;
            }
        }
        if (!sInReadSet)
        {
            OSMutexLocker locker(&sMaxFDPosMutex);
#if EV_DEBUGGING
            qtss_printf("waitevent: Looping through writeset starting at %d\n", sCurrentFDPos);
#endif
            while((!(isSet = FD_ISSET(sCurrentFDPos, &sReturnedWriteSet))) && (sCurrentFDPos < sMaxFDPos))
                sCurrentFDPos++;

            if (isSet)
            {
#if EV_DEBUGGING
                qtss_printf("waitevent: Found an fd: %d in writeset\n", sCurrentFDPos);
#endif
                FD_CLR(sCurrentFDPos, &sReturnedWriteSet);
                return constructeventreq(req, sCurrentFDPos, EV_WR);
            }
            else
            {
                // This can happen if another thread calls select_removeevent at just the right
                // time, setting sMaxFDPos lower than it was when select() was last called.
                // Becase sMaxFDPos is used as the place to stop iterating over the read & write
                // masks, setting it lower can cause file descriptors in the mask to get skipped.
                // If they are skipped, that's ok, because those file descriptors were removed
                // by select_removeevent anyway. We need to make sure to finish iterating over
                // the masks and call select again, which is why we set sNumFDsProcessed
                // artificially here.
                sNumFDsProcessed = sNumFDsBackFromSelect;
                Assert(sNumFDsBackFromSelect > 0);
            }
        }
    }
    
    if (sNumFDsProcessed > 0)
    {
        OSMutexLocker locker(&sMaxFDPosMutex);
#if DEBUG
        //
        // In a very bizarre circumstance (sMaxFDPos goes down & then back up again, these
        // asserts could hit.
        //
        //for (int x = 0; x < sMaxFDPos; x++)
        //  Assert(!FD_ISSET(x, &sReturnedReadSet));
        //for (int y = 0; y < sMaxFDPos; y++)
        //  Assert(!FD_ISSET(y, &sReturnedWriteSet));
#endif  
#if EV_DEBUGGING
        qtss_printf("waitevent: Finished with all fds in set. Stopped traverse of writeset at %d maxFD = %d\n", sCurrentFDPos,sMaxFDPos);
#endif
        //We've just cycled through one select result. Re-init all the counting states
        sNumFDsProcessed = 0;
        sNumFDsBackFromSelect = 0;
        sCurrentFDPos = 0;
        sInReadSet = true;
    }
    
    
    
    while(!selecthasdata())
    {
        {
            OSMutexLocker locker(&sMaxFDPosMutex);
            //Prepare to call select. Preserve the read and write sets by copying their contents
            //into the corresponding "returned" versions, and then pass those into select
            ::memcpy(&sReturnedReadSet, &sReadSet, sizeof(fd_set));
            ::memcpy(&sReturnedWriteSet, &sWriteSet, sizeof(fd_set));
        }

        SInt64  yieldDur = 0;
        SInt64  yieldStart;
        
        //Periodically time out the select call just in case we
        //are deaf for some reason
        // on platforw's where our threading is non-preemptive, just poll select

        struct timeval  tv;
        tv.tv_usec = 0;

    #if THREADING_IS_COOPERATIVE
        tv.tv_sec = 0;
        
        if ( yieldDur > 4 )
            tv.tv_usec = 0;
        else
            tv.tv_usec = 5000;
    #else
        tv.tv_sec = 15;
    #endif

#if EV_DEBUGGING
        qtss_printf("waitevent: about to call select\n");
#endif

        yieldStart = OS::Milliseconds();
        OSThread::ThreadYield();
        
        yieldDur = OS::Milliseconds() - yieldStart;
#if EV_DEBUGGING
        static SInt64   numZeroYields;
        
        if ( yieldDur > 1 )
        {
            qtss_printf( "select_waitevent time in OSThread::Yield() %i, numZeroYields %i\n", (SInt32)yieldDur, (SInt32)numZeroYields );
            numZeroYields = 0;
        }
        else
            numZeroYields++;

#endif

        sNumFDsBackFromSelect = ::select(sMaxFDPos+1, &sReturnedReadSet, &sReturnedWriteSet, NULL, &tv);

#if EV_DEBUGGING
        qtss_printf("waitevent: back from select. Result = %d\n", sNumFDsBackFromSelect);
#endif
    }
    

    if (sNumFDsBackFromSelect >= 0)
        return EINTR;   //either we've timed out or gotten some events. Either way, force caller
                        //to call waitevent again.
    return sNumFDsBackFromSelect;
}

bool selecthasdata()
{
    if (sNumFDsBackFromSelect < 0)
    {
        int err=OSThread::GetErrno();
        
#if EV_DEBUGGING
        if (err == ENOENT) 
        {
             qtss_printf("selectHasdata: found error ENOENT==2 \n");
        }
#endif

        if ( 
#if __solaris__
            err == ENOENT || // this happens on Solaris when an HTTP fd is closed
#endif      
            err == EBADF || //this might happen if a fd is closed right before calling select
            err == EINTR 
           ) // this might happen if select gets interrupted
             return false;
        return true;//if there is an error from select, we want to make sure and return to the caller
    }
        
    if (sNumFDsBackFromSelect == 0)
        return false;//if select returns 0, we've simply timed out, so recall select
    
    if (FD_ISSET(sPipes[0], &sReturnedReadSet))
    {
#if EV_DEBUGGING
        qtss_printf("selecthasdata: Got some data on the pipe fd\n");
#endif
        //we've gotten data on the pipe file descriptor. Clear the data.
        // increasing the select buffer fixes a hanging problem when the Darwin server is under heavy load
        // CISCO contribution
        char theBuffer[4096]; 
        (void)::read(sPipes[0], &theBuffer[0], 4096);

        FD_CLR(sPipes[0], &sReturnedReadSet);
        sNumFDsBackFromSelect--;
        
        {
            //Check the fds to close array, and if there are any in it, close those descriptors
            OSMutexLocker locker(&sMaxFDPosMutex);
            for (UInt32 theIndex = 0; ((sFDsToCloseArray[theIndex] != -1) && (theIndex < sizeof(fd_set) * 8)); theIndex++)
            {
                (void)::close(sFDsToCloseArray[theIndex]);
                sFDsToCloseArray[theIndex] = -1;
            }
        }
    }
    Assert(!FD_ISSET(sPipes[0], &sReturnedWriteSet));
    
    if (sNumFDsBackFromSelect == 0)
        return false;//if the pipe file descriptor is the ONLY data we've gotten, recall select
    else
        return true;//we've gotten a real event, return that to the caller
}

#endif //!MACOSXEVENTQUEUE

