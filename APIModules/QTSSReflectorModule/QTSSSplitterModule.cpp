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
    File:       QTSSSplitterModule.cpp

    Contains:   Implementation of QTSSSplitterModule class. 
                    
    
    
    

*/

#include "QTSSSplitterModule.h"
#include "QTSSModuleUtils.h"
#include "ReflectorSession.h"
#include "OSArrayObjectDeleter.h"
#include "OSMemory.h"

//ReflectorOutput objects
#include "RTPSessionOutput.h"
#include "RelayOutput.h"

//SourceInfo objects
#include "RTSPSourceInfo.h"


// ATTRIBUTES

static QTSS_AttributeID         sOutputAttr                 = qtssIllegalAttrID;
static QTSS_AttributeID         sSessionAttr                = qtssIllegalAttrID;
static QTSS_AttributeID         sStreamCookieAttr           =   qtssIllegalAttrID;

static QTSS_AttributeID         sRemoteHostRespondedWithAnErrorErr  = qtssIllegalAttrID;
static QTSS_AttributeID         sRemoteHostRefusedConnectionErr     = qtssIllegalAttrID;
static QTSS_AttributeID         sExpectedDigitFilenameErr           = qtssIllegalAttrID;
static QTSS_AttributeID         sBadTrackIDErr                      = qtssIllegalAttrID;

// STATIC DATA

static const UInt32 kSessionStartingIdleTimeInMsec =    20;
static const StrPtrLen      kCacheControlHeader("no-cache");
static QTSS_PrefsObject sServerPrefs = NULL;

static OSRefTable*              sSessionMap             = NULL;

// Important strings
static StrPtrLen    sRCFSuffix(".rcf");
static StrPtrLen    sRTSPSourceStr("relay_source");

// FUNCTION PROTOTYPES

static QTSS_Error QTSSSplitterModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error Register(QTSS_Register_Params* inParams);
static QTSS_Error Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error Shutdown();
static QTSS_Error ProcessRTSPRequest(QTSS_StandardRTSP_Params* inParams);
static QTSS_Error DoDescribe(QTSS_StandardRTSP_Params* inParams);
static ReflectorSession* FindOrCreateSession(StrPtrLen* inPath, QTSS_StandardRTSP_Params* inParams);
static QTSS_Error HandleSourceInfoErr(QTSS_Error rtspSourceInfoErr, QTSS_StandardRTSP_Params* inParams,
                                ReflectorSession* inSession, RTSPClient* inClient);
static void     DeleteSessionOnError(ReflectorSession* inSession, QTSS_ClientSessionObject inCliSession);
static void     NullOutSessionAttr(QTSS_ClientSessionObject inSession);
static QTSS_Error DoSetup(QTSS_StandardRTSP_Params* inParams, ReflectorSession* inSession);
static QTSS_Error DoPlay(QTSS_StandardRTSP_Params* inParams, ReflectorSession* inSession);
static QTSS_Error DestroySession(QTSS_ClientSessionClosing_Params* inParams);
static void IssueTeardown(ReflectorSession* inSession);
static void RequestSocketEvent(QTSS_StreamRef inStream, UInt32 inEventMask);

// FUNCTION IMPLEMENTATIONS

QTSS_Error QTSSSplitterModule_Main(void* inPrivateArgs)
{
    return _stublibrary_main(inPrivateArgs, QTSSSplitterModuleDispatch);
}


QTSS_Error  QTSSSplitterModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
    switch (inRole)
    {
        case QTSS_Register_Role:
            return Register(&inParams->regParams);
        case QTSS_Initialize_Role:
            return Initialize(&inParams->initParams);
        case QTSS_RTSPPreProcessor_Role:
            return ProcessRTSPRequest(&inParams->rtspRequestParams);
        case QTSS_ClientSessionClosing_Role:
            return DestroySession(&inParams->clientSessionClosingParams);
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
    (void)QTSS_AddRole(QTSS_RTSPPreProcessor_Role);
    (void)QTSS_AddRole(QTSS_ClientSessionClosing_Role);
    
    // Add text messages attributes
    static char*        sRemoteHostRespondedWithAnErrorName     = "QTSSSplitterModuleRemoteHostError";
    static char*        sRemoteHostRefusedConnectionName        = "QTSSSplitterModuleRemoteHostRefused";
    static char*        sExpectedDigitFilenameName              = "QTSSSplitterModuleExpectedDigitFilename";
    static char*        sBadTrackIDErrName                      = "QTSSSplitterModuleBadTrackID";

    (void)QTSS_AddStaticAttribute(qtssTextMessagesObjectType, sRemoteHostRespondedWithAnErrorName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssTextMessagesObjectType, sRemoteHostRespondedWithAnErrorName, &sRemoteHostRespondedWithAnErrorErr);

    (void)QTSS_AddStaticAttribute(qtssTextMessagesObjectType, sRemoteHostRefusedConnectionName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssTextMessagesObjectType, sRemoteHostRefusedConnectionName, &sRemoteHostRefusedConnectionErr);

    (void)QTSS_AddStaticAttribute(qtssTextMessagesObjectType, sExpectedDigitFilenameName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssTextMessagesObjectType, sExpectedDigitFilenameName, &sExpectedDigitFilenameErr);

    (void)QTSS_AddStaticAttribute(qtssTextMessagesObjectType, sBadTrackIDErrName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssTextMessagesObjectType, sBadTrackIDErrName, &sBadTrackIDErr);

    // Add an RTP session attribute for tracking ReflectorSession objects
    static char*        sOutputName = "QTSSSplitterModuleOutput";
    static char*        sSessionName= "QTSSSplitterModuleSession";
    static char*        sStreamCookieName   = "QTSSSplitterModuleStreamCookie";

    (void)QTSS_AddStaticAttribute(qtssClientSessionObjectType, sOutputName, NULL, qtssAttrDataTypeVoidPointer);
    (void)QTSS_IDForAttr(qtssClientSessionObjectType, sOutputName, &sOutputAttr);

    (void)QTSS_AddStaticAttribute(qtssClientSessionObjectType, sSessionName, NULL, qtssAttrDataTypeVoidPointer);
    (void)QTSS_IDForAttr(qtssClientSessionObjectType, sSessionName, &sSessionAttr);

    (void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sStreamCookieName, NULL, qtssAttrDataTypeVoidPointer);
    (void)QTSS_IDForAttr(qtssRTPStreamObjectType, sStreamCookieName, &sStreamCookieAttr);

    // Reflector stream needs to setup some parameters too.
    ReflectorStream::Register();
    // RTPSessionOutput needs to do the same
    RTPSessionOutput::Register();
    
    // Tell the server our name!
    static char* sModuleName = "QTSSSplitterModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    return QTSS_NoErr;
}


QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{
    // Setup module utils
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);
    sSessionMap = NEW OSRefTable();
    sServerPrefs = inParams->inPrefs;
    
    //
    // Instead of passing our own module prefs object, as one might expect,
    // here we pass in the QTSSReflectorModule's, because the prefs that
    // apply to ReflectorStream are stored in that module's prefs
    StrPtrLen theReflectorModule("QTSSReflectorModule");
    QTSS_ModulePrefsObject theReflectorPrefs =
        QTSSModuleUtils::GetModulePrefsObject(QTSSModuleUtils::GetModuleObjectByName(theReflectorModule));

    // Call helper class initializers
    ReflectorStream::Initialize(theReflectorPrefs);
    ReflectorSession::Initialize();
        
    // Report to the server that this module handles DESCRIBE, SETUP, PLAY, PAUSE, and TEARDOWN
    static QTSS_RTSPMethod sSupportedMethods[] = { qtssDescribeMethod, qtssSetupMethod, qtssTeardownMethod, qtssPlayMethod, qtssPauseMethod };
    QTSSModuleUtils::SetupSupportedMethods(inParams->inServer, sSupportedMethods, 5);

    return QTSS_NoErr;
}

QTSS_Error Shutdown()
{
    return QTSS_NoErr;
}

QTSS_Error ProcessRTSPRequest(QTSS_StandardRTSP_Params* inParams)
{
    QTSS_RTSPMethod* theMethod = NULL;
    UInt32 theLen = 0;
    if ((QTSS_GetValuePtr(inParams->inRTSPRequest, qtssRTSPReqMethod, 0,
            (void**)&theMethod, &theLen) != QTSS_NoErr) || (theLen != sizeof(QTSS_RTSPMethod)))
    {
        Assert(0);
        return QTSS_RequestFailed;
    }

    if (*theMethod == qtssDescribeMethod)
        return DoDescribe(inParams);
        
    RTPSessionOutput** theOutput = NULL;
    QTSS_Error theErr = QTSS_GetValuePtr(inParams->inClientSession, sOutputAttr, 0, (void**)&theOutput, &theLen);
    if ((theErr != QTSS_NoErr) || (theLen != sizeof(RTPSessionOutput*)))
        return QTSS_RequestFailed;
    
    switch (*theMethod)
    {
        case qtssSetupMethod:
            return DoSetup(inParams, (*theOutput)->GetReflectorSession());
        case qtssPlayMethod:
            return DoPlay(inParams, (*theOutput)->GetReflectorSession());
        case qtssTeardownMethod:
            // Tell the server that this session should be killed, and send a TEARDOWN response
            (void)QTSS_Teardown(inParams->inClientSession);
            (void)QTSS_SendStandardRTSPResponse(inParams->inRTSPRequest, inParams->inClientSession, 0);
            break;
        case qtssPauseMethod:
            (void)QTSS_Pause(inParams->inClientSession);
            (void)QTSS_SendStandardRTSPResponse(inParams->inRTSPRequest, inParams->inClientSession, 0);
            break;
        default:
            break;
    }           
    return QTSS_NoErr;
}


QTSS_Error DoDescribe(QTSS_StandardRTSP_Params* inParams)
{
    QTSS_Error theErr = QTSS_NoErr;
    
    // If this URL doesn't end with an .rcf, don't even bother. Ah, if only the QTSSReflectorModule
    // could make this same check as well
    StrPtrLen theURI;
    (void)QTSS_GetValuePtr(inParams->inRTSPRequest, qtssRTSPReqURI, 0, (void**)&theURI.Ptr, &theURI.Len);
    if ((theURI.Len < sRCFSuffix.Len) || (!sRCFSuffix.EqualIgnoreCase(&theURI.Ptr[theURI.Len - sRCFSuffix.Len], sRCFSuffix.Len)))
        return QTSS_NoErr;

    // Check and see if we are in the process of setting this connection up already
    ReflectorSession* theSession = NULL;
    UInt32 theLen = sizeof(ReflectorSession*);
    (void)QTSS_GetValue(inParams->inClientSession, sSessionAttr, 0, (void*)&theSession, &theLen);

    if (theSession == NULL)
    {
        // If we are not already in the process of initializing and setting up this ReflectorSession,
        // attempt to create one or attach to one.

        // Check and see if the full path to this file matches an existing ReflectorSession
        StrPtrLen thePathPtr;
        OSCharArrayDeleter rcfPath(QTSSModuleUtils::GetFullPath(    inParams->inRTSPRequest,
                                                                    qtssRTSPReqFilePath,
                                                                    &thePathPtr.Len, NULL));
        
        thePathPtr.Ptr = rcfPath.GetObject();
        theSession = FindOrCreateSession(&thePathPtr, inParams);
        // If this function returned an error, this request shouldn't be handled by this module
        if (theSession == NULL)
            return QTSS_NoErr;
    }
    if (!((RTSPSourceInfo*)theSession->GetSourceInfo())->IsDescribeComplete())
    {
        // Only the session owner need worry about this code. this is an easy.
        // Way of checking this.
        Assert(theLen == sizeof(ReflectorSession*));
        
        // If we haven't finished the describe yet, call
        // Describe again on the RTSPSourceInfo object.
        QTSS_Error theErr = ((RTSPSourceInfo*)theSession->GetSourceInfo())->Describe();
        if (theErr != QTSS_NoErr)
            return HandleSourceInfoErr(theErr, inParams, theSession,
                                        ((RTSPSourceInfo*)theSession->GetSourceInfo())->GetRTSPClient());
        else
        {
            // Describe has completed. At this point we can setup the ReflectorSession.
            // However, tell it not to consider this session completely setup yet, as 
            theErr = theSession->SetupReflectorSession(theSession->GetSourceInfo(), inParams, ReflectorSession::kDontMarkSetup);
            if (theErr != QTSS_NoErr)
            {
                // If we get an error here, for some reason we couldn't bind the ports, etc, etc.
                // Just abort
                DeleteSessionOnError(theSession, inParams->inClientSession);
                return theErr;
            }
        }
    }
    
    if (!theSession->IsSetup())
    {
        // Only the session owner need worry about this code. this is an easy.
        // Way of checking this.
        Assert(theLen == sizeof(ReflectorSession*));

        // If we get here, the DESCRIBE has completed, but if we are the owner that isn't enough.
        // We need to make sure that the SETUP and PLAY requests execute as well.
        theErr = ((RTSPSourceInfo*)theSession->GetSourceInfo())->SetupAndPlay();
        if (theErr != QTSS_NoErr)
            return HandleSourceInfoErr(theErr, inParams, theSession,
                                        ((RTSPSourceInfo*)theSession->GetSourceInfo())->GetRTSPClient());

        // We've completed the SETUP and PLAY process if we are here. The ReflectorSession
        // is completely setup.
        theSession->ManuallyMarkSetup();
        
        // NULL out the sSessionAttr, we don't need it anymore.
        NullOutSessionAttr(inParams->inClientSession);
    }
    
    //ok, we've found or setup the proper reflector session, create an RTPSessionOutput object,
    //and add it to the session's list of outputs
    RTPSessionOutput* theNewOutput = NEW RTPSessionOutput(inParams->inClientSession, theSession, sServerPrefs, sStreamCookieAttr );
    theSession->AddOutput(theNewOutput);
    
    // And vice-versa, store this reflector session in the RTP session.
    (void)QTSS_SetValue(inParams->inClientSession, sOutputAttr, 0, &theNewOutput, sizeof(theNewOutput));

    // Finally, send the DESCRIBE response
    
    //above function has signalled that this request belongs to us, so let's respond
    iovec theDescribeVec[2] = { 0 };
    
    Assert(theSession->GetLocalSDP()->Ptr != NULL);
    theDescribeVec[1].iov_base = theSession->GetLocalSDP()->Ptr;
    theDescribeVec[1].iov_len = theSession->GetLocalSDP()->Len;
    (void)QTSS_AppendRTSPHeader(inParams->inRTSPRequest, qtssCacheControlHeader,
                                kCacheControlHeader.Ptr, kCacheControlHeader.Len);
    QTSSModuleUtils::SendDescribeResponse(inParams->inRTSPRequest, inParams->inClientSession,
                                            &theDescribeVec[0], 2, theSession->GetLocalSDP()->Len); 
    return QTSS_NoErr;
}

ReflectorSession* FindOrCreateSession(StrPtrLen* inPath, QTSS_StandardRTSP_Params* inParams)
{
    // This function assumes that inPath is NULL terminated
    
    StrPtrLen theFileData;
    RTSPSourceInfo* theInfo = NULL;
    
    (void)QTSSModuleUtils::ReadEntireFile(inPath->Ptr, &theFileData);
    if (theFileData.Len > 0)
        theInfo = NEW RTSPSourceInfo(Socket::kNonBlockingSocketType);
    else
        return NULL;
    
    // We need to interpret this file as a standard prefs file, so let the
    // FilePrefsSource object parse it, then call ParsePrefs on the RTSPSourceInfo object,
    // which will parse out the RCF metadata.
    
    RelayPrefsSource thePrefsSource(true);// Allow duplicates
    (void)thePrefsSource.InitFromConfigFile(inPath->Ptr);
    QTSS_Error theErr = theInfo->ParsePrefs(&thePrefsSource, 0);
    if (theErr != QTSS_NoErr)
    {
        delete theInfo;
        return NULL;
    }
        
    // Ok, look for a reflector session matching the URL specified in the RCF file.
    // A unique broadcast is defined by the URL, the URL is the argument to resolve.
     
    OSMutexLocker locker(sSessionMap->GetMutex());
    OSRef* theSessionRef = sSessionMap->Resolve(theInfo->GetRTSPClient()->GetURL());
    ReflectorSession* theSession = NULL;
    
    if (theSessionRef == NULL)
    {
        //If this URL doesn't already have a reflector session, we must make a new
        //one. We already have the proper sourceInfo object, so we only need to construct the session
        
        theSession = NEW ReflectorSession(theInfo->GetRTSPClient()->GetURL(), theInfo);
                
        //put the session's ID into the session map.
        theErr = sSessionMap->Register(theSession->GetRef());
        Assert(theErr == QTSS_NoErr);

        //unless we do this, the refcount won't increment (and we'll delete the session prematurely
        OSRef* debug = sSessionMap->Resolve(theInfo->GetRTSPClient()->GetURL());
        Assert(debug == theSession->GetRef());
        
        // Create a socket stream for the TCP socket in the RTSPClient object. The socket stream will
        // allow this module to receive events on the socket
        QTSS_StreamRef theSockStream = NULL;
        theErr = QTSS_CreateStreamFromSocket(theInfo->GetRTSPClient()->GetSocket()->GetSocket()->GetSocketFD(), &theSockStream);
        Assert(theErr == QTSS_NoErr);
        Assert(theSockStream != NULL);
        
        // Store the socket stream in the Reflector Session so we can get at it easily later on
        theSession->SetSocketStream(theSockStream);
        
        // This RTSP session is the "owner" of this ReflectorSession, and will be responsible
        // for setting it up properly, so we should make sure this attribute gets set
        UInt32 theLen = sizeof(theSession);
        theErr = QTSS_SetValue(inParams->inClientSession, sSessionAttr, 0, (void*)&theSession, theLen);
        Assert(theErr == QTSS_NoErr);
    }
    else
    {
        // We aren't the owner of this ReflectorSession, and only the owner needs to keep this
        // RTSPSourceInfo object around.
        delete theInfo;
        
        theSession = (ReflectorSession*)theSessionRef->GetObject();

        if (!theSession->IsSetup())
        {
            // We are not the creator of this session, and it may not be setup yet. If it isn't,
            // we should simply wait for it to be setup.
            sSessionMap->Release(theSession->GetRef());

            // Give the owner some time to finish setting it up.
            (void)QTSS_SetIdleTimer(kSessionStartingIdleTimeInMsec);
            return NULL; // There isn't a completed session... yet.............
        }
    }
        
    Assert(theSession != NULL);
    return theSession;
}

QTSS_Error HandleSourceInfoErr(QTSS_Error rtspSourceInfoErr, QTSS_StandardRTSP_Params* inParams,
                                ReflectorSession* inSession, RTSPClient* inClient)
{       
    // If we get an EAGAIN here, the DESCRIBE hasn't completed yet
    if ((rtspSourceInfoErr == EAGAIN) || (rtspSourceInfoErr == EINPROGRESS))
    {
        // We're making an assumption here that inClient only uses one socket to connect to
        // the server. We only have one stream, so we have to make that assumption.

        // Note that it is not necessary to have any kind of timeout here, because the server
        // naturally times out idle connections. If the server doesn't respond for awhile,
        // this session will naturally go away
        inClient->GetSocket()->GetSocket()->DontAutoCleanup();
        RequestSocketEvent(inSession->GetSocketStream(), inClient->GetSocket()->GetEventMask());
        return QTSS_NoErr; // We'll get called in the same method again when there is more work to do
    }
    
    // We've encountered a fatal error for this session, so delete it.
    DeleteSessionOnError(inSession, inParams->inClientSession);
    
    if (rtspSourceInfoErr == QTSS_RequestFailed)
    {
        // This happens if the remote host responded with an error.
        char tempBuf[20];
        qtss_sprintf(tempBuf, "%"_U32BITARG_"", inClient->GetStatus());
        StrPtrLen tempBufPtr(&tempBuf[0]);
        return QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssServerGatewayTimeout,
                                                    sRemoteHostRespondedWithAnErrorErr, &tempBufPtr);
    }
    else
        return QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssServerGatewayTimeout,
                                                    sRemoteHostRefusedConnectionErr);
}

void    DeleteSessionOnError(ReflectorSession* inSession, QTSS_ClientSessionObject inCliSession)
{
    // Make sure to destroy the socket stream as well
    Assert(inSession->GetSocketStream() != NULL);
    (void)QTSS_DestroySocketStream(inSession->GetSocketStream());

    OSMutexLocker locker (sSessionMap->GetMutex());
    //decrement the ref count
    sSessionMap->Release(inSession->GetRef());
    
    // We are here if we are the owner of this session and we encountered an error
    // while trying to setup the session. We have the session map mutex, so the
    // refcount at this point *must* be 0.
    Assert(inSession->GetRef()->GetRefCount() == 0);
    sSessionMap->UnRegister(inSession->GetRef());
    delete inSession;
    
    // Make sure the session is NULLd out, because it's deleted now!
    NullOutSessionAttr(inCliSession);
}

void    NullOutSessionAttr(QTSS_ClientSessionObject inSession)
{
    ReflectorSession* theNull = NULL;
    UInt32 theLen = sizeof(theNull);
    QTSS_Error theErr = QTSS_SetValue(inSession, sSessionAttr, 0, (void*)&theNull, theLen);
    Assert(theErr == QTSS_NoErr);
}

QTSS_Error DoSetup(QTSS_StandardRTSP_Params* inParams, ReflectorSession* inSession)
{
    //unless there is a digit at the end of this path (representing trackID), don't
    //even bother with the request
    char* theDigitStr = NULL;
    (void)QTSS_GetValueAsString(inParams->inRTSPRequest, qtssRTSPReqFileDigit, 0, &theDigitStr);
    QTSSCharArrayDeleter theDigitStrDeleter(theDigitStr);
	if (theDigitStr == NULL)
        return  QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssClientBadRequest, sExpectedDigitFilenameErr);
    
    UInt32 theTrackID = ::strtol(theDigitStr, NULL, 10);
	
    QTSS_Error theErr = QTSS_NoErr;
    
    // Get info about this trackID
    SourceInfo::StreamInfo* theStreamInfo = inSession->GetSourceInfo()->GetStreamInfoByTrackID(theTrackID);
    // If theStreamInfo is NULL, we don't have a legit track, so return an error
    if (theStreamInfo == NULL)
        return QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssClientBadRequest,
                                                    sBadTrackIDErr);
    
    StrPtrLen* thePayloadName = &theStreamInfo->fPayloadName;
    QTSS_RTPPayloadType thePayloadType = theStreamInfo->fPayloadType;

    QTSS_RTPStreamObject newStream = NULL;
    {
        // Ok, this is completely crazy but I can't think of a better way to do this that's
        // safe so we'll do it this way for now. Because the ReflectorStreams use this session's
        // stream queue, we need to make sure that each ReflectorStream is not reflecting to this
        // session while we call QTSS_AddRTPStream. One brutal way to do this is to grab each
        // ReflectorStream's mutex, which will stop every reflector stream from running.
        
        for (UInt32 x = 0; x < inSession->GetNumStreams(); x++)
            inSession->GetStreamByIndex(x)->GetMutex()->Lock();
            
        theErr = QTSS_AddRTPStream(inParams->inClientSession, inParams->inRTSPRequest, &newStream, 0);

        for (UInt32 y = 0; y < inSession->GetNumStreams(); y++)
            inSession->GetStreamByIndex(y)->GetMutex()->Unlock();
            
        if (theErr != QTSS_NoErr)
            return theErr;
    }
    
    // Set up dictionary items for this stream
    theErr = QTSS_SetValue(newStream, qtssRTPStrPayloadName, 0, thePayloadName->Ptr, thePayloadName->Len);
    Assert(theErr == QTSS_NoErr);
    theErr = QTSS_SetValue(newStream, qtssRTPStrPayloadType, 0, &thePayloadType, sizeof(thePayloadType));
    Assert(theErr == QTSS_NoErr);
    theErr = QTSS_SetValue(newStream, qtssRTPStrTrackID, 0, &theTrackID, sizeof(theTrackID));
    Assert(theErr == QTSS_NoErr);

    // Place the stream cookie in this stream for future reference
    void* theStreamCookie = inSession->GetStreamCookie(theTrackID);
    Assert(theStreamCookie != NULL);
    theErr = QTSS_SetValue(newStream, sStreamCookieAttr, 0, &theStreamCookie, sizeof(theStreamCookie));
    Assert(theErr == QTSS_NoErr);

    // Set the number of quality levels.
    static UInt32 sNumQualityLevels = ReflectorSession::kNumQualityLevels;
    theErr = QTSS_SetValue(newStream, qtssRTPStrNumQualityLevels, 0, &sNumQualityLevels, sizeof(sNumQualityLevels));
    Assert(theErr == QTSS_NoErr);
    
    //send the setup response
    (void)QTSS_AppendRTSPHeader(inParams->inRTSPRequest, qtssCacheControlHeader,
                                kCacheControlHeader.Ptr, kCacheControlHeader.Len);
    (void)QTSS_SendStandardRTSPResponse(inParams->inRTSPRequest, newStream, 0);
    return QTSS_NoErr;
}


QTSS_Error DoPlay(QTSS_StandardRTSP_Params* inParams, ReflectorSession* inSession)
{
    // Tell the session what the bitrate of this reflection is. This is nice for logging,
    // it also allows the server to scale the TCP buffer size appropriately if we are
    // interleaving the data over TCP. This must be set before calling QTSS_Play so the
    // server can use it from within QTSS_Play
    UInt32 bitsPerSecond =  inSession->GetBitRate();
    (void)QTSS_SetValue(inParams->inClientSession, qtssCliSesMovieAverageBitRate, 0, &bitsPerSecond, sizeof(bitsPerSecond));

    //Server shouldn't send RTCP (reflector does it), but the server should append the server info app packet
    QTSS_Error theErr = QTSS_Play(inParams->inClientSession, inParams->inRTSPRequest, qtssPlayFlagsAppendServerInfo);
    if (theErr != QTSS_NoErr)
        return theErr;
    
    //and send a standard play response
    (void)QTSS_SendStandardRTSPResponse(inParams->inRTSPRequest, inParams->inClientSession, 0);
    return QTSS_NoErr;
}

QTSS_Error DestroySession(QTSS_ClientSessionClosing_Params* inParams)
{
    // Check and see if we are in the process of tearing down this connection already
    ReflectorSession* theSession = NULL;
    UInt32 theLen = sizeof(ReflectorSession*);
    (void)QTSS_GetValue(inParams->inClientSession, sSessionAttr, 0, (void*)&theSession, &theLen);
    
    if (theSession != NULL)
        IssueTeardown(theSession);
    else
    {
        RTPSessionOutput** theOutput = NULL;
        QTSS_Error theErr = QTSS_GetValuePtr(inParams->inClientSession, sOutputAttr, 0, (void**)&theOutput, &theLen);
        if ((theErr != QTSS_NoErr) || (theLen != sizeof(RTPSessionOutput*)) || (theOutput == NULL))
            return QTSS_RequestFailed;
    
        // This function removes the output from the ReflectorSession, then
        // checks to see if the session should go away. If it should, this deletes it
        theSession = (*theOutput)->GetReflectorSession();
        theSession->RemoveOutput(*theOutput);
        delete (*theOutput);

        //check if the ReflectorSession should be deleted
        //(it should if its ref count has dropped to 0)
        OSMutexLocker locker (sSessionMap->GetMutex());
        //decrement the ref count
        sSessionMap->Release(theSession->GetRef());
        if (theSession->GetRef()->GetRefCount() == 0)
        {
            sSessionMap->UnRegister(theSession->GetRef());
            
            theLen = sizeof(theSession);
            theErr = QTSS_SetValue(inParams->inClientSession, sSessionAttr, 0, (void*)&theSession, theLen);
            if (theErr != QTSS_NoErr)
                delete theSession;
            else
                IssueTeardown(theSession);
        }   
    }
    return QTSS_NoErr;
}

void IssueTeardown(ReflectorSession* inSession)
{
    // Tell the RTSPSourceInfo object to initiate or continue the Teardown process
    QTSS_Error theErr = ((RTSPSourceInfo*)inSession->GetSourceInfo())->Teardown();
    if ((theErr == EAGAIN) || (theErr == EINPROGRESS))
    {
        RTSPClient* theClient = ((RTSPSourceInfo*)inSession->GetSourceInfo())->GetRTSPClient();
        theClient->GetSocket()->GetSocket()->DontAutoCleanup();
        RequestSocketEvent(inSession->GetSocketStream(), theClient->GetSocket()->GetEventMask());
    }
    else
    {
        // Make sure to destroy the socket stream as well
        Assert(inSession->GetSocketStream() != NULL);
        (void)QTSS_DestroySocketStream(inSession->GetSocketStream());

        delete inSession;
    }
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
