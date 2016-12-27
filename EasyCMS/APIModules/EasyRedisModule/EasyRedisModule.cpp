/*
	Copyleft (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
#include "EasyRedisModule.h"

#include "OSHeaders.h"
#include "QTSSModuleUtils.h"
#include "EasyRedisClient.h"
#include "QTSServerInterface.h"
#include "HTTPSessionInterface.h"
#include "Format.h"
#include "EasyUtil.h"
#include "Resources.h"
#include "ScopeGuard.h"

#include "Windows/hiredis.h"

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
static char*            sDefaultRedisPassword = "EasyDSSEasyDarwinEasyCMSEasyCamera";

//static EasyRedisClient* sRedisClient = nullptr;//the object pointer that package the redis operation
static bool				sIfConSucess = false;
static OSMutex			sMutex;

static redisContext*	redisContext_ = nullptr;

static void RedisErrorHandler(function<void()> func);

// FUNCTION PROTOTYPES
static QTSS_Error EasyRedisModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParamBlock);
static QTSS_Error Register(QTSS_Register_Params* inParams);
static QTSS_Error Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error RereadPrefs();

static bool RedisConnect();
static QTSS_Error RedisTTL();
static QTSS_Error RedisSetDevice(Easy_DeviceInfo_Params* inParams);
static QTSS_Error RedisDelDevice(Easy_DeviceInfo_Params* inParams);
static QTSS_Error RedisGetAssociatedDarwin(QTSS_GetAssociatedDarwin_Params* inParams);

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
	case Easy_RedisSetDevice_Role:
		return RedisSetDevice(&inParamBlock->DeviceInfoParams);
	case Easy_RedisDelDevice_Role:
		return RedisDelDevice(&inParamBlock->DeviceInfoParams);
	case Easy_RedisTTL_Role:
		return RedisTTL();
	case Easy_RedisGetEasyDarwin_Role:
		return RedisGetAssociatedDarwin(&inParamBlock->GetAssociatedDarwinParams);
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
	(void)QTSS_AddRole(Easy_RedisSetDevice_Role);
	(void)QTSS_AddRole(Easy_RedisDelDevice_Role);
	(void)QTSS_AddRole(Easy_RedisGetEasyDarwin_Role);
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

bool RedisConnect()
{
	if (sIfConSucess)
	{
		return true;
	}

	struct timeval timeout = { 2, 0 }; // 1.5 seconds
	redisContext_ = redisConnectWithTimeout(sRedis_IP, sRedisPort, timeout);
	if (!redisContext_ || redisContext_->err)
	{
		if (redisContext_)
		{
			RedisErrorHandler([&]()
			{
				printf("Redis context connect error \n");
			});
		}
		else
		{
			printf("Connection error: can't allocate redis context\n");
		}

		return false;
	}

	auto auth = Format("auth %s", string(sRedisPassword));
	auto reply = static_cast<redisReply*>(redisCommand(redisContext_, auth.c_str()));
	auto replyGuard = MakeGuard([&]()
	{
		if (reply)
		{
			freeReplyObject(reply);
		}
	});

	if (!reply || string(reply->str) != string("OK"))
	{
		RedisErrorHandler([&]()
		{
			printf("Redis auth error\n");
		});

		return false;
	}

	sIfConSucess = true;

	printf("Connect Redis success\n");

	return true;
}

QTSS_Error RedisTTL()
{
	OSMutexLocker mutexLock(&sMutex);

	if (!RedisConnect())
	{
		return QTSS_NotConnected;
	}
	string server(QTSServerInterface::GetServer()->GetServerName().Ptr);
	string id(QTSServerInterface::GetServer()->GetCloudServiceNodeID());
	auto load = QTSServerInterface::GetServer()->GetNumServiceSessions();

	auto expire = Format("expire %s:%s 15", server, id);
	auto reply = static_cast<redisReply*>(redisCommand(redisContext_, expire.c_str()));
	auto replyGuard = MakeGuard([&]()
	{
		if (reply)
		{
			freeReplyObject(reply);
		}
	});

	if (!reply)
	{
		RedisErrorHandler([&]()
		{
			printf("Redis expire EasyCMS error\n");
		});

		return QTSS_NotConnected;
	}


	if (reply->integer == 0)
	{
		string cmsIp(QTSServerInterface::GetServer()->GetPrefs()->GetServiceWANIP());
		auto cmsPort = QTSServerInterface::GetServer()->GetPrefs()->GetServiceWANPort();
		auto hmset = Format("hmset %s:%s %s %s %s %d %s %d", string(EASY_REDIS_EASYCMS), id, string(EASY_REDIS_IP), cmsIp,
			string(EASY_REDIS_PORT), cmsPort, string(EASY_REDIS_LOAD), load);
		auto replyHmset = static_cast<redisReply*>(redisCommand(redisContext_, hmset.c_str()));
		auto replyHmsetGuard = MakeGuard([&]()
		{
			if (replyHmset)
			{
				freeReplyObject(replyHmset);
			}
		});

		if (!replyHmset)
		{
			RedisErrorHandler([&]()
			{
				printf("Redis hmset EasyCMS error\n");
			});

			return QTSS_NotConnected;
		}

		auto replyExpire = static_cast<redisReply*>(redisCommand(redisContext_, expire.c_str()));
		auto replyExpireGuard = MakeGuard([&]()
		{
			if (replyExpire)
			{
				freeReplyObject(replyExpire);
			}
		});

		if (!replyExpire)
		{
			RedisErrorHandler([&]()
			{
				printf("Redis expire new EasyCMS error\n");
			});

			return QTSS_NotConnected;
		}

	}
	else if (reply->integer == 1)
	{
		auto hset = Format("hset %s:%s %s %d", server, id, string(EASY_REDIS_LOAD), load);
		auto replyHset = static_cast<redisReply*>(redisCommand(redisContext_, hset.c_str()));
		auto replyHsetGuard = MakeGuard([&]()
		{
			if (replyHset)
			{
				freeReplyObject(replyHset);
			}
		});

		if (!replyHset)
		{
			RedisErrorHandler([&]()
			{
				printf("Redis hset EasyCMS load erro\n ");
			});

			return QTSS_NotConnected;
		}
	}

	return QTSS_NoErr;
}

QTSS_Error RedisSetDevice(Easy_DeviceInfo_Params* inParams)
{
	OSMutexLocker mutexLock(&sMutex);

	if (!RedisConnect())
	{
		return QTSS_NotConnected;
	}

	if (!inParams->serial_ || string(inParams->serial_).empty())
	{
		return QTSS_BadArgument;
	}

	string id(QTSServerInterface::GetServer()->GetCloudServiceNodeID());
	auto hmset = Format("hmset %s:%s %s %s %s %s %s %s %s %s", string(EASY_REDIS_DEVICE), string(inParams->serial_),
		string(EASY_REDIS_TYPE), string(inParams->type_), string(EASY_REDIS_CHANNEL), string(inParams->channels_), string(EASY_REDIS_EASYCMS), id,
		string(EASY_REDIS_TOKEN), string(inParams->token_));
	auto reply = static_cast<redisReply*>(redisCommand(redisContext_, hmset.c_str()));
	auto replyGuard = MakeGuard([&]()
	{
		if (reply)
		{
			freeReplyObject(reply);
		}
	});

	if (!reply)
	{
		RedisErrorHandler([&]()
		{
			printf("Redis hmset Device error\n");
		});

		return QTSS_NotConnected;
	}

	if (string(reply->str) == string("OK"))
	{
		auto expire = Format("expire %s:%s 150", string(EASY_REDIS_DEVICE), string(inParams->serial_));
		auto replyExpire = static_cast<redisReply*>(redisCommand(redisContext_, expire.c_str()));
		auto replyExpireGuard = MakeGuard([&]()
		{
			if (replyExpire)
			{
				freeReplyObject(replyExpire);
			}
		});

		if (!replyExpire)
		{
			RedisErrorHandler([&]()
			{
				printf("Redis expire Device error\n");
			});

			return QTSS_NotConnected;
		}
	}
	else
	{
		return QTSS_RequestFailed;
	}

	return QTSS_NoErr;
}

QTSS_Error RedisDelDevice(Easy_DeviceInfo_Params* inParams)
{
	OSMutexLocker mutexLock(&sMutex);

	if (!RedisConnect())
	{
		return QTSS_NotConnected;
	}

	if (!inParams->serial_ || string(inParams->serial_).empty())
	{
		return QTSS_BadArgument;
	}

	auto del = Format("del %s:%s", EASY_REDIS_DEVICE, string(inParams->serial_));
	auto reply = static_cast<redisReply*>(redisCommand(redisContext_, del.c_str()));
	auto replyGuard = MakeGuard([&]()
	{
		if (reply)
		{
			freeReplyObject(reply);
		}
	});

	if (!reply)
	{
		RedisErrorHandler([&]()
		{
			printf("Redis del Device error\n");
		});

		return QTSS_NotConnected;
	}

	if (reply->integer == 0)
	{
		return QTSS_RequestFailed;
	}

	return QTSS_NoErr;
}

QTSS_Error RedisGetAssociatedDarwin(QTSS_GetAssociatedDarwin_Params* inParams)
{
	OSMutexLocker mutexLock(&sMutex);

	if (!RedisConnect())
	{
		return QTSS_NotConnected;
	}

	string exists = Format("exists %s:%s/%s", string(EASY_REDIS_LIVE), string(inParams->inSerial), string(inParams->inChannel));
	auto reply = static_cast<redisReply*>(redisCommand(redisContext_, exists.c_str()));
	auto replyGuard = MakeGuard([&]()
	{
		if (reply)
		{
			freeReplyObject(reply);
		}
	});

	if (!reply)
	{
		RedisErrorHandler([&]()
		{
			printf("Redis exists Live error\n");
		});

		return QTSS_NotConnected;
	}

	if (reply->integer == 1)
	{
		string strTemp = Format("hmget %s:%s/%s %s", string(EASY_REDIS_LIVE), string(inParams->inSerial),
			string(inParams->inChannel), string(EASY_REDIS_EASYDARWIN));
		auto replyHmget = static_cast<redisReply*>(redisCommand(redisContext_, strTemp.c_str()));
		auto replyHmgetGuard = MakeGuard([&]()
		{
			if (replyHmget)
			{
				freeReplyObject(replyHmget);
			}
		});

		if (!replyHmget)
		{
			RedisErrorHandler([&]()
			{
				printf("Redis hmget Live error\n");
			});

			return QTSS_NotConnected;
		}
		string easydarwin = Format("%s:", string(EASY_REDIS_EASYDARWIN));
		easydarwin += replyHmget->element[0]->str;

		strTemp = Format("hmget %s %s %s %s", easydarwin, string(EASY_REDIS_IP), string(EASY_REDIS_HTTP),
			string(EASY_REDIS_RTSP));
		auto replyHmgetEasyDarwin = static_cast<redisReply*>(redisCommand(redisContext_, strTemp.c_str()));
		auto replyHmgetEasyDarwinGuard = MakeGuard([&]()
		{
			if (replyHmgetEasyDarwin)
			{
				freeReplyObject(replyHmgetEasyDarwin);
			}
		});

		if (!replyHmgetEasyDarwin)
		{
			RedisErrorHandler([&]()
			{
				printf("Redis hmget EasyDarwin error\n");
			});

			return QTSS_NotConnected;
		}

		if (replyHmgetEasyDarwin->type == EASY_REDIS_REPLY_NIL)
		{
			return QTSS_RequestFailed;
		}

		if (replyHmgetEasyDarwin->type == EASY_REDIS_REPLY_ARRAY && replyHmgetEasyDarwin->elements == 3)
		{
			bool ok = true;
			for (int i = 0; i < replyHmgetEasyDarwin->elements; ++i)
			{
				if (replyHmgetEasyDarwin->element[i]->type == EASY_REDIS_REPLY_NIL)
				{
					ok = ok && false;
				}
			}

			if (ok)
			{
				string ip(replyHmgetEasyDarwin->element[0]->str);
				string httpPort(replyHmgetEasyDarwin->element[1]->str);
				string rtspPort(replyHmgetEasyDarwin->element[2]->str);
				memcpy(inParams->outDssIP, ip.c_str(), ip.size());
				memcpy(inParams->outHTTPPort, httpPort.c_str(), httpPort.size());
				memcpy(inParams->outDssPort, rtspPort.c_str(), rtspPort.size());
				inParams->isOn = true;
			}
			else
			{
				return QTSS_RequestFailed;
			}
		}
	}
	else
	{
		string keys = Format("keys %s:*", string(EASY_REDIS_EASYDARWIN));
		auto replyKeys = static_cast<redisReply*>(redisCommand(redisContext_, keys.c_str()));
		auto replyKeysGuard = MakeGuard([&]()
		{
			if (replyKeys)
			{
				freeReplyObject(replyKeys);
			}
		});

		if (!replyKeys)
		{
			RedisErrorHandler([&]()
			{
				printf("Redis keys EasyDarwin error\n");
			});

			return QTSS_NotConnected;
		}

		if (replyKeys->elements > 0)
		{
			multimap<int, tuple<string, string, string>> easydarwinMap;
			for (size_t i = 0; i < replyKeys->elements; ++i)
			{
				auto replyTemp = replyKeys->element[i];
				if (replyTemp->type == EASY_REDIS_REPLY_NIL)
				{
					continue;
				}

				string strTemp = Format("hmget %s %s %s %s %s ", string(replyTemp->str), string(EASY_REDIS_LOAD), string(EASY_REDIS_IP),
					string(EASY_REDIS_HTTP), string(EASY_REDIS_RTSP));
				auto replyHmget = static_cast<redisReply*>(redisCommand(redisContext_, strTemp.c_str()));
				auto replyHmgetGuard = MakeGuard([&]()
				{
					if (replyHmget)
					{
						freeReplyObject(replyHmget);
					}
				});

				if (!replyHmget)
				{
					RedisErrorHandler([&]()
					{
						printf("Redis hmget EasyDarwin info error\n");
					});

					return QTSS_NotConnected;
				}

				if (replyHmget->type == EASY_REDIS_REPLY_NIL)
				{
					continue;
				}

				auto loadReply = replyHmget->element[0];
				auto ipReply = replyHmget->element[1];
				auto httpReply = replyHmget->element[2];
				auto rtspReply = replyHmget->element[3];
				auto load = stoi(loadReply->str);
				string ip(ipReply->str);
				string http(httpReply->str);
				string rtsp(rtspReply->str);

				easydarwinMap.emplace(load, make_tuple(ip, http, rtsp));
			}

			if (easydarwinMap.empty())
			{
				return QTSS_RequestFailed;
			}

			auto easydarwin = easydarwinMap.begin()->second;
			auto ip = std::get<0>(easydarwin);
			auto http = std::get<1>(easydarwin);
			auto rtsp = std::get<2>(easydarwin);
			memcpy(inParams->outDssIP, ip.c_str(), ip.size());
			memcpy(inParams->outHTTPPort, http.c_str(), http.size());
			memcpy(inParams->outDssPort, rtsp.c_str(), rtsp.size());
			inParams->isOn = false;
		}
		else
		{
			return QTSS_NoErr;
		}

	}

	return QTSS_NoErr;
}

static void RedisErrorHandler(function<void()> func)
{
	//printf("Connection error: %s\n", redisContext_->errstr);

	sIfConSucess = false;
	redisFree(redisContext_);
	redisContext_ = nullptr;

	func();
}