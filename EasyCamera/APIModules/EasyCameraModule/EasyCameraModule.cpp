/*
	Copyright (c) 2013-2016 EasyDarwin.ORG.  All rights reserved.
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