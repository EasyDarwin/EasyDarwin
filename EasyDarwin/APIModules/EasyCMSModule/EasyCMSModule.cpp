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
#include "ReflectorSession.h"

// STATIC DATA
static QTSS_PrefsObject				sServerPrefs		= NULL;	//服务器主配置
static QTSS_ServerObject			sServer				= NULL;	//服务器对象
static QTSS_ModulePrefsObject		sEasyCMSModulePrefs	= NULL;	//当前模块配置


// FUNCTION PROTOTYPES
static QTSS_Error EasyCMSModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error Register_EasyCMSModule(QTSS_Register_Params* inParams);
static QTSS_Error Initialize_EasyCMSModule(QTSS_Initialize_Params* inParams);
static QTSS_Error RereadPrefs_EasyCMSModule();

class ReflectorSessionCheckTask : public Task
{
public:
	ReflectorSessionCheckTask() : Task() {this->SetTaskName("ReflectorSessionCheckTask"); this->Signal(Task::kStartEvent); }
	virtual ~ReflectorSessionCheckTask() {}

private:
	virtual SInt64 Run();
};
static ReflectorSessionCheckTask *pTask=NULL;


SInt64 ReflectorSessionCheckTask::Run()
{
	OSRefTable* reflectorSessionMap = QTSServerInterface::GetServer()->GetReflectorSessionMap();

	OSMutexLocker locker(reflectorSessionMap->GetMutex());

	SInt64 sNowTime = OS::Milliseconds();
	for (OSRefHashTableIter theIter(reflectorSessionMap->GetHashTable()); !theIter.IsDone(); theIter.Next())
	{
		OSRef* theRef = theIter.GetCurrent();
		ReflectorSession* theSession = (ReflectorSession*)theRef->GetObject();

		SInt64  sCreateTime = theSession->GetInitTimeMS();
		if( (theSession->GetNumOutputs()==0) && (sNowTime-sCreateTime>=20*1000) )//当前客户端列表为0，并且是在ReflectorSession创建20秒之后
		{
			qtss_printf("没有客户端观看当前转发媒体\n");
			QTSS_RoleParams theParams;
			theParams.easyFreeStreamParams.inStreamName = theSession->GetSessionName();
			QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kFreeStreamRole, 0);
			(void)theModule->CallDispatch(Easy_FreeStream_Role, &theParams);
		}
	}  
	return 30*1000;//每30秒检查一次
}








//向EasyCMS发送MSG_CS_FREE_STREAM_REQ
static QTSS_Error FreeStream_EasyCMSModule(Easy_FreeStream_Params* inParams);

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
		case Easy_FreeStream_Role:
			return FreeStream_EasyCMSModule(&inParams->easyFreeStreamParams);
	}
    return QTSS_NoErr;
}

QTSS_Error Register_EasyCMSModule(QTSS_Register_Params* inParams)
{
    // Do role & attribute setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_RereadPrefs_Role);
	(void)QTSS_AddRole(Easy_FreeStream_Role);
   
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

	pTask = new ReflectorSessionCheckTask();
    return QTSS_NoErr;
}

QTSS_Error RereadPrefs_EasyCMSModule()
{
	return QTSS_NoErr;
}

//算法描述：动态创建EasyCMSSession对象，同时触发该对象向EasyCMS发送停止推流请求；然后等待EasyCMS的回应，如果EasyCMS正确回应，则析构EasyCMSSession对象
//如果在一定时间内没有收到EasyCMS的回应，则进行重发（或者析构）
QTSS_Error FreeStream_EasyCMSModule(Easy_FreeStream_Params* inParams)
{
	QTSS_Error theErr = QTSS_NoErr;

	if( inParams->inStreamName != NULL )
	{
		//创建并开始EasyCMSSession对象
		EasyCMSSession * pCMSSession = new EasyCMSSession();
		theErr = pCMSSession->FreeStream(inParams->inStreamName);
		if(theErr == QTSS_NoErr)
			pCMSSession->Signal(Task::kStartEvent);
		else
			pCMSSession->Signal(Task::kKillEvent);
	}

	return theErr;
}