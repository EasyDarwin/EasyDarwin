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
    File:       QTSSMP3StreamingModule.cpp

    Contains:   This module will reflect a ShoutCast/IceCast-style MP3 broadcast 
                out to multiple clients.
    
    Written by: Steve Ussery    

*/

#ifndef kVersionString
#include "revision.h"
#endif

#include "QTSSMP3StreamingModule.h"
#include "QTSSModuleUtils.h"
#include "QTSSRollingLog.h"
#include "StringParser.h"
#include "OSMemory.h"
#include "OS.h"

#define DEBUG_MP3STREAMING_MODULE 0

#if DEBUG_MP3STREAMING_MODULE
#define DTRACE(s)       qtss_printf(s)
#define DTRACE1(s,n1)       qtss_printf(s,n1)
#define DTRACE2(s,n1,n2)    qtss_printf(s,n1,n2)
#else
#define DTRACE(s)
#define DTRACE1(s,n1)
#define DTRACE2(s,n1,n2)
#endif

class QTSSMP3AccessLog;

//**************************************************
#define kOKHeader "OK\r\n"
#ifdef __MacOSX__
#define kClientAcceptHeader "HTTP/1.1 200 OK\r\nServer: QuickTime Streaming Server %s/%s\r\nContent-Type: audio/mpeg\r\nCache-Control: no-cache\r\nPragma: no-cache\r\nConnection: close\r\n"
#define kM3UReplyHeader "HTTP/1.1 200 OK\r\nServer: QuickTime Streaming Server %s/%s\r\nContent-Type: audio/mpegurl\r\nCache-Control: no-cache\r\nPragma: no-cache\r\nConnection: close\r\nContent-Length:"
#else
#define kClientAcceptHeader "HTTP/1.1 200 OK\r\nServer: EasyDarwin %s/%s\r\nContent-Type: audio/mpeg\r\nCache-Control: no-cache\r\nPragma: no-cache\r\nConnection: close\r\n"
#define kM3UReplyHeader "HTTP/1.1 200 OK\r\nServer: EasyDarwin %s/%s\r\nContent-Type: audio/mpegurl\r\nCache-Control: no-cache\r\nPragma: no-cache\r\nConnection: close\r\nContent-Length:"
#endif
#define kSourceReject "Error - Mount Point Taken or Invalid\r\n"
#define kSourceBadPassword "Error - Bad Password\r\n"
#define kClientCLHeader "Content-Length: 54000000\r\n\r\n"
#define kRemoteAddressSize 20
#define kBandwidthToAddEstimate 16000

// STATIC DATA
static QTSS_ModulePrefsObject   sPrefs      = NULL;
static QTSS_PrefsObject     sServerPrefs    = NULL;
static OSBufferPool*        sOSBufferPoolPtr = NULL;
static QTSSMP3AccessLog*    sMP3AccessLog   = NULL;
static QTSS_ServerObject    sServer     = NULL;
static OSMutex*         sLogMutex   = NULL; // MP3 access log isn't reentrant
static OSMutex*         sAtomicMutex    = NULL;
static Bool16           sServerIdle     = true;

// Server global MP3 Session management classes.
static MP3BroadcasterQueue  sMP3BroadcasterQueue;
static MP3SessionTable      sMP3SessionTable;

// PREFERENCE VALUES
static UInt32           sBroadcastBufferSize        = 8192;
static UInt32           sDefaultBroadcastBufferSize     = 8192;
static char*            sBroadcastPassword      = NULL;
static char*            sDefaultBroadcastPassword   = " ";
static Bool16           sMP3StreamingEnabled        = true;
static Bool16           sDefaultMP3StreamingEnabled     = true;
static SInt32           sMaximumConnections         = 0;
static SInt32           sMaximumBandwidth       = 0;
static UInt32           sDefaultRollInterval    = 7;
static Bool16           sDefaultLogTimeInGMT    = true;
static UInt32           sDefaultMaxLogBytes     = 10240000;
static SInt32           sDefaultFlowControlTimeInMSec   = 10000;
static SInt32           sMaxFlowControlTimeInMSec       = 10000;

static Bool16   sDefaultLogEnabled  = true;
static char*    sDefaultLogName     = "mp3_access";
static char*    sDefaultLogDir = NULL;

static char*    sLogName        = NULL;
static char*    sLogDir         = NULL;
static Bool16   sLogEnabled     = true;
static UInt32   sMaxLogBytes    = 10240000;
static UInt32   sRollInterval   = 7;
static Bool16   sLogTimeInGMT   = true;

static char* sLogHeader =   "#Software: %s\n"
                                "#Version: %s\n"    //%s == version
                                "#Date: %s\n"       //%s == date/time
                                "#Remark: All date values are in %s.\n" //%s == "GMT" or "local time"
                                "#Remark: c-duration is in seconds.\n"
                                "#Fields: c-ip c-user-agent [date time] cs-uri c-status c-bytes c-duration\n";
                                
static char gClientAcceptHeader[1024];
static char gM3UReplyHeader[1024];
                    
// FUNCTION PROTOTYPES

static QTSS_Error QTSSMP3StreamingModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error Register(QTSS_Register_Params* inParams);
static QTSS_Error Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error FilterRequest(QTSS_Filter_Params* inParams);
static QTSS_Error RereadPrefs();
static QTSS_Error SessionClosing(QTSS_RTSPSession_Params* inParams);
static QTSS_Error StateChange(QTSS_StateChange_Params* stateChangeParams);
static QTSS_Error Shutdown();

static void IncrementMP3SessionCount();
static void DecrementMP3SessionCount();
static void IncrementTotalMP3Bytes(UInt32 bytes);
static Bool16 CheckBandwidth(SInt32 bandwidth);

#if DEBUG_MP3STREAMING_MODULE
    static void PrintStringBuffer(StrPtrLen& stringBuffer);
#else
    #define PrintStringBuffer(stringBuffer) ;
#endif

static void KeepSession(QTSS_RTSPRequestObject theRequest,Bool16 keep);
static UInt32 GetRTSPSessionID(QTSS_RTSPSessionObject session);
static Bool16 IsActiveBroadcastSession(QTSS_RTSPSessionObject session);
static MP3BroadcasterSession* FindBroadcastSession(QTSS_RTSPSessionObject session);
static Bool16 IsBroadcastPassword(StrPtrLen& theRequest);
static Bool16 IsShoutcastPassword(StrPtrLen& theRequest);
static Bool16 IsHTTPGet(StrPtrLen& theRequest);
static Bool16 IsA_m3u_URL(char* theURL);
static Bool16 IsMetaDataURL(char* theURL);
static void   ParseMetaDataURL(char* theURL);
static Bool16 CheckPassword(QTSS_Filter_Params* inParams, StrPtrLen& theRequest, StrPtrLen& mountpoint);
static Bool16 NeedsMetaData(StrPtrLen& theRequest);
static Bool16 ParseURL(StrPtrLen& theRequest, char* outURL, UInt16 maxlen);
static QTSS_Error LogRequest(QTSS_RTSPSessionObject inRTSPSession, MP3ClientSession* client);
static void WriteStartupMessage();
static void WriteShutdownMessage();
static void url_strcpy(char* dest, const char* src);
static QTSS_Error ReEnterFilterRequest(QTSS_Filter_Params* inParams, MP3Session* mp3Session);

static Bool16 IsHTTP(StrPtrLen& theRequest);
static Bool16 IsRTSP(StrPtrLen& theRequest);


//**************************************************
// CLASS DECLARATIONS
//**************************************************

class QTSSMP3AccessLog : public QTSSRollingLog
{
    public:
    
        QTSSMP3AccessLog();
        virtual ~QTSSMP3AccessLog() {}
    
        virtual char* GetLogName() { return QTSSModuleUtils::GetStringAttribute(sPrefs, "mp3_request_logfile_name", sDefaultLogName); }
        virtual char* GetLogDir()  { return QTSSModuleUtils::GetStringAttribute(sPrefs, "mp3_request_logfile_dir", sDefaultLogDir); }
        virtual UInt32 GetRollIntervalInDays()  { return sRollInterval; }
        virtual UInt32 GetMaxLogBytes()         { return sMaxLogBytes; }
        virtual time_t WriteLogHeader(FILE *inFile);
};

// ---------------------------------------------------------------------------
// CLASS IMPLEMENTATIONS
// ---------------------------------------------------------------------------

// ****************************************************************************
// QTSSMP3AccessLog -- subclass of QTSSRollingLog
// ****************************************************************************
QTSSMP3AccessLog::QTSSMP3AccessLog() : QTSSRollingLog() 
{
    this->SetTaskName("QTSSMP3AccessLog");
}

time_t QTSSMP3AccessLog::WriteLogHeader(FILE *inFile)
{
    //The point of this header is to record the exact time the log file was created,
    //in a format that is easy to parse through whenever we open the file again.
    //This is necessary to support log rolling based on a time interval, and POSIX doesn't
    //support a create date in files.
    time_t calendarTime = ::time(NULL);
    Assert(-1 != calendarTime);
    if (-1 == calendarTime)
        return -1;

    struct tm  timeResult;
    struct tm* theLocalTime = qtss_localtime(&calendarTime, &timeResult);
    Assert(NULL != theLocalTime);
    if (NULL == theLocalTime)
        return -1;
    
    char tempBuffer[1024] = { 0 };
    qtss_strftime(tempBuffer, sizeof(tempBuffer), "#Log File Created On: %m/%d/%Y %H:%M:%S\n", theLocalTime);
    this->WriteToLog(tempBuffer, !kAllowLogToRoll);
    tempBuffer[0] = '\0';
    
    //format a date for the startup time
    char theDateBuffer[QTSSRollingLog::kMaxDateBufferSizeInBytes] = { 0 };
    Bool16 result = QTSSRollingLog::FormatDate(theDateBuffer, false);
    
    if (result)
    {
        StrPtrLen serverName;
        (void)QTSS_GetValuePtr(sServer, qtssSvrServerName, 0, (void**)&serverName.Ptr, &serverName.Len);
        StrPtrLen serverVersion;
        (void)QTSS_GetValuePtr(sServer, qtssSvrServerVersion, 0, (void**)&serverVersion.Ptr, &serverVersion.Len);
        qtss_sprintf(tempBuffer, sLogHeader, serverName.Ptr , serverVersion.Ptr, 
                            theDateBuffer, sLogTimeInGMT ? "GMT" : "local time");
        this->WriteToLog(tempBuffer, !kAllowLogToRoll);
    }
        
    return calendarTime;
}

// ****************************************************************************
// MP3Session -- This is a base class to hold all the MP3 related
// session state info.
// ****************************************************************************

// initialize static value of class.
SInt32 MP3Session::sTotalNumMP3Sessions = 0L;


MP3Session::MP3Session(QTSS_RTSPSessionObject sess, QTSS_StreamRef stream) : 
    fState(0),
    fResult(200),
    fStream(stream),
    fSession(sess),
    fSessID(0)
{
    sTotalNumMP3Sessions++;
    sMP3SessionTable.RegisterSession(this);
}

MP3Session::MP3Session() : 
    fState(0),
    fResult(0),
    fStream(NULL),
    fSession(NULL),
    fSessID(0)
{
    // this should never be called
    Assert(0);
}

MP3Session::~MP3Session()
{
    --sTotalNumMP3Sessions;
    sMP3SessionTable.UnRegisterSession(this);
    fStream = NULL;
    fSession = NULL;
    fSessID = 0;
}

UInt8 MP3Session::IsA() const
{
    return kMP3UndefinedSessionType;
}

// ****************************************************************************
// MP3BroadcasterSession -- This is a class to hold all the MP3 Broadcaster
// session state info.
// ****************************************************************************
MP3BroadcasterSession::MP3BroadcasterSession()
{
    // this should never be called.
    Assert(0);
}

MP3BroadcasterSession::MP3BroadcasterSession(QTSS_RTSPSessionObject sess, QTSS_StreamRef stream) : 

    MP3Session(sess, stream),
    fMP3ClientQueue(NULL),
    fDataBufferLen(0),
    fDataBufferSize(sBroadcastBufferSize),
    fNewSongName(false)
{
    fBuffer = (char*) sOSBufferPoolPtr->Get();
    InitBroadcastSessionState();
    fHeader[0] = '\0';
    fSongName[0] = '\0';
}

MP3BroadcasterSession::~MP3BroadcasterSession()
{
    SetState(MP3BroadcasterSession::kBroadcasterShutDownState);
    delete fMP3ClientQueue;
    fMP3ClientQueue = NULL;
    sOSBufferPoolPtr->Put(fBuffer);
    fBuffer = NULL;
}

UInt8 MP3BroadcasterSession::IsA() const
{
    return kMP3BroadcasterSessionType;
}

void MP3BroadcasterSession::InitBroadcastSessionState()
{
    ::memset(fMountpoint, 0, kURLBufferSize);
    ::memset(fHeader, 0, kHeaderBufferSize);
    ::memset(fSongName, 0, kSongNameBufferSize);
    ::memset(fBuffer, 0, fDataBufferSize);
    SetMountpoint("/");
    fDataBufferLen = 0;
    SetSessionID(0);
    if (fMP3ClientQueue == NULL)
        fMP3ClientQueue = NEW MP3ClientQueue();
    SetState(MP3BroadcasterSession::kBroadcasterInitState);
}

// Run this broadcaster session's state machine.
// This is called each time through the QTSS_FilterRequest mode which
// is highly re-entrant.
QTSS_Error MP3BroadcasterSession::ExecuteState()
{
    QTSS_Error err = QTSS_NoErr;
    
    // The only states we handle here are kBroadcasterGetHeaderState and
    // the kBroadcasterRecvDataState. All others are ignored.
    
    if (GetState() == MP3BroadcasterSession::kBroadcasterGetHeaderState)
    {
        err = GetBroadcastHeaders();
    }
    else if (GetState() == MP3BroadcasterSession::kBroadcasterRecvDataState)
    {
        err = GetBroadcastData();
    }
    
    return err;
}

// Put this broadcast session in a state where it will accept incoming
// broadcaster connection request.
void MP3BroadcasterSession::AcceptBroadcastSessionState()
{
    SetState(MP3BroadcasterSession::kBroadcasterValidatePasswordState);
}

// Put this broadcast session in a state where it has accepted
// the broadcaster connection request.
void MP3BroadcasterSession::AcceptPasswordState()
{
    SetState(MP3BroadcasterSession::kBroadcasterGetHeaderState);
}

// Put this broadcast session in a state where it will reflect
// MP3 data to connected clients.
void MP3BroadcasterSession::AcceptDataStreamState()
{
    SetState(MP3BroadcasterSession::kBroadcasterRecvDataState);
}

// Put this broadcast session in a state where it will no
// longer accept connections or reflect MP3 data.
void MP3BroadcasterSession::ShutDownState()
{
    SetState(MP3BroadcasterSession::kBroadcasterShutDownState);
}

// Set the mount point URL that clients will associate
// with this broadcaster session.
void MP3BroadcasterSession::SetMountpoint(char* mp)
{
    StrPtrLen mpStr(mp);    
    SetMountpoint(mpStr);
}

// Set the mount point URL that clients will associate
// with this broadcaster session.
void MP3BroadcasterSession::SetMountpoint(StrPtrLen& mp)
{
    ::memset(fMountpoint, 0, kURLBufferSize);
    if (mp.Ptr != NULL && mp.Len > 0 && mp.Len < kURLBufferSize)
        ::memcpy(fMountpoint, mp.Ptr, mp.Len);
}

// Set the song name of the currently broadcasting session.
void MP3BroadcasterSession::SetSongName(char* sn)
{
    if (sn != NULL && *sn != '\0')
    {
        // This mutex is to guard against updating a
        // song name string while we're updating our queued
        // clients.
        OSMutexLocker locker(&fSongNameMutex);
        url_strcpy(fSongName, sn);
        fNewSongName = true;
    }
}

// See if a given mount point URL matches.
Bool16 MP3BroadcasterSession::MountpointEqual(char* mp)
{
    StrPtrLen mpStr(mp);    
    return MountpointEqual(mpStr);
}

// See if a given mount point URL matches.
Bool16 MP3BroadcasterSession::MountpointEqual(StrPtrLen& mp)
{
    if (0 == *fMountpoint)
        return false;
        
    return mp.Equal(fMountpoint);
}

// Add a MP3 client session ref to our broadcaster
QTSS_Error MP3BroadcasterSession::AddClient(QTSS_RTSPSessionObject sess, QTSS_StreamRef stream)
{
    if (fMP3ClientQueue == NULL)
    {
        Assert(0);
        return QTSS_RequestFailed;
    }
    return fMP3ClientQueue->AddClient(sess, stream, this);
}

// Is this RTSP session one of my clients?
Bool16 MP3BroadcasterSession::IsMyClient(QTSS_RTSPSessionObject sess)
{
    if (fMP3ClientQueue == NULL)
    {
        return false;
    }
    return fMP3ClientQueue->InQueue(sess);
}

// Remove a MP3 client session ref from our broadcaster
QTSS_Error MP3BroadcasterSession::RemoveClient(QTSS_RTSPSessionObject sess)
{
    if (fMP3ClientQueue == 0)
    {
        Assert(0);
        return QTSS_BadArgument;
    }
    return fMP3ClientQueue->RemoveClient(sess);
}

//Send acknowledgement response to broacaster
QTSS_Error MP3BroadcasterSession::SendOKResponse()
{
    QTSS_Error theErr = QTSS_NoErr;
    if (GetStreamRef() == 0)
    {
        Assert(0);
        return theErr;
    }
    theErr = QTSS_Write(GetStreamRef(), kOKHeader, ::strlen(kOKHeader), NULL, qtssWriteFlagsBufferData);
    if (theErr != QTSS_NoErr)
    {
        return theErr;
    }
    theErr = QTSS_Flush(GetStreamRef());
    Assert(theErr == QTSS_NoErr);
    return theErr;
}

// Read the broadcast headers
QTSS_Error MP3BroadcasterSession::GetBroadcastHeaders()
{
    QTSS_Error theErr = QTSS_NoErr;
    UInt32 theLen = 0;
    
    Assert(GetStreamRef() != 0);
    
    ::memset(fHeader, 0, kHeaderBufferSize);
    theErr = QTSS_Read(GetStreamRef(), fHeader, kHeaderBufferSize, &theLen);
    if (theErr == QTSS_WouldBlock)
    {
        // we're blocked from reading the broadcast headers so we will
        // schedule a read so we can try again later.
        theErr = QTSS_RequestEvent(GetStreamRef(), QTSS_ReadableEvent);
        Assert(theErr == QTSS_NoErr);
    }
    else if (theErr == QTSS_NoErr)
    {
        // The read of the headers succeeded
        TerminateHeaders();
        // parse headers here...
        if (strncmp(fHeader, "icy-name", 8) != 0)
            AcceptDataStreamState();
        // Send another OK response.
        SendOKResponse();
        // schedule the first read of the data stream.
        theErr = QTSS_RequestEvent(GetStreamRef(), QTSS_ReadableEvent);
        Assert(theErr == QTSS_NoErr);
    }
    else
    {
        // The read failed with an error other than QTSS_WouldBlock
        DTRACE1("MP3BroadcasterSession::GetBroadcastHeaders() - read failed with err = %" _S32BITARG_ "\n", theErr);
        Assert(0);
    }
    return theErr;
}

// Read the broadcast data stream
QTSS_Error MP3BroadcasterSession::GetBroadcastData()
{
    QTSS_Error theErr = QTSS_NoErr;

    if (GetStreamRef() == 0)
    {
        Assert(0);
        return theErr;
    }
    theErr = QTSS_Read(GetStreamRef(), fBuffer, fDataBufferSize, &fDataBufferLen);
    if (theErr == QTSS_WouldBlock)
    {
        // we're blocked from reading the broadcast headers so we will
        // schedule a read so we can try again later.
        theErr = QTSS_RequestEvent(GetStreamRef(), QTSS_ReadableEvent);
        Assert(theErr == QTSS_NoErr);
    }
    else if (theErr == QTSS_NoErr)
    {
        // The read of the broadcast stream succeeded       
        // Send all our clients a copy of the data
        theErr = SendClientsData();
        
        // Set up to read the next chunk of incoming broadcast data
        // If we didn't do this we'd exit the kFilterRequest state after returning.
        // By doing this we get called back when more data arrives.
        theErr = QTSS_RequestEvent(GetStreamRef(), QTSS_ReadableEvent);
        Assert(theErr == QTSS_NoErr);
    }
    else if (theErr == QTSS_NotConnected)
    {
        DTRACE("MP3BroadcasterSession::GetBroadcastData() - read failed with err = QTSS_NotConnected\n");
        ShutDownState();
    }
    else
    {
        DTRACE1("MP3BroadcasterSession::GetBroadcastData() - read failed with err = %" _S32BITARG_ "\n", theErr);
        Assert(0);
        ShutDownState();
    }
    return theErr;
}

// Send the broadcast data stream to all our clients
QTSS_Error MP3BroadcasterSession::SendClientsData()
{
    QTSS_Error theErr = QTSS_NoErr;

    if (fMP3ClientQueue == 0)
    {
        return theErr;
    }
    if (fDataBufferLen > 0)
    {
        // Make sure all queued client sessions have the
        // current song name
        PreflightClients();
        
        // Send MP3 data to everything in our client queue.
        
        theErr = fMP3ClientQueue->SendToAllClients(fBuffer, fDataBufferLen);
    }
    return theErr;
}

// Update all of our queued clients with the currently
// active song name
void MP3BroadcasterSession::PreflightClients()
{
    if (fMP3ClientQueue == 0)
    {
        return;
    }
    if (fNewSongName)
    {
        OSMutexLocker locker(&fSongNameMutex);
        // Copy new song name to all queue MP3ClientSessions.
        // and attempt to resend data that was previously blocked.
        fMP3ClientQueue->PreflightClients(GetSongName());
        fNewSongName = false;
    }
}

// Search through the audiocast headers and terminate them with a '\0'
// to mark the end.
void MP3BroadcasterSession::TerminateHeaders()
{
    char *bp = fHeader;
    char *end = fHeader + kHeaderBufferSize;
    while (bp < end)
    {
        // scan for newlines until end of buffer.
        if (*bp++ == '\n')
        {
            // if the next line after newline starts with
            // 'x' or 'i' we assume it's an 'x-audiocast-xxx' line
            // or an 'icy-xxxx' line, otherwise we're done.
            if (*bp != 'x' && *bp != 'i')
                break;
        }
    }
    // make sure we didn't go past the end of buffer.
    if (bp > end)
        bp = end;
    // terminate the buffer.
    *bp = '\0';
}

// ****************************************************************************
// MP3ClientSession -- This is a class to hold all the MP3Client state info.
// ****************************************************************************
MP3ClientSession::MP3ClientSession()
{
    Assert(0);
    // this should never be called.
}

MP3ClientSession::MP3ClientSession( QTSS_RTSPSessionObject sess, 
                                        QTSS_StreamRef stream,
                                        MP3BroadcasterSession* owner) : 
    MP3Session(sess, stream),
    fBytesSent(0),
    fCurrentBitRate(0),
    fLastBitRateBytes(0),
    fLastBitRateUpdateTime(0),
        fConnectTime(0),
    fSendCount(0),
    fRetryCount(0),
    fOwner(owner),
    fNewSongName(false),
    fWasBlocked(false),
    fBlockTime(0),
        fNeedsContentLength(false),
    fWantsMetaData(true)
{
    QTSS_Error err = QTSS_NoErr;
    const char* kType = "MP3 Client";
    
    ::memset(fSongName, 0, kSongNameBufferSize);
    ::memset(fHeader, 0, kHeaderBufferSize);
    ::memset(fHostName, 0, kHostNameBufferSize);
    ::memset(fUserAgent, 0, kUserAgentBufferSize);
    ::memset(fRequestBuffer, 0, kRequestBufferSize);
    
    ParseRequestParams(stream);
    
    QTSS_LockObject(sServer);
    UInt32 index;
    UInt32 tempInt;
    char tempBuffer[1024];
    UInt32 len;
    QTSS_CreateObjectValue(sServer, qtssSvrConnectedUsers, qtssConnectedUserObjectType, &index, &fQTSSObject);
    QTSS_SetValue(fQTSSObject, qtssConnectionType, 0, kType, strlen(kType) + 1);
    if (fHostName[0] != '\0')
        QTSS_SetValue(fQTSSObject, qtssConnectionHostName, 0, fHostName, ::strlen(fHostName) + 1);
    QTSS_SetValuePtr(fQTSSObject, qtssConnectionBytesSent, &fBytesSent, sizeof(fBytesSent));
    QTSS_SetValuePtr(fQTSSObject, qtssConnectionCurrentBitRate, &fCurrentBitRate, sizeof(fBytesSent));
    tempInt = 0;
    QTSS_SetValue(fQTSSObject, qtssConnectionPacketLossPercent, 0, &tempInt, sizeof(tempInt));
    fConnectTime = OS::Milliseconds();
    QTSS_SetValue(fQTSSObject, qtssConnectionCreateTimeInMsec, 0, &fConnectTime, sizeof(fConnectTime));

    len = sizeof(tempBuffer);
    err = QTSS_GetValue(sess, qtssRTSPSesLocalAddrStr, 0, tempBuffer, &len);
    if (err == QTSS_NoErr)
        (void)QTSS_SetValue(fQTSSObject, qtssConnectionSessLocalAddrStr, 0, tempBuffer, len);

    len = sizeof(tempBuffer);
    err = QTSS_GetValue(sess, qtssRTSPSesRemoteAddrStr, 0, tempBuffer, &len);
    if (err == QTSS_NoErr)
        (void)QTSS_SetValue(fQTSSObject, qtssConnectionSessRemoteAddrStr, 0, tempBuffer, len);

    char* mountpoint = owner->GetMountpoint();
    (void)QTSS_SetValue(fQTSSObject, qtssConnectionMountPoint, 0, mountpoint, strlen(mountpoint) + 1);
    
    QTSS_UnlockObject(sServer);
            
    // Increment the server's MP3 session count.
    IncrementMP3SessionCount();
}

MP3ClientSession::~MP3ClientSession()
{
    SetState(MP3ClientSession::kClientShutDownState);

    QTSS_Object sessionObject;
    UInt32 len = sizeof(QTSS_Object);

    QTSS_LockObject(sServer);
    for (int x = 0; QTSS_GetValue(sServer, qtssSvrConnectedUsers, x, &sessionObject, &len) == QTSS_NoErr; x++)
    {
        Assert(sessionObject != NULL);
        Assert(len == sizeof(QTSS_Object));

        if (sessionObject == fQTSSObject)
        {
            (void)QTSS_RemoveValue(sServer, qtssSvrConnectedUsers, x);
            break;
        }
    }
    QTSS_UnlockObject(sServer);
    // Decrease the server's count of MP3 sessions by one.
    DecrementMP3SessionCount();
}

UInt8 MP3ClientSession::IsA() const
{
    return kMP3ClientSessionType;
}

// Send the client it HTTP response
QTSS_Error MP3ClientSession::SendResponse()
{
    QTSS_Error theErr = QTSS_NoErr;
    UInt32 len = 0;
    char buffer[512];
    
    if (GetState() == MP3ClientSession::kClientInitState)
    {
        //
        // Note: We don't Parse any client request HTTP parms here...
        //
        SetState(MP3ClientSession::kClientSendResponse);
        theErr = QTSS_Write(GetStreamRef(), gClientAcceptHeader, ::strlen(gClientAcceptHeader), NULL, qtssWriteFlagsBufferData);        
        if (theErr != QTSS_NoErr)
        {
            SetState(MP3ClientSession::kClientShutDownState);
            return theErr;
        }
        // x-audiocast header parameters are added and sent here
        if (fWantsMetaData)
        {
            // add the "icy-metaint:xxxx" parameter to the header.
            qtss_sprintf(buffer, "icy-metaint:%d\r\n", kClientMetaInt);
            ::strcat(fHeader, buffer);
        }
        len = ::strlen(fHeader);
        if (len > 0)
        {
            theErr = QTSS_Write(GetStreamRef(), fHeader, len, NULL, qtssWriteFlagsBufferData);      
            if (theErr != QTSS_NoErr)
            {
                SetState(MP3ClientSession::kClientShutDownState);
                return theErr;
            }
        }
        // ...and send the phony content length or just a newline to mark end of header
                if (WantsContentLength())
                    theErr = QTSS_Write(GetStreamRef(), kClientCLHeader, ::strlen(kClientCLHeader), NULL, qtssWriteFlagsBufferData);
                else
                    theErr = QTSS_Write(GetStreamRef(), "\r\n", 2, NULL, qtssWriteFlagsBufferData);
        if (theErr != QTSS_NoErr)
        {
            Assert(0);
            SetState(MP3ClientSession::kClientShutDownState);
            return theErr;
        }
        else
        {
            SetState(MP3ClientSession::kClientSendDataState);
        }
        
        // 
        // Make sure the client session remains in FilterRequest server role.
        // We do this by scheduling ourself to be called back in 
        // kClientPollInterval milliseconds. When called back again we will
        // continue to schedule ourself until we get into another 
        //
        
        KeepSession((QTSS_RTSPRequestObject) GetStreamRef(), true);
        theErr = QTSS_SetIdleTimer(kClientPollInterval);
        if (theErr != QTSS_NoErr)
        {
            Assert(0);
            SetState(MP3ClientSession::kClientShutDownState);
        }
    }
    return theErr;
}

// Send the broadcast data stream to our client including meta-data
// if needed.
QTSS_Error MP3ClientSession::SendMP3Data(char* buffer, UInt32 bufferlen)
{
    QTSS_Error theErr = QTSS_NoErr;
    UInt32 sendcount;
    UInt32 sendlen = 0L;
    UInt32 remaininglen = 0L;
    
    // get the broadcast sendcount into our local copy.
    sendcount = fSendCount;
    Assert(buffer != NULL);
    // We only send data if we are int the kClientSendDataState.
    if (GetState() != MP3ClientSession::kClientSendDataState)
    {
        return theErr;
    }
    // If we are using meta data then check and see if we hit our 
    // meta-data send interval.
    // if so calculate the second write size.
    if (fWantsMetaData && ((sendcount + bufferlen) > kClientMetaInt))
    {
        // sendlen is the amount of buffered data to send before meta data
        // is interposed.
        sendlen = (UInt32) kClientMetaInt - sendcount;
        // remainglen is the reaming amount of buffered data to send
        // after sending the meta-data.
        remaininglen = bufferlen - sendlen;
    }
    else
    {
        // either it's not time to send metadata or we are not 
        // doing meta-data for this client.
        sendlen = bufferlen;
        remaininglen = 0L;
    }
    // Send up to sendlen bytes of data from the buffer.
    QTSS_StreamRef stream = GetStreamRef();
    
    Assert(stream != NULL);
    
    Assert(sendlen <= bufferlen);
    
    theErr = QTSS_Write(stream, buffer, sendlen, NULL, qtssWriteFlagsBufferData);
    sendcount += sendlen;
    
    if (theErr != QTSS_NoErr)
    {
        SetState(MP3ClientSession::kClientShutDownState);
        return theErr;
    }
    // If there is meta-data this time then send it followed by remaininglen bytes
    // from the buffer.
    if (remaininglen != 0L)
    {
        // send the meta-data
        theErr = SendMetaData();
        if (theErr != QTSS_NoErr)
        {
            SetState(MP3ClientSession::kClientShutDownState);
            return theErr;
        }
        // sendcount must be zero after sending meta-data.
        sendcount = 0;
        // send the remainder of the data buffer.
        theErr = QTSS_Write(GetStreamRef(), buffer+sendlen, remaininglen, NULL, qtssWriteFlagsBufferData);
        sendcount += remaininglen;
    
        if (theErr != QTSS_NoErr)
        {
            SetState(MP3ClientSession::kClientShutDownState);
            return theErr;
        }
    }
    fSendCount = sendcount;
    // Flush to perform the actuall send to the client.
    theErr = QTSS_Flush(GetStreamRef());
    if (theErr == QTSS_WouldBlock)
    {
        // the client is flow controlled.
        SInt64 curTime = OS::Milliseconds();
        fWasBlocked = true;
        QTSS_RequestEvent(GetStreamRef(), QTSS_WriteableEvent);
                DTRACE2("Got blocked at time %qd, numRetries = %" _S32BITARG_ "\n", curTime, fRetryCount);
        if (fBlockTime == 0)
            fBlockTime = curTime;
        fRetryCount++;
        if (curTime - fBlockTime > sMaxFlowControlTimeInMSec)
        {
            SetResult(453);
                        DTRACE1("MP3ClientSession::SendMP3Data - too many tries. Terminating client session = %" _S32BITARG_ "\n", GetSessionID());
            SetState(MP3ClientSession::kClientShutDownState);
        }
    }
    else if (theErr != QTSS_NoErr)
    {
                DTRACE1("MP3ClientSession::SendMP3Data - Terminating client session = %" _S32BITARG_ "\n", GetSessionID());
        SetState(MP3ClientSession::kClientShutDownState);
    }
    else
    {
        // we successfully sent data to the client.
        // fSendCount only gets incremented if the QTSS_Flush() was successful
        SetResult(200);
        fWasBlocked = false;
        fBlockTime = 0;
        fRetryCount = 0;
    }
    
    fBytesSent += bufferlen;
    // Increase the server's total MP3 byte count attribute
    IncrementTotalMP3Bytes(bufferlen);
    UpdateBitRateInternal(OS::Milliseconds());
    
    return theErr;
}

// Retry to send previously blocked data.
QTSS_Error MP3ClientSession::RetrySendData()
{
    QTSS_Error theErr = QTSS_NoErr;
    
    // Flush to perform the actuall send to the client.
    theErr = QTSS_Flush(GetStreamRef());
    if (theErr == QTSS_WouldBlock)
    {
        // the client is still flow controlled.
        fWasBlocked = true;
        QTSS_RequestEvent(GetStreamRef(), QTSS_WriteableEvent);
        return theErr;
    }
    else if (theErr != QTSS_NoErr)
    {
        // some error other than QTSS_WouldBlock occured.
        SetResult(454);
        SetState(MP3ClientSession::kClientShutDownState);
        return theErr;
    }
    else
    {
        // we successfully sent data to the client.
        // cancel future retries
        fWasBlocked = false;
        SetResult(200);
        fRetryCount = 0;
    }
    return QTSS_NoErr;
}

// Send the broadcast meta data stream to our client
QTSS_Error MP3ClientSession::SendMetaData()
{
    QTSS_Error theErr = QTSS_NoErr;
    char buffer[1024];
    UInt16 bufferlen;
    
    // Don't allow an update of the song name while we're sending
    // it to the client.
    OSMutexLocker locker(&fSongNameMutex);
    // format the meta-data
    ::memset(buffer, 0, 1024);
    // Make sure we have something to send
    if (!fNewSongName || fSongName[0] == '\0')
    {
        // setup to write a single NULL byte.
        bufferlen = 1;
    }
    else
    {
        // Setup to write meta-data + pad NULL bytes.
        // first value in the buffer is the number of 16 byte chunks
        // to send.
        char tmp[512];
        qtss_sprintf(tmp, "StreamTitle='%s';StreamUrl='';", fSongName);
        bufferlen = ::strlen(tmp);
        buffer[0] = (unsigned char)((bufferlen/16) + 1);
        ::strcat(buffer, tmp);
        bufferlen = (buffer[0]*16) + 1;
    }
    // send the meta-data
    theErr = QTSS_Write(GetStreamRef(), buffer, bufferlen, NULL, qtssWriteFlagsBufferData);
    
    if (theErr == QTSS_NoErr)
    {
        // meta data was written successfully
        fNewSongName = false;
    }
    fBytesSent += bufferlen;
    return theErr;
}

// Set the x-audiocast headers of the currently broadcasting client.
// Lines that end in '\n' will have a '\r' prepended.
void MP3ClientSession::SetHeader(char* header)
{
    char *bp = header;
    char *cp = fHeader;
    char *end = fHeader + kHeaderBufferSize;
    while (cp < end)
    {
        if (*bp == '\0')
        {
            // end of header buffer string
            break;
        }
        // scan for newlines .
        if (*bp == '\n')
        {
            // add a CR before the newline
            *cp = '\r';
            cp++;
            *cp = '\n';
        }
        else
        {
            // otherwise just copy the byte
            *cp = *bp;
        }
        cp++;
        bp++;
    }
    // make sure we didn't go past the end of buffer.
    if (cp > end)
        cp = end;
    // terminate the buffer.
    *cp = '\0';
}

// Set the song name of the currently broadcasting client.
void MP3ClientSession::SetSongName(char* sn)
{
    if (sn != NULL && *sn != '\0')
    {
        // This mutex is to guard against updating a
        // song name string while we're trying to send it
        // to a remote client.
        OSMutexLocker locker(&fSongNameMutex);
        ::strcpy(fSongName, sn);
        fNewSongName = true;
    }
}

void MP3ClientSession::UpdateBitRateInternal(const SInt64& curTime)
{   
    if (curTime < fLastBitRateUpdateTime + 10000)
        return;     // update the bitrate every 10 seconds
    
    UInt32 bitsInInterval = (fBytesSent - fLastBitRateBytes) * 8;
    SInt64 updateTime = (curTime - fLastBitRateUpdateTime) / 1000;
    if (updateTime > 0) // leave Bit Rate the same if updateTime is 0 also don't divide by 0.
        fCurrentBitRate = (UInt32) ( bitsInInterval / updateTime );
    fLastBitRateBytes = fBytesSent;
    fLastBitRateUpdateTime = curTime;
}

void MP3ClientSession::ParseRequestParams(QTSS_StreamRef stream)
{   
    QTSS_Error err = QTSS_NoErr;
    char *bp = NULL;
    unsigned int idx;
    // get a copy of the first line of the HTTP request
    QTSS_RTSPRequestObject theRequest = (QTSS_RTSPRequestObject) stream;
    StrPtrLen theFullRequest;
    if (theRequest != NULL)
        err = QTSS_GetValuePtr(theRequest, qtssRTSPReqFullRequest, 0, (void**)&theFullRequest.Ptr, &theFullRequest.Len);
    else
        err = QTSS_BadArgument;
    if (err == QTSS_NoErr)
    {
                // make sure we don't overflow our buffer
                if (theFullRequest.Len >= kRequestBufferSize)
                    theFullRequest.Len = kRequestBufferSize-1;
                // copy the request into our private class buffer.
        ::memcpy(fRequestBuffer, theFullRequest.Ptr, theFullRequest.Len);
        // first see if the client wants to recieve meta-data
        fWantsMetaData = NeedsMetaData(theFullRequest);

                // Do we need to send a phony HTTP content length field?
        if (::strstr(fRequestBuffer, "MSIE") != NULL)
                    fNeedsContentLength = true;
        else if (::strstr(fRequestBuffer, "RMA/1.0") != NULL)
                    fNeedsContentLength = true;
        else if (::strstr(fRequestBuffer, "NSPlayer") != NULL)
                    fNeedsContentLength = true;
        else 
                    fNeedsContentLength = false;
        // Get a copy ot the requestor's host name if it exists
        bp = ::strstr(fRequestBuffer, "Host:");
        if (bp != NULL)
        {
            bp += ::strlen("Host:");
            while (*bp == ' ')
                bp++;
            idx = 0;
            while (bp[idx])
            {
                if (idx >= kHostNameBufferSize-1)
                    break;
                if (bp[idx] == '\n' || bp[idx] == '\r')
                    break;
                fHostName[idx] = bp[idx];
                idx++;
            }
        }
        // Get a copy ot the requesting User Agent's name if it exists
        bp = ::strstr(fRequestBuffer, "User-Agent:");
        if (bp != NULL)
        {
            bp += ::strlen("User-Agent:");
            while (*bp == ' ')
                bp++;
            idx = 0;
            while (bp[idx])
            {
                if (idx >= kUserAgentBufferSize-1)
                    break;
                if (bp[idx] == '\n' || bp[idx] == '\r')
                    break;
                fUserAgent[idx] = bp[idx];
                idx++;
            }
        }
        // Now that we're done parsing the request we will
        // terminate first line of the request with '\0'.
        for (idx=0; idx < theFullRequest.Len; idx++)
        {
            if (fRequestBuffer[idx] == '\r' || fRequestBuffer[idx] == '\n')
            {
                fRequestBuffer[idx] = '\0';
                break;
            }
        }
    }
    else
    {
        // dummy value so that we have something to log 
        ::strcpy(fRequestBuffer, "GET /* HTTP/1.0");
    }
}

// ****************************************************************************
// MP3SessionRef -- This class is just a wrapper class for handling the
// mapping of RTSP Session refs to the corresponding MP3 Session class refs.
// It will be the an element of our lookup hash table.
// ****************************************************************************
MP3SessionRef::MP3SessionRef(MP3Session* mp3Session) : 
    fNextHashEntry(NULL),
    fHashValue(0),
    fMP3Session(mp3Session)
{
}

MP3SessionRef::~MP3SessionRef()
{
}

// ****************************************************************************
// MP3SessionRefKey -- This class is used to generate hash keys for looking
// up values in our MP3SessionTable. The Hash key is just the RTSP Session
// reference value.
// ****************************************************************************
MP3SessionRefKey::MP3SessionRefKey(MP3SessionRef* mp3SessRef) : 
    fKeyValue(mp3SessRef)
{
    fHashValue = 0L;
    if (fKeyValue != NULL)
    {
        fMP3Session = fKeyValue->GetMP3Session();
        if (fMP3Session != NULL)
            fHashValue = (PointerSizedInt)fMP3Session->GetSession();
    }
    else
    {
        fMP3Session = NULL;
    }
}

MP3SessionRefKey::MP3SessionRefKey(QTSS_RTSPSessionObject rtspSessRef) : 
    fKeyValue(NULL),
    fHashValue((PointerSizedInt)rtspSessRef),
    fMP3Session(NULL)
{
}

MP3SessionRefKey::~MP3SessionRefKey()
{
}

// ****************************************************************************
// MP3SessionTable -- This class provides a way to map RTSP Session references
// into the corresponding MP3Session class instances if any.
// ****************************************************************************
MP3SessionTable::MP3SessionTable(UInt32 tableSize) : 
    fTable(tableSize),
    fMutex()
{
}

MP3SessionTable::~MP3SessionTable()
{
}

// Attempt to add a new MP3Session ref to the table's map.
// returns true on success or false if it fails.
Bool16 MP3SessionTable::RegisterSession(MP3Session* session)
{
    // sanity check
    if (session == NULL)
    {
        Assert(0);
        return false;
    }
    // construct a new entry for the table.
    MP3SessionRef* newEntry = NEW MP3SessionRef(session);
    if (newEntry == NULL)
    {
        return false;
    }
    // generate it's hash key.
    MP3SessionRefKey key(newEntry);
    // Lock the table while we try and add the new entry.
    OSMutexLocker locker(&fMutex);
    // check for a duplicate entry;
    MP3SessionRef* duplicateRef = fTable.Map(&key);
    if (duplicateRef != NULL)
    {
        delete newEntry;
        Assert(0);
        return false;
    }
    // add the new entry to the table.
    fTable.Add(newEntry);
    return true;
}

// Given an QTSS_RTSPSessionObject resolve it into a MP3Session class
// reference. Returns NULL if there's none in out map.
MP3Session* MP3SessionTable::Resolve(QTSS_RTSPSessionObject rtspSession)
{
    // sanity check
    if (rtspSession == NULL)
    {
        Assert(0);
        return NULL;
    }
    // generate it's hash key.
    MP3SessionRefKey key(rtspSession);
    // Lock the table while we do our lookup.
    OSMutexLocker locker(&fMutex);
    // Look for the entry in the table's map;
    MP3SessionRef* entry = fTable.Map(&key);
    if (entry != NULL)
    {
        // We found it. return the entry's MP3Session ref.
        return entry->GetMP3Session();
    }
    // it's not in the table.
    return NULL;
}

// Attempt to remove a  MP3Session ref from the table's map.
// returns true on success or false if it fails.
Bool16 MP3SessionTable::UnRegisterSession(MP3Session* session)
{
    // sanity check
    if (session == NULL)
    {
        Assert(0);
        return false;
    }
    // generate it's hash key.
    MP3SessionRef entryRef(session);
    MP3SessionRefKey key(&entryRef);
    // Lock the table while we try and remove the entry.
    OSMutexLocker locker(&fMutex);
    // look for the entry in the table's map;
    MP3SessionRef* entry = fTable.Map(&key);
    if (entry != NULL)
    {
        // We found it. Remove it from the table and delete the 
        // reference instance created by RegisterSession().
        fTable.Remove(entry);
        delete entry;
        return true;
    }
    // it's not in the table.
    return false;
}

// ****************************************************************************
// MP3BroadcasterQueue -- This is a class manages a queue of MP3Broadcaster 
// objects.
// ****************************************************************************
MP3BroadcasterQueue::MP3BroadcasterQueue()
{
}

MP3BroadcasterQueue::~MP3BroadcasterQueue()
{
}

// Create a new MP3BroadcasterSession object and queue it.
QTSS_Error MP3BroadcasterQueue::CreateBroadcaster(QTSS_RTSPSessionObject sess, QTSS_StreamRef stream, StrPtrLen& mountpt)
{
    QTSS_Error theErr = QTSS_NoErr;
    UInt32 uSessID = 0L;
    
    if (sess == 0 || stream == 0)
    {
        return QTSS_BadArgument;
    }
    
    if ((fQueue.GetLength() > 0) && (FindByMountPoint(mountpt) != NULL))
    {
            theErr = QTSS_Write(stream, kSourceReject, ::strlen(kSourceReject), NULL, qtssWriteFlagsBufferData);
            if (theErr != QTSS_NoErr)
            {
        return theErr;
            }
            theErr = QTSS_Flush(stream);
            DTRACE("MP3BroadcasterQueue::CreateBroadcaster - duplicate mountpoint\n");
            return QTSS_BadArgument;
    }
    
    if ( (fQueue.GetLength() > 0) &&  InQueue(sess) )
    {
            theErr = QTSS_Write(stream, kSourceBadPassword, ::strlen(kSourceBadPassword), NULL, qtssWriteFlagsBufferData);
            if (theErr != QTSS_NoErr)
            {
        return theErr;
            }
            theErr = QTSS_Flush(stream);
            DTRACE("MP3BroadcasterQueue::CreateBroadcaster - duplicate session\n");
            return QTSS_BadArgument;
    }
    
    MP3BroadcasterSession* broadcaster = NEW MP3BroadcasterSession(sess, stream);
    if (broadcaster == NULL)
    {
            return QTSS_NotEnoughSpace;
    }
    uSessID = GetRTSPSessionID(sess);
    broadcaster->SetSessionID(uSessID);
    broadcaster->SetMountpoint(mountpt);
    broadcaster->AcceptPasswordState(); 
    OSQueueElem* elem = NEW OSQueueElem(broadcaster);
    if (elem != 0)
    {
        OSMutexLocker locker(&fMutex);
        fQueue.EnQueue(elem);
        DTRACE1("MP3BroadcasterQueue::CreateBroadcaster() Session(%" _S32BITARG_ ") broadcaster added!\n", uSessID);
        // send the "OK" back to the broadcaster
        theErr = broadcaster->SendOKResponse();
        // If we failed to send the "OK" probably becuase of flow control
        // the only thing to do is to fail the attempt.
        if (theErr != QTSS_NoErr)
            broadcaster->ShutDownState();
    }
    else
    {
        delete broadcaster;
        broadcaster = NULL;
        theErr = QTSS_NotEnoughSpace;
    }
    return theErr;
}

// Dequeue a MP3BroadcasterSession object and delete it.
QTSS_Error MP3BroadcasterQueue::RemoveBroadcaster(QTSS_RTSPSessionObject sess)
{
    UInt32 uSessID = 0L;
    if (sess == 0)
    {
        return QTSS_BadArgument;
    }
    OSMutexLocker locker(&fMutex);
    OSQueueIter iter(&fQueue);
    OSQueueElem* elem = NULL;
    while ((elem = iter.GetCurrent()) != 0)
    {
        MP3BroadcasterSession* current = (MP3BroadcasterSession*) elem->GetEnclosingObject();
        if ((current != 0) && (current->GetSession() == sess))
        {
            elem->Remove();
            uSessID = current->GetSessionID();
            DTRACE1("MP3BroadcasterQueue::RemoveBroadcaster() Session(%" _S32BITARG_ ") broadcaster removed!\n", uSessID);
            delete elem;
            delete current;
            return QTSS_NoErr;
        }
        iter.Next();
    }
    return QTSS_BadArgument;
}

// Find and dequeue a MP3ClientSession object and delete it.
QTSS_Error MP3BroadcasterQueue::RemoveClient(QTSS_RTSPSessionObject sess)
{
    if (sess == 0)
    {
        return QTSS_BadArgument;
    }
    OSMutexLocker locker(&fMutex);
    OSQueueIter iter(&fQueue);
    OSQueueElem* elem = NULL;
    while ((elem = iter.GetCurrent()) != 0)
    {
        MP3BroadcasterSession* current = (MP3BroadcasterSession*) elem->GetEnclosingObject();
        if ((current != 0) && (current->IsMyClient(sess)))
        {
            return current->RemoveClient(sess);
        }
        iter.Next();
    }
    return QTSS_BadArgument;
}

    
    
// See if a particular Broadcaster RTSP session is in our queue.
Bool16 MP3BroadcasterQueue::InQueue(QTSS_RTSPSessionObject sess)
{
    if (sess == 0)
    {
        return false;
    }
    OSMutexLocker locker(&fMutex);
    OSQueueIter iter(&fQueue);
    OSQueueElem* elem = NULL;
    while ((elem = iter.GetCurrent()) != 0)
    {
        MP3BroadcasterSession* current = (MP3BroadcasterSession*) elem->GetEnclosingObject();
        if ((current != 0) && (current->GetSession() == sess))
        {
            return true;
        }
        iter.Next();
    }
    return false;
}


// See if a particular Client RTSP session is in any of our broadcasters' queues.
Bool16 MP3BroadcasterQueue::IsActiveClient(QTSS_RTSPSessionObject sess)
{
    if (sess == 0)
    {
        return false;
    }
    OSMutexLocker locker(&fMutex);
    OSQueueIter iter(&fQueue);
    OSQueueElem* elem = NULL;
    while ((elem = iter.GetCurrent()) != 0)
    {
        MP3BroadcasterSession* current = (MP3BroadcasterSession*) elem->GetEnclosingObject();
        if ((current != 0) && (current->IsMyClient(sess)))
        {
            return true;
        }
        iter.Next();
    }
    return false;
}


// Find a MP3BroadcasterSession in the queue by it's mountpoint.
MP3BroadcasterSession* MP3BroadcasterQueue::FindByMountPoint(char* mountpoint)
{
    if (mountpoint == NULL || mountpoint[0] == '\0')
    {
        return NULL;
    }
    OSMutexLocker locker(&fMutex);
    OSQueueIter iter(&fQueue);
    OSQueueElem* elem = NULL;
    while ((elem = iter.GetCurrent()) != 0)
    {
        MP3BroadcasterSession* current = (MP3BroadcasterSession*) elem->GetEnclosingObject();
        if ((current != 0) && (current->MountpointEqual(mountpoint)))
        {
            return current;
        }
        iter.Next();
    }
    return NULL;
}

// Find a MP3BroadcasterSession in the queue by it's mountpoint.
MP3BroadcasterSession* MP3BroadcasterQueue::FindByMountPoint(StrPtrLen& mountpoint)
{
    if (mountpoint.Ptr == NULL || mountpoint.Len == 0)
    {
        return NULL;
    }
    OSMutexLocker locker(&fMutex);
    OSQueueIter iter(&fQueue);
    OSQueueElem* elem = NULL;
    while ((elem = iter.GetCurrent()) != 0)
    {
        MP3BroadcasterSession* current = (MP3BroadcasterSession*) elem->GetEnclosingObject();
        if ((current != 0) && (current->MountpointEqual(mountpoint)))
        {
            return current;
        }
        iter.Next();
    }
    return NULL;
}

// Find a MP3BroadcasterSession in the queue by it's RTSP Session.
MP3BroadcasterSession* MP3BroadcasterQueue::FindBySession(QTSS_RTSPSessionObject sess)
{
    if (sess == 0)
    {
        return NULL;
    }
    OSMutexLocker locker(&fMutex);
    OSQueueIter iter(&fQueue);
    OSQueueElem* elem = NULL;
    while ((elem = iter.GetCurrent()) != 0)
    {
        MP3BroadcasterSession* current = (MP3BroadcasterSession*) elem->GetEnclosingObject();
        if ((current != 0) && (current->GetSession() == sess))
        {
            return current;
        }
        iter.Next();
    }
    return NULL;
}

    
// Mark all broadcast sessions to be terminated when they are next called.
void MP3BroadcasterQueue::TerminateAllBroadcastSessions()
{
    OSMutexLocker locker(&fMutex);
    OSQueueElem* elem = NULL;
    OSQueueIter iter(&fQueue);
    
    while ((elem = iter.GetCurrent()) != NULL)
    {
        MP3BroadcasterSession* current = (MP3BroadcasterSession*) elem->GetEnclosingObject();
        if (current != NULL)
        {
            current->SetState(MP3BroadcasterSession::kBroadcasterShutDownState);
        }
        iter.Next();
    }
}

// ****************************************************************************
// MP3ClientQueue -- This is a class maintains a queue of MP3Client objects.
// ****************************************************************************
MP3ClientQueue::MP3ClientQueue()
{
}

MP3ClientQueue::~MP3ClientQueue()
{
    // Make sure all queued client sessions are marked for termination since
    // since they are now orphaned and no longer have an owner.
    TerminateClients();
}

// Create a new MP3ClientSession object and queue it.
QTSS_Error MP3ClientQueue::AddClient(QTSS_RTSPSessionObject sess, QTSS_StreamRef stream,
                                        MP3BroadcasterSession* owner)
{
    QTSS_Error theErr = QTSS_NoErr;
    UInt32 uSessID = 0L;
    
    if (sess == 0 || stream == 0)
    {
        return QTSS_BadArgument;
    }
    
    if ( InQueue (sess) )
    {
        return QTSS_NoErr;
    }
    
    MP3ClientSession* client = NEW MP3ClientSession(sess, stream, owner);
    if (client == 0)
    {
        return QTSS_NotEnoughSpace;
    }
    uSessID = GetRTSPSessionID(sess);
    client->SetSessionID(uSessID);
    if (owner != NULL)
    {
        // Get a copy of the x-audiocast headers from the
        // broadcaster's cached copy.
        client->SetHeader(owner->GetHeader());
        // Make sure we have a song name also
        client->SetSongName(owner->GetSongName());
    }
    OSQueueElem* elem = NEW OSQueueElem(client);
    if (elem != 0)
    {
        OSMutexLocker locker(&fMutex);
        fQueue.EnQueue(elem);
        DTRACE1("MP3ClientQueue::AddClient() Session(%" _S32BITARG_ ") client added!\n", uSessID);
        // send the "OK" back to the broadcaster
        theErr = client->SendResponse();
    }
    else
    {
        delete client;
        theErr = QTSS_NotEnoughSpace;
    }
    return theErr;
}

// Dequeue a MP3ClientSession object and delete it.
QTSS_Error MP3ClientQueue::RemoveClient(QTSS_RTSPSessionObject sess)
{
    UInt32 uSessID = 0L;
    if (sess == 0)
    {
        return QTSS_BadArgument;
    }
    OSMutexLocker locker(&fMutex);
    OSQueueIter iter(&fQueue);
    OSQueueElem* elem = NULL;
    while ((elem = iter.GetCurrent()) != 0)
    {
        MP3ClientSession* curClient = (MP3ClientSession*) elem->GetEnclosingObject();
        if ((curClient != 0) && (curClient->GetSession() == sess))
        {
            elem->Remove();
            uSessID = curClient->GetSessionID();
            DTRACE1("MP3ClientQueue::RemoveClient() Session(%" _S32BITARG_ ") client removed!\n", uSessID);
            delete elem;
            delete curClient;
            return QTSS_NoErr;
        }
        iter.Next();
    }
    return QTSS_BadArgument;
}

    
// See if a particular Client RTSP session is in our queue.
Bool16 MP3ClientQueue::InQueue(QTSS_RTSPSessionObject sess)
{
    if (sess == 0)
    {
        return false;
    }
    OSMutexLocker locker(&fMutex);
    OSQueueIter iter(&fQueue);
    OSQueueElem* elem = NULL;
    while ((elem = iter.GetCurrent()) != 0)
    {
        MP3ClientSession* curClient = (MP3ClientSession*) elem->GetEnclosingObject();
        if ((curClient != 0) && (curClient->GetSession() == sess))
        {
            return true;
        }
        iter.Next();
    }
    return false;
}

    
// Send MP3 data to all queued MP3ClientSession objects.
QTSS_Error MP3ClientQueue::SendToAllClients(char* buffer, UInt32 bufferlen)
{
    QTSS_Error theErr = QTSS_NoErr;
    OSMutexLocker locker(&fMutex);
    OSQueueElem* elem = NULL;
    OSQueueIter iter(&fQueue);
    
    while ((elem = iter.GetCurrent()) != NULL)
    {
        MP3ClientSession* curClient = (MP3ClientSession*) elem->GetEnclosingObject();
        if (curClient != NULL)
        {
            QTSS_Error err;
            
            err = curClient->SendMP3Data(buffer, bufferlen);
            if (err != QTSS_NoErr)
                theErr = err;
        }
        iter.Next();
    }
    return theErr;
}

// Update the current song name of queued MP3ClientSession objects.
// and attempt to resend data thatwas previously blocked.
void MP3ClientQueue::PreflightClients(char* sn)
{
    QTSS_Error theErr = QTSS_NoErr;
    OSMutexLocker locker(&fMutex);
    OSQueueElem* elem = NULL;
    OSQueueIter iter(&fQueue);
    
    while ((elem = iter.GetCurrent()) != NULL)
    {
        MP3ClientSession* curClient = (MP3ClientSession*) elem->GetEnclosingObject();
        if (curClient != NULL)
        {
            if (curClient->WasBlocked())
            {
                theErr = curClient->RetrySendData();            
            }   
            curClient->SetSongName(sn);
        }
        iter.Next();
    }
}

// Mark all clients to be terminated when they are next polled.
// This should be called by the MP3BroadcasterSession whenever it dies
// with active client else we'll have a nasty memory leak and clients
// will pause until they timeout.
void MP3ClientQueue::TerminateClients()
{
    OSMutexLocker locker(&fMutex);
    OSQueueElem* elem = NULL;
    OSQueueIter iter(&fQueue);
    
    while ((elem = iter.GetCurrent()) != NULL)
    {
        MP3ClientSession* curClient = (MP3ClientSession*) elem->GetEnclosingObject();
        if (curClient != NULL)
        {
            curClient->SetOwner(NULL);
            curClient->SetState(MP3ClientSession::kClientShutDownState);
        }
        iter.Next();
    }
}

// ****************************************************************************
// MODULE FUNCTION IMPLEMENTATIONS
// ****************************************************************************
#if DEBUG_MP3STREAMING_MODULE
    void PrintStringBuffer(StrPtrLen& stringBuffer)
    {
        int len = (int)stringBuffer.Len;
        char localBuffer[256];
        
        if (len > 255)
            len = 255;
        ::memcpy(localBuffer, stringBuffer.Ptr, len);
        localBuffer[len] = '\0';
        qtss_printf("%s", localBuffer);
    }
#endif

inline void KeepSession(QTSS_RTSPRequestObject theRequest,Bool16 keep)
{
    (void)QTSS_SetValue(theRequest, qtssRTSPReqRespKeepAlive, 0, &keep, sizeof(keep));
}

QTSS_Error QTSSMP3StreamingModule_Main(void* inPrivateArgs)
{
    return _stublibrary_main(inPrivateArgs, QTSSMP3StreamingModuleDispatch);
}

// QTSSMP3StreamingModuleDispatch - Dispatch all the API roles that this module handles
QTSS_Error  QTSSMP3StreamingModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
    switch (inRole)
    {
        case QTSS_Register_Role:
            return Register(&inParams->regParams);
        case QTSS_Initialize_Role:
            return Initialize(&inParams->initParams);
        case QTSS_RTSPFilter_Role:
            return FilterRequest(&inParams->rtspFilterParams);
        case QTSS_RereadPrefs_Role:
            return RereadPrefs();
        case QTSS_RTSPSessionClosing_Role:
            return SessionClosing(&inParams->rtspSessionClosingParams);
        case QTSS_StateChange_Role:
            return StateChange(&inParams->stateChangeParams);
        case QTSS_Shutdown_Role:
            return Shutdown();
    }
    return QTSS_NoErr;
}

// Register - register all the API roles that this module handles
QTSS_Error Register(QTSS_Register_Params* inParams)
{
    // Do role setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_RereadPrefs_Role);
    (void)QTSS_AddRole(QTSS_RTSPFilter_Role);
    (void)QTSS_AddRole(QTSS_RTSPSessionClosing_Role);
    (void)QTSS_AddRole(QTSS_StateChange_Role);
    (void)QTSS_AddRole(QTSS_Shutdown_Role);
    
    // Tell the server our name!
    static char* sModuleName = "QTSSMP3StreamingModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    return QTSS_NoErr;
}

QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{
    // Setup module utils
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);
    sServer = inParams->inServer;
    sServerPrefs = inParams->inPrefs;
    sPrefs = QTSSModuleUtils::GetModulePrefsObject(inParams->inModule);

    RereadPrefs();

    sOSBufferPoolPtr = NEW OSBufferPool(sBroadcastBufferSize);
    sLogMutex = NEW OSMutex();
    sAtomicMutex = NEW OSMutex();
    
    // pre-format the standard HTTP reply headers
    qtss_sprintf(gClientAcceptHeader, kClientAcceptHeader, kVersionString, kBuildString);
    qtss_sprintf(gM3UReplyHeader, kM3UReplyHeader, kVersionString, kBuildString);
    
    sMP3AccessLog = NEW QTSSMP3AccessLog();
    
    if (sMP3AccessLog != NULL && sLogEnabled)
        sMP3AccessLog->EnableLog();
    WriteStartupMessage();

    return QTSS_NoErr;
}

// RereadPrefs - refread our prefs from the prefs file.
QTSS_Error RereadPrefs()
{
    delete [] sDefaultLogDir;
    (void)QTSS_GetValueAsString(sServerPrefs, qtssPrefsErrorLogDir, 0, &sDefaultLogDir);

    delete [] sLogName;
    sLogName = QTSSModuleUtils::GetStringAttribute(sPrefs, "mp3_request_logfile_name", sDefaultLogName);
    
    delete [] sLogDir;
    sLogDir  =  QTSSModuleUtils::GetStringAttribute(sPrefs, "mp3_request_logfile_dir", sDefaultLogDir);

    QTSSModuleUtils::GetAttribute(sPrefs, "mp3_streaming_enabled",  qtssAttrDataTypeBool16,
            &sMP3StreamingEnabled, &sDefaultMP3StreamingEnabled, sizeof(sMP3StreamingEnabled));

    delete [] sBroadcastPassword;
    sBroadcastPassword = QTSSModuleUtils::GetStringAttribute(sPrefs, "mp3_broadcast_password", sDefaultBroadcastPassword);
        
    // if broadcast password is blank or empty don't allow MP3 streaming.
    if (sBroadcastPassword == NULL || *sBroadcastPassword == '\0' || *sBroadcastPassword == ' ')
    {
        sMP3StreamingEnabled = false;
    }

    QTSSModuleUtils::GetAttribute(sPrefs, "mp3_broadcast_buffer_size",  qtssAttrDataTypeUInt32,
            &sBroadcastBufferSize, &sDefaultBroadcastBufferSize, sizeof(sBroadcastBufferSize));

    QTSSModuleUtils::GetAttribute(sPrefs, "mp3_max_flow_control_time", qtssAttrDataTypeSInt32,
                &sMaxFlowControlTimeInMSec, &sDefaultFlowControlTimeInMSec, sizeof(sMaxFlowControlTimeInMSec));

    QTSSModuleUtils::GetAttribute(sPrefs, "mp3_request_logging", qtssAttrDataTypeBool16,
                                &sLogEnabled, &sDefaultLogEnabled, sizeof(sLogEnabled));

    QTSSModuleUtils::GetAttribute(sPrefs, "mp3_request_logfile_size", qtssAttrDataTypeUInt32,
                                &sMaxLogBytes, &sDefaultMaxLogBytes, sizeof(sMaxLogBytes));

    QTSSModuleUtils::GetAttribute(sPrefs, "mp3_request_logfile_interval", qtssAttrDataTypeUInt32,
                                &sRollInterval, &sDefaultRollInterval, sizeof(sRollInterval));

    QTSSModuleUtils::GetAttribute(sPrefs, "mp3_request_logtime_in_gmt", qtssAttrDataTypeBool16,
                                &sLogTimeInGMT, &sDefaultLogTimeInGMT, sizeof(sLogTimeInGMT));

    UInt32 len = sizeof(SInt32);
    (void) QTSS_GetValue(sServerPrefs, qtssPrefsMaximumConnections, 0, (void*)&sMaximumConnections, &len);
    (void) QTSS_GetValue(sServerPrefs, qtssPrefsMaximumBandwidth, 0, (void*)&sMaximumBandwidth, &len);
    
    // handle changing the sLogEnabled state of the access log.
    if (sMP3AccessLog != NULL)
    {
        if (sLogEnabled)
        {
            if (!sMP3AccessLog->IsLogEnabled())
                sMP3AccessLog->EnableLog();
        }
        else
        {
            if (sMP3AccessLog->IsLogEnabled())
                sMP3AccessLog->CloseLog();
        }
    }

    return QTSS_NoErr;
}

// SessionClosing - clean up broadcaster/client connections.
QTSS_Error SessionClosing(QTSS_RTSPSession_Params* inParams)
{
    Assert(inParams != NULL);
#if DEBUG_MP3STREAMING_MODULE
    UInt32 uSessID = 0L;
    uSessID = GetRTSPSessionID(inParams->inRTSPSession);
    DTRACE1("Closing SessionID = %" _S32BITARG_ "\n", uSessID);
#endif
    MP3Session* mp3sess = sMP3SessionTable.Resolve(inParams->inRTSPSession);
    // If Resolve() returns a non-NULL value then we are closing
    // the RTSP session related to either a reflected MP3 broadcast
    // session or we are closing an RTSP session related to an
    // MP3 client session.
    //
    // If Resolve() returns NULL this RTSP session does not belong to
    // this module and we just ignore it by returning QTSS_NoErr.
    if (mp3sess != NULL)
    {
        if  (mp3sess->IsA() == kMP3BroadcasterSessionType)
        {
            // Remove the MP3BroadcasterSession from the global
            // server queue and delete its instance.
            return sMP3BroadcasterQueue.RemoveBroadcaster(inParams->inRTSPSession);
        }
        else if (mp3sess->IsA() == kMP3ClientSessionType)
        {
            MP3ClientSession* client = (MP3ClientSession*)mp3sess;
            MP3BroadcasterSession* owner = client->GetOwner();
            if (sLogEnabled)
                LogRequest(inParams->inRTSPSession, client);
            if (owner != NULL)
            {
                // Let the the owner, a MP3BroadcasterSession, 
                // remove the client from its queue and delete it.
                return owner->RemoveClient(inParams->inRTSPSession);
            }
            else
            {
                // otherwise, we no longer have an owner for this
                // client session and we just delete the instance.
                delete client;
                return QTSS_NoErr;
            }
        }
    }
    return QTSS_NoErr;      
}

QTSS_Error Shutdown()
{
    WriteShutdownMessage();
    return QTSS_NoErr;
}

QTSS_Error StateChange(QTSS_StateChange_Params* stateChangeParams)
{
    if (stateChangeParams->inNewState == qtssIdleState)
    {
        WriteShutdownMessage();
        sMP3BroadcasterQueue.TerminateAllBroadcastSessions();
    }
    else
    {
        WriteStartupMessage();
    }
    
    return QTSS_NoErr;
}

// IncrementMP3SessionCount - This increments the MP3 session counter in the server's attributes
void IncrementMP3SessionCount()
{
    QTSS_Error err = QTSS_NoErr;
    
    // Increment the server's MP3 session count.
    UInt32* ptrValue = NULL;
    UInt32 valueLen = sizeof(ptrValue);
    
    OSMutexLocker locker(sAtomicMutex);
    
    err = QTSS_GetValuePtr(sServer, qtssMP3SvrCurConn, 0, (void**)&ptrValue, &valueLen);
    if ((err == QTSS_NoErr) && (ptrValue != NULL))
    {
        *ptrValue += 1;
    }
    else
    {
        DTRACE1("QTSS_GetValuePtr() for qtssMP3SvrCurConn failed with err = %" _S32BITARG_ "\n", (SInt32)err);
    }
    // bump the total sessions count up also
    err = QTSS_GetValuePtr(sServer, qtssMP3SvrTotalConn, 0, (void**)&ptrValue, &valueLen);
    if ((err == QTSS_NoErr) && (ptrValue != NULL))
    {
        *ptrValue += 1;
    }
    else
    {
        DTRACE1("QTSS_GetValuePtr() for qtssMP3SvrTotalConn failed with err = %" _S32BITARG_ "\n", (SInt32)err);
    }
}

// DecrementMP3SessionCount - This decrements the MP3 session counter in the server's attributes
void DecrementMP3SessionCount()
{
    QTSS_Error err = QTSS_NoErr;
    
    // Decrement the server's MP3 session count.
    UInt32* ptrValue = NULL;
    UInt32 valueLen = sizeof(ptrValue);
    
    err = QTSS_GetValuePtr(sServer, qtssMP3SvrCurConn, 0, (void**)&ptrValue, &valueLen);
    if ((err == QTSS_NoErr) && (ptrValue != NULL))
    {
        OSMutexLocker locker(sAtomicMutex);
        *ptrValue -= 1;
    }
    else
    {
        DTRACE1("QTSS_GetValuePtr() for qtssMP3SvrCurConn failed with err = %" _S32BITARG_ "\n", (SInt32)err);
    }
}

// IncrementTotalMP3Bytes - This increments the MP3 byte counter in the server's attributes
void IncrementTotalMP3Bytes(UInt32 bytes)
{
    QTSS_Error err = QTSS_NoErr;
    
    // Increment the server's MP3 total byte count.
    UInt64* ptrValue = NULL;
    UInt32 valueLen = sizeof(ptrValue);
    
    err = QTSS_GetValuePtr(sServer, qtssMP3SvrTotalBytes, 0, (void**)&ptrValue, &valueLen);
    if ((err == QTSS_NoErr) && (ptrValue != NULL))
    {
        OSMutexLocker locker(sAtomicMutex);
        *ptrValue += bytes;
    }
    else
    {
        DTRACE1("QTSS_GetValuePtr() for qtssMP3SvrTotalBytes failed with err = %" _S32BITARG_ "\n", (SInt32)err);
    }
}

// CheckBandwidth - Check and see if we can handle any more bandwidth.
Bool16 CheckBandwidth(SInt32 bandwidth)
{
    QTSS_Error err = QTSS_NoErr;
    
    // Get our current bandwidth and check against server's maximum.
    SInt32* ptrValue = NULL;
    UInt32 valueLen = sizeof(ptrValue);
    
    err = QTSS_GetValuePtr(sServer, qtssMP3SvrCurBandwidth, 0, (void**)&ptrValue, &valueLen);
    if ((err == QTSS_NoErr) && (ptrValue != NULL))
    {
        OSMutexLocker locker(sAtomicMutex);
        if (((*ptrValue + bandwidth)/1000) <= sMaximumBandwidth)
            return true;
    }
    else
    {
        DTRACE1("QTSS_GetValuePtr() for qtssMP3SvrCurBandwidth failed with err = %" _S32BITARG_ "\n", (SInt32)err);
    }
    return false;
}

// GetRTSPSessionID - returns the RTSP Session ID for the given RTSPSession object.
// return 0 on failure.
UInt32 GetRTSPSessionID(QTSS_RTSPSessionObject session)
{
    UInt32 uSessID = 0L;
    UInt32 paramLen = sizeof(uSessID);
    
    QTSS_Error err = QTSS_GetValue(session, qtssRTSPSesID, 0, (void*)&uSessID, &paramLen);      
    if (err != QTSS_NoErr) 
    {
        uSessID = 0L;
    }
    return uSessID;
}

// IsActiveBroadcastSession - This is true when the specified session belongs to
// a current broadcast session. We use this to let us know if we were called back
// in our FilterRequest() role because a read or write was blocked previously.
inline Bool16 IsActiveBroadcastSession(QTSS_RTSPSessionObject session)
{
    return (FindBroadcastSession(session) != NULL);
}

// FindBroadcastSession - Given a RTSP session find the MP3BroadcasterSession
// object that handles this session. returns NULL pointer if none.
MP3BroadcasterSession* FindBroadcastSession(QTSS_RTSPSessionObject session)
{
    MP3Session* mp3sess = sMP3SessionTable.Resolve(session);
    if (mp3sess != NULL && mp3sess->IsA() == kMP3BroadcasterSessionType)
        return (MP3BroadcasterSession*) mp3sess;
    else
        return NULL;
}

// This determines if an incoming request is an broadcaster sending
// us his password. It will be in the format of:
// "SOURCE <password> <mountpoint>\n".
Bool16 IsBroadcastPassword(StrPtrLen& theRequest)
{
    if (IsShoutcastPassword(theRequest))
        return true;
    StrPtrLen token = theRequest;
    token.Len = 6;
    return token.Equal(StrPtrLen("SOURCE"));
}

// This determines if an incoming request is a Shoutcast broadcaster sending
// us his password. It will be in the format of:
// <password>\n".
Bool16 IsShoutcastPassword(StrPtrLen& theRequest)
{
    char* bp = (char*)theRequest.Ptr;
    int i;
    int n = (int)theRequest.Len;
    
    for (i=0; i<n; i++)
    {
        // any request with an embeded whitespace cannot be a
        // Shoutcast password request.
        if (*bp == ' ' || *bp == '\t')
            return false;
    }
    // otherwise we assume it is.
    return true;
}

Bool16 IsProtocolString(StrPtrLen& source, char *protocol)
{

    UInt32 protocolLen = ::strlen(protocol);
    if (protocolLen <= source.Len) // must be smaller or same
    {
        source.Len = protocolLen; // trim off the /version from protocol/version
        if (source.EqualIgnoreCase(protocol))
            return true;
    }
    
	return false;

}

Bool16 IsProtocol(StrPtrLen& theRequest, char* protocol)
{
    StringParser reqParse(&theRequest);
	StrPtrLen line;
	reqParse.GetThruEOL(&line);
    StringParser lineParse(&line);

    StrPtrLen lastWord;
    StrPtrLen currentWord;

    lineParse.ConsumeUntilWhitespace(&currentWord);
    lineParse.ConsumeWhitespace();

    if (IsProtocolString(currentWord, protocol))//test for line starting with protocol/vers in a response from the client
        return true;

    if (line.Len > 0) do 
    {
        lastWord = currentWord;
        lineParse.ConsumeUntilWhitespace(&currentWord);
        lineParse.ConsumeWhitespace();
    } while (currentWord.Len > 0);
 
    if (IsProtocolString(lastWord, protocol))// test for line ending with protocol/version  as in a request from the client
        return true;

    return false;
}

Bool16 IsHTTP(StrPtrLen& theRequest)
{
   return IsProtocol(theRequest, "HTTP");
}

Bool16 IsRTSP(StrPtrLen& theRequest)
{
   return IsProtocol(theRequest, "RTSP");
}


// This determines if an incoming request is an HTTP GET
// request.
Bool16 IsHTTPGet(StrPtrLen& theRequest)
{
	StrPtrLen token = theRequest;
	token.Len = 3;
    Bool16 found = false;
    if (token.EqualIgnoreCase(StrPtrLen("GET")) && IsHTTP(theRequest) )
        found = true;
	return found;
}

// Is this URL a *.m3u file request.
Bool16 IsA_m3u_URL(char* theURL)
{
    if (::strstr(theURL, ".m3u") != 0)
        return true;
    return false;
}

// Is this URL a metdata xfer.
Bool16 IsMetaDataURL(char* theURL)
{
    if (::strncmp(theURL, "/admin.cgi?mode=updinfo", 23) == 0)
        return true;
    return false;
}

// Parse this URL and extract metdata.
void ParseMetaDataURL(char* theURL)
{
    char URLBuffer[kURLBufferSize];
    
    if (theURL == NULL)
    {
        Assert(0);
        return;
    }
    else if (::strlen(theURL) == 0)
    {
        Assert(0);
        return;
    }
    DTRACE1("##ParseMetaDataURL() - meta-data URL <%s>\n", theURL);
    
    StrPtrLen urlPtr(theURL);
    StrPtrLen token;
    StringParser urlParser(&urlPtr);
    
    urlParser.ConsumeUntil(&token, '&');
    if ( !token.Equal(StrPtrLen("/admin.cgi?mode=updinfo")) )
    {
        // expected '/admin.cgi?mode=updinfo' in URL!
        Assert(0);
        return;
    }
    if ( !urlParser.Expect('&') )
    {
        // expected '&' in URL after '/admin.cgi?mode=updinfo'
        Assert(0);
        return;
    }
    urlParser.ConsumeUntil(&token, '=');
    if ( !token.Equal(StrPtrLen("pass")) )
    {
        // expected 'pass=' in URL!
        Assert(0);
        return;
    }
    if ( !urlParser.Expect('=') )
    {
        // expected '=' in URL after 'pass'!
        Assert(0);
        return;
    }
    urlParser.ConsumeUntil(&token, '&');
    if ( !token.Equal((const char*)sBroadcastPassword) )
    {
        DTRACE1("##ParseMetaDataURL() - expected Password of '%s' in URL!\n", sBroadcastPassword);
        return;
    }
    else
    {
        if ( !urlParser.Expect('&') )
        {
            // expected '&' in URL after 'pass=xxx'!
            Assert(0);
            return;
        }
    }
    
    urlParser.ConsumeUntil(&token, '=');
    if ( !token.Equal(StrPtrLen("mount")) )
    {
        // expected 'mount=' in URL!
        Assert(0);
        return;
    }
    
    if ( !urlParser.Expect('=') )
    {
        // expected '=' in URL after 'mount'!
        Assert(0);
        return;
    }
    
    urlParser.ConsumeUntil(&token, '&');
    if ( token.Len < 1 )
    {
        // expected mountpoint in URL!
        Assert(0);
        return;
    }
    else
    {
        if ( !urlParser.Expect('&') )
        {
            // expected '&' in URL after 'mount=xxx'!
            Assert(0);
            return;
        }
    }
    ::memset(URLBuffer, 0, kURLBufferSize);
    ::memcpy(URLBuffer, token.Ptr, token.Len);
    
    DTRACE1("##ParseMetaDataURL() - got mountpoint of '%s' in URL.\n", URLBuffer);
    MP3BroadcasterSession* xs = sMP3BroadcasterQueue.FindByMountPoint(token);
    if (xs == NULL)
    {
        DTRACE("##ParseMetaDataURL() - no matching mountpoint found!\n");
        return;
    }
    
    urlParser.ConsumeUntil(&token, '=');
    if ( !token.Equal(StrPtrLen("song")) )
    {
        // expected 'song=' in URL!
        Assert(0);
        return;
    }
    
    if ( !urlParser.Expect('=') )
    {
        // expected '=' in URL after 'song'!
        Assert(0);
        return;
    }
    xs->SetSongName(urlParser.GetCurrentPosition());
}

void SendBroadcastAuthErr(QTSS_Filter_Params* inParams, char * errormessage)
{
    if (errormessage != NULL)
    {
        UInt32 len = ::strlen(errormessage);
        (void)QTSS_Write(inParams->inRTSPRequest,errormessage ,len, NULL, 0);
        (void)QTSS_SetValue(inParams->inRTSPRequest, qtssRTSPReqRespMsg, 0, errormessage, len);
    }

    QTSS_SessionStatusCode inStatusCode = qtssClientUnAuthorized;
    (void)QTSS_SetValue(inParams->inRTSPRequest, qtssRTSPReqStatusCode, 0, &inStatusCode, sizeof(inStatusCode));

    const Bool16 sFalse = false;
    (void)QTSS_SetValue(inParams->inRTSPRequest, qtssRTSPReqRespKeepAlive, 0, &sFalse, sizeof(sFalse));

}

// This parses and checks the password in the broadcaster's
// incoming request.
Bool16 CheckPassword(QTSS_Filter_Params* inParams, StrPtrLen& theRequest, StrPtrLen& mountpoint)
{
    Bool16 passwordOK = false;
    StrPtrLen strPtr;
    
    mountpoint.Ptr = NULL;
    mountpoint.Len = 0;
    StringParser reqParse(&theRequest);
    reqParse.ConsumeUntilWhitespace(&strPtr);
    char* tmp;

    if ( strPtr.Equal(StrPtrLen("SOURCE")) )
    {
            
        //it's an x-audio broadcast request
        // PARSE X-AUDIO HERE!!!!
        reqParse.ConsumeWhitespace();
        reqParse.ConsumeUntilWhitespace(&strPtr);
        tmp = strPtr.GetAsCString();
        // Let's see if the password matches...
        DTRACE1("CheckPassword() - #Got X-AUDIO password: %s\n", tmp);
        if (strcmp(tmp, sBroadcastPassword) != 0)
        {
            SendBroadcastAuthErr(inParams, "ERROR - Bad Password\r\n"); //ice cast style error
            delete[] tmp;
            return false;
        }
        delete[] tmp;
        // get the mount point
        reqParse.ConsumeWhitespace();
        reqParse.ConsumeUntilWhitespace(&strPtr);
        mountpoint.Ptr = strPtr.Ptr;
        mountpoint.Len = strPtr.Len;
        tmp = strPtr.GetAsCString();
        DTRACE1("CheckPassword() - #Got X-AUDIO mountpoint: %s\n", tmp);
        delete[] tmp;
        passwordOK = true;
    }
    else if ( IsShoutcastPassword(strPtr) )
    {
        tmp = strPtr.GetAsCString();
        if (strPtr.Len > 1)
            tmp[strPtr.Len] = '\0';
        // Let's see if the password matches...
        DTRACE1("CheckPassword() - #Got Shoutcast password: %s\n", tmp);
        if (strcmp(tmp, sBroadcastPassword) != 0)
        {
            SendBroadcastAuthErr(inParams, "invalid password\r\n"); // shoutcast style error
            delete[] tmp;
            return false;
        }
        delete[] tmp;
        mountpoint.Ptr = "/";
        mountpoint.Len = 1;
        passwordOK = true;
    }
    return passwordOK;
}


Bool16 GetHeaderAndValueFromeLine(StrPtrLen *line, StrPtrLen* header, char separator, StrPtrLen* value)
{
    StringParser lineParser(line);  
    lineParser.ConsumeWhitespace();

    Bool16 foundHeader = lineParser.GetThru(header, separator);

    lineParser.ConsumeWhitespace();
    lineParser.GetThruEOL(value);

    return foundHeader;
}


// Find out if the HTTP header wants meta-data.
Bool16 NeedsMetaData(StrPtrLen& theRequest)
{


    StrPtrLen       line;
    StrPtrLen       header;
    StrPtrLen       value;
    StringParser    requestParser(&theRequest); 

    requestParser.GetThruEOL(NULL); // strip off command line
    while ( requestParser.GetThruEOL(&line) || line.Len > 0 ) // pull out each line
    {
        if (!GetHeaderAndValueFromeLine(&line, &header, ':', &value))
            return false; // no separator found
       
        if (header.EqualIgnoreCase("icy-metadata") && (StringParser(&value).ConsumeInteger() > 0) )
            return true; // icy-metadata: (non zero value)
    }

    return false;

}

/*  NeedsMetaData Test cases
StrPtrLen temp;
temp.Set("\r\nIcy-metadatA: 1\r");
 if ( NeedsMetaData(temp) )
    printf("icy-metadata: 1-1\n");

temp.Set("\ricy-metadata: 1\r");
if ( NeedsMetaData(temp) )
    printf("icy-metadata: 1-2\n");

temp.Set("\nicy-metadata: 1\r");
if ( NeedsMetaData(temp) )
    printf("icy-metadata: 1 -3\n");

temp.Set("\r\nicy-metadata:1\r");
if ( NeedsMetaData(temp) )
    printf("icy-metadata:1-4\n");

temp.Set("\r\nicy-metadata:\r");
if ( NeedsMetaData(temp) )
    printf("icy-metadata:-5\n");

temp.Set("\r\nicy-metadata 1\n");
if ( NeedsMetaData(temp) )
    printf("icy-metadata 1-6\n");

temp.Set("\r\nicy-metadata: 0\r");
if ( NeedsMetaData(temp) )
    printf("icy-metadata: 0-7\n");

temp.Set("\r\nicy-metadata:  3\r");
if ( NeedsMetaData(temp) )
    printf("icy-metadata: 3-8\n");

temp.Set("\r\nIcy-metadatA: 1");
 if ( NeedsMetaData(temp) )
    printf("icy-metadata: 1-0\n");

temp.Set("\nIcy-metadatA: 1");
 if ( NeedsMetaData(temp) )
    printf("icy-metadata: 1-10\n");
*/


// Parse out the URL from the HTTP GET line.
Bool16 ParseURL(StrPtrLen& theRequest, char* outURL, UInt16 maxlen)
{
    StringParser reqParse(&theRequest);
    StrPtrLen strPtr;
    
    ::memset(outURL, 0, maxlen);
    reqParse.ConsumeWord(&strPtr);

    if ( !strPtr.Equal(StrPtrLen("GET")) )
    {
        return false;
    }
    reqParse.ConsumeWhitespace();
    reqParse.ConsumeUntilWhitespace(&strPtr);
    if (strPtr.Len == 0)
        return false;
    else if ((UInt16)strPtr.Len > maxlen-1)
        strPtr.Len = maxlen-1;
    ::memcpy(outURL, strPtr.Ptr, strPtr.Len);

    return true;
}

void    WriteStartupMessage()
{
        if (!sServerIdle)
            return;
    sServerIdle = false;
    
    //format a date for the startup time
    char theDateBuffer[QTSSRollingLog::kMaxDateBufferSizeInBytes];
    Bool16 result = QTSSRollingLog::FormatDate(theDateBuffer, false);
    
    char tempBuffer[1024];
    if (result)
        qtss_sprintf(tempBuffer, "#Remark: Streaming beginning STARTUP %s\n", theDateBuffer);
        
    // log startup message to error log as well.
    if ((result) && (sMP3AccessLog != NULL))
        sMP3AccessLog->WriteToLog(tempBuffer, kAllowLogToRoll);
}

void    WriteShutdownMessage()
{
        if (sServerIdle)
            return;
    sServerIdle = true;
    
    //log shutdown message
    //format a date for the shutdown time
    char theDateBuffer[QTSSRollingLog::kMaxDateBufferSizeInBytes];
    Bool16 result = QTSSRollingLog::FormatDate(theDateBuffer, false);
    
    char tempBuffer[1024];
    if (result)
        qtss_sprintf(tempBuffer, "#Remark: Streaming beginning SHUTDOWN %s\n", theDateBuffer);

    if ( result && sMP3AccessLog != NULL )
        sMP3AccessLog->WriteToLog(tempBuffer, kAllowLogToRoll);
}

QTSS_Error LogRequest(QTSS_RTSPSessionObject inRTSPSession, MP3ClientSession* client)
{
    char logbuffer[1024];
    char reqBuffer[kRequestBufferSize];
        char userAgent[kUserAgentBufferSize];
    char theDateBuffer[QTSSRollingLog::kMaxDateBufferSizeInBytes];
    UInt16 reqResult = 200;
    UInt32 byteCount = 54000000;
        UInt32 duration = 0;
    QTSS_Error theError = QTSS_NoErr;
        
        // Sanity check...
        if (sMP3AccessLog == NULL)
            return QTSS_NoErr;
        
        // LogRequest() is not re-entrant. Hold a mutex until done.
    OSMutexLocker locker(sLogMutex);
    
    // Construct the timestamp for the entry
    Bool16 result = QTSSRollingLog::FormatDate(theDateBuffer, sLogTimeInGMT);
    // this should never happen, but just in case...
    if (!result)
        theDateBuffer[0] = '\0';
    
    // Get the IP address of the client who made the request
    char remoteAddress[kRemoteAddressSize] = {0};
    StrPtrLen theClientIPAddressStr;
    theClientIPAddressStr.Set(remoteAddress,kRemoteAddressSize);
    QTSS_Error err = QTSS_GetValue( inRTSPSession, 
                                        qtssRTSPSesRemoteAddrStr, 
                                        0, 
                                        (void*)theClientIPAddressStr.Ptr, 
                                        &theClientIPAddressStr.Len);
    if (err != QTSS_NoErr) 
        ::strcpy(remoteAddress, "127.0.0.1");
    
    // Pull the actual HTTP request line from the MP3 client session.
    ::strcpy(reqBuffer, client->GetRequest());
    
        // Get the user agent's name or "unknown user agent" if none was specified
        // in the original HTTP request.
        userAgent[0] = '\0';
    ::strcpy(userAgent, client->GetUserAgent());
        if (userAgent[0] == '\0')
            ::strcpy(userAgent, "unknown user agent");
    
        // Determine how may bytes this client was streamed.
    byteCount = client->GetTotalCount();
        
        // Calculate the total connect duration in seconds.
        duration = (UInt32)((OS::Milliseconds() - client->GetConnectTime())/1000);
        
        // Finally, get the HTTP result code.
    reqResult = client->GetResult();

    // Format the access log entry parameters here...
    qtss_sprintf(logbuffer, "%s \"%s\" [%s] \"%s\" %d %" _S32BITARG_ " %" _S32BITARG_ "\n",
        remoteAddress,
                userAgent,
        theDateBuffer,
        reqBuffer,
        reqResult,
        byteCount,
                duration
    );
    
        // Commit it to disk...
        sMP3AccessLog->WriteToLog(logbuffer, kAllowLogToRoll);
    return theError;
}

// url_strcpy - works like strcpy except that it handles URL escape
// conversions as it copies the string.
void url_strcpy(char* dest, const char* src)
{
    int c1, c2;
    while(*src)
    {
        if (*src == '%')
        {
            src++;
            c1 = *src++;
            if (c1 >= '0' && c1 <= '9')
                c1 -= '0';
            else if (c1 >= 'A' && c1 <= 'F')
                c1 -= 'A' + 10;
            else if (c1 >= 'a' && c1 <= 'f')
                c1 -= 'a' + 10;
            else
                c1 = 0;
            c2 = *src;
            if (c2 >= '0' && c2 <= '9')
                c2 -= '0';
            else if (c2 >= 'A' && c2 <= 'F')
                c2 -= 'A' + 10;
            else if (c2 >= 'a' && c2 <= 'f')
                c2 -= 'a' + 10;
            else
                c2 = 32;
            *dest = (c1 * 16) + c2;
        }
        else
        {
            *dest = *src;
        }
        // we need to replace single quotes with
        // a tick so that the client can parse meta-data.
        if (*dest == '\'')
        {
            *dest   = '`';
        }
        dest++;
        src++;
    }
    *dest = '\0';
}

// ReEnterFilterRequest - This function is called whenever we
// have called back our module in the QTSS_FilterRequest role
// after calling it the first time.
// This will happen in one of two ways:
// 1. A MP3BroadcasterSession posted a read request event and data
// became available on its stream.
// 2. The Idle timer called us back to poll a MP3ClientSession to
// see if it needs to die.
QTSS_Error ReEnterFilterRequest(QTSS_Filter_Params* inParams, MP3Session* mp3Session)
{
    QTSS_Error err = QTSS_NoErr;
    QTSS_RTSPRequestObject theRequest = inParams->inRTSPRequest;
    
    if (mp3Session != NULL)
    {
        UInt8 sessTyp = mp3Session->IsA();
        switch(sessTyp)
        {
        case kMP3BroadcasterSessionType:
            // this is a MP3 broadcaster session. Invoke it's state machine.
            return ((MP3BroadcasterSession*)mp3Session)->ExecuteState();
        case kMP3ClientSessionType:
            // this is a MP3 client session. If we are active schedule ourself
            // to be called back again in kClientPollInterval milliseconds.
            if (mp3Session->GetState() == MP3ClientSession::kClientSendDataState)
            {
                MP3ClientSession* client = (MP3ClientSession*)mp3Session;
                if (client->WasBlocked())
                    client->RetrySendData();
                
                KeepSession(theRequest, true);
                err = QTSS_SetIdleTimer(kClientPollInterval);
                return err;
            }
            else
            {
                KeepSession(theRequest, false);
            }
            return QTSS_NoErr;
        default:
            Assert(0);
            return QTSS_NoErr;
        }
    }
    return err;
}

// FilterRequest - filter the incoming HTTP/Broadcast request.
QTSS_Error FilterRequest(QTSS_Filter_Params* inParams)
{
	QTSS_Error err = QTSS_NoErr;
	QTSS_RTSPRequestObject theRequest = inParams->inRTSPRequest;
		
	// Pull the actual request data from this RTSP session's attributes.

	StrPtrLen theFullRequest;
	err = QTSS_GetValuePtr(theRequest, qtssRTSPReqFullRequest, 0, (void**)&theFullRequest.Ptr, &theFullRequest.Len);
	if (err != QTSS_NoErr) 
	{
            return QTSS_NoErr;
	}

    if (IsRTSP(theFullRequest)) // don't process rtsp requests
        return  QTSS_NoErr;

	if (!sMP3StreamingEnabled || sServerIdle)
	{
            // This allows MP3 streaming to be disabled through a pref setting
            // or ignore requests when the server is in the idle state.

            if (NeedsMetaData(theFullRequest)) // this is an HTTP icy request
                return QTSSModuleUtils::SendHTTPErrorResponse(theRequest,qtssServerUnavailable,true, NULL);
        
            return QTSS_NoErr;
	}
	
	// See if this RTSP session is an existing MP3 Session.
	// If it is then we have re-entered FilterRequest() from a previous 
	// invokation and we will call ReEnterFilterRequest() to do its thing.
	MP3Session* mp3Session = sMP3SessionTable.Resolve(inParams->inRTSPSession);
	if (mp3Session != NULL)
	{
            err = ReEnterFilterRequest(inParams, mp3Session);
            return err;
    }
    
    // Check and see if this request is a broadcaster sending it's password or
    // an HTTP GET. If it's not, it's assumed to be a RTSP request and we ignore 
    // it. In that case, some other module must handle this request.
#if DEBUG_MP3STREAMING_MODULE
    DTRACE("####\n");
    PrintStringBuffer(theFullRequest);
#endif
    if (!IsBroadcastPassword(theFullRequest) && !IsHTTPGet(theFullRequest))
    {
        return QTSS_NoErr;
    }
    
    MP3BroadcasterSession* xs = NULL;
    
#if DEBUG_MP3STREAMING_MODULE
    // Debugging info - Retrieve the RTSP session ID associated with this session.
    UInt32 uSessID = 0L;
    uSessID = GetRTSPSessionID(inParams->inRTSPSession);
    DTRACE1("####\n###Filtering SessionID = %" _S32BITARG_ "\n", uSessID);
#endif
    
    // See if we have exceeded our maximum number of client connections
    if (MP3Session::GetTotalNumMP3Sessions() >= sMaximumConnections)
    {
		// we are currently at our connection limit. return a 400 error for compatibility with icecast style error.
		return QTSSModuleUtils::SendHTTPErrorResponse(theRequest,qtssClientBadRequest,true, NULL);
    }
    else if (!CheckBandwidth(kBandwidthToAddEstimate))
    {
		// we are currently at our bandwidth limit. return a 400 error for compatibility with icecast style error.
		return QTSSModuleUtils::SendHTTPErrorResponse(theRequest,qtssClientBadRequest,true, NULL);
    }
    
    StrPtrLen mountpoint;
	if (!IsHTTP(theFullRequest) && IsBroadcastPassword(theFullRequest) && CheckPassword(inParams, theFullRequest, mountpoint))
    {
        err = sMP3BroadcasterQueue.CreateBroadcaster(inParams->inRTSPSession, inParams->inRTSPRequest, mountpoint);
        if (err != QTSS_NoErr)
        {
                    // CreateBroadcaster returned an error but it has already sent the reject reply 
                    // so there's nothing left to do.
                    return QTSS_NoErr;
        }
        // If we're here this is a new connection request from a broadcaster.
        // Read the broadcast headers from the broadcaster
        xs = FindBroadcastSession(inParams->inRTSPSession);
        if (xs != NULL)
            err =  xs->GetBroadcastHeaders();
        else
            err = QTSS_NoErr;
        return err;
    }
    else if ( IsHTTPGet(theFullRequest) )
    {
        char theURL[kURLBufferSize];
        
        MP3BroadcasterSession* xs = NULL;
        if (!ParseURL(theFullRequest, theURL, kURLBufferSize))
        {
            return QTSS_NoErr;
        }
        // if this was a '*.m3u' file URL then handle it here
                // sendind back a reply containing the URL.
                // This effectively accomplishes a redirect.
        if ( IsA_m3u_URL(theURL) )
        {
                        char tmpbuf[1024];
                        tmpbuf[sizeof(tmpbuf) -1] = 0;
                        char tmp[1024];
                        tmp[sizeof(tmp) -1] = 0;
            // remove the '.m3u' prefix from the URL
                        UInt32 ulen = ::strlen(theURL) - 4;
                        UInt32 serverIPAddr;
                        UInt8 x1, x2, x3, x4;
                        theURL[ulen] = '\0';
                        // Get the server IP address for building the reply playlist path
                        ulen = sizeof(serverIPAddr);
                        err = QTSS_GetValue(sServer, qtssSvrDefaultIPAddr, 0, &serverIPAddr, &ulen);
                        if (err == QTSS_NoErr)
                        {
                            x1 = (UInt8)((serverIPAddr >> 24) & 0xff);
                            x2 = (UInt8)((serverIPAddr >> 16) & 0xff);
                            x3 = (UInt8)((serverIPAddr >> 8) & 0xff);
                            x4 = (UInt8)((serverIPAddr) & 0xff);
                        }
                        else
                        {
                            x1 = 127; x2 = x3 = 0; x4 = 1;
                        }

                        // construct the reply string for the client.
                        qtss_snprintf(tmp,sizeof(tmp) -1, "http://%d.%d.%d.%d:8000%s", x1,x2,x3,x4, theURL);
                        ulen = ::strlen(tmp);
                        qtss_snprintf(tmpbuf,sizeof(tmpbuf) -1, "%s %"   _U32BITARG_   "\r\n\r\n%s\r\n", gM3UReplyHeader, ulen, tmp);
                        ulen = ::strlen(tmpbuf);
            // send the reply to the client.
            err = QTSS_Write(inParams->inRTSPRequest, tmpbuf, ulen, NULL, qtssWriteFlagsBufferData);
            return QTSS_NoErr;
        }
        // if this was a meta-data transfer URL then handle it here
        if ( IsMetaDataURL(theURL) )
        {
            // Parse meta-data request here
            ParseMetaDataURL(theURL);
            // tell the broadcaster we got it.
            err = QTSS_Write(inParams->inRTSPRequest, kOKHeader, ::strlen(kOKHeader), NULL, qtssWriteFlagsBufferData);
            return QTSS_NoErr;
        }
        // Since this URL was not a metadata transfer let's assume it is
        // a broadcast mount point and search for it.
        xs = sMP3BroadcasterQueue.FindByMountPoint(theURL);
        if (xs == NULL)
        {
           if (NeedsMetaData(theFullRequest)) // this is an HTTP icy request
                return QTSSModuleUtils::SendHTTPErrorResponse(theRequest,qtssClientNotFound,true, NULL);

			// Let some other module handle this URL.
            return QTSS_NoErr;
        }
        // if we're here this is a new MP3 connection request from a client.
        // Start the clients stream up...
        err = xs->AddClient(inParams->inRTSPSession, inParams->inRTSPRequest);
        return err;
    }
    return QTSS_NoErr;
}

