#include <stdio.h>

#include "EasyRedisModule.h"
#include "OSHeaders.h"
#include "QTSSModuleUtils.h"
#include "QTSServerInterface.h"
#include "ReflectorSession.h"
#include "EasyUtil.h"

#include "Windows/hiredis.h"

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

static bool				sIfConSucess = false;
static OSMutex			sMutex;

static redisContext*	redisContext_ = nullptr;

static void RedisErrorHandler(function<void()> func);


// FUNCTION PROTOTYPES
static QTSS_Error   EasyRedisModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParamBlock);
static QTSS_Error   Register(QTSS_Register_Params* inParams);
static QTSS_Error   Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error   RereadPrefs();
static QTSS_Error	RedisConnect();
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
	{
		return QTSS_NoErr;
	}

	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	redisContext_ = redisConnectWithTimeout(sRedis_IP, sRedisPort, timeout);
	if (!redisContext_ || redisContext_->err)
	{
		if (redisContext_)
		{
			printf("Connection error: %s\n", redisContext_->errstr);
			sIfConSucess = false;
			redisFree(redisContext_);
		}
		else
		{
			printf("Connection error: can't allocate redis context\n");
		}

		return QTSS_NotConnected;
	}

	sIfConSucess = true;

	char chKey[128] = { 0 };
	sprintf(chKey, "auth %s", sRedisPassword);

	auto reply = static_cast<redisReply*>(redisCommand(redisContext_, chKey));
	if (!reply || string(reply->str) != string("OK"))
	{
		RedisErrorHandler([&]()
		{
			if (reply)
			{
				freeReplyObject(reply);
			}
		});

		return QTSS_NotConnected;
	}
	freeReplyObject(reply);

	return QTSS_NoErr;
}

QTSS_Error RedisTTL()
{
	OSMutexLocker mutexLock(&sMutex);

	if (RedisConnect() != QTSS_NoErr)
	{
		return QTSS_NotConnected;
	}
	auto server = QTSServerInterface::GetServerName().Ptr;
	auto guid = QTSServerInterface::GetServer()->GetCloudServiceNodeID();
	auto load = QTSServerInterface::GetServer()->GetNumRTPSessions();

	char chKey[128] = { 0 };
	sprintf(chKey, "expire %s:%s 15", server, guid);
	auto reply = static_cast<redisReply*>(redisCommand(redisContext_, chKey));

	if (!reply)
	{
		RedisErrorHandler([&]() {});

		return QTSS_NotConnected;
	}

	if (reply->integer == 0)
	{
		auto ip = QTSServerInterface::GetServer()->GetPrefs()->GetServiceWANIP();
		auto http = QTSServerInterface::GetServer()->GetPrefs()->GetServiceWanPort();
		auto rtsp = QTSServerInterface::GetServer()->GetPrefs()->GetRTSPWANPort();

		sprintf(chKey, "hmset %s:%s IP %s HTTP %d RTSP %d Load %d", server, guid, ip, http, rtsp, load);
		auto replyHmset = static_cast<redisReply*>(redisCommand(redisContext_, chKey));
		if (!replyHmset)
		{
			RedisErrorHandler([&]()
			{
				freeReplyObject(reply);
			});

			return QTSS_NotConnected;
		}
		freeReplyObject(replyHmset);

		sprintf(chKey, "expire %s:%s 15", server, guid);
		auto replyExpire = static_cast<redisReply*>(redisCommand(redisContext_, chKey));
		if (!replyExpire)
		{
			RedisErrorHandler([&]()
			{
				freeReplyObject(reply);
				freeReplyObject(replyHmset);
			});

			return QTSS_NotConnected;
		}
		freeReplyObject(replyExpire);
	}
	else if (reply->integer == 1)
	{
		sprintf(chKey, "hset %s:%s Load %d", server, guid, load);
		auto replyHset = static_cast<redisReply*>(redisCommand(redisContext_, chKey));
		if (!replyHset)
		{
			RedisErrorHandler([&]()
			{
				freeReplyObject(reply);
			});

			return QTSS_NotConnected;
		}
		freeReplyObject(replyHset);
	}
	freeReplyObject(reply);

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
		sprintf(chKey, "hdel %s", chKey);
		auto reply = static_cast<redisReply*>(redisCommand(redisContext_, chKey));
		if (!reply)
		{
			RedisErrorHandler([&]() {});

			return QTSS_NotConnected;
		}
		return QTSS_NoErr;
	}

	char chTemp[128] = { 0 };
	sprintf(chTemp, "hmset Live:%s/%d output %d EasyDarwin %s", inParams->inStreamName, inParams->inChannel, inParams->inNumOutputs, QTSServerInterface::GetServer()->GetCloudServiceNodeID());
	auto reply = static_cast<redisReply*>(redisCommand(redisContext_, chTemp));
	if (!reply)
	{
		RedisErrorHandler([&]() {});

		return QTSS_NotConnected;
	}

	if (string(reply->str) == string("OK"))
	{
		sprintf(chKey, "expire Live:%s/%d 150", inParams->inStreamName, inParams->inChannel);
		auto replyExpire = static_cast<redisReply*>(redisCommand(redisContext_, chKey));
		if(replyExpire)
			freeReplyObject(replyExpire);
	}
	else
	{
		freeReplyObject(reply);
		return QTSS_RequestFailed;
	}

	freeReplyObject(reply);

	return QTSS_NoErr;
}

QTSS_Error RedisGetAssociatedCMS(QTSS_GetAssociatedCMS_Params* inParams)
{
	OSMutexLocker mutexLock(&sMutex);

	//if (!sIfConSucess)
	//	return QTSS_NotConnected;

	//char chTemp[128] = { 0 };

	////1. get the list of EasyDarwin
	//easyRedisReply * reply = static_cast<easyRedisReply *>(sRedisClient->SMembers("Device"));
	//if (reply == nullptr)
	//{
	//	sRedisClient->Free();
	//	sIfConSucess = false;
	//	return QTSS_NotConnected;
	//}

	////2.judge if the EasyCMS is ilve and contain serial  device
	//if ((reply->elements > 0) && (reply->type == EASY_REDIS_REPLY_ARRAY))
	//{
	//	easyRedisReply* childReply;
	//	for (size_t i = 0; i < reply->elements; i++)
	//	{
	//		childReply = reply->element[i];
	//		std::string strChileReply(childReply->str);

	//		sprintf(chTemp, "exists %s", (strChileReply + "_Live").c_str());
	//		sRedisClient->AppendCommand(chTemp);

	//		sprintf(chTemp, "sismember %s %s", (strChileReply + "_DevName").c_str(), inParams->inSerial);
	//		sRedisClient->AppendCommand(chTemp);
	//	}

	//	easyRedisReply *reply2 = nullptr, *reply3 = nullptr;
	//	for (size_t i = 0; i < reply->elements; i++)
	//	{
	//		if (sRedisClient->GetReply(reinterpret_cast<void**>(&reply2)) != EASY_REDIS_OK)
	//		{
	//			EasyFreeReplyObject(reply);
	//			if (reply2)
	//			{
	//				EasyFreeReplyObject(reply2);
	//			}
	//			sRedisClient->Free();
	//			sIfConSucess = false;
	//			return QTSS_NotConnected;
	//		}
	//		if (sRedisClient->GetReply(reinterpret_cast<void**>(&reply3)) != EASY_REDIS_OK)
	//		{
	//			EasyFreeReplyObject(reply);
	//			if (reply3)
	//			{
	//				EasyFreeReplyObject(reply3);
	//			}
	//			sRedisClient->Free();
	//			sIfConSucess = false;
	//			return QTSS_NotConnected;
	//		}

	//		if ((reply2->type == EASY_REDIS_REPLY_INTEGER) && (reply2->integer == 1) &&
	//			(reply3->type == EASY_REDIS_REPLY_INTEGER) && (reply3->integer == 1))
	//		{//find it
	//			std::string strIpPort(reply->element[i]->str);
	//			int ipos = strIpPort.find(':');//judge error
	//			memcpy(inParams->outCMSIP, strIpPort.c_str(), ipos);
	//			memcpy(inParams->outCMSPort, &strIpPort[ipos + 1], strIpPort.size() - ipos - 1);
	//			//break;//can't break,as 1 to 1
	//		}
	//		EasyFreeReplyObject(reply2);
	//		EasyFreeReplyObject(reply3);
	//	}
	//}
	//EasyFreeReplyObject(reply);
	return QTSS_NoErr;
}

QTSS_Error RedisChangeRtpNum()
{
	//OSMutexLocker mutexLock(&sMutex);
	//if (!sIfConSucess)
	//	return QTSS_NotConnected;

	//char chKey[128] = { 0 };
	//easyRedisReply* reply = nullptr;
	//auto id = QTSServerInterface::GetServer()->GetCloudServiceNodeID();
	//sprintf(chKey, "hset %s:%s Load %d", QTSServerInterface::GetServerName().Ptr, QTSServerInterface::GetServer()->GetCloudServiceNodeID(), QTSServerInterface::GetServer()->GetNumRTPSessions());
	//sRedisClient->AppendCommand(chKey);

	//sRedisClient->GetReply(reinterpret_cast<void**>(&reply));
	//if (reply)
	//{
	//	EasyFreeReplyObject(reply);
	//}

	return QTSS_NoErr;
}

QTSS_Error RedisJudgeStreamID(QTSS_JudgeStreamID_Params* inParams)
{
	////算法描述，删除指定sessionID对应的key，如果成功删除，表明SessionID存在，验证通过，否则验证失败
	//OSMutexLocker mutexLock(&sMutex);
	//if (!sIfConSucess)
	//	return QTSS_NotConnected;

	//char chKey[128] = { 0 };
	//sprintf(chKey, "SessionID_%s", inParams->inStreanID);//如果key存在则返回整数类型1，否则返回整数类型0

	//int ret = sRedisClient->Delete(chKey);

	//if (ret == -1)//fatal err,need reconnect
	//{
	//	sRedisClient->Free();
	//	sIfConSucess = false;

	//	return QTSS_NotConnected;
	//}
	//else if (ret == 0)
	//{
	//	*(inParams->outresult) == 1;
	//	return QTSS_NoErr;
	//}
	//else
	//{
	//	return ret;
	//}
	return 0;
}

static void RedisErrorHandler(function<void()> func)
{
	printf("connect error");

	sIfConSucess = false;
	redisFree(redisContext_);

	func();
}