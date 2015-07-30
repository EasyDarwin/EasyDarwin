/*
	Copyright (c) 2013-2014 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       EasyHLSModule.cpp
    Contains:   Implementation of EasyHLSModule class. 
*/

#ifndef __Win32__
// For gethostbyname
#include <netdb.h>
#endif

#include "EasyHLSModule.h"
#include "QTSSModuleUtils.h"
#include "OSArrayObjectDeleter.h"
#include "OSMemory.h"
#include "ClientSocket.h"
#include "SocketUtils.h"

class HLSSessionCheckingTask : public Task
{
    public:
    
        //
        // Task that just polls on all the sockets in the pool, sending data on all available sockets
        HLSSessionCheckingTask() : Task() {this->SetTaskName("HLSSessionCheckingTask");  this->Signal(Task::kStartEvent); }
        virtual ~HLSSessionCheckingTask() {}
    
    private:
        virtual SInt64 Run();
        
        enum
        {
            kProxyTaskPollIntervalMsec = 60*1000
        };
};

// STATIC DATA
static QTSS_PrefsObject         sServerPrefs = NULL;
static HLSSessionCheckingTask*	sCheckingTask = NULL;
static OSRefTable*              sHLSSessionMap = NULL;

// FUNCTION PROTOTYPES

static QTSS_Error EasyHLSModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error Register(QTSS_Register_Params* inParams);
static QTSS_Error Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error Shutdown();

// FUNCTION IMPLEMENTATIONS


QTSS_Error EasyHLSModule_Main(void* inPrivateArgs)
{
    return _stublibrary_main(inPrivateArgs, EasyHLSModuleDispatch);
}

QTSS_Error  EasyHLSModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
    switch (inRole)
    {
        case QTSS_Register_Role:
            return Register(&inParams->regParams);
        case QTSS_Initialize_Role:
            return Initialize(&inParams->initParams);
        case QTSS_Shutdown_Role:
            return Shutdown();
		case Easy_HLSOpen_Role:
			return QTSS_NoErr;
		case Easy_HLSClose_Role:
			return QTSS_NoErr;
    }
    return QTSS_NoErr;
}


QTSS_Error Register(QTSS_Register_Params* inParams)
{
    // Do role & attribute setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_Shutdown_Role);
    (void)QTSS_AddRole(Easy_HLSOpen_Role); 
	(void)QTSS_AddRole(Easy_HLSClose_Role); 
    
    // Tell the server our name!
    static char* sModuleName = "EasyHLSModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    return QTSS_NoErr;
}


QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{
    // Setup module utils
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);
    
    //
    // Setup global data structures
    sServerPrefs = inParams->inPrefs;
    sCheckingTask = NEW HLSSessionCheckingTask();
    sHLSSessionMap = NEW OSRefTable();

    return QTSS_NoErr;
}

QTSS_Error Shutdown()
{
    return QTSS_NoErr;
}

SInt64 HLSSessionCheckingTask::Run()
{
	return kProxyTaskPollIntervalMsec;
}
