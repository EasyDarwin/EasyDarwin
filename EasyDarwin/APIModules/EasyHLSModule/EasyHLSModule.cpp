/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
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
#include "QTSSMemoryDeleter.h"
#include "QueryParamList.h"
#include "OSRef.h"
#include "StringParser.h"
#include "EasyHLSSession.h"

#include "QTSServerInterface.h"

#include "ParseDevice.h"
#include <libEasyHttp.h>

class HLSSessionCheckingTask : public Task
{
    public:
        // Task that just polls on all the sockets in the pool, sending data on all available sockets
        HLSSessionCheckingTask() : Task() 
		{
			this->SetTaskName("HLSSessionCheckingTask");  
			uIndex = 0;
			this->Signal(Task::kStartEvent); 
		}
        virtual ~HLSSessionCheckingTask() {}
    
    private:
        virtual SInt64 Run();
        
		UInt32 uIndex;
        enum
        {
            kProxyTaskPollIntervalMsec = 1000
        };
};

// STATIC DATA
static QTSS_PrefsObject         sServerPrefs		= NULL;
static HLSSessionCheckingTask*	sCheckingTask		= NULL;

static QTSS_ServerObject sServer					= NULL;
static QTSS_ModulePrefsObject       sModulePrefs	= NULL;

// 设备映射表
CParseDevice*			parseDevice = NULL;

static StrPtrLen	sHLSSuffix("EasyHLSModule");

#define QUERY_STREAM_NAME	"name"
#define QUERY_STREAM_URL	"url"
#define QUERY_STREAM_CMD	"cmd"
#define QUERY_STREAM_CMD_START "start"
#define QUERY_STREAM_CMD_STOP "stop"

static char*	sDevicePrefs		= NULL;
static char*	sDefaultDevicePrefs = "./devices.xml";
static unsigned int	sDeviceNum			= 0;

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

    // Setup global data structures
    sServerPrefs = inParams->inPrefs;

    sServer = inParams->inServer;
    
    sModulePrefs = QTSSModuleUtils::GetModulePrefsObject(inParams->inModule);

    // Call helper class initializers
    EasyHLSSession::Initialize(sModulePrefs);

	RereadPrefs();

    sCheckingTask = NEW HLSSessionCheckingTask();

    return QTSS_NoErr;
}

QTSS_Error RereadPrefs()
{
	delete [] sDevicePrefs;
    
    sDevicePrefs = QTSSModuleUtils::GetStringAttribute(sModulePrefs, "device_prefs_file", sDefaultDevicePrefs);

	parseDevice = NEW CParseDevice();
	if (success != parseDevice->Init())
	{
		qtss_printf("parseDevice Init fail\n");
		return QTSS_Unimplemented;
	}

	sDeviceNum = parseDevice->LoadDeviceXml(sDevicePrefs);
	if(sDeviceNum == 0)
	{
		qtss_printf("parseDevice LoadDeviceXml %s fail\n", sDevicePrefs);
	}

	return QTSS_NoErr;
}


SInt64 HLSSessionCheckingTask::Run()
{
	EasyHttp_Get("http://111.1.6.132:11000/AppCamera/CheckDevStateRestaurant?count=10000&isBuildIn=1");
        
    if(uIndex >= sDeviceNum)
		return -1;

	DeviceInfo* pDeviceInfo = parseDevice->GetDeviceInfoByIdIndex(uIndex);
	qtss_printf("%d/%d name=%s url=%s\n", uIndex, sDeviceNum, pDeviceInfo->m_szIdname, pDeviceInfo->m_szSourceUrl);

	Easy_StartHLSSession(pDeviceInfo->m_szIdname, pDeviceInfo->m_szSourceUrl, 0, NULL);

	uIndex ++;
	return kProxyTaskPollIntervalMsec;
}

QTSS_Error EasyHLSOpen(Easy_HLSOpen_Params* inParams)
{	
	OSRefTable* sHLSSessionMap =  QTSServerInterface::GetServer()->GetHLSSessionMap();

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
	OSRefTable* sHLSSessionMap =  QTSServerInterface::GetServer()->GetHLSSessionMap();

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
        delete session;
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