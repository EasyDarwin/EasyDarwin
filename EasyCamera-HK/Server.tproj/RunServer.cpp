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
	 File:       main.cpp

	 Contains:   main function to drive streaming server.
 */

#include <errno.h>

#include "RunServer.h"
#include "SafeStdLib.h"
#include "OS.h"
#include "OSMemory.h"
#include "OSThread.h"
#include "Socket.h"
#include "ev.h"
#include "Task.h"
#include "IdleTask.h"
#include "TimeoutTask.h"

#ifndef __Win32__
#include <sys/types.h>
#include <unistd.h>
#include <OSArrayObjectDeleter.h>
#endif

#include "QTSServerInterface.h"
#include "QTSServer.h"

#include <stdlib.h>
#include <sys/stat.h>

QTSServer* sServer = NULL;
int sStatusUpdateInterval = 0;
Bool16 sHasPID = false;
UInt64 sLastDebugPackets = 0;
SInt64 sLastDebugTotalQuality = 0;
#ifdef __sgi__ 
#include <sched.h>
#endif

QTSS_ServerState StartServer(XMLPrefsParser* inPrefsSource, PrefsSource* inMessagesSource, int statsUpdateInterval, QTSS_ServerState inInitialState, Bool16 inDontFork, UInt32 debugLevel, UInt32 debugOptions)
{
	//Mark when we are done starting up. If auto-restart is enabled, we want to make sure
	//to always exit with a status of 0 if we encountered a problem WHILE STARTING UP. This
	//will prevent infinite-auto-restart-loop type problems
	Bool16 doneStartingUp = false;
	QTSS_ServerState theServerState = qtssStartingUpState;

	sStatusUpdateInterval = statsUpdateInterval;

	//Initialize utility classes
	OS::Initialize();
	OSThread::Initialize();

	Socket::Initialize();
	//SocketUtils::Initialize(!inDontFork);

//#if !MACOSXEVENTQUEUE
	::select_startevents();//initialize the select() implementation of the event queue
//#endif

	//start the server
	QTSSDictionaryMap::Initialize();
	QTSServerInterface::Initialize();// this must be called before constructing the server object
	sServer = NEW QTSServer();
	sServer->SetDebugLevel(debugLevel);
	sServer->SetDebugOptions(debugOptions);

	// re-parse config file
	inPrefsSource->Parse();

	sServer->Initialize(inPrefsSource, inMessagesSource);

	if (inInitialState == qtssShuttingDownState)
	{
		sServer->InitModules(inInitialState);
		return inInitialState;
	}

	if (sServer->GetServerState() != qtssFatalErrorState)
	{
		UInt32 numShortTaskThreads = 0;
		UInt32 numBlockingThreads = 0;
		UInt32 numThreads = 0;
		UInt32 numProcessors = 0;

		if (OS::ThreadSafe())
		{
			numShortTaskThreads = sServer->GetPrefs()->GetNumThreads(); // whatever the prefs say
			if (numShortTaskThreads == 0)
			{
				numProcessors = OS::GetNumProcessors();
				// 1 worker thread per processor, up to 2 threads.
				// Note: Limiting the number of worker threads to 2 on a MacOS X system with > 2 cores
				//     results in better performance on those systems, as of MacOS X 10.5.  Future
				//     improvements should make this limit unnecessary.
				if (numProcessors > 2)
					numShortTaskThreads = 2;
				else
					numShortTaskThreads = numProcessors;
			}

			numBlockingThreads = sServer->GetPrefs()->GetNumBlockingThreads(); // whatever the prefs say
			if (numBlockingThreads == 0)
				numBlockingThreads = 1;

		}
		if (numShortTaskThreads == 0)
			numShortTaskThreads = 1;

		if (numBlockingThreads == 0)
			numBlockingThreads = 1;

		numThreads = numShortTaskThreads + numBlockingThreads;
		//qtss_printf("Add threads shortask=%lu blocking=%lu\n",numShortTaskThreads, numBlockingThreads);
		TaskThreadPool::SetNumShortTaskThreads(numShortTaskThreads);
		TaskThreadPool::SetNumBlockingTaskThreads(numBlockingThreads);
		TaskThreadPool::AddThreads(numThreads);
		sServer->InitNumThreads(numThreads);

#if DEBUG
		qtss_printf("Number of task threads: %"_U32BITARG_"\n", numThreads);
#endif

		// Start up the server's global tasks, and start listening
		TimeoutTask::Initialize();     // The TimeoutTask mechanism is task based,
									// we therefore must do this after adding task threads
									// this be done before starting the sockets and server tasks
	}

	//Make sure to do this stuff last. Because these are all the threads that
	//do work in the server, this ensures that no work can go on while the server
	//is in the process of staring up
	if (sServer->GetServerState() != qtssFatalErrorState)
	{
		IdleTask::Initialize();
		Socket::StartThread();
		OSThread::Sleep(1000);

		//
		// On Win32, in order to call modwatch the Socket EventQueue thread must be
		// created first. Modules call modwatch from their initializer, and we don't
		// want to prevent them from doing that, so module initialization is separated
		// out from other initialization, and we start the Socket EventQueue thread first.
		// The server is still prevented from doing anything as of yet, because there
		// aren't any TaskThreads yet.
		sServer->InitModules(inInitialState);
		sServer->StartTasks();
		theServerState = sServer->GetServerState();
	}

	if (theServerState != qtssFatalErrorState)
	{
		CleanPid(true);
		WritePid(!inDontFork);

		doneStartingUp = true;
		qtss_printf("EasyCamera done starting up\n");
		OSMemory::SetMemoryError(ENOMEM);
	}

	//
	 // Tell the caller whether the server started up or not
	return theServerState;
}

void WritePid(Bool16 forked)
{
#ifndef __Win32__
	// WRITE PID TO FILE
	OSCharArrayDeleter thePidFileName(sServer->GetPrefs()->GetPidFilePath());
	FILE *thePidFile = fopen(thePidFileName, "w");
	if (thePidFile)
	{
		if (!forked)
			fprintf(thePidFile, "%d\n", getpid());    // write own pid
		else
		{
			fprintf(thePidFile, "%d\n", getppid());    // write parent pid
			fprintf(thePidFile, "%d\n", getpid());    // and our own pid in the next line
		}
		fclose(thePidFile);
		sHasPID = true;
	}
#endif
}

void CleanPid(Bool16 force)
{
#ifndef __Win32__
	if (sHasPID || force)
	{
		OSCharArrayDeleter thePidFileName(sServer->GetPrefs()->GetPidFilePath());
		unlink(thePidFileName);
	}
#endif
}

void RunServer()
{
	Bool16 restartServer = false;
	UInt32 loopCount = 0;
	UInt32 debugLevel = 0;
	Bool16 printHeader = false;
	Bool16 printStatus = false;


	//just wait until someone stops the server or a fatal error occurs.
	QTSS_ServerState theServerState = sServer->GetServerState();
	while ((theServerState != qtssShuttingDownState) &&
		(theServerState != qtssFatalErrorState))
	{
#ifdef __sgi__
		OSThread::Sleep(999);
#else
		OSThread::Sleep(1000);
#endif

		if ((sServer->SigIntSet()) || (sServer->SigTermSet()))
		{
			//
			// start the shutdown process
			theServerState = qtssShuttingDownState;
			(void)QTSS_SetValue(QTSServerInterface::GetServer(), qtssSvrState, 0, &theServerState, sizeof(theServerState));

			if (sServer->SigIntSet())
				restartServer = true;
		}
	}

	//Now, make sure that the server can't do any work
	TaskThreadPool::RemoveThreads();

	//now that the server is definitely stopped, it is safe to initate
	//the shutdown process
	delete sServer;

	CleanPid(false);
	//ok, we're ready to exit. If we're quitting because of some fatal error
	//while running the server, make sure to let the parent process know by
	//exiting with a nonzero status. Otherwise, exit with a 0 status
	if (theServerState == qtssFatalErrorState || restartServer)
		::exit(-2);//-2 signals parent process to restart server
}
