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
	 File:       win32main.cpp
	 Contains:   main function to drive streaming server on win32.
 */

#include "getopt.h"
#include "FilePrefsSource.h"

#include "RunServer.h"
#include "QTSServer.h"
#include "QTSSExpirationDate.h"
#include "GenerateXMLPrefs.h"

 //
 // Data
static FilePrefsSource sPrefsSource(true); // Allow dups
static XMLPrefsParser* sXMLParser = NULL;
static FilePrefsSource sMessagesSource;
static int sStatsUpdateInterval = 0;
static SERVICE_STATUS_HANDLE sServiceStatusHandle = 0;
static QTSS_ServerState sInitialState = qtssRunningState;

//
// Functions
static void ReportStatus(DWORD inCurrentState, DWORD inExitCode);
static void InstallService(char* inServiceName);
static void RemoveService(char *inServiceName);
static void RunAsService(char* inServiceName);
void WINAPI ServiceControl(DWORD);
void WINAPI ServiceMain(DWORD argc, LPTSTR *argv);

int main(int argc, char * argv[])
{
	extern char* optarg;

	//First thing to do is to read command-line arguments.
	int ch;

	char* theConfigFilePath = "./easycamera.cfg";
	char* theXMLFilePath = "./easycamera.xml";
	Bool16 notAService = false;
	Bool16 theXMLPrefsExist = true;
	Bool16 dontFork = false;

#if _DEBUG
	char* compileType = "Compile_Flags/_DEBUG; ";
#else
	char* compileType = "";
#endif

	qtss_printf("%s/%s ( Build/%s; Platform/%s; %s%s) Built on: %s\n", QTSServerInterface::GetServerName().Ptr,
		QTSServerInterface::GetServerVersion().Ptr,
		QTSServerInterface::GetServerBuild().Ptr,
		QTSServerInterface::GetServerPlatform().Ptr,
		compileType,
		QTSServerInterface::GetServerComment().Ptr,
		QTSServerInterface::GetServerBuildDate().Ptr);

	while ((ch = getopt(argc, argv, "vdxp:o:c:irsS:I")) != EOF) // opt: means requires option
	{
		switch (ch)
		{
		case 'v':

			qtss_printf("%s/%s ( Build/%s; Platform/%s; %s%s) Built on: %s\n", QTSServerInterface::GetServerName().Ptr,
				QTSServerInterface::GetServerVersion().Ptr,
				QTSServerInterface::GetServerBuild().Ptr,
				QTSServerInterface::GetServerPlatform().Ptr,
				compileType,
				QTSServerInterface::GetServerComment().Ptr,
				QTSServerInterface::GetServerBuildDate().Ptr);

			qtss_printf("usage: %s [ -d | -p port | -v | -c /myconfigpath.xml | -o /myconfigpath.conf | -x | -S numseconds | -I | -h ]\n", QTSServerInterface::GetServerName().Ptr);
			qtss_printf("-d: Don't run as a Win32 Service\n");
			qtss_printf("-p XXX: Specify the default RTSP listening port of the server\n");
			qtss_printf("-c c:\\myconfigpath.xml: Specify a config file path\n");
			qtss_printf("-o c:\\myconfigpath.conf: Specify a DSS 1.x / 2.x config file path\n");
			qtss_printf("-x: Force create new .xml config file from 1.x / 2.x config\n");
			qtss_printf("-i: Install the EasyCamera service\n");
			qtss_printf("-r: Remove the EasyCamera service\n");
			qtss_printf("-s: Start the EasyCamera service\n");
			qtss_printf("-S n: Display server stats in the console every \"n\" seconds\n");
			qtss_printf("-I: Start the server in the idle state\n");
			::exit(0);
		case 'd':
			notAService = true;
			break;
		case 'c':
			Assert(optarg != NULL);// this means we didn't declare getopt options correctly or there is a bug in getopt.
			theXMLFilePath = optarg;
			break;
		case 'S':
			Assert(optarg != NULL);// this means we didn't declare getopt options correctly or there is a bug in getopt.
			sStatsUpdateInterval = ::atoi(optarg);
			break;
		case 'o':
			Assert(optarg != NULL);// this means we didn't declare getopt options correctly or there is a bug in getopt.
			theConfigFilePath = optarg;
			break;
		case 'x':
			theXMLPrefsExist = false; // Force us to generate a new XML prefs file
			break;
		case 'i':
			qtss_printf("Installing the EasyCamera service...\n");
			::InstallService("EasyCamera");
			qtss_printf("Starting the EasyCamera service...\n");
			::RunAsService("EasyCamera");
			::exit(0);
			break;
		case 'r':
			qtss_printf("Removing the EasyCamera service...\n");
			::RemoveService("EasyCamera");
			::exit(0);
		case 's':
			qtss_printf("Starting the EasyCamera service...\n");
			::RunAsService("EasyCamera");
			::exit(0);
		case 'I':
			sInitialState = qtssIdleState;
			break;
		default:
			break;
		}
	}

	//
	// Check expiration date

	QTSSExpirationDate::PrintExpirationDate();
	if (QTSSExpirationDate::IsSoftwareExpired())
	{
		qtss_printf("EasyCamera has expired\n");
		::exit(0);
	}

	//
	// Create an XML prefs parser object using the specified path
	sXMLParser = new XMLPrefsParser(theXMLFilePath);

	//
	// Check to see if the XML file exists as a directory. If it does,
	// just bail because we do not want to overwrite a directory
	if (sXMLParser->DoesFileExistAsDirectory())
	{
		qtss_printf("Directory located at location where streaming server prefs file should be.\n");
		::exit(0);
	}

	if (!sXMLParser->CanWriteFile())
	{
		qtss_printf("Cannot write to the streaming server prefs file.\n");
		::exit(0);
	}

	// If we aren't forced to create a new XML prefs file, whether
	// we do or not depends solely on whether the XML prefs file exists currently.
	if (theXMLPrefsExist)
		theXMLPrefsExist = sXMLParser->DoesFileExist();

	if (!theXMLPrefsExist)
	{
		//
		//Construct a Prefs Source object to get server preferences

		int prefsErr = sPrefsSource.InitFromConfigFile(theConfigFilePath);
		if (prefsErr)
			qtss_printf("Could not load configuration file at %s.\n Generating a new prefs file at %s\n", theConfigFilePath, theXMLFilePath);

		//
		// Generate a brand-new XML prefs file out of the old prefs
		int xmlGenerateErr = GenerateAllXMLPrefs(&sPrefsSource, sXMLParser);
		if (xmlGenerateErr)
		{
			qtss_printf("Fatal Error: Could not create new prefs file at: %s. (%d)\n", theConfigFilePath, OSThread::GetErrno());
			::exit(-1);
		}
	}

	//
	// Parse the configs from the XML file
	int xmlParseErr = sXMLParser->Parse();
	if (xmlParseErr)
	{
		qtss_printf("Fatal Error: Could not load configuration file at %s. (%d)\n", theXMLFilePath, OSThread::GetErrno());
		::exit(-1);
	}

	//
	// Construct a messages source object
	sMessagesSource.InitFromConfigFile("qtssmessages.txt");

	//
	// Start Win32 DLLs
	WORD wsVersion = MAKEWORD(1, 1);
	WSADATA wsData;
	(void)::WSAStartup(wsVersion, &wsData);

	if (notAService)
	{
		// If we're running off the command-line, don't do the service initiation crap.
		::StartServer(sXMLParser, &sMessagesSource, sStatsUpdateInterval, sInitialState, false, 0, kRunServerDebug_Off); // No stats update interval for now
		::RunServer();
		::exit(0);
	}

	SERVICE_TABLE_ENTRY dispatchTable[] =
	{
		{ "", ServiceMain },
		{ NULL, NULL }
	};

	//
	// In case someone runs the server improperly, print out a friendly message.
	qtss_printf("EasyCamera must either be started from the DOS Console\n");
	qtss_printf("using the -d command-line option, or using the Service Control Manager\n\n");
	qtss_printf("Waiting for the Service Control Manager to start EasyCamera...\n");
	BOOL theErr = ::StartServiceCtrlDispatcher(dispatchTable);
	if (!theErr)
	{
		qtss_printf("Fatal Error: Couldn't start Service\n");
		::exit(-1);
	}

	return (0);
}

void __stdcall ServiceMain(DWORD /*argc*/, LPTSTR *argv)
{
	char* theServerName = argv[0];

	sServiceStatusHandle = ::RegisterServiceCtrlHandler(theServerName, &ServiceControl);
	if (sServiceStatusHandle == 0)
	{
		qtss_printf("Failure registering service handler");
		return;
	}

	//
	// Report our status
	::ReportStatus(SERVICE_START_PENDING, NO_ERROR);


	//
	// Start & Run the server - no stats update interval for now
	if (::StartServer(sXMLParser, &sMessagesSource, sStatsUpdateInterval, sInitialState, false, 0, kRunServerDebug_Off) != qtssFatalErrorState)
	{
		::ReportStatus(SERVICE_RUNNING, NO_ERROR);
		::RunServer(); // This function won't return until the server has died

		//
		// Ok, server is done...
		::ReportStatus(SERVICE_STOPPED, NO_ERROR);
	}
	else
		::ReportStatus(SERVICE_STOPPED, ERROR_BAD_COMMAND); // I dunno... report some error

}

void WINAPI ServiceControl(DWORD inControlCode)
{
	QTSS_ServerState theState;
	QTSServerInterface* theServer = QTSServerInterface::GetServer();
	DWORD theStatusReport = SERVICE_START_PENDING;

	if (theServer != NULL)
		theState = theServer->GetServerState();
	else
		theState = qtssStartingUpState;

	switch (inControlCode)
	{
		// Stop the service.
		//
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		{
			if (theState == qtssStartingUpState)
				break;

			//
			// Signal the server to shut down.
			theState = qtssShuttingDownState;
			if (theServer != NULL)
				theServer->SetValue(qtssSvrState, 0, &theState, sizeof(theState));
			break;
		}
	case SERVICE_CONTROL_PAUSE:
		{
			if (theState != qtssRunningState)
				break;

			//
			// Signal the server to refuse new connections.
			theState = qtssRefusingConnectionsState;
			if (theServer != NULL)
				theServer->SetValue(qtssSvrState, 0, &theState, sizeof(theState));
			break;
		}
	case SERVICE_CONTROL_CONTINUE:
		{
			if (theState != qtssRefusingConnectionsState)
				break;

			//
			// Signal the server to refuse new connections.
			theState = qtssRefusingConnectionsState;
			if (theServer != NULL)
				theServer->SetValue(qtssSvrState, 0, &theState, sizeof(theState));
			break;
		}
	case SERVICE_CONTROL_INTERROGATE:
		break; // Just update our status

	default:
		break;
	}

	if (theServer != NULL)
	{
		theState = theServer->GetServerState();

		// Convert a QTSS state to a Win32 Service state
		switch (theState)
		{
		case qtssStartingUpState:           theStatusReport = SERVICE_START_PENDING;    break;
		case qtssRunningState:              theStatusReport = SERVICE_RUNNING;          break;
		case qtssRefusingConnectionsState:  theStatusReport = SERVICE_PAUSED;           break;
		case qtssFatalErrorState:           theStatusReport = SERVICE_STOP_PENDING;     break;
		case qtssShuttingDownState:         theStatusReport = SERVICE_STOP_PENDING;     break;
		default:                            theStatusReport = SERVICE_RUNNING;          break;
		}
	}
	else
		theStatusReport = SERVICE_START_PENDING;

	qtss_printf("Reporting status from ServiceControl function\n");
	::ReportStatus(theStatusReport, NO_ERROR);
}

void ReportStatus(DWORD inCurrentState, DWORD inExitCode)
{
	static Bool16 sFirstTime = 1;
	static UInt32 sCheckpoint = 0;
	static SERVICE_STATUS sStatus;

	if (sFirstTime)
	{
		sFirstTime = false;

		//
		// Setup the status structure
		sStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
		sStatus.dwCurrentState = SERVICE_START_PENDING;
		//sStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SHUTDOWN;
		sStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
		sStatus.dwWin32ExitCode = 0;
		sStatus.dwServiceSpecificExitCode = 0;
		sStatus.dwCheckPoint = 0;
		sStatus.dwWaitHint = 0;
	}

	if (sStatus.dwCurrentState == SERVICE_START_PENDING)
		sStatus.dwCheckPoint = ++sCheckpoint;
	else
		sStatus.dwCheckPoint = 0;

	sStatus.dwCurrentState = inCurrentState;
	sStatus.dwServiceSpecificExitCode = inExitCode;
	BOOL theErr = SetServiceStatus(sServiceStatusHandle, &sStatus);
	if (theErr == 0)
	{
		DWORD theerrvalue = ::GetLastError();
	}
}

void RunAsService(char* inServiceName)
{
	SC_HANDLE   theService;
	SC_HANDLE   theSCManager;

	theSCManager = ::OpenSCManager(
		NULL,                   // machine (NULL == local)
		NULL,                   // database (NULL == default)
		SC_MANAGER_ALL_ACCESS   // access required
	);
	if (!theSCManager)
		return;

	theService = ::OpenService(
		theSCManager,               // SCManager database
		inServiceName,               // name of service
		SERVICE_ALL_ACCESS);

	SERVICE_STATUS lpServiceStatus;

	if (theService)
	{
		const SInt32 kNotRunning = 1062;
		Bool16 stopped = ::ControlService(theService, SERVICE_CONTROL_STOP, &lpServiceStatus);
		if (!stopped && ((SInt32) ::GetLastError() != kNotRunning))
			qtss_printf("Stopping Service Error: %d\n", ::GetLastError());

		Bool16 started = ::StartService(theService, 0, NULL);
		if (!started)
			qtss_printf("Starting Service Error: %d\n", ::GetLastError());

		::CloseServiceHandle(theService);
	}

	::CloseServiceHandle(theSCManager);
}

void InstallService(char* inServiceName)
{
	SC_HANDLE   theService;
	SC_HANDLE   theSCManager;

	TCHAR thePath[512];
	TCHAR theQuotedPath[522];

	BOOL theErr = ::GetModuleFileName(NULL, thePath, 512);
	if (!theErr)
		return;

	qtss_sprintf(theQuotedPath, "\"%s\"", thePath);

	theSCManager = ::OpenSCManager(
		NULL,                   // machine (NULL == local)
		NULL,                   // database (NULL == default)
		SC_MANAGER_ALL_ACCESS   // access required
	);
	if (!theSCManager)
	{
		qtss_printf("Failed to install EasyCamera Service\n");
		return;
	}

	theService = CreateService(
		theSCManager,               // SCManager database
		inServiceName,               // name of service
		inServiceName,               // name to display
		SERVICE_ALL_ACCESS,         // desired access
		SERVICE_WIN32_OWN_PROCESS,  // service type
		SERVICE_AUTO_START,       // start type
		SERVICE_ERROR_NORMAL,       // error control type
		theQuotedPath,               // service's binary
		NULL,                       // no load ordering group
		NULL,                       // no tag identifier
		NULL,       // dependencies
		NULL,                       // LocalSystem account
		NULL);                      // no password

	if (theService)
	{
		::CloseServiceHandle(theService);
		qtss_printf("Installed EasyCamera Service\n");
	}
	else
		qtss_printf("Failed to install EasyCamera Service\n");

	::CloseServiceHandle(theSCManager);
}

void RemoveService(char *inServiceName)
{
	SC_HANDLE   theSCManager;
	SC_HANDLE   theService;

	theSCManager = ::OpenSCManager(
		NULL,                   // machine (NULL == local)
		NULL,                   // database (NULL == default)
		SC_MANAGER_ALL_ACCESS   // access required
	);
	if (!theSCManager)
	{
		qtss_printf("Failed to remove EasyCamera Service\n");
		return;
	}

	theService = ::OpenService(theSCManager, inServiceName, SERVICE_ALL_ACCESS);
	if (theService != NULL)
	{
		Bool16 stopped = ::ControlService(theService, SERVICE_CONTROL_STOP, NULL);
		if (!stopped)
			qtss_printf("Stopping Service Error: %d\n", ::GetLastError());

		(void)::DeleteService(theService);
		::CloseServiceHandle(theService);
		qtss_printf("Removed EasyCamera Service\n");
	}
	else
		qtss_printf("Failed to remove EasyCamera Service\n");

	::CloseServiceHandle(theSCManager);
}