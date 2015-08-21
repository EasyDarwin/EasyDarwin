/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#include "CommonDef.h"
#ifdef _WIN32
#include <io.h>
#include <Windows.h>
#else
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <errno.h>
#endif
#include <string.h> //for memcpy
#include <algorithm>

void toupperstring(string &str)
{
	transform(str.begin(), str.end(), str.begin(), (int(*)(int))toupper);
}

int  PullMemory_Byte(void *srcBuff, int &nOffset, void *dstBuff, int nSize)
{
	if (srcBuff == NULL || nOffset<0 || dstBuff == NULL || nSize == 0)
	{
		return -1;
	}

	memcpy(dstBuff, ((char*)srcBuff)+nOffset, nSize);

	nOffset+=nSize;

	return 0;
}

int PushMemory_Byte(void *dstBuff, int &nOffset, const void *srcBuff, int nSize)
{
	if (srcBuff == NULL || nOffset<0 || dstBuff == NULL || nSize == 0)
	{
		return -1;
	}

	memcpy(((char*)dstBuff)+nOffset, srcBuff, nSize);

	nOffset += nSize;

	return 0;

}


string host2ip(const char* host)
{
#ifndef _WIN32
    typedef struct hostent HOSTENT;
    //signal(SIGPIPE, SIG_IGN);
#endif
    HOSTENT *host_entry = ::gethostbyname(host);
    string ip;
    if (host_entry == NULL)
    {
        switch (h_errno)
        {
        case HOST_NOT_FOUND:
            fputs("The host was not found.\n", stderr);
            break;
        case NO_ADDRESS:
            fputs("The name is valid but it has no address.\n", stderr);
            break;
        case NO_RECOVERY:
            fputs("A non-recoverable name server error occurred.\n", stderr);
            break;
        case TRY_AGAIN:
            fputs("The name server is temporarily unavailable.", stderr);
            break;
        }
    }
    else
    {
        ip = string(inet_ntoa(*((struct in_addr *) host_entry->h_addr_list[0])));
    }

	return ip;
}

