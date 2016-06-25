#include <stdio.h>

#include "EasyRedisModule.h"
#include "OSHeaders.h"
#include "QTSSModuleUtils.h"
#include "MyAssert.h"
#include "EasyRedisClient.h"
#include "QTSServerInterface.h"
#include "ReflectorSession.h"

// STATIC VARIABLES
static QTSS_ModulePrefsObject	modulePrefs		= NULL;
static QTSS_PrefsObject			sServerPrefs	= NULL;
static QTSS_ServerObject		sServer			= NULL;

// Redis IP
static char*            sRedis_IP				= NULL;
static char*            sDefaultRedis_IP_Addr	= "127.0.0.1";
// Redis Port
static UInt16			sRedisPort				= 6379;
static UInt16			sDefaultRedisPort		= 6379;
// Redis user
static char*            sRedisUser				= NULL;
static char*            sDefaultRedisUser		= "admin";
// Redis password
static char*            sRedisPassword			= NULL;
static char*            sDefaultRedisPassword	= "admin";

static char*			sRTSPWanIP				= NULL;
static char*			sDefaultRTSPWanIP		= "127.0.0.1";

static UInt16			sRTSPWanPort			= 554;
static UInt16			sDefaultRTSPWanPort		= 554;

static EasyRedisClient* sRedisClient			= NULL;//the object pointer that package the redis operation
static bool				sIfConSucess			= false;
static OSMutex			sMutex;

// FUNCTION PROTOTYPES
static QTSS_Error   EasyRedisModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParamBlock);
static QTSS_Error   Register(QTSS_Register_Params* inParams);
static QTSS_Error   Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error   RereadPrefs();
static QTSS_Error	RedisConnect();
static QTSS_Error	RedisInit();
static QTSS_Error	RedisTTL();
static QTSS_Error	RedisAddPushName(QTSS_StreamName_Params* inParams);
static QTSS_Error	RedisDelPushName(QTSS_StreamName_Params* inParams);
static QTSS_Error	RedisChangeRtpNum();
static QTSS_Error	RedisGetAssociatedCMS(QTSS_GetAssociatedCMS_Params* inParams);
static QTSS_Error	RedisJudgeStreamID(QTSS_JudgeStreamID_Params* inParams);


QTSS_Error EasyRedisModule_Main(void* inPrivateArgs)
{
	return _stublibrary_main(inPrivateArgs, EasyRedisModuleDispatch);
}

QTSS_Error  EasyRedisModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParamBlock)
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
	case Easy_RedisAddPushStream_Role:
		return RedisAddPushName(&inParamBlock->StreamNameParams);
	case Easy_RedisDelPushStream_Role:
		return RedisDelPushName(&inParamBlock->StreamNameParams);
	case Easy_RedisGetAssociatedCMS_Role:
		return RedisGetAssociatedCMS(&inParamBlock->GetAssociatedCMSParams);
	case Easy_RedisJudgeStreamID_Role:
		return RedisJudgeStreamID(&inParamBlock->JudgeStreamIDParams);
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
	(void)QTSS_AddRole(Easy_RedisAddPushStream_Role);
	(void)QTSS_AddRole(Easy_RedisDelPushStream_Role);
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
	delete [] sRedis_IP;
    sRedis_IP = QTSSModuleUtils::GetStringAttribute(modulePrefs, "redis_ip", sDefaultRedis_IP_Addr);

	QTSSModuleUtils::GetAttribute(modulePrefs, "redis_port", qtssAttrDataTypeUInt16, &sRedisPort, &sDefaultRedisPort, sizeof(sRedisPort));

	delete [] sRedisUser;
    sRedisUser = QTSSModuleUtils::GetStringAttribute(modulePrefs, "redis_user", sDefaultRedisUser);
	
	delete [] sRedisPassword;
    sRedisPassword = QTSSModuleUtils::GetStringAttribute(modulePrefs, "redis_password", sDefaultRedisPassword);

	//get EasyDarwin WAN ip and port
	delete [] sRTSPWanIP;
    sRTSPWanIP = QTSSModuleUtils::GetStringAttribute(modulePrefs, "rtsp_wan_ip", sDefaultRTSPWanIP);

	QTSSModuleUtils::GetAttribute(modulePrefs, "rtsp_wan_port", qtssAttrDataTypeUInt16, &sRTSPWanPort, &sDefaultRTSPWanPort, sizeof(sRTSPWanPort));

	return QTSS_NoErr;

}


QTSS_Error RedisConnect()
{
	if(sIfConSucess)
		return QTSS_NoErr;

	std::size_t timeout = 1;//timeout second
	if(sRedisClient->ConnectWithTimeOut(sRedis_IP,sRedisPort,timeout) == EASY_REDIS_OK)//return 0 if connect sucess
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
	return (QTSS_Error)(!sIfConSucess);
}

QTSS_Error RedisInit()//only called by RedisConnect after connect redis sucess
{
	//每一次与redis连接后，都应该清除上一次的数据存储，使用覆盖或者直接清除的方式,串行命令使用管线更加高效
	char chTemp[128] = {0};

	do 
	{
		//1,redis密码认证
		sprintf(chTemp,"auth %s",sRedisPassword);
		sRedisClient->AppendCommand(chTemp);

		//2,EasyDarwin唯一信息存储(覆盖上一次的存储)
		sprintf(chTemp,"sadd EasyDarwinName %s:%d",sRTSPWanIP,sRTSPWanPort);
		sRedisClient->AppendCommand(chTemp);

		//3,EasyDarwin属性存储,设置多个filed使用hmset，单个使用hset(覆盖上一次的存储)
		sprintf(chTemp,"hmset %s:%d_Info IP %s PORT %d RTP %d",sRTSPWanIP,sRTSPWanPort,sRTSPWanIP,sRTSPWanPort,QTSServerInterface::GetServer()->GetNumRTPSessions());
		sRedisClient->AppendCommand(chTemp);

		//4,清除推流名称存储
		sprintf(chTemp,"del %s:%d_PushName",sRTSPWanIP,sRTSPWanPort);
		sRedisClient->AppendCommand(chTemp);

		char* strAllPushName;
		OSRefTable * reflectorTable = QTSServerInterface::GetServer()->GetReflectorSessionMap();
		OSMutex *mutexMap = reflectorTable->GetMutex();
		int iPos = 0,iLen = 0;

		mutexMap->Lock();
		strAllPushName = new char[reflectorTable->GetNumRefsInTable()*128 + 1];//为每一个推流名称分配128字节的内存
		memset(strAllPushName,0,reflectorTable->GetNumRefsInTable()*128 + 1);//内存初始化为0
		for (OSRefHashTableIter theIter(reflectorTable->GetHashTable()); !theIter.IsDone(); theIter.Next())
		{
			OSRef* theRef			=	theIter.GetCurrent();
			ReflectorSession  * theSession = (ReflectorSession  *)theRef->GetObject();
			char * chPushName = theSession->GetSessionName();
			if(chPushName)
			{
				strAllPushName[iPos++]=' ';
				memcpy(strAllPushName+iPos,chPushName,strlen(chPushName));
				iPos = iPos + strlen(chPushName);
			}
		}   
		mutexMap->Unlock();

		char *chNewTemp = new char[strlen(strAllPushName)+128];//注意，这里不能再使用chTemp，因为长度不确定，可能导致缓冲区溢出

		//5,推流名称存储
		sprintf(chNewTemp,"sadd %s:%d_PushName%s",sRTSPWanIP,sRTSPWanPort,strAllPushName);
		sRedisClient->AppendCommand(chTemp);


		delete[] chNewTemp;
		delete[] strAllPushName;

		//6,保活，设置15秒，这之后当前EasyDarwin已经开始提供服务了
		sprintf(chTemp,"setex %s:%d_Live 15 1",sRTSPWanIP,sRTSPWanPort);
		sRedisClient->AppendCommand(chTemp);

		bool bBreak = false;
		easyRedisReply* reply = NULL;
		for(int i=0;i<6;i++)
		{
			if(EASY_REDIS_OK != sRedisClient->GetReply((void**)&reply))
			{
				bBreak = true;
				if(reply)
					EasyFreeReplyObject(reply);
				break;
			}
			EasyFreeReplyObject(reply);
		}
		if(bBreak)//说明redisGetReply出现了错误
			break;
		return QTSS_NoErr;
	} while (0);
	//走到这说明出现了错误，需要进行重连,重连操作再下一次执行命令时进行,在这仅仅是置标志位
	sRedisClient->Free();
	sIfConSucess = false;
	return (QTSS_Error)false;
}

QTSS_Error RedisAddPushName(QTSS_StreamName_Params* inParams)
{
	OSMutexLocker mutexLock(&sMutex);
	if(!sIfConSucess)
		return QTSS_NotConnected;

	char chKey[128]={0};
	sprintf(chKey,"%s:%d_PushName",sRTSPWanIP,sRTSPWanPort);

	int ret = sRedisClient->SAdd(chKey,inParams->inStreamName);
	if( ret == -1)//fatal err,need reconnect
	{
		sRedisClient->Free();
		sIfConSucess = false;
	}

	return ret;
}

QTSS_Error RedisDelPushName(QTSS_StreamName_Params* inParams)
{
	OSMutexLocker mutexLock(&sMutex);
	if(!sIfConSucess)
		return QTSS_NotConnected;

	char chKey[128]={0};
	sprintf(chKey,"%s:%d_PushName",sRTSPWanIP,sRTSPWanPort);

	int ret = sRedisClient->SRem(chKey,inParams->inStreamName);
	if( ret == -1)//fatal err,need reconnect
	{
		sRedisClient->Free();
		sIfConSucess = false;
	}
	return ret;
}

QTSS_Error RedisTTL()//注意当网络在一段时间很差时可能会因为超时时间达到而导致key被删除，这时应该重新设置该key
{

	OSMutexLocker mutexLock(&sMutex);

	if(RedisConnect() != QTSS_NoErr)//每一次执行命令之前都先连接redis,如果当前redis还没有成功连接
		return QTSS_NotConnected;

	char chKey[128]={0};//注意128位是否足够
	sprintf(chKey,"%s:%d_Live 15",sRTSPWanIP,sRTSPWanPort);//更改超时时间

	int ret =  sRedisClient->SetExpire(chKey,15);
	if(ret == -1)//fatal error
	{
		sRedisClient->Free();
		sIfConSucess = false;
		return QTSS_NotConnected;
	}
	else if(ret == 1)
	{
		return QTSS_NoErr;
	}
	else if(ret == 0)//the key doesn't exist, reset
	{
		sprintf(chKey,"%s:%d_Live",sRTSPWanIP,sRTSPWanPort);
		int retret = sRedisClient->SetEX(chKey,15,"1");
		if(retret == -1)//fatal error
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

	if(!sIfConSucess)
		return QTSS_NotConnected;

	char chTemp[128] = {0};

	//1. get the list of EasyDarwin
	easyRedisReply * reply = (easyRedisReply *)sRedisClient->SMembers("EasyCMSName");
	if(reply == NULL)
	{
		sRedisClient->Free();
		sIfConSucess = false;
		return QTSS_NotConnected;
	}

	//2.judge if the EasyCMS is ilve and contain serial  device
	if( (reply->elements>0) && (reply->type == EASY_REDIS_REPLY_ARRAY) )
	{
		easyRedisReply* childReply = NULL;
		for(size_t i = 0;i<reply->elements;i++)
		{
			childReply		=	reply->element[i];
			std::string strChileReply(childReply->str);

			sprintf(chTemp,"exists %s",(strChileReply+"_Live").c_str());
			sRedisClient->AppendCommand(chTemp);

			sprintf(chTemp,"sismember %s %s",(strChileReply+"_DevName").c_str(),inParams->inSerial);
			sRedisClient->AppendCommand(chTemp);
		}

		easyRedisReply *reply2 = NULL,*reply3 = NULL;
		for(size_t i = 0;i<reply->elements;i++)
		{
			if(sRedisClient->GetReply((void**)&reply2) != EASY_REDIS_OK)
			{
				EasyFreeReplyObject(reply);
				EasyFreeReplyObject(reply2);
				sRedisClient->Free();
				sIfConSucess = false;
				return QTSS_NotConnected;
			}
			if(sRedisClient->GetReply((void**)&reply3) != EASY_REDIS_OK)
			{
				EasyFreeReplyObject(reply);
				EasyFreeReplyObject(reply3);
				sRedisClient->Free();
				sIfConSucess = false;
				return QTSS_NotConnected;
			}

			if( (reply2->type == EASY_REDIS_REPLY_INTEGER) && (reply2->integer==1) &&
				(reply3->type == EASY_REDIS_REPLY_INTEGER) && (reply3->integer==1) )
			{//find it
				std::string strIpPort(reply->element[i]->str);
				int ipos = strIpPort.find(':');//judge error
				memcpy(inParams->outCMSIP,strIpPort.c_str(),ipos);
				memcpy(inParams->outCMSPort,&strIpPort[ipos+1],strIpPort.size()-ipos-1);
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
	if(!sIfConSucess)
		return QTSS_NotConnected;

	char chKey[128] = {0};
	sprintf(chKey,"%s:%d_Info",sRTSPWanIP,sRTSPWanPort);//hset对RTP属性进行覆盖更新

	char chRtpNum[16] = {0};
	sprintf(chRtpNum,"%d",QTSServerInterface::GetServer()->GetNumRTPSessions());

	int ret = sRedisClient->HashSet(chKey,"RTP",chRtpNum);
	if( ret == -1)//fatal err,need reconnect
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
	if(!sIfConSucess)
		return QTSS_NotConnected;

	bool bReval = false;
	char chKey[128] = {0};
	sprintf(chKey,"SessionID_%s",inParams->inStreanID);//如果key存在则返回整数类型1，否则返回整数类型0

	int ret = sRedisClient->Delete(chKey);

	if( ret == -1)//fatal err,need reconnect
	{
		sRedisClient->Free();
		sIfConSucess = false;

		return QTSS_NotConnected;
	}
	else if(ret == 0)
	{
		*(inParams->outresult) == 1;
		return QTSS_NoErr;
	}
	else
	{
		return ret;
	}
}