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
	 File:       Socket.cpp

	 Contains:   implements Socket class



 */

#include <string.h>

#ifndef __Win32__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include <unistd.h>
#include <netinet/tcp.h>

#endif

#include <errno.h>

#include "Socket.h"
#include "SocketUtils.h"

#ifdef USE_NETLOG
#include <netlog.h>
#else
#if defined(__Win32__) || defined(__sgi__) || defined(__osf__) || defined(__hpux__)		
typedef int socklen_t; // missing from some platform includes
#endif
#endif


EventThread* Socket::sEventThread = NULL;

Socket::Socket(Task *notifytask, UInt32 inSocketType)
	: EventContext(EventContext::kInvalidFileDesc, sEventThread),
	fState(inSocketType),
	fLocalAddrStrPtr(NULL),
	fLocalDNSStrPtr(NULL),
	fPortStr(fPortBuffer, kPortBufSizeInBytes)
{
	fLocalAddr.sin_addr.s_addr = 0;
	fLocalAddr.sin_port = 0;

	fDestAddr.sin_addr.s_addr = 0;
	fDestAddr.sin_port = 0;

	this->SetTask(notifytask);

#if SOCKET_DEBUG
	fLocalAddrStr.Set(fLocalAddrBuffer, sizeof(fLocalAddrBuffer));
#endif

}

OS_Error Socket::Open(int theType)
{
	Assert(fFileDesc == EventContext::kInvalidFileDesc);
	fFileDesc = ::socket(PF_INET, theType, 0);
	if (fFileDesc == EventContext::kInvalidFileDesc)
		return (OS_Error)OSThread::GetErrno();

	//
	// Setup this socket's event context
	if (fState & kNonBlockingSocketType)
		this->InitNonBlocking(fFileDesc);

	return OS_NoErr;
}

void Socket::ReuseAddr()
{
	int one = 1;
	int err = ::setsockopt(fFileDesc, SOL_SOCKET, SO_REUSEADDR, (char*)&one, sizeof(int));
	Assert(err == 0);
}

void Socket::NoDelay()
{
	int one = 1;
	int err = ::setsockopt(fFileDesc, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(int));
	Assert(err == 0);
}

void Socket::KeepAlive()
{
	int one = 1;
	int err = ::setsockopt(fFileDesc, SOL_SOCKET, SO_KEEPALIVE, (char*)&one, sizeof(int));
	Assert(err == 0);
}

void    Socket::SetSocketBufSize(UInt32 inNewSize)
{

#if SOCKET_DEBUG
	int value;
	int buffSize = sizeof(value);
	int error = ::getsockopt(fFileDesc, SOL_SOCKET, SO_SNDBUF, (void*)&value, (socklen_t*)&buffSize);
#endif

	int bufSize = inNewSize;
	int err = ::setsockopt(fFileDesc, SOL_SOCKET, SO_SNDBUF, (char*)&bufSize, sizeof(int));
	AssertV(err == 0, OSThread::GetErrno());

#if SOCKET_DEBUG
	int setValue;
	error = ::getsockopt(fFileDesc, SOL_SOCKET, SO_SNDBUF, (void*)&setValue, (socklen_t*)&buffSize);
	qtss_printf("Socket::SetSocketBufSize ");
	if (fState & kBound)
	{
		if (NULL != this->GetLocalAddrStr())
			this->GetLocalAddrStr()->PrintStr(":");
		if (NULL != this->GetLocalPortStr())
			this->GetLocalPortStr()->PrintStr(" ");
	}
	else
		qtss_printf("unbound ");
	qtss_printf("socket=%d old SO_SNDBUF =%d inNewSize=%d setValue=%d\n", (int)fFileDesc, value, bufSize, setValue);
#endif

}

OS_Error    Socket::SetSocketRcvBufSize(UInt32 inNewSize)
{
#if SOCKET_DEBUG
	int value;
	int buffSize = sizeof(value);
	int error = ::getsockopt(fFileDesc, SOL_SOCKET, SO_RCVBUF, (void*)&value, (socklen_t*)&buffSize);
#endif

	int bufSize = inNewSize;
	int err = ::setsockopt(fFileDesc, SOL_SOCKET, SO_RCVBUF, (char*)&bufSize, sizeof(int));

#if SOCKET_DEBUG
	int setValue;
	error = ::getsockopt(fFileDesc, SOL_SOCKET, SO_RCVBUF, (void*)&setValue, (socklen_t*)&buffSize);
	qtss_printf("Socket::SetSocketRcvBufSize ");
	if (fState & kBound)
	{
		if (NULL != this->GetLocalAddrStr())
			this->GetLocalAddrStr()->PrintStr(":");
		if (NULL != this->GetLocalPortStr())
			this->GetLocalPortStr()->PrintStr(" ");
	}
	else
		qtss_printf("unbound ");
	qtss_printf("socket=%d old SO_RCVBUF =%d inNewSize=%d setValue=%d\n", (int)fFileDesc, value, bufSize, setValue);
#endif


	if (err == -1)
		return OSThread::GetErrno();

	return OS_NoErr;
}


OS_Error Socket::Bind(UInt32 addr, UInt16 port, UInt16 test)
{
	socklen_t len = sizeof(fLocalAddr);
	::memset(&fLocalAddr, 0, sizeof(fLocalAddr));
	fLocalAddr.sin_family = AF_INET;
	fLocalAddr.sin_port = htons(port);
	fLocalAddr.sin_addr.s_addr = htonl(addr);

	int err;

#if 0
	if (test) // pick some ports or conditions to return an error on.
	{
		if (6971 == port)
		{
			fLocalAddr.sin_port = 0;
			fLocalAddr.sin_addr.s_addr = 0;
			return EINVAL;
		}
		else
		{
			err = ::bind(fFileDesc, (sockaddr *)&fLocalAddr, sizeof(fLocalAddr));
		}
	}
	else
#endif
		err = ::bind(fFileDesc, (sockaddr *)&fLocalAddr, sizeof(fLocalAddr));


	if (err == -1)
	{
		fLocalAddr.sin_port = 0;
		fLocalAddr.sin_addr.s_addr = 0;
		return (OS_Error)OSThread::GetErrno();
	}
	else ::getsockname(fFileDesc, (sockaddr *)&fLocalAddr, &len); // get the kernel to fill in unspecified values
	fState |= kBound;
	return OS_NoErr;
}

StrPtrLen*  Socket::GetLocalAddrStr()
{
	//Use the array of IP addr strings to locate the string formatted version
	//of this IP address.
	if (fLocalAddrStrPtr == NULL)
	{
		for (UInt32 x = 0; x < SocketUtils::GetNumIPAddrs(); x++)
		{
			if (SocketUtils::GetIPAddr(x) == ntohl(fLocalAddr.sin_addr.s_addr))
			{
				fLocalAddrStrPtr = SocketUtils::GetIPAddrStr(x);
				break;
			}
		}
	}

#if SOCKET_DEBUG    
	if (fLocalAddrStrPtr == NULL)
	{   // shouldn't happen but no match so it was probably a failed socket connection or accept. addr is probably 0.

		fLocalAddrBuffer[0] = 0;
		fLocalAddrStrPtr = &fLocalAddrStr;
		struct in_addr theAddr;
		theAddr.s_addr = ntohl(fLocalAddr.sin_addr.s_addr);
		SocketUtils::ConvertAddrToString(theAddr, &fLocalAddrStr);

		printf("Socket::GetLocalAddrStr Search IPs failed, numIPs=%d\n", SocketUtils::GetNumIPAddrs());
		for (UInt32 x = 0; x < SocketUtils::GetNumIPAddrs(); x++)
		{
			printf("ip[%"   _U32BITARG_   "]=", x); SocketUtils::GetIPAddrStr(x)->PrintStr("\n");
		}
		printf("this ip = %d = ", theAddr.s_addr); fLocalAddrStrPtr->PrintStr("\n");

		if (theAddr.s_addr == 0 || fLocalAddrBuffer[0] == 0)
			fLocalAddrStrPtr = NULL; // so the caller can test for failure
	}
#endif 

	Assert(fLocalAddrStrPtr != NULL);
	return fLocalAddrStrPtr;
}

StrPtrLen*  Socket::GetLocalDNSStr()
{
	//Do the same thing as the above function, but for DNS names
	Assert(fLocalAddr.sin_addr.s_addr != INADDR_ANY);
	if (fLocalDNSStrPtr == NULL)
	{
		for (UInt32 x = 0; x < SocketUtils::GetNumIPAddrs(); x++)
		{
			if (SocketUtils::GetIPAddr(x) == ntohl(fLocalAddr.sin_addr.s_addr))
			{
				fLocalDNSStrPtr = SocketUtils::GetDNSNameStr(x);
				break;
			}
		}
	}

	//if we weren't able to get this DNS name, make the DNS name the same as the IP addr str.
	if (fLocalDNSStrPtr == NULL)
		fLocalDNSStrPtr = this->GetLocalAddrStr();

	Assert(fLocalDNSStrPtr != NULL);
	return fLocalDNSStrPtr;
}

StrPtrLen*  Socket::GetLocalPortStr()
{
	if (fPortStr.Len == kPortBufSizeInBytes)
	{
		int temp = ntohs(fLocalAddr.sin_port);
		qtss_sprintf(fPortBuffer, "%d", temp);
		fPortStr.Len = ::strlen(fPortBuffer);
	}
	return &fPortStr;
}

OS_Error Socket::Send(const char* inData, const UInt32 inLength, UInt32* outLengthSent)
{
	Assert(inData != NULL);

	if (!(fState & kConnected))
		return (OS_Error)ENOTCONN;

	int err;
	do {
		err = ::send(fFileDesc, inData, inLength, 0);//flags??
	} while ((err == -1) && (OSThread::GetErrno() == EINTR));
	if (err == -1)
	{
		//Are there any errors that can happen if the client is connected?
		//Yes... EAGAIN. Means the socket is now flow-controleld
		int theErr = OSThread::GetErrno();
		if ((theErr != EAGAIN) && (this->IsConnected()))
			fState ^= kConnected;//turn off connected state flag
		return (OS_Error)theErr;
	}

	*outLengthSent = err;
	return OS_NoErr;
}

OS_Error Socket::WriteV(const struct iovec* iov, const UInt32 numIOvecs, UInt32* outLenSent)
{
	Assert(iov != NULL);

	if (!(fState & kConnected))
		return (OS_Error)ENOTCONN;

	int err;
	do {
#ifdef __Win32__
		DWORD theBytesSent = 0;
		err = ::WSASend(fFileDesc, (LPWSABUF)iov, numIOvecs, &theBytesSent, 0, NULL, NULL);
		if (err == 0)
			err = theBytesSent;
#else
		err = ::writev(fFileDesc, iov, numIOvecs);//flags??
#endif
	} while ((err == -1) && (OSThread::GetErrno() == EINTR));
	if (err == -1)
	{
		// Are there any errors that can happen if the client is connected?
		// Yes... EAGAIN. Means the socket is now flow-controleld
		int theErr = OSThread::GetErrno();
		if ((theErr != EAGAIN) && (this->IsConnected()))
			fState ^= kConnected;//turn off connected state flag
		return (OS_Error)theErr;
	}
	if (outLenSent != NULL)
		*outLenSent = (UInt32)err;

	return OS_NoErr;
}

OS_Error Socket::Read(void *buffer, const UInt32 length, UInt32 *outRecvLenP)
{
	Assert(outRecvLenP != NULL);
	Assert(buffer != NULL);

	if (!(fState & kConnected))
		return (OS_Error)ENOTCONN;

	//int theRecvLen = ::recv(fFileDesc, buffer, length, 0);//flags??
	int theRecvLen;
	do {
		theRecvLen = ::recv(fFileDesc, (char*)buffer, length, 0);//flags??
	} while ((theRecvLen == -1) && (OSThread::GetErrno() == EINTR));

	if (theRecvLen == -1)
	{
		// Are there any errors that can happen if the client is connected?
		// Yes... EAGAIN. Means the socket is now flow-controleld
		int theErr = OSThread::GetErrno();
		if ((theErr != EAGAIN) && (this->IsConnected()))
			fState ^= kConnected;//turn off connected state flag
		return (OS_Error)theErr;
	}
	//if we get 0 bytes back from read, that means the client has disconnected.
	//Note that and return the proper error to the caller
	else if (theRecvLen == 0)
	{
		fState ^= kConnected;
		return (OS_Error)ENOTCONN;
	}
	Assert(theRecvLen > 0);
	*outRecvLenP = (UInt32)theRecvLen;
	return OS_NoErr;
}
