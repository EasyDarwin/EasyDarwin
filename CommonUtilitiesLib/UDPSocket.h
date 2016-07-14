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
	 File:       UDPSocket.h

	 Contains:   Adds additional Socket functionality specific to UDP.




 */


#ifndef __UDPSOCKET_H__
#define __UDPSOCKET_H__

#ifndef __Win32__
#include <sys/socket.h>
#include <sys/uio.h>
#endif

#include "Socket.h"
#include "UDPDemuxer.h"


class UDPSocket : public Socket
{
public:

	//Another socket type flag (in addition to the ones defined in Socket.h).
	//The value of this can't conflict with those!
	enum
	{
		kWantsDemuxer = 0x0100 //UInt32
	};

	UDPSocket(Task* inTask, UInt32 inSocketType);
	virtual ~UDPSocket() { if (fDemuxer != NULL) delete fDemuxer; }

	//Open
	OS_Error    Open() { return Socket::Open(SOCK_DGRAM); }

	OS_Error    JoinMulticast(UInt32 inRemoteAddr);
	OS_Error    LeaveMulticast(UInt32 inRemoteAddr);
	OS_Error    SetTtl(UInt16 timeToLive);
	OS_Error    SetMulticastInterface(UInt32 inLocalAddr);

	//returns an ERRNO
	OS_Error        SendTo(UInt32 inRemoteAddr, UInt16 inRemotePort,
		void* inBuffer, UInt32 inLength);

	OS_Error        RecvFrom(UInt32* outRemoteAddr, UInt16* outRemotePort,
		void* ioBuffer, UInt32 inBufLen, UInt32* outRecvLen);

	//A UDP socket may or may not have a demuxer associated with it. The demuxer
	//is a data structure so the socket can associate incoming data with the proper
	//task to process that data (based on source IP addr & port)
	UDPDemuxer*         GetDemuxer() { return fDemuxer; }

private:

	UDPDemuxer* fDemuxer;
	struct sockaddr_in  fMsgAddr;
};
#endif // __UDPSOCKET_H__

