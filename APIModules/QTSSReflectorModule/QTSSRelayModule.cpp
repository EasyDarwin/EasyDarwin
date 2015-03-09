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
    File:       QTSSReflectorModule.cpp

    Contains:   Implementation of QTSSReflectorModule class. 
                    
    
    
*/

#include "QTSSRelayModule.h"
#include "QTSSModuleUtils.h"
//#include "ReflectorSession.h"
#include "RelaySession.h"
#include "ReflectorStream.h"
#include "OSArrayObjectDeleter.h"
#include "QTSS_Private.h"

#include "defaultPaths.h"
#include "OSMemory.h"
#include "OSRef.h"
#include "IdleTask.h"
#include "Task.h"
#include "OS.h"
#include "Socket.h"
#include "SocketUtils.h"
#include "XMLParser.h"
#include "OSArrayObjectDeleter.h"

//ReflectorOutput objects
#include "RTPSessionOutput.h"
#include "RelayOutput.h"

//SourceInfo objects
#include "SDPSourceInfo.h"
#include "RelaySDPSourceInfo.h"
#include "RCFSourceInfo.h"
#include "RTSPSourceInfo.h"

#include <sys/stat.h>
#ifndef __Win32__
#include <netdb.h>
#endif

#ifndef kVersionString
#include "revision.h"
#endif

#if DEBUG
#define REFLECTOR_MODULE_DEBUGGING 0
#else
#define REFLECTOR_MODULE_DEBUGGING 0
#endif

// STATIC DATA

static char*                sRelayPrefs     = NULL;
static char*                sRelayStatsURL      = NULL;
static StrPtrLen            sRequestHeader;
static QTSS_ModulePrefsObject       sPrefs          = NULL;
static QTSS_ServerObject        sServer         = NULL;
static QTSS_Object          sAttributes     = NULL;
static Bool16               sSkipAuthorization  = true;
static Bool16               sIsRelaySession     = true;
static char*                sIsRelaySessionAttrName = "QTSSRelayModuleIsRelaySession";
static QTSS_AttributeID         sIsRelaySessionAttr = qtssIllegalAttrID;

static int                  sRelayPrefModDate = -1;

// ATTRIBUTES

static QTSS_AttributeID     sRelayModulePrefParseErr  = qtssIllegalAttrID;

static char* sDefaultRelayPrefs = DEFAULTPATHS_ETC_DIR "relayconfig.xml";

#ifdef __MacOSX__
#define kResponseHeader	"HTTP/1.0 200 OK\r\nServer: QuickTimeStreamingServer/%s/%s\r\nConnection: Close\r\nContent-Type: text/html\r\n\r\n<HTML><TITLE>Relay Stats</TITLE><BODY>"
#else
#define kResponseHeader	"HTTP/1.0 200 OK\r\nServer: EasyDarwinServer/%s/%s\r\nConnection: Close\r\nContent-Type: text/html\r\n\r\n<HTML><TITLE>Relay Stats</TITLE><BODY>"
#endif

static char sResponseHeader[1024];

static OSQueue*     sSessionQueue = NULL;
static OSQueue*     sAnnouncedQueue = NULL;
static OSQueue*     sRTSPSourceInfoQueue = NULL;
static OSQueue*     sRTSPSessionIDQueue = NULL;

static XMLParser*   sRelayPrefsFile = NULL;
static OSMutex      sResolverMutex;
class DNSResolverThread;
DNSResolverThread*  sResolverThread = NULL;
Bool16              sDoResolveAgain = false;

// This struct is used when Rereading Relay Prefs
struct SourceInfoQueueElem
{
    SourceInfoQueueElem(SourceInfo* inInfo, Bool16 inRTSPInfo) :fElem(), fSourceInfo(inInfo),
                                                                fIsRTSPSourceInfo(inRTSPInfo),
                                                                fShouldDelete(true) { fElem.SetEnclosingObject(this); }
    ~SourceInfoQueueElem() {}
    
    OSQueueElem fElem;
    SourceInfo* fSourceInfo;
    Bool16      fIsRTSPSourceInfo;
    Bool16      fShouldDelete;
};

// This struct is used when setting up announced source relays
struct RTSPSessionIDQueueElem
{
    RTSPSessionIDQueueElem(UInt32 rtspSessionID) :fElem(), fRTSPSessionID(rtspSessionID) { fElem.SetEnclosingObject(this); }
    ~RTSPSessionIDQueueElem() {}
    
    OSQueueElem fElem;
    UInt32 fRTSPSessionID;
};


// FUNCTION PROTOTYPES

static QTSS_Error QTSSRelayModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error   Register(QTSS_Register_Params* inParams);
static QTSS_Error Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error Shutdown();
static QTSS_Error RereadPrefs();

static RelaySession* CreateSession(SourceInfoQueueElem* inElem);
static RelaySession* FindSession(SourceInfoQueueElem* inElem);
static RelaySession* FindNextSession(SourceInfoQueueElem* inElem, OSQueueIter* inIterPtr);

static void AddOutputs(SourceInfo* inInfo, RelaySession* inSession, Bool16 inIsRTSPSourceInfo);
static void RemoveOutput(ReflectorOutput* inOutput, RelaySession* inSession);

static QTSS_Error Filter(QTSS_StandardRTSP_Params* inParams);
static void FindRelaySessions(OSQueue* inSessionQueue);

static QTSS_Error SetupAnnouncedSessions(QTSS_StandardRTSP_Params* inParams);
static RTSPSessionIDQueueElem* FindRTSPSessionIDQueueElem(UInt32 inSessionID);
static RTSPSourceInfo* FindAnnouncedSourceInfo(UInt32 inIP, StrPtrLen& inURL);
static RTSPSourceInfo* FindExistingRelayInfo(UInt32 inIP, StrPtrLen& inURL);

static void RereadRelayPrefs(XMLParser* prefsParser);
static void FindSourceInfos(OSQueue* inSessionQueue, OSQueue* inAnnouncedQueue, XMLParser* prefsParser);
static void ClearSourceInfos(OSQueue* inQueue);

static QTSS_Error RouteAuthorization(QTSS_StandardRTSP_Params* inParams);
static Bool16 IsRelayRequest(UInt16 inPort);

static void ReadRelayPrefsFile();
static Bool16 CheckDNSNames(XMLParser* prefsFile, Bool16 doResolution);
static void ResolveDNSAddr(XMLTag* tag);


// DNS Resolution can block a thread for a long time (there's no async version), and on X
// we only have 1 task thread, so the resolution must be on another thread.  However, we
// want to do the rest of the RereadPrefs on the main task thread, so we need both a thread
// and a task.  The thread resolves the addresses and fires the task, the task deletes
// the thread and is deleted itself as soon as it's done.

class DNSResolverThread : public OSThread
{
    class RereadPrefsTask : public Task
    {
        

        public:
            RereadPrefsTask() : Task () {this->SetTaskName("DNSResolverThread::RereadPrefsTask");}
            virtual SInt64 Run()
            {
                RereadRelayPrefs(sRelayPrefsFile);
                delete sResolverThread;
                sResolverThread = NULL;
                delete sRelayPrefsFile;
                
                // we need to see if reread prefs has been called again while we were resolving.
                // If so, it just exited without doing anything, and we need to start over to pick
                // up whatever changed.
                sResolverMutex.Lock();
                sRelayPrefsFile = NULL;
                if (sDoResolveAgain)
                {
                    sDoResolveAgain = false;
                    sResolverMutex.Unlock();
                    ReadRelayPrefsFile();
                }
                else sResolverMutex.Unlock();
    
                return -1;
            }
    };

public:
    static void ResolveRelayPrefs(XMLParser* relayPrefs)
    {
        sResolverMutex.Lock();
            
        if (sRelayPrefsFile == NULL)
        {
            sRelayPrefsFile = relayPrefs;
            sResolverThread = new DNSResolverThread();
            sResolverThread->Start();
        }
        else
        {
            sDoResolveAgain = true;    // it's already resolving, tell it to try again when it's done
            delete relayPrefs;
        }
        
        sResolverMutex.Unlock();
    }
    
    virtual void Entry()
    {
        if (sRelayPrefsFile != NULL)
        {
            CheckDNSNames(sRelayPrefsFile, true);
            RereadPrefsTask* tempTask = new RereadPrefsTask();
            tempTask->Signal(Task::kIdleEvent);
        }
    }
};

// FUNCTION IMPLEMENTATIONS

QTSS_Error QTSSRelayModule_Main(void* inPrivateArgs)
{
    return _stublibrary_main(inPrivateArgs, QTSSRelayModuleDispatch);
}


QTSS_Error  QTSSRelayModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
    switch (inRole)
    {
        case QTSS_Register_Role:
            return Register(&inParams->regParams);
        case QTSS_Initialize_Role:
            return Initialize(&inParams->initParams);
        case QTSS_RereadPrefs_Role:
        
            return RereadPrefs();
        case QTSS_RTSPFilter_Role:
            return Filter(&inParams->rtspRequestParams);
        case QTSS_RTSPRoute_Role:
            return RouteAuthorization(&inParams->rtspRouteParams);
        case QTSS_RTSPPostProcessor_Role:
            return SetupAnnouncedSessions(&inParams->rtspPostProcessorParams);
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
    (void)QTSS_AddRole(QTSS_RTSPFilter_Role);
    (void)QTSS_AddRole(QTSS_RTSPRoute_Role);
    (void)QTSS_AddRole(QTSS_RereadPrefs_Role);
    (void)QTSS_AddRole(QTSS_RTSPPostProcessor_Role);
    
    // get the text for the error message from the server atrribute.
    static char* sRelayModulePrefParseErrName   = "QTSSRelayModulePrefParseError";
    (void)QTSS_AddStaticAttribute(qtssTextMessagesObjectType, sRelayModulePrefParseErrName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssTextMessagesObjectType, sRelayModulePrefParseErrName, &sRelayModulePrefParseErr);

        // add a boolean attribute for the relay session to the client session object
        // we set it to true if we know that it is a relay session else we set it to false
        // (set to true for those client sessions that are created for the relay when announces come in) 
        (void)QTSS_AddStaticAttribute(qtssClientSessionObjectType, sIsRelaySessionAttrName, NULL, qtssAttrDataTypeBool16);
    (void)QTSS_IDForAttr(qtssClientSessionObjectType, sIsRelaySessionAttrName, &sIsRelaySessionAttr);
       
    RelaySession::Register();
    
    RelayOutput::Register();
    // Reflector session needs to setup some parameters too.
    ReflectorStream::Register();

    // Tell the server our name!
    static char* sModuleName = "QTSSRelayModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    return QTSS_NoErr;
}


QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{
    // Setup module utils
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);
    sSessionQueue = NEW OSQueue();
    sAnnouncedQueue = NEW OSQueue();
    sRTSPSourceInfoQueue = NEW OSQueue();
    sRTSPSessionIDQueue = NEW OSQueue();
    sPrefs = QTSSModuleUtils::GetModulePrefsObject(inParams->inModule);
    sServer = inParams->inServer;
    sAttributes = QTSSModuleUtils::GetModuleAttributesObject(inParams->inModule);
    
    qtss_sprintf(sResponseHeader, kResponseHeader, kVersionString, kBuildString);
    
    // Call helper class initializers
    // this function calls the ReflectorSession base class Initialize function
    RelaySession::Initialize(sAttributes);
            
#if QTSS_RELAY_EXTERNAL_MODULE
    // The reflector is dependent on a number of objects in the Common Utilities
    // library that get setup by the server if the reflector is internal to the
    // server.
    //
    // So, if the reflector is being built as a code fragment, it must initialize
    // those pieces itself
#if !MACOSXEVENTQUEUE
    ::select_startevents();//initialize the select() implementation of the event queue
#endif
    OS::Initialize();
    Socket::Initialize();
    SocketUtils::Initialize();

    const UInt32 kNumReflectorThreads = 8;
    TaskThreadPool::AddThreads(kNumReflectorThreads);
    IdleTask::Initialize();
    Socket::StartThread();
#endif

    //
    // Instead of passing our own module prefs object, as one might expect,
    // here we pass in the QTSSReflectorModule's, because the prefs that
    // apply to ReflectorStream are stored in that module's prefs
    StrPtrLen theReflectorModule("QTSSReflectorModule");
    QTSS_ModulePrefsObject theReflectorPrefs =
        QTSSModuleUtils::GetModulePrefsObject(QTSSModuleUtils::GetModuleObjectByName(theReflectorModule));

    // Call helper class initializers
    ReflectorStream::Initialize(theReflectorPrefs);
    RereadPrefs();

    return QTSS_NoErr;
}

QTSS_Error Shutdown()
{
#if QTSS_REFLECTOR_EXTERNAL_MODULE
    TaskThreadPool::RemoveThreads();
#endif
    return QTSS_NoErr;
}

QTSS_Error RereadPrefs()
{
    delete [] sRelayPrefs;
    delete [] sRelayStatsURL;
    
    sRelayPrefs = QTSSModuleUtils::GetStringAttribute(sPrefs, "relay_prefs_file", sDefaultRelayPrefs);
    sRelayStatsURL = QTSSModuleUtils::GetStringAttribute(sPrefs, "relay_stats_url", "");

    ReadRelayPrefsFile();
    return QTSS_NoErr;
}

RelaySession* CreateSession(SourceInfoQueueElem* inElem)
{
    RelaySession* theSession = NEW RelaySession(NULL, inElem->fSourceInfo);
    QTSS_Error theErr = theSession->SetupRelaySession(inElem->fSourceInfo);
    inElem->fShouldDelete = false; // SourceInfo will be deleted by the RelaySession
    if (theErr != QTSS_NoErr)
    {
        delete theSession;
        return NULL;
    }
    
    theSession->FormatHTML(NULL);
    
    sSessionQueue->EnQueue(theSession->GetQueueElem());
    return theSession;  
}

RelaySession* FindSession(SourceInfoQueueElem* inElem)
{
    OSQueueIter theIter(sSessionQueue);
    return FindNextSession(inElem, &theIter);
}

RelaySession* FindNextSession(SourceInfoQueueElem* inElem, OSQueueIter* inIterPtr)
{
    RelaySession* theSession = NULL;

    for ( ;!inIterPtr->IsDone(); inIterPtr->Next())
    {
        theSession = (RelaySession*)inIterPtr->GetCurrent()->GetEnclosingObject();
        if (theSession->Equal(inElem->fSourceInfo))
            return theSession;
    }
    
    return NULL;
}

void RemoveOutput(ReflectorOutput* inOutput, RelaySession* inSession)
{
    // This function removes the output from the RelaySession, then
    // checks to see if the session should go away. If it should, this deletes it
    inSession->RemoveOutput(inOutput,false);
    delete inOutput;
        
    if (inSession->GetNumOutputs() == 0)
    {
        sSessionQueue->Remove(inSession->GetQueueElem());
        delete inSession;
    }   
}

void RemoveAllOutputs(RelaySession* inSession)
{
    OSMutexLocker locker(RelayOutput::GetQueueMutex());
    for (OSQueueIter iter(RelayOutput::GetOutputQueue()); !iter.IsDone(); )
    {
        RelayOutput* theOutput = (RelayOutput*)iter.GetCurrent()->GetEnclosingObject();
        
        iter.Next();
                if(theOutput->GetRelaySession() == inSession)
                {   
                    inSession->RemoveOutput(theOutput,false);
                    delete theOutput;
                }
    }
    
    Assert(inSession->GetNumOutputs() == 0);
    sSessionQueue->Remove(inSession->GetQueueElem());
    delete inSession;
}

QTSS_Error Filter(QTSS_StandardRTSP_Params* inParams)
{
    UInt32 theLen = 0;
    char* theFullRequest = NULL;
    (void)QTSS_GetValuePtr(inParams->inRTSPRequest, qtssRTSPReqFullRequest, 0, (void**)&theFullRequest, &theLen);
    
    OSMutexLocker locker(RelayOutput::GetQueueMutex());

    // Check to see if this is a request we should handle
    if ((sRequestHeader.Ptr == NULL) || (sRequestHeader.Len == 0))
        return QTSS_NoErr;
    if ((theFullRequest == NULL) || (theLen < sRequestHeader.Len))
        return QTSS_NoErr;
    if (::memcmp(theFullRequest, sRequestHeader.Ptr, sRequestHeader.Len) != 0)
        return QTSS_NoErr;

    // Keep-alive should be off!
    Bool16 theFalse = false;
    (void)QTSS_SetValue(inParams->inRTSPRequest, qtssRTSPReqRespKeepAlive, 0, &theFalse, sizeof(theFalse));

    (void)QTSS_Write(inParams->inRTSPRequest, sResponseHeader, ::strlen(sResponseHeader), NULL, 0);
    
    // First build up a queue of all RelaySessions with RelayOutputs. This way,
    // we can present a list that's sorted.
    OSQueue theSessionQueue;
    FindRelaySessions(&theSessionQueue);
    
    static StrPtrLen    sNoRelays("<H2>There are no currently active relays</H2>");
    if (theSessionQueue.GetLength() == 0)
        (void)QTSS_Write(inParams->inRTSPRequest, sNoRelays.Ptr, sNoRelays.Len, NULL, 0);
    
    // Now go through our RelaySessionQueue, writing the info for each RelaySession,
    // and all the outputs associated with that session
    for (OSQueueIter iter(&theSessionQueue); !iter.IsDone(); iter.Next())
    {
        RelaySession* theSession = (RelaySession*)iter.GetCurrent()->GetEnclosingObject();
        (void)QTSS_Write(inParams->inRTSPRequest, theSession->GetSourceInfoHTML()->Ptr, theSession->GetSourceInfoHTML()->Len, NULL, 0);

        for (OSQueueIter iter2(RelayOutput::GetOutputQueue()); !iter2.IsDone(); iter2.Next())
        {
            RelayOutput* theOutput = (RelayOutput*)iter2.GetCurrent()->GetEnclosingObject();
            if (theSession == theOutput->GetRelaySession())
            {
                (void)QTSS_Write(inParams->inRTSPRequest, theOutput->GetOutputInfoHTML()->Ptr, theOutput->GetOutputInfoHTML()->Len, NULL, 0);

                // Write current stats for this output
                char theStatsBuf[1024];
                qtss_sprintf(theStatsBuf, "Current stats for this relay: %"_U32BITARG_" packets per second. %"_U32BITARG_" bits per second. %"_64BITARG_"d packets since it started. %"_64BITARG_"d bits since it started<P>", theOutput->GetCurPacketsPerSecond(), theOutput->GetCurBitsPerSecond(), theOutput->GetTotalPacketsSent(), theOutput->GetTotalBytesSent());
                (void)QTSS_Write(inParams->inRTSPRequest, &theStatsBuf[0], ::strlen(theStatsBuf), NULL, 0);
            }
        }
    }
    static StrPtrLen    sResponseEnd("</BODY></HTML>");
    (void)QTSS_Write(inParams->inRTSPRequest, sResponseEnd.Ptr, sResponseEnd.Len, NULL, 0);
    
    for (OSQueueIter iter3(&theSessionQueue); !iter3.IsDone(); )
    {
        // Cleanup the memory we had allocated in FindRelaySessions
        OSQueueElem* theElem = iter3.GetCurrent();
        iter3.Next();
        theSessionQueue.Remove(theElem);
        delete theElem;
    }
    return QTSS_NoErr;
}

void FindRelaySessions(OSQueue* inSessionQueue)
{
    for (OSQueueIter theIter(RelayOutput::GetOutputQueue()); !theIter.IsDone(); theIter.Next())
    {
        RelayOutput* theOutput = (RelayOutput*)theIter.GetCurrent()->GetEnclosingObject();
        Bool16 found = false;
        
        // Check to see if we've already seen this RelaySession
        for (OSQueueIter theIter2(inSessionQueue); !theIter2.IsDone(); theIter2.Next())
        {
            if (theOutput->GetRelaySession() == theIter2.GetCurrent()->GetEnclosingObject())
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            // We haven't seen this one yet, so put it on the queue.
            OSQueueElem* theElem = NEW OSQueueElem(theOutput->GetRelaySession());
            inSessionQueue->EnQueue(theElem);
        }
    }
}

QTSS_Error SetupAnnouncedSessions(QTSS_StandardRTSP_Params* inParams)
{

    QTSS_Error theErr = QTSS_NoErr;
    Bool16 setup = false;
    Bool16 play = false;
    
    // Get the RTSP Method
    QTSS_RTSPMethod* theMethod = NULL;
    UInt32 theLen = 0;
    theErr = QTSS_GetValuePtr(inParams->inRTSPRequest, qtssRTSPReqMethod, 0, (void**)&theMethod, &theLen);
    if ((theErr != QTSS_NoErr) || (theLen != sizeof(QTSS_RTSPMethod)))
    {
        Assert(0);
        return QTSS_NoErr;
    }
    
    // Get the SETUP Transport Mode
    QTSS_RTPTransportMode* theMode = NULL;
    theLen = 0;
    theErr = QTSS_GetValuePtr(inParams->inRTSPRequest, qtssRTSPReqTransportMode, 0, (void**)&theMode, &theLen);
    if ((theErr != QTSS_NoErr) || (theLen != sizeof(QTSS_RTPTransportMode)))
    {
        Assert(0);
        return QTSS_NoErr; 
    }   
        
    if ( (*theMethod == qtssSetupMethod) && (*theMode == qtssRTPTransportModeRecord) )
        setup = true;
    else if (*theMethod == qtssPlayMethod || *theMethod == qtssRecordMethod)
        play = true;
    
    // We are only interested in SETUP mode=receive or mode=record and PLAY or RECORD requests
    if (!(setup || play))
        return QTSS_NoErr;
    
    // Get the RTSP Status Code
    QTSS_RTSPStatusCode* theStatusCode = NULL;
    theLen = 0;
    theErr = QTSS_GetValuePtr(inParams->inRTSPRequest, qtssRTSPReqStatusCode, 0, (void**)&theStatusCode, &theLen);
    if ((theErr != QTSS_NoErr) || (theLen != sizeof(QTSS_RTSPStatusCode)))
    {
        Assert(0);
        return QTSS_NoErr;
    }
    
    // We are only interested in 200 OK requests, ofcourse
    if(*theStatusCode != qtssSuccessOK)
        return QTSS_NoErr;
     
    // Now get the RTSP Session ID
    UInt32* theSessionID = 0;
    theLen = 0;
    theErr = QTSS_GetValuePtr(inParams->inRTSPSession, qtssRTSPSesID, 0, (void**)&theSessionID, &theLen);
    if ((theErr != QTSS_NoErr) || (theLen != sizeof(UInt32)))
    {
        Assert(0);
        return QTSS_NoErr;
    }
    
    OSMutexLocker locker(RelayOutput::GetQueueMutex());
        
    RTSPSessionIDQueueElem* theElem = FindRTSPSessionIDQueueElem(*theSessionID);
    
    // If the RTSPSessionID is not in the queue
    if (theElem == NULL)
    {
        // If it's a SETUP mode=receive or record add the RTSPSessionID to the queue
        if(setup)
        {
            RTSPSessionIDQueueElem* idElem = NEW RTSPSessionIDQueueElem(*theSessionID);
            sRTSPSessionIDQueue->EnQueue(&idElem->fElem);
        }
        
        // Nothing more to do
        return QTSS_NoErr;
    }
    
    // If theElem is not NULL
    if(setup)                       // No need to add it to the queue twice
        return QTSS_NoErr; 
    
    // At this point it has to be a PLAY request with the session ID in the queue
    
    // Remove the session ID from the queue
    sRTSPSessionIDQueue->Remove(&theElem->fElem);
    delete theElem;  
    theElem = 0;
    
    // Get the Remote IP address
    UInt32* theIP = 0;
    theLen = 0;
    theErr = QTSS_GetValuePtr(inParams->inRTSPSession, qtssRTSPSesRemoteAddr, 0, (void**)&theIP, &theLen);
    if ((theErr != QTSS_NoErr) || (theLen != sizeof(UInt32)))
    {
        Assert(0);
        return QTSS_NoErr;
    }
    
    // Get the URI
    theLen = 0;
    char* theURLStr = NULL;
    theErr = QTSS_GetValuePtr(inParams->inRTSPRequest, qtssRTSPReqURI, 0, (void**)&theURLStr, &theLen);
    if ((theErr != QTSS_NoErr) || (theURLStr == NULL))
    {
        Assert(0);
        return QTSS_NoErr;
    }
    
    StrPtrLen theURL(theURLStr, theLen);
    
    // Check if a source info exists for this ANNOUNCED URL
    RTSPSourceInfo* info = FindAnnouncedSourceInfo(*theIP, theURL);
    if(info == NULL)            // if a source info does not exists, we don't have to do anything
        return QTSS_NoErr;
    
    // Check if a relay has already been set up for this
    // if it has, remove all the outputs, delete the session
    // before creating a new source info etc...
    RTSPSourceInfo* existingInfo = FindExistingRelayInfo(*theIP, theURL);
    if(existingInfo != NULL)
    {
        RelaySession* session = existingInfo->GetRelaySession();
        if(session == NULL)
            delete existingInfo;
        else
            RemoveAllOutputs(session);
    }
    
    RTSPSourceInfo* newInfo = NEW RTSPSourceInfo(*info);
    newInfo->SetAnnounceActualIP(*theIP); // this is used later along with the source url to check if a relay is already set up
    
    UInt16 thePort = 0;
    theLen = sizeof(UInt16);
    theErr = QTSS_GetValue(sServer, qtssSvrRTSPPorts, 0, (void *) &thePort, &theLen);
    
    if ((theErr != QTSS_NoErr) || (theLen != sizeof(UInt16)))
    {
        delete newInfo;
        return QTSS_NoErr;
    }
        
    newInfo->SetSourceParameters(INADDR_LOOPBACK, thePort, theURL);
    newInfo->StartSessionCreatorTask(sSessionQueue, sRTSPSourceInfoQueue);
    
    return QTSS_NoErr;
}

RTSPSessionIDQueueElem* FindRTSPSessionIDQueueElem(UInt32 inSessionID)
{
    for (OSQueueIter iter(sRTSPSessionIDQueue); !iter.IsDone(); iter.Next())
    {
        RTSPSessionIDQueueElem* theElem = (RTSPSessionIDQueueElem*)iter.GetCurrent()->GetEnclosingObject();
        
        if (theElem->fRTSPSessionID == inSessionID)
            return theElem;
    }
    
    return NULL;
}

RTSPSourceInfo* FindAnnouncedSourceInfo(UInt32 inIP, StrPtrLen& inURL)
{
    RTSPSourceInfo* info = NULL;
    Bool16 ipMatchfound = false;

    // check to see if the ip address + URL match any of the source infos in the sAnnouncedQueue.
    // in this order:
    // 1. IP + URL match
    // 2. IP match
    // 3. All IPs+URLs match (meaning all announced broadcasts)
    
    for (OSQueueIter iter(sAnnouncedQueue); !iter.IsDone(); iter.Next())
    {
        SourceInfo* theInfo = ((SourceInfoQueueElem*)iter.GetCurrent()->GetEnclosingObject())->fSourceInfo;
        
//      RTSPSourceInfo* announceInfo = dynamic_cast<RTSPSourceInfo *>(theInfo);
        RTSPSourceInfo* announceInfo = (RTSPSourceInfo *)(theInfo);
//      if ((announceInfo == NULL) || (!announceInfo->IsAnnounce()))
//          continue;
    
        UInt32 infoIP = announceInfo->GetAnnounceIP();
        StrPtrLen infoURL(announceInfo->GetAnnounceURL());  
        
        // if there is a source info for ANNOUNCE ALL, keep this info until a better one is found
        if (!ipMatchfound && (infoIP == 0) && ((infoURL.Ptr == NULL) || (infoURL.Len == 0)))
        {
            info = announceInfo;
            continue;
        }
        
        // if there is a source info for ANNOUNCE any URL from an IP, keep this info until a better one is found
        if ((infoIP == inIP) && ((infoURL.Ptr == NULL) || (infoURL.Len == 0)))
        {
            info = announceInfo;
            ipMatchfound = true;
            continue;
        }
        
        // if there is a source info for ANNOUNCE for the URL + IP combo, return this as the source info
        // strip leading path delimiters 
        UInt32 count = 0;
        while (count < inURL.Len && inURL.Ptr[count] == '/')
        {   count ++;
        }
        StrPtrLen announcedURL(&inURL.Ptr[count],inURL.Len - count);

        // strip leading path delimiters 
        count = 0;
        while (count < infoURL.Len && infoURL.Ptr[count] == '/')
        {   count ++;
        }
        StrPtrLen configMountPoint(&infoURL.Ptr[count],infoURL.Len - count);
        
        if ((infoIP == inIP) && (configMountPoint.Equal(announcedURL)))
        {
            info = announceInfo;
            break;
        }   
    }
    
    // cast the source info to RTSPSourceInfo because all the elements of the sAnnouncedQueue
    // are RTSPSourceInfos anyway.
    // Don't really like this but I suppose it will do for now
    return info;
}

RTSPSourceInfo* FindExistingRelayInfo(UInt32 inIP, StrPtrLen& inURL)
{   
    if (inURL.Ptr == NULL)
        return NULL;
        
    for (OSQueueIter iter(sRTSPSourceInfoQueue); !iter.IsDone(); iter.Next())
    {       
        RTSPSourceInfo* info = (RTSPSourceInfo*)iter.GetCurrent()->GetEnclosingObject();
    
        if ((inIP == info->GetAnnounceActualIP()) && inURL.Equal(info->GetSourceURL()))
            return info;
    }
    
    return NULL;
}

void RereadRelayPrefs(XMLParser* prefsParser)
{
    // Construct a SourceInfo object for each relayable thingy
    OSQueue sourceInfoQueue;
    ClearSourceInfos(sAnnouncedQueue);
    FindSourceInfos(&sourceInfoQueue, sAnnouncedQueue, prefsParser);
    
    // Ok, we have all our SourceInfo objects. Now, lets alter the list of Relay
    // outputs to match
    
    // Go through the queue of Relay outputs, removing the source Infos that
    // are already going
    OSMutexLocker locker(RelayOutput::GetQueueMutex());

    // We must reread the relay stats URL here, because we have the RelayOutput
    // mutex so we know that Filter can't be filtering now
    delete [] sRequestHeader.Ptr;
    sRequestHeader.Ptr = NULL;
    sRequestHeader.Len = 0;

    if ((sRelayStatsURL != NULL) && (::strlen(sRelayStatsURL) > 0))
    {
        // sRequestHeader line should look like: GET /sRelayStatsURL HTTP
        sRequestHeader.Ptr = NEW char[::strlen(sRelayStatsURL) + 15];
        ::strcpy(sRequestHeader.Ptr, "GET /");
        ::strcat(sRequestHeader.Ptr, sRelayStatsURL);
        ::strcat(sRequestHeader.Ptr, " HTTP");
        sRequestHeader.Len = ::strlen(sRequestHeader.Ptr);
    }

    for (OSQueueIter iter(RelayOutput::GetOutputQueue()); !iter.IsDone(); )
    {
        RelayOutput* theOutput = (RelayOutput*)iter.GetCurrent()->GetEnclosingObject();
        
        Bool16 stillActive = false;
        
        // Check to see if this RelayOutput matches one of the streams in the
        // SourceInfo queue. If it does, this has 2 implications:
        //
        // 1) The stream in the SourceInfo that matches should NOT be setup as a
        // new RelayOutput, because it already exists. (Equal will set a flag in
        // the StreamInfo object saying as much)
        //
        // 2) This particular RelayOutput should remain (not be deleted).
        for (OSQueueIter iter2(&sourceInfoQueue); !iter2.IsDone(); iter2.Next())
        {
            SourceInfo* theInfo =
                ((SourceInfoQueueElem*)iter2.GetCurrent()->GetEnclosingObject())->fSourceInfo;
            if (theOutput->Equal(theInfo))
            {
                stillActive = true;
                break;
            }
        }
        
        // Also check if this RelayOutput matches one of the streams in the 
        // AnnouncedInfo queue. If it does, the implications are same as above.
        // Perform this check only if it is not already found in the first queue
        if(!stillActive) 
        {
            for (OSQueueIter iter3(sAnnouncedQueue); !iter3.IsDone(); iter3.Next())
            {
                SourceInfo* theInfo =
                    ((SourceInfoQueueElem*)iter3.GetCurrent()->GetEnclosingObject())->fSourceInfo;
                if (theOutput->Equal(theInfo))
                {
                    stillActive = true;
                    break;
                }
            }
        }
        
        iter.Next();    

        // This relay output is no longer on our list
        if (!stillActive)
            RemoveOutput(theOutput, theOutput->GetRelaySession());
    }
    
    // We've pruned the list of RelayOutputs of all outputs that are no longer
    // going on. Now, all that is left to do is to create any new outputs (and sessions)
    // that are just starting up
    
    for (OSQueueIter iter4(&sourceInfoQueue); !iter4.IsDone(); iter4.Next())
    {
        SourceInfoQueueElem* theElem = (SourceInfoQueueElem*)iter4.GetCurrent()->GetEnclosingObject();
        if (theElem->fSourceInfo->GetNumNewOutputs() == 0)  // Because we've already checked for Outputs
            continue;                           // That already exist, we know which ones are new
        
        RelaySession* theSession = FindSession(theElem);
        
        if (theSession == NULL)
        {
            if (theElem->fIsRTSPSourceInfo)
            {
                theElem->fShouldDelete = false;
//              RTSPSourceInfo* theInfo = dynamic_cast<RTSPSourceInfo *>(theElem->fSourceInfo);
                RTSPSourceInfo* theInfo = (RTSPSourceInfo *)(theElem->fSourceInfo);
                theInfo->StartSessionCreatorTask(sSessionQueue, sRTSPSourceInfoQueue);
            }
            else
            {
                theSession = CreateSession(theElem);
                if (theSession != NULL)
                    AddOutputs(theElem->fSourceInfo, theSession, theElem->fIsRTSPSourceInfo);
            }       
        }
        else
        {
            AddOutputs(theElem->fSourceInfo, theSession, theElem->fIsRTSPSourceInfo);
        }
    }
    
    // For the announced source infos, find a session that is already active, and add any
    // output that isn't already set up. If a session isn't found, don't create it!
    for (OSQueueIter iter5(sAnnouncedQueue); !iter5.IsDone(); iter5.Next())
    {
        SourceInfoQueueElem* theElem = (SourceInfoQueueElem*)iter5.GetCurrent()->GetEnclosingObject();
        if (theElem->fSourceInfo->GetNumNewOutputs() == 0)  // Because we've already checked for Outputs
            continue;                           // That already exist, we know which ones are new
            
        for (OSQueueIter iter6(sSessionQueue); !iter6.IsDone(); )
        {
        
            RelaySession* theSession = FindNextSession(theElem, &iter6);
            if(!iter6.IsDone())
                iter6.Next();
                
            if (theSession == NULL)
                continue;

            AddOutputs(theElem->fSourceInfo, theSession, theElem->fIsRTSPSourceInfo);   
        }
    }
    
    // Clear only the first source info queue. The announced source info queue
    // must be kept around so that we know which announced broadcasts to relay
    ClearSourceInfos(&sourceInfoQueue);
}

void AddOutputs(SourceInfo* inInfo, RelaySession* inSession, Bool16 inIsRTSPSourceInfo)
{
    for (UInt32 x = 0; x < inInfo->GetNumOutputs(); x++)
    {
        SourceInfo::OutputInfo* theOutputInfo = inInfo->GetOutputInfo(x);
        if (theOutputInfo->fAlreadySetup)
            continue;
                
        RelayOutput* theOutput = NEW RelayOutput(inInfo, x, inSession, inIsRTSPSourceInfo);
        if (theOutput->IsValid())
            inSession->AddOutput(theOutput, false);
        else
            delete theOutput;
    }
}

void FindSourceInfos(OSQueue* inSessionQueue, OSQueue* inAnnouncedQueue, XMLParser* prefsParser)
{
    SourceInfoQueueElem* theElem = NULL;
        
    XMLTag* tag = prefsParser->GetRootTag();
    XMLTag* relayTag;
    UInt32 index = 0;
    while ((relayTag = tag->GetEmbeddedTagByNameAndAttr("OBJECT", "TYPE", "relay", index)) != NULL)
    {
        index++;
        
        XMLTag* enabledTag = relayTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "enabled");
        if ((enabledTag != NULL) && (enabledTag->GetValue() != NULL) && !strcmp(enabledTag->GetValue(), "false"))
            continue;   // this relay pref is disabled
        
        XMLTag* sourceTag = relayTag->GetEmbeddedTagByNameAndAttr("OBJECT", "CLASS", "source");
        if (sourceTag == NULL)
            continue;

        char* type = sourceTag->GetAttributeValue("TYPE");
        if (type == NULL)
            continue;

        if (!strcmp(type, "relay_sdp"))
        {
            XMLTag* fileNameTag = sourceTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "sdp_path");
            if (!fileNameTag)
                continue;
                
            char* fileName = fileNameTag->GetValue();
            if (fileName == NULL)
                continue;

            StrPtrLen theFileData;
            (void)QTSSModuleUtils::ReadEntireFile(fileName, &theFileData);
            if (theFileData.Len > 0) // There is a Relay SDP file!
            {
                RelaySDPSourceInfo* theInfo = NEW RelaySDPSourceInfo(&theFileData);

                // Only keep this sdp file around if there are input streams &
                // output streams described in it.
                if ((theInfo->GetNumStreams() == 0) || (theInfo->GetNumOutputs() == 0))
                    delete theInfo;
                else
                {
                    theElem = NEW SourceInfoQueueElem(theInfo, false);
                    inSessionQueue->EnQueue(&theElem->fElem);
                }
                delete [] theFileData.Ptr;// We don't need the file data anymore
            }
        }
        else if (!strcmp(type, "udp_source"))
        {
            RCFSourceInfo* theRCFInfo = NEW RCFSourceInfo(relayTag);
            // Only keep this sdp file around if there are input streams &
            // output streams described in it.
            if ((theRCFInfo->GetNumStreams() == 0) || (theRCFInfo->GetNumOutputs() == 0))
                delete theRCFInfo;
            else
            {
                theElem = NEW SourceInfoQueueElem(theRCFInfo, false);
                inSessionQueue->EnQueue(&theElem->fElem);
            }
        }
        else if (!strcmp(type, "rtsp_source"))
        {
            // This is an rtsp source, so go through the describe, setup, play connect
            // process before moving on.
            RTSPSourceInfo* theRTSPInfo = NEW RTSPSourceInfo(0);
            QTSS_Error theErr = theRTSPInfo->ParsePrefs(relayTag, false);
            if (theErr == QTSS_NoErr)
            {
                // We have to do this *after* doing the DESCRIBE because parsing
                // the relay destinations depends on information in the SDP
                theRTSPInfo->ParseRelayDestinations(relayTag);
                
                if (theRTSPInfo->GetNumOutputs() == 0)
                    delete theRTSPInfo;
                else
                {
                    theElem = NEW SourceInfoQueueElem(theRTSPInfo, true);
                    inSessionQueue->EnQueue(&theElem->fElem);
                }
            }
            else
                delete theRTSPInfo;
        }
        else if (!strcmp(type, "announced_source"))
        {
            // This is an announced rtsp source, so just set up an RTSPSourceInfo object and parse
            // through the destinations. Leave the rest for later.
            RTSPSourceInfo* theRTSPInfo = NEW RTSPSourceInfo(true);
            QTSS_Error theErr = theRTSPInfo->ParsePrefs(relayTag, true);
            if (theErr == QTSS_NoErr)
            {
                theRTSPInfo->ParseRelayDestinations(relayTag);
                
                if (theRTSPInfo->GetNumOutputs() == 0)
                    delete theRTSPInfo;
                else
                {
                    theElem = NEW SourceInfoQueueElem(theRTSPInfo, true);
                    // put this sourceinfo elem on the global sAnnouncedQueue so that
                    // its session can be set up when an ANNOUNCE is received
                    inAnnouncedQueue->EnQueue(&theElem->fElem);
                }
            }
            else
                delete theRTSPInfo;
        }
    }
}

void ClearSourceInfos(OSQueue* inQueue)
{
    for (OSQueueIter theIter(inQueue); !theIter.IsDone(); )
    {
        // Get the current queue element, and make sure to move onto the
        // next one, as the current one is going away.
        SourceInfoQueueElem* theElem = (SourceInfoQueueElem*)theIter.GetCurrent()->GetEnclosingObject();

        theIter.Next();
    
        inQueue->Remove(&theElem->fElem);
        
        // If we're supposed to delete this SourceInfo, then do so
        if (theElem->fShouldDelete)
            delete theElem->fSourceInfo;
        delete theElem;
    }
}

QTSS_Error RouteAuthorization(QTSS_StandardRTSP_Params* inParams)
{
    QTSS_Error theErr = QTSS_NoErr;
    UInt32 theLen = 0;
    
    // Get the Remote IP address
    UInt32* theIP = 0;
    theErr = QTSS_GetValuePtr(inParams->inRTSPSession, qtssRTSPSesRemoteAddr, 0, (void**)&theIP, &theLen);
    if ((theErr != QTSS_NoErr) || (theLen != sizeof(UInt32)))
    {
        Assert(0);
        return QTSS_NoErr;
    }
    
    if (*theIP != INADDR_LOOPBACK)
        return QTSS_NoErr;
        
    // Get the Remote Port
    UInt16* thePort = 0;
    theLen = 0;
    theErr = QTSS_GetValuePtr(inParams->inRTSPSession, qtssRTSPSesRemotePort, 0, (void**)&thePort, &theLen);
    if ((theErr != QTSS_NoErr) || (theLen != sizeof(UInt16)))
    {
        Assert(0);
        return QTSS_NoErr;
    }
    
    // Check to see if any of the outputs has an announced source
    // whose source URI and request port match theURL and thePort
    if (IsRelayRequest(*thePort))
    {
        // ask server to skip authorization
        if (QTSS_NoErr != QTSS_SetValue(inParams->inRTSPRequest,qtssRTSPReqSkipAuthorization, 0, &sSkipAuthorization, sizeof(sSkipAuthorization)))
            return QTSS_RequestFailed; // bail on the request if the SetValue call fails!
    
        // set the client session QTSSRelayModuleIsRelaySession attribute to true
                 if (QTSS_NoErr != QTSS_SetValue(inParams->inClientSession, sIsRelaySessionAttr, 0,(void *)&sIsRelaySession, sizeof(sIsRelaySession)))
            return QTSS_RequestFailed; // bail on the request if the SetValue call fails!                  
    }
                
    return QTSS_NoErr;
}

Bool16 IsRelayRequest(UInt16 inPort)
{
    for (OSQueueIter iter(sRTSPSourceInfoQueue); !iter.IsDone(); iter.Next())
    {       
        RTSPSourceInfo* info = (RTSPSourceInfo*)iter.GetCurrent()->GetEnclosingObject();

        if (inPort == (info->GetClientSocket())->GetLocalPort())
            return true;
    }
    
    return false;
}

void ReadRelayPrefsFile()
{
    // check to see if file has changed
    struct stat buf;
    if (::stat(sRelayPrefs, &buf) >= 0)
    {
        if (sRelayPrefModDate == buf.st_mtime)
            return;
            
        sRelayPrefModDate = buf.st_mtime;
    }
    
    XMLParser* xmlFile = new XMLParser(sRelayPrefs);
    
    if (!xmlFile->DoesFileExist())
    {   
        delete xmlFile;
        return;     // just return with no error logged
    }
      
    char errorBuffer[1000];
    if (!xmlFile->ParseFile(errorBuffer, sizeof(errorBuffer)))
    {
        // log this error to the error log
        QTSSModuleUtils::LogError(  qtssWarningVerbosity, sRelayModulePrefParseErr, 0, NULL, NULL);
        delete xmlFile;
        return;     // file was invalid, so no source infos
    }
    
    if (!CheckDNSNames(xmlFile, false))
    {
        // do reread prefs stuff now
        RereadRelayPrefs(xmlFile);
        delete xmlFile;
        return;
    }
    
    // otherwise we need to do the DNS resolution on another thread
    DNSResolverThread::ResolveRelayPrefs(xmlFile); // will keep or delete xmlFile
}

Bool16 CheckDNSNames(XMLParser* prefsFile, Bool16 doResolution)
{
    XMLTag* tag = prefsFile->GetRootTag();
    XMLTag* relayTag;
    XMLTag* prefTag;
    UInt32 index = 0;
    while ((relayTag = tag->GetEmbeddedTagByNameAndAttr("OBJECT", "TYPE", "relay", index)) != NULL)
    {
        index++;
        
        XMLTag* enabledTag = relayTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "enabled");
        if ((enabledTag != NULL) && (enabledTag->GetValue() != NULL) && !strcmp(enabledTag->GetValue(), "false"))
            continue;   // this relay pref is disabled
        
        XMLTag* sourceTag = relayTag->GetEmbeddedTagByNameAndAttr("OBJECT", "CLASS", "source");
        if (sourceTag == NULL)
            continue;
            
        prefTag = sourceTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "in_addr");
        if (prefTag != NULL)
        {
            char* destAddrStr = prefTag->GetValue();
            if (destAddrStr  != NULL)
                if (SocketUtils::ConvertStringToAddr(destAddrStr) == INADDR_NONE)
                {
                    if (doResolution)
                        ResolveDNSAddr(prefTag);
                    else
                        return true;
                }
        }
        prefTag = sourceTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "source_addr");
        if (prefTag != NULL)
        {
            char* srcAddrStr = prefTag->GetValue();
            if (srcAddrStr != NULL)
                if (SocketUtils::ConvertStringToAddr(srcAddrStr) == INADDR_NONE)
                {
                    if (doResolution)
                        ResolveDNSAddr(prefTag);
                    else
                        return true;
                }
        }
        
        UInt32 numTags = relayTag->GetNumEmbeddedTags();
        for (UInt32 y = 0; y < numTags; y++)
        {
            XMLTag* destTag = relayTag->GetEmbeddedTagByNameAndAttr("OBJECT", "CLASS", "destination", y);
            if (destTag == NULL)
                break;
                
            prefTag = destTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "out_addr");
            if (prefTag != NULL)
            {
                char* outAddrStr = prefTag->GetValue();
                if (outAddrStr  != NULL)
                    if (SocketUtils::ConvertStringToAddr(outAddrStr) == INADDR_NONE)
                    {
                        if (doResolution)
                            ResolveDNSAddr(prefTag);
                        else
                            return true;
                    }
            }
            prefTag = destTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "dest_addr");
            if (prefTag != NULL)
            {
                char* destAddrStr = prefTag->GetValue();
                if (destAddrStr  != NULL)
                    if (SocketUtils::ConvertStringToAddr(destAddrStr) == INADDR_NONE)
                    {
                        if (doResolution)
                            ResolveDNSAddr(prefTag);
                        else
                            return true;
                    }
            }
        }
    }
    
    return false;
}

void ResolveDNSAddr(XMLTag* tag)
{
    struct in_addr inAddr;
    struct hostent* theHostent = ::gethostbyname(tag->GetValue());      
    if (theHostent != NULL)
    {
        char buffer[50];
        StrPtrLen temp(buffer);
        inAddr.s_addr = *(UInt32*)(theHostent->h_addr_list[0]);
        SocketUtils::ConvertAddrToString(inAddr, &temp);
        tag->SetValue(buffer);
    }
}

