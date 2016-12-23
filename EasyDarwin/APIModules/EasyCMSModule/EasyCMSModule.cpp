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
#include "OSRef.h"
#include "QTSServerInterface.h"
#include "EasyCMSSession.h"

// STATIC DATA
static QTSS_PrefsObject				sServerPrefs = nullptr;
static QTSS_ServerObject			sServer = nullptr;
static QTSS_ModulePrefsObject		sEasyCMSModulePrefs = nullptr;

// FUNCTION PROTOTYPES
static QTSS_Error EasyCMSModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error Register_EasyCMSModule(QTSS_Register_Params* inParams);
static QTSS_Error Initialize_EasyCMSModule(QTSS_Initialize_Params* inParams);
static QTSS_Error RereadPrefs_EasyCMSModule();

//向EasyCMS发送MSG_CS_FREE_STREAM_REQ
static QTSS_Error FreeStream_EasyCMSModule(Easy_StreamInfo_Params* inParams);

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
	case Easy_CMSFreeStream_Role:
		return FreeStream_EasyCMSModule(&inParams->easyStreamInfoParams);
	default: break;
	}
	return QTSS_NoErr;
}

QTSS_Error Register_EasyCMSModule(QTSS_Register_Params* inParams)
{
	// Do role & attribute setup
	(void)QTSS_AddRole(QTSS_Initialize_Role);
	(void)QTSS_AddRole(QTSS_RereadPrefs_Role);
	(void)QTSS_AddRole(Easy_CMSFreeStream_Role);

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

	RereadPrefs_EasyCMSModule();

	return QTSS_NoErr;
}

QTSS_Error RereadPrefs_EasyCMSModule()
{
	return QTSS_NoErr;
}

//算法描述：
/**
 * \brief 动态创建EasyCMSSession对象，同时触发该对象向EasyCMS发送停止推流请求；
 * 然后等待EasyCMS的回应，如果EasyCMS正确回应，则析构EasyCMSSession对象如果在一定时间内没有收到
 * EasyCMS的回应，则进行重发（或者析构）
 * \param inParams
 * \return
 */
QTSS_Error FreeStream_EasyCMSModule(Easy_StreamInfo_Params* inParams)
{
	QTSS_Error theErr = QTSS_NoErr;

	if (inParams->inStreamName != NULL)
	{
		auto pCMSSession = new EasyCMSSession();
		theErr = pCMSSession->FreeStream(inParams->inStreamName);
		if (theErr == QTSS_NoErr)
			pCMSSession->Signal(Task::kStartEvent);
		else
			pCMSSession->Signal(Task::kKillEvent);
	}

	return theErr;
}