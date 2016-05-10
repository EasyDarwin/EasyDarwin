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
    File:       ClientSocket.h

    
    
*/


#ifndef __CLIENT_SOCKET__
#define __CLIENT_SOCKET__

#include "OSHeaders.h"
#include "TCPSocket.h"

class ClientSocket
{
    public:
    
        ClientSocket();
        virtual ~ClientSocket() {}
        
        void    Set(UInt32 hostAddr, UInt16 hostPort)
            { fHostAddr = hostAddr; fHostPort = hostPort; }
            
        //
        // Sends data to the server. If this returns EAGAIN or EINPROGRESS, call again
        // until it returns OS_NoErr or another error. On subsequent calls, you need not
        // provide a buffer.
        //
        // When this call returns EAGAIN or EINPROGRESS, caller should use GetEventMask
        // and GetSocket to wait for a socket event.
        OS_Error    Send(char* inData, const UInt32 inLength);

        //
        // Sends an ioVec to the server. Same conditions apply as above function 
        virtual OS_Error    SendV(iovec* inVec, UInt32 inNumVecs) = 0;
        
        //
        // Reads data from the server. If this returns EAGAIN or EINPROGRESS, call
        // again until it returns OS_NoErr or another error. This call may return OS_NoErr
        // and 0 for rcvLen, in which case you should call it again.
        //
        // When this call returns EAGAIN or EINPROGRESS, caller should use GetEventMask
        // and GetSocket to wait for a socket event.
        virtual OS_Error    Read(void* inBuffer, const UInt32 inLength, UInt32* outRcvLen) = 0;
        
        //
        // ACCESSORS
        UInt32          GetHostAddr()           { return fHostAddr; }
        virtual UInt32  GetLocalAddr() = 0;

        // If one of the above methods returns EWOULDBLOCK or EINPROGRESS, you
        // can check this to see what events you should wait for on the socket
        UInt32      GetEventMask()          { return fEventMask; }
        Socket*     GetSocket()             { return fSocketP; }
        
        virtual void    SetRcvSockBufSize(UInt32 inSize) = 0;

    protected:
    
        // Generic connect function
        OS_Error    Connect(TCPSocket* inSocket);   
        // Generic open function
        OS_Error    Open(TCPSocket* inSocket);
        
        OS_Error    SendSendBuffer(TCPSocket* inSocket);
    
        UInt32      fHostAddr;
        UInt16      fHostPort;
        
        UInt32      fEventMask;
        Socket*     fSocketP;

        enum
        {
            kSendBufferLen = 64*1024
        };
        
        // Buffer for sends.
        char        fSendBuf[kSendBufferLen + 1];
        StrPtrLen   fSendBuffer;
        UInt32      fSentLength;
};

class TCPClientSocket : public ClientSocket
{
    public:
        
        TCPClientSocket(UInt32 inSocketType);
        virtual ~TCPClientSocket() {}
        
        //
        // Implements the ClientSocket Send and Receive interface for a TCP connection
        virtual OS_Error    SendV(iovec* inVec, UInt32 inNumVecs);
        virtual OS_Error    Read(void* inBuffer, const UInt32 inLength, UInt32* outRcvLen);

        virtual UInt32  GetLocalAddr() { return fSocket.GetLocalAddr(); }
        virtual void    SetRcvSockBufSize(UInt32 inSize) { fSocket.SetSocketRcvBufSize(inSize); }
        virtual void    SetOptions(int sndBufSize = 8192,int rcvBufSize=1024);
        
        virtual UInt16  GetLocalPort() { return fSocket.GetLocalPort(); }
        
    private:
    
        TCPSocket   fSocket;
};

class HTTPClientSocket : public ClientSocket
{
    public:
        
        HTTPClientSocket(const StrPtrLen& inURL, UInt32 inCookie, UInt32 inSocketType);
        virtual ~HTTPClientSocket();
        
        //
        // Closes the POST half of the RTSP / HTTP connection
        void                ClosePost()     { delete fPostSocket; fPostSocket = NULL; }

        //
        // Implements the ClientSocket Send and Receive interface for an RTSP / HTTP connection
        virtual OS_Error    SendV(iovec* inVec, UInt32 inNumVecs);
		// Both SendV and Read use the fSendBuffer; so you cannot have both operations be running at the same time.
        virtual OS_Error    Read(void* inBuffer, const UInt32 inLength, UInt32* outRcvLen);

        virtual UInt32  GetLocalAddr() { return fGetSocket.GetLocalAddr();  }
        virtual void    SetRcvSockBufSize(UInt32 inSize) { fGetSocket.SetSocketRcvBufSize(inSize); }

    private:
    
        void        EncodeVec(iovec* inVec, UInt32 inNumVecs);

        StrPtrLen   fURL;
        UInt32      fCookie;
        
        UInt32      fSocketType;
        UInt32      fGetReceived;
        TCPSocket   fGetSocket;
        TCPSocket*  fPostSocket;
};

#endif //__CLIENT_SOCKET__
