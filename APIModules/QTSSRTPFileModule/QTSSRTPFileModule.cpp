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
    File:       QTSSFileModule.cpp

    Contains:   Implementation of module described in QTSSFileModule.h. 
                    

*/

#include <string.h>

#include "QTSSRTPFileModule.h"

#include "RTPFileSession.h"
#include "OSMemory.h"
#include "OSArrayObjectDeleter.h"
#include "QTSSModuleUtils.h"
#include "StringFormatter.h"
#include "SDPSourceInfo.h"
#include "QTSSMemoryDeleter.h"

#include "QTSS.h"


struct FileSession
{
    public:
    
        FileSession() :     fAdjustedPlayTime(0), fNextPacketLen(0), 
                            fStream(NULL), fSpeed(1), fStartTime(0), fStopTime(-1)
        {}
        
        RTPFileSession      fFile;
        SInt64              fAdjustedPlayTime;
        QTSS_PacketStruct   fPacketStruct;
        UInt32              fNextPacketLen;
        QTSS_RTPStreamObject    fStream;
        Float32             fSpeed;
        Float64             fStartTime;
        Float64             fStopTime;
};



// ref to the prefs dictionary object
static QTSS_ModulePrefsObject       sFileModulePrefs;

static StrPtrLen sRTPSuffix(".rtp");
static StrPtrLen sSDPHeader1("v=0\r\ns=");
static StrPtrLen sSDPHeader2;
static StrPtrLen sSDPHeader3("c=IN IP4 ");
static StrPtrLen sSDPHeader4("\r\na=control:/\r\n");
static const StrPtrLen              kCacheControlHeader("must-revalidate");

// ATTRIBUTES IDs

static QTSS_AttributeID sFileSessionAttr                = qtssIllegalAttrID;

static QTSS_AttributeID sSeekToNonexistentTimeErr       = qtssIllegalAttrID;
static QTSS_AttributeID sBadQTFileErr                   = qtssIllegalAttrID;
static QTSS_AttributeID sExpectedDigitFilenameErr       = qtssIllegalAttrID;
static QTSS_AttributeID sTrackDoesntExistErr            = qtssIllegalAttrID;

// OTHER DATA

static UInt32               sFlowControlProbeInterval   = 50;
static UInt32               sDefaultFlowControlProbeInterval= 50;
static Float32              sMaxAllowedSpeed            = 4;
static Float32              sDefaultMaxAllowedSpeed     = 4;

// FUNCTIONS

static QTSS_Error QTSSRTPFileModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParamBlock);
static QTSS_Error   Register(QTSS_Register_Params* inParams);
static QTSS_Error Initialize(QTSS_Initialize_Params* inParamBlock);
static QTSS_Error RereadPrefs();
static QTSS_Error ProcessRTSPRequest(QTSS_StandardRTSP_Params* inParamBlock);
static QTSS_Error DoDescribe(QTSS_StandardRTSP_Params* inParamBlock);
static QTSS_Error CreateRTPFileSession(QTSS_StandardRTSP_Params* inParamBlock, const StrPtrLen& inPath, FileSession** outFile);
static QTSS_Error DoSetup(QTSS_StandardRTSP_Params* inParamBlock);
static QTSS_Error DoPlay(QTSS_StandardRTSP_Params* inParamBlock);
static QTSS_Error SendPackets(QTSS_RTPSendPackets_Params* inParams);
static QTSS_Error DestroySession(QTSS_ClientSessionClosing_Params* inParams);



QTSS_Error QTSSRTPFileModule_Main(void* inPrivateArgs)
{
    return _stublibrary_main(inPrivateArgs, QTSSRTPFileModuleDispatch);
}

QTSS_Error  QTSSRTPFileModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParamBlock)
{
    switch (inRole)
    {
        case QTSS_Register_Role:
            return Register(&inParamBlock->regParams);
        case QTSS_Initialize_Role:
            return Initialize(&inParamBlock->initParams);
        case QTSS_RereadPrefs_Role:
            return RereadPrefs();
        case QTSS_RTSPPreProcessor_Role:
            return ProcessRTSPRequest(&inParamBlock->rtspPreProcessorParams);
        case QTSS_RTPSendPackets_Role:
            return SendPackets(&inParamBlock->rtpSendPacketsParams);
        case QTSS_ClientSessionClosing_Role:
            return DestroySession(&inParamBlock->clientSessionClosingParams);
    }
    return QTSS_NoErr;
}

QTSS_Error Register(QTSS_Register_Params* inParams)
{
    // Register for roles
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_RTSPPreProcessor_Role);
    (void)QTSS_AddRole(QTSS_ClientSessionClosing_Role);
    (void)QTSS_AddRole(QTSS_RereadPrefs_Role);

    // Add text messages attributes
    static char*        sSeekToNonexistentTimeName  = "QTSSFileModuleSeekToNonExistentTime";
    static char*        sBadQTFileName              = "QTSSFileModuleBadQTFile";
    static char*        sExpectedDigitFilenameName  = "QTSSFileModuleExpectedDigitFilename";
    static char*        sTrackDoesntExistName       = "QTSSFileModuleTrackDoesntExist";
    
    (void)QTSS_AddStaticAttribute(qtssTextMessagesObjectType, sSeekToNonexistentTimeName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssTextMessagesObjectType, sSeekToNonexistentTimeName, &sSeekToNonexistentTimeErr);

    (void)QTSS_AddStaticAttribute(qtssTextMessagesObjectType, sBadQTFileName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssTextMessagesObjectType, sBadQTFileName, &sBadQTFileErr);

    (void)QTSS_AddStaticAttribute(qtssTextMessagesObjectType, sExpectedDigitFilenameName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssTextMessagesObjectType, sExpectedDigitFilenameName, &sExpectedDigitFilenameErr);

    (void)QTSS_AddStaticAttribute(qtssTextMessagesObjectType, sTrackDoesntExistName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssTextMessagesObjectType, sTrackDoesntExistName, &sTrackDoesntExistErr);
    
    // Add an RTP session attribute for tracking FileSession objects
    static char*        sFileSessionName    = "QTSSRTPFileModuleSession";

    (void)QTSS_AddStaticAttribute(qtssClientSessionObjectType, sFileSessionName, NULL, qtssAttrDataTypeVoidPointer);
    (void)QTSS_IDForAttr(qtssClientSessionObjectType, sFileSessionName, &sFileSessionAttr);
    
    // Tell the server our name!
    static char* sModuleName = "QTSSRTPFileModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    return QTSS_NoErr;
}

QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);

    //
    // We need some prefs created and maintained by the QTSSFileModule,
    // cuz we don't want to duplicate basically the same stuff
    StrPtrLen theFileModule("QTSSFileModule");
    sFileModulePrefs = QTSSModuleUtils::GetModulePrefsObject(QTSSModuleUtils::GetModuleObjectByName(theFileModule));

    static StrPtrLen sEHeader("\r\ne=");
    static StrPtrLen sUHeader("\r\nu=");
    static StrPtrLen sHTTP("http://");
    static StrPtrLen sAdmin("admin@");
    
    // Read our preferences
    RereadPrefs();

    //build the sdp that looks like: \r\ne=http://streaming.apple.com\r\ne=qts@apple.com\r\n.

    // Get the default DNS name of the server
    StrPtrLen theDefaultDNS;
    (void)QTSS_GetValuePtr(inParams->inServer, qtssSvrDefaultDNSName, 0,
                                    (void**)&theDefaultDNS.Ptr, &theDefaultDNS.Len);
    
    StrPtrLen sdpURL;
    StrPtrLen adminEmail;
    sdpURL.Ptr = QTSSModuleUtils::GetStringAttribute(sFileModulePrefs, "sdp_url", "");
    sdpURL.Len = ::strlen(sdpURL.Ptr);
    
    adminEmail.Ptr = QTSSModuleUtils::GetStringAttribute(sFileModulePrefs, "admin_email", "");
    adminEmail.Len = ::strlen(adminEmail.Ptr);
    
    UInt32 sdpURLLen = sdpURL.Len;
    UInt32 adminEmailLen = adminEmail.Len;
    
    if (sdpURLLen == 0)
        sdpURLLen = theDefaultDNS.Len + sHTTP.Len + 1;
    if (adminEmailLen == 0)
        adminEmailLen = theDefaultDNS.Len + sAdmin.Len; 
    
    //calculate the length of the string & allocate memory
    sSDPHeader2.Len = (sEHeader.Len * 2) + sdpURLLen + adminEmailLen + 10;
    sSDPHeader2.Ptr = NEW char[sSDPHeader2.Len];

    //write it!
    StringFormatter sdpFormatter(sSDPHeader2);
    sdpFormatter.Put(sUHeader);

    //if there are preferences for default URL & admin email, use those. Otherwise, build the
    //proper string based on default dns name.
    if (sdpURL.Len == 0)
    {
        sdpFormatter.Put(sHTTP);
        sdpFormatter.Put(theDefaultDNS);
        sdpFormatter.PutChar('/');
    }
    else
        sdpFormatter.Put(sdpURL);
    
    sdpFormatter.Put(sEHeader);
    
    //now do the admin email.
    if (adminEmail.Len == 0)
    {
        sdpFormatter.Put(sAdmin);
        sdpFormatter.Put(theDefaultDNS);
    }
    else
        sdpFormatter.Put(adminEmail);
        
    sdpFormatter.PutEOL();
    sSDPHeader2.Len = (UInt32)sdpFormatter.GetCurrentOffset();
    
    delete [] sdpURL.Ptr;
    delete [] adminEmail.Ptr;

    // Report to the server that this module handles DESCRIBE, SETUP, PLAY, PAUSE, and TEARDOWN
    static QTSS_RTSPMethod sSupportedMethods[] = { qtssDescribeMethod, qtssSetupMethod, qtssTeardownMethod, qtssPlayMethod, qtssPauseMethod };
    QTSSModuleUtils::SetupSupportedMethods(inParams->inServer, sSupportedMethods, 5);
    
    return QTSS_NoErr;
}

QTSS_Error RereadPrefs()
{
    QTSSModuleUtils::GetAttribute(sFileModulePrefs, "flow_control_probe_interval",  qtssAttrDataTypeUInt32,
                                &sFlowControlProbeInterval, &sDefaultFlowControlProbeInterval, sizeof(sFlowControlProbeInterval));

    QTSSModuleUtils::GetAttribute(sFileModulePrefs, "max_allowed_speed",    qtssAttrDataTypeFloat32,
                                &sMaxAllowedSpeed, &sDefaultMaxAllowedSpeed, sizeof(sMaxAllowedSpeed));

    return QTSS_NoErr;
}


QTSS_Error ProcessRTSPRequest(QTSS_StandardRTSP_Params* inParams)
{
    QTSS_RTSPMethod* theMethod = NULL;
    UInt32 theMethodLen = 0;
    if ((QTSS_GetValuePtr(inParams->inRTSPRequest, qtssRTSPReqMethod, 0,
            (void**)&theMethod, &theMethodLen) != QTSS_NoErr) || (theMethodLen != sizeof(QTSS_RTSPMethod)))
    {
        Assert(0);
        return QTSS_RequestFailed;
    }
    
    switch (*theMethod)
    {
        case qtssDescribeMethod:
            return DoDescribe(inParams);
        case qtssSetupMethod:
            return DoSetup(inParams);
        case qtssPlayMethod:
            return DoPlay(inParams);
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

QTSS_Error DoDescribe(QTSS_StandardRTSP_Params* inParamBlock)
{
    // Check and see if this is a request we should handle. We handle all requests with URLs that
    // end in a '.rtp'
    char* theFullPathStr = NULL;
    QTSS_Error theError = QTSS_GetValueAsString(inParamBlock->inRTSPRequest, qtssRTSPReqLocalPath, 0, &theFullPathStr);
	QTSSCharArrayDeleter theFullPathDeleter(theFullPathStr);
    Assert(theError == QTSS_NoErr);
    
    StrPtrLen theFullPath(theFullPathStr);
    
    if ((theFullPath.Len <= sRTPSuffix.Len) ||
        (!sRTPSuffix.NumEqualIgnoreCase(&theFullPath.Ptr[theFullPath.Len - sRTPSuffix.Len], sRTPSuffix.Len)))
        return QTSS_RequestFailed;
    
    // It is, so let's set everything up...
    
    //
    // Get the FileSession for this DESCRIBE, if any.
    UInt32 theLen = sizeof(FileSession*);
    FileSession*    theFile = NULL;
    QTSS_Error      theErr = QTSS_NoErr;

    (void)QTSS_GetValue(inParamBlock->inClientSession, sFileSessionAttr, 0, (void*)&theFile, &theLen);

    if ( theFile != NULL )  
    {
        //
        // There is already a file for this session. This can happen if there are multiple DESCRIBES,
        // or a DESCRIBE has been issued with a Session ID, or some such thing.
        
        if ( !theFullPath.Equal( *theFile->fFile.GetMoviePath() ) )
        {
            delete theFile;
            theFile = NULL;
            
            // NULL out the attribute value, just in case.
            (void)QTSS_SetValue(inParamBlock->inClientSession, sFileSessionAttr, 0, &theFile, sizeof(theFile));
        }
    }
    
    if ( theFile == NULL )
    {   
        theErr = CreateRTPFileSession(inParamBlock, theFullPath, &theFile);
        if (theErr != QTSS_NoErr)
        {
            (void)QTSS_Teardown(inParamBlock->inClientSession);
            return theErr;
        }
    
        // Store this newly created file object in the RTP session.
        theErr = QTSS_SetValue(inParamBlock->inClientSession, sFileSessionAttr, 0, &theFile, sizeof(theFile));
    }
    
    //generate the SDP.
    UInt32 totalSDPLength = sSDPHeader1.Len;
    iovec theSDPVec[10];//1 for the RTSP header, 6 for the sdp header, 1 for the sdp body
    theSDPVec[1].iov_base = sSDPHeader1.Ptr;
    theSDPVec[1].iov_len = sSDPHeader1.Len;
    
    //filename goes here
    //(void)QTSS_GetValuePtr(inParamBlock->inRTSPRequest, qtssRTSPReqFilePath, 0, (void**)&theSDPVec[2].iov_base, (UInt32*)&theSDPVec[2].iov_len);
	char* filenameStr = NULL;
	(void)QTSS_GetValueAsString(inParamBlock->inRTSPRequest, qtssRTSPReqFilePath, 0, &filenameStr);
    QTSSCharArrayDeleter filenameStrDeleter(filenameStr);
	theSDPVec[2].iov_base = filenameStr;
	theSDPVec[2].iov_len = ::strlen(filenameStr);
	
    totalSDPLength += theSDPVec[2].iov_len;
    
    //url & admin email goes here
    theSDPVec[3].iov_base = sSDPHeader2.Ptr;
    theSDPVec[3].iov_len = sSDPHeader2.Len;
    totalSDPLength += sSDPHeader2.Len;

    //connection header
    theSDPVec[4].iov_base = sSDPHeader3.Ptr;
    theSDPVec[4].iov_len = sSDPHeader3.Len;
    totalSDPLength += sSDPHeader3.Len;
    
    //append IP addr
    (void)QTSS_GetValuePtr(inParamBlock->inRTSPSession, qtssRTSPSesLocalAddrStr, 0,
                                                (void**)&theSDPVec[5].iov_base, (UInt32*)&theSDPVec[5].iov_len);
    totalSDPLength += theSDPVec[5].iov_len;

    //last static sdp line
    theSDPVec[6].iov_base = sSDPHeader4.Ptr;
    theSDPVec[6].iov_len = sSDPHeader4.Len;
    totalSDPLength += sSDPHeader4.Len;
    
    //now append content-determined sdp
    theSDPVec[7].iov_base = theFile->fFile.GetSDPFile()->Ptr;
    theSDPVec[7].iov_len = theFile->fFile.GetSDPFile()->Len;
    totalSDPLength += theSDPVec[7].iov_len;
    
    Assert(theSDPVec[2].iov_base != NULL);


    // Append the Last Modified header to be a good caching proxy citizen before sending the Describe
    //(void)QTSS_AppendRTSPHeader(inParamBlock->inRTSPRequest, qtssLastModifiedHeader,
    //                              theFile->fFile.GetQTFile()->GetModDateStr(), DateBuffer::kDateBufferLen);
    (void)QTSS_AppendRTSPHeader(inParamBlock->inRTSPRequest, qtssCacheControlHeader,
                                    kCacheControlHeader.Ptr, kCacheControlHeader.Len);

    //ok, we have a filled out iovec. Let's send it!
    QTSSModuleUtils::SendDescribeResponse(inParamBlock->inRTSPRequest, inParamBlock->inClientSession,
                                                                    &theSDPVec[0], 8, totalSDPLength);  
	
    return QTSS_NoErr;
}

QTSS_Error CreateRTPFileSession(QTSS_StandardRTSP_Params* inParamBlock, const StrPtrLen& inPath, FileSession** outFile)
{   
    *outFile = NEW FileSession();
    StrPtrLen thePath(inPath);
    RTPFileSession::ErrorCode theErr = (*outFile)->fFile.Initialize(thePath, 8);
    if (theErr != RTPFileSession::errNoError)
    {
        delete *outFile;
        *outFile = NULL;
        
        if (theErr == RTPFileSession::errFileNotFound)
            return QTSSModuleUtils::SendErrorResponse(  inParamBlock->inRTSPRequest,
                                                        qtssClientNotFound,
                                                        sBadQTFileErr);
        AssertV(0, theErr);
    }
    return QTSS_NoErr;
}


QTSS_Error DoSetup(QTSS_StandardRTSP_Params* inParamBlock)
{
    //setup this track in the file object 
    FileSession* theFile = NULL;
    UInt32 theLen = sizeof(FileSession*);
    QTSS_Error theErr = QTSS_GetValue(inParamBlock->inClientSession, sFileSessionAttr, 0, (void*)&theFile, &theLen);
    if ((theErr != QTSS_NoErr) || (theLen != sizeof(FileSession*)))
    {
        // This is possible, as clients are not required to send a DESCRIBE. If we haven't set
        // anything up yet, set everything up
		char* theFullPathStr = NULL;
        theErr = QTSS_GetValueAsString(inParamBlock->inRTSPRequest, qtssRTSPReqLocalPath, 0, &theFullPathStr);
        Assert(theErr == QTSS_NoErr);
		QTSSCharArrayDeleter theFullPathDeleter(theFullPathStr);
		StrPtrLen theFullPath(theFullPathStr);
        	
        if ((theFullPath.Len <= sRTPSuffix.Len) ||
            (!sRTPSuffix.NumEqualIgnoreCase(&theFullPath.Ptr[theFullPath.Len - sRTPSuffix.Len], sRTPSuffix.Len)))
            return QTSS_RequestFailed;

        theErr = CreateRTPFileSession(inParamBlock, theFullPath, &theFile);
        if (theErr != QTSS_NoErr)
            return theErr;

        // Store this newly created file object in the RTP session.
        theErr = QTSS_SetValue(inParamBlock->inClientSession, sFileSessionAttr, 0, &theFile, sizeof(theFile));
    }

    //unless there is a digit at the end of this path (representing trackID), don't
    //even bother with the request
    char* theDigitStr = NULL;
    (void)QTSS_GetValueAsString(inParamBlock->inRTSPRequest, qtssRTSPReqFileDigit, 0, &theDigitStr);
    QTSSCharArrayDeleter theDigitStrDeleter(theDigitStr);
	if (theDigitStr == NULL)
        return QTSSModuleUtils::SendErrorResponse(inParamBlock->inRTSPRequest,
                                                    qtssClientBadRequest, sExpectedDigitFilenameErr);
    
    UInt32 theTrackID = ::strtol(theDigitStr, NULL, 10);
	
    RTPFileSession::ErrorCode qtfileErr = theFile->fFile.AddTrack(theTrackID);
    
    //if we get an error back, forward that error to the client
    if (qtfileErr == RTPFileSession::errTrackDoesntExist)
        return QTSSModuleUtils::SendErrorResponse(inParamBlock->inRTSPRequest,
                                                    qtssClientNotFound, sTrackDoesntExistErr);
    else if (qtfileErr != RTPFileSession::errNoError)
        return QTSSModuleUtils::SendErrorResponse(inParamBlock->inRTSPRequest,
                                                    qtssUnsupportedMediaType, sBadQTFileErr);

    //find the payload for this track ID (if applicable)
    StrPtrLen* thePayload = NULL;
    UInt32 thePayloadType = qtssUnknownPayloadType;
    Float32 bufferDelay = (Float32) 3.0; // FIXME need a constant defined for 3.0 value. It is used multiple places
    
    for (UInt32 x = 0; x < theFile->fFile.GetSourceInfo()->GetNumStreams(); x++)
    {
        SourceInfo::StreamInfo* theStreamInfo = theFile->fFile.GetSourceInfo()->GetStreamInfo(x);
        if (theStreamInfo->fTrackID == theTrackID)
        {
            thePayload = &theStreamInfo->fPayloadName;
            thePayloadType = theStreamInfo->fPayloadType;
            bufferDelay = theStreamInfo->fBufferDelay;
            break;
        }   
    }

    //Create a new RTP stream           
    QTSS_RTPStreamObject newStream = NULL;
    theErr = QTSS_AddRTPStream(inParamBlock->inClientSession, inParamBlock->inRTSPRequest, &newStream, 0);
    if (theErr != QTSS_NoErr)
        return theErr;
    
    // Set the payload type, payload name & timescale of this track
    SInt32 theTimescale = theFile->fFile.GetTrackTimeScale(theTrackID);
    
    theErr = QTSS_SetValue(newStream, qtssRTPStrBufferDelayInSecs, 0, &bufferDelay, sizeof(bufferDelay));
    Assert(theErr == QTSS_NoErr);
    theErr = QTSS_SetValue(newStream, qtssRTPStrPayloadName, 0, thePayload->Ptr, thePayload->Len);
    Assert(theErr == QTSS_NoErr);
    theErr = QTSS_SetValue(newStream, qtssRTPStrPayloadType, 0, &thePayloadType, sizeof(thePayloadType));
    Assert(theErr == QTSS_NoErr);
    theErr = QTSS_SetValue(newStream, qtssRTPStrTimescale, 0, &theTimescale, sizeof(theTimescale));
    Assert(theErr == QTSS_NoErr);
    theErr = QTSS_SetValue(newStream, qtssRTPStrTrackID, 0, &theTrackID, sizeof(theTrackID));
    Assert(theErr == QTSS_NoErr);
    
    // Set the number of quality levels. Allow up to 6
    static UInt32 sNumQualityLevels = 0;
    
    theErr = QTSS_SetValue(newStream, qtssRTPStrNumQualityLevels, 0, &sNumQualityLevels, sizeof(sNumQualityLevels));
    Assert(theErr == QTSS_NoErr);
    
    // Get the SSRC of this track
    UInt32* theTrackSSRC = NULL;
    UInt32 theTrackSSRCSize = 0;
    (void)QTSS_GetValuePtr(newStream, qtssRTPStrSSRC, 0, (void**)&theTrackSSRC, &theTrackSSRCSize);

    // The RTP stream should ALWAYS have an SSRC assuming QTSS_AddStream succeeded.
    Assert((theTrackSSRC != NULL) && (theTrackSSRCSize == sizeof(UInt32)));
    
    //give the file some info it needs.
    theFile->fFile.SetTrackSSRC(theTrackID, *theTrackSSRC);
    theFile->fFile.SetTrackCookie(theTrackID, newStream);
    
    //
    // Our array has now been updated to reflect the fields requested by the client.
    //send the setup response
    //(void)QTSS_AppendRTSPHeader(inParamBlock->inRTSPRequest, qtssLastModifiedHeader,
    //                          theFile->fFile.GetQTFile()->GetModDateStr(), DateBuffer::kDateBufferLen);
    (void)QTSS_AppendRTSPHeader(inParamBlock->inRTSPRequest, qtssCacheControlHeader,
                                kCacheControlHeader.Ptr, kCacheControlHeader.Len);

    //send the setup response
    (void)QTSS_SendStandardRTSPResponse(inParamBlock->inRTSPRequest, newStream, 0);
    return QTSS_NoErr;
}

QTSS_Error DoPlay(QTSS_StandardRTSP_Params* inParamBlock)
{
    FileSession** theFile = NULL;
    UInt32 theLen = 0;
    QTSS_Error theErr = QTSS_GetValuePtr(inParamBlock->inClientSession, sFileSessionAttr, 0, (void**)&theFile, &theLen);
    if ((theErr != QTSS_NoErr) || (theLen != sizeof(FileSession*)))
        return QTSS_RequestFailed;

    Float64* theStartTime = 0;
    theErr = QTSS_GetValuePtr(inParamBlock->inRTSPRequest, qtssRTSPReqStartTime, 0, (void**)&theStartTime, &theLen);
    if ((theErr != QTSS_NoErr) || (theLen != sizeof(Float64)))
        return QTSSModuleUtils::SendErrorResponse(  inParamBlock->inRTSPRequest,
                                                    qtssClientBadRequest, sSeekToNonexistentTimeErr);
                                                    
    RTPFileSession::ErrorCode qtFileErr = (*theFile)->fFile.Seek(*theStartTime);
    if (qtFileErr != RTPFileSession::errNoError)
        return QTSSModuleUtils::SendErrorResponse(  inParamBlock->inRTSPRequest,
                                                    qtssClientBadRequest, sSeekToNonexistentTimeErr);
                                                        
    //make sure to clear the next packet the server would have sent!
    (*theFile)->fPacketStruct.packetData = NULL;
    
    // Set the movie duration and size parameters
    Float64 movieDuration = (*theFile)->fFile.GetMovieDuration();
    (void)QTSS_SetValue(inParamBlock->inClientSession, qtssCliSesMovieDurationInSecs, 0, &movieDuration, sizeof(movieDuration));
    
    UInt64 movieSize = (*theFile)->fFile.GetAddedTracksRTPBytes();
    (void)QTSS_SetValue(inParamBlock->inClientSession, qtssCliSesMovieSizeInBytes, 0, &movieSize, sizeof(movieSize));
    
    //UInt32 bitsPerSecond =    (*theFile)->fFile.GetBytesPerSecond() * 8;
    //(void)QTSS_SetValue(inParamBlock->inClientSession, qtssCliSesMovieAverageBitRate, 0, &bitsPerSecond, sizeof(bitsPerSecond));

    //
    // For the purposes of the speed header, check to make sure all tracks are
    // over a reliable transport
    Bool16 allTracksReliable = true;
    
    // Set the timestamp & sequence number parameters for each track.
    QTSS_RTPStreamObject* theRef = NULL;
    for (   UInt32 theStreamIndex = 0;
            QTSS_GetValuePtr(inParamBlock->inClientSession, qtssCliSesStreamObjects, theStreamIndex, (void**)&theRef, &theLen) == QTSS_NoErr;
            theStreamIndex++)
    {
        UInt32* theTrackID = NULL;
        theErr = QTSS_GetValuePtr(*theRef, qtssRTPStrTrackID, 0, (void**)&theTrackID, &theLen);
        Assert(theErr == QTSS_NoErr);
        Assert(theTrackID != NULL);
        Assert(theLen == sizeof(UInt32));
        
        SInt16 theSeqNum = (*theFile)->fFile.GetNextTrackSequenceNumber(*theTrackID);
        SInt32 theTimestamp = (*theFile)->fFile.GetSeekTimestamp(*theTrackID);
        
        Assert(theRef != NULL);
        Assert(theLen == sizeof(QTSS_RTPStreamObject));
        
        theErr = QTSS_SetValue(*theRef, qtssRTPStrFirstSeqNumber, 0, &theSeqNum, sizeof(theSeqNum));
        Assert(theErr == QTSS_NoErr);
        theErr = QTSS_SetValue(*theRef, qtssRTPStrFirstTimestamp, 0, &theTimestamp, sizeof(theTimestamp));
        Assert(theErr == QTSS_NoErr);

        if (allTracksReliable)
        {
            QTSS_RTPTransportType theTransportType = qtssRTPTransportTypeUDP;
            theLen = sizeof(theTransportType);
            theErr = QTSS_GetValue(*theRef, qtssRTPStrTransportType, 0, &theTransportType, &theLen);
            Assert(theErr == QTSS_NoErr);
            
            if (theTransportType == qtssRTPTransportTypeUDP)
                allTracksReliable = false;
        }
    }
    
    //Tell the server to start playing this movie. We do want it to send RTCP SRs, but
    //we DON'T want it to write the RTP header
    theErr = QTSS_Play(inParamBlock->inClientSession, inParamBlock->inRTSPRequest, qtssPlayFlagsSendRTCP);
    if (theErr != QTSS_NoErr)
        return theErr;

    // qtssRTPSesAdjustedPlayTimeInMsec is valid only after calling QTSS_Play
    // theAdjustedPlayTime is a way to delay the sending of data until a time after the 
    // the Play response should have been received.
    SInt64* theAdjustedPlayTime = 0;
    theErr = QTSS_GetValuePtr(inParamBlock->inClientSession, qtssCliSesAdjustedPlayTimeInMsec, 0, (void**)&theAdjustedPlayTime, &theLen);
    Assert(theErr == QTSS_NoErr);
    Assert(theAdjustedPlayTime != NULL);
    Assert(theLen == sizeof(SInt64));
    (*theFile)->fAdjustedPlayTime = *theAdjustedPlayTime;
    
    //
    // This module supports the Speed header if the client wants the stream faster than normal.
    Float32 theSpeed = 1;
    theLen = sizeof(theSpeed);
    theErr = QTSS_GetValue(inParamBlock->inRTSPRequest, qtssRTSPReqSpeed, 0, &theSpeed, &theLen);
    Assert(theErr != QTSS_BadArgument);
    Assert(theErr != QTSS_NotEnoughSpace);
    
    if (theErr == QTSS_NoErr)
    {
        if (theSpeed > sMaxAllowedSpeed)
            theSpeed = sMaxAllowedSpeed;
        if ((theSpeed <= 0) || (!allTracksReliable))
            theSpeed = 1;
    }
    
    (*theFile)->fSpeed = theSpeed;
    
    if (theSpeed != 1)
    {
        //
        // If our speed is not 1, append the RTSP speed header in the response
        char speedBuf[32];
        qtss_sprintf(speedBuf, "%10.5f", theSpeed);
        StrPtrLen speedBufPtr(speedBuf);
        (void)QTSS_AppendRTSPHeader(inParamBlock->inRTSPRequest, qtssSpeedHeader,
                                    speedBufPtr.Ptr, speedBufPtr.Len);
    }
    
    //
    // Record the requested start and stop time.

    (*theFile)->fStopTime = -1;
    theLen = sizeof((*theFile)->fStopTime);
    theErr = QTSS_GetValue(inParamBlock->inRTSPRequest, qtssRTSPReqStopTime, 0, &(*theFile)->fStopTime, &theLen);
    Assert(theErr == QTSS_NoErr);
            
    (*theFile)->fStartTime = 0;
    theLen = sizeof((*theFile)->fStopTime);
    theErr = QTSS_GetValue(inParamBlock->inRTSPRequest, qtssRTSPReqStartTime, 0, &(*theFile)->fStartTime, &theLen);
    Assert(theErr == QTSS_NoErr);

    //the client doesn't necessarily specify this information in a play,
    //if it doesn't, fall back on some defaults.
    if ((*theFile)->fStartTime == -1)
        (*theFile)->fStartTime = 0;
        
    (void)QTSS_SendStandardRTSPResponse(inParamBlock->inRTSPRequest, inParamBlock->inClientSession, qtssPlayRespWriteTrackInfo);
    return QTSS_NoErr;
}

QTSS_Error SendPackets(QTSS_RTPSendPackets_Params* inParams)
{
    FileSession** theFile = NULL;
    UInt32 theLen = 0;
    QTSS_Error theErr = QTSS_GetValuePtr(inParams->inClientSession, sFileSessionAttr, 0, (void**)&theFile, &theLen);
    if ((theErr != QTSS_NoErr) || (theLen != sizeof(FileSession*)))
    {
        Assert( 0 );
        return QTSS_RequestFailed;
    }
    
    while (true)
    {   
        if ((*theFile)->fPacketStruct.packetData == NULL)
        {
            void* theCookie = NULL;
            Float64 theTransmitTime = (*theFile)->fFile.GetNextPacket((UInt8**)&(*theFile)->fPacketStruct.packetData, &(*theFile)->fNextPacketLen, &theCookie);
            
            //
            // Check to see if we should stop playing based on a client specified stop time
            if (((*theFile)->fStopTime != -1) && (theTransmitTime > (*theFile)->fStopTime))
            {
                // We should indeed stop playing
                (void)QTSS_Pause(inParams->inClientSession);
                inParams->outNextPacketTime = qtssDontCallSendPacketsAgain;
                return QTSS_NoErr;
            }

            //
            // Adjust transmit time based on speed
            Float64 theOffsetFromStartTime = theTransmitTime - (*theFile)->fStartTime;
            theTransmitTime = (*theFile)->fStartTime + (theOffsetFromStartTime / (*theFile)->fSpeed);
            
            (*theFile)->fStream = (QTSS_RTPStreamObject)theCookie;
            (*theFile)->fPacketStruct.packetTransmitTime = (*theFile)->fAdjustedPlayTime + ((SInt64)(theTransmitTime * 1000));
#if RTP_FILE_MODULE_DEBUGGING >= 8
            UInt16* theSeqNumPtr = (UInt16*)(*theFile)->fNextPacket;
            UInt32* theTimestampP = (UInt32*)(*theFile)->fNextPacket;
            UInt32* theTrackID = NULL;
            (void)QTSS_GetValuePtr((*theFile)->fStream, qtssRTPStrTrackID, 0, (void**)&theTrackID, &theLen);
            qtss_printf("Got packet. Seq num: %d. Timestamp: %d. TrackID: %d. Transmittime: %f\n",theSeqNumPtr[1], theTimestampP[1], *theTrackID, theTransmitTime);
#endif
        }
        
        //We are done playing all streams!
        if ((*theFile)->fPacketStruct.packetData == NULL)
        {
#if RTP_FILE_MODULE_DEBUGGING >= 8
            qtss_printf("done w all packets\n");
#endif
            inParams->outNextPacketTime = qtssDontCallSendPacketsAgain;
            return QTSS_NoErr;
        }
        //we have a packet that needs to be sent now
        Assert((*theFile)->fStream != NULL);

        // Send the packet!
        theErr =  QTSS_Write((*theFile)->fStream, &(*theFile)->fPacketStruct, (*theFile)->fNextPacketLen, NULL, qtssWriteFlagsIsRTP);

        if ( theErr == QTSS_WouldBlock )
        {   
            if ((*theFile)->fPacketStruct.packetTransmitTime == -1)
                inParams->outNextPacketTime = sFlowControlProbeInterval;    // for buffering, try me again in # MSec
            else
            {
                Assert((*theFile)->fPacketStruct.packetTransmitTime > inParams->inCurrentTime);
                inParams->outNextPacketTime = (*theFile)->fPacketStruct.packetTransmitTime - inParams->inCurrentTime;
            }
                
            return QTSS_NoErr;
        }

        (*theFile)->fPacketStruct.packetData = NULL;
    }
    Assert(0);
    return QTSS_NoErr;
}

QTSS_Error DestroySession(QTSS_ClientSessionClosing_Params* inParams)
{
    FileSession** theFile = NULL;
    UInt32 theLen = 0;
    QTSS_Error theErr = QTSS_GetValuePtr(inParams->inClientSession, sFileSessionAttr, 0, (void**)&theFile, &theLen);
    if ((theErr != QTSS_NoErr) || (theLen != sizeof(FileSession*)) || (theFile == NULL))
        return QTSS_RequestFailed;

    delete *theFile;
    return QTSS_NoErr;
}
