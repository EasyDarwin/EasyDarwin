/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       BaseSessionInterface.h

    Contains:   Presents an API for session-wide resources for modules to use.
                Implements the CMS Session dictionary for QTSS API. 
*/

#ifndef __BASESESSIONINTERFACE_H__
#define __BASESESSIONINTERFACE_H__

#include "BaseRequestStream.h"
#include "BaseResponseStream.h"
#include "Task.h"
#include "QTSS.h"
#include "QTSSDictionary.h"
#include "atomic.h"
#include "base64.h"

class BaseSessionInterface : public QTSSDictionary, public Task
{
public:

    //Initialize must be called right off the bat to initialize dictionary resources
    static void     Initialize();
    
    BaseSessionInterface();
    virtual ~BaseSessionInterface();
    
    //Is this session alive? If this returns false, clean up and begone as
    //fast as possible
    Bool16 IsLiveSession()      { return fSocket.IsConnected() && fLiveSession; }
    
    // Allows clients to refresh the timeout
    void RefreshTimeout()       { fTimeoutTask.RefreshTimeout(); }

    // In order to facilitate sending out of band data on the RTSP connection,
    // other objects need to have direct pointer access to this object. But,
    // because this object is a task object it can go away at any time. If # of
    // object holders is > 0, the RTSPSession will NEVER go away. However,
    // the object managing the session should be aware that if IsLiveSession returns
    // false it may be wise to relinquish control of the session
    void IncrementObjectHolderCount() { (void)atomic_add(&fObjectHolders, 1); }
    void DecrementObjectHolderCount();
    
    //Two main things are persistent through the course of a session, not
    //associated with any one request. The RequestStream (which can be used for
    //getting data from the client), and the socket. OOps, and the ResponseStream
    BaseRequestStream*  GetInputStream()    { return &fInputStream; }
    BaseResponseStream* GetOutputStream()   { return &fOutputStream; }
    TCPSocket*          GetSocket()         { return &fSocket; }
    OSMutex*            GetSessionMutex()   { return &fSessionMutex; }
    
    UInt32              GetSessionIndex()      { return fSessionIndex; }
    
    // Request Body Length
    // This object can enforce a length of the request body to prevent callers
    // of Read() from overrunning the request body and going into the next request.
    // -1 is an unknown request body length. If the body length is unknown,
    // this object will do no length enforcement. 
    void                SetRequestBodyLength(SInt32 inLength)   { fRequestBodyLen = inLength; }
    SInt32              GetRemainingReqBodyLen()                { return fRequestBodyLen; }

	OSRef*	GetRef()	{return &fDevRef; }
	QTSS_Error RegDevSession(const char* serial, UInt32 serailLen);
	QTSS_Error UpdateDevRedis();
	QTSS_Error UpdateDevSnap(const char* inSnapTime, const char* inSnapJpg);
	void UnRegDevSession();
    
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
        kFirstCMSSessionID     = 1,    //UInt32
    };

    //Each RTSP session has a unique number that identifies it.

    char                fUserNameBuf[kMaxUserNameLen];
    char                fUserPasswordBuf[kMaxUserPasswordLen];
    char                fSessionID[QTSS_MAX_SESSION_ID_LENGTH];
	char				fLastSMSSessionID[QTSS_MAX_SESSION_ID_LENGTH];

	char				fSerial[EASY_MAX_SERIAL_LENGTH];
	StrPtrLen			fDevSerialPtr;

	OSRef				fDevRef;

    TimeoutTask         fTimeoutTask;//allows the session to be timed out
    
    BaseRequestStream   fInputStream;
    BaseResponseStream  fOutputStream;
    
    // Any RTP session sending interleaved data on this RTSP session must
    // be prevented from writing while an RTSP request is in progress
    OSMutex             fSessionMutex;


    //+rt  socket we get from "accept()"
    TCPSocket           fSocket;
    TCPSocket*          fOutputSocketP;
    TCPSocket*          fInputSocketP;  // <-- usually same as fSocketP, unless we're HTTP Proxying
    
    void        SnarfInputSocket( BaseSessionInterface* fromRTSPSession );
    
    // What session type are we?
    QTSS_SessionType    fSessionType;
    Bool16              fLiveSession;
    unsigned int        fObjectHolders;

    UInt32              fSessionIndex;
    UInt32              fLocalAddr;
    UInt32              fRemoteAddr;
    SInt32              fRequestBodyLen;
    
    UInt16              fLocalPort;
    UInt16              fRemotePort;
    
	Bool16				fAuthenticated;

    static unsigned int		sSessionIndexCounter;

    // Dictionary support Param retrieval function
    static void*        SetupParams(QTSSDictionary* inSession, UInt32* outLen);
    
    static QTSSAttrInfoDict::AttrInfo   sAttributes[];
};
#endif // __BASESESSIONINTERFACE_H__

