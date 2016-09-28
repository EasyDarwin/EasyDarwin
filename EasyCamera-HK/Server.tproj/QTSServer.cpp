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
	 File:       QTSServer.cpp
	 Contains:   Implements object defined in QTSServer.h
 */


#ifndef __Win32__
#include <sys/types.h>
#include <dirent.h>
#endif
#include <errno.h>

#ifndef __Win32__
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#endif

#include "QTSServer.h"

#include "OSMemory.h"
#include "TCPListenerSocket.h"
#include "Task.h"

#include "QTSS_Private.h"
#include "QTSSCallbacks.h"
#include "QTSSModuleUtils.h"

 //Compile time modules
#include "QTSSErrorLogModule.h"
#include "EasyCMSModule.h"
#include "EasyCameraModule.h"

#include "QTSSFile.h"
#include "OS.h"

// CLASS DEFINITIONS
QTSS_Callbacks  QTSServer::sCallbacks;
XMLPrefsParser* QTSServer::sPrefsSource = NULL;
PrefsSource*    QTSServer::sMessagesSource = NULL;


QTSServer::~QTSServer()
{
	//
	// Grab the server mutex. This is to make sure all gets & set values on this
	// object complete before we start deleting stuff
	OSMutexLocker* serverlocker = new OSMutexLocker(this->GetServerObjectMutex());

	//
	// Grab the prefs mutex. This is to make sure we can't reread prefs
	// WHILE shutting down, which would cause some weirdness for QTSS API
	// (some modules could get QTSS_RereadPrefs_Role after QTSS_Shutdown, which would be bad)
	OSMutexLocker* locker = new OSMutexLocker(this->GetPrefs()->GetMutex());

	QTSS_ModuleState theModuleState;
	theModuleState.curRole = QTSS_Shutdown_Role;
	theModuleState.curTask = NULL;
	OSThread::SetMainThreadData(&theModuleState);

	for (UInt32 x = 0; x < QTSServerInterface::GetNumModulesInRole(QTSSModule::kShutdownRole); x++)
		(void)QTSServerInterface::GetModule(QTSSModule::kShutdownRole, x)->CallDispatch(QTSS_Shutdown_Role, NULL);

	OSThread::SetMainThreadData(NULL);

	delete fSrvrMessages;
	delete locker;
	delete serverlocker;
	delete fSrvrPrefs;
}

Bool16 QTSServer::Initialize(XMLPrefsParser* inPrefsSource, PrefsSource* inMessagesSource)
{
	fServerState = qtssFatalErrorState;
	sPrefsSource = inPrefsSource;
	sMessagesSource = inMessagesSource;
	this->InitCallbacks();

	//
	// DICTIONARY INITIALIZATION

	QTSSModule::Initialize();
	QTSServerPrefs::Initialize();
	QTSSMessages::Initialize();
	QTSSFile::Initialize();

	// STUB SERVER INITIALIZATION
	//
	// Construct stub versions of the prefs and messages dictionaries. We need
	// both of these to initialize the server, but they have to be stubs because
	// their QTSSDictionaryMaps will presumably be modified when modules get loaded.

	fSrvrPrefs = new QTSServerPrefs(inPrefsSource, false); // First time, don't write changes to the prefs file
	fSrvrMessages = new QTSSMessages(inMessagesSource);
	QTSSModuleUtils::Initialize(fSrvrMessages, this, QTSServerInterface::GetErrorLogStream());

	SetAssertLogger(this->GetErrorLogStream());// the error log stream is our assert logger

	// Load ERROR LOG module only. This is good in case there is a startup error.

	QTSSModule* theLoggingModule = new QTSSModule("QTSSErrorLogModule");
	(void)theLoggingModule->SetupModule(&sCallbacks, &QTSSErrorLogModule_Main);
	(void)AddModule(theLoggingModule);
	this->BuildModuleRoleArrays();

	//
	// STARTUP TIME - record it
	fStartupTime_UnixMilli = OS::Milliseconds();
	fGMTOffset = OS::GetGMTOffset();

	fServerState = qtssStartingUpState;
	return true;
}

void QTSServer::InitModules(QTSS_ServerState inEndState)
{
	//
	// LOAD AND INITIALIZE ALL MODULES

	// temporarily set the verbosity on missing prefs when starting up to debug level
	// This keeps all the pref messages being written to the config file from being logged.
	// don't exit until the verbosity level is reset back to the initial prefs.
	LoadCompiledInModules();
	this->BuildModuleRoleArrays();

	fSrvrPrefs->SetErrorLogVerbosity(qtssWarningVerbosity); // turn off info messages while initializing compiled in modules.
   //
	// CREATE MODULE OBJECTS AND READ IN MODULE PREFS

	// Finish setting up modules. Create our final prefs & messages objects,
	// register all global dictionaries, and invoke the modules in their Init roles.
	fStubSrvrPrefs = fSrvrPrefs;
	fStubSrvrMessages = fSrvrMessages;

	fSrvrPrefs = new QTSServerPrefs(sPrefsSource, true); // Now write changes to the prefs file. First time, we don't because the error messages won't get printed.
	QTSS_ErrorVerbosity serverLevel = fSrvrPrefs->GetErrorLogVerbosity(); // get the real prefs verbosity and save it.
	fSrvrPrefs->SetErrorLogVerbosity(qtssWarningVerbosity); // turn off info messages while loading dynamic modules


	fSrvrMessages = new QTSSMessages(sMessagesSource);
	QTSSModuleUtils::Initialize(fSrvrMessages, this, QTSServerInterface::GetErrorLogStream());

	this->SetVal(qtssSvrMessages, &fSrvrMessages, sizeof(fSrvrMessages));
	this->SetVal(qtssSvrPreferences, &fSrvrPrefs, sizeof(fSrvrPrefs));

	//
	// ADD REREAD PREFERENCES SERVICE
	(void)QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kServiceDictIndex)->
		AddAttribute(QTSS_REREAD_PREFS_SERVICE, (QTSS_AttrFunctionPtr)QTSServer::RereadPrefsService, qtssAttrDataTypeUnknown, qtssAttrModeRead);

	//
	// INVOKE INITIALIZE ROLE
	this->DoInitRole();

	if (fServerState != qtssFatalErrorState)
		fServerState = inEndState; // Server is done starting up!   


	fSrvrPrefs->SetErrorLogVerbosity(serverLevel); // reset the server's verbosity back to the original prefs level.
}

void QTSServer::StartTasks()
{
	// Start listening
	for (UInt32 x = 0; x < fNumListeners; x++)
		fListeners[x]->RequestEvent(EV_RE);
}

void    QTSServer::LoadCompiledInModules()
{
#ifndef DSS_DYNAMIC_MODULES_ONLY
	// MODULE DEVELOPERS SHOULD ADD THE FOLLOWING THREE LINES OF CODE TO THIS
	// FUNCTION IF THEIR MODULE IS BEING COMPILED INTO THE SERVER.
	//
	// QTSSModule* myModule = new QTSSModule("__MODULE_NAME__");
	// (void)myModule->Initialize(&sCallbacks, &__MODULE_MAIN_ROUTINE__);
	// (void)AddModule(myModule);
	//
	// The following modules are all compiled into the server. 

#endif //DSS_DYNAMIC_MODULES_ONLY

	QTSSModule* theCameraModule = new QTSSModule("EasyCameraModule");
	(void)theCameraModule->SetupModule(&sCallbacks, &EasyCameraModule_Main);
	(void)AddModule(theCameraModule);

	QTSSModule* theCMSModule = new QTSSModule("EasyCMSModule");
	(void)theCMSModule->SetupModule(&sCallbacks, &EasyCMSModule_Main);
	(void)AddModule(theCMSModule);

}

void    QTSServer::InitCallbacks()
{
	sCallbacks.addr[kNewCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_New;
	sCallbacks.addr[kDeleteCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_Delete;
	sCallbacks.addr[kMillisecondsCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_Milliseconds;
	sCallbacks.addr[kConvertToUnixTimeCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_ConvertToUnixTime;

	sCallbacks.addr[kAddRoleCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_AddRole;
	sCallbacks.addr[kCreateObjectTypeCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_CreateObjectType;
	sCallbacks.addr[kAddAttributeCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_AddAttribute;
	sCallbacks.addr[kIDForTagCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_IDForAttr;
	sCallbacks.addr[kGetAttributePtrByIDCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_GetValuePtr;
	sCallbacks.addr[kGetAttributeByIDCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_GetValue;
	sCallbacks.addr[kSetAttributeByIDCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_SetValue;
	sCallbacks.addr[kCreateObjectValueCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_CreateObject;
	sCallbacks.addr[kGetNumValuesCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_GetNumValues;

	sCallbacks.addr[kWriteCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_Write;
	sCallbacks.addr[kWriteVCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_WriteV;
	sCallbacks.addr[kFlushCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_Flush;
	sCallbacks.addr[kReadCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_Read;
	sCallbacks.addr[kSeekCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_Seek;
	sCallbacks.addr[kAdviseCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_Advise;

	sCallbacks.addr[kAddServiceCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_AddService;
	sCallbacks.addr[kIDForServiceCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_IDForService;
	sCallbacks.addr[kDoServiceCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_DoService;

	sCallbacks.addr[kRequestEventCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_RequestEvent;
	sCallbacks.addr[kSetIdleTimerCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_SetIdleTimer;
	sCallbacks.addr[kSignalStreamCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_SignalStream;

	sCallbacks.addr[kOpenFileObjectCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_OpenFileObject;
	sCallbacks.addr[kCloseFileObjectCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_CloseFileObject;

	sCallbacks.addr[kCreateSocketStreamCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_CreateStreamFromSocket;
	sCallbacks.addr[kDestroySocketStreamCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_DestroySocketStream;

	sCallbacks.addr[kAddStaticAttributeCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_AddStaticAttribute;
	sCallbacks.addr[kAddInstanceAttributeCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_AddInstanceAttribute;
	sCallbacks.addr[kRemoveInstanceAttributeCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_RemoveInstanceAttribute;

	sCallbacks.addr[kGetAttrInfoByIndexCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_GetAttrInfoByIndex;
	sCallbacks.addr[kGetAttrInfoByNameCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_GetAttrInfoByName;
	sCallbacks.addr[kGetAttrInfoByIDCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_GetAttrInfoByID;
	sCallbacks.addr[kGetNumAttributesCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_GetNumAttributes;


	sCallbacks.addr[kGetValueAsStringCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_GetValueAsString;
	sCallbacks.addr[kTypeToTypeStringCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_TypeToTypeString;
	sCallbacks.addr[kTypeStringToTypeCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_TypeStringToType;
	sCallbacks.addr[kStringToValueCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_StringToValue;
	sCallbacks.addr[kValueToStringCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_ValueToString;

	sCallbacks.addr[kRemoveValueCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_RemoveValue;

	sCallbacks.addr[kRequestGlobalLockCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_RequestLockedCallback;
	sCallbacks.addr[kIsGlobalLockedCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_IsGlobalLocked;
	sCallbacks.addr[kUnlockGlobalLock] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_UnlockGlobalLock;

	sCallbacks.addr[kLockObjectCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_LockObject;
	sCallbacks.addr[kUnlockObjectCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_UnlockObject;
	sCallbacks.addr[kSetAttributePtrCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_SetValuePtr;

	sCallbacks.addr[kSetIntervalRoleTimerCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_SetIdleRoleTimer;

	sCallbacks.addr[kLockStdLibCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_LockStdLib;
	sCallbacks.addr[kUnlockStdLibCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_UnlockStdLib;

}

Bool16 QTSServer::AddModule(QTSSModule* inModule)
{
	Assert(inModule->IsInitialized());

	// Prepare to invoke the module's Register role. Setup the Register param block
	QTSS_ModuleState theModuleState;

	theModuleState.curModule = inModule;
	theModuleState.curRole = QTSS_Register_Role;
	theModuleState.curTask = NULL;
	OSThread::SetMainThreadData(&theModuleState);

	// Currently we do nothing with the module name
	QTSS_RoleParams theRegParams;
	theRegParams.regParams.outModuleName[0] = 0;

	// If the module returns an error from the QTSS_Register role, don't put it anywhere
	if (inModule->CallDispatch(QTSS_Register_Role, &theRegParams) != QTSS_NoErr)
		return false;

	OSThread::SetMainThreadData(NULL);

	//
	// Update the module name to reflect what was returned from the register role
	theRegParams.regParams.outModuleName[QTSS_MAX_MODULE_NAME_LENGTH - 1] = 0;
	if (theRegParams.regParams.outModuleName[0] != 0)
		inModule->SetValue(qtssModName, 0, theRegParams.regParams.outModuleName, ::strlen(theRegParams.regParams.outModuleName), false);

	//
	// Give the module object a prefs dictionary. Instance attributes are allowed for these objects.
	QTSSPrefs* thePrefs = NEW QTSSPrefs(sPrefsSource, inModule->GetValue(qtssModName), QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kModulePrefsDictIndex), true);
	thePrefs->RereadPreferences();
	inModule->SetPrefsDict(thePrefs);

	//
	// Add this module to the array of module (dictionaries)
	UInt32 theNumModules = this->GetNumValues(qtssSvrModuleObjects);
	QTSS_Error theErr = this->SetValue(qtssSvrModuleObjects, theNumModules, &inModule, sizeof(QTSSModule*), QTSSDictionary::kDontObeyReadOnly);
	Assert(theErr == QTSS_NoErr);

	//
	// Add this module to the module queue
	sModuleQueue.EnQueue(inModule->GetQueueElem());

	return true;
}

void QTSServer::BuildModuleRoleArrays()
{
	OSQueueIter theIter(&sModuleQueue);
	QTSSModule* theModule = NULL;

	// Make sure these variables are cleaned out in case they've already been inited.

	DestroyModuleRoleArrays();

	// Loop through all the roles of all the modules, recording the number of
	// modules in each role, and also recording which modules are doing what.

	for (UInt32 x = 0; x < QTSSModule::kNumRoles; x++)
	{
		sNumModulesInRole[x] = 0;
		for (theIter.Reset(); !theIter.IsDone(); theIter.Next())
		{
			theModule = (QTSSModule*)theIter.GetCurrent()->GetEnclosingObject();
			if (theModule->RunsInRole(x))
				sNumModulesInRole[x] += 1;
		}

		if (sNumModulesInRole[x] > 0)
		{
			UInt32 moduleIndex = 0;
			sModuleArray[x] = new QTSSModule*[sNumModulesInRole[x] + 1];
			for (theIter.Reset(); !theIter.IsDone(); theIter.Next())
			{
				theModule = (QTSSModule*)theIter.GetCurrent()->GetEnclosingObject();
				if (theModule->RunsInRole(x))
				{
					sModuleArray[x][moduleIndex] = theModule;
					moduleIndex++;
				}
			}
		}
	}
}

void QTSServer::DestroyModuleRoleArrays()
{
	for (UInt32 x = 0; x < QTSSModule::kNumRoles; x++)
	{
		sNumModulesInRole[x] = 0;
		if (sModuleArray[x] != NULL)
			delete[] sModuleArray[x];
		sModuleArray[x] = NULL;
	}
}

void QTSServer::DoInitRole()
{
	QTSS_RoleParams theInitParams;
	theInitParams.initParams.inServer = this;
	theInitParams.initParams.inPrefs = fSrvrPrefs;
	theInitParams.initParams.inMessages = fSrvrMessages;
	theInitParams.initParams.inErrorLogStream = &sErrorLogStream;

	QTSS_ModuleState theModuleState;
	theModuleState.curRole = QTSS_Initialize_Role;
	theModuleState.curTask = NULL;
	OSThread::SetMainThreadData(&theModuleState);

	for (UInt32 x = 0; x < QTSServerInterface::GetNumModulesInRole(QTSSModule::kInitializeRole); x++)
	{
		QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kInitializeRole, x);
		theInitParams.initParams.inModule = theModule;
		theModuleState.curModule = theModule;
		QTSS_Error theErr = theModule->CallDispatch(QTSS_Initialize_Role, &theInitParams);

		if (theErr != QTSS_NoErr)
		{
			// If the module reports an error when initializing itself,
			// delete the module and pretend it was never there.
			QTSSModuleUtils::LogError(qtssWarningVerbosity, qtssMsgInitFailed, theErr,
				theModule->GetValue(qtssModName)->Ptr);

			sModuleQueue.Remove(theModule->GetQueueElem());
			delete theModule;
		}
	}

	OSThread::SetMainThreadData(NULL);
}

QTSS_Error QTSServer::RereadPrefsService(QTSS_ServiceFunctionArgsPtr /*inArgs*/)
{
	//
	// This function can only be called safely when the server is completely running.
	// Ensuring this is a bit complicated because of preemption. Here's how it's done...

	QTSServerInterface* theServer = QTSServerInterface::GetServer();

	// This is to make sure this function isn't being called before the server is
	// completely started up.
	if ((theServer == NULL) || (theServer->GetServerState() != qtssRunningState))
		return QTSS_OutOfState;

	// Because the server must have started up, and because this object always stays
	// around (until the process dies), we can now safely get this object.
	QTSServerPrefs* thePrefs = theServer->GetPrefs();

	// Grab the prefs mutex. We want to make sure that calls to RereadPrefsService
	// are serialized. This also prevents the server from shutting down while in
	// this function, because the QTSServer destructor grabs this mutex as well.
	OSMutexLocker locker(thePrefs->GetMutex());

	// Finally, check the server state again. The state may have changed
	// to qtssShuttingDownState or qtssFatalErrorState in this time, though
	// at this point we have the prefs mutex, so we are guarenteed that the
	// server can't actually shut down anymore
	if (theServer->GetServerState() != qtssRunningState)
		return QTSS_OutOfState;

	// Ok, we're ready to reread preferences now.

	//
	// Reread preferences
	sPrefsSource->Parse();
	thePrefs->RereadServerPreferences(true);

	// Delete all the streams
	QTSSModule** theModule = NULL;
	UInt32 theLen = 0;

	for (int y = 0; QTSServerInterface::GetServer()->GetValuePtr(qtssSvrModuleObjects, y, (void**)&theModule, &theLen) == QTSS_NoErr; y++)
	{
		Assert(theModule != NULL);
		Assert(theLen == sizeof(QTSSModule*));

		(*theModule)->GetPrefsDict()->RereadPreferences();

#if DEBUG
		theModule = NULL;
		theLen = 0;
#endif
	}

	//
	// Go through each module's prefs object and have those reread as well

	//
	// Now that we are done rereading the prefs, invoke all modules in the RereadPrefs
	// role so they can update their internal prefs caches.
	for (UInt32 x = 0; x < QTSServerInterface::GetNumModulesInRole(QTSSModule::kRereadPrefsRole); x++)
	{
		QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kRereadPrefsRole, x);
		(void)theModule->CallDispatch(QTSS_RereadPrefs_Role, NULL);
	}
	return QTSS_NoErr;
}


