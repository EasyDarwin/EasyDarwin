/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       ClientSocket.h

    Contains:   Client Socket
*/
#include "ClientSocket.h"

namespace EasyDarwin { namespace libEasyCMS
{
static char temp[2048]; 
static char * STRTOCHAR(StrPtrLen *theStr)
{
	temp[0] = 0;
	UInt32 len = theStr->Len < 2047 ? theStr->Len : 2047;
	if (theStr->Len > 0 || NULL != theStr->Ptr)
	{   memcpy(temp,theStr->Ptr,len); 
		temp[len] = 0;
	}
	else 
		strcpy(temp,"Empty Ptr or len is 0");
	return temp;
}

ClientSocket::ClientSocket()
:   fHostAddr(0),
	fHostPort(0),
	fEventMask(0),
	fSocketP(NULL),
	fSendBuffer(fSendBuf, 0),
	fSentLength(0)
{}

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
	{
		fSendBuffer.Len = fSentLength = 0; // Message was sent
	}
	else
	{
		// Message wasn't entirely sent. Caller should wait for a read event on the POST socket
		fSocketP = inSocket;
		fEventMask = EV_WR;
	}
	return theErr;
}


TCPClientSocket::TCPClientSocket(UInt32 inSocketType)
	: fSocket(NULL, inSocketType)
{
	//
	// It is necessary to open the socket right when we construct the
	// object because the QTSSSplitterModule that uses this class uses
	// the socket file descriptor in the QTSS_CreateStreamFromSocket call.
	fSocketP = &fSocket;
	this->Open(&fSocket);
}

void TCPClientSocket::SetOptions(int sndBufSize,int rcvBufSize)
{   //set options on the socket

	//printf("TCPClientSocket::SetOptions sndBufSize=%d,rcvBuf=%d,keepAlive=%d,noDelay=%d\n",sndBufSize,rcvBufSize,(int)keepAlive,(int)noDelay);
	int err = 0;
	err = ::setsockopt(fSocket.GetSocketFD(), SOL_SOCKET, SO_SNDBUF, (char*)&sndBufSize, sizeof(int));
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
}}