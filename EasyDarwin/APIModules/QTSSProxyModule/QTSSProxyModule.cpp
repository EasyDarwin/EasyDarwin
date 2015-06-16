/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
 */
/*
    File:       QTSSProxyModule.cpp

    Contains:   Implementation of QTSSProxyModule class. 
                    
    This module is intended to be used in a stripped down version of QTSS/DSS
    in order to handle firewall proxy behaviour for the RTP protocol.
    

*/

#ifndef __Win32__
//
// For gethostbyname
#include <netdb.h>
#endif

#include "QTSSProxyModule.h"
#include "QTSSModuleUtils.h"
#include "UDPSocketPool.h"
#include "OSArrayObjectDeleter.h"
#include "OSMemory.h"
#include "RTSPClient.h"
#include "ClientSocket.h"
#include "SocketUtils.h"

class ProxyTask : public Task
{
    public:
    
        //
        // Task that just polls on all the sockets in the pool, sending data on all available sockets
        ProxyTask() : Task() {this->SetTaskName("ProxyTask");  this->Signal(Task::kStartEvent); }
        virtual ~ProxyTask() {}
    
    private:
    
        virtual SInt64 Run();
        
        enum
        {
            kProxyTaskPollIntervalMsec = 10
        };
};


class ProxySocketPool : public UDPSocketPool
{
    public:
    
        //
        // Global pool of UDP sockets used by the Proxy for receiving
        // UDP data from the upstream servers.
        ProxySocketPool() {}
        virtual ~ProxySocketPool() {}
        
        virtual UDPSocketPair*      ConstructUDPSocketPair();
        virtual void                DestructUDPSocketPair(UDPSocketPair *inPair);
        virtual void                SetUDPSocketOptions(UDPSocketPair* inPair);
};


class ProxyDemuxerTask : public UDPDemuxerTask
{
    public:
        
        ProxyDemuxerTask(QTSS_RTPStreamObject inStream, UDPSocketPair* inSockets)
                                                :   UDPDemuxerTask(),
                                                    fStream(inStream),
                                                    fQueueElem(this),
                                                    fSockets(inSockets) {}
        virtual ~ProxyDemuxerTask() {}
        
        //
        // ACCESSORS
        
        QTSS_RTPStreamObject    GetStream()             { return fStream; }
        OSQueueElem*            GetQueueElem()          { return &fQueueElem; }
        UDPSocketPair*          GetSockets()            { return fSockets; }
        UInt16                  GetOriginServerPort()   { return fOriginServerPort; }
        
        //
        // SetOriginServerPort - 
        // The origin server tells us what ports to send to on the SETUP
        // response. This object gets created on the SETUP request, so we
        // don't know the origin server ports when it is constructed. Once
        // we know what they are, call this method to let the object know what the ports are
        void                    SetOriginServerPort(UInt16 inPort) { fOriginServerPort = inPort; }

    private:
    
        QTSS_RTPStreamObject    fStream;
        OSQueueElem             fQueueElem;
        UDPSocketPair*          fSockets;
        UInt16                  fOriginServerPort;
};


class ProxyClientInfo
{
    public:
    
        //
        // This class contains all the proxy specific resources used by a single client of the
        // proxy. It is a UDP demuxer because it needs to get signalled when there is incoming
        // UDP data for this client
        ProxyClientInfo() :     fStream(NULL),
                                fClientSocket(Socket::kNonBlockingSocketType),
                                fClient(&fClientSocket, false),
                                fLastDemuxerTask(NULL) {}
        ~ProxyClientInfo();
        
        //
        // There are generally several streams for a single client. Each stream must
        // have a different source port from the origin server. These functions
        // accomplish this mapping.
        ProxyDemuxerTask*   AddStream(QTSS_RTPStreamObject inStream);
        ProxyDemuxerTask*   GetDemuxerTaskForStream(QTSS_RTPStreamObject inStream);
        
        ProxyDemuxerTask* GetLastDemuxerTask() { return fLastDemuxerTask; }

        //
        // An RTSPClient object that handles the details of communicating with
        // the upstream server over RTSP        
        RTSPClient*                 GetRTSPClient()     { return &fClient; }

        //
        // A QTSS_SocketStream for doing async I/O in QTSS API with the RTSP socket
        QTSS_SocketStream       GetSocketStream()       { return fStream; }
        void                    SetSocketStream(QTSS_SocketStream inStream) { fStream = inStream; }
        
    private:
      
        //
        // All the resources needed by 1 client of the proxy stored here, so this all can be
        // stuffed into a ClientSession
        QTSS_SocketStream           fStream;
        TCPClientSocket             fClientSocket;
        RTSPClient                  fClient;
        ProxyDemuxerTask*           fLastDemuxerTask;
        OSQueue                     fDemuxerTaskQueue;
        OSRef                       fRef;
};


// ATTRIBUTES

static QTSS_AttributeID         sProxyClientInfoAttr            = qtssIllegalAttrID;

static QTSS_AttributeID         sPortNumberTooBigErr            = qtssIllegalAttrID;
static QTSS_AttributeID         sRemoteHostRefusedConnectionErr = qtssIllegalAttrID;
static QTSS_AttributeID         sNoTransportHeaderInSetupErr    = qtssIllegalAttrID;

// STATIC DATA

static QTSS_PrefsObject         sServerPrefs = NULL;
static ProxySocketPool*         sSocketPool = NULL;
static ProxyTask*               sProxyTask = NULL;
static OSRefTable*              sSessionMap = NULL;


// FUNCTION PROTOTYPES

static QTSS_Error QTSSProxyModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error Register(QTSS_Register_Params* inParams);
static QTSS_Error Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error Shutdown();
static QTSS_Error DestroySession(QTSS_ClientSessionClosing_Params* inParams);
static QTSS_Error ProcessRTSPRequest(QTSS_StandardRTSP_Params* inParams);
static QTSS_Error ProcessIncomingRTCPPacket(QTSS_RTCPProcess_Params* inParams);

static QTSS_Error HandleRTSPClientErr(QTSS_RTSPRequestObject inRequest, ProxyClientInfo* inClient, QTSS_Error inErr);
static void RequestSocketEvent(QTSS_StreamRef inStream, UInt32 inEventMask);
static QTSS_Error   DoRequestPreProcessing(ProxyClientInfo* inProxyClientInfo, QTSS_RTSPRequestObject inRequest, QTSS_ClientSessionObject inSession);
static QTSS_Error   DoRequestPostProcessing(ProxyClientInfo* inProxyClientInfo, QTSS_RTSPRequestObject inRequest,
                                    QTSS_ClientSessionObject inClientSession);
static UInt32 ReplaceSessionAndTransportHeaders(StrPtrLen* inResponse, iovec* outNewResponse, StrPtrLen* inNewSessionID,
                                                        StrPtrLen* inNewTransportHeader, UInt32* outNewResponseLen);


// FUNCTION IMPLEMENTATIONS


QTSS_Error QTSSProxyModule_Main(void* inPrivateArgs)
{
    return _stublibrary_main(inPrivateArgs, QTSSProxyModuleDispatch);
}


QTSS_Error  QTSSProxyModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
    switch (inRole)
    {
        case QTSS_Register_Role:
            return Register(&inParams->regParams);
        case QTSS_Initialize_Role:
            return Initialize(&inParams->initParams);
        case QTSS_RTSPRequest_Role:
            return ProcessRTSPRequest(&inParams->rtspRequestParams);
        case QTSS_ClientSessionClosing_Role:
            return DestroySession(&inParams->clientSessionClosingParams);
        case QTSS_RTCPProcess_Role:
            return ProcessIncomingRTCPPacket(&inParams->rtcpProcessParams);
        case QTSS_Shutdown_Role:
            return Shutdown();
    }
    return QTSS_NoErr;
}


QTSS_Error Register(QTSS_Register_Params* inParams)
{
    // Do role & attribute setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_Shutdown_Role);
    (void)QTSS_AddRole(QTSS_RTSPRequest_Role);
    (void)QTSS_AddRole(QTSS_ClientSessionClosing_Role);
    (void)QTSS_AddRole(QTSS_RTCPProcess_Role);
    
    // Tell the server our name!
    static char* sModuleName = "QTSSProxyModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    static char*        sProxyClientInfoName    = "QTSSProxyModuleProxyClientInfo";

    (void)QTSS_AddStaticAttribute(qtssClientSessionObjectType, sProxyClientInfoName, NULL, qtssAttrDataTypeVoidPointer);
    (void)QTSS_IDForAttr(qtssClientSessionObjectType, sProxyClientInfoName, &sProxyClientInfoAttr);

    static char*        sPortNumberToBigErrName = "QTSSProxyModulePortNumberTooBig";
    static char*        sRemoteHostRefusedConnectionErrName = "QTSSProxyModuleRemoteHostRefusedConnection";
    static char*        sNoTransportHeaderInSetupErrName    = "QTSSProxyModuleNoTransportHeaderInSetup";

    (void)QTSS_AddStaticAttribute(qtssTextMessagesObjectType, sPortNumberToBigErrName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssTextMessagesObjectType, sPortNumberToBigErrName, &sPortNumberTooBigErr);

    (void)QTSS_AddStaticAttribute(qtssTextMessagesObjectType, sRemoteHostRefusedConnectionErrName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssTextMessagesObjectType, sRemoteHostRefusedConnectionErrName, &sRemoteHostRefusedConnectionErr);

    (void)QTSS_AddStaticAttribute(qtssTextMessagesObjectType, sNoTransportHeaderInSetupErrName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssTextMessagesObjectType, sNoTransportHeaderInSetupErrName, &sNoTransportHeaderInSetupErr);

    return QTSS_NoErr;
}


QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{
    // Setup module utils
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);
    
    //
    // Setup global data structures
    sServerPrefs = inParams->inPrefs;
    sSocketPool = NEW ProxySocketPool();
    sProxyTask = NEW ProxyTask();
    sSessionMap = NEW OSRefTable();
    
    // Report to the server that this module handles DESCRIBE, SETUP, PLAY, PAUSE, and TEARDOWN
    static QTSS_RTSPMethod sSupportedMethods[] = { qtssDescribeMethod, qtssSetupMethod, qtssTeardownMethod, qtssPlayMethod, qtssPauseMethod };
    QTSSModuleUtils::SetupSupportedMethods(inParams->inServer, sSupportedMethods, 5);

    return QTSS_NoErr;
}

QTSS_Error Shutdown()
{
    return QTSS_NoErr;
}

QTSS_Error DestroySession(QTSS_ClientSessionClosing_Params* inParams)
{
    ProxyClientInfo* theClient = NULL;
    UInt32 theLen = sizeof(theClient);
    QTSS_Error theErr = QTSS_GetValue(inParams->inClientSession, sProxyClientInfoAttr, 0,
                                                                &theClient, &theLen);

    if (theErr != QTSS_NoErr)
        return theErr;
        
    delete theClient;
    
    //
    // NULL out the attribute so in case there is a race condition no one will
    // get this dangling pointer
    (void)QTSS_SetValue(inParams->inClientSession, sProxyClientInfoAttr, 0, NULL, 0);
    return QTSS_NoErr;
}

QTSS_Error ProcessRTSPRequest(QTSS_StandardRTSP_Params* inParams)
{
    ProxyClientInfo* theClient = NULL;
    UInt32 theLen = sizeof(theClient);
    QTSS_Error theErr = QTSS_GetValue(inParams->inClientSession, sProxyClientInfoAttr, 0,
                                                                &theClient, &theLen);

    //
    // If there is no client yet, this is the first request made on this session.
    // Create an RTSPClient
    if (theErr != QTSS_NoErr)
    {
        theClient = NEW ProxyClientInfo();
        
        //
        // Parse out the DNS name of the origin server and the port
        StrPtrLen theDNSNameAndPort;
        theErr = QTSS_GetValuePtr(inParams->inRTSPHeaders, qtssHostHeader, 0, (void**)&theDNSNameAndPort.Ptr, &theDNSNameAndPort.Len);
        Assert(theErr == QTSS_NoErr);
        
        StringParser extractPortNumber(&theDNSNameAndPort);
        
        StrPtrLen theDNSNamePtr;
        extractPortNumber.GetThru(&theDNSNamePtr, ':');
        UInt32 thePort = extractPortNumber.ConsumeInteger(NULL);
        
        //
        // For now, if there was no port specified, use 554
        if (thePort == 0)
            thePort = 554;
            
        //
        // Make sure the port is in the range of a 2-byte integer
        if (thePort > 65536)
            return QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssClientBadRequest,
                                                        sPortNumberTooBigErr);
        
        //
        // gethostbyname takes a NULL terminated C-string
        OSCharArrayDeleter theDNSName(theDNSNamePtr.GetAsCString());

        //
        // Do a DNS lookup on the host to find its IP address
        struct hostent* theHostent = ::gethostbyname(theDNSName);
        
        UInt32 theIPAddr = 0;
        if (theHostent != NULL)
            theIPAddr = ntohl(*(UInt32*)(theHostent->h_addr_list[0]));
        else
            theIPAddr = SocketUtils::ConvertStringToAddr(theDNSName);
            
        //
        // Give this information to the ClientSocket object
        theClient->GetRTSPClient()->GetSocket()->Set(theIPAddr, (UInt16)thePort);
        
        theErr = QTSS_SetValue(inParams->inClientSession, sProxyClientInfoAttr, 0, &theClient, sizeof(theClient));
        Assert(theErr == QTSS_NoErr);

        //
        // Start the process of connecting to the origin server, first
        // just by telling the ClientSocket to send an empty message
        theErr = theClient->GetRTSPClient()->GetSocket()->Send(NULL, 0);
        if (theErr != QTSS_NoErr)
            //
            // we are connecting. This function will be called when connection is set up
            return HandleRTSPClientErr(inParams->inRTSPRequest, theClient, theErr);
    }
    
    //
    // If we aren't in the middle of sending a request, we have to set it up
    if (!theClient->GetRTSPClient()->IsTransactionInProgress())
        theErr = DoRequestPreProcessing(theClient, inParams->inRTSPRequest, inParams->inClientSession);
    else
        //
        // Continue our attempt to send this request
        theErr = theClient->GetRTSPClient()->DoTransaction();
    
    if (theErr != QTSS_NoErr)
        HandleRTSPClientErr(inParams->inRTSPRequest, theClient, theErr);
    else
        //
        // The response has come back from the origin server. Do whatever
        // processing we need on it, and send the response to the proxy client.
        theErr = DoRequestPostProcessing(theClient, inParams->inRTSPRequest, inParams->inClientSession);

    return theErr;
}

QTSS_Error HandleRTSPClientErr(QTSS_RTSPRequestObject inRequest, ProxyClientInfo* inClient, QTSS_Error inErr)
{
    if (inClient->GetSocketStream() == NULL)
    {
        QTSS_SocketStream theSockStream = NULL;

        // Create a socket stream for the TCP socket in the RTSPClient object. The socket stream will
        // allow this module to receive events on the socket
        QTSS_Error theErr = QTSS_CreateStreamFromSocket(inClient->GetRTSPClient()->GetSocket()->GetSocket()->GetSocketFD(), &theSockStream);
        Assert(theErr == QTSS_NoErr);
        Assert(theSockStream != NULL);
    
        //
        // Cache it for future use
        inClient->SetSocketStream(theSockStream);
    }

    if ((inErr == EAGAIN) || (inErr == EINPROGRESS))
    {
        //
        // Still not done. Return back to the server and wait for an event

        // We're making an assumption here that inClient only uses one socket to connect to
        // the server. We only have one stream, so we have to make that assumption.

        // Note that it is not necessary to have any kind of timeout here, because the server
        // naturally times out idle connections. If the server doesn't respond for awhile,
        // this session will naturally go away
        inClient->GetRTSPClient()->GetSocket()->GetSocket()->DontAutoCleanup();
        RequestSocketEvent(inClient->GetSocketStream(), inClient->GetRTSPClient()->GetSocket()->GetEventMask());
        return QTSS_NoErr; // We'll get called in the same method again when there is more work to do
    }
    
    if (inErr == QTSS_RequestFailed)
    {
        //
        // The remote host responded with a non 200 response. Forward it onto 
        // the proxy client as normal, let it figure out what to do.
        
        //
        // Or this might just be that we got back an error from QTSS_AddRTPStream,
        // in which case we don't need to do anything special.
        return QTSS_NoErr;
    }
    else
        return QTSSModuleUtils::SendErrorResponse(inRequest, qtssServerGatewayTimeout,
                                                    sRemoteHostRefusedConnectionErr);
}

void RequestSocketEvent(QTSS_StreamRef inStream, UInt32 inEventMask)
{
    //
    // Job of this function is to convert a CommonUtilitiesLib event mask to a QTSS Event mask
    QTSS_EventType theEvent = 0;
    
    if (inEventMask & EV_RE)
        theEvent |= QTSS_ReadableEvent;
    if (inEventMask & EV_WR)
        theEvent |= QTSS_WriteableEvent;
        
    QTSS_Error theErr = QTSS_RequestEvent(inStream, theEvent);
    Assert(theErr == QTSS_NoErr);
}


QTSS_Error  DoRequestPreProcessing(ProxyClientInfo* inProxyClientInfo,
                                    QTSS_RTSPRequestObject inRequest,
                                    QTSS_ClientSessionObject inSession)
{   
    QTSS_RTSPMethod* theMethod = NULL;
    UInt32 theLen = 0;
    QTSS_Error theErr = QTSS_GetValuePtr(inRequest, qtssRTSPReqMethod, 0,
                                                                (void**)&theMethod, &theLen);
    Assert(theErr == QTSS_NoErr);
    
    char theTransportHeaderBuf[128];
    StrPtrLen theTransportHeader(theTransportHeaderBuf, 0);

    //
    // The session ID passed to us is this server's session ID. Make sure to replace it
    // with the session ID passed to us by the upstream server, or else the upstream
    // server will not be able to properly handle this request.
    StrPtrLen* theProxyClientSessionIDPtr = inProxyClientInfo->GetRTSPClient()->GetSessionID();
    
    //
    // Truncate the \r\n at the end of this
    StrPtrLen theProxyClientSessionID(*theProxyClientSessionIDPtr);
    if (theProxyClientSessionID.Len > 0)
    {
        Assert(theProxyClientSessionID.Len >= 2);
        //
        // Truncate off the EOL
        if ((theProxyClientSessionID.Ptr[theProxyClientSessionID.Len - 1] == '\r') ||
            (theProxyClientSessionID.Ptr[theProxyClientSessionID.Len - 1] == '\n'))
            theProxyClientSessionID.Len--;
        if ((theProxyClientSessionID.Ptr[theProxyClientSessionID.Len - 1] == '\r') ||
            (theProxyClientSessionID.Ptr[theProxyClientSessionID.Len - 1] == '\n'))
            theProxyClientSessionID.Len--;
            
        //
        // Truncate off "Session: "
        Assert(theProxyClientSessionID.Len > 9);
        theProxyClientSessionID.Ptr += 9;
        theProxyClientSessionID.Len -= 9;
    }
    
    StrPtrLen theRequest;
    theErr = QTSS_GetValuePtr(inRequest, qtssRTSPReqFullRequest, 0,
            (void**)&theRequest.Ptr, &theRequest.Len);
    Assert(theErr == QTSS_NoErr);
    
    if (*theMethod == qtssSetupMethod)
    {
        //
        // If this is a SETUP, we need to do some special processing,
        // and rewrite the Transport header before sending it on.

        //
        // First, tell the server to setup a new QTSS_RTPStreamObject
        // The proxy only supports UDP transport right now, so make sure that that is what we get
        QTSS_RTPStreamObject theStream = NULL;
        theErr = QTSS_AddRTPStream(inSession, inRequest, &theStream, qtssASFlagsForceUDPTransport);
        if (theErr != QTSS_NoErr)
        {
            Assert(theErr == QTSS_RequestFailed);   // if any other error is returned,
                                                    // our error handling logic will get confused.
            return theErr;
        }
        
        //
        // Tell the ProxyClientInfo object about this new stream.
        // This will also allocate client ports on this proxy.
        ProxyDemuxerTask* theProxyDemuxerTask = inProxyClientInfo->AddStream(theStream);
        
        //
        // Build a new Transport header
        UInt16 theRTPPort = theProxyDemuxerTask->GetSockets()->GetSocketA()->GetLocalPort();
        
        qtss_sprintf(theTransportHeaderBuf, "Transport: RTP/AVP;unicast;client_port=%d-%d\r\n", theRTPPort, theRTPPort + 1);
        theTransportHeader.Len = ::strlen(theTransportHeaderBuf);
    }
    
    //
    // Build the new request and send it
    
    iovec theNewRequest[5];
    UInt32 totalResponseLen = 0;
    
    UInt32 theNumVecs = ReplaceSessionAndTransportHeaders(&theRequest, theNewRequest, &theProxyClientSessionID, &theTransportHeader, &totalResponseLen);

    return inProxyClientInfo->GetRTSPClient()->SendRTSPRequest(theNewRequest, theNumVecs);
}

QTSS_Error  DoRequestPostProcessing(ProxyClientInfo* inProxyClientInfo, QTSS_RTSPRequestObject inRequest,
                                    QTSS_ClientSessionObject inClientSession)
{
    
    QTSS_RTSPMethod* theMethod = NULL;
    UInt32 theLen = 0;
    QTSS_Error theErr = QTSS_GetValuePtr(inRequest, qtssRTSPReqMethod, 0, (void**)&theMethod, &theLen);
    Assert(theErr == QTSS_NoErr);

    char theTransportHeaderBuf[128];
    StrPtrLen theTransportHeader(theTransportHeaderBuf, 0);

    StrPtrLen theSessionID;
    theErr = QTSS_GetValuePtr(inClientSession, qtssCliSesRTSPSessionID, 0, (void**)&theSessionID.Ptr, &theSessionID.Len);
    Assert(theErr == QTSS_NoErr);
    
    StrPtrLen theResponse(inProxyClientInfo->GetRTSPClient()->GetResponse(), inProxyClientInfo->GetRTSPClient()->GetResponseLen());

    if ((*theMethod == qtssSetupMethod) &&
        (inProxyClientInfo->GetRTSPClient()->GetStatus() == 200))
    {
        ProxyDemuxerTask* thisStreamsDemuxer = inProxyClientInfo->GetLastDemuxerTask();
        thisStreamsDemuxer->SetOriginServerPort(inProxyClientInfo->GetRTSPClient()->GetServerPort());
        
        //
        // Build a new Transport header. This should basically be exactly what the
        // server would be sending back if the proxy module wasn't generating the response
        UInt16 theSvrRTPPort = 0;
        UInt16 theCliRTPPort = 0;
        UInt32 theLen = sizeof(theSvrRTPPort);
        theErr = QTSS_GetValue(thisStreamsDemuxer->GetStream(), qtssRTPStrSvrRTPPort, 0, &theSvrRTPPort, &theLen);
        Assert(theErr == QTSS_NoErr);
        theErr = QTSS_GetValue(thisStreamsDemuxer->GetStream(), qtssRTPStrClientRTPPort, 0, &theCliRTPPort, &theLen);
        Assert(theErr == QTSS_NoErr);
        
        char theIPAddrStr[20];
        theLen = 20;
        theErr = QTSS_GetValue(inClientSession, qtssCliRTSPSessLocalAddrStr, 0, &theIPAddrStr[0], &theLen);
        Assert(theErr == QTSS_NoErr);
        theIPAddrStr[theLen] = '\0';
        
        qtss_sprintf(theTransportHeader.Ptr, "Transport: RTP/AVP;unicast;client_port=%d-%d;server_port=%d-%d;source=%s\r\n",
            theCliRTPPort, theCliRTPPort + 1, theSvrRTPPort, theSvrRTPPort + 1, theIPAddrStr);
        
        theTransportHeader.Len = ::strlen(theTransportHeader.Ptr);
    }
    else if ((*theMethod == qtssPlayMethod) &&
            (inProxyClientInfo->GetRTSPClient()->GetStatus() == 200))
    {
        //
        // The client session needs to know that we are entering the playing state.
        // Otherwise, when we write RTP packet data to the QTSS_RTPStreamObjects,
        // the server won't really be able to figure out what to do with the packets.
        theErr = QTSS_Play(inClientSession, inRequest, 0);
        if (theErr != QTSS_NoErr)
        {
            Assert(theErr == QTSS_RequestFailed);
            return theErr;
        }
    }
    else if ((*theMethod == qtssPauseMethod) &&
            (inProxyClientInfo->GetRTSPClient()->GetStatus() == 200))
    {
        //
        // Same thing as above, just let the server know that we are no longer playing
        theErr = QTSS_Pause(inClientSession);
        Assert(theErr == QTSS_NoErr);
    }
    //
    // Build our ioVec with the new response
    // When building the iovec used for the response, we need to leave the first
    // iovec in the array empty, per QTSS API conventions.
    iovec theNewResponse[7];
    theNewResponse[0].iov_len = 0;
    UInt32 totalResponseLen = 0;

    //
    // Internally, the server keeps an RTSP Session ID for this QTSS_ClientSessionObject.
    // We need to replace the one the upstream server has given us with this server's
    // RTSP Session ID. Otherwise, when the server receives the next RTSP request,
    // this server will not be able to map the RTSP session ID properly. 
    UInt32 theNumVecs = ReplaceSessionAndTransportHeaders(&theResponse, &theNewResponse[1], &theSessionID, &theTransportHeader, &totalResponseLen);
    theNumVecs++; // Take into account that the first one is empty
    //
    // Also add in the content body of the response if there is one
    theNewResponse[theNumVecs].iov_base = inProxyClientInfo->GetRTSPClient()->GetContentBody();
    theNewResponse[theNumVecs].iov_len = inProxyClientInfo->GetRTSPClient()->GetContentLength();
    totalResponseLen += theNewResponse[theNumVecs].iov_len;
    theNumVecs++;
    
    UInt32 lenWritten = 0;
    theErr = QTSS_WriteV(inRequest, theNewResponse, theNumVecs, totalResponseLen, &lenWritten);
    Assert(lenWritten == totalResponseLen);
    Assert(theErr == QTSS_NoErr);
    
    return QTSS_NoErr;
}

UInt32 ReplaceSessionAndTransportHeaders(StrPtrLen* inResponse, iovec* outNewResponse, StrPtrLen* inNewSessionID,
                                                        StrPtrLen* inNewTransportHeader, UInt32* outNewResponseLen)
{
    static StrPtrLen sTransportHeader("Transport");
    static StrPtrLen sSessionHeader("Session");

    StringParser reqParser(inResponse);
    StrPtrLen theHeaderName;

    UInt32 curVecIndex = 0;
    outNewResponse[0].iov_base = inResponse->Ptr;
    *outNewResponseLen = 0;
    
    while (reqParser.GetDataRemaining() > 0)
    {
        reqParser.ConsumeWord(&theHeaderName);

        if (theHeaderName.EqualIgnoreCase(sTransportHeader.Ptr, sTransportHeader.Len))
        {
            //
            // Mark off the length of the last section of the header
			outNewResponse[curVecIndex].iov_len = (UInt32) ((PointerSizedUInt) theHeaderName.Ptr) - ((PointerSizedUInt) outNewResponse[curVecIndex].iov_base );
            *outNewResponseLen += outNewResponse[curVecIndex].iov_len;
            
            //
            // Insert the new Transport header
            outNewResponse[curVecIndex+1].iov_base = inNewTransportHeader->Ptr;
            outNewResponse[curVecIndex+1].iov_len = inNewTransportHeader->Len;
            *outNewResponseLen += inNewTransportHeader->Len;
            
            //
            // Move onto the next iovec
            curVecIndex+=2;
            
            //
            // Mark the start of the next section
            reqParser.GetThruEOL(NULL);
            outNewResponse[curVecIndex].iov_base = reqParser.GetCurrentPosition();
        }
        else if (theHeaderName.EqualIgnoreCase(sSessionHeader.Ptr, sSessionHeader.Len))
        {
            reqParser.ConsumeLength(NULL, 2); //skip over ": "
            
            //
            // Mark off the length of the last section of the header
            outNewResponse[curVecIndex].iov_len = (UInt32) ((PointerSizedUInt) reqParser.GetCurrentPosition()) - ((PointerSizedUInt) outNewResponse[curVecIndex].iov_base);
            *outNewResponseLen += outNewResponse[curVecIndex].iov_len;
            
            //
            // Insert new session ID
            outNewResponse[curVecIndex+1].iov_base = inNewSessionID->Ptr;
            outNewResponse[curVecIndex+1].iov_len = inNewSessionID->Len;
            *outNewResponseLen += inNewSessionID->Len;
            
            //
            // Move onto the next empty vec
            curVecIndex+=2;
            
            //
            // Move past the session ID. Be careful, there may be a ';' that we need to look for
            
            StrPtrLen theSessionID;
            reqParser.GetThruEOL(&theSessionID);
            
            outNewResponse[curVecIndex].iov_base = &theSessionID.Ptr[theSessionID.Len];
    
            while (theSessionID.Ptr < outNewResponse[curVecIndex].iov_base)
            {
                if (*theSessionID.Ptr == ';')
                {
                    outNewResponse[curVecIndex].iov_base = theSessionID.Ptr;
                    break;
                }
                theSessionID.Ptr++;
            }       
        }
        else
            reqParser.GetThruEOL(NULL);
    }
    
    //
    // Finish off the vec by marking the len of the last section
    outNewResponse[curVecIndex].iov_len = (UInt32) ((PointerSizedUInt) reqParser.GetCurrentPosition()) -  ((PointerSizedUInt) outNewResponse[curVecIndex].iov_base );

    //
    // And finish off the total length
    *outNewResponseLen += outNewResponse[curVecIndex].iov_len;

    //
    // Return the number of vecs written
    return curVecIndex+1;
}

QTSS_Error ProcessIncomingRTCPPacket(QTSS_RTCPProcess_Params* inParams)
{
    ProxyClientInfo* theClient = NULL;
    UInt32 theLen = sizeof(theClient);
    QTSS_Error theErr = QTSS_GetValue(inParams->inClientSession, sProxyClientInfoAttr, 0,
                                                                &theClient, &theLen);

    //
    // This role receives ALL RTCP packets. We are only interested in RTCP packets
    // sent to proxy sessions. So we figure this out based on whether there
    // is a ProxyClientInfo object in the client session
    if (theErr != QTSS_NoErr)
        return QTSS_NoErr;
        
    //
    // Let's forward this RTCP packet to the right upstream server
    ProxyDemuxerTask* theTask = theClient->GetDemuxerTaskForStream(inParams->inRTPStream);
    Assert(theTask != NULL);
    if (theTask == NULL)
        return QTSS_NoErr;

    //
    // Using the RTCP socket (SocketB) of the pair, send the packet to the origin server's
    // RTCP port (the port number stored is the RTP port)
    (void)theTask->GetSockets()->GetSocketB()->
        SendTo(theTask->GetRemoteAddr(), theTask->GetOriginServerPort() + 1, inParams->inRTCPPacketData, inParams->inRTCPPacketDataLen);

    return QTSS_NoErr;
}


SInt64 ProxyTask::Run()
{
    const UInt32 kMaxRTCPPacketSize = 2048;
    char thePacketBuffer[kMaxRTCPPacketSize];
    QTSS_PacketStruct thePacketStruct;
    thePacketStruct.packetTransmitTime = QTSS_Milliseconds();
    thePacketStruct.packetData = thePacketBuffer;

    (void)this->GetEvents();    

    OSMutexLocker locker(sSocketPool->GetMutex());
    for (OSQueueIter iter(sSocketPool->GetSocketQueue()); !iter.IsDone(); iter.Next())
    {
        UInt32 theRemoteAddr = 0;
        UInt16 theRemotePort = 0;

        UDPSocketPair* thePair = (UDPSocketPair*)iter.GetCurrent()->GetEnclosingObject();
        Assert(thePair != NULL);
        
        for (UInt32 x = 0; x < 2; x++)
        {
            QTSS_WriteFlags theFlags = qtssWriteFlagsNoFlags;
            
            UDPSocket* theSocket = NULL;
            if (x == 0)
            {
                theFlags = qtssWriteFlagsIsRTP;
                theSocket = thePair->GetSocketA();
            }
            else
            {
                theFlags = qtssWriteFlagsIsRTCP;
                theSocket = thePair->GetSocketB();
            }
            
            Assert(theSocket->GetDemuxer() != NULL);
            OSMutexLocker locker(theSocket->GetDemuxer()->GetMutex());
            
            //get all the outstanding packets for this socket
            while (true)
            {
                UInt32 thePacketLen = 0;
                theSocket->RecvFrom(&theRemoteAddr, &theRemotePort, thePacketStruct.packetData, 
                                kMaxRTCPPacketSize, &thePacketLen);
                if (thePacketLen == 0)
                    break;//no more packets on this socket!
                    
                ProxyDemuxerTask* theDemuxerTask = (ProxyDemuxerTask*)theSocket->GetDemuxer()->GetTask(theRemoteAddr, 0);
                if (theDemuxerTask != NULL)
                {
                    QTSS_RTPStreamObject theStream = theDemuxerTask->GetStream();
                    (void)QTSS_Write(theStream, &thePacketStruct, thePacketLen, NULL, theFlags);
                }
            }
        }
    }
    return kProxyTaskPollIntervalMsec;
}


UDPSocketPair*  ProxySocketPool::ConstructUDPSocketPair()
{
    Assert(sProxyTask != NULL);
    //construct a pair of UDP sockets, the lower one for RTP data (outgoing only, no demuxer
    //necessary), and one for RTCP data (incoming, so definitely need a demuxer).
    //These are nonblocking sockets that DON'T receive events (we are going to poll for data)
    return NEW
        UDPSocketPair(  NEW UDPSocket(sProxyTask, UDPSocket::kWantsDemuxer | Socket::kNonBlockingSocketType),
                        NEW UDPSocket(sProxyTask, UDPSocket::kWantsDemuxer | Socket::kNonBlockingSocketType));
}

void ProxySocketPool::DestructUDPSocketPair(UDPSocketPair* inPair)
{
    delete inPair->GetSocketA();
    delete inPair->GetSocketB();
    delete inPair;
}

void ProxySocketPool::SetUDPSocketOptions(UDPSocketPair* inPair)
{
#ifdef __Win32__
    //
    // On Win32, apparently the socket buffer size matters even though this is UDP and being
    // used for sending... on UNIX typically the socket buffer size doesn't matter because the
    // packet goes right down to the driver. On Win32, unless this is really big, we get packet loss.
    inPair->GetSocketA()->SetSocketBufSize(256 * 1024);
    inPair->GetSocketB()->SetSocketBufSize(256 * 1024);
#endif

    //
    // Always set the Rcv buf size for the RTCP sockets. This is important because the
    // server is going to be getting many packets on these sockets.
    inPair->GetSocketA()->SetSocketRcvBufSize(512 * 1024);
    inPair->GetSocketB()->SetSocketRcvBufSize(512 * 1024);
}


ProxyClientInfo::~ProxyClientInfo()
{
    UInt32 theHostAddr = fClient.GetSocket()->GetHostAddr();

    for (OSQueueIter iter(&fDemuxerTaskQueue); !iter.IsDone(); )
    {
        OSQueueElem* theElem = iter.GetCurrent();
        ProxyDemuxerTask* theTask = (ProxyDemuxerTask*)theElem->GetEnclosingObject();
        
        //
        // Move onto the next element becase we are going to be deleting this one
        iter.Next();
        theElem->Remove(); // Get off the queue before deleting
        
        //
        // Clean up
        UDPSocketPair* thePair = theTask->GetSockets();
        thePair->GetSocketA()->GetDemuxer()->UnregisterTask(theHostAddr, 0, theTask);
        thePair->GetSocketB()->GetDemuxer()->UnregisterTask(theHostAddr, 0, theTask);
        
        delete theTask;     
        sSocketPool->ReleaseUDPSocketPair(thePair);
    }
}

ProxyDemuxerTask*   ProxyClientInfo::GetDemuxerTaskForStream(QTSS_RTPStreamObject inStream)
{
    //
    // Iterate through the queue of demuxer tasks, finding the one with
    // the stream that matches this input
    for (OSQueueIter iter(&fDemuxerTaskQueue); !iter.IsDone(); iter.Next())
    {
        ProxyDemuxerTask* theTask = (ProxyDemuxerTask*)iter.GetCurrent()->GetEnclosingObject();
        if (theTask->GetStream() == inStream)
            return theTask;
    }
    return NULL;    
}


ProxyDemuxerTask* ProxyClientInfo::AddStream(QTSS_RTPStreamObject inStream)
{
    //
    // Allocate some UDP sockets out of our pool to receive the UDP data from
    // the server. Demuxing is based on the origin server's (host's) IP addr, so
    // pass that to the socket pool so it can properly allocate a pair of UDP sockets.
    // We don't know what the remote port is yet (we only find out when we get the
    // SETUP response from the origin), so just pass in 0.
    UInt32 theHostAddr = fClient.GetSocket()->GetHostAddr();
    UInt32 theLocalAddr = fClient.GetSocket()->GetLocalAddr();
    
    UDPSocketPair* thePair = sSocketPool->GetUDPSocketPair(theLocalAddr, 0, theHostAddr, 0);

    fLastDemuxerTask = NEW ProxyDemuxerTask(inStream, thePair);
    fDemuxerTaskQueue.EnQueue(fLastDemuxerTask->GetQueueElem());
    
    //
    // Tell the demuxers for these sockets to send any packets from this IP addr
    // to the ProxyDemuxerTask for this stream. This is how incoming UDP packets
    // will be routed to the proper QTSS_RTPStreamObject
    thePair->GetSocketA()->GetDemuxer()->RegisterTask(theHostAddr, 0, fLastDemuxerTask);
    thePair->GetSocketB()->GetDemuxer()->RegisterTask(theHostAddr, 0, fLastDemuxerTask);
    
    //
    // return the newly created ProxyDemuxerTask
    return fLastDemuxerTask;
}
