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

#include "EasyCameraModule.h"
#include "QTSSModuleUtils.h"
#include "OSRef.h"
#include "StringParser.h"
#include "QTSServerInterface.h"

#include "EasyCameraSource.h"
#include "EasyPusherAPI.h"

#ifdef __Win32__
#define EasyPusher_KEY "6A34714D6C3469576B5A7541787A4E58714D77334576464659584E35513246745A584A684C6D56345A536C58444661672F704C67523246326157346D516D466962334E68514449774D545A4659584E355247467964326C75564756686257566863336B3D"
#else
#define EasyPusher_KEY "6A34714D6C354F576B597141787A4E58714D77334576566C59584E35593246745A584A684931634D5671442B6B75424859585A7062695A4359574A76633246414D6A41784E6B566863336C4559584A33615735555A5746745A57467A65513D3D"
#endif

// STATIC DATA
static QTSS_PrefsObject				sServerPrefs			= NULL;	//服务器主配置
static QTSS_ServerObject			sServer					= NULL;	//服务器对象
static QTSS_ModulePrefsObject		sEasyCameraModulePrefs	= NULL;	//当前模块配置

static EasyCameraSource*			sCameraSource			= NULL; //唯一EasyCameraSource对象

// FUNCTION PROTOTYPES
static QTSS_Error EasyCameraModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error Register_EasyCameraModule(QTSS_Register_Params* inParams);
static QTSS_Error Initialize_EasyCameraModule(QTSS_Initialize_Params* inParams);
static QTSS_Error RereadPrefs_EasyCameraModule();

static QTSS_Error StartStream(Easy_StartStream_Params* inParams);
static QTSS_Error StopStream(Easy_StopStream_Params* inParams);


// FUNCTION IMPLEMENTATIONS
QTSS_Error EasyCameraModule_Main(void* inPrivateArgs)
{
    return _stublibrary_main(inPrivateArgs, EasyCameraModuleDispatch);
}

QTSS_Error  EasyCameraModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
    switch (inRole)
    {
        case QTSS_Register_Role:
            return Register_EasyCameraModule(&inParams->regParams);
        case QTSS_Initialize_Role:
            return Initialize_EasyCameraModule(&inParams->initParams);
        case QTSS_RereadPrefs_Role:
            return RereadPrefs_EasyCameraModule();
		case Easy_StartStream_Role:
			return StartStream(&inParams->startStreaParams);
	}
    return QTSS_NoErr;
}

QTSS_Error Register_EasyCameraModule(QTSS_Register_Params* inParams)
{
	//加载前进行授权验证
	int isEasyPusherActivated = EasyPusher_Activate(EasyPusher_KEY);
	switch(isEasyPusherActivated)
	{
	case EASY_ACTIVATE_INVALID_KEY:
		printf("EasyPusher_KEY is EASY_ACTIVATE_INVALID_KEY!\n");
		break;
	case EASY_ACTIVATE_TIME_ERR:
		printf("EasyPusher_KEY is EASY_ACTIVATE_TIME_ERR!\n");
		break;
	case EASY_ACTIVATE_PROCESS_NAME_LEN_ERR:
		printf("EasyPusher_KEY is EASY_ACTIVATE_PROCESS_NAME_LEN_ERR!\n");
		break;
	case EASY_ACTIVATE_PROCESS_NAME_ERR:
		printf("EasyPusher_KEY is EASY_ACTIVATE_PROCESS_NAME_ERR!\n");
		break;
	case EASY_ACTIVATE_VALIDITY_PERIOD_ERR:
		printf("EasyPusher_KEY is EASY_ACTIVATE_VALIDITY_PERIOD_ERR!\n");
		break;
	case EASY_ACTIVATE_SUCCESS:
		//printf("EasyPusher_KEY is EASY_ACTIVATE_SUCCESS!\n");
		break;
	}

	if(EASY_ACTIVATE_SUCCESS != isEasyPusherActivated)
		return QTSS_RequestFailed;

    // Do role & attribute setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_RereadPrefs_Role);
	(void)QTSS_AddRole(Easy_StartStream_Role);
   
    // Tell the server our name!
    static char* sModuleName = "EasyCameraModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    return QTSS_NoErr;
}

QTSS_Error Initialize_EasyCameraModule(QTSS_Initialize_Params* inParams)
{
    // Setup module utils
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);

    // Setup global data structures
    sServerPrefs = inParams->inPrefs;
    sServer = inParams->inServer;
    sEasyCameraModulePrefs = QTSSModuleUtils::GetModulePrefsObject(inParams->inModule);

	//读取EasyCMSModule配置
	RereadPrefs_EasyCameraModule();

	EasyCameraSource::Initialize(sEasyCameraModulePrefs);

	//创建并开始EasyCMSSession对象
	sCameraSource = new EasyCameraSource();
	sCameraSource->Signal(Task::kStartEvent);

    return QTSS_NoErr;
}

QTSS_Error RereadPrefs_EasyCameraModule()
{
	return QTSS_NoErr;
}

QTSS_Error StartStream(Easy_StartStream_Params* inParams)
{
	QTSS_Error theErr = QTSS_Unimplemented;

	if(sCameraSource)
	{
		theErr = sCameraSource->StartStreaming(inParams->inSerial, inParams->inChannel, inParams->inStreamID, inParams->inProtocol, inParams->inIP, inParams->inPort);
	}
	return theErr;
}