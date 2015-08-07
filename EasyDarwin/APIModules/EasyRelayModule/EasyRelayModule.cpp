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
#include "QueryParamList.h"
#include "OSRef.h"
#include "StringParser.h"
#include "QueryParamList.h"
#include "EasyRelaySession.h"
#include "QTSServerInterface.h"

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

static char*            sLocal_IP_Addr = NULL;
static char*            sDefaultLocal_IP_Addr = "127.0.0.1";

#define QUERY_STREAM_NAME	"name"
#define QUERY_STREAM_URL	"url"
#define QUERY_STREAM_CMD	"cmd"
#define QUERY_STREAM_CMD_START "start"
#define QUERY_STREAM_CMD_STOP "stop"

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
	delete [] sLocal_IP_Addr;
    sLocal_IP_Addr = QTSSModuleUtils::GetStringAttribute(sPrefs, "local_ip_address", sDefaultLocal_IP_Addr);

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
	//解析命令
    char* theFullPathStr = NULL;
    QTSS_Error theErr = QTSS_GetValueAsString(inParams->inRTSPRequest, qtssRTSPReqFileName, 0, &theFullPathStr);
    Assert(theErr == QTSS_NoErr);
    QTSSCharArrayDeleter theFullPathStrDeleter(theFullPathStr);
        
    if (theErr != QTSS_NoErr)
        return NULL;

    StrPtrLen theFullPath(theFullPathStr);

    if (theFullPath.Len != sRelaySuffix.Len )
	return NULL;

	StrPtrLen endOfPath2(&theFullPath.Ptr[theFullPath.Len -  sRelaySuffix.Len], sRelaySuffix.Len);
    if (!endOfPath2.Equal(sRelaySuffix))
    {   
        return NULL;
    }

	//解析查询字符串
    char* theQueryStr = NULL;
    theErr = QTSS_GetValueAsString(inParams->inRTSPRequest, qtssRTSPReqQueryString, 0, &theQueryStr);
    Assert(theErr == QTSS_NoErr);
    QTSSCharArrayDeleter theQueryStringDeleter(theQueryStr);
        
    if (theErr != QTSS_NoErr)
        return NULL;

    StrPtrLen theQueryString(theQueryStr);

	QueryParamList parList(theQueryStr);

	const char* sName = parList.DoFindCGIValueForParam(QUERY_STREAM_NAME);
	if(sName == NULL) return NULL;

	const char* sURL = parList.DoFindCGIValueForParam(QUERY_STREAM_URL);
	if(sURL == NULL) return NULL;

	const char* sCMD = parList.DoFindCGIValueForParam(QUERY_STREAM_CMD);

	bool bStop = false;
	if(sCMD)
	{
		if(::strcmp(sCMD,QUERY_STREAM_CMD_STOP) == 0)
			bStop = true;
	}

	StrPtrLen streamName((char*)sName);
	//从接口获取信息结构体
	EasyRelaySession* session = NULL;
	//首先查找Map里面是否已经有了对应的流
	OSRef* sessionRef = sRelaySessionMap->Resolve(&streamName);
	if(sessionRef != NULL)
	{
		session = (EasyRelaySession*)sessionRef->GetObject();
	}
	else
	{
		if(bStop) return NULL;

		session = NEW EasyRelaySession((char*)sURL, EasyRelaySession::kRTSPTCPClientType, (char*)sName);

		QTSS_Error theErr = session->RelaySessionStart();

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

	sRelaySessionMap->Release(session->GetRef());

	if(bStop)
	{
		sRelaySessionMap->UnRegister(session->GetRef());
		session->Signal(Task::kKillEvent);
		return QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssSuccessOK, 0); 
	}

	QTSS_RTSPStatusCode statusCode = qtssRedirectPermMoved;
	QTSS_SetValue(inParams->inRTSPRequest, qtssRTSPReqStatusCode, 0, &statusCode, sizeof(statusCode));

	// Get the ip addr out of the prefs dictionary
	UInt16 thePort = 554;
	UInt32 theLen = sizeof(UInt16);
	theErr = QTSServerInterface::GetServer()->GetPrefs()->GetValue(qtssPrefsRTSPPorts, 0, &thePort, &theLen);
	Assert(theErr == QTSS_NoErr);   

	//构造本地URL
	char url[QTSS_MAX_URL_LENGTH] = { 0 };

	qtss_sprintf(url,"rtsp://%s:%d/%s.sdp", sLocal_IP_Addr, thePort, sName);
	StrPtrLen locationRedirect(url);

	Bool16 sFalse = false;
	(void)QTSS_SetValue(inParams->inRTSPRequest, qtssRTSPReqRespKeepAlive, 0, &sFalse, sizeof(sFalse));
	QTSS_AppendRTSPHeader(inParams->inRTSPRequest, qtssLocationHeader, locationRedirect.Ptr, locationRedirect.Len);	
	return QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssRedirectPermMoved, 0);
}