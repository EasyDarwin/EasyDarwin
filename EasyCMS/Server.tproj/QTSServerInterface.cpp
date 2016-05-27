/*
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 1999-2008 Apple Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 *
 */
/*
	Copyleft (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
/*
    File:       QTSServerInterface.cpp
    Contains:   Implementation of object defined in QTSServerInterface.h.
*/

//INCLUDES:

#ifndef kVersionString
#include "revision.h"
#endif
#include "QTSServerInterface.h"
#include "HTTPSessionInterface.h"

#include "OSRef.h"
#include "HTTPProtocol.h"
#ifndef __MacOSX__
#include "revision.h"
#endif

// STATIC DATA

UInt32                  QTSServerInterface::sServerAPIVersion = QTSS_API_VERSION;
QTSServerInterface*     QTSServerInterface::sServer = NULL;
#if __MacOSX__
StrPtrLen               QTSServerInterface::sServerNameStr("EasyCMS");
#else
StrPtrLen               QTSServerInterface::sServerNameStr("EasyCMS");
#endif

// kVersionString from revision.h, include with -i at project level
StrPtrLen               QTSServerInterface::sServerVersionStr(kVersionString);
StrPtrLen               QTSServerInterface::sServerBuildStr(kBuildString);
StrPtrLen               QTSServerInterface::sServerCommentStr(kCommentString);

StrPtrLen               QTSServerInterface::sServerPlatformStr(kPlatformNameString);
StrPtrLen               QTSServerInterface::sServerBuildDateStr(__DATE__ ", "__TIME__);
char                    QTSServerInterface::sServerHeader[kMaxServerHeaderLen];
StrPtrLen               QTSServerInterface::sServerHeaderPtr(sServerHeader, kMaxServerHeaderLen);

QTSSModule**            QTSServerInterface::sModuleArray[QTSSModule::kNumRoles];
UInt32                  QTSServerInterface::sNumModulesInRole[QTSSModule::kNumRoles];
OSQueue                 QTSServerInterface::sModuleQueue;
QTSSErrorLogStream      QTSServerInterface::sErrorLogStream;

QTSSAttrInfoDict::AttrInfo  QTSServerInterface::sAttributes[] = 
{   /*fields:   fAttrName, fFuncPtr, fAttrDataType, fAttrPermission */
    /* 0  */ { "qtssServerAPIVersion",          NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 1  */ { "qtssSvrDefaultDNSName",         NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead },
    /* 2  */ { "qtssSvrDefaultIPAddr",          NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead },
    /* 3  */ { "qtssSvrServerName",             NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 4  */ { "qtssSvrServerVersion",      NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 5  */ { "qtssSvrServerBuildDate",    NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 6  */ { "qtssSvrHTTPPorts",              NULL,   qtssAttrDataTypeUInt16,     qtssAttrModeRead },
    /* 7  */ { "qtssSvrHTTPServerHeader",       NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 8  */ { "qtssSvrState",					NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite  },
    /* 9  */ { "qtssSvrIsOutOfDescriptors",     IsOutOfDescriptors,     qtssAttrDataTypeBool16, qtssAttrModeRead },
    /* 10 */ { "qtssCurrentSessionCount",		NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead },
	
    /* 11 */ { "qtssSvrHandledMethods",         NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModeWrite | qtssAttrModePreempSafe  },
    /* 12 */ { "qtssSvrModuleObjects",          NULL,   qtssAttrDataTypeQTSS_Object,qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 13 */ { "qtssSvrStartupTime",            NULL,   qtssAttrDataTypeTimeVal,    qtssAttrModeRead },
    /* 14 */ { "qtssSvrGMTOffsetInHrs",         NULL,   qtssAttrDataTypeSInt32,     qtssAttrModeRead },
    /* 15 */ { "qtssSvrDefaultIPAddrStr",       NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead },
    
	/* 16 */ { "qtssSvrPreferences",            NULL,   qtssAttrDataTypeQTSS_Object,qtssAttrModeRead | qtssAttrModeInstanceAttrAllowed},
    /* 17 */ { "qtssSvrMessages",               NULL,   qtssAttrDataTypeQTSS_Object,qtssAttrModeRead },
    /* 18 */ { "qtssSvrClientSessions",         NULL,   qtssAttrDataTypeQTSS_Object,qtssAttrModeRead },
    /* 19 */ { "qtssSvrCurrentTimeMilliseconds",CurrentUnixTimeMilli,   qtssAttrDataTypeTimeVal,qtssAttrModeRead},
    /* 20 */ { "qtssSvrCPULoadPercent",         NULL,   qtssAttrDataTypeFloat32,    qtssAttrModeRead},
    
    /* 21 */ { "qtssSvrConnectedUsers",         NULL, qtssAttrDataTypeQTSS_Object,      qtssAttrModeRead | qtssAttrModeWrite },
    /* 22 */ { "qtssSvrServerBuild",           NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 23 */ { "qtssSvrServerPlatform",        NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 24 */ { "qtssSvrHTTPServerComment",     NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
	/* 25 */ { "qtssSvrNumThinned",            NULL,   qtssAttrDataTypeSInt32,     qtssAttrModeRead | qtssAttrModePreempSafe },

    /* 26 */ { "qtssSvrNumThreads",            NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe }
};

void    QTSServerInterface::Initialize()
{
    for (UInt32 x = 0; x < qtssSvrNumParams; x++)
        QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kServerDictIndex)->
            SetAttribute(x, sAttributes[x].fAttrName, sAttributes[x].fFuncPtr,
                sAttributes[x].fAttrDataType, sAttributes[x].fAttrPermission);

    //Write out a premade server header
    StringFormatter serverFormatter(sServerHeaderPtr.Ptr, kMaxServerHeaderLen);
	serverFormatter.Put(HTTPProtocol::GetHeaderString(httpServerHeader)->Ptr, HTTPProtocol::GetHeaderString(httpServerHeader)->Len);
    serverFormatter.Put(": ");
    serverFormatter.Put(sServerNameStr);
    serverFormatter.PutChar('/');
    serverFormatter.Put(sServerVersionStr);
    serverFormatter.PutChar(' ');

    serverFormatter.PutChar('(');
    serverFormatter.Put("Build/");
    serverFormatter.Put(sServerBuildStr);
    serverFormatter.Put("; ");
    serverFormatter.Put("Platform/");
    serverFormatter.Put(sServerPlatformStr);
    serverFormatter.PutChar(';');
 
    if (sServerCommentStr.Len > 0)
    {
        serverFormatter.PutChar(' ');
        serverFormatter.Put(sServerCommentStr);
    }
    serverFormatter.PutChar(')');


    sServerHeaderPtr.Len = serverFormatter.GetCurrentOffset();
    Assert(sServerHeaderPtr.Len < kMaxServerHeaderLen);
}

QTSServerInterface::QTSServerInterface()
 :  QTSSDictionary(QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kServerDictIndex), &fMutex),
    fSrvrPrefs(NULL),
    fSrvrMessages(NULL),
    fServerState(qtssStartingUpState),
    fDefaultIPAddr(0),
    fListeners(NULL),
    fNumListeners(0),
    fStartupTime_UnixMilli(0),
    fGMTOffset(0),
    fNumHTTPSessions(0),
    fCPUPercent(0),
    fCPUTimeUsedInSec(0),
    fSigInt(false),
    fSigTerm(false),
    fDebugLevel(0),
    fDebugOptions(0),    
    fMaxLate(0),
    fTotalLate(0),
    fCurrentMaxLate(0),
    fTotalQuality(0),
    fNumThinned(0),
    fNumThreads(0),
	fIfConSucess(false)//add
{
    for (UInt32 y = 0; y < QTSSModule::kNumRoles; y++)
    {
        sModuleArray[y] = NULL;
        sNumModulesInRole[y] = 0;
    }

    this->SetVal(qtssSvrState,              &fServerState,              sizeof(fServerState));
    this->SetVal(qtssServerAPIVersion,      &sServerAPIVersion,         sizeof(sServerAPIVersion));
    this->SetVal(qtssSvrDefaultIPAddr,      &fDefaultIPAddr,            sizeof(fDefaultIPAddr));
    this->SetVal(qtssSvrServerName,         sServerNameStr.Ptr,         sServerNameStr.Len);
    this->SetVal(qtssSvrServerVersion,      sServerVersionStr.Ptr,      sServerVersionStr.Len);
    this->SetVal(qtssSvrServerBuildDate,    sServerBuildDateStr.Ptr,    sServerBuildDateStr.Len);
    this->SetVal(qtssSvrHTTPServerHeader,   sServerHeaderPtr.Ptr,       sServerHeaderPtr.Len);
    this->SetVal(qtssCurrentSessionCount,	&fNumHTTPSessions,		sizeof(fNumHTTPSessions));
    this->SetVal(qtssSvrStartupTime,        &fStartupTime_UnixMilli,    sizeof(fStartupTime_UnixMilli));
    this->SetVal(qtssSvrGMTOffsetInHrs,     &fGMTOffset,                sizeof(fGMTOffset));
    this->SetVal(qtssSvrCPULoadPercent,     &fCPUPercent,               sizeof(fCPUPercent));

    this->SetVal(qtssSvrServerBuild,        sServerBuildStr.Ptr,    sServerBuildStr.Len);
    this->SetVal(qtssSvrHTTPServerComment,  sServerCommentStr.Ptr,  sServerCommentStr.Len);
    this->SetVal(qtssSvrServerPlatform,     sServerPlatformStr.Ptr, sServerPlatformStr.Len);

    this->SetVal(qtssSvrNumThinned,         &fNumThinned,               sizeof(fNumThinned));
    this->SetVal(qtssSvrNumThreads,         &fNumThreads,               sizeof(fNumThreads));

	qtss_sprintf(fDMSServiceID, "EasyCMS%s", "Config EasyCMS Uid");
    
    sServer = this;

}

void QTSServerInterface::LogError(QTSS_ErrorVerbosity inVerbosity, char* inBuffer)
{
    QTSS_RoleParams theParams;
    theParams.errorParams.inVerbosity = inVerbosity;
    theParams.errorParams.inBuffer = inBuffer;

    for (UInt32 x = 0; x < QTSServerInterface::GetNumModulesInRole(QTSSModule::kErrorLogRole); x++)
        (void)QTSServerInterface::GetModule(QTSSModule::kErrorLogRole, x)->CallDispatch(QTSS_ErrorLog_Role, &theParams);

    // If this is a fatal error, set the proper attribute in the RTSPServer dictionary
    if ((inVerbosity == qtssFatalVerbosity) && (sServer != NULL))
    {
        QTSS_ServerState theState = qtssFatalErrorState;
        (void)sServer->SetValue(qtssSvrState, 0, &theState, sizeof(theState));
    }
}

void QTSServerInterface::SetValueComplete(UInt32 inAttrIndex, QTSSDictionaryMap* inMap,
							UInt32 inValueIndex, void* inNewValue, UInt32 inNewValueLen)
{
    if (inAttrIndex == qtssSvrState)
    {
        Assert(inNewValueLen == sizeof(QTSS_ServerState));
        
        //
        // Invoke the server state change role
        QTSS_RoleParams theParams;
        theParams.stateChangeParams.inNewState = *(QTSS_ServerState*)inNewValue;
        
        static QTSS_ModuleState sStateChangeState = { NULL, 0, NULL, false };
        if (OSThread::GetCurrent() == NULL)
            OSThread::SetMainThreadData(&sStateChangeState);
        else
            OSThread::GetCurrent()->SetThreadData(&sStateChangeState);

        UInt32 numModules = QTSServerInterface::GetNumModulesInRole(QTSSModule::kStateChangeRole);
        {
            for (UInt32 theCurrentModule = 0; theCurrentModule < numModules; theCurrentModule++)
            {  
                QTSSModule* theModule = QTSServerInterface::GetModule(QTSSModule::kStateChangeRole, theCurrentModule);
                (void)theModule->CallDispatch(QTSS_StateChange_Role, &theParams);
            }
        }

        //
        // Make sure to clear out the thread data
        if (OSThread::GetCurrent() == NULL)
            OSThread::SetMainThreadData(NULL);
        else
            OSThread::GetCurrent()->SetThreadData(NULL);
    }
}

void* QTSServerInterface::CurrentUnixTimeMilli(QTSSDictionary* inServer, UInt32* outLen)
{
    QTSServerInterface* theServer = (QTSServerInterface*)inServer;
    theServer->fCurrentTime_UnixMilli = OS::TimeMilli_To_UnixTimeMilli(OS::Milliseconds()); 
    
    // Return the result
    *outLen = sizeof(theServer->fCurrentTime_UnixMilli);
    return &theServer->fCurrentTime_UnixMilli;
}

void* QTSServerInterface::IsOutOfDescriptors(QTSSDictionary* inServer, UInt32* outLen)
{
    QTSServerInterface* theServer = (QTSServerInterface*)inServer;
    
    theServer->fIsOutOfDescriptors = false;
    for (UInt32 x = 0; x < theServer->fNumListeners; x++)
    {
        if (theServer->fListeners[x]->IsOutOfDescriptors())
        {
            theServer->fIsOutOfDescriptors = true;
            break;
        }
    }
    // Return the result
    *outLen = sizeof(theServer->fIsOutOfDescriptors);
    return &theServer->fIsOutOfDescriptors;
}

QTSS_Error  QTSSErrorLogStream::Write(void* inBuffer, UInt32 inLen, UInt32* outLenWritten, UInt32 inFlags)
{
	// For the error log stream, the flags are considered to be the verbosity
	// of the error.
	if (inFlags >= qtssIllegalVerbosity)
		inFlags = qtssMessageVerbosity;

	QTSServerInterface::LogError(inFlags, (char*)inBuffer);
	if (outLenWritten != NULL)
		*outLenWritten = inLen;
        
	return QTSS_NoErr;
}

void QTSSErrorLogStream::LogAssert(char* inMessage)
{
    QTSServerInterface::LogError(qtssAssertVerbosity, inMessage);
}

//redis,add,begin
bool QTSServerInterface::ConRedis()//连接redis服务器
{
	if(fIfConSucess)
		return true;

	struct timeval timeout = { 0, 500000 }; // 0.5 seconds
	fRedisCon = redisConnectWithTimeout(fRedisIP.c_str(), fRedisPort, timeout);//test,redis的ip和端口应该在xml中指定
	if (fRedisCon == NULL || fRedisCon->err)
	{
		if (fRedisCon) {
			qtss_printf("INFO:Connect redis failed,%s\n", fRedisCon->errstr);
			redisFree(fRedisCon);
		} else {
			qtss_printf("INFO:Connect redis failed,can't allocate redis context\n");
		}
		fIfConSucess=false;
	}
	else
	{
		fIfConSucess=true;
		struct timeval timeoutEx = { 1, 0 }; // 1seconds,设置socket接收和发送超时
		redisSetTimeout(fRedisCon,timeoutEx);

		RedisInit();//可能在这个函数的执行过程中，redis连接又失败了，所以真正的连接失败或者成功要看fIfConSucess
		qtss_printf("INFO:Connect redis sucess\n");
	}
	return fIfConSucess;
}
bool QTSServerInterface::RedisCommand(const char * strCommand)//执行一些不关心结果的命令，只要知道成功或者失败，一般为写
{
	OSMutexLocker mutexLock(&fRedisMutex);

	if(!ConRedis())//每一次执行命令之前都先连接redis,如果当前redis还没有成功连接
		return false;
	redisReply* reply = (redisReply*)redisCommand(fRedisCon,strCommand);
	//需要注意的是，如果返回的对象是NULL，则表示客户端和服务器之间出现严重错误，必须重新链接。
	if (NULL == reply) 
	{
		redisFree(fRedisCon);
		fIfConSucess=false;
		return false;
	}
	//printf("%s\n",strCommand);
	//printf("%s\n",reply->str);
	freeReplyObject(reply);
	return true;
}
bool QTSServerInterface::RedisAddDevName(const char * strPushNmae)
{
	char chTemp[128]={0};//注意128位是否足够
	sprintf(chTemp,"sadd %s:%d_DevName %s",fCMSIP.c_str(),fCMSPort,strPushNmae);//将推流名称加入到set中
	return RedisCommand(chTemp);
}
bool QTSServerInterface::RedisDelDevName(const char * strPushNmae)
{
	char chTemp[128]={0};//注意128位是否足够
	sprintf(chTemp,"srem %s:%d_DevName %s",fCMSIP.c_str(),fCMSPort,strPushNmae);//将推流名称从set中移除
	return RedisCommand(chTemp);
}
bool  QTSServerInterface::RedisTTL()//注意当网络在一段时间很差时可能会因为超时时间达到而导致key被删除，这时应该重新设置该key
{

	OSMutexLocker mutexLock(&fRedisMutex);

	bool bReval=false;
	if(!ConRedis())//每一次执行命令之前都先连接redis,如果当前redis还没有成功连接
		return false;

	char chTemp[128]={0};//注意128位是否足够
	sprintf(chTemp,"expire  %s:%d_Live 15",fCMSIP.c_str(),fCMSPort);//更改超时时间

	redisReply* reply = (redisReply*)redisCommand(fRedisCon,chTemp);
	//需要注意的是，如果返回的对象是NULL，则表示客户端和服务器之间出现严重错误，必须重新链接。
	if (NULL == reply) 
	{
		redisFree(fRedisCon);
		fIfConSucess=false;
		return false;
	}

	if(reply->type==REDIS_REPLY_INTEGER&&reply->integer==1)//正常情况
	{
		bReval=true;
	}
	else if(reply->type==REDIS_REPLY_INTEGER&&reply->integer==0)//说明当前key已经不存在了,那么我们需要重新生成该key
	{
		sprintf(chTemp,"setex %s:%d_Live 15 1",fCMSIP.c_str(),fCMSPort);
		bReval=RedisCommand(chTemp);
	}
	else//其他情况
	{
		//当redis服务器不具有写的权限，但配置文件配置了持久化时，可能出现这种情况
		Assert(0);
	}
	freeReplyObject(reply);
	return bReval;
}
//以管线的方式执行串行命令，减少等待时间,但是如果下一个命令需要用到上一个命令的返回结果，则无法使用管线。
bool QTSServerInterface::RedisGetAssociatedDarWin(string& strDeviceSerial,string &strCameraSerial,string& strDssIP,string& strDssPort)//获得与当前设备序列号和摄像头序列号关联的EasyDarWin,也就说找到该设备正在推送的EasyDarWin
{
	//算法描述，获得darwin列表，对于每一个darwin判断其是否存活，如果存活，根据查找其正在推流列表看是否存在指定的设备序列号和摄像头序列号
	OSMutexLocker mutexLock(&fRedisMutex);
	
	if(!ConRedis())
		return false;

	bool bReVal=false;
	//由设备序列号和摄像序列号合成推流名称，EasyDarWIn在redis上的存储为：设备序列号/摄像头序列号.sdp
	string strPushName=strDeviceSerial+'/'+strCameraSerial+".sdp";
	char chTemp[128]={0};
	sprintf(chTemp,"smembers EasyDarWinName");

	
	redisReply* reply = (redisReply*)redisCommand(fRedisCon,chTemp);//获得EsayDarWin列表
	if (NULL == reply)//错误，需要进行重连
	{
		redisFree(fRedisCon);
		fIfConSucess=false;
		return false;
	}
	printf("%d\n",reply->type);
	if(reply->elements>0&&reply->type==REDIS_REPLY_ARRAY)//smembers返回的是数组
	{
		//这里可以使用管线
		redisReply* childReply=NULL;
		for(size_t i=0;i<reply->elements;i++)//对于每一个EasyDarWin判断其存活性和（与指定设备序列号和摄像头序列号）关联性
		{
			childReply		=	reply->element[i];
			string strChileReply(childReply->str);

			sprintf(chTemp,"exists %s",(strChileReply+"_Live").c_str());//判断EasyDarWin是否存活
			if(redisAppendCommand(fRedisCon,chTemp)!=REDIS_OK)
			{
				freeReplyObject(reply);
				redisFree(fRedisCon);
				fIfConSucess=false;
				return false;
			}
			
			sprintf(chTemp,"sismember %s %s",(strChileReply+"_PushName").c_str(),strPushName.c_str());//判断DarWin上是否存在该推流设备
			if(redisAppendCommand(fRedisCon,chTemp)!=REDIS_OK)
			{
				freeReplyObject(reply);
				redisFree(fRedisCon);
				fIfConSucess=false;
				return false;
			}
		}

		redisReply *reply2=NULL,*reply3=NULL;
		for(size_t i=0;i<reply->elements;i++)
		{
			if(redisGetReply(fRedisCon,(void**)&reply2)!=REDIS_OK)
			{
				freeReplyObject(reply);
				freeReplyObject(reply2);
				redisFree(fRedisCon);
				fIfConSucess=false;
				return false;
			}
			if(redisGetReply(fRedisCon,(void**)&reply3)!=REDIS_OK)
			{
				freeReplyObject(reply);
				freeReplyObject(reply3);
				redisFree(fRedisCon);
				fIfConSucess=false;
				return false;
			}

			if(reply2->type==REDIS_REPLY_INTEGER&&reply2->integer==1&&
				reply3->type==REDIS_REPLY_INTEGER&&reply3->integer==1)
			{//找到了
				string strIpPort(reply->element[i]->str);
				int ipos=strIpPort.find(':');//错误判断
				strDssIP=strIpPort.substr(0,ipos);
				strDssPort=strIpPort.substr(ipos+1,strIpPort.size()-ipos-1);
				

				//freeReplyObject(reply2);
				//freeReplyObject(reply3);
				bReVal=true;
				//break;//找到了也不能加break,而是需要执行完，因为对每一个请求必须有一个应答，否则次序就发生了混乱
			}
			freeReplyObject(reply2);
			freeReplyObject(reply3);
		}
	}
	else//没有可用的EasyDarWin
	{

	}
	freeReplyObject(reply);
	return bReVal;
}
bool QTSServerInterface::RedisGetBestDarWin(string& strDssIP,string& strDssPort)
{
	//算法描述：获取DarWin列表，然后获取每一个darWin的存活信息和RTP属性
	OSMutexLocker mutexLock(&fRedisMutex);

	if(!ConRedis())
		return false;

	bool bReVal=true;
	char chTemp[128]={0};
	sprintf(chTemp,"smembers EasyDarWinName");

	redisReply* reply = (redisReply*)redisCommand(fRedisCon,chTemp);//获得EsayDarWin列表
	if (NULL == reply)//错误，需要进行重连
	{
		redisFree(fRedisCon);
		fIfConSucess=false;
		return false;
	}

	if(reply->elements>0&&reply->type==REDIS_REPLY_ARRAY)//smembers返回的是数组
	{
		//这里可以使用管线
		redisReply* childReply=NULL;
		for(size_t i=0;i<reply->elements;i++)//对于每一个EasyDarWin判断其存活性和RTP属性
		{
			childReply		=	reply->element[i];
			string strChileReply(childReply->str);

			sprintf(chTemp,"exists %s",(strChileReply+"_Live").c_str());//判断EasyDarWin是否存活
			if(redisAppendCommand(fRedisCon,chTemp)!=REDIS_OK)
			{
				freeReplyObject(reply);
				redisFree(fRedisCon);
				fIfConSucess=false;
				return false;
			}
			
			sprintf(chTemp,"hget %s %s",(strChileReply+"_Info").c_str(),"RTP");//判断DarWin上是否存在该推流设备
			if(redisAppendCommand(fRedisCon,chTemp)!=REDIS_OK)
			{
				freeReplyObject(reply);
				redisFree(fRedisCon);
				fIfConSucess=false;
				return false;
			}
		}

		int key=-1,keynum=0;
		redisReply *reply2=NULL,*reply3=NULL;
		for(size_t i=0;i<reply->elements;i++)
		{
			if(redisGetReply(fRedisCon,(void**)&reply2)!=REDIS_OK)
			{
				freeReplyObject(reply);
				freeReplyObject(reply2);
				redisFree(fRedisCon);
				fIfConSucess=false;
				return false;
			}

			if(redisGetReply(fRedisCon,(void**)&reply3)!=REDIS_OK)
			{
				freeReplyObject(reply);
				freeReplyObject(reply3);
				redisFree(fRedisCon);
				fIfConSucess=false;
				return false;
			}

			if(reply2->type==REDIS_REPLY_INTEGER&&reply2->integer==1&&
				reply3->type==REDIS_REPLY_STRING)//如果filed对应的value存在，返回REDIS_REPLY_STRING，否则返回REDIS_REPLY_NIL
			{//找到了
				int RTPNum=atoi(reply3->str);
				if(key==-1)
				{
					key=i;
					keynum=RTPNum;
				}
				else
				{
					if(RTPNum<keynum)//找到更好的了
					{
						key=i;
						keynum=RTPNum;
					}
				}
			}
			freeReplyObject(reply2);
			freeReplyObject(reply3);
		}
		if(key==-1)//没有存活的
		{
			bReVal=false;
		}
		else
		{
			string strIpPort(reply->element[key]->str);
			int ipos	=		strIpPort.find(':');//错误判断
			strDssIP	=		strIpPort.substr(0,ipos);
			strDssPort	=		strIpPort.substr(ipos+1,strIpPort.size()-ipos-1);
		}
	}
	else//没有可用的EasyDarWin
	{
		bReVal=false;
	}
	freeReplyObject(reply);
	return bReVal;
}
bool QTSServerInterface::RedisGetAssociatedRms(string& strDeviceSerial,string &strCameraSerial,string& strRmsIP,string& strRmsPort)//获得与当前设备序列号和摄像头序列号关联的RMS
{
	//算法描述，获得RMS列表，对于每一个RMS判断其是否存活，如果存活，查找正在录像设备列表
	OSMutexLocker mutexLock(&fRedisMutex);

	if(!ConRedis())
		return false;

	bool bReVal=false;
	string strRecordingDevName=strDeviceSerial+'_'+strCameraSerial;//设备序列号_摄像头序列号
	char chTemp[128]={0};
	sprintf(chTemp,"smembers RMSName");

	redisReply* reply = (redisReply*)redisCommand(fRedisCon,chTemp);//获得RMS列表
	if (NULL == reply)//错误，需要进行重连
	{
		redisFree(fRedisCon);
		fIfConSucess=false;
		return false;
	}

	if(reply->elements>0&&reply->type==REDIS_REPLY_ARRAY)//smembers返回的是数组
	{
		//这里可以使用管线
		redisReply* childReply=NULL;
		for(size_t i=0;i<reply->elements;i++)//对于每一个RMS判断其存活性和（指定设备序列号和摄像头序列号）是否关联
		{
			childReply		=	reply->element[i];
			string strChileReply(childReply->str);

			sprintf(chTemp,"exists %s",(strChileReply+"_Live").c_str());//判断RMS是否存活
			if(redisAppendCommand(fRedisCon,chTemp)!=REDIS_OK)
			{
				freeReplyObject(reply);
				redisFree(fRedisCon);
				fIfConSucess=false;
				return false;
			}

			sprintf(chTemp,"sismember %s %s",(strChileReply+"_RecordingDevName").c_str(),strRecordingDevName.c_str());//判断RMS是否对该设备正在录像
			if(redisAppendCommand(fRedisCon,chTemp)!=REDIS_OK)
			{
				freeReplyObject(reply);
				redisFree(fRedisCon);
				fIfConSucess=false;
				return false;
			}
		}

		redisReply *reply2=NULL,*reply3=NULL;
		for(size_t i=0;i<reply->elements;i++)
		{
			if(redisGetReply(fRedisCon,(void**)&reply2)!=REDIS_OK)
			{
				freeReplyObject(reply);
				freeReplyObject(reply2);
				redisFree(fRedisCon);
				fIfConSucess=false;
				return false;
			}

			if(redisGetReply(fRedisCon,(void**)&reply3)!=REDIS_OK)
			{
				freeReplyObject(reply);
				freeReplyObject(reply3);
				redisFree(fRedisCon);
				fIfConSucess=false;
				return false;
			}

			if(reply2->type==REDIS_REPLY_INTEGER&&reply2->integer==1&&
				reply3->type==REDIS_REPLY_INTEGER&&reply3->integer==1)
			{//找到了
				string strIpPort(reply->element[i]->str);
				int ipos=strIpPort.find(':');//错误判断
				strRmsIP=strIpPort.substr(0,ipos);
				strRmsPort=strIpPort.substr(ipos+1,strIpPort.size()-ipos-1);


				//freeReplyObject(reply2);
				//freeReplyObject(reply3);
				bReVal=true;
				//break;
			}
			freeReplyObject(reply2);
			freeReplyObject(reply3);
		}
	}
	else//没有可用的RMS
	{

	}
	freeReplyObject(reply);
	return bReVal;
}
bool QTSServerInterface::RedisGetBestRms(string& strRmsIP,string& strRmsPort)//获得当前录像数最小的RMS
{
	//算法描述：获取RMS列表，然后获取每一个RMS的存活信息和正在录像个数
	OSMutexLocker mutexLock(&fRedisMutex);

	if(!ConRedis())
		return false;

	bool bReVal=true;
	char chTemp[128]={0};
	sprintf(chTemp,"smembers RMSName");

	redisReply* reply = (redisReply*)redisCommand(fRedisCon,chTemp);//获得EsayDarWin列表
	if (NULL == reply)//错误，需要进行重连
	{
		redisFree(fRedisCon);
		fIfConSucess=false;
		return false;
	}

	if(reply->elements>0&&reply->type==REDIS_REPLY_ARRAY)//smembers返回的是数组
	{
		//这里可以使用管线
		redisReply* childReply=NULL;
		for(size_t i=0;i<reply->elements;i++)//对于每一个EasyDarWin判断其存活性和RTP属性
		{
			childReply		=	reply->element[i];
			string strChileReply(childReply->str);

			sprintf(chTemp,"exists %s",(strChileReply+"_Live").c_str());//判断RMSn是否存活
			if(redisAppendCommand(fRedisCon,chTemp)!=REDIS_OK)
			{
				freeReplyObject(reply);
				redisFree(fRedisCon);
				fIfConSucess=false;
				return false;
			}

			sprintf(chTemp,"hget %s %s",(strChileReply+"_Info").c_str(),"RecordingCount");//获取RMS当前录像个数
			if(redisAppendCommand(fRedisCon,chTemp)!=REDIS_OK)
			{
				freeReplyObject(reply);
				redisFree(fRedisCon);
				fIfConSucess=false;
				return false;
			}
		}

		int key=-1,keynum=0;
		redisReply *reply2=NULL,*reply3=NULL;
		for(size_t i=0;i<reply->elements;i++)
		{
			if(redisGetReply(fRedisCon,(void**)&reply2)!=REDIS_OK)
			{
				freeReplyObject(reply);
				freeReplyObject(reply2);
				redisFree(fRedisCon);
				fIfConSucess=false;
				return false;
			}

			if(redisGetReply(fRedisCon,(void**)&reply3)!=REDIS_OK)
			{
				freeReplyObject(reply);
				freeReplyObject(reply3);
				redisFree(fRedisCon);
				fIfConSucess=false;
				return false;
			}

			if(reply2->type==REDIS_REPLY_INTEGER&&reply2->integer==1&&
				reply3->type==REDIS_REPLY_STRING)//如果filed对应的value存在，返回REDIS_REPLY_STRING，否则返回REDIS_REPLY_NIL
			{//找到了
				int RecordingNum=atoi(reply3->str);
				if(key==-1)
				{
					key=i;
					keynum=RecordingNum;
				}
				else
				{
					if(RecordingNum<keynum)//找到更好的了
					{
						key=i;
						keynum=RecordingNum;
					}
				}
			}
			freeReplyObject(reply2);
			freeReplyObject(reply3);
		}
		if(key==-1)//没有存活的
		{
			bReVal=false;
		}
		else
		{
			string strIpPort(reply->element[key]->str);
			int ipos	=		strIpPort.find(':');//错误判断
			strRmsIP	=		strIpPort.substr(0,ipos);
			strRmsPort	=		strIpPort.substr(ipos+1,strIpPort.size()-ipos-1);
		}
	}
	else//没有可用的EasyDarWin
	{
		bReVal=false;
	}
	freeReplyObject(reply);
	return bReVal;
}
bool QTSServerInterface::RedisGenSession(string& strSessioionID,UInt32 iTimeoutMil)//生成唯一的sessionID并存储在redis上
{
	//算法秒速，生成随机sessionID，看redis上是否有存储，没有就存在redis上，有的话就再生成，直到没有为止
	OSMutexLocker mutexLocker(&fRedisMutex);
	
	if(!ConRedis())
		return false;

	redisReply* reply=NULL;
	char chTemp[128]={0};
	
	do 
	{
		if(reply)//释放上一个回应
			freeReplyObject(reply);

		strSessioionID=OSMapEx::GenerateSessionIdForRedis(fCMSIP.c_str(),fCMSPort);//生成
		sprintf(chTemp,"exists SessionID_%s",strSessioionID.c_str());
		reply = (redisReply*)redisCommand(fRedisCon,chTemp);
		if (NULL == reply)//错误，需要进行重连
		{
			redisFree(fRedisCon);
			fIfConSucess=false;
			return false;
		}
	}
	while(reply->type==REDIS_REPLY_INTEGER&&reply->integer==1);
	freeReplyObject(reply);//释放最后一个的回应
	
	//走到这说明找到了一个唯一的SessionID，现在将它存储到redis上
	sprintf(chTemp,"setex SessionID_%s %d 1",strSessioionID.c_str(),iTimeoutMil/1000);//高级版本支持setpx来设置超时时间为ms
	return RedisCommand(chTemp);
}
bool QTSServerInterface::RedisInit()//连接redis成功之后调用该函数执行一些初始化的工作
{
	//每一次与redis连接后，都应该清除上一次的数据存储，使用覆盖或者直接清除的方式,串行命令使用管线更加高效
	char chTemp[128]={0};
	char chPassword[]="~ziguangwulian~iguangwulian~guangwulian~uangwulian";
	do 
	{
		//1,redis密码认证
		sprintf(chTemp,"auth %s",chPassword);
		if(redisAppendCommand(fRedisCon,chTemp)!=REDIS_OK)
			break;

		//2,CMS唯一信息存储(覆盖上一次的存储)
		sprintf(chTemp,"sadd CMSName %s:%d",fCMSIP.c_str(),fCMSPort);
		if(redisAppendCommand(fRedisCon,chTemp)!=REDIS_OK)
			break;

		//3,CMS属性存储,设置多个filed使用hmset，单个使用hset(覆盖上一次的存储)
		sprintf(chTemp,"hmset %s:%d_Info IP %s PORT %d",fCMSIP.c_str(),fCMSPort,fCMSIP.c_str(),fCMSPort);
		if(redisAppendCommand(fRedisCon,chTemp)!=REDIS_OK)
			break;
		//4,清除设备名称存储，因为连接之前和连接之后的设备可能一斤该发生了变化，因此必须先执行清楚操作
		sprintf(chTemp,"del %s:%d_DevName",fCMSIP.c_str(),fCMSPort);
		if(redisAppendCommand(fRedisCon,chTemp)!=REDIS_OK)
			break;

		OSMutex *mutexMap=fDeviceMap.GetMutex();
		OSHashMap  *deviceMap=fDeviceMap.GetMap();
		OSRefIt itRef;
		string strAllDevices;
		mutexMap->Lock();
		for(itRef = deviceMap->begin();itRef != deviceMap->end();itRef++)
		{
			strDevice *deviceInfo=(((HTTPSessionInterface*)(itRef->second->GetObjectPtr()))->GetDeviceInfo());
			strAllDevices=strAllDevices+' '+deviceInfo->serial_;
		}
		mutexMap->Unlock();

		char *chNewTemp=new char[strAllDevices.size()+128];//注意，这里不能再使用chTemp，因为长度不确定，可能导致缓冲区溢出
		//5,设备名称存储
		sprintf(chNewTemp,"sadd %s:%d_DevName%s",fCMSIP.c_str(),fCMSPort,strAllDevices.c_str());
		if(redisAppendCommand(fRedisCon,chNewTemp)!=REDIS_OK)
		{
			delete[] chNewTemp;
			break;
		}
		delete[] chNewTemp;

		//6,保活，设置15秒，这之后当前CMS已经开始提供服务了
		sprintf(chTemp,"setex %s:%d_Live 15 1",fCMSIP.c_str(),fCMSPort);
		if(redisAppendCommand(fRedisCon,chTemp)!=REDIS_OK)
			break;

		bool bBreak=false;
		redisReply* reply = NULL;
		for(int i=0;i<6;i++)
		{
			if(REDIS_OK != redisGetReply(fRedisCon,(void**)&reply))
			{
				bBreak=true;
				freeReplyObject(reply);
				break;
			}
			freeReplyObject(reply);
		}
		if(bBreak)//说明redisGetReply出现了错误
			break;
		return true;
	} while (0);
	//走到这说明出现了错误，需要进行重连,重连操作再下一次执行命令时进行,在这仅仅是置标志位
	redisFree(fRedisCon);
	fIfConSucess=false;
	
	return false;
}
//redis.add,end