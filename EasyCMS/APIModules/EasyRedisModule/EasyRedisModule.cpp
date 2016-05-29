#include <stdio.h>
#include "EasyRedisModule.h"
#include "OSHeaders.h"
#include "QTSSModuleUtils.h"
#include "MyAssert.h"
#include "EasyRedisClient.h"
#include "QTSServerInterface.h"
#include "HTTPSessionInterface.h"

// STATIC VARIABLES
static QTSS_ModulePrefsObject	modulePrefs		= NULL;
static QTSS_PrefsObject			sServerPrefs    = NULL;
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
// EasyCMS
static char*			sCMSIP					= NULL;
static UInt16			sCMSPort				= 10000;
static EasyRedisClient* sRedisClient			= NULL;//the object pointer that package the redis operation
static bool				sIfConSucess			= false;
static OSMutex			sMutex;

// FUNCTION PROTOTYPES
static QTSS_Error   EasyRedisModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParamBlock);
static QTSS_Error   Register(QTSS_Register_Params* inParams);
static QTSS_Error   Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error   RereadPrefs();

static QTSS_Error RedisConnect();
static QTSS_Error RedisInit();
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
	case QTSS_AddDevName_Role:
		return RedisAddDevName(&inParamBlock->StreamNameParams);
	case QTSS_DelDevName_Role:
		return RedisDelDevName(&inParamBlock->StreamNameParams);
	case QTSS_TTL_Role:
		return RedisTTL();
	case QTSS_GetAssociatedDarwin_Role:
		return RedisGetAssociatedDarwin(&inParamBlock->GetAssociatedDarwinParams);
	case QTSS_GetBestDarwin_Role:
		return RedisGetBestDarwin(&inParamBlock->GetBestDarwinParams);
	case QTSS_GenStreamID_Role:
		return RedisGenStreamID(&inParamBlock->GenStreamIDParams);
	}
	return QTSS_NoErr;
}

QTSS_Error Register(QTSS_Register_Params* inParams)
{
	// Do role setup
	(void)QTSS_AddRole(QTSS_Initialize_Role);
	(void)QTSS_AddRole(QTSS_RereadPrefs_Role);
	(void)QTSS_AddRole(QTSS_TTL_Role);
	(void)QTSS_AddRole(QTSS_AddDevName_Role);
	(void)QTSS_AddRole(QTSS_DelDevName_Role);
	(void)QTSS_AddRole(QTSS_GetAssociatedDarwin_Role);
	(void)QTSS_AddRole(QTSS_GetBestDarwin_Role);
	(void)QTSS_AddRole(QTSS_GenStreamID_Role);
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
	delete [] sRedis_IP;
    sRedis_IP = QTSSModuleUtils::GetStringAttribute(modulePrefs, "redis_ip", sDefaultRedis_IP_Addr);

	QTSSModuleUtils::GetAttribute(modulePrefs, "redis_port", qtssAttrDataTypeUInt16, &sRedisPort, &sDefaultRedisPort, sizeof(sRedisPort));

	delete [] sRedisUser;
    sRedisUser = QTSSModuleUtils::GetStringAttribute(modulePrefs, "redis_user", sDefaultRedisUser);
	
	delete [] sRedisPassword;
    sRedisPassword = QTSSModuleUtils::GetStringAttribute(modulePrefs, "redis_password", sDefaultRedisPassword);
	
	//get cms ip and port
	delete [] sCMSIP;
	(void)QTSS_GetValueAsString(sServerPrefs, qtssPrefsMonitorWANIPAddr, 0, &sCMSIP);

	UInt32 len = sizeof(SInt32);
	(void) QTSS_GetValue(sServerPrefs, qtssPrefsMonitorWANPort, 0, (void*)&sCMSPort, &len);

	return QTSS_NoErr;
}

QTSS_Error RedisConnect()
{
	if(sIfConSucess)
		return QTSS_NoErr;

	std::size_t timeout = 1;//timeout second
	if(sRedisClient->ConnectWithTimeOut(sRedis_IP,sRedisPort,timeout) != -1)
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
	char chTemp[128]={0};

	do 
	{
		//1,redis密码认证
		sprintf(chTemp,"auth %s",sRedisPassword);
		sRedisClient->AppendCommand(chTemp);

		//2,CMS唯一信息存储(覆盖上一次的存储)
		sprintf(chTemp,"sadd CMSName %s:%d",sCMSIP,sCMSPort);
		sRedisClient->AppendCommand(chTemp);


		//3,CMS属性存储,设置多个filed使用hmset，单个使用hset(覆盖上一次的存储)
		sprintf(chTemp,"hmset %s:%d_Info IP %s PORT %d",sCMSIP,sCMSPort,sCMSIP,sCMSPort);
		sRedisClient->AppendCommand(chTemp);

		//4,清除设备名称存储，因为连接之前和连接之后的设备可能一斤该发生了变化，因此必须先执行清楚操作
		sprintf(chTemp,"del %s:%d_DevName",sCMSIP,sCMSPort);
		sRedisClient->AppendCommand(chTemp);

		OSRefTableEx*  deviceRefTable = QTSServerInterface::GetServer()->GetDeviceSessionMap();
		OSMutex *mutexMap = deviceRefTable->GetMutex();
		OSHashMap  *deviceMap = deviceRefTable->GetMap();
		OSRefIt itRef;
		string strAllDevices;
		mutexMap->Lock();
		for(itRef = deviceMap->begin();itRef != deviceMap->end();itRef++)
		{
			strDevice *deviceInfo=(((HTTPSessionInterface*)(itRef->second->GetObjectPtr()))->GetDeviceInfo());
			strAllDevices=strAllDevices+' '+deviceInfo->serial_;
		}
		mutexMap->Unlock();

		char *chNewTemp = new char[strAllDevices.size()+128];//注意，这里不能再使用chTemp，因为长度不确定，可能导致缓冲区溢出
		//5,设备名称存储
		sprintf(chNewTemp,"sadd %s:%d_DevName%s",sCMSIP,sCMSPort,strAllDevices.c_str());
		sRedisClient->AppendCommand(chNewTemp);
		delete[] chNewTemp;

		//6,保活，设置15秒，这之后当前CMS已经开始提供服务了
		sprintf(chTemp,"setex %s:%d_Live 15 1",sCMSIP,sCMSPort);
		sRedisClient->AppendCommand(chTemp);

		bool bBreak=false;
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

	sIfConSucess=false;
	return (QTSS_Error)false;
}

QTSS_Error RedisAddDevName(QTSS_StreamName_Params* inParams)
{
	OSMutexLocker mutexLock(&sMutex);
	if(!sIfConSucess)
		return QTSS_NotConnected;

	char chKey[128]={0};
	sprintf(chKey,"%s:%d_DevName",sCMSIP,sCMSPort);

	int ret = sRedisClient->SAdd(chKey,inParams->inStreamName);
	if( ret == -1)//fatal err,need reconnect
	{
		sRedisClient->Free();
		sIfConSucess = false;
	}

	return ret;
}

QTSS_Error RedisDelDevName(QTSS_StreamName_Params* inParams)
{	
	OSMutexLocker mutexLock(&sMutex);
	if(!sIfConSucess)
		QTSS_NotConnected;

	char chKey[128]={0};
	sprintf(chKey,"%s:%d_DevName",sCMSIP,sCMSPort);

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
	sprintf(chKey,"%s:%d_Live 15",sCMSIP,sCMSPort);//更改超时时间

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
		sprintf(chKey,"%s:%d_Live",sCMSIP,sCMSPort);
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

QTSS_Error RedisGetAssociatedDarwin(QTSS_GetAssociatedDarwin_Params* inParams)
{
	OSMutexLocker mutexLock(&sMutex);

	if(!sIfConSucess)
		return QTSS_NotConnected;

	char chPushName[128] = {0};
	sprintf(chPushName,"%s/%s.sdp",inParams->inSerial,inParams->inChannel);
	char chTemp[128] = {0};

	//1. get the list of EasyDarwin
	easyRedisReply * reply = (easyRedisReply *)sRedisClient->SMembers("EasyDarWinName");
	if(reply == NULL)
	{
		sRedisClient->Free();
		sIfConSucess = false;
		return QTSS_NotConnected;
	}

	//2.judge if the EasyDarwin is ilve and contain serial/channel.sdp
	if( (reply->elements>0) && (reply->type == EASY_REDIS_REPLY_ARRAY) )
	{
		easyRedisReply* childReply = NULL;
		for(size_t i = 0;i<reply->elements;i++)
		{
			childReply		=	reply->element[i];
			string strChileReply(childReply->str);

			sprintf(chTemp,"exists %s",(strChileReply+"_Live").c_str());
			sRedisClient->AppendCommand(chTemp);

			sprintf(chTemp,"sismember %s %s",(strChileReply+"_PushName").c_str(),chPushName);
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
				string strIpPort(reply->element[i]->str);
				int ipos = strIpPort.find(':');//judge error
				memcpy(inParams->outDssIP,strIpPort.c_str(),ipos);
				memcpy(inParams->outDssPort,&strIpPort[ipos+1],strIpPort.size()-ipos-1);
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

	if(!sIfConSucess)
		return QTSS_NotConnected;


	char chTemp[128]={0};

	//1. get the list of EasyDarwin
	easyRedisReply * reply = (easyRedisReply *)sRedisClient->SMembers("EasyDarWinName");
	if(reply == NULL)
	{
		sRedisClient->Free();
		sIfConSucess = false;
		return QTSS_NotConnected;
	}

	//2.judge if the EasyDarwin is ilve and get the RTP
	if( (reply->elements>0) && (reply->type == EASY_REDIS_REPLY_ARRAY) )
	{
		easyRedisReply* childReply=NULL;
		for(size_t i=0;i<reply->elements;i++)
		{
			childReply		=	reply->element[i];
			string strChileReply(childReply->str);

			sprintf(chTemp,"exists %s",(strChileReply+"_Live").c_str());
			sRedisClient->AppendCommand(chTemp);

			sprintf(chTemp,"hget %s %s",(strChileReply+"_Info").c_str(),"RTP");
			sRedisClient->AppendCommand(chTemp);
		}

		int key = -1,keynum = 0;
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

			if( (reply2->type == EASY_REDIS_REPLY_INTEGER) && (reply2->integer == 1) &&
				(reply3->type == EASY_REDIS_REPLY_STRING))
			{//find it
				int RTPNum=atoi(reply3->str);
				if(key==-1)
				{
					key=i;
					keynum=RTPNum;
				}
				else
				{
					if(RTPNum<keynum)//find better
					{
						key=i;
						keynum=RTPNum;
					}
				}
			}
			EasyFreeReplyObject(reply2);
			EasyFreeReplyObject(reply3);
		}
		if(key==-1)//no one live
		{
			theErr = QTSS_Unimplemented;
		}
		else
		{
			string strIpPort(reply->element[key]->str);
			int ipos	=		strIpPort.find(':');//judge error
			memcpy(inParams->outDssIP,strIpPort.c_str(),ipos);
			memcpy(inParams->outDssPort,&strIpPort[ipos+1],strIpPort.size()-ipos-1);
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

	if(!sIfConSucess)
		return QTSS_NotConnected;

	easyRedisReply* reply=NULL;
	char chTemp[128]={0};
	string strSessioionID;

	do 
	{
		if(reply)//释放上一个回应
			EasyFreeReplyObject(reply);

		strSessioionID = OSMapEx::GenerateSessionIdForRedis(sCMSIP,sCMSPort);

		sprintf(chTemp,"SessionID_%s",strSessioionID.c_str());
		reply = (easyRedisReply*)sRedisClient->Exists(chTemp);
		if (NULL == reply)//错误，需要进行重连
		{
			sRedisClient->Free();
			sIfConSucess = false;
			return QTSS_NotConnected;
		}
	}
	while( (reply->type == EASY_REDIS_REPLY_INTEGER) && (reply->integer==1) );
	EasyFreeReplyObject(reply);//释放最后一个的回应

	//走到这说明找到了一个唯一的SessionID，现在将它存储到redis上
	sprintf(chTemp,"SessionID_%s",strSessioionID.c_str());//高级版本支持setpx来设置超时时间为ms
	if(sRedisClient->SetEX(chTemp,inParams->inTimeoutMil/1000,"1") == -1)
	{
		sRedisClient->Free();
		sIfConSucess = false;
		return QTSS_NotConnected;
	}
	strcpy(inParams->outStreanID,strSessioionID.c_str());
	return QTSS_NoErr;
}