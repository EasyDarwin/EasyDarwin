/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#ifndef _EASYDARWIN_COMMON_DEF_H
#define _EASYDARWIN_COMMON_DEF_H

#include <stdio.h>
#include <string>
#include <time.h>
using namespace std;

#ifdef _WIN32
#define os_snprintf(str, len, fmt, ...) sprintf_s(str, len, fmt, ##__VA_ARGS__)
#else
#define os_snprintf(str, len, fmt, ...) snprintf(str, len, fmt, ##__VA_ARGS__)
#endif
#define MAX_SESSION_ID_LEN							32
#define MAX_STREAM_ID_LEN							32
/*
 * Data Type Definition.
 */
typedef void*    LHANDLE;

typedef	unsigned long	    uLong; 
typedef	signed long	        sLong; 

typedef signed char     	s8;
typedef unsigned char   	u8;
typedef int           		s32;
typedef unsigned int		u32;
typedef signed short		s16;
typedef unsigned short  	u16;

#if defined(WIN32)
typedef signed __int64		s64;
typedef unsigned __int64	u64;
#elif defined(__linux__)
typedef signed long long	s64;
typedef unsigned long long	u64;
#endif

#ifndef NULL
#define NULL    0
#endif

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#define	INLINE			__inline
#define EXTERN			extern

#ifndef MAX_PATH
#define MAX_PATH	256
#endif

#define MAKE_TIME_FLAG_DATE_AND_TIME 0X0
#define MAKE_TIME_FLAG_DATE_ONLY	 0X1
#define MAKE_TIME_FLAG_TIME_ONLY     0X2
#define MAKE_TIME_FLAG_TIME_TINY	 0x3

void   toupperstring(string &str);
int    PullMemory_Byte(void *srcBuff, int &nOffset, void *dstBuff, int nSize);
int	   PushMemory_Byte(void *dstBuff, int &nOffset, const void *srcBuff, int nSize);


#ifndef NULL
#define NULL    0
#endif


#define MAX_PASSWORD_LEN    64
#define MAX_SERIALNO_LEN    64
#define MAX_USERNAME_LEN	64
#define MAX_HOSTNAME_LEN    64

struct EasyDarwinDeviceAccount
{
	int     state;
	char	address[MAX_HOSTNAME_LEN];
	int		port;
	char    serialno[MAX_SERIALNO_LEN];
	char	username[MAX_USERNAME_LEN];
	char    password[MAX_PASSWORD_LEN];
	int		streamFlag;
	int		enableAudio;
	int		aac_encode;
	int		bufferSize;
};

struct EasyDarwinServer
{
	char     host[MAX_HOSTNAME_LEN];
	int      port;
	char     streamID[MAX_SESSION_ID_LEN*2];       //session ID
	char     sessionID[MAX_STREAM_ID_LEN*2];
};

string host2ip(const char* host);

#endif

