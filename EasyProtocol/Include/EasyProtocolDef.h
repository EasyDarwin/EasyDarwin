/*
	Copyright (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/

#ifndef EASY_PROTOCOL_DEF_H
#define	EASY_PROTOCOL_DEF_H

#if (defined(_WIN32))
#ifndef _DLL_
#define Easy_API
#else
#ifdef EASY_API_EXPORTS
#define Easy_API __declspec(dllexport) 
#else  
#define Easy_API __declspec(dllimport)
#endif
#endif
#elif defined(__linux__)
#define Easy_API
#endif
//#define Easy_API
#define EASY_PROTOCOL_VERSION                         "1.0"
#define EASY_PROTOCOL_STREAM_MAIN						"0"
#define EASY_PROTOCOL_STREAM_SUB						"1"

/*!
\ingroup EasyDarwin_Tag_Define
\{
*/
#define EASY_TAG_SESSIONS								"Sessions"
#define EASY_TAG_BITRATE								"Bitrate"
#define EASY_TAG_L_SOURCE								"source"
#define EASY_TAG_L_INDEX								"index"
#define EASY_TAG_L_CAMERA							    "camera"
#define EASY_TAG_TAG									"Tag"
#define EASY_TAG_DEVICE_COUNT							"DeviceCount"
#define EASY_TAG_CHANNEL_COUNT							"ChannelCount"
#define EASY_TAG_URL									"URL"
#define EASY_TAG_L_URL									"url"
#define EASY_TAG_SNAP_URL								"SnapURL"
#define EASY_TAG_SERVER_IP                              "Server_IP"
#define EASY_TAG_SERVER_PORT                            "Server_PORT"
#define EASY_TAG_TYPE									"Type"
#define EASY_TAG_SERIAL									"Serial"
#define EASY_TAG_CHANNEL								"Channel"
#define	EASY_TAG_CHANNELS								"Channels"
#define EASY_TAG_L_CHANNEL								"channel"
#define EASY_TAG_IMAGE								    "Image"
#define EASY_TAG_RESERVE								"Reserve"
#define EASY_TAG_L_RESERVE								"reserve"
#define EASY_TAG_ROOT									"EasyDarwin"
#define EASY_TAG_HEADER									"Header"
#define EASY_TAG_BODY									"Body"
#define EASY_TAG_VERSION								"Version"
#define EASY_TAG_TERMINAL_TYPE							"TerminalType"
#define EASY_TAG_APP_TYPE								"AppType"
#define EASY_TAG_SESSION_ID								"SessionID"
#define EASY_TAG_MESSAGE_TYPE							"MessageType"
#define EASY_TAG_CSEQ									"CSeq"
#define EASY_TAG_ERROR_NUM								"ErrorNum"
#define EASY_TAG_ERROR_STRING							"ErrorString"
#define EASY_TAG_REDIRECT								"Redirect"
#define EASY_TAG_SERVER_ADDRESS							"ServerAddress"
#define EASY_TAG_KEY									"Key"
#define EASY_TAG_NAME									"Name"
#define EASY_TAG_L_NAME									"name"
#define EASY_TAG_PASSWORD								"Password"
#define EASY_TAG_LAST_LOGIN_TIME						"LastLoginTime"
#define EASY_TAG_LAST_LOGIN_ADDRESS						"LastLoginAddress"
#define EASY_TAG_PERMISSION								"Permission"
#define EASY_TAG_PAGE_NUM								"PageNum"
#define EASY_TAG_DEVICE									"Device"
#define EASY_TAG_DEVICES								"Devices"
#define EASY_TAG_L_DEVICE								"device"
#define EASY_TAG_DEVICE_TYPE							"DeviceType"
#define EASY_TAG_DEVICE_SUM_PER_PAGE					"DeviceSumPerPage"
#define EASY_TAG_DEVICE_SUM								"DeviceSum"
#define EASY_TAG_DEVICE_LIST							"DeviceList"
#define EASY_TAG_INVALID_ACCESS_DATE					"InvalidAccessDate"
#define EASY_TAG_INVALID_ACCESS_TIME					"InvalidAccessTime"
#define EASY_TAG_FROM									"From"
#define EASY_TAG_TO										"To"
#define EASY_TAG_VIA									"Via"
#define EASY_TAG_STATUS									"Status"
#define EASY_TAG_SNAPSHOT								"Snapshot"
#define EASY_TAG_DESCRIPTION							"Description"
#define EASY_TAG_TOKEN									"Token"
#define EASY_TAG_CUSTOM_FIELD							"CustomField"
#define EASY_TAG_ORDER									"Order"
#define EASY_TAG_SERVICE_TYPE							"ServiceType"
#define EASY_TAG_SERVICE_UNIT							"ServiceUnit"
#define EASY_TAG_WAN_IP									"WanIP"
#define EASY_TAG_LAN_IP									"LanIP"
#define EASY_TAG_PORT									"Port"
#define EASY_TAG_LOAD									"Load"
#define EASY_TAG_USER									"User"
#define EASY_TAG_USER_NAME								"UserName"
#define EASY_TAG_PROTOCOL								"Protocol"
#define EASY_TAG_L_PROTOCOL								"protocol"
#define EASY_TAG_AUDIO									"Audio"
#define EASY_TAG_VIDEO									"Video"
#define EASY_TAG_CMD									"Command"
#define EASY_TAG_L_CMD                                  "command"
#define EASY_TAG_VALUE									"Value"
#define EASY_TAG_IP										"IP"
#define EASY_TAG_LIVE_TYPE								"LiveType"
#define EASY_TAG_USER_PLAY_COUNT						"UserPlayCount"
#define EASY_TAG_PROXY_PLAY_COUNT						"ProxyPlayCount"
#define EASY_TAG_PLAY_STREAM_ID							"PlayStreamID"
#define EASY_TAG_DEVICE_SERVER							"DeviceServer"
#define EASY_TAG_TIME									"Time"
#define EASY_TAG_L_TIME									"time"
#define EASY_TAG_SCHEDULE								"Schedule" 
#define EASY_TAG_STREAM_ID								"StreamID"
#define EASY_TAG_L_STREAM_ID							"streamid"
#define EASY_TAG_PLAY_COUNT								"PlayCount"
#define EASY_TAG_SESSION_COUNT							"SessionCount"
#define EASY_TAG_RECORDS								"Records"
#define EASY_TAG_AUDIENCE_NUM							"AudienceNum"
#define EASY_TAG_ACTION_TYPE                            "ActionType"
#define EASY_TAG_L_ACTION_TYPE                          "actiontype"
#define EASY_TAG_SPEED                                  "Speed"
#define EASY_TAG_L_SPEED                                "speed"
#define EASY_TAG_PRESET									"Preset"
#define EASY_TAG_L_PRESET								"preset"
#define EASY_TAG_AUDIO_TYPE								"AudioType"
#define EASY_TAG_AUDIO_DATA								"AudioData"
#define EASY_TAG_PTS									"Pts"

#define EASY_TAG_SERVER_HEADER							"Server"
#define EASY_TAG_SERVER_RUNNING_TIME					"RunningTime"
#define EASY_TAG_SERVER_HARDWARE						"Hardware"
#define EASY_TAG_SERVER_INTERFACE_VERSION				"InterfaceVersion"

#define EASY_TAG_CONFIG_SERVICE_LAN_PORT				"ServiceLanPort"
#define EASY_TAG_CONFIG_SERVICE_LAN_IP					"ServiceLanIP"
#define EASY_TAG_CONFIG_SERIVCE_WAN_PORT				"ServiceWanPort"
#define EASY_TAG_CONFIG_SERVICE_WAN_IP					"ServiceWanIP"
#define EASY_TAG_CONFIG_NGINX_LOCAL_PATH				"NginxLocalPath"
#define EASY_TAG_CONFIG_NGINX_WEB_PATH					"NginxWebPath"
#define EASY_TAG_CONFIG_NGINX_RTMP_PATH					"NginxRTMPPath"

/*!
\}
*/


/*!
\ingroup EasyDarwin_Message_Type_Define
\{
*/
#define MSG_DS_REGISTER_REQ									0x0001//设备注册
#define MSG_SD_REGISTER_ACK									0xe001
#define MSG_SD_PUSH_STREAM_REQ								0x0002//EasyCMS请求设备开始推流
#define MSG_DS_PUSH_STREAM_ACK								0xe002
#define MSG_SD_STREAM_STOP_REQ								0x0003//EasyCMS请求设备停止推流
#define MSG_DS_STREAM_STOP_ACK								0xe003
#define MSG_CS_DEVICE_LIST_REQ								0x0004//EasyClient向EasyCMS请求设备列表
#define MSG_SC_DEVICE_LIST_ACK								0xe004
#define MSG_CS_CAMERA_LIST_REQ								0x0005//EasyClient向EasyCMS请求摄像机列表
#define MSG_SC_CAMERA_LIST_ACK								0xe005
#define MSG_CS_GET_STREAM_REQ								0x0006//EasyClient向EasyCMS请求流
#define MSG_SC_GET_STREAM_ACK								0xe006
#define MSG_CS_FREE_STREAM_REQ								0x0007//EasyClient向EasyCMS请求释放流
#define MSG_SC_FREE_STREAM_ACK								0xe007
#define MSG_DS_POST_SNAP_REQ								0x0008//设备向EasyCMS上传快照
#define MSG_SD_POST_SNAP_ACK								0xe008
#define MSG_CS_DEVICE_INFO_REQ								0x0009//EasyClient向EasyCMS请求设备信息
#define MSG_SC_DEVICE_INFO_ACK								0xe009

#define MSG_CS_PTZ_CONTROL_REQ                              0x000a
#define MSG_SC_PTZ_CONTROL_ACK                              0xe00a
#define MSG_SD_CONTROL_PTZ_REQ                              0x000b
#define MSG_DS_CONTROL_PTZ_ACK                              0xe00b

#define MSG_CS_PRESET_CONTROL_REQ                           0x000c
#define MSG_SC_PRESET_CONTROL_ACK                           0xe00c
#define MSG_SD_CONTROL_PRESET_REQ                           0x000d
#define MSG_DS_CONTROL_PRESET_ACK                           0xe00d

#define MSG_CS_TALKBACK_CONTROL_REQ							0x000e
#define MSG_SC_TALKBACK_CONTROL_ACK							0xe00e
#define MSG_SD_CONTROL_TALKBACK_REQ							0x000f
#define MSG_DS_CONTROL_TALKBACK_ACK							0xe00f

#define	MSG_SC_SERVER_INFO_ACK								0x1000
#define MSG_SC_SERVER_BASE_CONFIG_ACK						0x1001
#define MSG_SC_SERVER_SET_BASE_CONFIG_ACK					0x1002


#define MSG_SC_START_HLS_ACK								0xeee0
#define MSG_SC_HLS_SESSION_LIST_ACK							0xeee1
#define MSG_SC_RTSP_PUSH_SESSION_LIST_ACK					0xeee2
#define MSG_SC_LIST_RECORD_ACK								0xeee3
#define MSG_SC_EXCEPTION									0xeeee

/*!
\}
*/

/*!
\ingroup EasyDarwin_Error_Define
\{
*/
#define EASY_ERROR_SUCCESS_OK							200             ///< Success OK
#define EASY_ERROR_SUCCESS_CREATED                   	201             ///< Success Created
#define EASY_ERROR_SUCCESS_ACCEPTED                  	202             ///< Success Accepted
#define EASY_ERROR_SUCCESS_NO_CONTENT                	204             ///< Success No Content
#define EASY_ERROR_SUCCESS_PARTIAL_CONTENT           	206             ///< Success Partial Content
#define EASY_ERROR_REDIRECT_PERMANENT_MOVED          	301             ///< Redirect Permanent Moved
#define EASY_ERROR_REDIRECT_TEMP_MOVED               	302             ///< Redirect Temp Moved
#define EASY_ERROR_REDIRECT_SEE_OTHER                	303             ///< Redirect See Other
#define EASY_ERROR_USE_PROXY                         	305             ///< Use Proxy
#define EASY_ERROR_CLIENT_BAD_REQUEST                	400             ///< Client Bad Request
#define EASY_ERROR_CLIENT_UNAUTHORIZED               	401             ///< Client Unauthorized
#define EASY_ERROR_PAYMENT_REQUIRED                  	402             ///< Payment Required
#define EASY_ERROR_CLIENT_FORBIDDEN                  	403             ///< Client Forbidden
#define EASY_ERROR_NOT_FOUND                         	404             ///< Not Found
#define EASY_ERROR_METHOD_NOT_ALLOWED                	405             ///< Method Not Allowed
#define EASY_ERROR_PROXY_AUTHENTICATION_REQUIRED     	407             ///< Proxy Authentication Required
#define EASY_ERROR_REQUEST_TIMEOUT                   	408             ///< Request Timeout
#define EASY_ERROR_CONFLICT                          	409             ///< Conflict
#define EASY_ERROR_PRECONDITION_FAILED               	412             ///< Precondition Failed
#define EASY_ERROR_UNSUPPORTED_MEDIA_TYPE            	415             ///< Unsupported Media Type
#define EASY_ERROR_SERVER_INTERNAL_ERROR             	500             ///< Server Internal Error
#define EASY_ERROR_SERVER_NOT_IMPLEMENTED            	501             ///< Server Not Implemented
#define EASY_ERROR_SERVER_BAD_GATEWAY                	502             ///< Server Bad Gateway
#define EASY_ERROR_SERVER_UNAVAILABLE                	503             ///< Server Unavailable
#define EASY_ERROR_VERSION_NOT_SUPPORTED        		505             ///< RTSP Version Not Supported

#define EASY_ERROR_MEMCACHE_NOT_FOUND					600				///< Memcache Not Found
#define EASY_ERROR_DATABASE_NOT_FOUND					601				///< Database Not Found
#define EASY_ERROR_USER_NOT_FOUND						602				///< User Not Found
#define EASY_ERROR_DEVICE_NOT_FOUND						603				///< Device Not Found
#define EASY_ERROR_SESSION_NOT_FOUND					604				///< Session Not Found
#define EASY_ERROR_SERVICE_NOT_FOUND					605				///< Service Not Found
#define EASY_ERROR_PASSWORD_ERROR						620				///< Password Error
#define EASY_ERROR_XML_PARSE_ERROR						621				///< XML Parse Error
#define EASY_ERROR_PERMISSION_ERROR						622				///< Permission Error
#define EASY_ERROR_LOCAL_SYSTEM_ERROR					623             ///< Local System Error
#define EASY_ERROR_PARAM_ERROR							624             ///< Param Error

#define EASY_ERROR_DEVICE_VERSION_TOO_OLD				700             ///< Device Version Too Old
#define EASY_ERROR_DEVICE_FAILURE						701             ///< Device Failure

/*!
\}
*/

/*!
\ingroup EasyDarwin_Time_Format_Define
\{
*/
enum EasyDarwinTimeFormat
{
	EASY_TIME_FORMAT_YYYYMMDDHHMMSS = 1,    ///< Format like 2014-08-31 08:15:30
	EASY_TIME_FORMAT_YYYYMMDDHHMMSSEx,		///<Format like 20140831081530
	EASY_TIME_FORMAT_YYYYMMDD,              ///< Format like 2014-08-31
	EASY_TIME_FORMAT_HHMMSS                 ///< Format like 08:15:30    
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
	EASY_DEVICE_STATUS_OFFLINE = 0,		///< DEVICE_STATUS_OFFLINE
	EASY_DEVICE_STATUS_ONLINE			///< DEVICE_STATUS_ONLINE 
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
	EASY_PROTOCOL_TYPE_RTSP = 1,			///< RTSP
	EASY_PROTOCOL_TYPE_HLS					///< HLS
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
	EASY_MEDIA_ENCODE_AUDIO_AAC = 1,			///< AAC
	EASY_MEDIA_ENCODE_VIDEO_H264				///< H264
};
/*!
\}
*/

/*!
\ingroup EasyDarwin_Terminal_Type_Define
\{
*/
enum EasyDarwinTerminalType
{
	EASY_TERMINAL_TYPE_ARM = 1,				///< ARM	
	EASY_TERMINAL_TYPE_Android = 2,				///< ANDROID
	EASY_TERMINAL_TYPE_IOS = 3,				///< IOS
	EASY_TERMINAL_TYPE_WEB = 4,				///< WEB
	EASY_TERMINAL_TYPE_WINDOWS = 5,				///< WINDOWS
	EASY_TERMINAL_TYPE_LINUX = 6					///< LINUX
};
/*!
\}
*/

/*!
\ingroup EasyDarwin_App_Type_Define
\{
*/
enum EasyDarwinAppType
{
	EASY_APP_TYPE_CAMERA = 1,					///< EasyCamera
	EASY_APP_TYPE_NVR = 2,					///< EasyNVR
	EASY_APP_TYPE_EASYDARWIN = 3						///< EasyDarwin
};
/*!
\}
*/

/*!
\ingroup EasyDarwin_Snap_Type_Define
\{
*/
enum EasyDarwinSnapType
{
	EASY_SNAP_TYPE_JPEG = 1,					///< JPEG
	EASY_SNAP_TYPE_IDR							///< IDR
};
/*!
\}
*/

/*!
\ingroup EasyDarwin_PTZ_ACTION_Type_Define
\{
*/
enum EasyDarwinPTZActionType
{
	EASY_PTZ_ACTION_TYPE_CONTINUOUS = 1,				///< CONTINUOUS
	EASY_PTZ_ACTION_TYPE_SINGLE 						///< SINGLE
};
/*!
\}
*/

/*!
\ingroup EasyDarwin_PTZ_CMD_Type_Define
\{
*/
enum EasyDarwinPTZCMDType
{
	EASY_PTZ_CMD_TYPE_STOP = 1,				    ///< STOP
	EASY_PTZ_CMD_TYPE_UP, 						///< UP
	EASY_PTZ_CMD_TYPE_DOWN, 					///< DOWN
	EASY_PTZ_CMD_TYPE_LEFT, 					///< LEFT
	EASY_PTZ_CMD_TYPE_RIGHT, 					///< RIGHT
	EASY_PTZ_CMD_TYPE_LEFTUP, 					///< LEFTUP
	EASY_PTZ_CMD_TYPE_LEFTDOWN, 				///< LEFTDOWN
	EASY_PTZ_CMD_TYPE_RIGHTUP, 					///< RIGHTUP
	EASY_PTZ_CMD_TYPE_RIGHTDOWN, 				///< RIGHTDOWN
	EASY_PTZ_CMD_TYPE_ZOOMIN, 					///< ZOOMIN
	EASY_PTZ_CMD_TYPE_ZOOMOUT, 					///< ZOOMOUT
	EASY_PTZ_CMD_TYPE_FOCUSIN, 					///< FOCUSIN
	EASY_PTZ_CMD_TYPE_FOCUSOUT, 				///< FOCUSOUT
	EASY_PTZ_CMD_TYPE_APERTUREIN, 				///< APERTUREIN
	EASY_PTZ_CMD_TYPE_APERTUREOUT, 				///< APERTUREOUT
};
/*!
\}
*/

/*!
\ingroup EasyDarwin_Preset_CMD_Type_Define
\{
*/
enum EasyDarwinPresetCMDType
{
	EASY_PRESET_CMD_TYPE_GOTO = 1,				    ///< GOTO
	EASY_PRESET_CMD_TYPE_SET, 						///< SET
	EASY_PRESET_CMD_TYPE_REMOVE, 					///< REMOVE

};
/*!
\}
*/

/*!
\ingroup EasyDarwin_Talkback_Audio_Type_Define
\{
*/
enum EasyDarwinTalkbackAudioType
{
	EASY_TALKBACK_AUDIO_TYPE_G711A = 1,				///< G711A
	EASY_TALKBACK_AUDIO_TYPE_G711U, 				///< G711U
	EASY_TALKBACK_AUDIO_TYPE_G726, 					///< G726

};
/*!
\}
*/

/*!
\ingroup EasyDarwin_Talkback_CMD_Type_Define
\{
*/
enum EasyDarwinTalkbackCMDType
{
	EASY_TALKBACK_CMD_TYPE_START = 1,				///< START
	EASY_TALKBACK_CMD_TYPE_SENDDATA, 				///< SENDDATA
	EASY_TALKBACK_CMD_TYPE_STOP, 					///< STOP

};
/*!
\}
*/

#endif	/* EASY_PROTOCOL_DEF_H */