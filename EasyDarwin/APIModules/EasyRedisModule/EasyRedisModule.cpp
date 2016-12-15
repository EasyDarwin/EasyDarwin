#include <stdio.h>

#include "EasyRedisModule.h"
#include "OSHeaders.h"
#include "QTSSModuleUtils.h"
#include "EasyRedisClient.h"
#include "QTSServerInterface.h"
#include "ReflectorSession.h"

// STATIC VARIABLES
static QTSS_ModulePrefsObject	modulePrefs = nullptr;
static QTSS_PrefsObject			sServerPrefs = nullptr;
static QTSS_ServerObject		sServer = nullptr;

// Redis IP
static char*            sRedis_IP = nullptr;
static char*            sDefaultRedis_IP_Addr = "127.0.0.1";
// Redis Port
static UInt16			sRedisPort = 6379;
static UInt16			sDefaultRedisPort = 6379;
// Redis password
static char*            sRedisPassword = nullptr;
static char*            sDefaultRedisPassword = "admin";

static EasyRedisClient* sRedisClient = nullptr;//the object pointer that package the redis operation
static bool				sIfConSucess = false;
static OSMutex			sMutex;

// FUNCTION PROTOTYPES
static QTSS_Error   EasyRedisModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParamBlock);
static QTSS_Error   Register(QTSS_Register_Params* inParams);
static QTSS_Error   Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error   RereadPrefs();
static QTSS_Error	RedisConnect();
static QTSS_Error	RedisInit();
static QTSS_Error	RedisTTL();
static QTSS_Error	RedisUpdateStream(Easy_StreamInfo_Params* inParams);
static QTSS_Error	RedisChangeRtpNum();
static QTSS_Error	RedisGetAssociatedCMS(QTSS_GetAssociatedCMS_Params* inParams);
static QTSS_Error	RedisJudgeStreamID(QTSS_JudgeStreamID_Params* inParams);

QTSS_Error EasyRedisModule_Main(void* inPrivateArgs)
{
	return _stublibrary_main(inPrivateArgs, EasyRedisModuleDispatch);
}

QTSS_Error EasyRedisModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParamBlock)
{
	switch (inRole)
	{
	case QTSS_Register_Role:
		return Register(&inParamBlock->regParams);
	case QTSS_Initialize_Role:
		return Initialize(&inParamBlock->initParams);
	case QTSS_RereadPrefs_Role:
		return RereadPrefs();
	case Easy_RedisTTL_Role:
		return RedisTTL();
	case Easy_RedisChangeRTPNum_Role:
		return RedisChangeRtpNum();
	case Easy_RedisUpdateStreamInfo_Role:
		return RedisUpdateStream(&inParamBlock->easyStreamInfoParams);
	case Easy_RedisGetAssociatedCMS_Role:
		return RedisGetAssociatedCMS(&inParamBlock->GetAssociatedCMSParams);
	case Easy_RedisJudgeStreamID_Role:
		return RedisJudgeStreamID(&inParamBlock->JudgeStreamIDParams);
	default: break;
	}
	return QTSS_NoErr;
}

QTSS_Error Register(QTSS_Register_Params* inParams)
{
	// Do role setup
	(void)QTSS_AddRole(QTSS_Initialize_Role);
	(void)QTSS_AddRole(QTSS_RereadPrefs_Role);
	(void)QTSS_AddRole(Easy_RedisTTL_Role);
	(void)QTSS_AddRole(Easy_RedisChangeRTPNum_Role);
	(void)QTSS_AddRole(Easy_RedisUpdateStreamInfo_Role);
	(void)QTSS_AddRole(Easy_RedisGetAssociatedCMS_Role);
	(void)QTSS_AddRole(Easy_RedisJudgeStreamID_Role);

	// Tell the server our name!
	static char* sModuleName = "EasyRedisModule";
	::strcpy(inParams->outModuleName, sModuleName);

	return QTSS_NoErr;
}

QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{
	QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);
	sServer = inParams->inServer;
	sServerPrefs = inParams->inPrefs;
	modulePrefs = QTSSModuleUtils::GetModulePrefsObject(inParams->inModule);

	sRedisClient = new EasyRedisClient();

	RereadPrefs();

	RedisConnect();

	return QTSS_NoErr;
}

QTSS_Error RereadPrefs()
{
	delete[] sRedis_IP;
	sRedis_IP = QTSSModuleUtils::GetStringAttribute(modulePrefs, "redis_ip", sDefaultRedis_IP_Addr);

	QTSSModuleUtils::GetAttribute(modulePrefs, "redis_port", qtssAttrDataTypeUInt16, &sRedisPort, &sDefaultRedisPort, sizeof(sRedisPort));

	delete[] sRedisPassword;
	sRedisPassword = QTSSModuleUtils::GetStringAttribute(modulePrefs, "redis_password", sDefaultRedisPassword);

	return QTSS_NoErr;

}


QTSS_Error RedisConnect()
{
	if (sIfConSucess)
		return QTSS_NoErr;

	std::size_t timeout = 1;//timeout second
	if (sRedisClient->ConnectWithTimeOut(sRedis_IP, sRedisPort, timeout) == EASY_REDIS_OK)//return 0 if connect sucess
	{
		qtss_printf("Connect redis sucess\n");
		sIfConSucess = true;
		std::size_t timeoutSocket = 1;//timeout socket second
		sRedisClient->SetTimeout(timeoutSocket);
		RedisInit();
	}
	else
	{
		qtss_printf("Connect redis failed\n");
		sIfConSucess = false;
	}
	return static_cast<QTSS_Error>(!sIfConSucess);
}

QTSS_Error RedisInit()//only called by RedisConnect after connect redis sucess
{
	char chTemp[128] = { 0 };

	sprintf(chTemp, "auth %s", sRedisPassword);
	sRedisClient->AppendCommand(chTemp);

	easyRedisReply* reply = nullptr;

	if (EASY_REDIS_OK != sRedisClient->GetReply(reinterpret_cast<void**>(&reply)))
	{
		if (reply)
			EasyFreeReplyObject(reply);	
		sRedisClient->Free();
		sIfConSucess = false;
		return static_cast<QTSS_Error>(false);
	}

	EasyFreeReplyObject(reply);
	return QTSS_NoErr;
}

QTSS_Error RedisUpdateStream(Easy_StreamInfo_Params* inParams)
{
	OSMutexLocker mutexLock(&sMutex);
	if (!sIfConSucess)
		return QTSS_NotConnected;

	auto ret = 0;
	char chKey[128] = { 0 };
	sprintf(chKey, "%s:%s/%d", "Live", inParams->inStreamName, inParams->inChannel);

	if (inParams->inAction == easyRedisActionDelete)
	{
		sRedisClient->Delete(chKey);
		if (ret == -1)//fatal err,need reconnect
		{
			sRedisClient->Free();
			sIfConSucess = false;
		}
		return ret;
	}

	char chRtpNum[16] = { 0 };
	sprintf(chRtpNum, "%d", inParams->inNumOutputs);

	ret = sRedisClient->HashSet(chKey, "output", chRtpNum);
	if (ret == -1)//fatal err,need reconnect
	{
		sRedisClient->Free();
		sIfConSucess = false;
	}

	ret = sRedisClient->HashSet(chKey, "EasyDarwin", QTSServerInterface::GetServer()->GetCloudServiceNodeID());
	if (ret == -1)//fatal err,need reconnect
	{
		sRedisClient->Free();
		sIfConSucess = false;
	}

	return ret;
}

QTSS_Error RedisTTL()//注意当网络在一段时间很差时可能会因为超时时间达到而导致key被删除，这时应该重新设置该key
{

	OSMutexLocker mutexLock(&sMutex);

	if (RedisConnect() != QTSS_NoErr)//每一次执行命令之前都先连接redis,如果当前redis还没有成功连接
		return QTSS_NotConnected;

	char chKey[128] = { 0 };//注意128位是否足够
	sprintf(chKey, "%s:%s", QTSServerInterface::GetServer()->GetServerName().Ptr, QTSServerInterface::GetServer()->GetCloudServiceNodeID());//更改超时时间

	auto ret = sRedisClient->SetExpire(chKey, 15);
	if (ret == -1)//fatal error
	{
		sRedisClient->Free();
		sIfConSucess = false;
		return QTSS_NotConnected;
	}
	else if (ret == 1)
	{
		return QTSS_NoErr;
	}
	else if (ret == 0)//the key doesn't exist, reset
	{
		int retret = sRedisClient->SetEX(chKey, 15, "1");
		if (retret == -1)//fatal error
		{
			sRedisClient->Free();
			sIfConSucess = false;
		}
		return retret;
	}
	else
	{
		return ret;
	}
}

QTSS_Error RedisGetAssociatedCMS(QTSS_GetAssociatedCMS_Params* inParams)
{
	OSMutexLocker mutexLock(&sMutex);

	if (!sIfConSucess)
		return QTSS_NotConnected;

	char chTemp[128] = { 0 };

	//1. get the list of EasyDarwin
	easyRedisReply * reply = static_cast<easyRedisReply *>(sRedisClient->SMembers("EasyCMSName"));
	if (reply == nullptr)
	{
		sRedisClient->Free();
		sIfConSucess = false;
		return QTSS_NotConnected;
	}

	//2.judge if the EasyCMS is ilve and contain serial  device
	if ((reply->elements > 0) && (reply->type == EASY_REDIS_REPLY_ARRAY))
	{
		easyRedisReply* childReply;
		for (size_t i = 0; i < reply->elements; i++)
		{
			childReply = reply->element[i];
			std::string strChileReply(childReply->str);

			sprintf(chTemp, "exists %s", (strChileReply + "_Live").c_str());
			sRedisClient->AppendCommand(chTemp);

			sprintf(chTemp, "sismember %s %s", (strChileReply + "_DevName").c_str(), inParams->inSerial);
			sRedisClient->AppendCommand(chTemp);
		}

		easyRedisReply *reply2 = nullptr, *reply3 = nullptr;
		for (size_t i = 0; i < reply->elements; i++)
		{
			if (sRedisClient->GetReply(reinterpret_cast<void**>(&reply2)) != EASY_REDIS_OK)
			{
				EasyFreeReplyObject(reply);
				if (reply2)
				{
					EasyFreeReplyObject(reply2);
				}
				sRedisClient->Free();
				sIfConSucess = false;
				return QTSS_NotConnected;
			}
			if (sRedisClient->GetReply(reinterpret_cast<void**>(&reply3)) != EASY_REDIS_OK)
			{
				EasyFreeReplyObject(reply);
				if (reply3)
				{
					EasyFreeReplyObject(reply3);
				}
				sRedisClient->Free();
				sIfConSucess = false;
				return QTSS_NotConnected;
			}

			if ((reply2->type == EASY_REDIS_REPLY_INTEGER) && (reply2->integer == 1) &&
				(reply3->type == EASY_REDIS_REPLY_INTEGER) && (reply3->integer == 1))
			{//find it
				std::string strIpPort(reply->element[i]->str);
				int ipos = strIpPort.find(':');//judge error
				memcpy(inParams->outCMSIP, strIpPort.c_str(), ipos);
				memcpy(inParams->outCMSPort, &strIpPort[ipos + 1], strIpPort.size() - ipos - 1);
				//break;//can't break,as 1 to 1
			}
			EasyFreeReplyObject(reply2);
			EasyFreeReplyObject(reply3);
		}
	}
	EasyFreeReplyObject(reply);
	return QTSS_NoErr;
}

QTSS_Error RedisChangeRtpNum()
{
	OSMutexLocker mutexLock(&sMutex);
	if (!sIfConSucess)
		return QTSS_NotConnected;

	char chKey[128] = { 0 };
	sprintf(chKey, "%s:%d_Info", QTSServerInterface::GetServer()->GetPrefs()->GetServiceWANIP(), QTSServerInterface::GetServer()->GetPrefs()->GetRTSPWANPort());//hset对RTP属性进行覆盖更新

	char chRtpNum[16] = { 0 };
	sprintf(chRtpNum, "%d", QTSServerInterface::GetServer()->GetNumRTPSessions());

	int ret = sRedisClient->HashSet(chKey, "RTP", chRtpNum);
	if (ret == -1)//fatal err,need reconnect
	{
		sRedisClient->Free();
		sIfConSucess = false;
	}

	return ret;
}

QTSS_Error RedisJudgeStreamID(QTSS_JudgeStreamID_Params* inParams)
{
	//算法描述，删除指定sessionID对应的key，如果成功删除，表明SessionID存在，验证通过，否则验证失败
	OSMutexLocker mutexLock(&sMutex);
	if (!sIfConSucess)
		return QTSS_NotConnected;

	char chKey[128] = { 0 };
	sprintf(chKey, "SessionID_%s", inParams->inStreanID);//如果key存在则返回整数类型1，否则返回整数类型0

	int ret = sRedisClient->Delete(chKey);

	if (ret == -1)//fatal err,need reconnect
	{
		sRedisClient->Free();
		sIfConSucess = false;

		return QTSS_NotConnected;
	}
	else if (ret == 0)
	{
		*(inParams->outresult) == 1;
		return QTSS_NoErr;
	}
	else
	{
		return ret;
	}
}