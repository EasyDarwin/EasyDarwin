/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005
 *					All rights reserved
 *
 *  This file is part of GPAC / common tools sub-project
 *
 *  GPAC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  GPAC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef _GF_THREAD_H_
#define _GF_THREAD_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *	\file <gpac/thread.h>
 *	\brief threading functions.
 */

 /*!
 *	\addtogroup thr_grp threading
 *	\ingroup utils_grp
 *	\brief Threading and Mutual Exclusion Functions
 *
 *This section documents the threading of the GPAC framework. These provide an easy way to implement
 *safe multithreaded tools.
 *	@{
 */

#include <gpac/tools.h>

/*!
 *\brief Thread states
 *
 *Inidcates the execution status of a thread
 */
enum
{
	/*! the thread has been initialized but is not started yet*/
	GF_THREAD_STATUS_STOP = 0,
	/*! the thread is running*/
	GF_THREAD_STATUS_RUN = 1,
	/*! the thread has exited its body function*/
	GF_THREAD_STATUS_DEAD = 2
};

/*!
 *\brief abstracted thread object
 *
 *The abstracted thread object allows you to execute some code independently of the main process of your application.
*/
typedef struct __tag_thread GF_Thread;

/*!
 *\brief thread constructor
 *
 *Constructs a new thread object
 *\param log name of the thread if any
 */
GF_Thread *gf_th_new(const char *name);
/*!
 *\brief thread destructor
 *
 * Kills the thread if running and destroys the object
 *\param th the thread object
 */
void gf_th_del(GF_Thread *th);

/*!
 *	\brief thread run function callback
 *
 *The gf_thread_run type is the type for the callback of the \ref gf_thread_run function
 *\param par opaque user data
 *\return exit code of the thread, usually 1 for error and 0 if normal execution
 *
 */
typedef u32 (*gf_thread_run)(void *par);

/*!
 *\brief thread execution
 *
 *Executes the thread with the given function
 *\param th the thread object
 *\param run the function this thread will call
 *\param par the argument to the function the thread will call
 *\note A thread may be run several times but cannot be run twice in the same time.
 */
GF_Err gf_th_run(GF_Thread *th, gf_thread_run run, void *par);
/*!
 *\brief thread stoping
 *
 *Waits for the thread exit until return
 *\param th the thread object
 */
void gf_th_stop(GF_Thread *th);
/*!
 *\brief thread status query
 *
 *Gets the thread status
 *\param th the thread object
 */
u32 gf_th_status(GF_Thread *th);

/*!
 * thread priorities
 */
enum
{
	/*!Idle Priority*/
	GF_THREAD_PRIORITY_IDLE=0,
	/*!Less Idle Priority*/
	GF_THREAD_PRIORITY_LESS_IDLE,
	/*!Lowest Priority*/
	GF_THREAD_PRIORITY_LOWEST,
	/*!Low Priority*/
	GF_THREAD_PRIORITY_LOW,
	/*!Normal Priority (the default one)*/
	GF_THREAD_PRIORITY_NORMAL,
	/*!High Priority*/
	GF_THREAD_PRIORITY_HIGH,
	/*!Highest Priority*/
	GF_THREAD_PRIORITY_HIGHEST,
	/*!First real-time priority*/
	GF_THREAD_PRIORITY_REALTIME,
	/*!Last real-time priority*/
	GF_THREAD_PRIORITY_REALTIME_END=255
};

/*!
 *\brief thread priority
 *
 *Sets the thread execution priority level.
 *\param th the thread object
 *\param priority the desired priority
 *\note this should be used with caution, especially use of real-time priorities.
 */
void gf_th_set_priority(GF_Thread *th, s32 priority);
/*!
 *\brief current thread ID
 *
 *Gets the ID of the current thread the caller is in.
*/
u32 gf_th_id();

#ifdef GPAC_ANDROID
/*!
 * Register a function that will be called before pthread_exist is called
 */
GF_Err gf_register_before_exit_function(GF_Thread *t, u32 (*toRunBeforePthreadExit)(void *param));

/*! Get the current Thread if any. May return NULL
 */
GF_Thread * gf_th_current();

#endif /* GPAC_ANDROID */

/*!
 *\brief abstracted mutex object
 *
 *The abstracted mutex object allows you to make sure that portions of the code (typically access to variables) cannot be executed
 *by two threads (or a thread and the main process) at the same time.
*/
typedef struct __tag_mutex GF_Mutex;
/*
 *\brief mutex constructor
 *
 *Contructs a new mutex object
 *\param log name of the thread if any
*/
GF_Mutex *gf_mx_new(const char *name);
/*
 *\brief mutex denstructor
 *
 *Destroys a mutex object. This will wait for the mutex to be released if needed.
 *\param mx the mutex object
*/
void gf_mx_del(GF_Mutex *mx);
/*
 *\brief mutex locking
 *
 *Locks the mutex object, making sure that another thread locking this mutex cannot execute until the mutex is unlocked.
 *\param mx the mutex object
 *\return 1 if success, 0 if error locking the mutex (which should never happen)
*/
u32 gf_mx_p(GF_Mutex *mx);
/*
 *\brief mutex unlocking
 *
 *Unlocks the mutex object, allowing other threads waiting on this mutex to continue their execution
 *\param mx the mutex object
*/
void gf_mx_v(GF_Mutex *mx);
/*
 *\brief mutex non-blocking lock
 *
 *Attemps to lock the mutex object without blocking until the object is released.
 *\param mx the mutex object
 *\return 1 if the mutex has been successfully locked, in which case it shall then be unlocked, or 0 if the mutex is locked by another thread.
*/
Bool gf_mx_try_lock(GF_Mutex *mx);


/*********************************************************************
					Semaphore Object
**********************************************************************/
/*!
 *\brief abstracted semaphore object
 *
 *The abstracted semaphore object allows you to control how portions of the code (typically access to variables) are executed
 *by two threads (or a thread and the main process) at the same time. The best image for a semaphore is a limited set
 *of money coins (always easy to understand hmm?). If no money is in the set, nobody can buy anything until a coin is
 *put back in the set. When the set is full, the money is wasted (call it "the bank"...).
*/
typedef struct __tag_semaphore GF_Semaphore;
/*
 *\brief semaphore constructor
 *
 *Constructs a new semaphore object
 *\param MaxCount the maximum notification count of this semaphore
 *\param InitCount the initial notification count of this semaphore upon construction
 *\return the semaphore object
 */
GF_Semaphore *gf_sema_new(u32 MaxCount, u32 InitCount);
/*
 *\brief semaphore destructor
 *
 *Destructs the semaphore object. This will wait for the semaphore to be released if needed.
 */
void gf_sema_del(GF_Semaphore *sm);
/*
 *\brief semaphore notifivation
 *
 *Notifies the semaphore of a certain amount of releases.
 *\param sm the semaphore object
 *\param nb_rel sm the number of release to notify
 *\return the number of previous notification count in the semaphore
*/
u32 gf_sema_notify(GF_Semaphore *sm, u32 nb_rel);
/*
 *\brief semaphore wait
 *
 *Waits for the semaphore to be accessible (eg, may wait an infinite time).
 *\param sm the semaphore object
*/
void gf_sema_wait(GF_Semaphore *sm);
/*
 *\brief semaphore time wait
 *
 *Waits for a certain for the semaphore to be accessible, and returns when semaphore is accessible or wait time has passed.
 *\param sm the semaphore object
 *\param time_out the amount of time to wait for the release in milliseconds
 *\return returns 1 if the semaphore was released before the timeout, 0 if the semaphore is still not released after the timeout.
*/
Bool gf_sema_wait_for(GF_Semaphore *sm, u32 time_out);

/*! @} */

#ifdef __cplusplus
}
#endif


#endif		/*_GF_THREAD_H_*/

