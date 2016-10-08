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
	 File:       Task.cpp

	 Contains:   implements Task class


 */

#include "Task.h"
#include "OS.h"
#include "OSMemory.h"
#include "OSMutexRW.h"


unsigned int Task::sShortTaskThreadPicker = 0;
unsigned int Task::sBlockingTaskThreadPicker = 0;

OSMutexRW       TaskThreadPool::sMutexRW;
static char* sTaskStateStr = "live_"; //Alive

Task::Task()
	: fEvents(0), fUseThisThread(NULL), fDefaultThread(NULL), fWriteLock(false), fTimerHeapElem(), fTaskQueueElem(), pickerToUse(&Task::sShortTaskThreadPicker)
{
#if DEBUG
	fInRunCount = 0;
#endif
	this->SetTaskName("unknown");

	fTaskQueueElem.SetEnclosingObject(this);
	fTimerHeapElem.SetEnclosingObject(this);

}

void Task::SetTaskName(char* name)
{
	if (name == NULL)
		return;

	::strncpy(fTaskName, sTaskStateStr, sizeof(fTaskName));
	::strncat(fTaskName, name, sizeof(fTaskName));
	fTaskName[sizeof(fTaskName) - 1] = 0; //terminate in case it is longer than ftaskname.

}

bool Task::Valid()
{
	if ((this->fTaskName == NULL) || (0 != ::strncmp(sTaskStateStr, this->fTaskName, 5)))
	{
		if (TASK_DEBUG) qtss_printf("Task::Valid Found invalid task = %p\n", (void *)this);

		return false;
	}

	return true;
}

Task::EventFlags Task::GetEvents()
{
	//Mask off every event currently in the mask except for the alive bit, of course,
	//which should remain unaffected and unreported by this call.
	EventFlags events = fEvents & kAliveOff;
	//(void)atomic_sub(&fEvents, events);
	fEvents.fetch_sub(events);
	return events;
}

void Task::Signal(EventFlags events)
{
	if (!this->Valid())
		return;

	//Fancy no mutex implementation. We atomically mask the new events into
	//the event mask. Because atomic_or returns the old state of the mask,
	//we only schedule this task once.
	events |= kAlive;
	//EventFlags oldEvents = atomic_or(&fEvents, events);
	auto oldEvents = fEvents.fetch_or(events);
	if ((!(oldEvents & kAlive)) && (TaskThreadPool::sNumTaskThreads > 0))
	{
		if (fDefaultThread != NULL && fUseThisThread == NULL)
			fUseThisThread = fDefaultThread;

		if (fUseThisThread != NULL)// Task needs to be placed on a particular thread.
		{

			if (TASK_DEBUG)
			{
				if (fTaskName[0] == 0) ::strcpy(fTaskName, " _Corrupt_Task");
				qtss_printf("Task::Signal EnQueue TaskName=%s fUseThisThread=%p q_elem=%p enclosing=%p\n", fTaskName, (void *)fUseThisThread, (void *)&fTaskQueueElem, (void *) this);
				if (TaskThreadPool::sTaskThreadArray[0] == fUseThisThread) qtss_printf("Task::Signal  RTSP Thread running  TaskName=%s \n", fTaskName);
			}

			fUseThisThread->fTaskQueue.EnQueue(&fTaskQueueElem);
		}
		else
		{
			//find a thread to put this task on
			//unsigned int theThreadIndex = atomic_add((unsigned int *)pickerToUse, 1);

			unsigned int theThreadIndex = 0;
			{
				OSMutexLocker locker(&fAtomicMutex);
				theThreadIndex = ++(*pickerToUse);
			}

			if (&Task::sShortTaskThreadPicker == pickerToUse)
			{
				theThreadIndex %= TaskThreadPool::sNumShortTaskThreads;

				if (TASK_DEBUG)  qtss_printf("Task::Signal EnQueue TaskName=%s using Task::sShortTaskThreadPicker=%u numShortTaskThreads=%"   _U32BITARG_   " short task range=[0-%"   _U32BITARG_   "] thread index =%u \n", fTaskName, Task::sShortTaskThreadPicker, TaskThreadPool::sNumShortTaskThreads, TaskThreadPool::sNumShortTaskThreads - 1, theThreadIndex);
			}
			else if (&Task::sBlockingTaskThreadPicker == pickerToUse)
			{
				theThreadIndex %= TaskThreadPool::sNumBlockingTaskThreads;
				theThreadIndex += TaskThreadPool::sNumShortTaskThreads; //don't pick from lower non-blocking (short task) threads.

				if (TASK_DEBUG)  qtss_printf("Task::Signal EnQueue TaskName=%s using Task::sBlockingTaskThreadPicker=%u numBlockingThreads=%"   _U32BITARG_   " blocking thread range=[%"   _U32BITARG_   "-%"   _U32BITARG_   "] thread index =%u \n", fTaskName, Task::sBlockingTaskThreadPicker, TaskThreadPool::sNumBlockingTaskThreads, TaskThreadPool::sNumShortTaskThreads, TaskThreadPool::sNumBlockingTaskThreads + TaskThreadPool::sNumShortTaskThreads - 1, theThreadIndex);
			}
			else
			{
				if (TASK_DEBUG) if (fTaskName[0] == 0) ::strcpy(fTaskName, " _Corrupt_Task");

				return;
			}


			if (TASK_DEBUG) if (fTaskName[0] == 0) ::strcpy(fTaskName, " _Corrupt_Task");

			if (TASK_DEBUG) qtss_printf("Task::Signal EnQueue B TaskName=%s theThreadIndex=%u thread=%p fTaskQueue.GetLength(%"   _U32BITARG_   ") q_elem=%p enclosing=%p\n", fTaskName, theThreadIndex, (void *)TaskThreadPool::sTaskThreadArray[theThreadIndex], TaskThreadPool::sTaskThreadArray[theThreadIndex]->fTaskQueue.GetQueue()->GetLength(), (void *)&fTaskQueueElem, (void *) this);
			TaskThreadPool::sTaskThreadArray[theThreadIndex]->fTaskQueue.EnQueue(&fTaskQueueElem);
			if (TASK_DEBUG) qtss_printf("Task::Signal EnQueue A TaskName=%s theThreadIndex=%u thread=%p fTaskQueue.GetLength(%"   _U32BITARG_   ") q_elem=%p enclosing=%p\n", fTaskName, theThreadIndex, (void *)TaskThreadPool::sTaskThreadArray[theThreadIndex], TaskThreadPool::sTaskThreadArray[theThreadIndex]->fTaskQueue.GetQueue()->GetLength(), (void *)&fTaskQueueElem, (void *) this);

		}
	}
	else
		if (TASK_DEBUG) qtss_printf("Task::Signal Sent to dead TaskName=%s  q_elem=%p  enclosing=%p\n", fTaskName, (void *)&fTaskQueueElem, (void *) this);
}


void Task::GlobalUnlock()
{
	if (this->fWriteLock)
	{
		this->fWriteLock = false;
		TaskThreadPool::sMutexRW.Unlock();
	}
}

void Task::SetThreadPicker(unsigned int* picker)
{
	pickerToUse = picker;
	Assert(pickerToUse != NULL);
	if (TASK_DEBUG)
	{
		if (fTaskName[0] == 0) ::strcpy(fTaskName, " _Corrupt_Task");

		if (&Task::sShortTaskThreadPicker == pickerToUse)
		{
			qtss_printf("Task::SetThreadPicker sShortTaskThreadPicker for task=%s\n", fTaskName);
		}
		else if (&Task::sBlockingTaskThreadPicker == pickerToUse)
		{
			qtss_printf("Task::SetThreadPicker sBlockingTaskThreadPicker for task=%s\n", fTaskName);
		}
		else
		{
			qtss_printf("Task::SetThreadPicker ERROR unknown picker for task=%s\n", fTaskName);
		}
	}

}

void TaskThread::Entry()
{
	Task* theTask = NULL;

	while (true)
	{
		theTask = this->WaitForTask();

		//
		// WaitForTask returns NULL when it is time to quit
		if (theTask == NULL || false == theTask->Valid())
			return;

		bool doneProcessingEvent = false;

		while (!doneProcessingEvent)
		{
			//If a task holds locks when it returns from its Run function,
			//that would be catastrophic and certainly lead to a deadlock
#if DEBUG
			Assert(this->GetNumLocksHeld() == 0);
			Assert(theTask->fInRunCount == 0);
			theTask->fInRunCount++;
#endif
			theTask->fUseThisThread = NULL; // Each invocation of Run must independently
											// request a specific thread.
			SInt64 theTimeout = 0;

			if (theTask->fWriteLock)
			{
				OSMutexWriteLocker mutexLocker(&TaskThreadPool::sMutexRW);
				if (TASK_DEBUG) qtss_printf("TaskThread::Entry run global locked TaskName=%s CurMSec=%.3f thread=%p task=%p\n", theTask->fTaskName, OS::StartTimeMilli_Float(), (void *) this, (void *)theTask);

				theTimeout = theTask->Run();
				theTask->fWriteLock = false;
			}
			else
			{
				OSMutexReadLocker mutexLocker(&TaskThreadPool::sMutexRW);
				if (TASK_DEBUG) qtss_printf("TaskThread::Entry run TaskName=%s CurMSec=%.3f thread=%p task=%p\n", theTask->fTaskName, OS::StartTimeMilli_Float(), (void *) this, (void *)theTask);

				theTimeout = theTask->Run();

			}
#if DEBUG
			Assert(this->GetNumLocksHeld() == 0);
			theTask->fInRunCount--;
			Assert(theTask->fInRunCount == 0);
#endif          
			if (theTimeout < 0)
			{
				if (TASK_DEBUG)
				{
					qtss_printf("TaskThread::Entry delete TaskName=%s CurMSec=%.3f thread=%p task=%p\n", theTask->fTaskName, OS::StartTimeMilli_Float(), (void *) this, (void *)theTask);

					theTask->fUseThisThread = NULL;

					if (NULL != fHeap.Remove(&theTask->fTimerHeapElem))
						qtss_printf("TaskThread::Entry task still in heap before delete\n");

					if (NULL != theTask->fTaskQueueElem.InQueue())
						qtss_printf("TaskThread::Entry task still in queue before delete\n");

					theTask->fTaskQueueElem.Remove();

					if (theTask->fEvents &~Task::kAlive)
						qtss_printf("TaskThread::Entry flags still set  before delete\n");

					//(void)atomic_sub(&theTask->fEvents, 0);
					theTask->fEvents.fetch_sub(0);

					::strncat(theTask->fTaskName, " deleted", sizeof(theTask->fTaskName) - 1);
				}
				theTask->fTaskName[0] = 'D'; //mark as dead
				delete theTask;
				theTask = NULL;
				doneProcessingEvent = true;

			}
			else if (theTimeout == 0)
			{
				//We want to make sure that 100% definitely the task's Run function WILL
				//be invoked when another thread calls Signal. We also want to make sure
				//that if an event sneaks in right as the task is returning from Run()
				//(via Signal) that the Run function will be invoked again.
				/*doneProcessingEvent = compare_and_store(Task::kAlive, 0, &theTask->fEvents);
				if (doneProcessingEvent)
					theTask = NULL;*/

				unsigned int val = Task::kAlive;
				doneProcessingEvent = theTask->fEvents.compare_exchange_weak(val, 0);
				if (doneProcessingEvent)
					theTask = nullptr;
			}
			else
			{
				//note that if we get here, we don't reset theTask, so it will get passed into
				//WaitForTask
				if (TASK_DEBUG) qtss_printf("TaskThread::Entry insert TaskName=%s in timer heap thread=%p elem=%p task=%p timeout=%.2f\n", theTask->fTaskName, (void *) this, (void *)&theTask->fTimerHeapElem, (void *)theTask, (float)theTimeout / (float)1000);
				theTask->fTimerHeapElem.SetValue(OS::Milliseconds() + theTimeout);
				fHeap.Insert(&theTask->fTimerHeapElem);
				//(void)atomic_or(&theTask->fEvents, Task::kIdleEvent);
				theTask->fEvents.fetch_or(Task::kIdleEvent);
				doneProcessingEvent = true;
			}


#if TASK_DEBUG
			SInt64  yieldStart = OS::Milliseconds();
#endif

			this->ThreadYield();
#if TASK_DEBUG
			SInt64  yieldDur = OS::Milliseconds() - yieldStart;
			static SInt64   numZeroYields;

			if (yieldDur > 1)
			{
				if (TASK_DEBUG) qtss_printf("TaskThread::Entry time in Yield %qd, numZeroYields %qd \n", yieldDur, numZeroYields);
				numZeroYields = 0;
			}
			else
				numZeroYields++;
#endif

		}
	}
}

Task* TaskThread::WaitForTask()
{
	while (true)
	{
		SInt64 theCurrentTime = OS::Milliseconds();

		if ((fHeap.PeekMin() != NULL) && (fHeap.PeekMin()->GetValue() <= theCurrentTime))
		{
			if (TASK_DEBUG) qtss_printf("TaskThread::WaitForTask found timer-task=%s thread %p fHeap.CurrentHeapSize(%"   _U32BITARG_   ") taskElem = %p enclose=%p\n", ((Task*)fHeap.PeekMin()->GetEnclosingObject())->fTaskName, (void *) this, fHeap.CurrentHeapSize(), (void *)fHeap.PeekMin(), (void *)fHeap.PeekMin()->GetEnclosingObject());
			return (Task*)fHeap.ExtractMin()->GetEnclosingObject();
		}

		//if there is an element waiting for a timeout, figure out how long we should wait.
		SInt64 theTimeout = 0;
		if (fHeap.PeekMin() != NULL)
			theTimeout = fHeap.PeekMin()->GetValue() - theCurrentTime;
		Assert(theTimeout >= 0);

		//
		// Make sure we can't go to sleep for some ridiculously short
		// period of time
		// Do not allow a timeout below 10 ms without first verifying reliable udp 1-2mbit live streams. 
		// Test with easydarwin.xml pref reliablUDP printfs enabled and look for packet loss and check client for  buffer ahead recovery.
		if (theTimeout < 10)
			theTimeout = 10;

		//wait...
		OSQueueElem* theElem = fTaskQueue.DeQueueBlocking(this, (SInt32)theTimeout);
		if (theElem != NULL)
		{
			if (TASK_DEBUG) qtss_printf("TaskThread::WaitForTask found signal-task=%s thread %p fTaskQueue.GetLength(%"   _U32BITARG_   ") taskElem = %p enclose=%p\n", ((Task*)theElem->GetEnclosingObject())->fTaskName, (void *) this, fTaskQueue.GetQueue()->GetLength(), (void *)theElem, (void *)theElem->GetEnclosingObject());
			return (Task*)theElem->GetEnclosingObject();
		}

		//
		// If we are supposed to stop, return NULL, which signals the caller to stop
		if (OSThread::GetCurrent()->IsStopRequested())
			return NULL;
	}
}

TaskThread** TaskThreadPool::sTaskThreadArray = NULL;
UInt32       TaskThreadPool::sNumTaskThreads = 0;
UInt32       TaskThreadPool::sNumShortTaskThreads = 0;
UInt32       TaskThreadPool::sNumBlockingTaskThreads = 0;

bool TaskThreadPool::AddThreads(UInt32 numToAdd)
{
	Assert(sTaskThreadArray == NULL);
	sTaskThreadArray = new TaskThread*[numToAdd];

	for (UInt32 x = 0; x < numToAdd; x++)
	{
		sTaskThreadArray[x] = NEW TaskThread();
		sTaskThreadArray[x]->Start();
		if (TASK_DEBUG)  qtss_printf("TaskThreadPool::AddThreads sTaskThreadArray[%"   _U32BITARG_   "]=%p\n", x, sTaskThreadArray[x]);
	}
	sNumTaskThreads = numToAdd;

	if (0 == sNumShortTaskThreads)
		sNumShortTaskThreads = numToAdd;

	return true;
}

TaskThread* TaskThreadPool::GetThread(UInt32 index)
{

	Assert(sTaskThreadArray != NULL);
	if (index >= sNumTaskThreads)
		return NULL;

	return sTaskThreadArray[index];

}




void TaskThreadPool::RemoveThreads()
{
	//Tell all the threads to stop
	for (UInt32 x = 0; x < sNumTaskThreads; x++)
		sTaskThreadArray[x]->SendStopRequest();

	//Because any (or all) threads may be blocked on the queue, cycle through
	//all the threads, signalling each one
	for (UInt32 y = 0; y < sNumTaskThreads; y++)
		sTaskThreadArray[y]->fTaskQueue.GetCond()->Signal();

	//Ok, now wait for the selected threads to terminate, deleting them and removing
	//them from the queue.
	for (UInt32 z = 0; z < sNumTaskThreads; z++)
		delete sTaskThreadArray[z];

	sNumTaskThreads = 0;
}
