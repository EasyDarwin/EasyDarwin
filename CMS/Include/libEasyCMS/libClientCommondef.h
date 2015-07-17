/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#ifndef _LIB_CLIENT_COMMON_DEF_H
#define _LIB_CLIENT_COMMON_DEF_H

#include <string>
#include <vector>
using namespace std;

///////////////////////////////////////////////////////////////////////////

namespace EasyDarwin { namespace libEasyCMS
{

#define LIB_CLIENT_RETURN_ERROR_OK 0

enum ClientMsgType
{
	CLIENT_ERROR = 0,
	CLIENT_MSG_REGISTER_USER,
	CLIENT_MSG_LOGIN,
	CLIENT_MSG_HEARTBEAT,
	CLIENT_MSG_GET_DEVICE_LIST,
	CLIENT_MSG_GET_DEVICE_INFO,
	CLIENT_MSG_GET_USER_INFO,
	CLIENT_MSG_PUBLISH_STREAM,
	CLIENT_MSG_PUBLISH_STREAM_START
};

#define LOGIN_TYPE_USER_ACCESS		"UserAccess"
#define LOGIN_TYPE_PHONE_NUM		"PhoneNum"
#define LOGIN_TYPE_EMAIL			"Email"
#define LOGIN_TYPE_SERIAL			"Serail"

enum
{
    Easy_NoErr              = 0,
    Easy_RequestFailed      = -1,
    Easy_Unimplemented      = -2,
    Easy_RequestArrived     = -3,
    Easy_OutOfState         = -4,
    Easy_NotAModule         = -5,
    Easy_WrongVersion       = -6,
    Easy_IllegalService     = -7,
    Easy_BadIndex           = -8,
    Easy_ValueNotFound      = -9,
    Easy_BadArgument        = -10,
    Easy_ReadOnly           = -11,
    Easy_NotPreemptiveSafe  = -12,
    Easy_NotEnoughSpace     = -13,
    Easy_WouldBlock         = -14,
    Easy_NotConnected       = -15,
    Easy_FileNotFound       = -16,
    Easy_NoMoreData         = -17,
    Easy_AttrDoesntExist    = -18,
    Easy_AttrNameExists     = -19,
    Easy_InstanceAttrsNotAllowed= -20,
	Easy_InvalidSocket		= -21,
	Easy_MallocError		= -22,
	Easy_ConnectError		= -23,
	Easy_SendError			= -24
};
typedef int Easy_Error;

/////////////////////////////////////////////////////////////////////////////////////////////
typedef void(*ClientLogCallback)(const char* msg, void* pClient);

}
}

#endif