/*
	Copyright (c) 2013-2016 EasyDarwin.ORG.  All rights reserved.
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

#ifdef __Win32__
#define EasyHLS_KEY "333565546A4969576B5A7341645068577151654B6B76464659584E355247467964326C754C6D56345A536C5737366F412F704A6C59584E35"
#else
#define EasyHLS_KEY "333565546A4A4F576B596F41645068577151654B6B76566C59584E355A47467964326C75766C62767167442B6B6D566863336B3D"
#endif

#include "EasyHLSModule.h"
#include "QTSSModuleUtils.h"
#include "OSArrayObjectDeleter.h"
#include "OSMemory.h"
#include "QTSSMemoryDeleter.h"
#include "QueryParamList.h"
#include "OSRef.h"
#include "StringParser.h"
#include "EasyHLSSession.h"

#include "QTSServerInterface.h"

class HLSSessionCheckingTask : public Task
{
    public:
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
static QTSS_PrefsObject         sServerPrefs		= NULL;
static HLSSessionCheckingTask*	sCheckingTask		= NULL;

static OSRefTable*				sHLSSessionMap		= NULL;

static QTSS_ServerObject sServer					= NULL;
static QTSS_ModulePrefsObject       sModulePrefs	= NULL;

static StrPtrLen	sHLSSuffix("EasyHLSModule");

#define QUERY_STREAM_NAME	"name"
#define QUERY_STREAM_URL	"url"
#define QUERY_STREAM_CMD	"cmd"
#define QUERY_STREAM_CMD_START "start"
#define QUERY_STREAM_CMD_STOP "stop"

// FUNCTION PROTOTYPES
static QTSS_Error EasyHLSModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error Register(QTSS_Register_Params* inParams);
static QTSS_Error Initialize(QTSS_Initialize_Params* inParams);

static QTSS_Error RereadPrefs();

static QTSS_Error EasyHLSOpen(Easy_HLSOpen_Params* inParams);
static QTSS_Error EasyHLSClose(Easy_HLSClose_Params* inParams);
static char* GetHLSUrl(char* inSessionName);

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
        case QTSS_RereadPrefs_Role:
            return RereadPrefs();
		case Easy_HLSOpen_Role:		//Start HLS Streaming
			return EasyHLSOpen(&inParams->easyHLSOpenParams);
		case Easy_HLSClose_Role:	//Stop HLS Streaming
			return EasyHLSClose(&inParams->easyHLSCloseParams);
    }
    return QTSS_NoErr;
}

QTSS_Error Register(QTSS_Register_Params* inParams)
{

	int isEasyHLSActivated = EasyHLS_Activate(EasyHLS_KEY);
	switch(isEasyHLSActivated)
	{
	case EASY_ACTIVATE_INVALID_KEY:
		printf("EasyHLS_KEY is EASY_ACTIVATE_INVALID_KEY!\n");
		break;
	case EASY_ACTIVATE_TIME_ERR:
		printf("EasyHLS_KEY is EASY_ACTIVATE_TIME_ERR!\n");
		break;
	case EASY_ACTIVATE_PROCESS_NAME_LEN_ERR:
		printf("EasyHLS_KEY is EASY_ACTIVATE_PROCESS_NAME_LEN_ERR!\n");
		break;
	case EASY_ACTIVATE_PROCESS_NAME_ERR:
		printf("EasyHLS_KEY is EASY_ACTIVATE_PROCESS_NAME_ERR!\n");
		break;
	case EASY_ACTIVATE_VALIDITY_PERIOD_ERR:
		printf("EasyHLS_KEY is EASY_ACTIVATE_VALIDITY_PERIOD_ERR!\n");
		break;
	case EASY_ACTIVATE_SUCCESS:
		printf("EasyHLS_KEY is EASY_ACTIVATE_SUCCESS!\n");
		break;
	}

	if(EASY_ACTIVATE_SUCCESS != isEasyHLSActivated)
		return QTSS_RequestFailed;


    // Do role & attribute setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_RereadPrefs_Role);   
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

	sHLSSessionMap =  QTSServerInterface::GetServer()->GetHLSSessionMap();

    // Setup global data structures
    sServerPrefs = inParams->inPrefs;
    sCheckingTask = NEW HLSSessionCheckingTask();

    sServer = inParams->inServer;
    
    sModulePrefs = QTSSModuleUtils::GetModulePrefsObject(inParams->inModule);

    // Call helper class initializers
    EasyHLSSession::Initialize(sModulePrefs);

	RereadPrefs();

    return QTSS_NoErr;
}

QTSS_Error RereadPrefs()
{
	return QTSS_NoErr;
}


SInt64 HLSSessionCheckingTask::Run()
{
	return kProxyTaskPollIntervalMsec;
}

QTSS_Error EasyHLSOpen(Easy_HLSOpen_Params* inParams)
{
	OSMutexLocker locker (sHLSSessionMap->GetMutex());

	EasyHLSSession* session = NULL;
	//首先查找MAP里面是否已经有了对应的流
	StrPtrLen streamName(inParams->inStreamName);
	OSRef* clientSesRef = sHLSSessionMap->Resolve(&streamName);
	if(clientSesRef != NULL)
	{
		session = (EasyHLSSession*)clientSesRef->GetObject();
	}
	else
	{
		session = NEW EasyHLSSession(&streamName);

		OS_Error theErr = sHLSSessionMap->Register(session->GetRef());
		Assert(theErr == QTSS_NoErr);

		//增加一次对RelaySession的无效引用，后面会统一释放
		OSRef* debug = sHLSSessionMap->Resolve(&streamName);
		Assert(debug == session->GetRef());
	}
	
	//到这里，肯定是有一个EasyHLSSession可用的
	session->HLSSessionStart(inParams->inRTSPUrl, inParams->inTimeout);

	if(inParams->outHLSUrl)
		qtss_sprintf(inParams->outHLSUrl,"%s",session->GetHLSURL());

	sHLSSessionMap->Release(session->GetRef());

	return QTSS_NoErr;
}

QTSS_Error EasyHLSClose(Easy_HLSClose_Params* inParams)
{
	OSMutexLocker locker (sHLSSessionMap->GetMutex());

	//首先查找Map里面是否已经有了对应的流
	StrPtrLen streamName(inParams->inStreamName);

	OSRef* clientSesRef = sHLSSessionMap->Resolve(&streamName);

	if(NULL == clientSesRef) return QTSS_RequestFailed;

	EasyHLSSession* session = (EasyHLSSession*)clientSesRef->GetObject();

	session->HLSSessionRelease();

	sHLSSessionMap->Release(session->GetRef());

    if (session->GetRef()->GetRefCount() == 0)
    {   
        qtss_printf("EasyHLSModule.cpp:EasyHLSClose UnRegister and delete session =%p refcount=%"_U32BITARG_"\n", session->GetRef(), session->GetRef()->GetRefCount() ) ;       
        sHLSSessionMap->UnRegister(session->GetRef());
		session->Signal(Task::kKillEvent);
    }
	return QTSS_NoErr;
}

char* GetHLSUrl(char* inSessionName)
{
	OSRefTable* sHLSSessionMap =  QTSServerInterface::GetServer()->GetHLSSessionMap();

	OSMutexLocker locker (sHLSSessionMap->GetMutex());

	char* hlsURL = NULL;
	//首先查找Map里面是否已经有了对应的流
	StrPtrLen streamName(inSessionName);

	OSRef* clientSesRef = sHLSSessionMap->Resolve(&streamName);

	if(NULL == clientSesRef) return NULL;

	EasyHLSSession* session = (EasyHLSSession*)clientSesRef->GetObject();

	hlsURL = session->GetHLSURL();

	sHLSSessionMap->Release(session->GetRef());

	return hlsURL;
}