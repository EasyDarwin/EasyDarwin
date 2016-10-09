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
	 File:       ClientSocket.cpp



 */
#ifndef __Win32__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#endif


#include "ClientSocket.h"
#include "OSMemory.h"
#include "base64.h"
#include "MyAssert.h"

#define CLIENT_SOCKET_DEBUG 0


ClientSocket::ClientSocket()
	: fHostAddr(0),
	fHostPort(0),
	fEventMask(0),
	fSocketP(nullptr),
	fSendBuffer(fSendBuf, 0),
	fSentLength(0)
{
}

OS_Error ClientSocket::Open(TCPSocket* inSocket)
{
	OS_Error theErr = OS_NoErr;
	if (!inSocket->IsBound())
	{
		theErr = inSocket->Open();
		if (theErr == OS_NoErr)
			theErr = inSocket->Bind(0, 0);

		if (theErr != OS_NoErr)
			return theErr;

		inSocket->NoDelay();
#if __FreeBSD__ || __MacOSX__
		// no KeepAlive -- probably should be off for all platforms.
#else
		inSocket->KeepAlive();
#endif

	}
	return theErr;
}

OS_Error ClientSocket::Connect(TCPSocket* inSocket)
{
	OS_Error theErr = this->Open(inSocket);
	Assert(theErr == OS_NoErr);
	if (theErr != OS_NoErr)
		return theErr;

	if (!inSocket->IsConnected())
	{
		theErr = inSocket->Connect(fHostAddr, fHostPort);
		if ((theErr == EINPROGRESS) || (theErr == EAGAIN))
		{
			fSocketP = inSocket;
			fEventMask = EV_RE | EV_WR;
			return theErr;
		}
	}
	return theErr;
}

OS_Error ClientSocket::Send(char* inData, const UInt32 inLength)
{
	iovec theVec[1];
	theVec[0].iov_base = (char*)inData;
	theVec[0].iov_len = inLength;

	return this->SendV(theVec, 1);
}

OS_Error ClientSocket::SendSendBuffer(TCPSocket* inSocket)
{
	OS_Error theErr = OS_NoErr;
	UInt32 theLengthSent = 0;

	if (fSendBuffer.Len == 0)
		return OS_NoErr;

	do
	{
		// theLengthSent should be reset to zero before passing its pointer to Send function
		// otherwise the old value will be used and it will go into an infinite loop sometimes
		theLengthSent = 0;
		//
		// Loop, trying to send the entire message.
		theErr = inSocket->Send(fSendBuffer.Ptr + fSentLength, fSendBuffer.Len - fSentLength, &theLengthSent);
		fSentLength += theLengthSent;

	} while (theLengthSent > 0);

	if (theErr == OS_NoErr)
		fSendBuffer.Len = fSentLength = 0; // Message was sent
	else
	{
		// Message wasn't entirely sent. Caller should wait for a read event on the POST socket
		fSocketP = inSocket;
		fEventMask = EV_WR;
	}
	return theErr;
}


TCPClientSocket::TCPClientSocket(UInt32 inSocketType)
	: fSocket(nullptr, inSocketType)
{
	//
	// It is necessary to open the socket right when we construct the
	// object because the QTSSSplitterModule that uses this class uses
	// the socket file descriptor in the QTSS_CreateStreamFromSocket call.
	fSocketP = &fSocket;
	this->Open(&fSocket);
}

void TCPClientSocket::SetOptions(int sndBufSize, int rcvBufSize)
{   //set options on the socket

	//qtss_printf("TCPClientSocket::SetOptions sndBufSize=%d,rcvBuf=%d,keepAlive=%d,noDelay=%d\n",sndBufSize,rcvBufSize,(int)keepAlive,(int)noDelay);
	int err = ::setsockopt(fSocket.GetSocketFD(), SOL_SOCKET, SO_SNDBUF, (char*)&sndBufSize, sizeof(int));
	AssertV(err == 0, OSThread::GetErrno());

	err = ::setsockopt(fSocket.GetSocketFD(), SOL_SOCKET, SO_RCVBUF, (char*)&rcvBufSize, sizeof(int));
	AssertV(err == 0, OSThread::GetErrno());

#if __FreeBSD__ || __MacOSX__
	struct timeval time;
	//int len = sizeof(time);
	time.tv_sec = 0;
	time.tv_usec = 0;

	err = ::setsockopt(fSocket.GetSocketFD(), SOL_SOCKET, SO_RCVTIMEO, (char*)&time, sizeof(time));
	AssertV(err == 0, OSThread::GetErrno());

	err = ::setsockopt(fSocket.GetSocketFD(), SOL_SOCKET, SO_SNDTIMEO, (char*)&time, sizeof(time));
	AssertV(err == 0, OSThread::GetErrno());
#endif

}

OS_Error TCPClientSocket::SendV(iovec* inVec, UInt32 inNumVecs)
{
	if (fSendBuffer.Len == 0)
	{
		for (UInt32 count = 0; count < inNumVecs; count++)
		{
			::memcpy(fSendBuffer.Ptr + fSendBuffer.Len, inVec[count].iov_base, inVec[count].iov_len);
			fSendBuffer.Len += inVec[count].iov_len;
			Assert(fSendBuffer.Len < ClientSocket::kSendBufferLen);
		}
	}

	OS_Error theErr = this->Connect(&fSocket);
	if (theErr != OS_NoErr)
		return theErr;

	return this->SendSendBuffer(&fSocket);
}

OS_Error TCPClientSocket::Read(void* inBuffer, const UInt32 inLength, UInt32* outRcvLen)
{
	this->Connect(&fSocket);
	OS_Error theErr = fSocket.Read(inBuffer, inLength, outRcvLen);
	if (theErr != OS_NoErr)
		fEventMask = EV_RE;
	return theErr;
}


HTTPClientSocket::HTTPClientSocket(const StrPtrLen& inURL, UInt32 inCookie, UInt32 inSocketType)
	: fCookie(inCookie),
	fSocketType(inSocketType),
	fGetReceived(0),

	fGetSocket(nullptr, inSocketType),
	fPostSocket(nullptr)
{
	fURL.Ptr = NEW char[inURL.Len + 1];
	fURL.Len = inURL.Len;
	::memcpy(fURL.Ptr, inURL.Ptr, inURL.Len);
	fURL.Ptr[fURL.Len] = '\0';
}

HTTPClientSocket::~HTTPClientSocket()
{
	delete[] fURL.Ptr;
	delete fPostSocket;
}

OS_Error HTTPClientSocket::Read(void* inBuffer, const UInt32 inLength, UInt32* outRcvLen)
{
	//
	// Bring up the GET connection if we need to
	if (!fGetSocket.IsConnected())
	{
#if CLIENT_SOCKET_DEBUG
		qtss_printf("HTTPClientSocket::Read: Sending GET\n");
#endif
		qtss_sprintf(fSendBuffer.Ptr, "GET %s HTTP/1.0\r\nX-SessionCookie: %"   _U32BITARG_   "\r\nAccept: application/x-rtsp-rtp-interleaved\r\nUser-Agent: QTSS/2.0\r\n\r\n", fURL.Ptr, fCookie);
		fSendBuffer.Len = ::strlen(fSendBuffer.Ptr);
		Assert(fSentLength == 0);
	}

	OS_Error theErr = this->Connect(&fGetSocket);
	if (theErr != OS_NoErr)
		return theErr;

	if (fSendBuffer.Len > 0)
	{
		theErr = this->SendSendBuffer(&fGetSocket);
		if (theErr != OS_NoErr)
			return theErr;
		fSentLength = 1; // So we know to execute the receive code below.
	}

	// We are done sending the GET. If we need to receive the GET response, do that here
	if (fSentLength > 0)
	{
		*outRcvLen = 0;
		do
		{
			// Loop, trying to receive the entire response.
			theErr = fGetSocket.Read(&fSendBuffer.Ptr[fGetReceived], kSendBufferLen - fGetReceived, outRcvLen);
			fGetReceived += *outRcvLen;

			// Check to see if we've gotten a \r\n\r\n. If we have, then we've received
			// the entire GET
			fSendBuffer.Ptr[fGetReceived] = '\0';
			char* theGetEnd = ::strstr(fSendBuffer.Ptr, "\r\n\r\n");

			if (theGetEnd != nullptr)
			{
				// We got the entire GET response, so we are ready to move onto
				// real RTSP response data. First skip past the \r\n\r\n
				theGetEnd += 4;

#if CLIENT_SOCKET_DEBUG
				qtss_printf("HTTPClientSocket::Read: Received GET response\n");
#endif

				// Whatever remains is part of an RTSP request, so move that to
				// the beginning of the buffer and blow away the GET
				*outRcvLen = fGetReceived - (theGetEnd - fSendBuffer.Ptr);
				::memcpy(inBuffer, theGetEnd, *outRcvLen);
				fGetReceived = fSentLength = 0;
				return OS_NoErr;
			}

			Assert(fGetReceived < inLength);
		} while (*outRcvLen > 0);

#if CLIENT_SOCKET_DEBUG
		qtss_printf("HTTPClientSocket::Read: Waiting for GET response\n");
#endif
		// Message wasn't entirely received. Caller should wait for a read event on the GET socket
		Assert(theErr != OS_NoErr);
		fSocketP = &fGetSocket;
		fEventMask = EV_RE;
		return theErr;
	}

	theErr = fGetSocket.Read(&((char*)inBuffer)[fGetReceived], inLength - fGetReceived, outRcvLen);
	if (theErr != OS_NoErr)
	{
#if CLIENT_SOCKET_DEBUG
		//qtss_printf("HTTPClientSocket::Read: Waiting for data\n");
#endif
		fSocketP = &fGetSocket;
		fEventMask = EV_RE;
	}
#if CLIENT_SOCKET_DEBUG
	//else
		//qtss_printf("HTTPClientSocket::Read: Got some data\n");
#endif
	return theErr;
}

OS_Error HTTPClientSocket::SendV(iovec* inVec, UInt32 inNumVecs)
{
	//
	// Bring up the POST connection if we need to
	if (fPostSocket == nullptr)
		fPostSocket = NEW TCPSocket(nullptr, fSocketType);

	if (!fPostSocket->IsConnected())
	{
#if CLIENT_SOCKET_DEBUG
		qtss_printf("HTTPClientSocket::Send: Sending POST\n");
#endif
		qtss_sprintf(fSendBuffer.Ptr, "POST %s HTTP/1.0\r\nX-SessionCookie: %"   _U32BITARG_   "\r\nAccept: application/x-rtsp-rtp-interleaved\r\nUser-Agent: QTSS/2.0\r\n\r\n", fURL.Ptr, fCookie);
		fSendBuffer.Len = ::strlen(fSendBuffer.Ptr);
		this->encodeVec(inVec, inNumVecs);
	}

	OS_Error theErr = this->Connect(fPostSocket);
	if (theErr != OS_NoErr)
		return theErr;

	//
	// If we have nothing to send currently, this should be a new message, in which case
	// we can encode it and send it
	if (fSendBuffer.Len == 0)
		this->encodeVec(inVec, inNumVecs);

#if CLIENT_SOCKET_DEBUG
	//qtss_printf("HTTPClientSocket::Send: Sending data\n");
#endif
	return this->SendSendBuffer(fPostSocket);
}

void HTTPClientSocket::encodeVec(iovec* inVec, UInt32 inNumVecs)
{
	for (UInt32 count = 0; count < inNumVecs; count++)
	{
		fSendBuffer.Len += ::Base64encode(fSendBuffer.Ptr + fSendBuffer.Len, (char*)inVec[count].iov_base, inVec[count].iov_len);
		Assert(fSendBuffer.Len < ClientSocket::kSendBufferLen);
		fSendBuffer.Len = ::strlen(fSendBuffer.Ptr); //Don't trust what the above function returns for a length
	}
}
