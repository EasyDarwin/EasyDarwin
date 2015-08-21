/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/

#ifndef _CLIENT_SOCKET_H
#define _CLIENT_SOCKET_H
#include <string>
#include "TCPSocket.h"
#include "QTSS.h"

using namespace std;

namespace EasyDarwin { namespace libEasyCMS
{
#define CLIENT_STATE_DISCONNECTION 0
#define CLIENT_STATE_CONNECTTING 1
#define CLIENT_STATE_CONNECTED 2

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
            kSendBufferLen = EASY_REQUEST_BUFFER_SIZE_LEN	//512k
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
        virtual void    SetOptions(int sndBufSize = 8192,int rcvBufSize=4096);
        
        virtual UInt16  GetLocalPort() { return fSocket.GetLocalPort(); }
        
    private:
    
        TCPSocket   fSocket;
};

}}

#endif
