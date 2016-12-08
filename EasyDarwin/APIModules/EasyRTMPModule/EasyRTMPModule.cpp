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
#include "OSRef.h"
#include "StringParser.h"
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
static OSRefTable*				sRTMPSessionMap	= NULL;
static QTSS_PrefsObject			sServerPrefs = NULL;
static QTSS_ServerObject		sServer = NULL;
static QTSS_ModulePrefsObject	sPrefs = NULL;

// FUNCTION PROTOTYPES
static QTSS_Error EasyRTMPModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error Register(QTSS_Register_Params* inParams);
static QTSS_Error Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error RereadPrefs();
static QTSS_Error GetDeviceStream(Easy_GetDeviceStream_Params* inParams);

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
	(void)QTSS_AddRole(Easy_GetDeviceStream_Role);
    
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

		EasyRTMPSession* rtmpSe = NULL;
		OSRef* sessionRef = sRTMPSessionMap->Resolve(&inStreamName);
		if(sessionRef != NULL)
		{
			rtmpSe = (EasyRTMPSession*)sessionRef->GetObject();
		}
		else
		{
			OSRefTable* rtspSessionMap = QTSServerInterface::GetServer()->GetReflectorSessionMap();
			OSMutexLocker locker(rtspSessionMap->GetMutex());
			OSRef* theSessionRef = rtspSessionMap->Resolve(&inStreamName);
			ReflectorSession* theSession = NULL;

			if (theSessionRef == NULL)
			{
				theErr = QTSS_FileNotFound;
				break;
			}

			theSession = (ReflectorSession*)theSessionRef->GetObject();
			QTSS_ClientSessionObject clientSession = theSession->GetBroadcasterSession();
			Assert(theSession != NULL);

			if (clientSession == NULL)
			{
				theErr = QTSS_FileNotFound;
				break;
			}

			char* theFullRequestURL = NULL;
			(void)QTSS_GetValueAsString(clientSession, qtssCliSesFullURL, 0, &theFullRequestURL);
			QTSSCharArrayDeleter theFileNameStrDeleter(theFullRequestURL);

			if (theFullRequestURL == NULL)
			{
				theErr = QTSS_FileNotFound;
				break;
			}

			StrPtrLen inURL(theFullRequestURL);
			StrPtrLen inName(inParams->inDevice);
			rtmpSe = NEW EasyRTMPSession(&inName, &inURL, inParams->inChannel);

			QTSS_Error theErr = rtmpSe->SessionStart();

			if (theErr == QTSS_NoErr)
			{
				OS_Error theErr = sRTMPSessionMap->Register(rtmpSe->GetRef());
				Assert(theErr == QTSS_NoErr);
			}
			else
			{
				rtmpSe->Signal(Task::kKillEvent);
				theErr = QTSS_Unimplemented;
				break;
			}

			OSRef* debug = sRTMPSessionMap->Resolve(&inStreamName);
			Assert(debug == rtmpSe->GetRef());

			
			rtspSessionMap->Release(theSessionRef);
		}

		strcpy(inParams->outUrl, rtmpSe->GetRTMPURL());
		sRTMPSessionMap->Release(rtmpSe->GetRef());
		theErr = QTSS_NoErr;
		break;
	}

	return theErr;
}