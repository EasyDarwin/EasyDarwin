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
	 File:       EventContext.h

	 Contains:   An event context provides the intelligence to take an event
				 generated from a UNIX file descriptor (usually EV_RE or EV_WR)
				 and signal a Task.




 */

#ifndef __EVENT_CONTEXT_H__
#define __EVENT_CONTEXT_H__

#include "OSThread.h"
#include "Task.h"
#include "OSRef.h"
#include <atomic>

using namespace std;

#if MACOSXEVENTQUEUE
#ifdef AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER
#include <sys/ev.h>
#else
#include "ev.h"
#endif
#else
#include "ev.h"
#include "epollEvent.h"
#endif

 //enable to trace event context execution and the task associated with the context
#define EVENTCONTEXT_DEBUG 0

class EventThread;

class EventContext
{
public:

	//
	// Constructor. Pass in the EventThread you would like to receive
	// events for this context, and the fd that this context applies to
	EventContext(int inFileDesc, EventThread* inThread);
	virtual ~EventContext() { if (fAutoCleanup) this->Cleanup(); }

	//
	// InitNonBlocking
	//
	// Sets inFileDesc to be non-blocking. Once this is called, the
	// EventContext object "owns" the file descriptor, and will close it
	// when Cleanup is called. This is necessary because of some weird
	// select() behavior. DON'T CALL CLOSE ON THE FD ONCE THIS IS CALLED!!!!
	void            InitNonBlocking(int inFileDesc);

	//
	// Cleanup. Will be called by the destructor, but can be called earlier
	void            Cleanup();

	//
	// Arms this EventContext. Pass in the events you would like to receive
	void            RequestEvent(int theMask = EV_RE);


	//
	// Provide the task you would like to be notified
	void            SetTask(Task* inTask)
	{
		fTask = inTask;
		if (EVENTCONTEXT_DEBUG)
		{
			if (fTask == NULL)
				qtss_printf("EventContext::SetTask context=%p task= NULL\n", (void *) this);
			else
				qtss_printf("EventContext::SetTask context=%p task= %p name=%s\n", (void *) this, (void *)fTask, fTask->fTaskName);
		}
	}

	// when the HTTP Proxy tunnels takes over a TCPSocket, we need to maintain this context too
	void            SnarfEventContext(EventContext& fromContext);

	// Don't cleanup this socket automatically
	void            DontAutoCleanup() { fAutoCleanup = false; }

	// Direct access to the FD is not recommended, but is needed for modules
	// that want to use the Socket classes and need to request events on the fd.
	int             GetSocketFD() { return fFileDesc; }

	enum
	{
		kInvalidFileDesc = -1   //int
	};

protected:

	//
	// ProcessEvent
	//
	// When an event occurs on this file descriptor, this function
	// will get called. Default behavior is to Signal the associated
	// task, but that behavior may be altered / overridden.
	//
	// Currently, we always generate a Task::kReadEvent
	virtual void ProcessEvent(int /*eventBits*/)
	{
		if (EVENTCONTEXT_DEBUG)
		{
			if (fTask == NULL)
				qtss_printf("EventContext::ProcessEvent context=%p task=NULL\n", (void *) this);
			else
				qtss_printf("EventContext::ProcessEvent context=%p task=%p TaskName=%s\n", (void *)this, (void *)fTask, fTask->fTaskName);
		}

		if (fTask != NULL)
			fTask->Signal(Task::kReadEvent);
	}

	int             fFileDesc;

private:
	struct eventreq fEventReq;

	OSRef           fRef;
	PointerSizedInt fUniqueID;
	StrPtrLen       fUniqueIDStr;
	EventThread*    fEventThread;
	bool          fWatchEventCalled;
	int             fEventBits;
	bool          fAutoCleanup;

	Task*           fTask;
#if DEBUG
	bool          fModwatched;
#endif

	//static unsigned int sUniqueID;
	//static atomic_uint sUniqueID;
        static atomic<unsigned int> sUniqueID;

	friend class EventThread;
};

class EventThread : public OSThread
{
public:

	EventThread() : OSThread() {}
	virtual ~EventThread() {}

private:

	virtual void Entry();
	OSRefTable      fRefTable;

	friend class EventContext;
};

#endif //__EVENT_CONTEXT_H__
