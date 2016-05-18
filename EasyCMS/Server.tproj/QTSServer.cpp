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
#include "OSArrayObjectDeleter.h"
#include "SocketUtils.h"
#include "TCPListenerSocket.h"
#include "Task.h"

#include "QTSS_Private.h"
#include "QTSSCallbacks.h"
#include "QTSSModuleUtils.h"

//具体上层协议类
#include "HTTPSessionInterface.h"
#include "HTTPSession.h"
#include "QTSSFile.h"

//Compile time modules
#include "QTSSErrorLogModule.h"
#include "QTSSAuthModule.h"

// CLASS DEFINITIONS
class HTTPListenerSocket : public TCPListenerSocket
{
    public:
		HTTPListenerSocket() {}
        virtual ~HTTPListenerSocket() {}
        
        //获取处理端口事务的具体Task
        virtual Task*   GetSessionTask(TCPSocket** outSocket);
        
        //检测是否超过了最大处理负荷
        Bool16 OverMaxConnections(UInt32 buffer);
};


QTSS_Callbacks  QTSServer::sCallbacks;
XMLPrefsParser* QTSServer::sPrefsSource = NULL;
PrefsSource*    QTSServer::sMessagesSource = NULL;

QTSServer::~QTSServer()
{
    // Grab the server mutex. This is to make sure all gets & set values on this
    // object complete before we start deleting stuff
    OSMutexLocker* serverlocker = new OSMutexLocker(this->GetServerObjectMutex());
    
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

Bool16 QTSServer::Initialize(XMLPrefsParser* inPrefsSource, PrefsSource* inMessagesSource, UInt16 inPortOverride, Bool16 createListeners)
{
    static const UInt32 kDeviceSessionMapSize = 10000;
    fServerState = qtssFatalErrorState;
    sPrefsSource = inPrefsSource;
    sMessagesSource = inMessagesSource;
    this->InitCallbacks();

	//每一种类型对象字典集初始化
    QTSSModule::Initialize();
    QTSServerPrefs::Initialize();
    QTSSMessages::Initialize();
    HTTPSessionInterface::Initialize();
    QTSSFile::Initialize();
    
    //
    // STUB SERVER INITIALIZATION
    //
    // Construct stub versions of the prefs and messages dictionaries. We need
    // both of these to initialize the server, but they have to be stubs because
    // their QTSSDictionaryMaps will presumably be modified when modules get loaded.
    
    fSrvrPrefs = new QTSServerPrefs(inPrefsSource, false); // First time, don't write changes to the prefs file
    fSrvrMessages = new QTSSMessages(inMessagesSource);
    QTSSModuleUtils::Initialize(fSrvrMessages, this, QTSServerInterface::GetErrorLogStream());

    //
    // SETUP ASSERT BEHAVIOR
    //
    // Depending on the server preference, we will either break when we hit an
    // assert, or log the assert to the error log
    if (!fSrvrPrefs->ShouldServerBreakOnAssert())
        SetAssertLogger(this->GetErrorLogStream());// the error log stream is our assert logger

    //提前加载日志模块，记录Server启动过程
    QTSSModule* theLoggingModule = new QTSSModule("QTSSErrorLogModule");
    (void)theLoggingModule->SetupModule(&sCallbacks, &QTSSErrorLogModule_Main);
    (void)AddModule(theLoggingModule);

    this->BuildModuleRoleArrays();

    // DEFAULT IP ADDRESS & DNS NAME
    if (!this->SetDefaultIPAddr())
        return false;

    //
    // STARTUP TIME - record it
    fStartupTime_UnixMilli = OS::Milliseconds();
    fGMTOffset = OS::GetGMTOffset();
        
    //开始端口监听
    if (createListeners)
    {
        if ( !this->CreateListeners(false, fSrvrPrefs, inPortOverride) )
            QTSSModuleUtils::LogError(qtssWarningVerbosity, qtssMsgSomePortsFailed, 0);
    }
    
	//所有端口都无法监听，返回Server初始化失败
    if ( fNumListeners == 0 )
    {
		if (createListeners)
            QTSSModuleUtils::LogError(qtssWarningVerbosity, qtssMsgNoPortsSucceeded, 0);
        return false;
    }
	
	//for redis
	char * chTemp=fSrvrPrefs->GetRedisIP();
	fRedisIP=chTemp;
	delete[] chTemp;
	fRedisPort=fSrvrPrefs->GetRedisPort();

	chTemp=fSrvrPrefs->GetMonitorWANIP();
	fCMSIP=chTemp;
	delete[] chTemp;
	fCMSPort=fSrvrPrefs->GetMonitorWANPort();
	
	ConRedis();//连接redis
	//for redis

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
     
    LoadModules(fSrvrPrefs);
    LoadCompiledInModules();
    this->BuildModuleRoleArrays();

    fSrvrPrefs->SetErrorLogVerbosity(qtssWarningVerbosity); // turn off info messages while initializing compiled in modules.

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

Bool16 QTSServer::SetDefaultIPAddr()
{
    //check to make sure there is an available ip interface
    if (SocketUtils::GetNumIPAddrs() == 0)
    {
        QTSSModuleUtils::LogError(qtssFatalVerbosity, qtssMsgNotConfiguredForIP, 0);
        return false;
    }

    //find out what our default IP addr is & dns name
    UInt32 theNumAddrs = 0;
    UInt32* theIPAddrs = this->GetBindIPAddrs(fSrvrPrefs, &theNumAddrs);
    if (theNumAddrs == 1)
        fDefaultIPAddr = SocketUtils::GetIPAddr(0);
    else
        fDefaultIPAddr = theIPAddrs[0];
    delete [] theIPAddrs;
        
    for (UInt32 ipAddrIter = 0; ipAddrIter < SocketUtils::GetNumIPAddrs(); ipAddrIter++)
    {
        if (SocketUtils::GetIPAddr(ipAddrIter) == fDefaultIPAddr)
        {
            this->SetVal(qtssSvrDefaultDNSName, SocketUtils::GetDNSNameStr(ipAddrIter));
            Assert(this->GetValue(qtssSvrDefaultDNSName)->Ptr != NULL);
            this->SetVal(qtssSvrDefaultIPAddrStr, SocketUtils::GetIPAddrStr(ipAddrIter));
            Assert(this->GetValue(qtssSvrDefaultDNSName)->Ptr != NULL);
            break;
        }
    }
    if (this->GetValue(qtssSvrDefaultDNSName)->Ptr == NULL)
    {
        //If we've gotten here, what has probably happened is the IP address (explicitly
        //entered as a preference) doesn't exist
        QTSSModuleUtils::LogError(qtssFatalVerbosity, qtssMsgDefaultRTSPAddrUnavail, 0);
        return false;   
    }
    return true;
}               


Bool16 QTSServer::CreateListeners(Bool16 startListeningNow, QTSServerPrefs* inPrefs, UInt16 inPortOverride)
{
    struct PortTracking
    {
        PortTracking() : fPort(0), fIPAddr(0), fNeedsCreating(true) {}
        
        UInt16 fPort;
        UInt32 fIPAddr;
        Bool16 fNeedsCreating;
    };
    
    PortTracking* thePortTrackers = NULL;   
    UInt32 theTotalPortTrackers = 0;
    
    // Get the IP addresses from the pref
    UInt32 theNumAddrs = 0;
    UInt32* theIPAddrs = this->GetBindIPAddrs(inPrefs, &theNumAddrs);   
    UInt32 index = 0;
    
    if ( inPortOverride != 0)
    {
        theTotalPortTrackers = theNumAddrs; // one port tracking struct for each IP addr
        thePortTrackers = NEW PortTracking[theTotalPortTrackers];
        for (index = 0; index < theNumAddrs; index++)
        {
            thePortTrackers[index].fPort = inPortOverride;
            thePortTrackers[index].fIPAddr = theIPAddrs[index];
        }
    }
    else
    {
        UInt16 thePorts = GetServicePorts(inPrefs);
        theTotalPortTrackers = theNumAddrs;
        thePortTrackers = NEW PortTracking[theTotalPortTrackers];
        
        UInt32 currentIndex  = 0;
        
        for (index = 0; index < theNumAddrs; index++)
        {
            thePortTrackers[index].fPort = thePorts;
            thePortTrackers[index].fIPAddr = theIPAddrs[index];
        }
    }
    
        delete [] theIPAddrs;
    //
    // Now figure out which of these ports we are *already* listening on.
    // If we already are listening on that port, just move the pointer to the
    // listener over to the new array
    TCPListenerSocket** newListenerArray = NEW TCPListenerSocket*[theTotalPortTrackers];
    UInt32 curPortIndex = 0;
    
    for (UInt32 count = 0; count < theTotalPortTrackers; count++)
    {
        for (UInt32 count2 = 0; count2 < fNumListeners; count2++)
        {
            if ((fListeners[count2]->GetLocalPort() == thePortTrackers[count].fPort) &&
                (fListeners[count2]->GetLocalAddr() == thePortTrackers[count].fIPAddr))
            {
                thePortTrackers[count].fNeedsCreating = false;
                newListenerArray[curPortIndex++] = fListeners[count2];
                Assert(curPortIndex <= theTotalPortTrackers);
                break;
            }
        }
    }
    
    //
    // Create any new listeners we need
    for (UInt32 count3 = 0; count3 < theTotalPortTrackers; count3++)
    {
        if (thePortTrackers[count3].fNeedsCreating)
        {
            newListenerArray[curPortIndex] = NEW HTTPListenerSocket();
            QTSS_Error err = newListenerArray[curPortIndex]->Initialize(thePortTrackers[count3].fIPAddr, thePortTrackers[count3].fPort);

            char thePortStr[20];
            qtss_sprintf(thePortStr, "%hu", thePortTrackers[count3].fPort);
            
            //
            // If there was an error creating this listener, destroy it and log an error
            if ((startListeningNow) && (err != QTSS_NoErr))
                delete newListenerArray[curPortIndex];

            if (err == EADDRINUSE)
                QTSSModuleUtils::LogError(qtssWarningVerbosity, qtssListenPortInUse, 0, thePortStr);
            else if (err == EACCES)
                QTSSModuleUtils::LogError(qtssWarningVerbosity, qtssListenPortAccessDenied, 0, thePortStr);
            else if (err != QTSS_NoErr)
                QTSSModuleUtils::LogError(qtssWarningVerbosity, qtssListenPortError, 0, thePortStr);
            else
            {
                //
                // This listener was successfully created.
                if (startListeningNow)
                    newListenerArray[curPortIndex]->RequestEvent(EV_RE);
                curPortIndex++;
            }
        }
    }

    // Kill any listeners that we no longer need
    for (UInt32 count4 = 0; count4 < fNumListeners; count4++)
    {
        Bool16 deleteThisOne = true;
        
        for (UInt32 count5 = 0; count5 < curPortIndex; count5++)
        {
            if (newListenerArray[count5] == fListeners[count4])
                deleteThisOne = false;
        }
        
        if (deleteThisOne)
            fListeners[count4]->Signal(Task::kKillEvent);
    }
    
    //
    // Finally, make our server attributes and fListener privy to the new...
    fListeners = newListenerArray;
    fNumListeners = curPortIndex;
    UInt32 portIndex = 0;
    
    for (UInt32 count6 = 0; count6 < fNumListeners; count6++)
    {
        if  (fListeners[count6]->GetLocalAddr() != INADDR_LOOPBACK)
        {
            UInt16 thePort = fListeners[count6]->GetLocalPort();
            (void)this->SetValue(qtssSvrRTSPPorts, portIndex, &thePort, sizeof(thePort), QTSSDictionary::kDontObeyReadOnly);
            portIndex++;
        }
    }

    delete [] thePortTrackers;
    return (fNumListeners > 0);
}

UInt32* QTSServer::GetBindIPAddrs(QTSServerPrefs* inPrefs, UInt32* outNumAddrsPtr)
{
    UInt32 numAddrs = inPrefs->GetNumValues(qtssPrefsBindIPAddr);
    UInt32* theIPAddrArray = NULL;
    
    if (numAddrs == 0)
    {
        *outNumAddrsPtr = 1;
        theIPAddrArray = NEW UInt32[1];
        theIPAddrArray[0] = INADDR_ANY;
    }
    else
    {
        theIPAddrArray = NEW UInt32[numAddrs + 1];
        UInt32 arrIndex = 0;
        
        for (UInt32 theIndex = 0; theIndex < numAddrs; theIndex++)
        {
            // Get the ip addr out of the prefs dictionary
            QTSS_Error theErr = QTSS_NoErr;
            
            char* theIPAddrStr = NULL;
            theErr = inPrefs->GetValueAsString(qtssPrefsBindIPAddr, theIndex, &theIPAddrStr);
            if (theErr != QTSS_NoErr)
            {
                delete [] theIPAddrStr;
                break;
            }

            
            UInt32 theIPAddr = 0;
            if (theIPAddrStr != NULL)
            {
                theIPAddr = SocketUtils::ConvertStringToAddr(theIPAddrStr);
                delete [] theIPAddrStr;
                
                if (theIPAddr != 0)
                    theIPAddrArray[arrIndex++] = theIPAddr;
            }   
        }
        
        if ((numAddrs == 1) && (arrIndex == 0))
            theIPAddrArray[arrIndex++] = INADDR_ANY;
        else
            theIPAddrArray[arrIndex++] = INADDR_LOOPBACK;
    
        *outNumAddrsPtr = arrIndex;
    }
    
    return theIPAddrArray;
}

UInt16 QTSServer::GetServicePorts(QTSServerPrefs* inPrefs)
{       
    UInt16 thePort = 0;
    // Get the ip addr out of the prefs dictionary
    UInt32 theLen = sizeof(UInt16);
    QTSS_Error theErr = QTSS_NoErr;
    theErr = inPrefs->GetValue(qtssPrefsMonitorLANPort, 0, &thePort ,&theLen);
    Assert(theErr == QTSS_NoErr);   
    
    return thePort;
}

Bool16  QTSServer::SwitchPersonality()
{
#ifndef __Win32__  //not supported
    OSCharArrayDeleter runGroupName(fSrvrPrefs->GetRunGroupName());
    OSCharArrayDeleter runUserName(fSrvrPrefs->GetRunUserName());

	int groupID = 0;
	
    if (::strlen(runGroupName.GetObject()) > 0)
    {
        struct group* gr = ::getgrnam(runGroupName.GetObject());
        if (gr == NULL || ::setgid(gr->gr_gid) == -1)
        {
            char buffer[kErrorStrSize];

            QTSSModuleUtils::LogError(qtssFatalVerbosity, qtssMsgCannotSetRunGroup, 0,
                    runGroupName.GetObject(), qtss_strerror(OSThread::GetErrno(), buffer, sizeof(buffer)));
            return false;
        }
        groupID = gr->gr_gid;
    }

    if (::strlen(runUserName.GetObject()) > 0)
    {
        struct passwd* pw = ::getpwnam(runUserName.GetObject());

#if __MacOSX__
        if (pw != NULL && groupID != 0) //call initgroups before doing a setuid
            (void) initgroups(runUserName.GetObject(),groupID); 
#endif  

        if (pw == NULL || ::setuid(pw->pw_uid) == -1)
        {
            QTSSModuleUtils::LogError(qtssFatalVerbosity, qtssMsgCannotSetRunUser, 0,
                    runUserName.GetObject(), strerror(OSThread::GetErrno()));
            return false;
        }
    }

#endif  
   return true;
}


void    QTSServer::LoadCompiledInModules()
{
#ifndef DSS_DYNAMIC_MODULES_ONLY

	QTSSModule* theAuthModule = new QTSSModule("QTSSAuthModule");
	(void)theAuthModule->SetupModule(&sCallbacks, &QTSSAuthModule_Main);
	(void)AddModule(theAuthModule);

    // MODULE DEVELOPERS SHOULD ADD THE FOLLOWING THREE LINES OF CODE TO THIS
    // FUNCTION IF THEIR MODULE IS BEING COMPILED INTO THE SERVER.
    //
    // QTSSModule* myModule = new QTSSModule("__MODULE_NAME__");
    // (void)myModule->Initialize(&sCallbacks, &__MODULE_MAIN_ROUTINE__);
    // (void)AddModule(myModule);
    //
    // The following modules are all compiled into the server. 

#endif
}

void    QTSServer::InitCallbacks()
{
    sCallbacks.addr[kNewCallback] =                 (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_New;
    sCallbacks.addr[kDeleteCallback] =              (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_Delete;
    sCallbacks.addr[kMillisecondsCallback] =        (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_Milliseconds;
    sCallbacks.addr[kConvertToUnixTimeCallback] =   (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_ConvertToUnixTime;

    sCallbacks.addr[kAddRoleCallback] =             (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_AddRole;
    sCallbacks.addr[kCreateObjectTypeCallback] =    (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_CreateObjectType;
    sCallbacks.addr[kAddAttributeCallback] =        (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_AddAttribute;
    sCallbacks.addr[kIDForTagCallback] =            (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_IDForAttr;
    sCallbacks.addr[kGetAttributePtrByIDCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_GetValuePtr;
    sCallbacks.addr[kGetAttributeByIDCallback] =    (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_GetValue;
    sCallbacks.addr[kSetAttributeByIDCallback] =    (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_SetValue;
    sCallbacks.addr[kCreateObjectValueCallback] =   (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_CreateObject;
    sCallbacks.addr[kGetNumValuesCallback] =        (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_GetNumValues;

    sCallbacks.addr[kWriteCallback] =               (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_Write;
    sCallbacks.addr[kWriteVCallback] =              (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_WriteV;
    sCallbacks.addr[kFlushCallback] =               (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_Flush;
    sCallbacks.addr[kReadCallback] =                (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_Read;
    sCallbacks.addr[kSeekCallback] =                (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_Seek;
    sCallbacks.addr[kAdviseCallback] =              (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_Advise;

    sCallbacks.addr[kAddServiceCallback] =          (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_AddService;
    sCallbacks.addr[kIDForServiceCallback] =        (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_IDForService;
    sCallbacks.addr[kDoServiceCallback] =           (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_DoService;

    sCallbacks.addr[kRequestEventCallback] =        (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_RequestEvent;
    sCallbacks.addr[kSetIdleTimerCallback] =        (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_SetIdleTimer;
    sCallbacks.addr[kSignalStreamCallback] =        (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_SignalStream;

    sCallbacks.addr[kOpenFileObjectCallback] =      (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_OpenFileObject;
    sCallbacks.addr[kCloseFileObjectCallback] =     (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_CloseFileObject;

    sCallbacks.addr[kCreateSocketStreamCallback] =  (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_CreateStreamFromSocket;
    sCallbacks.addr[kDestroySocketStreamCallback] = (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_DestroySocketStream;

    sCallbacks.addr[kAddStaticAttributeCallback] =          (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_AddStaticAttribute;
    sCallbacks.addr[kAddInstanceAttributeCallback] =        (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_AddInstanceAttribute;
    sCallbacks.addr[kRemoveInstanceAttributeCallback] =     (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_RemoveInstanceAttribute;

    sCallbacks.addr[kGetAttrInfoByIndexCallback] =          (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_GetAttrInfoByIndex;
    sCallbacks.addr[kGetAttrInfoByNameCallback] =           (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_GetAttrInfoByName;
    sCallbacks.addr[kGetAttrInfoByIDCallback] =             (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_GetAttrInfoByID;
    sCallbacks.addr[kGetNumAttributesCallback] =            (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_GetNumAttributes;


    sCallbacks.addr[kGetValueAsStringCallback] =            (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_GetValueAsString;
    sCallbacks.addr[kTypeToTypeStringCallback] =            (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_TypeToTypeString;
    sCallbacks.addr[kTypeStringToTypeCallback] =            (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_TypeStringToType;
    sCallbacks.addr[kStringToValueCallback] =               (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_StringToValue;
    sCallbacks.addr[kValueToStringCallback] =               (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_ValueToString;

    sCallbacks.addr[kRemoveValueCallback] =                 (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_RemoveValue;

    sCallbacks.addr[kRequestGlobalLockCallback] =           (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_RequestLockedCallback;
    sCallbacks.addr[kIsGlobalLockedCallback] =              (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_IsGlobalLocked;
    sCallbacks.addr[kUnlockGlobalLock] =                    (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_UnlockGlobalLock;
    
    sCallbacks.addr[kLockObjectCallback] =                  (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_LockObject;
    sCallbacks.addr[kUnlockObjectCallback] =                (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_UnlockObject;
    sCallbacks.addr[kSetAttributePtrCallback] =             (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_SetValuePtr;
    
    sCallbacks.addr[kSetIntervalRoleTimerCallback] =        (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_SetIdleRoleTimer;
    
    sCallbacks.addr[kLockStdLibCallback] =                  (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_LockStdLib;
    sCallbacks.addr[kUnlockStdLibCallback] =                (QTSS_CallbackProcPtr)QTSSCallbacks::QTSS_UnlockStdLib;
	sCallbacks.addr[kEasySendMsgCallback] =					(QTSS_CallbackProcPtr)QTSSCallbacks::Easy_SendMsg;
}

void QTSServer::LoadModules(QTSServerPrefs* inPrefs)
{
    // Fetch the name of the module directory and open it.
    OSCharArrayDeleter theModDirName(inPrefs->GetModuleDirectory());
    
#ifdef __Win32__
    // NT doesn't seem to have support for the POSIX directory parsing APIs.
    OSCharArrayDeleter theLargeModDirName(NEW char[::strlen(theModDirName.GetObject()) + 3]);
    ::strcpy(theLargeModDirName.GetObject(), theModDirName.GetObject());
    ::strcat(theLargeModDirName.GetObject(), "\\*");
    
    WIN32_FIND_DATA theFindData;
    HANDLE theSearchHandle = ::FindFirstFile(theLargeModDirName.GetObject(), &theFindData);
    
    if (theSearchHandle == INVALID_HANDLE_VALUE)
    {
        QTSSModuleUtils::LogError(qtssWarningVerbosity, qtssMsgNoModuleFolder, 0);  
        return;
    }
    
    while (theSearchHandle != INVALID_HANDLE_VALUE)
    {
        this->CreateModule(theModDirName.GetObject(), theFindData.cFileName);
        
        if (!::FindNextFile(theSearchHandle, &theFindData))
        {
            ::FindClose(theSearchHandle);
            theSearchHandle = INVALID_HANDLE_VALUE;
        }
    }
#else       

    // POSIX version
	// opendir mallocs memory for DIR* so call closedir to free the allocated memory
    DIR* theDir = ::opendir(theModDirName.GetObject());
    if (theDir == NULL)
    {
        QTSSModuleUtils::LogError(qtssWarningVerbosity, qtssMsgNoModuleFolder, 0);  
        return;
    }
    
    while (true)
    {
        // Iterate over each file in the directory, attempting to construct
        // a module object from that file.
        
        struct dirent* theFile = ::readdir(theDir);
        if (theFile == NULL)
            break;
        
        this->CreateModule(theModDirName.GetObject(), theFile->d_name);
    }
	
	(void)::closedir(theDir);
	
#endif
}

void    QTSServer::CreateModule(char* inModuleFolderPath, char* inModuleName)
{
    // Ignore these silly directory names
        
    if (::strcmp(inModuleName, ".") == 0)
        return;
    if (::strcmp(inModuleName, "..") == 0)
        return;
    if (::strlen(inModuleName) == 0)
        return;
    if (*inModuleName == '.')
        return; // Fix 2572248. Do not attempt to load '.' files as modules at all 

    //
    // Construct a full path to this module
    UInt32 totPathLen = ::strlen(inModuleFolderPath) + ::strlen(inModuleName);
    OSCharArrayDeleter theModPath(NEW char[totPathLen + 4]);
    ::strcpy(theModPath.GetObject(), inModuleFolderPath);
    ::strcat(theModPath.GetObject(), kPathDelimiterString);
    ::strcat(theModPath.GetObject(), inModuleName);
            
    //
    // Construct a QTSSModule object, and attempt to initialize the module
    QTSSModule* theNewModule = NEW QTSSModule(inModuleName, theModPath.GetObject());
    QTSS_Error theErr = theNewModule->SetupModule(&sCallbacks);
    
    if (theErr != QTSS_NoErr)
    {
        QTSSModuleUtils::LogError(qtssWarningVerbosity, qtssMsgBadModule, theErr,
                                        inModuleName);
        delete theNewModule;
    }
    //
    // If the module was successfully initialized, add it to our module queue
    else if (!this->AddModule(theNewModule))
    {
        QTSSModuleUtils::LogError(qtssWarningVerbosity, qtssMsgRegFailed, theErr,
                                        inModuleName);
        delete theNewModule;
    }
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
    QTSSPrefs* thePrefs = NEW QTSSPrefs( sPrefsSource, inModule->GetValue(qtssModName), QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kModulePrefsDictIndex), true);
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
            delete [] sModuleArray[x];
        sModuleArray[x] = NULL; 
    }
}
 
void QTSServer::DoInitRole()
{
    QTSS_RoleParams theInitParams;
    theInitParams.initParams.inServer =         this;
    theInitParams.initParams.inPrefs =          fSrvrPrefs;
    theInitParams.initParams.inMessages =       fSrvrMessages;
    theInitParams.initParams.inErrorLogStream = &sErrorLogStream;
        
    QTSS_ModuleState theModuleState;
    theModuleState.curRole = QTSS_Initialize_Role;
    theModuleState.curTask = NULL;
    OSThread::SetMainThreadData(&theModuleState);

    //
    // Add the OPTIONS method as the one method the server handles by default (it handles
    // it internally). Modules that handle other RTSP methods will add 
    HTTPMethod theOptionsMethod = httpGetMethod;
    (void)this->SetValue(qtssSvrHandledMethods, 0, &theOptionsMethod, sizeof(theOptionsMethod));


// For now just disable the SetParameter to be compatible with Real.  It should really be removed only for clients that have problems with their SetParameter implementations like (Real Players).
// At the moment it isn't necesary to add the option.
//   QTSS_RTSPMethod	theSetParameterMethod = qtssSetParameterMethod;
//    (void)this->SetValue(qtssSvrHandledMethods, 0, &theSetParameterMethod, sizeof(theSetParameterMethod));
	
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

Task*   HTTPListenerSocket::GetSessionTask(TCPSocket** outSocket)
{
    Assert(outSocket != NULL);
    
    HTTPSession* theTask = NEW HTTPSession();
    *outSocket = theTask->GetSocket();  // out socket is not attached to a unix socket yet.

    if (this->OverMaxConnections(0))
        this->SlowDown();
    else
        this->RunNormal();
        
    return theTask;
}


Bool16 HTTPListenerSocket::OverMaxConnections(UInt32 buffer)
{
    QTSServerInterface* theServer = QTSServerInterface::GetServer();
    SInt32 maxConns = theServer->GetPrefs()->GetMaxConnections();
    Bool16 overLimit = false;
    
    if (maxConns > -1) // limit connections
    { 
        maxConns += buffer;
        if(theServer->GetNumServiceSessions() > (UInt32) maxConns)
        {
            overLimit = true;          
        }
    } 
    return overLimit;
     
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
    
    {
        //
        // Update listeners, ports, and IP addrs.
        OSMutexLocker locker(theServer->GetServerObjectMutex());
        (void)((QTSServer*)theServer)->SetDefaultIPAddr();
        (void)((QTSServer*)theServer)->CreateListeners(true, thePrefs, 0);
    }
    
    // Delete all the streams
    QTSSModule** theModule = NULL;
    UInt32 theLen = 0;
    
    for (int y = 0; QTSServerInterface::GetServer()->GetValuePtr(qtssSvrModuleObjects, y, (void**)(void*)&theModule, &theLen) == QTSS_NoErr; y++)
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


