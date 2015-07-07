/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/

/*!
\defgroup EasyDSS_Terminal_Type_Define
\defgroup EasyDSS_Device_Type_Define
\defgroup EasyDSS_Tag_Define
\defgroup EasyDSS_Attruibute_Define
\defgroup EasyDSS_Message_Type_Define
\defgroup EasyDSS_Error_Define
\defgroup EasyDSS_Permission_Define
\defgroup EasyDSS_Time_Format_Define
\defgroup EasyDSS_Device_Status_Define
\defgroup EasyDSS_Protocol_Type_Define
\defgroup EasyDSS_Live_Type_Define
\defgroup EasyDSSProtocolBase
\defgroup MSG_DEV_CMS_REGISTER_REQ
\defgroup MSG_DEV_CMS_REGISTER_RSP
\defgroup MSG_CMS_DEV_STREAM_START_REQ
\defgroup MSG_CMS_DEV_STREAM_START_RSP
\defgroup MSG_CMS_DEV_STREAM_STOP_REQ
\defgroup MSG_CMS_DEV_STREAM_STOP_RSP
\defgroup MSG_NGX_CMS_NEED_STREAM_REQ
\defgroup MSG_NGX_CMS_NEED_STREAM_RSP
*/

#ifndef EASYDSS_PROTOCOL_DEF_H
#define	EASYDSS_PROTOCOL_DEF_H

#if (defined(_WIN32))
#ifndef _DLL_
#define EASYDSS_API
#else
#ifdef EASYDSS_API_EXPORTS
#define EASYDSS_API __declspec(dllexport) 
#else  
#define EASYDSS_API __declspec(dllimport)
#endif
#endif
#elif defined(__linux__)
#define EASYDSS_API
#endif
//#define EASYDSS_API
#define EASYDSS_PROTOCOL_VERSION                            "1.0"
#define EASYDSS_PROTOCOL_STREAM_MAIN						"0"
#define EASYDSS_PROTOCOL_STREAM_SUB							"1"

/*!
\ingroup EasyDSS_Tag_Define
\{
*/
#define EASYDSS_TAG_ROOT									"EasyDarwin"
#define EASYDSS_TAG_HEADER									"Header"
#define EASYDSS_TAG_BODY									"Body"
#define EASYDSS_TAG_VERSION									"Version"
#define EASYDSS_TAG_TERMINAL_TYPE                           "TerminalType"
#define EASYDSS_TAG_SESSION_ID								"SessionID"
#define EASYDSS_TAG_MESSAGE_TYPE							"MessageType"
#define EASYDSS_TAG_CSEQ									"CSeq"
#define EASYDSS_TAG_ERROR_NUM								"ErrorNum"
#define EASYDSS_TAG_ERROR_STRING							"ErrorString"
#define EASYDSS_TAG_REDIRECT                                "Redirect"
#define EASYDSS_TAG_SERVER_ADDRESS                          "ServerAddress"
#define EASYDSS_TAG_SERVER_PORT                             "ServerPort"
#define EASYDSS_TAG_KEY                                     "Key"
#define EASYDSS_TAG_NAME									"Name"
#define EASYDSS_TAG_PASSWORD                                "Password"
#define EASYDSS_TAG_LAST_LOGIN_TIME							"LastLoginTime"
#define EASYDSS_TAG_LAST_LOGIN_ADDRESS						"LastLoginAddress"
#define EASYDSS_TAG_PERMISSION								"Permission"
#define EASYDSS_TAG_PAGE_NUM                                "PageNum"
#define EASYDSS_TAG_DEVICE                                  "Device"
#define EASYDSS_TAG_DEVICE_TYPE								"DeviceType"
#define EASYDSS_TAG_DEVICE_SUM_PER_PAGE                     "DeviceSumPerPage"
#define EASYDSS_TAG_DEVICE_SUM                              "DeviceSum"
#define EASYDSS_TAG_DEVICE_LIST                             "DeviceList"
#define EASYDSS_TAG_DEVICE_SERIAL                           "SerialNumber"
#define EASYDSS_TAG_DEVICE_NAME                             "DeviceName"
#define EASYDSS_TAG_INVALID_ACCESS_DATE						"InvalidAccessDate"
#define EASYDSS_TAG_INVALID_ACCESS_TIME						"InvalidAccessTime"
#define EASYDSS_TAG_FROM                                    "From"
#define EASYDSS_TAG_TO                                      "To"
#define EASYDSS_TAG_STATUS                                  "Status"
#define EASYDSS_TAG_SNAPSHOT                                "Snapshot"
#define EASYDSS_TAG_DESCRIPTION                             "Description"
#define EASYDSS_TAG_TOKEN                                   "Token"
#define EASYDSS_TAG_CUSTOM_FIELD                            "CustomField"
#define EASYDSS_TAG_ORDER                                   "Order"
#define EASYDSS_TAG_SERVICE_TYPE                            "ServiceType"
#define EASYDSS_TAG_SERVICE_UNIT                            "ServiceUnit"
#define EASYDSS_TAG_WAN_IP                                  "WanIP"
#define EASYDSS_TAG_LAN_IP                                  "LanIP"
#define EASYDSS_TAG_PORT                                    "Port"
#define EASYDSS_TAG_LOAD                                    "Load"
#define EASYDSS_TAG_USER									"User"
#define EASYDSS_TAG_USER_NAME								"UserName"
#define EASYDSS_TAG_PROTOCOL								"Protocol"
#define EASYDSS_TAG_AUDIO									"Audio"
#define EASYDSS_TAG_VIDEO									"Video"
#define EASYDSS_TAG_CMD										"Command"
#define EASYDSS_TAG_VALUE									"Value"
#define EASYDSS_TAG_CLIENT_SERIAL							"ClientSerial"
#define EASYDSS_TAG_LIVE_STREAM_ID							"LiveStreamID"
#define EASYDSS_TAG_CLIENT_SESSION							"ClientSession"
#define EASYDSS_TAG_IP										"IP"
#define EASYDSS_TAG_LIVE_TYPE								"LiveType"
#define EASYDSS_TAG_USER_PLAY_COUNT							"UserPlayCount"
#define EASYDSS_TAG_PROXY_PLAY_COUNT						"ProxyPlayCount"
#define EASYDSS_TAG_PLAY_STREAM_ID                          "PlayStreamID"
#define EASYDSS_TAG_DEVICE_SERVER							"DeviceServer"
#define EASYDSS_TAG_TIME									"Time"
#define EASYDSS_TAG_SCHEDULE								"Schedule" 
#define EASYDSS_TAG_STREAM_ID								"StreamID"
/*!
\}
*/


/*!
\ingroup EasyDSS_Message_Type_Define
\{
*/
#define MSG_DEV_CMS_REGISTER_REQ							0x0001
#define MSG_DEV_CMS_REGISTER_RSP							0xe001
#define MSG_CMS_DEV_STREAM_START_REQ						0x0002
#define MSG_CMS_DEV_STREAM_START_RSP						0xe002
#define MSG_CMS_DEV_STREAM_STOP_REQ							0x0003
#define MSG_CMS_DEV_STREAM_STOP_RSP							0xe003
#define MSG_NGX_CMS_NEED_STREAM_REQ							0x0004
#define MSG_NGX_CMS_NEED_STREAM_RSP							0xe004
/*!
\}
*/

/*!
\ingroup EasyDSS_Error_Define
\{
*/
#define EASYDSS_ERROR_SUCCESS_OK							200             ///< Success OK
#define EASYDSS_ERROR_SUCCESS_CREATED                   	201             ///< Success Created
#define EASYDSS_ERROR_SUCCESS_ACCEPTED                  	202             ///< Success Accepted
#define EASYDSS_ERROR_SUCCESS_NO_CONTENT                	204             ///< Success No Content
#define EASYDSS_ERROR_SUCCESS_PARTIAL_CONTENT           	206             ///< Success Partial Content
#define EASYDSS_ERROR_REDIRECT_PERMANENT_MOVED          	301             ///< Redirect Permanent Moved
#define EASYDSS_ERROR_REDIRECT_TEMP_MOVED               	302             ///< Redirect Temp Moved
#define EASYDSS_ERROR_REDIRECT_SEE_OTHER                	303             ///< Redirect See Other
#define EASYDSS_ERROR_USE_PROXY                         	305             ///< Use Proxy
#define EASYDSS_ERROR_CLIENT_BAD_REQUEST                	400             ///< Client Bad Request
#define EASYDSS_ERROR_CLIENT_UNAUTHORIZED               	401             ///< Client Unauthorized
#define EASYDSS_ERROR_PAYMENT_REQUIRED                  	402             ///< Payment Required
#define EASYDSS_ERROR_CLIENT_FORBIDDEN                  	403             ///< Client Forbidden
#define EASYDSS_ERROR_NOT_FOUND                         	404             ///< Not Found
#define EASYDSS_ERROR_METHOD_NOT_ALLOWED                	405             ///< Method Not Allowed
#define EASYDSS_ERROR_PROXY_AUTHENTICATION_REQUIRED     	407             ///< Proxy Authentication Required
#define EASYDSS_ERROR_REQUEST_TIMEOUT                   	408             ///< Request Timeout
#define EASYDSS_ERROR_CONFLICT                          	409             ///< Conflict
#define EASYDSS_ERROR_PRECONDITION_FAILED               	412             ///< Precondition Failed
#define EASYDSS_ERROR_UNSUPPORTED_MEDIA_TYPE            	415             ///< Unsupported Media Type
#define EASYDSS_ERROR_SERVER_INTERNAL_ERROR             	500             ///< Server Internal Error
#define EASYDSS_ERROR_SERVER_NOT_IMPLEMENTED            	501             ///< Server Not Implemented
#define EASYDSS_ERROR_SERVER_BAD_GATEWAY                	502             ///< Server Bad Gateway
#define EASYDSS_ERROR_SERVER_UNAVAILABLE                	503             ///< Server Unavailable
#define EASYDSS_ERROR_RTSP_VERSION_NOT_SUPPORTED        	505             ///< RTSP Version Not Supported
#define EASYDSS_ERROR_DEVICE_VERSION_TOO_OLD				15              ///< Device Version Too Old
#define EASYDSS_ERROR_DEVICE_FAILURE						16              ///< Device Failure
#define EASYDSS_ERROR_MEMCACHE_NOT_FOUND					600				///< Memcache Not Found
#define EASYDSS_ERROR_DATABASE_NOT_FOUND					601				///< Database Not Found
#define EASYDSS_ERROR_USER_NOT_FOUND						602				///< User Not Found
#define EASYDSS_ERROR_DEVICE_NOT_FOUND						603				///< Device Not Found
#define EASYDSS_ERROR_SESSION_NOT_FOUND						604				///< Session Not Found
#define EASYDSS_ERROR_SERVICE_NOT_FOUND						605				///< Service Not Found
#define EASYDSS_ERROR_PASSWORD_ERROR						620				///< Password Error
#define EASYDSS_ERROR_XML_PARSE_ERROR						621				///< XML Parse Error
#define EASYDSS_ERROR_PERMISSION_ERROR						622				///< Permission Error
#define EASYDSS_ERROR_LOCAL_SYSTEM_ERROR                    623             ///< Local System Error
#define EASYDSS_ERROR_PARAM_ERROR                           624             ///< Param Error

/*!
\}
*/

/*!
\ingroup EasyDSS_Time_Format_Define
\{
*/
enum EasyDSSTimeFormat
{
    EASYDSS_TIME_FORMAT_YYYYMMDDHHMMSS = 1,     ///< Format like 2014-08-31 08:15:30
    EASYDSS_TIME_FORMAT_YYYYMMDD,               ///< Format like 2014-08-31
    EASYDSS_TIME_FORMAT_HHMMSS                  ///< Format like 08:15:30    
};
/*!
\}
*/

/*!
\ingroup EasyDSS_Device_Status_Define
\{
*/
enum EasyDSSDeviceStatus
{
	EASYDSS_DEVICE_STATUS_OFFLINE = 0,		///< DEVICE_STATUS_OFFLINE
	EASYDSS_DEVICE_STATUS_ONLINE			///< DEVICE_STATUS_ONLINE 
};
/*!
\}
*/

/*!
\ingroup EasyDSS_Protocol_Type_Define
\{
*/
enum EasyDSSProtocolType
{
	EASYDSS_PROTOCOL_TYPE_RTSP = 1,			///< RTSP
	EASYDSS_PROTOCOL_TYPE_HLS				///< HLS
};
/*!
\}
*/

/*!
\ingroup EasyDSS_Media_Encode_Type_Define
\{
*/
enum EasyDSSMediaEncodeType
{
	EASYDSS_MEDIA_ENCODE_AUDIO_AAC = 1,			///< AAC
	EASYDSS_MEDIA_ENCODE_VIDEO_H264			///< H264
};
/*!
\}
*/

/*!
\ingroup EasyDSS_Device_Type_Define
\{
*/
enum EasyDSSTerminalType
{
	EASYDSS_TERMINAL_TYPE_CAMERA = 1					///< Camera	
};
/*!
\}
*/
    
#endif	/* EASYDSS_PROTOCOL_DEF_H */

