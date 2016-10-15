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
	 File:       TimeoutTask.cpp

	 Contains:   Implementation of TimeoutTask


 */

#include "TimeoutTask.h"
#include "OSMemory.h"

shared_ptr<TimeoutTaskThread> TimeoutTask::sThread = nullptr;

void TimeoutTask::Initialize()
{
	if (sThread == nullptr)
	{
		//sThread = new TimeoutTaskThread();
		sThread = make_shared<TimeoutTaskThread>();
		sThread->Signal(Task::kStartEvent);
	}

}


TimeoutTask::TimeoutTask(Task* inTask, SInt64 inTimeoutInMilSecs)
	: fTask(inTask), fQueueElem()
{
	fQueueElem.SetEnclosingObject(this);
	this->SetTimeout(inTimeoutInMilSecs);
	if (nullptr == inTask)
		fTask = (Task *) this;
	Assert(sThread != nullptr); // this can happen if RunServer intializes tasks in the wrong order

	OSMutexLocker locker(&sThread->fMutex);
	sThread->fQueue.EnQueue(&fQueueElem);
}

TimeoutTask::~TimeoutTask()
{
	OSMutexLocker locker(&sThread->fMutex);
	sThread->fQueue.Remove(&fQueueElem);
}

void TimeoutTask::SetTimeout(SInt64 inTimeoutInMilSecs)
{
	fTimeoutInMilSecs = inTimeoutInMilSecs;
	if (inTimeoutInMilSecs == 0)
		fTimeoutAtThisTime = 0;
	else
		fTimeoutAtThisTime = OS::Milliseconds() + fTimeoutInMilSecs;
}

SInt64 TimeoutTaskThread::Run()
{
	//ok, check for timeouts now. Go through the whole queue
	OSMutexLocker locker(&fMutex);
	SInt64 curTime = OS::Milliseconds();
	SInt64 intervalMilli = kIntervalSeconds * 1000;//always default to 60 seconds but adjust to smallest interval > 0
	SInt64 taskInterval = intervalMilli;

	for (OSQueueIter iter(&fQueue); !iter.IsDone(); iter.Next())
	{
		TimeoutTask* theTimeoutTask = (TimeoutTask*)iter.GetCurrent()->GetEnclosingObject();

		//if it's time to time this task out, signal it
		if ((theTimeoutTask->fTimeoutAtThisTime > 0) && (curTime >= theTimeoutTask->fTimeoutAtThisTime))
		{
#if TIMEOUT_DEBUGGING
			qtss_printf("TimeoutTask %" _S32BITARG_ " timed out. Curtime = %" _64BITARG_ "d, timeout time = %" _64BITARG_ "d\n", (SInt32)theTimeoutTask, curTime, theTimeoutTask->fTimeoutAtThisTime);
#endif
			theTimeoutTask->fTask->Signal(Task::kTimeoutEvent);
		}
		else
		{
			taskInterval = theTimeoutTask->fTimeoutAtThisTime - curTime;
			if ((taskInterval > 0) && (theTimeoutTask->fTimeoutInMilSecs > 0) && (intervalMilli > taskInterval))
				intervalMilli = taskInterval + 1000; // set timeout to 1 second past this task's timeout
#if TIMEOUT_DEBUGGING
			qtss_printf("TimeoutTask %" _S32BITARG_ " not being timed out. Curtime = %" _64BITARG_ "d. timeout time = %" _64BITARG_ "d\n", (SInt32)theTimeoutTask, curTime, theTimeoutTask->fTimeoutAtThisTime);
#endif
		}
	}
	(void)this->GetEvents();//we must clear the event mask!

	OSThread::ThreadYield();

#if TIMEOUT_DEBUGGING
	qtss_printf("TimeoutTaskThread::Run interval seconds= %" _S32BITARG_ "\n", (SInt32)intervalMilli / 1000);
#endif

	return intervalMilli;//don't delete me!
}
