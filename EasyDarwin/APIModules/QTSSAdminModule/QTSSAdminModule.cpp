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
	Copyright (c) 2013-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       QTSSAdminModule.cpp
    Contains:   Implements Admin module
*/

#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#endif // _WIN32

#ifndef __Win32__
    #include <unistd.h>     /* for getopt() et al */
#endif

#include <time.h>
#include <stdio.h>      /* for qtss_printf */
#include <stdlib.h>     /* for getloadavg & other useful stuff */
#include "QTSSAdminModule.h"
#include "OSArrayObjectDeleter.h"
#include "StringParser.h"
#include "StrPtrLen.h"
#include "QTSSModuleUtils.h"
#include "OSHashTable.h"
#include "OSMutex.h"
#include "StrPtrLen.h"
#include "OSRef.h"
#include "AdminElementNode.h"
#include "base64.h"
#include "OSMemory.h"
#include "md5.h"
#include "md5digest.h"
#include "OS.h"

#include "mongoose.h"
#include "frozen.h"

#include "QTSServerInterface.h"
#include "QTSServer.h"
#if __MacOSX__
#include <Security/Authorization.h>
#include <Security/AuthorizationTags.h>
#endif

#if __solaris__ || __linux__ || __sgi__ || __hpux__
	#include <crypt.h>
#endif

#define DEBUG_ADMIN_MODULE 0

class mongooseThread;

//**************************************************
#define kAuthNameAndPasswordBuffSize 512
#define kPasswordBuffSize kAuthNameAndPasswordBuffSize/2

// STATIC DATA
//**************************************************
#if DEBUG_ADMIN_MODULE
static UInt32	sRequestCount = 0;
#endif

static mongooseThread*		sMongooseThread = NULL;
static QTSS_Initialize_Params sQTSSparams;

//static char* sResponseHeader = "HTTP/1.0 200 OK\r\nServer: QTSS\r\nConnection: Close\r\nContent-Type: text/plain\r\n\r\n";
static char* sResponseHeader = "HTTP/1.0 200 OK";
static char* sUnauthorizedResponseHeader = "HTTP/1.1 401 Unauthorized\r\nWWW-Authenticate: Basic realm=\"QTSS/modules/admin\"\r\nServer: QTSS\r\nConnection: Close\r\nContent-Type: text/plain\r\n\r\n";
static char* sPermissionDeniedHeader = "HTTP/1.1 403 Forbidden\r\nConnection: Close\r\nContent-Type: text/html\r\n\r\n";
static char* sHTMLBody =  "<HTML><BODY>\n<P><b>Your request was denied by the server.</b></P>\n</BODY></HTML>\r\n\r\n";

static char* sVersionHeader = NULL;
static char* sConnectionHeader = "Connection: Close";
static char* kDefaultHeader = "Server: EasyDarwin";
static char* sContentType = "Content-Type: text/plain";
static char* sEOL = "\r\n";
static char* sEOM = "\r\n\r\n";
static char* sAuthRealm = "QTSS/modules/admin";
static char* sAuthResourceLocalPath = "/modules/admin/";

static QTSS_ServerObject        sServer = NULL;
static QTSS_ModuleObject        sModule = NULL;
static QTSS_ModulePrefsObject   sAdminPrefs = NULL;
static QTSS_ModulePrefsObject   sAccessLogPrefs = NULL;
static QTSS_ModulePrefsObject   sReflectorPrefs = NULL;
static QTSS_ModulePrefsObject	sHLSModulePrefs = NULL;

static QTSS_PrefsObject         sServerPrefs = NULL;
static AdminClass               *sAdminPtr = NULL;
static QueryURI                 *sQueryPtr = NULL;
static OSMutex*                 sAdminMutex = NULL;//admin module isn't reentrant
static UInt32                   sVersion=20030306;
static char *sDesc="Implements HTTP based Admin Protocol for accessing server attributes";
static char decodedLine[kAuthNameAndPasswordBuffSize] = { 0 };
static char codedLine[kAuthNameAndPasswordBuffSize] = { 0 }; 
static QTSS_TimeVal             sLastRequestTime = 0;
static UInt32                   sSessID = 0;

static StrPtrLen            sAuthRef("AuthRef");
#if __MacOSX__

static char*                sSecurityServerAuthKey = "com.apple.server.admin.streaming";
static AuthorizationItem    sRight = { sSecurityServerAuthKey, 0, NULL, 0 };
static AuthorizationRights  sRightSet = { 1, &sRight };
#endif

//ATTRIBUTES
//**************************************************
enum 
{ 
	kMaxRequestTimeIntervalMilli = 1000, 
	kDefaultRequestTimeIntervalMilli = 50
};
static UInt32 sDefaultRequestTimeIntervalMilli = kDefaultRequestTimeIntervalMilli;
static UInt32 sRequestTimeIntervalMilli = kDefaultRequestTimeIntervalMilli;

static Bool16 sAuthenticationEnabled = true;
static Bool16 sDefaultAuthenticationEnabled = true;

static Bool16 sLocalLoopBackOnlyEnabled = true;
static Bool16 sDefaultLocalLoopBackOnlyEnabled = true;

static Bool16 sEnableRemoteAdmin = true;
static Bool16 sDefaultEnableRemoteAdmin = true;

static QTSS_AttributeID sIPAccessListID = qtssIllegalAttrID;
static char*            sIPAccessList = NULL;
static char*            sLocalLoopBackAddress = "127.0.0.*";

static char*            sAdministratorGroup = NULL;
static char*            sDefaultAdministratorGroup = "admin";

static Bool16           sFlushing = false;
static QTSS_AttributeID sFlushingID = qtssIllegalAttrID;
static char*            sFlushingName = "QTSSAdminModuleFlushingState";
static UInt32           sFlushingLen = sizeof(sFlushing);

static QTSS_AttributeID sAuthenticatedID = qtssIllegalAttrID;
static char*            sAuthenticatedName = "QTSSAdminModuleAuthenticatedState";

//**************************************************
//web管理监听端口，默认80
static UInt16			sHttpPort = 8080;
static UInt16			sDefaultHttpPort = 80;

//web管理静态页加载路径
static char*			sDocumentRoot     = NULL;
static char*			sDefaultDocumentRoot = "./html/";

//**************************************************

static QTSS_Error QTSSAdminModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error Register(QTSS_Register_Params* inParams);
static QTSS_Error Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error FilterRequest(QTSS_Filter_Params* inParams);
static QTSS_Error RereadPrefs();
static QTSS_Error AuthorizeAdminRequest(QTSS_RTSPRequestObject request);
static Bool16 AcceptSession(QTSS_RTSPSessionObject inRTSPSession);


//***********************EasyDarwin WEB管理***********************

//用户认证
static	Bool16	EasyAdmin_UserAuthentication(const char* inUserName, const char* inPassword);

//获取服务累计运行时间(单位毫秒ms)
static	SInt64	EasyAdmin_GetServiceRunTime();

//===============系统栏S===============

	//---------------------------全局配置S---------------------------
	//获取RTSP端口
	static  UInt16	EasyAdmin_GetRTSPort();
	//设置RTSP端口
	static	void	EasyAdmin_SetRTSPort(UInt16 uPort);
	//获取HTTP服务端口
	static	UInt16	EasyAdmin_GetHTTPServicePort();
	//设置HTTP服务端口
	static	void	EasyAdmin_SetHTTPServicePort(UInt16 uPort);
	//获取流媒体文件目录
	static	char*	EasyAdmin_GetMoviesFolder();
	//设置流媒体文件目录
	static	void	EasyAdmin_SetMoviesFolder(char* folder);
	//获取日志文件目录
	static	char*	EasyAdmin_GetErrorLogFolder();
	//设置日志文件目录
	static	void	EasyAdmin_SetErrorLogFolder(char* folder);
	//获取WEB Admin端口
	static	UInt16	EasyAdmin_GetMongoosePort();
	//设置WEB Admin端口
	static	void	EasyAdmin_SetMongoosePort(UInt16 uPort);
	//获取Server版本信息
	static	char*	EasyAdmin_GetServerHeader();
	//---------------------------全局配置E---------------------------

	//---------------------------全局控制S---------------------------
	//重启EasyDarwin服务
	static	void	EasyAdmin_Restart();
	//---------------------------全局控制S---------------------------

//===============系统栏E===============


//===============RTSP直播栏S===============

	//---------------------------RTSP转发配置S---------------------------
	//转发缓冲时间获取
	static	UInt32	EasyAdmin_GetReflectBufferSecs();
	//设置缓冲时间
	static	void	EasyAdmin_SetReflectBufferSecs(UInt32 secs);
	//是否同步输出HLS
	static	bool	EasyAdmin_GetReflectHLSOutput();
	//设置是否同步输出HLS
	static	void	EasyAdmin_SetReflectHLSOutput(Bool16 hlsOutput);
	//---------------------------RTSP转发配置E---------------------------

//===============RTSP直播栏E===============


//===============HLS直播栏S===============

//---------------------------HLS配置S---------------------------

	//获取HLS分发的http服务地址
	static	char*	EasyAdmin_GetHlsHttpRoot();
	//设置HLS分发的http服务地址
	static	void	EasyAdmin_SetHlsHttpRoot(char* httpRoot);
	//获取HLS单个ts切片的时长
	static	UInt32	EasyAdmin_GetHlsTsDuration();
	//设置HLS单个ts切片的时长
	static	void	EasyAdmin_SetHlsTsDuration(UInt32 secs);
	//获取HLS ts切片数
	static	UInt32	EasyAdmin_GetHlsTsCapacity();
	//设置HLS ts切片数
	static	void	EasyAdmin_SetHlsTsCapacity(UInt32 uCapacity);
	//---------------------------HLS配置E---------------------------

	//---------------------------HLS列表S---------------------------
	//新增一路HLS直播
	static	bool	EasyAdmin_StartHLSession(char* inSessionName, const char* inRTSPUrl, UInt32 inTimeout);
	//结束一路HLS直播
	static	bool	EasyAdmin_StopHLSession(char* inSessionName);
	//获取HLS直播列表(json)
	static	char*	EasyAdmin_GetHLSessions();
	//获取RTSP推送列表(json)
	static	char*	EasyAdmin_GetRTSPSessions();
	//---------------------------HLS列表E---------------------------

//===============HLS直播栏E===============

//++++++++++++++++++++++++++++++++++++++++++++++++++++add arno
static const char *s_secret = ":-)";
static void generate_ssid(const char *user_name, const char *expiration_date,
						  char *ssid, size_t ssid_size) {
							  char hash[33];
							  mg_md5(hash, user_name, ":", expiration_date, ":", s_secret, NULL);
							  qtss_snprintf(ssid, ssid_size, "%s|%s|%s", user_name, expiration_date, hash);
}
static int check_auth(struct mg_connection *conn) {
	char name[100], password[100], ssid[100], expire[100], expire_epoch[100];
	mg_get_var(conn, "name", name, sizeof(name));
	mg_get_var(conn, "password", password, sizeof(password));
	if(EasyAdmin_UserAuthentication(name,password)) {
		time_t t = time(NULL) + 300; 
		qtss_snprintf(expire_epoch, sizeof(expire_epoch), "%lu", (unsigned long) t);
		strftime(expire, sizeof(expire), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));
		generate_ssid(name, expire_epoch, ssid, sizeof(ssid));
		mg_printf(conn,
			"HTTP/1.1 302 Moved\r\n"
			"Set-Cookie: ssid=%s; expire=\"%s\"; http-only; HttpOnly;\r\n"
			"Content-Length: 0\r\n"
			"Location: /\r\n\r\n",
			ssid, expire);
		return MG_TRUE;
	}
	else
	{
		mg_printf(conn, "HTTP/1.1 302 Moved\r\nLocation: %s\r\n\r\n","loginerror.html");
		return MG_FALSE;
	}
}
static int cookie_auth(struct mg_connection *conn) {
	char ssid[100], calculated_ssid[100], name[100], expire[100];
	if (strcmp(conn->uri, "/login.html") == 0||
		strcmp(conn->uri, "/loginerror.html") == 0||
		strstr(conn->uri,".js")!=NULL||
		strstr(conn->uri,".css")!=NULL||
		strstr(conn->uri,"images")!=NULL
		) {
			return MG_TRUE;
	}
	mg_parse_header(mg_get_header(conn, "Cookie"), "ssid", ssid, sizeof(ssid));
	if (sscanf(ssid, "%[^|]|%[^|]|", name, expire) == 2) {
		generate_ssid(name, expire, calculated_ssid, sizeof(calculated_ssid));
		if (strcmp(ssid, calculated_ssid) == 0) {
			return MG_TRUE;
		}
	}	
	mg_printf(conn, "HTTP/1.1 302 Moved\r\nLocation: %s\r\n\r\n", "login.html");
	return MG_FALSE;
}
void transform_msec(SInt64 msec, char *output, size_t size)
{

	SInt64 sec = msec / 1000;
	SInt64 day=sec / (60*60*24); 
	sec -= day * 60*60*24; 
	SInt64 hour=sec / (60*60);
	sec -= hour * 60*60; 
	SInt64 min = sec / 60; 
	sec -= min * 60; 
	qtss_snprintf(output, size, "%lld|%lld|%lld|%lld",day,hour, min, sec);

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static int serve_request(struct mg_connection *conn) 
{
	if ((strcmp(conn->uri, "/login.html") == 0&&strcmp(conn->request_method, "POST") == 0)||(strcmp(conn->uri, "/loginerror.html") == 0&&strcmp(conn->request_method, "POST") == 0)) {
		return  check_auth(conn);
	}
	if(strcmp(conn->uri, "/api/getServiceRunTime") == 0)
	{
		char output[128]={0};
		char sAjax[1024]={0};
		char* serverHeader = EasyAdmin_GetServerHeader();
		

		transform_msec(EasyAdmin_GetServiceRunTime(), output, sizeof(output));
		sprintf(sAjax, "{ \"RunTime\": \"%s\", \"version\": \"%s\"}",output,serverHeader);
		mg_printf_data(conn,sAjax);
		delete serverHeader;
		return MG_TRUE;
	}
	if(strcmp(conn->uri, "/api/setPort") == 0)
	{
		char n1[100],n2[100],n3[100],n4[100],n5[100],n6[100];
		int re=0;
		mg_get_var(conn,"n1",n1,sizeof(n1));
		mg_get_var(conn,"n2",n2,sizeof(n2));
		mg_get_var(conn,"n3",n3,sizeof(n3));
		mg_get_var(conn,"n4",n4,sizeof(n4));
		mg_get_var(conn,"n5",n5,sizeof(n5));
		mg_get_var(conn,"n6",n6,sizeof(n6));
		EasyAdmin_SetMongoosePort(atoi(n1));
		EasyAdmin_SetRTSPort(atoi(n2));
		EasyAdmin_SetHTTPServicePort(atoi(n3));
		EasyAdmin_SetReflectBufferSecs(atoi(n4));
		EasyAdmin_SetMoviesFolder(n5);
		EasyAdmin_SetErrorLogFolder(n6);
		mg_printf_data(conn,"{\"result\": %d}",re);
		return MG_TRUE;
	}
	if(strcmp(conn->uri, "/api/getPort") == 0)
	{
		char sAjax[1024]={0};
		char* moviesFolder = EasyAdmin_GetMoviesFolder();
		char* logFolder = EasyAdmin_GetErrorLogFolder();
		sprintf(sAjax, "{ \"MongoosePort\": %d , \"RTSPPort\": %d , \"HTTPPort\": %d,\"reflectbuffer\":%d,\"moviesfolder\":\"%s\",\"logfolder\":\"%s\"}",
			EasyAdmin_GetMongoosePort(),
			EasyAdmin_GetRTSPort(),
			EasyAdmin_GetHTTPServicePort(),
			EasyAdmin_GetReflectBufferSecs(),
			moviesFolder,
			logFolder);
		mg_printf_data(conn,sAjax);
		delete moviesFolder;
		delete logFolder;
		return MG_TRUE;
	}
	if(strcmp(conn->uri, "/api/restart") == 0)
	{

		int re=0;
		EasyAdmin_Restart();
		mg_printf_data(conn,"{\"result\": %d}",re);
		return MG_TRUE;
	}
	if(strcmp(conn->uri, "/api/setHLS") == 0)
	{
		char n1[100],n2[100],n3[100];
		int re=0;
		mg_get_var(conn,"n1",n1,sizeof(n1));
		mg_get_var(conn,"n2",n2,sizeof(n2));
		mg_get_var(conn,"n3",n3,sizeof(n3));
		EasyAdmin_SetHlsHttpRoot(n1);
		EasyAdmin_SetHlsTsDuration(atoi(n2));
		EasyAdmin_SetHlsTsCapacity(atoi(n3));
		mg_printf_data(conn,"{\"result\": %d}",re);
		return MG_TRUE;
	}
	if(strcmp(conn->uri, "/api/getHLS") == 0)
	{
		char sAjax[1024]={0};
		char* hlsHttpRoot = EasyAdmin_GetHlsHttpRoot();
		sprintf(sAjax, "{ \"httproot\":\"%s\" , \"tsd\": %d , \"tsc\": %d}",
			hlsHttpRoot,
			EasyAdmin_GetHlsTsDuration(),
			EasyAdmin_GetHlsTsCapacity());
		mg_printf_data(conn,sAjax);
		delete hlsHttpRoot;
		return MG_TRUE;
	}
	if(strcmp(conn->uri, "/api/setRTSP") == 0)
	{
		char n1[100],n2[100];
		int re=0;
		mg_get_var(conn,"n1",n1,sizeof(n1));
		mg_get_var(conn,"n2",n2,sizeof(n2));

		EasyAdmin_SetReflectBufferSecs(atoi(n1));
		EasyAdmin_SetReflectHLSOutput(atoi(n2));

		mg_printf_data(conn,"{\"result\": %d}",re);
		return MG_TRUE;
	}
	if(strcmp(conn->uri, "/api/getRTSP") == 0)
	{
		char sAjax[1024]={0};
		sprintf(sAjax, "{ \"buffersecs\":%d , \"out\": %d }",
			EasyAdmin_GetReflectBufferSecs(),
			EasyAdmin_GetReflectHLSOutput());
		mg_printf_data(conn,sAjax);
		return MG_TRUE;
	}
	
	if(strcmp(conn->uri, "/api/getHLSList") == 0)
	{
		char* sessionsJSON = EasyAdmin_GetHLSessions();
		mg_printf_data(conn,sessionsJSON);
		delete sessionsJSON;
		return MG_TRUE;
	}
	if(strcmp(conn->uri, "/api/getRTSPList") == 0)
	{
		char* sessionsJSON = EasyAdmin_GetRTSPSessions();
		mg_printf_data(conn,sessionsJSON);
		delete sessionsJSON;
		return MG_TRUE;
	}
	if(strcmp(conn->uri, "/api/addHLSList") == 0)
	{
		char n1[100],n2[100],n3[100];
		int re=0;
		mg_get_var(conn,"n1",n1,sizeof(n1));
		mg_get_var(conn,"n2",n2,sizeof(n2));
		mg_get_var(conn,"n3",n3,sizeof(n3));
		if(EasyAdmin_StartHLSession(n1, n2, atoi(n3)))
		{
			re=1;
		}
		mg_printf_data(conn,"{\"result\": %d}",re);
		return MG_TRUE;
	}
	if(strcmp(conn->uri, "/api/StopHLS") == 0)
	{
		char n1[100],n2[100],n3[100];
		int re=0;
		mg_get_var(conn,"n1",n1,sizeof(n1));
		if(EasyAdmin_StopHLSession(n1))
		{
			re=1;
		}
		mg_printf_data(conn,"{\"result\": %d}",re);
		return MG_TRUE;
	}
	return MG_FALSE; 
}
static int ev_handler(struct mg_connection *conn, enum mg_event ev) {
	switch (ev) {
		case MG_AUTH:
			return cookie_auth(conn);
		case MG_REQUEST: 
			return serve_request(conn);
		default: return MG_FALSE;
	}
}
//mongoose线程
class mongooseThread : public OSThread
{
    public:
        mongooseThread() : OSThread() {}
        virtual ~mongooseThread() {}

    private:
        void Entry();
};


//Mongoose
void mongooseThread::Entry()
{
	char sPath[MAX_PATH];
	strcpy(sPath,((QTSServer*)sServer)->sAbsolutePath);
	if((strlen(sPath)+strlen(sDocumentRoot))>=(MAX_PATH+1))
	{
		return;
	}
	::strncat(sPath,sDocumentRoot,strlen(sDocumentRoot));
	//printf("%s\n",sPath);
	
	struct mg_server *mongooseserver;
	// Create and configure the server
	mongooseserver = mg_create_server((void *) "1", ::ev_handler);
	
	//WEB监听端口
	char listening_port[6];
	sprintf(listening_port, "%d", sHttpPort);
	
	mg_set_option(mongooseserver, "listening_port", listening_port);
	mg_set_option(mongooseserver, "document_root", sPath); //donot use it

	//printf("mongoose listen on port:%s document path:%s \n", listening_port , sDocumentRoot);

//**********************************************************
	////获取RTSP端口
	//printf("RTSP Port:%d !!!!!!!!! \n", EasyAdmin_GetRTSPort());
	//EasyAdmin_SetRTSPort(888);

	////获取HTTP Service端口
	//printf("HTTP Service Port:%d !!!!!!!!! \n", EasyAdmin_GetHTTPServicePort());
	//EasyAdmin_SetHTTPServicePort(999);

	////获取Movies目录 
	//char* moviesFolder = EasyAdmin_GetMoviesFolder();
	//printf("Movies Folder:%s !!!!!!!!! \n", moviesFolder);
	//delete moviesFolder;
	//EasyAdmin_SetMoviesFolder("/etc/streaming/movies/");

	////获取日志目录 
	//char* logFolder = EasyAdmin_GetErrorLogFolder();
	//printf("Log Folder:%s !!!!!!!!! \n", logFolder);
	//delete logFolder;

	//char* serverHeader = EasyAdmin_GetServerHeader();
	//printf("%s \n", serverHeader);
	//delete serverHeader;

	//EasyAdmin_SetErrorLogFolder("/etc/streaming/Logs/");

	////获取转发缓冲时间
	//printf("Reflector Buffer Secs:%d \n", EasyAdmin_GetReflectBufferSecs());
	//EasyAdmin_SetReflectBufferSecs(8);

	////获取Mongoose端口
	//printf("Mongoose Port:%d \n", EasyAdmin_GetMongoosePort());
	//EasyAdmin_SetMongoosePort(818);

	////修改配置后重启
	//EasyAdmin_Restart();

	//printf("EasyAdmin RunTime:%ld \n", EasyAdmin_GetServiceRunTime());

	//printf("Get Reflector HLS Output Enable:%d \n", EasyAdmin_GetReflectHLSOutput());
	//EasyAdmin_SetReflectHLSOutput(true);

	////获取HLS分发的http服务地址
	//char* hlsHttpRoot = EasyAdmin_GetHlsHttpRoot();
	//printf("HLS HTTP Root:%s \n", hlsHttpRoot);
	//delete hlsHttpRoot;

	////设置HLS分发的http服务地址
	//EasyAdmin_SetHlsHttpRoot("http://8.8.8.8/888");

	////获取HLS单个ts切片的时长
	//printf("HLS TS Duration: %d \n",EasyAdmin_GetHlsTsDuration());
	////设置HLS单个ts切片的时长
	//EasyAdmin_SetHlsTsDuration(8);

	////获取HLS ts切片数
	//printf("HLS TS Capacity: %d \n",EasyAdmin_GetHlsTsCapacity());
	////设置HLS ts切片数
	//EasyAdmin_SetHlsTsCapacity(9);

	////启动一路HLS直播
	//EasyAdmin_StartHLSession("test", "rtsp://admin:admin@192.168.66.186/", 0);

	////获取HLS直播列表
	//char* sessionsJSON = EasyAdmin_GetHLSessions();
	//printf(sessionsJSON);
	//delete[] sessionsJSON;

	////停止一路HLS直播
	//EasyAdmin_StopHLSession("test");
//**********************************************************

	//run server
	for (;;) mg_poll_server((struct mg_server *) mongooseserver, 1000);
    mg_destroy_server(&mongooseserver);
}

#if !DEBUG_ADMIN_MODULE
    #define APITests_DEBUG() 
    #define ShowQuery_DEBUG()
#else
void ShowQuery_DEBUG()
{
    qtss_printf("======REQUEST #%"_U32BITARG_"======\n",++sRequestCount);
    StrPtrLen*  aStr;
    aStr = sQueryPtr->GetURL();
    qtss_printf("URL="); PRINT_STR(aStr); 

    aStr = sQueryPtr->GetQuery();
    qtss_printf("Query="); PRINT_STR(aStr); 

    aStr = sQueryPtr->GetParameters();
    qtss_printf("Parameters="); PRINT_STR(aStr); 

    aStr = sQueryPtr->GetCommand();
    qtss_printf("Command="); PRINT_STR(aStr); 
    qtss_printf("CommandID=%"_S32BITARG_" \n",sQueryPtr->GetCommandID());
    aStr = sQueryPtr->GetValue();
    qtss_printf("Value="); PRINT_STR(aStr); 
    aStr = sQueryPtr->GetType();
    qtss_printf("Type="); PRINT_STR(aStr); 
    aStr = sQueryPtr->GetAccess();
    qtss_printf("Access="); PRINT_STR(aStr); 
}       

void APITests_DEBUG()
{
    if (0)
    {   qtss_printf("QTSSAdminModule start tests \n");
    
        if (0)
        {
            qtss_printf("admin called locked \n");
            const int ksleeptime = 15;
            qtss_printf("sleeping for %d seconds \n",ksleeptime);
            sleep(ksleeptime);
            qtss_printf("done sleeping \n");
            qtss_printf("QTSS_GlobalUnLock \n");
            (void) QTSS_GlobalUnLock();
            qtss_printf("again sleeping for %d seconds \n",ksleeptime);
            sleep(ksleeptime);
        }
    
        if (0)
        {
            qtss_printf(" GET VALUE PTR TEST \n");

            QTSS_Object *sessionsPtr = NULL;
            UInt32      paramLen = sizeof(sessionsPtr);
            UInt32      numValues = 0;
            QTSS_Error  err = 0;
            
            err = QTSS_GetNumValues (sServer, qtssSvrClientSessions, &numValues);
            err = QTSS_GetValuePtr(sServer, qtssSvrClientSessions, 0, (void**)&sessionsPtr, &paramLen);
            qtss_printf("Admin Module Num Sessions = %"_U32BITARG_" sessions[0] = %"_S32BITARG_" err = %"_S32BITARG_" paramLen =%"_U32BITARG_"\n", numValues, (SInt32) *sessionsPtr,err,paramLen);
    
            UInt32      numAttr = 0;
            if (sessionsPtr)
            {   err = QTSS_GetNumAttributes (*sessionsPtr, &numAttr);
                qtss_printf("Admin Module Num attributes = %"_U32BITARG_" sessions[0] = %"_S32BITARG_"  err = %"_S32BITARG_"\n", numAttr, (SInt32) *sessionsPtr,err);
        
                QTSS_Object theAttributeInfo;
                char nameBuff[128];
                UInt32 len = 127;
                for (UInt32 i = 0; i < numAttr; i++)
                {   err = QTSS_GetAttrInfoByIndex(*sessionsPtr, i, &theAttributeInfo);
                    nameBuff[0] = 0;len = 127;
                    err = QTSS_GetValue (theAttributeInfo, qtssAttrName,0, nameBuff,&len);
                    nameBuff[len] = 0;
                    qtss_printf("found %s \n",nameBuff);
                }
            }
        }
        
        if (0)
        {
            qtss_printf(" GET VALUE TEST \n");

            QTSS_Object sessions = NULL;
            UInt32      paramLen = sizeof(sessions);
            UInt32      numValues = 0;
            QTSS_Error  err = 0;
            
            err = QTSS_GetNumValues (sServer, qtssSvrClientSessions, &numValues);
            err = QTSS_GetValue(sServer, qtssSvrClientSessions, 0, (void*)&sessions, &paramLen);
            qtss_printf("Admin Module Num Sessions = %"_U32BITARG_" sessions[0] = %"_S32BITARG_" err = %"_S32BITARG_" paramLen = %"_U32BITARG_"\n", numValues, (SInt32) sessions,err, paramLen);
            
            if (sessions)
            {
                UInt32      numAttr = 0;
                err = QTSS_GetNumAttributes (sessions, &numAttr);
                qtss_printf("Admin Module Num attributes = %"_U32BITARG_" sessions[0] = %"_S32BITARG_"  err = %"_S32BITARG_"\n", numAttr,(SInt32) sessions,err);
                
                QTSS_Object theAttributeInfo;
                char nameBuff[128];
                UInt32 len = 127;
                for (UInt32 i = 0; i < numAttr; i++)
                {   err = QTSS_GetAttrInfoByIndex(sessions, i, &theAttributeInfo);
                    nameBuff[0] = 0;len = 127;
                    err = QTSS_GetValue (theAttributeInfo, qtssAttrName,0, nameBuff,&len);
                    nameBuff[len] = 0;
                    qtss_printf("found %s \n",nameBuff);
                }
            }
        }
        

        if (0)
        {
            qtss_printf("----------------- Start test ----------------- \n");
            qtss_printf(" GET indexed pref TEST \n");

            QTSS_Error  err = 0;
            
            UInt32      numAttr = 1;
            err = QTSS_GetNumAttributes (sAdminPrefs, &numAttr);
            qtss_printf("Admin Module Num preference attributes = %"_U32BITARG_" err = %"_S32BITARG_"\n", numAttr, err);
                
            QTSS_Object theAttributeInfo;
            char valueBuff[512];
            char nameBuff[128];
            QTSS_AttributeID theID;
            UInt32 len = 127;
            UInt32 i = 0;
            qtss_printf("first pass over preferences\n");
            for ( i = 0; i < numAttr; i++)
            {   err = QTSS_GetAttrInfoByIndex(sAdminPrefs, i, &theAttributeInfo);
                nameBuff[0] = 0;len = 127;
                err = QTSS_GetValue (theAttributeInfo, qtssAttrName,0, nameBuff,&len);
                nameBuff[len]=0;

                theID = qtssIllegalAttrID; len = sizeof(theID);
                err = QTSS_GetValue (theAttributeInfo, qtssAttrID,0, &theID,&len);
                qtss_printf("found preference=%s \n",nameBuff);
            }
            valueBuff[0] = 0;len = 512;
            err = QTSS_GetValue (sAdminPrefs, theID,0, valueBuff,&len);valueBuff[len] = 0;
            qtss_printf("Admin Module QTSS_GetValue name = %s id = %"_S32BITARG_" value=%s err = %"_S32BITARG_"\n", nameBuff,theID, valueBuff, err);
            err = QTSS_SetValue (sAdminPrefs,theID,0, valueBuff,len);
            qtss_printf("Admin Module QTSS_SetValue name = %s id = %"_S32BITARG_" value=%s err = %"_S32BITARG_"\n", nameBuff,theID, valueBuff, err);
            
            {   QTSS_ServiceID id;
                (void) QTSS_IDForService(QTSS_REREAD_PREFS_SERVICE, &id);           
                (void) QTSS_DoService(id, NULL);
            }

            valueBuff[0] = 0;len = 512;
            err = QTSS_GetValue (sAdminPrefs, theID,0, valueBuff,&len);valueBuff[len] = 0;
            qtss_printf("Admin Module QTSS_GetValue name = %s id = %"_S32BITARG_" value=%s err = %"_S32BITARG_"\n", nameBuff,theID, valueBuff, err);
            err = QTSS_SetValue (sAdminPrefs,theID,0, valueBuff,len);
            qtss_printf("Admin Module QTSS_SetValue name = %s id = %"_S32BITARG_" value=%s err = %"_S32BITARG_"\n", nameBuff,theID, valueBuff, err);
                
            qtss_printf("second pass over preferences\n");
            for ( i = 0; i < numAttr; i++)
            {   err = QTSS_GetAttrInfoByIndex(sAdminPrefs, i, &theAttributeInfo);
                nameBuff[0] = 0;len = 127;
                err = QTSS_GetValue (theAttributeInfo, qtssAttrName,0, nameBuff,&len);
                nameBuff[len]=0;

                theID = qtssIllegalAttrID; len = sizeof(theID);
                err = QTSS_GetValue (theAttributeInfo, qtssAttrID,0, &theID,&len);
                qtss_printf("found preference=%s \n",nameBuff);
            }
            qtss_printf("----------------- Done test ----------------- \n");
        }
            
    }
}

#endif

inline void KeepSession(QTSS_RTSPRequestObject theRequest,Bool16 keep)
{
    (void)QTSS_SetValue(theRequest, qtssRTSPReqRespKeepAlive, 0, &keep, sizeof(keep));
}

// FUNCTION IMPLEMENTATIONS

QTSS_Error QTSSAdminModule_Main(void* inPrivateArgs)
{
    return _stublibrary_main(inPrivateArgs, QTSSAdminModuleDispatch);
}


QTSS_Error  QTSSAdminModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
    switch (inRole)
    {
        case QTSS_Register_Role:
            return Register(&inParams->regParams);
        case QTSS_Initialize_Role:
            return Initialize(&inParams->initParams);
        case QTSS_RTSPFilter_Role:
        {
			if (!sEnableRemoteAdmin) 
                break;
            return FilterRequest(&inParams->rtspFilterParams);
        }
        case QTSS_RTSPAuthorize_Role:
                return AuthorizeAdminRequest(inParams->rtspRequestParams.inRTSPRequest);
        case QTSS_RereadPrefs_Role:
            return RereadPrefs();
    }
    return QTSS_NoErr;
}

QTSS_Error Register(QTSS_Register_Params* inParams)
{
    // Do role & attribute setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_RTSPFilter_Role);
    (void)QTSS_AddRole(QTSS_RereadPrefs_Role);
    (void)QTSS_AddRole(QTSS_RTSPAuthorize_Role);
    
    (void)QTSS_AddStaticAttribute(qtssRTSPRequestObjectType, sFlushingName, NULL, qtssAttrDataTypeBool16);
    (void)QTSS_IDForAttr(qtssRTSPRequestObjectType, sFlushingName, &sFlushingID);

    (void)QTSS_AddStaticAttribute(qtssRTSPRequestObjectType, sAuthenticatedName, NULL, qtssAttrDataTypeBool16);
    (void)QTSS_IDForAttr(qtssRTSPRequestObjectType, sAuthenticatedName, &sAuthenticatedID);

    // Tell the server our name!
    static char* sModuleName = "QTSSAdminModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    return QTSS_NoErr;
}

QTSS_Error RereadPrefs()
{   

    delete [] sVersionHeader;
    sVersionHeader = QTSSModuleUtils::GetStringAttribute(sServer, "qtssSvrRTSPServerHeader", kDefaultHeader);

    delete [] sIPAccessList;
    sIPAccessList = QTSSModuleUtils::GetStringAttribute(sAdminPrefs, "IPAccessList", sLocalLoopBackAddress);
    sIPAccessListID = QTSSModuleUtils::GetAttrID(sAdminPrefs, "IPAccessList");

    QTSSModuleUtils::GetAttribute(sAdminPrefs, "Authenticate",     qtssAttrDataTypeBool16, &sAuthenticationEnabled, &sDefaultAuthenticationEnabled, sizeof(sAuthenticationEnabled));
    QTSSModuleUtils::GetAttribute(sAdminPrefs, "LocalAccessOnly",  qtssAttrDataTypeBool16, &sLocalLoopBackOnlyEnabled, &sDefaultLocalLoopBackOnlyEnabled, sizeof(sLocalLoopBackOnlyEnabled));
    QTSSModuleUtils::GetAttribute(sAdminPrefs, "RequestTimeIntervalMilli",     qtssAttrDataTypeUInt32, &sRequestTimeIntervalMilli, &sDefaultRequestTimeIntervalMilli, sizeof(sRequestTimeIntervalMilli));
    QTSSModuleUtils::GetAttribute(sAdminPrefs, "enable_remote_admin",  qtssAttrDataTypeBool16, &sEnableRemoteAdmin, &sDefaultEnableRemoteAdmin, sizeof(sDefaultEnableRemoteAdmin));

	QTSSModuleUtils::GetAttribute(sAdminPrefs, "http_port",     qtssAttrDataTypeUInt16, &sHttpPort, &sDefaultHttpPort, sizeof(sHttpPort));

	delete [] sDocumentRoot;
    sDocumentRoot = QTSSModuleUtils::GetStringAttribute(sAdminPrefs, "document_root", sDefaultDocumentRoot);
    
    delete [] sAdministratorGroup;
    sAdministratorGroup = QTSSModuleUtils::GetStringAttribute(sAdminPrefs, "AdministratorGroup", sDefaultAdministratorGroup);
    
    if (sRequestTimeIntervalMilli > kMaxRequestTimeIntervalMilli) 
    {   
		sRequestTimeIntervalMilli = kMaxRequestTimeIntervalMilli;
    }

    (void)QTSS_SetValue(sModule, qtssModDesc, 0, sDesc, strlen(sDesc)+1);   
    (void)QTSS_SetValue(sModule, qtssModVersion, 0, &sVersion, sizeof(sVersion));   

    return QTSS_NoErr;
}

QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{   
    sAdminMutex = NEW OSMutex();
    ElementNode_InitPtrArray();
    // Setup module utils
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);

    sQTSSparams = *inParams;
    sServer = inParams->inServer;
    sModule = inParams->inModule;
	
	sAccessLogPrefs = QTSSModuleUtils::GetModulePrefsObject(QTSSModuleUtils::GetModuleObjectByName("QTSSAccessLogModule"));
    sReflectorPrefs = QTSSModuleUtils::GetModulePrefsObject(QTSSModuleUtils::GetModuleObjectByName("QTSSReflectorModule"));
	sHLSModulePrefs = QTSSModuleUtils::GetModulePrefsObject(QTSSModuleUtils::GetModuleObjectByName("EasyHLSModule"));

    sAdminPrefs = QTSSModuleUtils::GetModulePrefsObject(sModule);
    sServerPrefs = inParams->inPrefs;

	RereadPrefs();
    
	//create Mongoose thread,start
	sMongooseThread = NEW mongooseThread();
	sMongooseThread->Start();
    
    return QTSS_NoErr;
}
    
void ReportErr(QTSS_Filter_Params* inParams, UInt32 err)
{   
    StrPtrLen* urlPtr = sQueryPtr->GetURL();
    StrPtrLen* evalMessagePtr = sQueryPtr->GetEvalMsg();
    char temp[32];
    
    if (urlPtr && evalMessagePtr)   
    {   qtss_sprintf(temp,"(%"_U32BITARG_")",err);
        (void)QTSS_Write(inParams->inRTSPRequest, "error:", strlen("error:"), NULL, 0);
        (void)QTSS_Write(inParams->inRTSPRequest, temp, strlen(temp), NULL, 0);
        if (sQueryPtr->VerboseParam())
        {   (void)QTSS_Write(inParams->inRTSPRequest, ";URL=", strlen(";URL="), NULL, 0);
            if (urlPtr) (void)QTSS_Write(inParams->inRTSPRequest, urlPtr->Ptr, urlPtr->Len, NULL, 0);
        }
        if (sQueryPtr->DebugParam())
        {
            (void)QTSS_Write(inParams->inRTSPRequest, ";", strlen(";"), NULL, 0);
            (void)QTSS_Write(inParams->inRTSPRequest, evalMessagePtr->Ptr, evalMessagePtr->Len, NULL, 0);               
        }
        (void)QTSS_Write(inParams->inRTSPRequest, "\r\n\r\n", 4, NULL, 0);
    }
}


inline Bool16 AcceptAddress(StrPtrLen *theAddressPtr)
{
    IPComponentStr ipComponentStr(theAddressPtr);
   
    Bool16 isLocalRequest = ipComponentStr.IsLocal();   
    if (sLocalLoopBackOnlyEnabled && isLocalRequest)
        return true;
     
     if (sLocalLoopBackOnlyEnabled && !isLocalRequest)
        return false;
    
    if  (QTSSModuleUtils::AddressInList(sAdminPrefs, sIPAccessListID, theAddressPtr))
        return true;

    return false;
}

inline Bool16 IsAdminRequest(StringParser *theFullRequestPtr)
{
    Bool16 handleRequest = false;
    if (theFullRequestPtr != NULL) do
    {
        StrPtrLen   strPtr;
        theFullRequestPtr->ConsumeWord(&strPtr);
        if ( !strPtr.Equal(StrPtrLen("GET")) ) break;   //it's a "Get" request
        
        theFullRequestPtr->ConsumeWhitespace();
        if ( !theFullRequestPtr->Expect('/') ) break;   
                
        theFullRequestPtr->ConsumeWord(&strPtr);
        if ( strPtr.Len == 0 || !strPtr.Equal(StrPtrLen("modules") )    ) break;
        if (!theFullRequestPtr->Expect('/') ) break;
            
        theFullRequestPtr->ConsumeWord(&strPtr);
        if ( strPtr.Len == 0 || !strPtr.Equal(StrPtrLen("admin") ) ) break;
        handleRequest = true;
        
    } while (false);

    return handleRequest;
}

inline void ParseAuthNameAndPassword(StrPtrLen *codedStrPtr, StrPtrLen* namePtr, StrPtrLen* passwordPtr)
 {
    if (!codedStrPtr || (codedStrPtr->Len >= kAuthNameAndPasswordBuffSize) ) 
    {  
		return; 
    }
    
    StrPtrLen   codedLineStr;
    StrPtrLen   nameAndPassword;
    memset(decodedLine,0,kAuthNameAndPasswordBuffSize);
    memset(codedLine,0,kAuthNameAndPasswordBuffSize);
    
    memcpy (codedLine,codedStrPtr->Ptr,codedStrPtr->Len);
    codedLineStr.Set((char*) codedLine, codedStrPtr->Len);  
    (void) Base64decode(decodedLine, codedLineStr.Ptr);
    
    nameAndPassword.Set((char*) decodedLine, strlen(decodedLine));
    StringParser parsedNameAndPassword(&nameAndPassword);
    
    parsedNameAndPassword.ConsumeUntil(namePtr,':');            
    parsedNameAndPassword.ConsumeLength(NULL, 1);

    // password can have whitespace, so read until the end of the line, not just until whitespace
    parsedNameAndPassword.ConsumeUntil(passwordPtr, StringParser::sEOLMask);
    
    namePtr->Ptr[namePtr->Len]= 0;
    passwordPtr->Ptr[passwordPtr->Len]= 0;
    
    //qtss_printf("decoded nameAndPassword="); PRINT_STR(&nameAndPassword); 
    //qtss_printf("decoded name="); PRINT_STR(namePtr); 
    //qtss_printf("decoded password="); PRINT_STR(passwordPtr); 

    return;
};


inline Bool16 OSXAuthenticate(StrPtrLen *keyStrPtr)
{
#if __MacOSX__
//  Authorization: AuthRef QWxhZGRpbjpvcGVuIHNlc2FtZQ==
    Bool16 result = false;
    
    if (keyStrPtr == NULL || keyStrPtr->Len  == 0)
        return result;
    
    char *encodedKey = keyStrPtr->GetAsCString();
    OSCharArrayDeleter encodedKeyDeleter(encodedKey);

    char *decodedKey = NEW char[Base64decode_len(encodedKey) + 1];
    OSCharArrayDeleter decodedKeyDeleter(decodedKey);

    (void) Base64decode(decodedKey, encodedKey);

    AuthorizationExternalForm  *receivedExtFormPtr = (AuthorizationExternalForm  *) decodedKey;
    AuthorizationRef  receivedAuthorization;
    OSStatus status = AuthorizationCreateFromExternalForm(receivedExtFormPtr, &receivedAuthorization);

    if (status != errAuthorizationSuccess) 
        return result;
        
    status = AuthorizationCopyRights(receivedAuthorization, &sRightSet, kAuthorizationEmptyEnvironment, kAuthorizationFlagExtendRights , NULL);
    if (status == errAuthorizationSuccess)
    {
        result = true;
    }

    AuthorizationFree(receivedAuthorization, kAuthorizationFlagDestroyRights);

    return result;

#else

    return false;

#endif

}

inline Bool16 HasAuthentication(StringParser *theFullRequestPtr, StrPtrLen* namePtr, StrPtrLen* passwordPtr, StrPtrLen* outAuthTypePtr)
{
//  Authorization: Basic QWxhZGRpbjpvcGVuIHNlc2FtZQ==
    Bool16 hasAuthentication = false;
    StrPtrLen   strPtr; 
    StrPtrLen   authType;
    StrPtrLen   authString;
    while (theFullRequestPtr->GetDataRemaining() > 0)
    {
        theFullRequestPtr->ConsumeWhitespace();     
        theFullRequestPtr->ConsumeUntilWhitespace(&strPtr);
        if ( strPtr.Len == 0 || !strPtr.Equal(StrPtrLen("Authorization:")) ) 
            continue;   
                
        theFullRequestPtr->ConsumeWhitespace();     
        theFullRequestPtr->ConsumeUntilWhitespace(&authType);
        if ( authType.Len == 0 ) 
            continue;

        theFullRequestPtr->ConsumeWhitespace();                 
        theFullRequestPtr->ConsumeUntil(&authString, StringParser::sEOLMask);
        if ( authString.Len == 0 ) 
            continue;

        if (outAuthTypePtr != NULL)
            outAuthTypePtr->Set(authType.Ptr, authType.Len);

        if (authType.Equal(StrPtrLen("Basic") ) )
        {
            (void) ParseAuthNameAndPassword(&authString,namePtr, passwordPtr);
            if (namePtr->Len == 0) 
                continue;

            hasAuthentication = true;
            break;
        }
        else if (authType.Equal(sAuthRef) )
        {
            namePtr->Set(NULL,0);
            passwordPtr->Set(authString.Ptr, authString.Len);
            hasAuthentication = true;
            break;
        } 
    };
    
    return hasAuthentication;
}

Bool16  Authenticate(QTSS_RTSPRequestObject request, StrPtrLen* namePtr, StrPtrLen* passwordPtr)
{
    Bool16 authenticated = true;
    
    char* authName = namePtr->GetAsCString();
    OSCharArrayDeleter authNameDeleter(authName);
    
    QTSS_ActionFlags authAction = qtssActionFlagsAdmin;
    
    // authenticate callback to retrieve the password 
    QTSS_Error err = QTSS_Authenticate(authName, sAuthResourceLocalPath, sAuthResourceLocalPath, authAction, qtssAuthBasic, request);
    if (err != QTSS_NoErr) {
         return false; // Couldn't even call QTSS_Authenticate...abandon!
    }
    
    // Get the user profile object from the request object that was created in the authenticate callback
    QTSS_UserProfileObject theUserProfile = NULL;
    UInt32 len = sizeof(QTSS_UserProfileObject);
    err = QTSS_GetValue(request, qtssRTSPReqUserProfile, 0, (void*)&theUserProfile, &len);
    Assert(len == sizeof(QTSS_UserProfileObject));
    if (err != QTSS_NoErr)
        authenticated = false;

    if(err == QTSS_NoErr) {
        char* reqPassword = passwordPtr->GetAsCString();
        OSCharArrayDeleter reqPasswordDeleter(reqPassword);
        char* userPassword = NULL;  
        (void) QTSS_GetValueAsString(theUserProfile, qtssUserPassword, 0, &userPassword);
        OSCharArrayDeleter userPasswordDeleter(userPassword);
    
        if(userPassword == NULL) {
            authenticated = false;
        }
        else {
#ifdef __Win32__
            // The password is md5 encoded for win32
            char md5EncodeResult[120];
            MD5Encode(reqPassword, userPassword, md5EncodeResult, sizeof(md5EncodeResult));
            if(::strcmp(userPassword, md5EncodeResult) != 0)
                authenticated = false;
#else
            if(::strcmp(userPassword, (char*)crypt(reqPassword, userPassword)) != 0)
                authenticated = false;
#endif
        }
    }
    
    char* realm = NULL;
    Bool16 allowed = true;
    //authorize callback to check authorization
    // allocates memory for realm
	err = QTSS_Authorize(request, &realm, &allowed);
	// QTSS_Authorize allocates memory for the realm string
	// we don't use the realm returned by the callback, but instead 
	// use our own.
	// delete the memory allocated for realm because we don't need it!
	OSCharArrayDeleter realmDeleter(realm);
    
    if(err != QTSS_NoErr) {
        qtss_printf("QTSSAdminModule::Authenticate: QTSS_Authorize failed\n");
        return false; // Couldn't even call QTSS_Authorize...abandon!
    }
    
    if(authenticated && allowed)
        return true;
        
    return false;
}


QTSS_Error AuthorizeAdminRequest(QTSS_RTSPRequestObject request)
{
    Bool16 allowed = false;
    
    // get the resource path
    // if the path does not match the admin path, don't handle the request
    char* resourcePath = QTSSModuleUtils::GetLocalPath_Copy(request);
    OSCharArrayDeleter resourcePathDeleter(resourcePath);
    
    if(strcmp(sAuthResourceLocalPath, resourcePath) != 0)
        return QTSS_NoErr;
    
    // get the type of request
    QTSS_ActionFlags action = QTSSModuleUtils::GetRequestActions(request);
    if(!(action & qtssActionFlagsAdmin))
        return QTSS_RequestFailed;
       
    QTSS_UserProfileObject theUserProfile = QTSSModuleUtils::GetUserProfileObject(request);
    if (NULL == theUserProfile)
        return QTSS_RequestFailed;
           
    (void) QTSS_SetValue(request,qtssRTSPReqURLRealm, 0, sAuthRealm, ::strlen(sAuthRealm));
    
    // Authorize the user if the user belongs to the AdministratorGroup (this is an admin module pref)
    UInt32 numGroups = 0;
    char** groupsArray = QTSSModuleUtils::GetGroupsArray_Copy(theUserProfile, &numGroups);
    
    if ((groupsArray != NULL) && (numGroups != 0))
    {
        UInt32 index = 0;
        for (index = 0; index < numGroups; index++)
        {
            if(strcmp(sAdministratorGroup, groupsArray[index]) == 0)
            {
                allowed = true;
                break;
            }
        }
    
        // delete the memory allocated in QTSSModuleUtils::GetGroupsArray_Copy call 
        delete [] groupsArray;
    }
    
    if(!allowed)
    {
        if (QTSS_NoErr != QTSS_SetValue(request,qtssRTSPReqUserAllowed, 0, &allowed, sizeof(allowed)))
            return QTSS_RequestFailed; // Bail on the request. The Server will handle the error
    }
    
    return QTSS_NoErr;
}


Bool16 AcceptSession(QTSS_RTSPSessionObject inRTSPSession)
{   
    char remoteAddress[20] = {0};
    StrPtrLen theClientIPAddressStr(remoteAddress,sizeof(remoteAddress));
    QTSS_Error err = QTSS_GetValue(inRTSPSession, qtssRTSPSesRemoteAddrStr, 0, (void*)theClientIPAddressStr.Ptr, &theClientIPAddressStr.Len);
    if (err != QTSS_NoErr) return false;
    
    return AcceptAddress(&theClientIPAddressStr);    
}

Bool16 StillFlushing(QTSS_Filter_Params* inParams,Bool16 flushing)
{   

    QTSS_Error err = QTSS_NoErr;
    if (flushing) 
    {   
        err = QTSS_Flush(inParams->inRTSPRequest);
        //qtss_printf("Flushing session=%"_U32BITARG_" QTSS_Flush err =%"_S32BITARG_"\n",sSessID,err); 
    }
    if (err == QTSS_WouldBlock) // more to flush later
    {   
        sFlushing = true;
        (void) QTSS_SetValue(inParams->inRTSPRequest, sFlushingID, 0, (void*)&sFlushing, sFlushingLen);
        err = QTSS_RequestEvent(inParams->inRTSPRequest, QTSS_WriteableEvent);
        KeepSession(inParams->inRTSPRequest,true);
        //qtss_printf("Flushing session=%"_U32BITARG_" QTSS_RequestEvent err =%"_S32BITARG_"\n",sSessID,err);
    }
    else 
    {
        sFlushing = false;
        (void) QTSS_SetValue(inParams->inRTSPRequest, sFlushingID, 0, (void*)&sFlushing, sFlushingLen);
        KeepSession(inParams->inRTSPRequest,false);
    
        if (flushing) // we were flushing so reset the LastRequestTime
        {   
            sLastRequestTime = QTSS_Milliseconds();
            //qtss_printf("Done Flushing session=%"_U32BITARG_"\n",sSessID);
            return true;
        }
    }
    
    return sFlushing;
}

Bool16 IsAuthentic(QTSS_Filter_Params* inParams,StringParser *fullRequestPtr)
{
    Bool16 isAuthentic = false;

    if (!sAuthenticationEnabled) // no authentication
    {
        isAuthentic = true;
    }
    else // must authenticate
    {
        StrPtrLen theClientIPAddressStr;
        (void) QTSS_GetValuePtr(inParams->inRTSPSession, qtssRTSPSesRemoteAddrStr, 0, (void**)&theClientIPAddressStr.Ptr, &theClientIPAddressStr.Len);
        Bool16 isLocal =  IPComponentStr(&theClientIPAddressStr).IsLocal(); 
        
        StrPtrLen authenticateName;
        StrPtrLen authenticatePassword;
        StrPtrLen authType;
        Bool16 hasAuthentication = HasAuthentication(fullRequestPtr,&authenticateName,&authenticatePassword, &authType);               
        if (hasAuthentication) 
        {
            if (authType.Equal(sAuthRef))
            {    
                if (isLocal)
                    isAuthentic = OSXAuthenticate(&authenticatePassword);
            }
            else
                isAuthentic = Authenticate(inParams->inRTSPRequest, &authenticateName,&authenticatePassword);
       }
    }
//    if (isAuthentic)
//        isAuthentic = AuthorizeAdminRequest(inParams->inRTSPRequest);
    (void) QTSS_SetValue(inParams->inRTSPRequest, sAuthenticatedID, 0, (void*)&isAuthentic, sizeof(isAuthentic));

    return isAuthentic;
}

inline Bool16 InWaitInterval(QTSS_Filter_Params* inParams)
{
    QTSS_TimeVal nextExecuteTime = sLastRequestTime + sRequestTimeIntervalMilli;
    QTSS_TimeVal currentTime = QTSS_Milliseconds();
    SInt32 waitTime = 0;
    if (currentTime < nextExecuteTime)
    {   
        waitTime = (SInt32) (nextExecuteTime - currentTime) + 1;
        //qtss_printf("(currentTime < nextExecuteTime) sSessID = %"_U32BITARG_" waitTime =%"_S32BITARG_" currentTime = %qd nextExecute = %qd interval=%"_U32BITARG_"\n",sSessID, waitTime, currentTime, nextExecuteTime,sRequestTimeIntervalMilli);
        (void)QTSS_SetIdleTimer(waitTime);
        KeepSession(inParams->inRTSPRequest,true);
        
        //qtss_printf("-- call me again after %"_S32BITARG_" millisecs session=%"_U32BITARG_" \n",waitTime,sSessID);
        return true;
    }
    sLastRequestTime = QTSS_Milliseconds();
    //qtss_printf("handle sessID=%"_U32BITARG_" time=%qd \n",sSessID,currentTime);
    return false;
}

inline void GetQueryData(QTSS_RTSPRequestObject theRequest)
{
    sAdminPtr = NEW AdminClass();
    Assert(sAdminPtr != NULL);
    if (sAdminPtr == NULL) 
    {   //qtss_printf ("NEW AdminClass() failed!! \n");
        return;
    }
    if (sAdminPtr != NULL) 
    {
        sAdminPtr->Initialize(&sQTSSparams,sQueryPtr);  // Get theData
    }
}

inline void SendHeader(QTSS_StreamRef inStream)
{
    (void)QTSS_Write(inStream, sResponseHeader, ::strlen(sResponseHeader), NULL, 0);
    (void)QTSS_Write(inStream, sEOL, ::strlen(sEOL), NULL, 0);              
    (void)QTSS_Write(inStream, sVersionHeader, ::strlen(sVersionHeader), NULL, 0);      
    (void)QTSS_Write(inStream, sEOL, ::strlen(sEOL), NULL, 0);              
    (void)QTSS_Write(inStream, sConnectionHeader, ::strlen(sConnectionHeader), NULL, 0);        
    (void)QTSS_Write(inStream, sEOL, ::strlen(sEOL), NULL, 0);      
    (void)QTSS_Write(inStream, sContentType, ::strlen(sContentType), NULL, 0);      
    (void)QTSS_Write(inStream, sEOM, ::strlen(sEOM), NULL, 0);      
}

inline void SendResult(QTSS_StreamRef inStream)
{
    SendHeader(inStream);       
    if (sAdminPtr != NULL)
        sAdminPtr->RespondToQuery(inStream,sQueryPtr,sQueryPtr->GetRootID());
        
}

inline Bool16 GetRequestAuthenticatedState(QTSS_Filter_Params* inParams) 
{
    Bool16 result = false;
    UInt32 paramLen = sizeof(result);
    QTSS_Error err = QTSS_GetValue(inParams->inRTSPRequest, sAuthenticatedID, 0, (void*)&result, &paramLen);
    if(err != QTSS_NoErr)
    {
           paramLen = sizeof(result);
           result = false;
           err =QTSS_SetValue(inParams->inRTSPRequest, sAuthenticatedID, 0, (void*)&result, paramLen);
    }     
    return result;
}

inline Bool16 GetRequestFlushState(QTSS_Filter_Params* inParams)
{   Bool16 result = false;
    UInt32 paramLen = sizeof(result);
    QTSS_Error err = QTSS_GetValue(inParams->inRTSPRequest, sFlushingID, 0, (void*)&result, &paramLen);
    if (err != QTSS_NoErr)
    {   paramLen = sizeof(result);
        result = false;
        //qtss_printf("no flush val so set to false session=%"_U32BITARG_" err =%"_S32BITARG_"\n",sSessID, err);
        err =QTSS_SetValue(inParams->inRTSPRequest, sFlushingID, 0, (void*)&result, paramLen);
        //qtss_printf("QTSS_SetValue flush session=%"_U32BITARG_" err =%"_S32BITARG_"\n",sSessID, err);
    }
    return result;
}

QTSS_Error FilterRequest(QTSS_Filter_Params* inParams)
{
    if (NULL == inParams || NULL == inParams->inRTSPSession || NULL == inParams->inRTSPRequest)
    {   Assert(0);
        return QTSS_NoErr;
    }

    OSMutexLocker locker(sAdminMutex);
    //check to see if we should handle this request. Invokation is triggered
    //by a "GET /" request
    
    QTSS_Error err = QTSS_NoErr;
    QTSS_RTSPRequestObject theRequest = inParams->inRTSPRequest;

    UInt32 paramLen = sizeof(sSessID);
    err = QTSS_GetValue(inParams->inRTSPSession, qtssRTSPSesID, 0, (void*)&sSessID, &paramLen);     
    if (err != QTSS_NoErr) 
        return QTSS_NoErr;

    StrPtrLen theFullRequest;
    err = QTSS_GetValuePtr(theRequest, qtssRTSPReqFullRequest, 0, (void**)&theFullRequest.Ptr, &theFullRequest.Len);
    if (err != QTSS_NoErr) 
        return QTSS_NoErr;
        
    
    StringParser fullRequest(&theFullRequest);
        
    if ( !IsAdminRequest(&fullRequest) ) 
        return QTSS_NoErr;
        
    if ( !AcceptSession(inParams->inRTSPSession) )
    {   (void)QTSS_Write(inParams->inRTSPRequest, sPermissionDeniedHeader, ::strlen(sPermissionDeniedHeader), NULL, 0);     
        (void)QTSS_Write(inParams->inRTSPRequest, sHTMLBody, ::strlen(sHTMLBody), NULL, 0);
        KeepSession(theRequest,false);
        return QTSS_NoErr;
    }
    
    if(!GetRequestAuthenticatedState(inParams)) // must authenticate before handling
    {
        if(QTSS_IsGlobalLocked()) // must NOT be global locked
            return QTSS_RequestFailed;
            
        if (!IsAuthentic(inParams,&fullRequest)) 
        {  
            (void)QTSS_Write(inParams->inRTSPRequest, sUnauthorizedResponseHeader, ::strlen(sUnauthorizedResponseHeader), NULL, 0);     
            (void)QTSS_Write(inParams->inRTSPRequest, sHTMLBody, ::strlen(sHTMLBody), NULL, 0);
            KeepSession(theRequest,false);
            return QTSS_NoErr;
        }
        
    }
    
    if (GetRequestFlushState(inParams)) 
    {   StillFlushing(inParams,true);
        return QTSS_NoErr;
    }
        
    if (!QTSS_IsGlobalLocked())
    {       
        if (InWaitInterval(inParams)) 
            return QTSS_NoErr; 

        //qtss_printf("New Request Wait for GlobalLock session=%"_U32BITARG_"\n",sSessID);
        (void)QTSS_RequestGlobalLock();
        KeepSession(theRequest,true);
        return QTSS_NoErr; 
    }
    
    //qtss_printf("Handle request session=%"_U32BITARG_"\n",sSessID);
    APITests_DEBUG();
    
    if (sQueryPtr != NULL) 
    {   delete sQueryPtr;
        sQueryPtr = NULL;   
    }
    sQueryPtr = NEW QueryURI(&theFullRequest);
    if (sQueryPtr == NULL) return QTSS_NoErr;
    
    ShowQuery_DEBUG();
    
    if (sAdminPtr != NULL) 
    {   delete sAdminPtr;
        sAdminPtr = NULL;
    }
    UInt32 result = sQueryPtr->EvalQuery(NULL, NULL);
    if (result == 0) do
    {
        if( ElementNode_CountPtrs() > 0)
        {   ElementNode_ShowPtrs();
            Assert(0);
        }
            
        GetQueryData(theRequest);
        
        SendResult(theRequest); 
        delete sAdminPtr;
        sAdminPtr = NULL;
        
        if (sQueryPtr && !sQueryPtr->QueryHasReponse())
        {   UInt32 err = 404;
            (void) sQueryPtr->EvalQuery(&err,NULL);
            ReportErr(inParams, err);
            break;
        }

        if (sQueryPtr && sQueryPtr->QueryHasReponse())
        {   ReportErr(inParams, sQueryPtr->GetEvaluResult());
        }
        
        if (sQueryPtr->fIsPref && sQueryPtr->GetEvaluResult() == 0)
        {   QTSS_ServiceID id;
            (void) QTSS_IDForService(QTSS_REREAD_PREFS_SERVICE, &id);           
            (void) QTSS_DoService(id, NULL);
        }
    } while(false);
    else
    {
        SendHeader(theRequest);         
        ReportErr(inParams, sQueryPtr->GetEvaluResult());
    }
    
    if (sQueryPtr != NULL) 
    {   delete sQueryPtr;
        sQueryPtr = NULL;
    }
    
    (void) StillFlushing(inParams,true);
    return QTSS_NoErr;

}

//*****************************************

Bool16 EasyAdmin_UserAuthentication(const char* inUserName, const char* inPassword)
{
	qtss_printf("User:%s Password:%s Authenticated!\n", inUserName, inPassword);
	return true;
}

UInt16 EasyAdmin_GetRTSPort()
{
	UInt16 port;
    UInt32 len = sizeof(UInt16);
    (void) QTSS_GetValue(sServerPrefs, qtssPrefsRTSPPorts, 0, (void*)&port, &len);  
    
    return port;
}

void EasyAdmin_SetRTSPort(UInt16 uPort)
{
	//(void) QTSS_RemoveValue(sServerPrefs, qtssPrefsRTSPPorts, 0);
	(void) QTSS_SetValue(sServerPrefs, qtssPrefsRTSPPorts, 0, &uPort, sizeof(uPort));
}

void EasyAdmin_Restart()
{
	exit(-2);
}

UInt16 EasyAdmin_GetHTTPServicePort()
{
	UInt16 port;
    UInt32 len = sizeof(UInt16);
    (void) QTSS_GetValue(sServerPrefs, easyPrefsHTTPServicePort, 0, (void*)&port, &len);  
    
    return port;
}

void EasyAdmin_SetHTTPServicePort(UInt16 uPort)
{
	(void) QTSS_SetValue(sServerPrefs, easyPrefsHTTPServicePort, 0, &uPort, sizeof(uPort));
}

char* EasyAdmin_GetMoviesFolder()
{
    char* movieFolderString = NULL;
    (void) QTSS_GetValueAsString (sServerPrefs, qtssPrefsMovieFolder, 0, &movieFolderString);
    
	return movieFolderString;
}

void EasyAdmin_SetMoviesFolder(char* folder)
{
	(void) QTSS_SetValue(sServerPrefs, qtssPrefsMovieFolder, 0, (void*)folder, strlen(folder));
}

char* EasyAdmin_GetErrorLogFolder()
{
    char* logFolderString = NULL;
    (void) QTSS_GetValueAsString (sServerPrefs, qtssPrefsErrorLogDir, 0, &logFolderString);
    
	return logFolderString;
}

void EasyAdmin_SetErrorLogFolder(char* folder)
{
	(void) QTSS_SetValue(sServerPrefs, qtssPrefsErrorLogDir, 0, (void*)folder, strlen(folder));
}

UInt32 EasyAdmin_GetReflectBufferSecs()
{
	if(sReflectorPrefs == NULL)
		return 0;

	UInt32 reflectBufferSecs = 0;
	QTSSModuleUtils::GetAttribute(sReflectorPrefs, "reflector_buffer_size_sec", qtssAttrDataTypeUInt32, &reflectBufferSecs, NULL, sizeof(reflectBufferSecs));
	return reflectBufferSecs;
}

void EasyAdmin_SetReflectBufferSecs(UInt32 secs)
{
	if(sReflectorPrefs == NULL)
		return ;
	QTSSModuleUtils::CreateAttribute(sReflectorPrefs, "reflector_buffer_size_sec", qtssAttrDataTypeUInt32, &secs, sizeof(UInt32));
}

UInt16 EasyAdmin_GetMongoosePort()
{
	return sHttpPort;
}

void EasyAdmin_SetMongoosePort(UInt16 port)
{
	QTSSModuleUtils::CreateAttribute(sAdminPrefs, "http_port", qtssAttrDataTypeUInt16, &port, sizeof(UInt16));
}

char* EasyAdmin_GetServerHeader()
{
	char* serverHeader = NULL;
    (void) QTSS_GetValueAsString (sServer, qtssSvrRTSPServerHeader, 0, &serverHeader);
    
	return serverHeader;
}

SInt64 EasyAdmin_GetServiceRunTime()
{
	SInt64 timeNow = OS::Milliseconds();

	SInt64 startupTime = 0;
	UInt32 startupTimeSize = sizeof(startupTime);
	(void)QTSS_GetValue(sServer, qtssSvrStartupTime, 0, &startupTime, &startupTimeSize);

	return (timeNow - startupTime);
}

bool EasyAdmin_GetReflectHLSOutput()
{
	if(sReflectorPrefs == NULL)
		return false;

	Bool16 hlsOutputEnabled = false;
	QTSSModuleUtils::GetAttribute(sReflectorPrefs, "hls_output_enabled", qtssAttrDataTypeBool16, &hlsOutputEnabled, NULL, sizeof(hlsOutputEnabled));
	return hlsOutputEnabled;
}

void EasyAdmin_SetReflectHLSOutput(Bool16 hlsOutput)
{
	QTSSModuleUtils::CreateAttribute(sReflectorPrefs, "hls_output_enabled", qtssAttrDataTypeBool16, &hlsOutput, sizeof(Bool16));
}

char* EasyAdmin_GetHlsHttpRoot()
{
	if(sHLSModulePrefs == NULL)
		return NULL;

	char* hlsHTTPRoot = NULL;
	hlsHTTPRoot = QTSSModuleUtils::GetStringAttribute(sHLSModulePrefs, "HTTP_ROOT_DIR", NULL);

	return hlsHTTPRoot;
}

void EasyAdmin_SetHlsHttpRoot(char* httpRoot)
{
	QTSSModuleUtils::CreateAttribute(sHLSModulePrefs, "HTTP_ROOT_DIR", qtssAttrDataTypeCharArray, httpRoot, strlen(httpRoot));
}

UInt32 EasyAdmin_GetHlsTsDuration()
{
	if(sHLSModulePrefs == NULL)
		return 0;

	UInt32 hlsDurationSec = 0;
	QTSSModuleUtils::GetAttribute(sHLSModulePrefs, "TARGET_DURATION", qtssAttrDataTypeUInt32, &hlsDurationSec, NULL, sizeof(hlsDurationSec));
	return hlsDurationSec;
}

void EasyAdmin_SetHlsTsDuration(UInt32 secs)
{
	if(sHLSModulePrefs == NULL)
		return ;
	QTSSModuleUtils::CreateAttribute(sHLSModulePrefs, "TARGET_DURATION", qtssAttrDataTypeUInt32, &secs, sizeof(UInt32));
}

UInt32 EasyAdmin_GetHlsTsCapacity()
{
	if(sHLSModulePrefs == NULL)
		return 0;

	UInt32 hlsCapacity = 0;
	QTSSModuleUtils::GetAttribute(sHLSModulePrefs, "PLAYLIST_CAPACITY", qtssAttrDataTypeUInt32, &hlsCapacity, NULL, sizeof(hlsCapacity));
	return hlsCapacity;
}

void EasyAdmin_SetHlsTsCapacity(UInt32 uCapacity)
{
	if(sHLSModulePrefs == NULL)
		return ;
	QTSSModuleUtils::CreateAttribute(sHLSModulePrefs, "PLAYLIST_CAPACITY", qtssAttrDataTypeUInt32, &uCapacity, sizeof(UInt32));
}

bool EasyAdmin_StartHLSession(char* inSessionName, const char* inRTSPUrl, UInt32 inTimeout)
{
	Easy_StartHLSession(inSessionName, inRTSPUrl, inTimeout, NULL);
	return true;
}

bool EasyAdmin_StopHLSession(char* inSessionName)
{
	Easy_StopHLSession(inSessionName);
	return true;
}

char* EasyAdmin_GetHLSessions()
{
	return (char*)Easy_GetHLSessions();
}
char* EasyAdmin_GetRTSPSessions()
{
	return (char*)Easy_GetRTSPPushSessions();
}