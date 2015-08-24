/*
	Copyleft (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
/*
    File:       HTTPSessionInterface.h
    Contains:   
*/

#ifndef __HTTPSESSIONINTERFACE_H__
#define __HTTPSESSIONINTERFACE_H__

#include "RTSPRequestStream.h"
#include "RTSPResponseStream.h"
#include "Task.h"
#include "QTSS.h"
#include "QTSSDictionary.h"
#include "atomic.h"
#include "base64.h"

class HTTPSessionInterface : public QTSSDictionary, public Task
{
public:

    static void     Initialize();
    
    HTTPSessionInterface();
    virtual ~HTTPSessionInterface();
    
    Bool16 IsLiveSession()      { return fSocket.IsConnected() && fLiveSession; }
    
    void RefreshTimeout()       { fTimeoutTask.RefreshTimeout(); }

    void IncrementObjectHolderCount() { (void)atomic_add(&fObjectHolders, 1); }
    void DecrementObjectHolderCount();
    
    RTSPRequestStream*  GetInputStream()    { return &fInputStream; }
    RTSPResponseStream* GetOutputStream()   { return &fOutputStream; }
    TCPSocket*          GetSocket()         { return &fSocket; }
    OSMutex*            GetSessionMutex()   { return &fSessionMutex; }
    
    UInt32              GetSessionIndex()      { return fSessionIndex; }
    
    void                SetRequestBodyLength(SInt32 inLength)   { fRequestBodyLen = inLength; }
    SInt32              GetRemainingReqBodyLen()                { return fRequestBodyLen; }
    
    // QTSS STREAM FUNCTIONS
    
    // Allows non-buffered writes to the client. These will flow control.
    
    // THE FIRST ENTRY OF THE IOVEC MUST BE BLANK!!!
    virtual QTSS_Error WriteV(iovec* inVec, UInt32 inNumVectors, UInt32 inTotalLength, UInt32* outLenWritten);
    virtual QTSS_Error Write(void* inBuffer, UInt32 inLength, UInt32* outLenWritten, UInt32 inFlags);
    virtual QTSS_Error Read(void* ioBuffer, UInt32 inLength, UInt32* outLenRead);
    virtual QTSS_Error RequestEvent(QTSS_EventType inEventMask);

	virtual QTSS_Error SendHTTPPacket(StrPtrLen* contentXML, Bool16 connectionClose, Bool16 decrement);

    enum
    {
        kMaxUserNameLen         = 32,
        kMaxUserPasswordLen     = 32
    };

protected:
    enum
    {
        kFirstHTTPSessionID     = 1,    //UInt32
    };

    //Each http session has a unique number that identifies it.

    char                fUserNameBuf[kMaxUserNameLen];
    char                fUserPasswordBuf[kMaxUserPasswordLen];
    char                fSessionID[QTSS_MAX_SESSION_ID_LENGTH];

    TimeoutTask         fTimeoutTask;//allows the session to be timed out
    
    RTSPRequestStream   fInputStream;
    RTSPResponseStream  fOutputStream;
    
    // Any RTP session sending interleaved data on this RTSP session must
    // be prevented from writing while an RTSP request is in progress
    OSMutex             fSessionMutex;


    //+rt  socket we get from "accept()"
    TCPSocket           fSocket;
    TCPSocket*          fOutputSocketP;
    TCPSocket*          fInputSocketP;  // <-- usually same as fSocketP, unless we're HTTP Proxying
    
    void        SnarfInputSocket( HTTPSessionInterface* fromHTTPSession );
    
    Bool16              fLiveSession;
    unsigned int        fObjectHolders;

    UInt32              fSessionIndex;
    UInt32              fLocalAddr;
    UInt32              fRemoteAddr;
    SInt32              fRequestBodyLen;
    
    UInt16              fLocalPort;
    UInt16              fRemotePort;
    
	Bool16				fAuthenticated;

    static unsigned int	sSessionIndexCounter;

    // Dictionary support Param retrieval function
    static void*        SetupParams(QTSSDictionary* inSession, UInt32* outLen);
    
    static QTSSAttrInfoDict::AttrInfo   sAttributes[];
};
#endif // __HTTPSESSIONINTERFACE_H__

