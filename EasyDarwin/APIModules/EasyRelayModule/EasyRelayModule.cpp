/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       QTSSOnDemandRelayModule.cpp
    Contains:   RTSP On Demand Relay Module
*/
#include "EasyRelayModule.h"
#include "QTSSModuleUtils.h"
#include "ReflectorSession.h"
#include "OSArrayObjectDeleter.h"
#include "QTSSMemoryDeleter.h"
#include "OSRef.h"
#include "StringParser.h"
#include "QueryParamList.h"
#include "EasyRelaySession.h"

#ifndef __Win32__
    #include <unistd.h>
	#include <netdb.h>
#endif

// STATIC DATA
static OSRefTable* sRelaySessionMap = NULL;
static QTSS_PrefsObject sServerPrefs = NULL;
static QTSS_ServerObject sServer = NULL;
static QTSS_ModulePrefsObject       sPrefs = NULL;

static StrPtrLen    sRelaySuffix("EasyRelayModule");

// FUNCTION PROTOTYPES
static QTSS_Error EasyRelayModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error Register(QTSS_Register_Params* inParams);
static QTSS_Error Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error RereadPrefs();

static QTSS_Error ProcessRTSPRequest(QTSS_StandardRTSP_Params* inParams);
static QTSS_Error DoDescribe(QTSS_StandardRTSP_Params* inParams);

// FUNCTION IMPLEMENTATIONS
QTSS_Error EasyRelayModule_Main(void* inPrivateArgs)
{
    return _stublibrary_main(inPrivateArgs, EasyRelayModuleDispatch);
}

QTSS_Error  EasyRelayModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
    switch (inRole)
    {
        case QTSS_Register_Role:
            return Register(&inParams->regParams);
        case QTSS_Initialize_Role:
            return Initialize(&inParams->initParams);
        case QTSS_RereadPrefs_Role:
            return RereadPrefs();
        case QTSS_RTSPPreProcessor_Role:
            return ProcessRTSPRequest(&inParams->rtspRequestParams);
   }
    return QTSS_NoErr;
}


QTSS_Error Register(QTSS_Register_Params* inParams)
{
    // Do role & attribute setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_RTSPPreProcessor_Role);
    (void)QTSS_AddRole(QTSS_RereadPrefs_Role);    
    
    // Tell the server our name!
    static char* sModuleName = "QTSSOnDemandRelayModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    return QTSS_NoErr;
}

QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{
    // Setup module utils
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);
	sRelaySessionMap = NEW OSRefTable();

    sServerPrefs = inParams->inPrefs;
    sServer = inParams->inServer;
    sPrefs = QTSSModuleUtils::GetModulePrefsObject(inParams->inModule);

    // Report to the server that this module handles DESCRIBE
	static QTSS_RTSPMethod sSupportedMethods[] = { qtssOptionsMethod, qtssDescribeMethod};
	QTSSModuleUtils::SetupSupportedMethods(inParams->inServer, sSupportedMethods, 2);

    RereadPrefs();
    
   return QTSS_NoErr;
}

QTSS_Error RereadPrefs()
{
	return QTSS_NoErr;
}

QTSS_Error ProcessRTSPRequest(QTSS_StandardRTSP_Params* inParams)
{
    QTSS_RTSPMethod* theMethod = NULL;

	UInt32 theLen = 0;
    if ((QTSS_GetValuePtr(inParams->inRTSPRequest, qtssRTSPReqMethod, 0,
            (void**)&theMethod, &theLen) != QTSS_NoErr) || (theLen != sizeof(QTSS_RTSPMethod)))
    {
        Assert(0);
        return QTSS_RequestFailed;
    }

    if (*theMethod == qtssDescribeMethod)
        return DoDescribe(inParams);
             
    return QTSS_NoErr;
}
QTSS_Error DoDescribe(QTSS_StandardRTSP_Params* inParams)
{
    char* theFullPathStr = NULL;
    QTSS_Error theErr = QTSS_GetValueAsString(inParams->inRTSPRequest, qtssRTSPReqLocalPath, 0, &theFullPathStr);
    Assert(theErr == QTSS_NoErr);
    QTSSCharArrayDeleter theFullPathStrDeleter(theFullPathStr);
        
    if (theErr != QTSS_NoErr)
        return NULL;

    StrPtrLen theFullPath(theFullPathStr);

    if (theFullPath.Len > sRelaySuffix.Len )
    {   
		StrPtrLen endOfPath2(&theFullPath.Ptr[theFullPath.Len -  sRelaySuffix.Len], sRelaySuffix.Len);
        if (!endOfPath2.Equal(sRelaySuffix))
        {   
            return NULL;
        }
    }


	//从接口获取信息结构体
	EasyRelaySession* session = NULL;
	//首先查找Map里面是否已经有了对应的流
	StrPtrLen streamName("live");
	OSRef* sessionRef = sRelaySessionMap->Resolve(&streamName);
	if(sessionRef != NULL)
	{
		session = (EasyRelaySession*)sessionRef->GetObject();
	}
	else
	{
		session = NEW EasyRelaySession("rtsp://admin:admin@192.168.1.106/", EasyRelaySession::kRTSPTCPClientType, "live");


		QTSS_Error theErr = session->HLSSessionStart();

		if(theErr == QTSS_NoErr)
		{
			OS_Error theErr = sRelaySessionMap->Register(session->GetRef());
			Assert(theErr == QTSS_NoErr);
		}
		else
		{
			session->Signal(Task::kKillEvent);
			return QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssClientNotFound, 0); 
		}

		//增加一次对RelaySession的无效引用，后面会统一释放
		OSRef* debug = sRelaySessionMap->Resolve(&streamName);
		Assert(debug == session->GetRef());
	}

    return QTSS_NoErr;
}