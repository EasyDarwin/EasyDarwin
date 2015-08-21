/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#ifndef _COMMON_THREAD_H_
#define _COMMON_THREAD_H_


#ifndef _PTHREAD_EMULATION
#define _PTHREAD_EMULATION

//#define VPXINFINITE 10000       //10second.
#define VPXINFINITE -1       //10second.

#if defined(_PURE_ADS_)
/* nothing for ADS */
#define THREAD_FUNCTION void *
#else

/* Thread management macros */
#ifdef _WIN32
/* Win32 */
//#define _WIN32_WINNT 0x500 /* WINBASE.H - Enable signal_object_and_wait */
#ifndef WINCE
#include <process.h>
#endif

#define THREAD_FUNCTION DWORD WINAPI
#define THREAD_FUNCTION_RETURN DWORD
#define THREAD_SPECIFIC_INDEX DWORD
#define pthread_t HANDLE
#define pthread_attr_t DWORD
//#define pthread_create(thhandle,attr,thfunc,tharg) (int)((HANDLE)_beginthreadex(NULL,0,(unsigned int (__stdcall *)(void *))thfunc,tharg,0,(unsigned*)thhandle)==NULL)
#define pthread_create(thhandle,attr,thfunc,tharg) (int)((*thhandle=(HANDLE)_beginthreadex(NULL,0,(unsigned int (__stdcall *)(void *))thfunc,tharg,0,NULL))==NULL)
//#define pthread_join(thread, result) ((WaitForSingleObject((thread),VPXINFINITE)!=WAIT_OBJECT_0) || !CloseHandle(thread))
#define pthread_join(thread, result) if((WaitForSingleObject((thread),VPXINFINITE))==WAIT_OBJECT_0)CloseHandle(thread)
#define pthread_detach(thread) if(thread!=NULL)CloseHandle(thread)
#define thread_sleep(nms) Sleep(nms)
#define pthread_cancel(thread) TerminateThread(thread,0) 
#define ts_key_create(ts_key, destructor) {ts_key = TlsAlloc();};
#define pthread_getspecific(ts_key) TlsGetValue(ts_key)
#define pthread_setspecific(ts_key, value) TlsSetValue(ts_key, (void *)value)
#define pthread_self() GetCurrentThreadId()
#else  //_WIN32
#ifdef __APPLE__
#include <mach/semaphore.h>
#include <mach/task.h>
#include <time.h>
#include <unistd.h>

#else  //__APPLE__
#include <semaphore.h>
#endif //__APPLE__

#ifdef X86
#define __USE_GNU 
#endif

#ifndef __linux__
#define _GNU_SOURCE
#endif

#include <pthread.h>

/* pthreads */
/* Nearly everything is already defined */
#define THREAD_FUNCTION void *
#define THREAD_FUNCTION_RETURN void *
#define THREAD_SPECIFIC_INDEX pthread_key_t
#define ts_key_create(ts_key, destructor) pthread_key_create (&(ts_key), destructor);
#endif //_WIN32

/* Syncrhronization macros: Win32 and Pthreads */
#ifdef _WIN32
#define sem_t HANDLE
#define pause(voidpara) __asm PAUSE
#define sem_init(sem, sem_attr1, sem_init_value) (*sem) = CreateSemaphore(sem_attr1,sem_init_value,100, NULL)
#define sem_wait(sem) (int)(WAIT_OBJECT_0 != WaitForSingleObject(*sem,INFINITE))
#define sem_trywait(sem) (int)(WaitForSingleObject( *sem, 0 ) == WAIT_TIMEOUT ? -1 : 0)
#define sem_post(sem) ReleaseSemaphore(*sem, 1, NULL)
#define sem_destroy(sem) if(*sem)((int)(CloseHandle(*sem))==TRUE)
#define thread_sleep(nms) Sleep(nms)

#else  //_WIN32

#ifdef __APPLE__
#define sem_t semaphore_t
#define sem_init(X,Y,Z) semaphore_create(mach_task_self(), X, SYNC_POLICY_FIFO, Z)
#define sem_wait(sem) (semaphore_wait(*sem) )
#define sem_trywait(sem) (semaphore_wait_timeout(*sem, TIMEOUT_IMMEDIATE))
#define sem_post(sem) semaphore_signal(*sem)
#define sem_destroy(sem) semaphore_destroy(mach_task_self(),*sem)
#define thread_sleep(nms) // { struct timespec ts;ts.tv_sec=0; ts.tv_nsec = 1000*nms;nanosleep(&ts, NULL);}
#else  //__APPLE__
#include <unistd.h>
#define thread_sleep(nms) usleep(nms*1000);// {struct timespec ts;ts.tv_sec=0; ts.tv_nsec = 1000*nms;nanosleep(&ts, NULL);}
#endif //__APPLE__
/* Not Windows. Assume pthreads */

#endif  //_WIN32

#if ARCH_X86 || ARCH_X86_64
#include "vpx_ports/x86.h"
#else
#define x86_pause_hint()
#endif

#endif //_PURE_ADS_

#endif //_PTHREAD_EMULATION

#endif // _COMMON_THREAD_H_

