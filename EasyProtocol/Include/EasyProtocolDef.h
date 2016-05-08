/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/

/*!
\defgroup EasyDarwin_Terminal_Type_Define
\defgroup EasyDarwin_Device_Type_Define
\defgroup EasyDarwin_Tag_Define
\defgroup EasyDarwin_Attruibute_Define
\defgroup EasyDarwin_Message_Type_Define
\defgroup EasyDarwin_Error_Define
\defgroup EasyDarwin_Permission_Define
\defgroup EasyDarwin_Time_Format_Define
\defgroup EasyDarwin_Device_Status_Define
\defgroup EasyDarwin_Protocol_Type_Define
\defgroup EasyDarwin_Live_Type_Define
\defgroup EasyProtocolBase
\defgroup MSG_DEV_CMS_REGISTER_REQ
\defgroup MSG_DEV_CMS_REGISTER_RSP
\defgroup MSG_CMS_DEV_STREAM_START_REQ
\defgroup MSG_CMS_DEV_STREAM_START_RSP
\defgroup MSG_CMS_DEV_STREAM_STOP_REQ
\defgroup MSG_CMS_DEV_STREAM_STOP_RSP
\defgroup MSG_CLI_CMS_DEVICE_LIST_REQ
\defgroup MSG_CLI_CMS_DEVICE_LIST_RSP
*/

#ifndef EASY_PROTOCOL_DEF_H
#define	EASY_PROTOCOL_DEF_H

#if (defined(_WIN32))
#ifndef _DLL_
#define EASYDARWIN_API
#else
#ifdef EASYDARWIN_API_EXPORTS
#define EASYDARWIN_API __declspec(dllexport) 
#else  
#define EASYDARWIN_API __declspec(dllimport)
#endif
#endif
#elif defined(__linux__)
#define EASYDARWIN_API
#endif
//#define EASYDARWIN_API
#define EASYDARWIN_PROTOCOL_VERSION                         "1.0"
#define EASYDARWIN_PROTOCOL_STREAM_MAIN						"0"
#define EASYDARWIN_PROTOCOL_STREAM_SUB						"1"

/*!
\ingroup EasyDarwin_Tag_Define
\{
*/
#define EASYDARWIN_TAG_ROOT									"EasyDarwin"
#define EASYDARWIN_TAG_HEADER								"Header"
#define EASYDARWIN_TAG_BODY									"Body"
#define EASYDARWIN_TAG_VERSION								"Version"
#define EASYDARWIN_TAG_TERMINAL_TYPE                        "TerminalType"
#define EASYDARWIN_TAG_SESSION_ID							"SessionID"
#define EASYDARWIN_TAG_MESSAGE_TYPE							"MessageType"
#define EASYDARWIN_TAG_CSEQ									"CSeq"
#define EASYDARWIN_TAG_ERROR_NUM							"ErrorNum"
#define EASYDARWIN_TAG_ERROR_STRING							"ErrorString"
#define EASYDARWIN_TAG_REDIRECT                             "Redirect"
#define EASYDARWIN_TAG_SERVER_ADDRESS                       "ServerAddress"
#define EASYDARWIN_TAG_SERVER_PORT                          "ServerPort"
#define EASYDARWIN_TAG_KEY                                  "Key"
#define EASYDARWIN_TAG_NAME									"Name"
#define EASYDARWIN_TAG_PASSWORD                             "Password"
#define EASYDARWIN_TAG_LAST_LOGIN_TIME						"LastLoginTime"
#define EASYDARWIN_TAG_LAST_LOGIN_ADDRESS					"LastLoginAddress"
#define EASYDARWIN_TAG_PERMISSION							"Permission"
#define EASYDARWIN_TAG_PAGE_NUM                             "PageNum"
#define EASYDARWIN_TAG_DEVICE                               "Device"
#define EASYDARWIN_TAG_DEVICE_TYPE							"DeviceType"
#define EASYDARWIN_TAG_DEVICE_SUM_PER_PAGE                  "DeviceSumPerPage"
#define EASYDARWIN_TAG_DEVICE_SUM                           "DeviceSum"
#define EASYDARWIN_TAG_DEVICE_LIST                          "DeviceList"
#define EASYDARWIN_TAG_DEVICE_SERIAL                        "SerialNumber"
#define EASYDARWIN_TAG_DEVICE_NAME                          "DeviceName"
#define EASYDARWIN_TAG_INVALID_ACCESS_DATE					"InvalidAccessDate"
#define EASYDARWIN_TAG_INVALID_ACCESS_TIME					"InvalidAccessTime"
#define EASYDARWIN_TAG_FROM                                 "From"
#define EASYDARWIN_TAG_TO                                   "To"
#define EASYDARWIN_TAG_STATUS                               "Status"
#define EASYDARWIN_TAG_SNAPSHOT                             "Snapshot"
#define EASYDARWIN_TAG_DESCRIPTION                          "Description"
#define EASYDARWIN_TAG_TOKEN                                "Token"
#define EASYDARWIN_TAG_CUSTOM_FIELD                         "CustomField"
#define EASYDARWIN_TAG_ORDER                                "Order"
#define EASYDARWIN_TAG_SERVICE_TYPE                         "ServiceType"
#define EASYDARWIN_TAG_SERVICE_UNIT                         "ServiceUnit"
#define EASYDARWIN_TAG_WAN_IP                               "WanIP"
#define EASYDARWIN_TAG_LAN_IP                               "LanIP"
#define EASYDARWIN_TAG_PORT                                 "Port"
#define EASYDARWIN_TAG_LOAD                                 "Load"
#define EASYDARWIN_TAG_USER									"User"
#define EASYDARWIN_TAG_USER_NAME							"UserName"
#define EASYDARWIN_TAG_PROTOCOL								"Protocol"
#define EASYDARWIN_TAG_AUDIO								"Audio"
#define EASYDARWIN_TAG_VIDEO								"Video"
#define EASYDARWIN_TAG_CMD									"Command"
#define EASYDARWIN_TAG_VALUE								"Value"
#define EASYDARWIN_TAG_CLIENT_SERIAL						"ClientSerial"
#define EASYDARWIN_TAG_LIVE_STREAM_ID						"LiveStreamID"
#define EASYDARWIN_TAG_CLIENT_SESSION						"ClientSession"
#define EASYDARWIN_TAG_IP									"IP"
#define EASYDARWIN_TAG_LIVE_TYPE							"LiveType"
#define EASYDARWIN_TAG_USER_PLAY_COUNT						"UserPlayCount"
#define EASYDARWIN_TAG_PROXY_PLAY_COUNT						"ProxyPlayCount"
#define EASYDARWIN_TAG_PLAY_STREAM_ID                       "PlayStreamID"
#define EASYDARWIN_TAG_DEVICE_SERVER						"DeviceServer"
#define EASYDARWIN_TAG_TIME									"Time"
#define EASYDARWIN_TAG_SCHEDULE								"Schedule" 
#define EASYDARWIN_TAG_STREAM_ID							"StreamID"
/*!
\}
*/


/*!
\ingroup EasyDarwin_Message_Type_Define
\{
*/
#define MSG_DS_REGISTER_REQ									0x0001//设备注册
#define MSG_SD_REGISTER_ACK									0xe001
#define MSG_SD_PUSH_STREAM_REQ								0x0002//CMS请求设备开始推流
#define MSG_DS_PUSH_STREAM_ACK								0xe002
#define MSG_SD_STREAM_STOP_REQ								0x0003//CMS请求设备停止推流
#define MSG_DS_STREAM_STOP_ACK								0xe003
#define MSG_CS_DEVICE_LIST_REQ								0x0004//客户端向CMS请求设备列表
#define MSG_SC_DEVICE_LIST_ACK								0xe004
#define MSG_CS_CAMERA_LIST_REQ								0x0005//客户端向CMS请求摄像机列表
#define MSG_SC_CAMERA_LIST_ACK								0xe005
#define MSG_CS_GET_STREAM_REQ								0x0006//客户端向CMS请求流
#define MSG_SC_GET_STREAM_ACK								0xe006
#define MSG_CS_FREE_STREAM_REQ								0x0007//客户端向CMS请求释放流
#define MSG_SC_FREE_STREAM_ACK								0xe007
#define MSG_DS_POST_SNAP_REQ								0x0008//设备向CMS上传快照
#define MSG_SD_POST_SNAP_ACK								0xe008

#define MSG_SC_LIST_RECORD_ACK								0xe009



//保留
#define MSG_CLI_SMS_HLS_ACK									0xeee0
#define MSG_CLI_SMS_HLS_LIST_ACK							0xeee1
#define MSG_CLI_SMS_PUSH_SESSION_LIST_ACK					0xeee2
//保留

#define MSG_SC_EXCEPTION									0xeeee

/*!
\}
*/

/*!
\ingroup EasyDarwin_Error_Define
\{
*/
#define EASYDARWIN_ERROR_SUCCESS_OK							200             ///< Success OK
#define EASYDARWIN_ERROR_SUCCESS_CREATED                   	201             ///< Success Created
#define EASYDARWIN_ERROR_SUCCESS_ACCEPTED                  	202             ///< Success Accepted
#define EASYDARWIN_ERROR_SUCCESS_NO_CONTENT                	204             ///< Success No Content
#define EASYDARWIN_ERROR_SUCCESS_PARTIAL_CONTENT           	206             ///< Success Partial Content
#define EASYDARWIN_ERROR_REDIRECT_PERMANENT_MOVED          	301             ///< Redirect Permanent Moved
#define EASYDARWIN_ERROR_REDIRECT_TEMP_MOVED               	302             ///< Redirect Temp Moved
#define EASYDARWIN_ERROR_REDIRECT_SEE_OTHER                	303             ///< Redirect See Other
#define EASYDARWIN_ERROR_USE_PROXY                         	305             ///< Use Proxy
#define EASYDARWIN_ERROR_CLIENT_BAD_REQUEST                	400             ///< Client Bad Request
#define EASYDARWIN_ERROR_CLIENT_UNAUTHORIZED               	401             ///< Client Unauthorized
#define EASYDARWIN_ERROR_PAYMENT_REQUIRED                  	402             ///< Payment Required
#define EASYDARWIN_ERROR_CLIENT_FORBIDDEN                  	403             ///< Client Forbidden
#define EASYDARWIN_ERROR_NOT_FOUND                         	404             ///< Not Found
#define EASYDARWIN_ERROR_METHOD_NOT_ALLOWED                	405             ///< Method Not Allowed
#define EASYDARWIN_ERROR_PROXY_AUTHENTICATION_REQUIRED     	407             ///< Proxy Authentication Required
#define EASYDARWIN_ERROR_REQUEST_TIMEOUT                   	408             ///< Request Timeout
#define EASYDARWIN_ERROR_CONFLICT                          	409             ///< Conflict
#define EASYDARWIN_ERROR_PRECONDITION_FAILED               	412             ///< Precondition Failed
#define EASYDARWIN_ERROR_UNSUPPORTED_MEDIA_TYPE            	415             ///< Unsupported Media Type
#define EASYDARWIN_ERROR_SERVER_INTERNAL_ERROR             	500             ///< Server Internal Error
#define EASYDARWIN_ERROR_SERVER_NOT_IMPLEMENTED            	501             ///< Server Not Implemented
#define EASYDARWIN_ERROR_SERVER_BAD_GATEWAY                	502             ///< Server Bad Gateway
#define EASYDARWIN_ERROR_SERVER_UNAVAILABLE                	503             ///< Server Unavailable
#define EASYDARWIN_ERROR_RTSP_VERSION_NOT_SUPPORTED        	505             ///< RTSP Version Not Supported
#define EASYDARWIN_ERROR_DEVICE_VERSION_TOO_OLD				15              ///< Device Version Too Old
#define EASYDARWIN_ERROR_DEVICE_FAILURE						16              ///< Device Failure
#define EASYDARWIN_ERROR_MEMCACHE_NOT_FOUND					600				///< Memcache Not Found
#define EASYDARWIN_ERROR_DATABASE_NOT_FOUND					601				///< Database Not Found
#define EASYDARWIN_ERROR_USER_NOT_FOUND						602				///< User Not Found
#define EASYDARWIN_ERROR_DEVICE_NOT_FOUND					603				///< Device Not Found
#define EASYDARWIN_ERROR_SESSION_NOT_FOUND					604				///< Session Not Found
#define EASYDARWIN_ERROR_SERVICE_NOT_FOUND					605				///< Service Not Found
#define EASYDARWIN_ERROR_PASSWORD_ERROR						620				///< Password Error
#define EASYDARWIN_ERROR_XML_PARSE_ERROR					621				///< XML Parse Error
#define EASYDARWIN_ERROR_PERMISSION_ERROR					622				///< Permission Error
#define EASYDARWIN_ERROR_LOCAL_SYSTEM_ERROR                 623             ///< Local System Error
#define EASYDARWIN_ERROR_PARAM_ERROR                        624             ///< Param Error

/*!
\}
*/

/*!
\ingroup EasyDarwin_Time_Format_Define
\{
*/
enum EasyDarwinTimeFormat
{
    EASYDARWIN_TIME_FORMAT_YYYYMMDDHHMMSS = 1,     ///< Format like 2014-08-31 08:15:30
    EASYDARWIN_TIME_FORMAT_YYYYMMDD,               ///< Format like 2014-08-31
    EASYDARWIN_TIME_FORMAT_HHMMSS                  ///< Format like 08:15:30    
};
/*!
\}
*/

/*!
\ingroup EasyDarwin_Device_Status_Define
\{
*/
enum EasyDarwinDeviceStatus
{
	EASYDARWIN_DEVICE_STATUS_OFFLINE = 0,		///< DEVICE_STATUS_OFFLINE
	EASYDARWIN_DEVICE_STATUS_ONLINE			///< DEVICE_STATUS_ONLINE 
};
/*!
\}
*/

/*!
\ingroup EasyDarwin_Protocol_Type_Define
\{
*/
enum EasyDarwinProtocolType
{
	EASYDARWIN_PROTOCOL_TYPE_RTSP = 1,			///< RTSP
	EASYDARWIN_PROTOCOL_TYPE_HLS				///< HLS
};
/*!
\}
*/

/*!
\ingroup EasyDarwin_Media_Encode_Type_Define
\{
*/
enum EasyDarwinMediaEncodeType
{
	EASYDARWIN_MEDIA_ENCODE_AUDIO_AAC = 1,			///< AAC
	EASYDARWIN_MEDIA_ENCODE_VIDEO_H264			///< H264
};
/*!
\}
*/

/*!
\ingroup EasyDarwin_Device_Type_Define
\{
*/
enum EasyDarwinTerminalType
{
	EASYDARWIN_TERMINAL_TYPE_CAMERA = 1					///< Camera	
};
/*!
\}
*/
    
#endif	/* EASY_PROTOCOL_DEF_H */