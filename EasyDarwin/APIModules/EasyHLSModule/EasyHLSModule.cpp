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

// STATIC DATA

// Default values for preferences
static Bool16   sDefaultLogEnabled          = true;
static char*    sDefaultLogName             = "EasyDarwin";
static char*    sDefaultLogDir = NULL;
static QTSS_PrefsObject     sServerPrefs = NULL;

class ProxyTask : public Task
{
    public:
    
        //
        // Task that just polls on all the sockets in the pool, sending data on all available sockets
        ProxyTask() : Task() {this->SetTaskName("ProxyTask");  this->Signal(Task::kStartEvent); }
        virtual ~ProxyTask() {}
    
    private:
    
        virtual SInt64 Run();
        
        enum
        {
            kProxyTaskPollIntervalMsec = 10
        };
};


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
    }
    return QTSS_NoErr;
}


QTSS_Error Register(QTSS_Register_Params* inParams)
{
    // Do role & attribute setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_Shutdown_Role);
    
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
    //sProxyTask = NEW ProxyTask();
    //sSessionMap = NEW OSRefTable();

    return QTSS_NoErr;
}

QTSS_Error Shutdown()
{
    return QTSS_NoErr;
}

SInt64 ProxyTask::Run()
{
	return 0;
}
