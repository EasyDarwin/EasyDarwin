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
static OSRefTable*              sHLSSessionMap		= NULL;
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

static QTSS_Error ProcessRTSPRequest(QTSS_StandardRTSP_Params* inParams);
static QTSS_Error DoDescribe(QTSS_StandardRTSP_Params* inParams);

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
        case QTSS_RTSPPreProcessor_Role:
            return ProcessRTSPRequest(&inParams->rtspRequestParams);
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
    (void)QTSS_AddRole(QTSS_RTSPPreProcessor_Role);
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
    sCheckingTask = NEW HLSSessionCheckingTask();
    sHLSSessionMap = NEW OSRefTable();

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
	session->HLSSessionStart(inParams->inRTSPUrl);

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
        delete session;
    }
	return QTSS_NoErr;
}

char* GetHLSUrl(char* inSessionName)
{
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

QTSS_Error ProcessRTSPRequest(QTSS_StandardRTSP_Params* inParams)
{
	OSMutexLocker locker (sHLSSessionMap->GetMutex());
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
	char* location = NULL;
	//解析命令
    char* theFullPathStr = NULL;
    QTSS_Error theErr = QTSS_GetValueAsString(inParams->inRTSPRequest, qtssRTSPReqFileName, 0, &theFullPathStr);
    Assert(theErr == QTSS_NoErr);
    QTSSCharArrayDeleter theFullPathStrDeleter(theFullPathStr);
        
    if (theErr != QTSS_NoErr)
        return NULL;

    StrPtrLen theFullPath(theFullPathStr);

    if (theFullPath.Len != sHLSSuffix.Len )
	return NULL;

	StrPtrLen endOfPath2(&theFullPath.Ptr[theFullPath.Len -  sHLSSuffix.Len], sHLSSuffix.Len);
    if (!endOfPath2.Equal(sHLSSuffix))
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

	const char* sCMD = parList.DoFindCGIValueForParam(QUERY_STREAM_CMD);

	bool bStop = false;
	if(sCMD)
	{
		if(::strcmp(sCMD,QUERY_STREAM_CMD_STOP) == 0)
			bStop = true;
	}

	if(bStop)
	{
		Easy_HLSClose_Params params;
		params.inStreamName = (char*)sName;
		EasyHLSClose(&params);
		return NULL;
	}
	else
	{
		if(sURL == NULL) return NULL;

		Easy_HLSOpen_Params params;
		params.inRTSPUrl = (char*)sURL;
		params.inStreamName = (char*)sName;

		EasyHLSOpen(&params);
	}

	location = GetHLSUrl((char*)sName);

	QTSS_RTSPStatusCode statusCode = qtssRedirectPermMoved;
	QTSS_SetValue(inParams->inRTSPRequest, qtssRTSPReqStatusCode, 0, &statusCode, sizeof(statusCode));

	StrPtrLen locationRedirect(location);

	Bool16 sFalse = false;
	(void)QTSS_SetValue(inParams->inRTSPRequest, qtssRTSPReqRespKeepAlive, 0, &sFalse, sizeof(sFalse));
	QTSS_AppendRTSPHeader(inParams->inRTSPRequest, qtssLocationHeader, locationRedirect.Ptr, locationRedirect.Len);	
	return QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssRedirectPermMoved, 0);

	return QTSS_NoErr;
}