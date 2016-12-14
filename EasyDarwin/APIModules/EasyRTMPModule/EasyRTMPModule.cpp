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

#include "EasyRTMPModule.h"
#include "QTSSModuleUtils.h"
#include "QTSSMemoryDeleter.h"
#include "ReflectorSession.h"
#include "MyAssert.h"

#include "EasyRTMPSession.h"

#ifdef __Win32__
#define EasyRTMP_KEY "79397037795969576B5A734161443959703843576B76464659584E355247467964326C754C6D56345A534E58444661672F704C67523246326157346D516D466962334E68514449774D545A4659584E355247467964326C75564756686257566863336B3D"
#define EasyRTSPClient_KEY "6A59754D6A3469576B5A75414A553558714C485A4576464659584E355247467964326C754C6D56345A534E58444661672F704C67523246326157346D516D466962334E68514449774D545A4659584E355247467964326C75564756686257566863336B3D"
#else
#define EasyRTMP_KEY "79397037795A4F576B596F4161443959703843576B76566C59584E355A47467964326C75766C634D5671442B6B75424859585A7062695A4359574A76633246414D6A41784E6B566863336C4559584A33615735555A5746745A57467A65513D3D"
#define EasyRTSPClient_KEY "6A59754D6A354F576B5971414A553558714C485A4576566C59584E355A47467964326C753456634D5671442B6B75424859585A7062695A4359574A76633246414D6A41784E6B566863336C4559584A33615735555A5746745A57467A65513D3D"
#endif

// STATIC DATA
static OSRefTable*				sRTMPSessionMap	= nullptr;
static QTSS_PrefsObject			sServerPrefs = nullptr;
static QTSS_ServerObject		sServer = nullptr;
static QTSS_ModulePrefsObject	sPrefs = nullptr;

// FUNCTION PROTOTYPES
static QTSS_Error EasyRTMPModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error Register(QTSS_Register_Params* inParams);
static QTSS_Error Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error RereadPrefs();
static QTSS_Error GetDeviceStream(Easy_GetDeviceStream_Params* inParams);
static QTSS_Error LiveDeviceStream(Easy_GetDeviceStream_Params* inParams);

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
		case Easy_GetDeviceStream_Role:
			return GetDeviceStream(&inParams->easyGetDeviceStreamParams);
		case Easy_LiveDeviceStream_Role:
			return LiveDeviceStream(&inParams->easyGetDeviceStreamParams);
    default: break;
    }
    return QTSS_NoErr;
}

QTSS_Error Register(QTSS_Register_Params* inParams)
{
	auto isEasyRTMPActivated = EasyRTMP_Activate(EasyRTMP_KEY);
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
	default: break;
	}

	if (EASY_ACTIVATE_SUCCESS != isEasyRTMPActivated)
		return QTSS_RequestFailed;

	auto isEasyRTSPClientActivated = EasyRTSP_Activate(EasyRTSPClient_KEY);
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
	default: break;
	}

	if(EASY_ACTIVATE_SUCCESS != isEasyRTSPClientActivated)
		return QTSS_RequestFailed;

    // Do role & attribute setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_RereadPrefs_Role); 
	(void)QTSS_AddRole(Easy_GetDeviceStream_Role);
	(void)QTSS_AddRole(Easy_LiveDeviceStream_Role);
    
    // Tell the server our name!
    static char* sModuleName = "EasyRTMPModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    return QTSS_NoErr;
}

QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{
    // Setup module utils
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);
	sRTMPSessionMap = QTSServerInterface::GetServer()->GetRTMPSessionMap();

    sServerPrefs = inParams->inPrefs;
    sServer = inParams->inServer;
    sPrefs = QTSSModuleUtils::GetModulePrefsObject(inParams->inModule);

    RereadPrefs();
    
   return QTSS_NoErr;
}

QTSS_Error RereadPrefs()
{
	return QTSS_NoErr;
}

QTSS_Error GetDeviceStream(Easy_GetDeviceStream_Params* inParams)
{
	QTSS_Error theErr = QTSS_Unimplemented;

	while (inParams->inDevice && inParams->inStreamType == easyRTMPType)
	{
		char theStreamName[QTSS_MAX_NAME_LENGTH] = { 0 };
		sprintf(theStreamName, "%s%s%d", inParams->inDevice, EASY_KEY_SPLITER, inParams->inChannel);
		StrPtrLen inStreamName(theStreamName);

		EasyRTMPSession* rtmpSe;
		auto sessionRef = sRTMPSessionMap->Resolve(&inStreamName);
		if(sessionRef != nullptr)
		{
			rtmpSe = static_cast<EasyRTMPSession*>(sessionRef->GetObject());
		}
		else
		{
			auto rtspSessionMap = QTSServerInterface::GetServer()->GetReflectorSessionMap();
			OSMutexLocker locker(rtspSessionMap->GetMutex());
			auto theSessionRef = rtspSessionMap->Resolve(&inStreamName);
			ReflectorSession* theSession;

			if (theSessionRef == nullptr)
			{
				theErr = QTSS_FileNotFound;
				break;
			}

			theSession = static_cast<ReflectorSession*>(theSessionRef->GetObject());
			auto clientSession = theSession->GetBroadcasterSession();
			Assert(theSession != NULL);

			if (clientSession == nullptr)
			{
				theErr = QTSS_FileNotFound;
				break;
			}

			char* theFullRequestURL = nullptr;
			(void)QTSS_GetValueAsString(clientSession, qtssCliSesFullURL, 0, &theFullRequestURL);
			QTSSCharArrayDeleter theFileNameStrDeleter(theFullRequestURL);

			if (theFullRequestURL == nullptr)
			{
				theErr = QTSS_FileNotFound;
				break;
			}

			StrPtrLen inURL(theFullRequestURL);
			StrPtrLen inName(inParams->inDevice);
			rtmpSe = new EasyRTMPSession(&inName, &inURL, inParams->inChannel);

			auto stErr = rtmpSe->SessionStart();

			if (stErr == QTSS_NoErr)
			{
				auto regErr = sRTMPSessionMap->Register(rtmpSe->GetRef());
				Assert(regErr == QTSS_NoErr);
			}
			else
			{
				rtmpSe->Signal(Task::kKillEvent);
				theErr = QTSS_Unimplemented;
				break;
			}

			auto debug = sRTMPSessionMap->Resolve(&inStreamName);
			Assert(debug == rtmpSe->GetRef());

			
			rtspSessionMap->Release(theSessionRef);
		}

		if(inParams->outUrl)
			strcpy(inParams->outUrl, rtmpSe->GetRTMPURL());

		sRTMPSessionMap->Release(rtmpSe->GetRef());
		theErr = QTSS_NoErr;
		break;
	}

	return theErr;
}


QTSS_Error LiveDeviceStream(Easy_GetDeviceStream_Params* inParams)
{
	QTSS_Error theErr = QTSS_ValueNotFound;

	while (inParams->inDevice && inParams->inStreamType == easyRTMPType)
	{
		char theStreamName[QTSS_MAX_NAME_LENGTH] = { 0 };
		sprintf(theStreamName, "%s%s%d", inParams->inDevice, EASY_KEY_SPLITER, inParams->inChannel);
		StrPtrLen inStreamName(theStreamName);

		EasyRTMPSession* rtmpSe;
		auto sessionRef = sRTMPSessionMap->Resolve(&inStreamName);
		if (sessionRef != nullptr)
		{
			rtmpSe = static_cast<EasyRTMPSession*>(sessionRef->GetObject());
			rtmpSe->RefreshTimeout();
		}
		else
		{
			break;
		}

		sRTMPSessionMap->Release(rtmpSe->GetRef());
		theErr = QTSS_NoErr;
		break;
	}

	return theErr;
}