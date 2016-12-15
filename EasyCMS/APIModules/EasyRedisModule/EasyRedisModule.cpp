#include "EasyRedisModule.h"

#include "OSHeaders.h"
#include "QTSSModuleUtils.h"
#include "EasyRedisClient.h"
#include "QTSServerInterface.h"
#include "HTTPSessionInterface.h"
#include "Format.h"
#include "EasyUtil.h"

#include <stdio.h>

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
static QTSS_Error EasyRedisModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParamBlock);
static QTSS_Error Register(QTSS_Register_Params* inParams);
static QTSS_Error Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error RereadPrefs();

static QTSS_Error RedisConnect();
static QTSS_Error RedisTTL();
static QTSS_Error RedisAddDevName(QTSS_StreamName_Params* inParams);
static QTSS_Error RedisDelDevName(QTSS_StreamName_Params* inParams);
static QTSS_Error RedisGetAssociatedDarwin(QTSS_GetAssociatedDarwin_Params* inParams);
static QTSS_Error RedisGetBestDarwin(QTSS_GetBestDarwin_Params * inParams);
static QTSS_Error RedisGenStreamID(QTSS_GenStreamID_Params* inParams);

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
	case Easy_RedisAddDevName_Role:
		return RedisAddDevName(&inParamBlock->StreamNameParams);
	case Easy_RedisDelDevName_Role:
		return RedisDelDevName(&inParamBlock->StreamNameParams);
	case Easy_RedisTTL_Role:
		return RedisTTL();
	case Easy_RedisGetEasyDarwin_Role:
		return RedisGetAssociatedDarwin(&inParamBlock->GetAssociatedDarwinParams);
	case Easy_RedisGetBestEasyDarwin_Role:
		return RedisGetBestDarwin(&inParamBlock->GetBestDarwinParams);
	case Easy_RedisGenStreamID_Role:
		return RedisGenStreamID(&inParamBlock->GenStreamIDParams);
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
	(void)QTSS_AddRole(Easy_RedisAddDevName_Role);
	(void)QTSS_AddRole(Easy_RedisDelDevName_Role);
	(void)QTSS_AddRole(Easy_RedisGetEasyDarwin_Role);
	(void)QTSS_AddRole(Easy_RedisGetBestEasyDarwin_Role);
	(void)QTSS_AddRole(Easy_RedisGenStreamID_Role);
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

	sRedisClient = new EasyRedisClient();

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
	}
	else
	{
		qtss_printf("Connect redis failed\n");
		sIfConSucess = false;
	}

	if (sIfConSucess)
	{
		char chKey[128] = { 0 };

		sprintf(chKey, "auth %s", sRedisPassword);
		sRedisClient->AppendCommand(chKey);
		easyRedisReply* reply = nullptr;
		sRedisClient->GetReply(reinterpret_cast<void**>(&reply));
		if (reply)
		{
			EasyFreeReplyObject(reply);
		}

		return QTSS_NoErr;
	}

	return QTSS_NotConnected;
}

QTSS_Error RedisAddDevName(QTSS_StreamName_Params* inParams)
{
	OSMutexLocker mutexLock(&sMutex);
	if (!sIfConSucess)
		return QTSS_NotConnected;

	if (!inParams->inDevice)
	{
		return QTSS_BadArgument;
	}

	auto deviceInfo = static_cast<strDevice*>(inParams->inDevice);

	char chKey[128] = { 0 };

	string type, channel;
	if (deviceInfo->eAppType == EASY_APP_TYPE_CAMERA)
	{
		type = "EasyCamera";
		channel = "1";
	}
	else if (deviceInfo->eAppType == EASY_APP_TYPE_NVR)
	{
		type = "EasyNVR";
		auto channels = deviceInfo->channels_;
		for (auto& item : channels)
		{
			channel += item.first + R"(/)";
		}
	}

	if (channel.empty())
	{
		channel = "0";
	}
	else
	{
		if (channel.back() == '/')
		{
			channel.pop_back();
		}
	}

	auto id = QTSServerInterface::GetServer()->GetCloudServiceNodeID();
	sprintf(chKey, "hmset Device:%s Type %s Channel %s EasyCMS %s Token %s", deviceInfo->serial_.c_str(),
		type.c_str(), channel.c_str(), id, deviceInfo->password_.c_str());

	sRedisClient->AppendCommand(chKey);

	easyRedisReply* replyTemp = nullptr;
	sRedisClient->GetReply(reinterpret_cast<void**>(&replyTemp));
	if (replyTemp)
	{
		EasyFreeReplyObject(replyTemp);
	}

	sprintf(chKey, "Device:%s", deviceInfo->serial_.c_str());
	sRedisClient->SetExpire(chKey, 150);

	return QTSS_NoErr;
}

QTSS_Error RedisDelDevName(QTSS_StreamName_Params* inParams)
{
	OSMutexLocker mutexLock(&sMutex);
	if (!sIfConSucess)
		return QTSS_NotConnected;

	if (!inParams->inDevice)
	{
		return QTSS_BadArgument;
	}

	auto deviceInfo = static_cast<strDevice*>(inParams->inDevice);

	char chKey[128] = { 0 };
	sprintf(chKey, "hdel Device:%s", deviceInfo->serial_.c_str());

	easyRedisReply* replyTemp = nullptr;
	sRedisClient->GetReply(reinterpret_cast<void**>(&replyTemp));
	if (replyTemp)
	{
		EasyFreeReplyObject(replyTemp);
	}

	return QTSS_NoErr;
}

QTSS_Error RedisTTL()
{
	OSMutexLocker mutexLock(&sMutex);

	if (RedisConnect() != QTSS_NoErr)
	{
		return QTSS_NotConnected;
	}

	char chKey[128] = { 0 };

	sprintf(chKey, "auth %s", sRedisPassword);
	sRedisClient->AppendCommand(chKey);
	easyRedisReply* reply = nullptr;
	sRedisClient->GetReply(reinterpret_cast<void**>(&reply));
	if (reply)
	{
		EasyFreeReplyObject(reply);
	}
	reply = nullptr;

	sprintf(chKey, "%s:%s", QTSServerInterface::GetServerName().Ptr, QTSServerInterface::GetServer()->GetCloudServiceNodeID());
	int ret = sRedisClient->SetExpire(chKey, 15);
	if (ret == -1)//fatal error
	{
		sRedisClient->Free();
		sIfConSucess = false;
		return QTSS_NotConnected;
	}
	if (ret == 1)
	{
		auto id = QTSServerInterface::GetServer()->GetCloudServiceNodeID();
		auto deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap()->GetMap();
		sprintf(chKey, "hset EasyCMS:%s Load %d", id, deviceMap->size());
		sRedisClient->AppendCommand(chKey);

		sRedisClient->GetReply(reinterpret_cast<void**>(&reply));
		if (reply)
		{
			EasyFreeReplyObject(reply);
		}

		return QTSS_NoErr;
	}
	if (ret == 0)//the key doesn't exist, reset
	{
		char chTemp[128]{ 0 };
		auto id = QTSServerInterface::GetServer()->GetCloudServiceNodeID();
		auto deviceMap = QTSServerInterface::GetServer()->GetDeviceSessionMap()->GetMap();
		auto cmsIp = QTSServerInterface::GetServer()->GetPrefs()->GetMonitorWANIP();
		auto cmsPort = QTSServerInterface::GetServer()->GetPrefs()->GetMonitorWANPort();
		sprintf(chTemp, "hmset EasyCMS:%s IP %s Port %d Load %d", id, cmsIp, cmsPort, deviceMap->size());
		sRedisClient->AppendCommand(chTemp);

		sRedisClient->GetReply(reinterpret_cast<void**>(&reply));
		if (reply)
		{
			EasyFreeReplyObject(reply);
		}

		sprintf(chKey, "%s:%s", QTSServerInterface::GetServerName().Ptr, QTSServerInterface::GetServer()->GetCloudServiceNodeID());
		sRedisClient->SetExpire(chKey, 15);
	}

	return QTSS_NoErr;
}

QTSS_Error RedisGetAssociatedDarwin(QTSS_GetAssociatedDarwin_Params* inParams)
{
	OSMutexLocker mutexLock(&sMutex);

	if (!sIfConSucess)
		return QTSS_NotConnected;

	string strPushName = Format("%s/%s", string(inParams->inSerial), string(inParams->inChannel));

	//1. get the list of EasyDarwin
	easyRedisReply * reply = static_cast<easyRedisReply*>(sRedisClient->SMembers("EasyDarwinName"));
	if (reply == nullptr)
	{
		sRedisClient->Free();
		sIfConSucess = false;
		return QTSS_NotConnected;
	}

	//2.judge if the EasyDarwin is ilve and contain serial/channel.sdp
	if ((reply->elements > 0) && (reply->type == EASY_REDIS_REPLY_ARRAY))
	{
		easyRedisReply* childReply;
		for (size_t i = 0; i < reply->elements; i++)
		{
			childReply = reply->element[i];
			string strChileReply(childReply->str);

			string strTemp = Format("exists %s", strChileReply + "_Live");
			sRedisClient->AppendCommand(strTemp.c_str());

			strTemp = Format("sismember %s %s", strChileReply + "_PushName", strPushName);
			sRedisClient->AppendCommand(strTemp.c_str());
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
				string strIpPort(reply->element[i]->str);
				int ipos = strIpPort.find(':');//judge error
				memcpy(inParams->outDssIP, strIpPort.c_str(), ipos);
				memcpy(inParams->outDssPort, &strIpPort[ipos + 1], strIpPort.size() - ipos - 1);
				//break;//can't break,as 1 to 1
			}
			EasyFreeReplyObject(reply2);
			EasyFreeReplyObject(reply3);
		}
	}
	EasyFreeReplyObject(reply);
	return QTSS_NoErr;
}

QTSS_Error RedisGetBestDarwin(QTSS_GetBestDarwin_Params * inParams)
{
	OSMutexLocker mutexLock(&sMutex);

	QTSS_Error theErr = QTSS_NoErr;

	if (!sIfConSucess)
		return QTSS_NotConnected;


	char chTemp[128] = { 0 };

	//1. get the list of EasyDarwin
	easyRedisReply * reply = static_cast<easyRedisReply *>(sRedisClient->SMembers("EasyDarwinName"));
	if (reply == nullptr)
	{
		sRedisClient->Free();
		sIfConSucess = false;
		return QTSS_NotConnected;
	}

	//2.judge if the EasyDarwin is ilve and get the RTP
	if ((reply->elements > 0) && (reply->type == EASY_REDIS_REPLY_ARRAY))
	{
		easyRedisReply* childReply;
		for (size_t i = 0; i < reply->elements; i++)
		{
			childReply = reply->element[i];
			string strChileReply(childReply->str);

			sprintf(chTemp, "exists %s", (strChileReply + "_Live").c_str());
			sRedisClient->AppendCommand(chTemp);

			sprintf(chTemp, "hget %s %s", (strChileReply + "_Info").c_str(), "RTP");
			sRedisClient->AppendCommand(chTemp);
		}

		int key = -1, keynum = 0;
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
				(reply3->type == EASY_REDIS_REPLY_STRING))
			{//find it
				int RTPNum = atoi(reply3->str);
				if (key == -1)
				{
					key = i;
					keynum = RTPNum;
				}
				else
				{
					if (RTPNum < keynum)//find better
					{
						key = i;
						keynum = RTPNum;
					}
				}
			}
			EasyFreeReplyObject(reply2);
			EasyFreeReplyObject(reply3);
		}
		if (key == -1)//no one live
		{
			theErr = QTSS_Unimplemented;
		}
		else
		{
			string strIpPort(reply->element[key]->str);
			int ipos = strIpPort.find(':');//judge error
			memcpy(inParams->outDssIP, strIpPort.c_str(), ipos);
			memcpy(inParams->outDssPort, &strIpPort[ipos + 1], strIpPort.size() - ipos - 1);
		}
	}
	else//没有可用的EasyDarWin
	{
		theErr = QTSS_Unimplemented;
	};
	EasyFreeReplyObject(reply);
	return theErr;
}

QTSS_Error RedisGenStreamID(QTSS_GenStreamID_Params* inParams)
{
	//算法秒速，生成随机sessionID，看redis上是否有存储，没有就存在redis上，有的话就再生成，直到没有为止
	OSMutexLocker mutexLock(&sMutex);

	if (!sIfConSucess)
		return QTSS_NotConnected;

	easyRedisReply* reply = nullptr;
	char chTemp[128] = { 0 };
	string strSessioionID;

	do
	{
		if (reply)//释放上一个回应
			EasyFreeReplyObject(reply);

		auto cmsIp = QTSServerInterface::GetServer()->GetPrefs()->GetMonitorWANIP();
		auto cmsPort = QTSServerInterface::GetServer()->GetPrefs()->GetMonitorWANPort();
		strSessioionID = OSMapEx::GenerateSessionIdForRedis(cmsIp, cmsPort);

		sprintf(chTemp, "SessionID_%s", strSessioionID.c_str());
		reply = static_cast<easyRedisReply*>(sRedisClient->Exists(chTemp));
		if (nullptr == reply)//错误，需要进行重连
		{
			sRedisClient->Free();
			sIfConSucess = false;
			return QTSS_NotConnected;
		}
	} while ((reply->type == EASY_REDIS_REPLY_INTEGER) && (reply->integer == 1));
	EasyFreeReplyObject(reply);//释放最后一个的回应

	//走到这说明找到了一个唯一的SessionID，现在将它存储到redis上
	sprintf(chTemp, "SessionID_%s", strSessioionID.c_str());//高级版本支持setpx来设置超时时间为ms
	if (sRedisClient->SetEX(chTemp, inParams->inTimeoutMil / 1000, "1") == -1)
	{
		sRedisClient->Free();
		sIfConSucess = false;
		return QTSS_NotConnected;
	}
	strcpy(inParams->outStreanID, strSessioionID.c_str());
	return QTSS_NoErr;
}