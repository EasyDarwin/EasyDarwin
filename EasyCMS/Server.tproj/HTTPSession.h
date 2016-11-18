/*
	Copyleft (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
/*!
  \file    HTTPSession.h
  \author  Babosa@EasyDarwin.org
  \date    2014-12-03
  \version 1.0
  \mainpage 使用引导

  网络调用主要流程\n
  Select -> ServiceSession -> DispatchMsgCenter -> ServiceSession -> Cleanup\n\n

  Copyright (c) 2014 EasyDarwin.org 版权所有\n

  \defgroup 服务单元网络事件处理流程
*/

#ifndef __HTTP_SESSION_H__
#define __HTTP_SESSION_H__

#include "HTTPSessionInterface.h"
#include "HTTPRequest.h"
#include "TimeoutTask.h"
#include "QTSSModule.h"
#include "DecoderHelper.h"
#include "OSQueue.h"

using namespace std;

typedef struct __DECODE_PARAM_T
{
	int				codec;
	int				width;
	int				height;
	int				gopTally;
	char*			imageData;
	unsigned int	imageSize;

} DECODE_PARAM_T;

class HTTPSession : public HTTPSessionInterface
{
public:
	HTTPSession();
	virtual ~HTTPSession();

	QTSS_Error SendHTTPPacket(StrPtrLen* contentXML, bool connectionClose, bool decrement);

	//char* GetDeviceSnap() { return fDeviceSnap; };
	char* GetDeviceSerial() { return (char*)fDevice.serial_.c_str(); };

	string GetTalkbackSession() const { return talkbackSession; }
	void SetTalkbackSession(const string& session) { talkbackSession = session; }


	/*void SetStreamPushInfo(EasyJsonValue &info) { fStreamPushInfo = info; }
	EasyJsonValue &GetStreamPushInfo() { return fStreamPushInfo; }*/

private:
	SInt64 Run();

	// Does request prep & request cleanup, respectively
	QTSS_Error setupRequest();
	void cleanupRequest();
	bool isRightChannel(const char* channel);

	QTSS_Error processRequest();//处理请求，单独放到一个状态中去处理，这样方便重复执行
	QTSS_Error execNetMsgErrorReqHandler(HTTPStatusCode errCode);//消息默认处理函数
	QTSS_Error execNetMsgDSRegisterReq(const char* json);//设备注册请求
	QTSS_Error execNetMsgDSPushStreamAck(const char* json);//设备的开始流回应
	QTSS_Error execNetMsgCSFreeStreamReq(const char *json);//客户端的停止直播请求
	QTSS_Error execNetMsgDSStreamStopAck(const char* json);//设备的停止推流回应
	QTSS_Error execNetMsgDSPostSnapReq(const char* json);//设备的快照更新请求
    QTSS_Error execNetMsgDSPTZControlAck(const char* json);
	QTSS_Error execNetMsgDSPresetControlAck(const char* json);

	QTSS_Error execNetMsgCSTalkbackControlReq(const char* json);
	QTSS_Error execNetMSGDSTalkbackControlAck(const char* json);

	QTSS_Error execNetMsgCSDeviceListReq(const char* json);//客户端获得设备列表，json接口
	QTSS_Error execNetMsgCSCameraListReq(const char* json);//客户端获得摄像头列表，json接口,仅对设备类型为NVR时有效

	QTSS_Error execNetMsgCSGetStreamReqRESTful(const char* queryString);//客户端拉流请求，Restful接口
    QTSS_Error execNetMsgCSFreeStreamReqRESTful(const char* queryString);//客户端停止直播请求，RESTful接口
	QTSS_Error execNetMsgCSGetDeviceListReqRESTful(const char* queryString);//客户端获得设备列表,restful接口
	QTSS_Error execNetMsgCSGetCameraListReqRESTful(const char* queryString);//客户端获得摄像头列表，restful接口，仅对设备类型为NVR时有效
    QTSS_Error execNetMsgCSPTZControlReqRESTful(const char* queryString);
	QTSS_Error execNetMsgCSPresetControlReqRESTful(const char* queryString);

	QTSS_Error dumpRequestData();//清空请求报文

	// test current connections handled by this object against server pref connection limit
	static bool overMaxConnections(UInt32 buffer);

	QTSS_Error rawData2Image(char* rawBuf, int bufSize, int codec, int width, int height);
	static int	yuv2BMPImage(unsigned int width, unsigned int height, char* yuvpbuf, unsigned int* rgbsize, unsigned char* rgbdata);

	HTTPRequest*        fRequest;
	OSMutex             fReadMutex;
	OSMutex				fSendMutex;

	enum
	{
		kReadingRequest = 0,		//读取报文
		kFilteringRequest = 1,		//过滤报文
		kPreprocessingRequest = 2,	//预处理报文
		kProcessingRequest = 3,		//处理报文
		kSendingResponse = 4,		//发送响应报文
		kCleaningUp = 5,			//清空本次处理的报文内容

		kReadingFirstRequest = 6,	//第一次读取Session报文，主要用来做Session协议区分（HTTP/TCP/RTSP等等）
		kHaveCompleteMessage = 7	// 读取到完整的报文
	};

	//UInt32 fCurrentModule;
	UInt32 fState;

	//QTSS_RoleParams     fRoleParams;//module param blocks for roles.
	QTSS_ModuleState    fModuleState;

	//char* fDeviceSnap;
	//EasyJsonValue fStreamPushInfo;

	// Channel Snap
	DECODE_PARAM_T		decodeParam;
	
	DecoderHelper		decoderHelper;

	string				talkbackSession;
};
#endif // __HTTP_SESSION_H__

