/*
	Copyright (c) 2013-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       EasyRTMPModule.cpp
    Contains:   RTMP live
*/

#include "QTSServerInterface.h"
#include "EasyRTMPModule.h"
#include "QTSSModuleUtils.h"
#include "OSArrayObjectDeleter.h"
#include "QTSSMemoryDeleter.h"
#include "QueryParamList.h"
#include "OSRef.h"
#include "StringParser.h"

#include "EasyRTSPClientAPI.h"
#include "EasyRTMPAPI.h"
#include "EasyRTMPSession.h"

#ifdef __Win32__
#define EasyRTMP_KEY "79397037795969576B5A734161443959703843576B76464659584E355247467964326C754C6D56345A534E58444661672F704C67523246326157346D516D466962334E68514449774D545A4659584E355247467964326C75564756686257566863336B3D"
#define EasyRTSPClient_KEY "6A59754D6A3469576B5A75414A553558714C485A4576464659584E355247467964326C754C6D56345A534E58444661672F704C67523246326157346D516D466962334E68514449774D545A4659584E355247467964326C75564756686257566863336B3D"
#else
#define EasyRTMP_KEY "79397037795A4F576B596F4161443959703843576B76566C59584E355A47467964326C75766C634D5671442B6B75424859585A7062695A4359574A76633246414D6A41784E6B566863336C4559584A33615735555A5746745A57467A65513D3D"
#define EasyRTSPClient_KEY "6A59754D6A354F576B5971414A553558714C485A4576566C59584E355A47467964326C753456634D5671442B6B75424859585A7062695A4359574A76633246414D6A41784E6B566863336C4559584A33615735555A5746745A57467A65513D3D"
#endif

// STATIC DATA
static OSRefTable*				sRTMPSessionMap	= NULL;
static QTSS_PrefsObject			sServerPrefs = NULL;
static QTSS_ServerObject		sServer = NULL;
static QTSS_ModulePrefsObject	sPrefs = NULL;

static char*					sLocal_IP_Addr = NULL;
static char*					sDefaultLocal_IP_Addr = "127.0.0.1";

// FUNCTION PROTOTYPES
static QTSS_Error EasyRTMPModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error Register(QTSS_Register_Params* inParams);
static QTSS_Error Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error RereadPrefs();

static QTSS_Error DoDescribe(QTSS_StandardRTSP_Params* inParams);

// FUNCTION IMPLEMENTATIONS
QTSS_Error EasyRTMPModule_Main(void* inPrivateArgs)
{
    return _stublibrary_main(inPrivateArgs, EasyRTMPModuleDispatch);
}

QTSS_Error  EasyRTMPModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
    switch (inRole)
    {
        case QTSS_Register_Role:
            return Register(&inParams->regParams);
        case QTSS_Initialize_Role:
            return Initialize(&inParams->initParams);
        case QTSS_RereadPrefs_Role:
            return RereadPrefs();
   }
    return QTSS_NoErr;
}

QTSS_Error Register(QTSS_Register_Params* inParams)
{
	int isEasyRTMPActivated = EasyRTMP_Activate(EasyRTMP_KEY);
	switch (isEasyRTMPActivated)
	{
	case EASY_ACTIVATE_INVALID_KEY:
		printf("EasyRTMP_KEY is EASY_ACTIVATE_INVALID_KEY!");
		break;
	case EASY_ACTIVATE_TIME_ERR:
		printf("EasyRTMP_KEY is EASY_ACTIVATE_TIME_ERR!");
		break;
	case EASY_ACTIVATE_PROCESS_NAME_LEN_ERR:
		printf("EasyRTMP_KEY is EASY_ACTIVATE_PROCESS_NAME_LEN_ERR!");
		break;
	case EASY_ACTIVATE_PROCESS_NAME_ERR:
		printf("EasyRTMP_KEY is EASY_ACTIVATE_PROCESS_NAME_ERR!");
		break;
	case EASY_ACTIVATE_VALIDITY_PERIOD_ERR:
		printf("EasyRTMP_KEY is EASY_ACTIVATE_VALIDITY_PERIOD_ERR!");
		break;
	case EASY_ACTIVATE_SUCCESS:
		printf("EasyRTMP_KEY is EASY_ACTIVATE_SUCCESS!");
		break;
	}

	if (EASY_ACTIVATE_SUCCESS != isEasyRTMPActivated)
		return QTSS_RequestFailed;

	int isEasyRTSPClientActivated = EasyRTSP_Activate(EasyRTSPClient_KEY);
	switch(isEasyRTSPClientActivated)
	{
	case EASY_ACTIVATE_INVALID_KEY:
		printf("EasyRTSPClient_KEY is EASY_ACTIVATE_INVALID_KEY!\n");
		break;
	case EASY_ACTIVATE_TIME_ERR:
		printf("EasyRTSPClient_KEY is EASY_ACTIVATE_TIME_ERR!\n");
		break;
	case EASY_ACTIVATE_PROCESS_NAME_LEN_ERR:
		printf("EasyRTSPClient_KEY is EASY_ACTIVATE_PROCESS_NAME_LEN_ERR!\n");
		break;
	case EASY_ACTIVATE_PROCESS_NAME_ERR:
		printf("EasyRTSPClient_KEY is EASY_ACTIVATE_PROCESS_NAME_ERR!\n");
		break;
	case EASY_ACTIVATE_VALIDITY_PERIOD_ERR:
		printf("EasyRTSPClient_KEY is EASY_ACTIVATE_VALIDITY_PERIOD_ERR!\n");
		break;
	case EASY_ACTIVATE_SUCCESS:
		//printf("EasyRTSPClient_KEY is EASY_ACTIVATE_SUCCESS!\n");
		break;
	}

	if(EASY_ACTIVATE_SUCCESS != isEasyRTSPClientActivated)
		return QTSS_RequestFailed;

    // Do role & attribute setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_RereadPrefs_Role);    
    
    // Tell the server our name!
    static char* sModuleName = "EasyRTMPModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    return QTSS_NoErr;
}

QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{
    // Setup module utils
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);
	sRTMPSessionMap = NEW OSRefTable();

    sServerPrefs = inParams->inPrefs;
    sServer = inParams->inServer;
    sPrefs = QTSSModuleUtils::GetModulePrefsObject(inParams->inModule);

    RereadPrefs();

	EasyRTMPSession* se = NEW EasyRTMPSession(&StrPtrLen("TEST"), &StrPtrLen("rtsp://192.168.66.138:554/and_111.sdp"), 0);
	se->SessionStart();
    
   return QTSS_NoErr;
}

QTSS_Error RereadPrefs()
{
	delete [] sLocal_IP_Addr;
    sLocal_IP_Addr = QTSSModuleUtils::GetStringAttribute(sPrefs, "local_ip_address", sDefaultLocal_IP_Addr);

	return QTSS_NoErr;
}


QTSS_Error DoDescribe(QTSS_StandardRTSP_Params* inParams)
{
	return 0;
 //   char* theFullPathStr = NULL;
 //   QTSS_Error theErr = QTSS_GetValueAsString(inParams->inRTSPRequest, qtssRTSPReqFileName, 0, &theFullPathStr);
 //   Assert(theErr == QTSS_NoErr);
 //   QTSSCharArrayDeleter theFullPathStrDeleter(theFullPathStr);
 //       
 //   if (theErr != QTSS_NoErr)
 //       return NULL;

 //   StrPtrLen theFullPath(theFullPathStr);


	//StrPtrLen endOfPath2(&theFullPath.Ptr[theFullPath.Len -  sRelaySuffix.Len], sRelaySuffix.Len);
 //   if (!endOfPath2.Equal(sRelaySuffix))
 //   {   
 //       return NULL;
 //   }

	////解析查询字符串
 //   char* theQueryStr = NULL;
 //   theErr = QTSS_GetValueAsString(inParams->inRTSPRequest, qtssRTSPReqQueryString, 0, &theQueryStr);
 //   Assert(theErr == QTSS_NoErr);
 //   QTSSCharArrayDeleter theQueryStringDeleter(theQueryStr);
 //       
 //   if (theErr != QTSS_NoErr)
 //       return NULL;

 //   StrPtrLen theQueryString(theQueryStr);

	//QueryParamList parList(theQueryStr);

	//const char* sName = parList.DoFindCGIValueForParam(QUERY_STREAM_NAME);
	//if(sName == NULL) return NULL;

	//const char* sURL = parList.DoFindCGIValueForParam(QUERY_STREAM_URL);
	////if(sURL == NULL) return NULL;

	//const char* sCMD = parList.DoFindCGIValueForParam(QUERY_STREAM_CMD);

	//bool bStop = false;
	//if(sCMD)
	//{
	//	if(::strcmp(sCMD,QUERY_STREAM_CMD_STOP) == 0)
	//		bStop = true;
	//}

	//StrPtrLen streamName((char*)sName);
	////从接口获取信息结构体
	//EasyRTMPSession* session = NULL;
	////首先查找Map里面是否已经有了对应的流
	//OSRef* sessionRef = sRTMPSessionMap->Resolve(&streamName);
	//if(sessionRef != NULL)
	//{
	//	session = (EasyRTMPSession*)sessionRef->GetObject();
	//}
	//else
	//{
	//	if(bStop) return NULL;

	//	if(sURL == NULL) return NULL;

	//	session = NEW EasyRTMPSession((char*)sURL, EasyRTMPSession::kRTSPTCPClientType, (char*)sName);

	//	QTSS_Error theErr = session->RelaySessionStart();

	//	if(theErr == QTSS_NoErr)
	//	{
	//		OS_Error theErr = sRTMPSessionMap->Register(session->GetRef());
	//		Assert(theErr == QTSS_NoErr);
	//	}
	//	else
	//	{
	//		session->Signal(Task::kKillEvent);
	//		return QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssClientNotFound, 0); 
	//	}

	//	//增加一次对RelaySession的无效引用，后面会统一释放
	//	OSRef* debug = sRTMPSessionMap->Resolve(&streamName);
	//	Assert(debug == session->GetRef());
	//}

	//sRTMPSessionMap->Release(session->GetRef());

	//if(bStop)
	//{
	//	sRTMPSessionMap->UnRegister(session->GetRef());
	//	session->Signal(Task::kKillEvent);
	//	return QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssSuccessOK, 0); 
	//}

	//QTSS_RTSPStatusCode statusCode = qtssRedirectPermMoved;
	//QTSS_SetValue(inParams->inRTSPRequest, qtssRTSPReqStatusCode, 0, &statusCode, sizeof(statusCode));

	//// Get the ip addr out of the prefs dictionary
	//UInt16 thePort = 554;
	//UInt32 theLen = sizeof(UInt16);
	//theErr = QTSServerInterface::GetServer()->GetPrefs()->GetValue(qtssPrefsRTSPPorts, 0, &thePort, &theLen);
	//Assert(theErr == QTSS_NoErr);   

	////构造本地URL
	//char url[QTSS_MAX_URL_LENGTH] = { 0 };

	//qtss_sprintf(url,"rtsp://%s:%d/%s.sdp", sLocal_IP_Addr, thePort, sName);
	//StrPtrLen locationRedirect(url);
}