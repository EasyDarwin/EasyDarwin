/*
	Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       EasyCMSModule.cpp
    Contains:   Implementation of EasyCMSModule class. 
*/

#include "EasyCMSModule.h"
#include "QTSSModuleUtils.h"
#include "QueryParamList.h"
#include "OSRef.h"
#include "StringParser.h"
#include "QTSServerInterface.h"
#include "EasyCMSSession.h"

// STATIC DATA
static QTSS_PrefsObject				sServerPrefs		= NULL;	//服务器主配置
static QTSS_ServerObject			sServer				= NULL;	//服务器对象
static QTSS_ModulePrefsObject		sEasyCMSModulePrefs	= NULL;	//当前模块配置


// FUNCTION PROTOTYPES
static QTSS_Error EasyCMSModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error Register_EasyCMSModule(QTSS_Register_Params* inParams);
static QTSS_Error Initialize_EasyCMSModule(QTSS_Initialize_Params* inParams);
static QTSS_Error RereadPrefs_EasyCMSModule();

//向CMS发送设备快照
static QTSS_Error StreamStop_EasyCMSModule(Easy_StreamStop_Params* inParams);

// FUNCTION IMPLEMENTATIONS
QTSS_Error EasyCMSModule_Main(void* inPrivateArgs)
{
    return _stublibrary_main(inPrivateArgs, EasyCMSModuleDispatch);
}

QTSS_Error  EasyCMSModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
    switch (inRole)
    {
        case QTSS_Register_Role:
            return Register_EasyCMSModule(&inParams->regParams);
        case QTSS_Initialize_Role:
            return Initialize_EasyCMSModule(&inParams->initParams);
        case QTSS_RereadPrefs_Role:
            return RereadPrefs_EasyCMSModule();
		case Easy_StreamStop_Role:
			return StreamStop_EasyCMSModule(&inParams->easyStreamStopParams);
	}
    return QTSS_NoErr;
}

QTSS_Error Register_EasyCMSModule(QTSS_Register_Params* inParams)
{
    // Do role & attribute setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_RereadPrefs_Role);
	(void)QTSS_AddRole(Easy_StreamStop_Role);
   
    // Tell the server our name!
    static char* sModuleName = "EasyCMSModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    return QTSS_NoErr;
}

QTSS_Error Initialize_EasyCMSModule(QTSS_Initialize_Params* inParams)
{
    // Setup module utils
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);

    // Setup global data structures
    sServerPrefs = inParams->inPrefs;
    sServer = inParams->inServer;
    sEasyCMSModulePrefs = QTSSModuleUtils::GetModulePrefsObject(inParams->inModule);

	//读取EasyCMSModule配置
	RereadPrefs_EasyCMSModule();

    return QTSS_NoErr;
}

QTSS_Error RereadPrefs_EasyCMSModule()
{
	return QTSS_NoErr;
}

//算法描述：动态创建EasyCMSSession对象，同时触发该对象向EasyCMS发送停止推流请求；然后等待EasyCMS的回应，如果EasyCMS正确回应，则析构EasyCMSSession对象
//如果在一定时间内没有收到EasyCMS的回应，则进行重发（或者析构）
QTSS_Error StreamStop_EasyCMSModule(Easy_StreamStop_Params* inParams)
{
	QTSS_Error theErr = QTSS_Unimplemented;


	if((inParams->inSerial != NULL) && (inParams->inChannel != NULL) )
	{
		//创建并开始EasyCMSSession对象
		EasyCMSSession * pCMSSession = new EasyCMSSession();
		pCMSSession->SetStreamStopInfo(inParams->inSerial,inParams->inChannel);
		pCMSSession->SetMsgType(EasyCMSSession::kStreamStopMsg);
		pCMSSession->Signal(Task::kStartEvent);
	}

	return theErr;
}