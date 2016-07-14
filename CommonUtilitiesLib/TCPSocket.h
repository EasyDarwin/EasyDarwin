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
	 File:       TCPSocket.h

	 Contains:   TCP socket object




 */

#ifndef __TCPSOCKET_H__
#define __TCPSOCKET_H__

#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#ifndef __Win32__
#include <sys/types.h>
#include <sys/socket.h>
#endif

#include "Socket.h"
#include "Task.h"
#include "StrPtrLen.h"

class TCPSocket : public Socket
{
public:

	//TCPSocket takes an optional task object which will get notified when
	//certain events happen on this socket. Those events are:
	//
	//S_DATA:               Data is currently available on the socket.
	//S_CONNECTIONCLOSING:  Client is closing the connection. No longer necessary
	//                      to call Close or Disconnect, Snd & Rcv will fail.
	TCPSocket(Task* notifytask, UInt32 inSocketType)
		: Socket(notifytask, inSocketType),
		fRemoteStr(fRemoteBuffer, kIPAddrBufSize) {}
	virtual ~TCPSocket() {}

	//Open
	OS_Error    Open() { return Socket::Open(SOCK_STREAM); }

	// Connect. Attempts to connect to the specified remote host. If this
	// is a non-blocking socket, this function may return EINPROGRESS, in which
	// case caller must wait for either an EV_RE or an EV_WR. You may call
	// CheckAsyncConnect at any time, which will return OS_NoErr if the connect
	// has completed, EINPROGRESS if it is still in progress, or an appropriate error
	// if the connect failed.
	OS_Error    Connect(UInt32 inRemoteAddr, UInt16 inRemotePort);
	//OS_Error  CheckAsyncConnect();

	// Basically a copy constructor for this object, also NULLs out the data
	// in tcpSocket.        
	void        SnarfSocket(TCPSocket& tcpSocket);

	//ACCESSORS:
	//Returns NULL if not currently available.

	UInt32      GetRemoteAddr() { return ntohl(fRemoteAddr.sin_addr.s_addr); }
	UInt16      GetRemotePort() { return ntohs(fRemoteAddr.sin_port); }
	//This function is NOT thread safe!
	StrPtrLen*  GetRemoteAddrStr();

protected:

	void        set(int inSocket, struct sockaddr_in* remoteaddr);

	enum
	{
		kIPAddrBufSize = 20 //UInt32
	};

	struct sockaddr_in  fRemoteAddr;
	char fRemoteBuffer[kIPAddrBufSize];
	StrPtrLen fRemoteStr;


	friend class TCPListenerSocket;
};
#endif // __TCPSOCKET_H__

