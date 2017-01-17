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
	 Copyleft (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	 Github: https://github.com/EasyDarwin
	 WEChat: EasyDarwin
	 Website: http://www.EasyDarwin.org
 */
 /*
	 File:       main.cpp
	 Contains:   main function to drive service server.
 */

#include <errno.h>

#include "RunServer.h"
#include "SafeStdLib.h"
#include "OS.h"
#include "OSThread.h"
#include "Socket.h"
#include "SocketUtils.h"
#include "ev.h"
#include "Task.h"
#include "IdleTask.h"
#include "TimeoutTask.h"
#include "QTSSRollingLog.h"

#ifndef __Win32__
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#endif
#include "QTSServerInterface.h"
#include "QTSServer.h"

#include <stdlib.h>
#include <OSArrayObjectDeleter.h>

QTSServer* sServer = nullptr;
int sStatusUpdateInterval = 0;
bool sHasPID = false;
UInt64 sLastStatusPackets = 0;
UInt64 sLastDebugPackets = 0;
SInt64 sLastDebugTotalQuality = 0;
#ifdef __sgi__ 
#include <sched.h>
#endif

//启动服务
QTSS_ServerState StartServer(XMLPrefsParser* inPrefsSource, PrefsSource* inMessagesSource, UInt16 inPortOverride, int statsUpdateInterval, QTSS_ServerState inInitialState, bool inDontFork, UInt32 debugLevel, UInt32 debugOptions)
{
	//Mark when we are done starting up. If auto-restart is enabled, we want to make sure
	//to always exit with a status of 0 if we encountered a problem WHILE STARTING UP. This
	//will prevent infinite-auto-restart-loop type problems
	bool doneStartingUp = false;
	QTSS_ServerState theServerState = qtssStartingUpState;

	//服务单元状态更新间隔
	sStatusUpdateInterval = statsUpdateInterval;

	//Initialize utility classes
	OS::Initialize();
	OSThread::Initialize();

	//建立网络事件(R/W/E事件)线程，仅建立，But未启动
	Socket::Initialize();
	//获取系统网卡数量sNumIPAddrs及对应的具体ip，存储在sIPAddrInfoArray结构体数组中
	SocketUtils::Initialize(!inDontFork);
	/*
	#if !MACOSXEVENTQUEUE
		::select_startevents();//initialize the select() implementation of the event queue
	#endif
	*/
#if !MACOSXEVENTQUEUE
#ifndef __Win32__    
	::epollInit();
#else
	::select_startevents();//initialize the select() implementation of the event queue        
#endif

#endif
	//初始化系统属性字典
	QTSSDictionaryMap::Initialize();
	//初始化Server对象属性字典，包括其他具有属性字典的类，都要先进行Initialize
	QTSServerInterface::Initialize();//此部分必须在QTSServer对象构造前调用
	sServer = new QTSServer();
	sServer->SetDebugLevel(debugLevel);
	sServer->SetDebugOptions(debugOptions);

	//重新解析xml配置
	inPrefsSource->Parse();

	//准备开启监听，接收并处理相关事务
	bool createListeners = true;
	if (qtssShuttingDownState == inInitialState)
		createListeners = false;

	//初始化服务实例
	sServer->Initialize(inPrefsSource, inMessagesSource, inPortOverride, createListeners);

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
			//工作线程
			numShortTaskThreads = sServer->GetPrefs()->GetNumThreads(); // whatever the prefs say
			if (numShortTaskThreads == 0) {
				numProcessors = OS::GetNumProcessors();
				// 1 worker thread per processor, up to 2 threads.
				// Note:Limiting the number of worker threads to 2 on a MacOS X system with > 2 cores
				//		results in better performance on those systems, as of MacOS X 10.5.  Future
				//		improvements should make this limit unnecessary.
				if (numProcessors > 2)
					numShortTaskThreads = 2;
				else
					numShortTaskThreads = numProcessors;
			}

			//协议处理线程
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
		qtss_printf("Number of task threads: %" _U32BITARG_ "\n", numThreads);
#endif

		// Start up the server's global tasks, and start listening
		//超时事件处理线程
		TimeoutTask::Initialize();  // The TimeoutTask mechanism is task based,
									// we therefore must do this after adding task threads
									// this be done before starting the sockets and server tasks
	}

	//Make sure to do this stuff last. Because these are all the threads that
	//do work in the server, this ensures that no work can go on while the server
	//is in the process of staring up
	if (sServer->GetServerState() != qtssFatalErrorState)
	{
		//Idle事务处理线程
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

		//Log 
		char msgStr[128];
		qtss_snprintf(msgStr, sizeof(msgStr), "EasyCMS Service done starting up");
		QTSServerInterface::LogError(qtssMessageVerbosity, msgStr);

		//OSMemory::SetMemoryError(ENOMEM);
	}

	//// SWITCH TO RUN USER AND GROUP ID
	//if (!sServer->SwitchPersonality())
	//    theServerState = qtssFatalErrorState;

	// Tell the caller whether the server started up or not
	return theServerState;
}

void WritePid(bool forked)
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

void CleanPid(bool force)
{
#ifndef __Win32__
	if (sHasPID || force)
	{
		OSCharArrayDeleter thePidFileName(sServer->GetPrefs()->GetPidFilePath());
		unlink(thePidFileName);
	}
#endif
}

void LogStatus(QTSS_ServerState theServerState)
{
	static QTSS_ServerState lastServerState = 0;
	static char *sPLISTHeader[] =
	{
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>",
#if __MacOSX__
		"<!DOCTYPE plist SYSTEM \"file://localhost/System/Library/DTDs/PropertyList.dtd\">",
#else
		"<!ENTITY % plistObject \"(array | data | date | dict | real | integer | string | true | false )\">",
		"<!ELEMENT plist %plistObject;>",
		"<!ATTLIST plist version CDATA \"0.9\">",
		"",
		"<!-- Collections -->",
		"<!ELEMENT array (%plistObject;)*>",
		"<!ELEMENT dict (key, %plistObject;)*>",
		"<!ELEMENT key (#PCDATA)>",
		"",
		"<!--- Primitive types -->",
		"<!ELEMENT string (#PCDATA)>",
		"<!ELEMENT data (#PCDATA)> <!-- Contents interpreted as Base-64 encoded -->",
		"<!ELEMENT date (#PCDATA)> <!-- Contents should conform to a subset of ISO 8601 (in particular, YYYY '-' MM '-' DD 'T' HH ':' MM ':' SS 'Z'.  Smaller units may be omitted with a loss of precision) -->",
		"",
		"<!-- Numerical primitives -->",
		"<!ELEMENT true EMPTY>  <!-- Boolean constant true -->",
		"<!ELEMENT false EMPTY> <!-- Boolean constant false -->",
		"<!ELEMENT real (#PCDATA)> <!-- Contents should represent a floating point number matching (\"+\" | \"-\")? d+ (\".\"d*)? (\"E\" (\"+\" | \"-\") d+)? where d is a digit 0-9.  -->",
		"<!ELEMENT integer (#PCDATA)> <!-- Contents should represent a (possibly signed) integer number in base 10 -->",
		"]>",
#endif
	};

	static int numHeaderLines = sizeof(sPLISTHeader) / sizeof(char*);

	static char*    sPlistStart = "<plist version=\"0.9\">";
	static char*    sPlistEnd = "</plist>";
	static char*    sDictStart = "<dict>";
	static char*    sDictEnd = "</dict>";

	static char*    sKey = "     <key>%s</key>\n";
	static char*    sValue = "     <string>%s</string>\n";

	static char *sAttributes[] =
	{
		/* 0  */ "qtssServerAPIVersion",
		/* 1  */ "qtssSvrDefaultDNSName",
		/* 2  */ "qtssSvrDefaultIPAddr",
		/* 3  */ "qtssSvrServerName",
		/* 4  */ "qtssSvrServerVersion",
		/* 5  */ "qtssSvrServerBuildDate",
		/* 6  */ "qtssSvrHTTPPorts",
		/* 7  */ "qtssSvrHTTPServerHeader",
		/* 8  */ "qtssSvrState",
		/* 9  */ "qtssSvrIsOutOfDescriptors",
		/* 10 */ "qtssCurrentSessionCount",

		/* 11 */ "qtssSvrHandledMethods",
		/* 12 */ "qtssSvrModuleObjects",
		/* 13 */ "qtssSvrStartupTime",
		/* 14 */ "qtssSvrGMTOffsetInHrs",
		/* 15 */ "qtssSvrDefaultIPAddrStr",

		/* 16 */ "qtssSvrPreferences",
		/* 17 */ "qtssSvrMessages",
		/* 18 */ "qtssSvrClientSessions",
		/* 19 */ "qtssSvrCurrentTimeMilliseconds",
		/* 20 */ "qtssSvrCPULoadPercent",

		/* 21 */ "qtssSvrConnectedUsers",
		/* 22 */ "qtssSvrServerBuild",
		/* 23 */ "qtssSvrServerPlatform",
		/* 24 */ "qtssSvrHTTPServerComment",
		/* 25 */ "qtssSvrNumThinned",
		/* 26 */ "qtssSvrNumThreads"
	};
	static int numAttributes = sizeof(sAttributes) / sizeof(char*);

	static StrPtrLen statsFileNameStr("server_status");

	if (false == sServer->GetPrefs()->ServerStatFileEnabled())
		return;

	UInt32 interval = sServer->GetPrefs()->GetStatFileIntervalSec();
	if (interval == 0 || (OS::UnixTime_Secs() % interval) > 0)
		return;

	// If the total number of HTTP sessions is 0  then we 
	// might not need to update the "server_status" file.
	char* thePrefStr = nullptr;
	// We start lastHTTPSessionCount off with an impossible value so that
	// we force the "server_status" file to be written at least once.
	static int lastHTTPSessionCount = -1;
	// Get the HTTP session count from the server.
	(void)QTSS_GetValueAsString(sServer, qtssCurrentSessionCount, 0, &thePrefStr);
	int currentHTTPSessionCount = ::atoi(thePrefStr);
	delete[] thePrefStr; thePrefStr = nullptr;
	if (currentHTTPSessionCount == 0 && currentHTTPSessionCount == lastHTTPSessionCount)
	{
		// we don't need to update the "server_status" file except the
		// first time we are in the idle state.
		if (theServerState == qtssIdleState && lastServerState == qtssIdleState)
		{
			lastHTTPSessionCount = currentHTTPSessionCount;
			lastServerState = theServerState;
			return;
		}
	}
	else
	{
		lastHTTPSessionCount = currentHTTPSessionCount;
	}

	StrPtrLenDel pathStr(sServer->GetPrefs()->GetErrorLogDir());
	StrPtrLenDel fileNameStr(sServer->GetPrefs()->GetStatsMonitorFileName());
	ResizeableStringFormatter pathBuffer(nullptr, 0);
	pathBuffer.PutFilePath(&pathStr, &fileNameStr);
	pathBuffer.PutTerminator();

	char*   filePath = pathBuffer.GetBufPtr();
	FILE*   statusFile = ::fopen(filePath, "w");
	char*   theAttributeValue = nullptr;
	int     i;

	if (statusFile != nullptr)
	{
		::chmod(filePath, 0640);
		for (i = 0; i < numHeaderLines; i++)
		{
			qtss_fprintf(statusFile, "%s\n", sPLISTHeader[i]);
		}

		qtss_fprintf(statusFile, "%s\n", sPlistStart);
		qtss_fprintf(statusFile, "%s\n", sDictStart);

		// show each element value
		for (i = 0; i < numAttributes; i++)
		{
			(void)QTSS_GetValueAsString(sServer, QTSSModuleUtils::GetAttrID(sServer, sAttributes[i]), 0, &theAttributeValue);
			if (theAttributeValue != nullptr)
			{
				qtss_fprintf(statusFile, sKey, sAttributes[i]);
				qtss_fprintf(statusFile, sValue, theAttributeValue);
				delete[] theAttributeValue;
				theAttributeValue = nullptr;
			}
		}

		qtss_fprintf(statusFile, "%s\n", sDictEnd);
		qtss_fprintf(statusFile, "%s\n\n", sPlistEnd);

		::fclose(statusFile);
	}
	lastServerState = theServerState;
}

void print_status(FILE* file, FILE* console, char* format, char* theStr)
{
	if (file) qtss_fprintf(file, format, theStr);
	if (console) qtss_fprintf(console, format, theStr);

}

void DebugLevel_1(FILE*   statusFile, FILE*   stdOut, bool printHeader)
{
	char*  thePrefStr = nullptr;
	static char numStr[12] = "";
	static char dateStr[25] = "";
	UInt32 theLen = 0;

	if (printHeader)
	{
		printf("****************************************");
	}

	delete[] thePrefStr; thePrefStr = nullptr;

	(void)QTSS_GetValueAsString(sServer, qtssCurrentSessionCount, 0, &thePrefStr);
	print_status(statusFile, stdOut, "%11s", thePrefStr);
	delete[] thePrefStr; thePrefStr = nullptr;

	qtss_snprintf(numStr, sizeof(numStr) - 1, "%"  _U32BITARG_  "", (SInt32)sServer->GetNumThinned());
	print_status(statusFile, stdOut, "%11s", numStr);

	char theDateBuffer[QTSSRollingLog::kMaxDateBufferSizeInBytes];
	(void)QTSSRollingLog::FormatDate(theDateBuffer, false);

	qtss_snprintf(dateStr, sizeof(dateStr) - 1, "%s", theDateBuffer);
	print_status(statusFile, stdOut, "%24s\n", dateStr);
}

FILE* LogDebugEnabled()
{
	if (DebugLogOn(sServer))
	{
		static StrPtrLen statsFileNameStr("server_debug_status");

		StrPtrLenDel pathStr(sServer->GetPrefs()->GetErrorLogDir());
		ResizeableStringFormatter pathBuffer(nullptr, 0);
		pathBuffer.PutFilePath(&pathStr, &statsFileNameStr);
		pathBuffer.PutTerminator();

		char*   filePath = pathBuffer.GetBufPtr();
		return ::fopen(filePath, "a");
	}

	return nullptr;
}

FILE* DisplayDebugEnabled()
{
	return (DebugDisplayOn(sServer)) ? stdout : nullptr;
}

void DebugStatus(UInt32 debugLevel, bool printHeader)
{

	FILE*   statusFile = LogDebugEnabled();
	FILE*   stdOut = DisplayDebugEnabled();

	if (debugLevel > 0)
		DebugLevel_1(statusFile, stdOut, printHeader);

	if (statusFile)
		::fclose(statusFile);
}

void FormattedTotalBytesBuffer(char *outBuffer, int outBufferLen, UInt64 totalBytes)
{
	Float32 displayBytes = 0.0;
	char  sizeStr[] = "B";
	char* format = nullptr;

	if (totalBytes > 1073741824) //GBytes
	{
		displayBytes = (Float32)((Float64)(SInt64)totalBytes / (Float64)(SInt64)1073741824);
		sizeStr[0] = 'G';
		format = "%.4f%s ";
	}
	else if (totalBytes > 1048576) //MBytes
	{
		displayBytes = (Float32)(SInt32)totalBytes / (Float32)(SInt32)1048576;
		sizeStr[0] = 'M';
		format = "%.3f%s ";
	}
	else if (totalBytes > 1024) //KBytes
	{
		displayBytes = (Float32)(SInt32)totalBytes / (Float32)(SInt32)1024;
		sizeStr[0] = 'K';
		format = "%.2f%s ";
	}
	else
	{
		displayBytes = (Float32)(SInt32)totalBytes;  //Bytes
		sizeStr[0] = 'B';
		format = "%4.0f%s ";
	}

	outBuffer[outBufferLen - 1] = 0;
	qtss_snprintf(outBuffer, outBufferLen - 1, format, displayBytes, sizeStr);
}

void PrintStatus(bool printHeader)
{
	char* thePrefStr = nullptr;
	UInt32 theLen = 0;

	(void)QTSS_GetValueAsString(sServer, qtssCurrentSessionCount, 0, &thePrefStr);
	qtss_printf("%11s", thePrefStr);
	delete[] thePrefStr; thePrefStr = nullptr;

	//获取当前时间
	char theDateBuffer[QTSSRollingLog::kMaxDateBufferSizeInBytes];
	(void)QTSSRollingLog::FormatDate(theDateBuffer, false);
	qtss_printf("%25s", theDateBuffer);

	qtss_printf("\n");
}

bool PrintHeader(UInt32 loopCount)
{
	return ((loopCount % (sStatusUpdateInterval * 10)) == 0) ? true : false;
}

bool PrintLine(UInt32 loopCount)
{
	return ((loopCount % sStatusUpdateInterval) == 0) ? true : false;
}

void RunServer()
{
	bool restartServer = false;
	UInt32 loopCount = 0;
	UInt32 debugLevel = 0;
	bool printHeader = false;
	bool printStatus = false;

	UInt32 num = 0;//add
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
		//add,redis,定时保活
		num++;
		if (num % 5 == 0)
		{
			num = 0;
			UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kRedisTTLRole);
			for (UInt32 currentModule = 0; currentModule < numModules; currentModule++)
			{
				QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kRedisTTLRole, currentModule);
				(void)theModule->CallDispatch(Easy_RedisTTL_Role, nullptr);
			}
		}
		//
		LogStatus(theServerState);

		if (sStatusUpdateInterval)
		{
			debugLevel = sServer->GetDebugLevel();
			printHeader = PrintHeader(loopCount);
			printStatus = PrintLine(loopCount);

			if (printStatus)
			{
				if (DebugOn(sServer)) // debug level display or logging is on
					DebugStatus(debugLevel, printHeader);

				if (!DebugDisplayOn(sServer))
					PrintStatus(printHeader); // default status output
			}
			
			loopCount++;

		}

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
