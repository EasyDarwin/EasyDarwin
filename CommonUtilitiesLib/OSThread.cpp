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
    File:       OSThread.cpp

    Contains:   Thread abstraction implementation

    
    
*/

#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include <string.h>
#include <errno.h>

#ifdef __MacOSX__
#include <mach/mach_types.h>
#include <mach/mach_time.h>
#endif

#ifndef __Win32__
    #if __PTHREADS__
        #include <pthread.h>
        #if USE_THR_YIELD
            #include <thread.h>
        #endif
    #else
        #include <mach/mach.h>
        #include <mach/cthreads.h>
    #endif
    #include <unistd.h>
    #include <grp.h>
    #include <pwd.h>
#endif

#include "OSThread.h"
#include "MyAssert.h"

#ifdef __sgi__ 
#include <time.h>
#endif


//
// OSThread.cp
//
void*   OSThread::sMainThreadData = NULL;

#ifdef __Win32__
DWORD   OSThread::sThreadStorageIndex = 0;
#elif __PTHREADS__
pthread_key_t OSThread::gMainKey = 0;
#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
pthread_attr_t OSThread::sThreadAttr;
#endif
#endif

char  OSThread::sUser[128]= "";
char  OSThread::sGroup[128]= "";

#if __linux__ ||  __MacOSX__
Bool16  OSThread::sWrapSleep = true;
#endif

void OSThread::Initialize()
{


#ifdef __Win32__
    sThreadStorageIndex = ::TlsAlloc();
    Assert(sThreadStorageIndex >= 0);
#elif __PTHREADS__
    pthread_key_create(&OSThread::gMainKey, NULL);
#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING

    //
    // Added for Solaris...
    
    pthread_attr_init(&sThreadAttr);
    /* Indicate we want system scheduling contention scope. This
       thread is permanently "bound" to an LWP */
    pthread_attr_setscope(&sThreadAttr, PTHREAD_SCOPE_SYSTEM);
#endif

#endif
}

OSThread::OSThread()
:   fStopRequested(false),
    fJoined(false),
    fThreadData(NULL)
{
}

OSThread::~OSThread()
{
    this->StopAndWaitForThread();
}



void OSThread::Start()
{
#ifdef __Win32__
    unsigned int theId = 0; // We don't care about the identifier
    fThreadID = (HANDLE)_beginthreadex( NULL,   // Inherit security
                                        0,      // Inherit stack size
                                        _Entry, // Entry function
                                        (void*)this,    // Entry arg
                                        0,      // Begin executing immediately
                                        &theId );
    Assert(fThreadID != NULL);
#elif __PTHREADS__
    pthread_attr_t* theAttrP;
#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
    //theAttrP = &sThreadAttr;
    theAttrP = 0;
#else
    theAttrP = NULL;
#endif
    int err = pthread_create((pthread_t*)&fThreadID, theAttrP, _Entry, (void*)this);
    Assert(err == 0);
#else
    fThreadID = (UInt32)cthread_fork((cthread_fn_t)_Entry, (any_t)this);
#endif
}

void OSThread::StopAndWaitForThread()
{
    fStopRequested = true;
    if (!fJoined)
        Join();
}

void OSThread::Join()
{
    // What we're trying to do is allow the thread we want to delete to complete
    // running. So we wait for it to stop.
    Assert(!fJoined);
    fJoined = true;
#ifdef __Win32__
    DWORD theErr = ::WaitForSingleObject(fThreadID, INFINITE);
    Assert(theErr == WAIT_OBJECT_0);
#elif __PTHREADS__
    void *retVal;
    pthread_join((pthread_t)fThreadID, &retVal);
#else
    cthread_join((cthread_t)fThreadID);
#endif
}

void OSThread::ThreadYield()
{
    // on platforms who's threading is not pre-emptive yield 
    // to another thread
#if THREADING_IS_COOPERATIVE
    #if __PTHREADS__
        #if USE_THR_YIELD
            thr_yield();
        #else
            sched_yield();
        #endif
    #endif
#endif
}

#include "OS.h"
void OSThread::Sleep(UInt32 inMsec)
{

#ifdef __Win32__
    ::Sleep(inMsec);
#elif __linux__ ||  __MacOSX__

    if (inMsec == 0)
        return;
        
    SInt64 startTime = OS::Milliseconds();
    SInt64 timeLeft = inMsec;
    SInt64 timeSlept = 0;
    UInt64 utimeLeft = 0;    

    do { // loop in case we leave the sleep early
        //qtss_printf("OSThread::Sleep time slept= %qd request sleep=%qd\n",timeSlept, timeLeft);
        timeLeft = inMsec - timeSlept;
        if (timeLeft < 1)
            break;
        
        utimeLeft = timeLeft * 1000;    
        //qtss_printf("OSThread::Sleep usleep=%qd\n", utimeLeft);
        ::usleep(utimeLeft);
        
        timeSlept = (OS::Milliseconds() - startTime);
        if (timeSlept < 0) // system time set backwards
            break;
            
    } while (timeSlept < inMsec);

    //qtss_printf("total sleep = %qd request sleep=%"_U32BITARG_"\n", timeSlept,inMsec);

#elif defined(__osf__) || defined(__hpux__)
    if (inMsec < 1000)
        ::usleep(inMsec * 1000); // useconds must be less than 1,000,000
    else
        ::sleep((inMsec + 500) / 1000); // round to the nearest whole second
#elif defined(__sgi__) 
	struct timespec ts;
	
	ts.tv_sec = 0;
	ts.tv_nsec = inMsec * 1000000;

	nanosleep(&ts, 0);
#else
    ::usleep(inMsec * 1000);
#endif
}

#ifdef __Win32__
unsigned int WINAPI OSThread::_Entry(LPVOID inThread)
#else
void* OSThread::_Entry(void *inThread)  //static
#endif
{
    OSThread* theThread = (OSThread*)inThread;
#ifdef __Win32__
    BOOL theErr = ::TlsSetValue(sThreadStorageIndex, theThread);
    Assert(theErr == TRUE);
#elif __PTHREADS__
    theThread->fThreadID = (pthread_t)pthread_self();
    pthread_setspecific(OSThread::gMainKey, theThread);
#else
    theThread->fThreadID = (UInt32)cthread_self();
    cthread_set_data(cthread_self(), (any_t)theThread);
#endif
    theThread->SwitchPersonality();
    //
    // Run the thread
    theThread->Entry();
    return NULL;
}


Bool16  OSThread::SwitchPersonality()
{
#if __linux__
   if (::strlen(sGroup) > 0)
    {
        struct group* gr = ::getgrnam(sGroup);
        if (gr == NULL || ::setgid(gr->gr_gid) == -1)
        {
            //qtss_printf("thread %"_U32BITARG_" setgid  to group=%s FAILED \n", (UInt32) this, sGroup);
            return false;
        }
        
        //qtss_printf("thread %"_U32BITARG_" setgid  to group=%s \n", (UInt32) this, sGroup);
    }
    
        
    if (::strlen(sUser) > 0)
    {
        struct passwd* pw = ::getpwnam(sUser);
        if (pw == NULL || ::setuid(pw->pw_uid) == -1)
        {
            //qtss_printf("thread %"_U32BITARG_" setuid  to user=%s FAILED \n", (UInt32) this, sUser);
            return false;
        }

        //qtss_printf("thread %"_U32BITARG_" setuid  to user=%s \n", (UInt32) this, sUser);
   }
#endif

   return true;
}


OSThread*   OSThread::GetCurrent()
{
#ifdef __Win32__
    return (OSThread *)::TlsGetValue(sThreadStorageIndex);
#elif __PTHREADS__
    return (OSThread *)pthread_getspecific(OSThread::gMainKey);
#else
    return (OSThread*)cthread_data(cthread_self());
#endif
}

#ifdef __Win32__
int OSThread::GetErrno()
{
    int winErr = ::GetLastError();

    
    // Convert to a POSIX errorcode. The *major* assumption is that
    // the meaning of these codes is 1-1 and each Winsock, etc, etc
    // function is equivalent in errors to the POSIX standard. This is 
    // a big assumption, but the server only checks for a small subset of
    // the real errors, on only a small number of functions, so this is probably ok.
    switch (winErr)
    {

        case ERROR_FILE_NOT_FOUND: return ENOENT;

        case ERROR_PATH_NOT_FOUND: return ENOENT;       




        case WSAEINTR:      return EINTR;
        case WSAENETRESET:  return EPIPE;
        case WSAENOTCONN:   return ENOTCONN;
        case WSAEWOULDBLOCK:return EAGAIN;
        case WSAECONNRESET: return EPIPE;
        case WSAEADDRINUSE: return EADDRINUSE;
        case WSAEMFILE:     return EMFILE;
        case WSAEINPROGRESS:return EINPROGRESS;
        case WSAEADDRNOTAVAIL: return EADDRNOTAVAIL;
        case WSAECONNABORTED: return EPIPE;
        case 0:             return 0;
        
        default:            return ENOTCONN;
    }
}
#endif
