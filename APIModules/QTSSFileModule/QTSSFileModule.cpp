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

#include "QTSSFileModule.h"

#include "QTRTPFile.h"
#include "QTFile.h"
#include "OSMemory.h"
#include "OSArrayObjectDeleter.h"
#include "QTSSMemoryDeleter.h"
#include "SDPSourceInfo.h"
#include "StringFormatter.h"
#include "QTSSModuleUtils.h"
#include "QTSS3GPPModuleUtils.h"
#include "ResizeableStringFormatter.h"
#include "StringParser.h"
#include "SDPUtils.h"

#include <errno.h>

#include "QTSS.h"

class FileSession
{
    public:
    
        FileSession() : fAdjustedPlayTime(0), fNextPacketLen(0), fLastQualityCheck(0),
                        fAllowNegativeTTs(false), fSpeed(1),
                        fStartTime(-1), fStopTime(-1), fStopTrackID(0), fStopPN(0),
                        fLastRTPTime(0), fLastPauseTime(0),fTotalPauseTime(0), fPaused(true), fAdjustPauseTime(true)
        { 
          fPacketStruct.packetData = NULL; fPacketStruct.packetTransmitTime = -1; fPacketStruct.suggestedWakeupTime=-1;
        }
        
        ~FileSession() {}
        
        QTRTPFile           fFile;
        SInt64              fAdjustedPlayTime;
        QTSS_PacketStruct   fPacketStruct;
        int                 fNextPacketLen;
        SInt64              fLastQualityCheck;
        SDPSourceInfo       fSDPSource;
        Bool16              fAllowNegativeTTs;
        Float32             fSpeed;
        Float64             fStartTime;
        Float64             fStopTime;
        
        UInt32              fStopTrackID;
        UInt64              fStopPN;

        UInt32              fLastRTPTime;
        UInt64              fLastPauseTime;
        SInt64              fTotalPauseTime;
        Bool16              fPaused;
        Bool16              fAdjustPauseTime;
};

// ref to the prefs dictionary object
static QTSS_ModulePrefsObject       sPrefs;
static QTSS_PrefsObject             sServerPrefs;
static QTSS_Object                  sServer;

static StrPtrLen sSDPSuffix(".sdp");
static  StrPtrLen sVersionHeader("v=0");
static  StrPtrLen sSessionNameHeader("s=");
static  StrPtrLen sPermanentTimeHeader("t=0 0");
static  StrPtrLen sConnectionHeader("c=IN IP4 0.0.0.0");
static StrPtrLen  sStaticControlHeader("a=control:*");
static  StrPtrLen  sEmailHeader;
static  StrPtrLen  sURLHeader;
static  StrPtrLen  sEOL("\r\n");
static  StrPtrLen sSDPNotValidMessage("Movie SDP is not valid.");

const   SInt16    sNumSDPVectors = 22;

// ATTRIBUTES IDs

static QTSS_AttributeID sFileSessionAttr                = qtssIllegalAttrID;

static QTSS_AttributeID sSeekToNonexistentTimeErr       = qtssIllegalAttrID;
static QTSS_AttributeID sNoSDPFileFoundErr              = qtssIllegalAttrID;
static QTSS_AttributeID sBadQTFileErr                   = qtssIllegalAttrID;
static QTSS_AttributeID sFileIsNotHintedErr             = qtssIllegalAttrID;
static QTSS_AttributeID sExpectedDigitFilenameErr       = qtssIllegalAttrID;
static QTSS_AttributeID sTrackDoesntExistErr            = qtssIllegalAttrID;

static QTSS_AttributeID sFileSessionPlayCountAttrID     = qtssIllegalAttrID;
static QTSS_AttributeID sFileSessionBufferDelayAttrID   = qtssIllegalAttrID;

static QTSS_AttributeID sRTPStreamLastSentPacketSeqNumAttrID   = qtssIllegalAttrID;

static QTSS_AttributeID sRTPStreamLastPacketSeqNumAttrID   = qtssIllegalAttrID;

// OTHER DATA

static UInt32				sFlowControlProbeInterval	= 10;
static UInt32				sDefaultFlowControlProbeInterval= 10;
static Float32              sMaxAllowedSpeed            = 4;
static Float32              sDefaultMaxAllowedSpeed     = 4;

// File Caching Prefs
static Bool16               sEnableSharedBuffers    = false;
static Bool16               sEnablePrivateBuffers   = false;

static UInt32               sSharedBufferUnitKSize  = 0;
static UInt32               sSharedBufferInc        = 0;
static UInt32               sSharedBufferUnitSize   = 0;
static UInt32               sSharedBufferMaxUnits   = 0;

static UInt32               sPrivateBufferUnitKSize = 0;
static UInt32               sPrivateBufferUnitSize  = 0;
static UInt32               sPrivateBufferMaxUnits  = 0;

static Float32              sAddClientBufferDelaySecs = 0;

static Bool16               sRecordMovieFileSDP = false;
static Bool16               sEnableMovieFileSDP = false;

static Bool16               sPlayerCompatibility = true;
static UInt32               sAdjustMediaBandwidthPercent = 50;
static SInt64               sAdjustRTPStartTimeMilli = 500;

static Bool16               sAllowInvalidHintRefs = false;

// Server preference we respect
static Bool16               sDisableThinning       = false;
static UInt16               sDefaultStreamingQuality = 0;

static const StrPtrLen              kCacheControlHeader("must-revalidate");
static const QTSS_RTSPStatusCode    kNotModifiedStatus          = qtssRedirectNotModified;


const Bool16				kAddPauseTimeToRTPTime = true;
const Bool16				kDontAddPauseTimeToRTPTime = false;

  
// FUNCTIONS

static QTSS_Error QTSSFileModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParamBlock);
static QTSS_Error Register(QTSS_Register_Params* inParams);
static QTSS_Error Initialize(QTSS_Initialize_Params* inParamBlock);
static QTSS_Error RereadPrefs();
static QTSS_Error ProcessRTSPRequest(QTSS_StandardRTSP_Params* inParamBlock);
static QTSS_Error DoDescribe(QTSS_StandardRTSP_Params* inParamBlock);
static QTSS_Error CreateQTRTPFile(QTSS_StandardRTSP_Params* inParamBlock, char* inPath, FileSession** outFile);
static QTSS_Error DoSetup(QTSS_StandardRTSP_Params* inParamBlock);
static QTSS_Error DoPlay(QTSS_StandardRTSP_Params* inParamBlock);
static QTSS_Error SendPackets(QTSS_RTPSendPackets_Params* inParams);
static QTSS_Error DestroySession(QTSS_ClientSessionClosing_Params* inParams);
static void       DeleteFileSession(FileSession* inFileSession);
static UInt32   WriteSDPHeader(FILE* sdpFile, iovec *theSDPVec, SInt16 *ioVectorIndex, StrPtrLen *sdpHeader);
static void     BuildPrefBasedHeaders();

QTSS_Error QTSSFileModule_Main(void* inPrivateArgs)
{
    return _stublibrary_main(inPrivateArgs, QTSSFileModuleDispatch);
}

inline UInt16 GetPacketSequenceNumber(void * packetDataPtr)
{
    return ntohs( ((UInt16*)packetDataPtr)[1]);
}

inline UInt16 GetLastPacketSeqNum(QTSS_Object stream)
{

    UInt16 lastSeqNum = 0;
    UInt32  theLen = sizeof(lastSeqNum);
    (void) QTSS_GetValue(stream, sRTPStreamLastPacketSeqNumAttrID, 0, (void*)&lastSeqNum, &theLen);

    return lastSeqNum;
}


inline SInt32 GetLastSentSeqNumber(QTSS_Object stream)
{
    UInt16 lastSeqNum = 0;
    UInt32  theLen = sizeof(lastSeqNum);
    QTSS_Error error = QTSS_GetValue(stream, sRTPStreamLastSentPacketSeqNumAttrID, 0, (void*)&lastSeqNum, &theLen);
    if (error == QTSS_ValueNotFound) // first packet
    {    return -1;
    }

    return (SInt32)lastSeqNum; // return UInt16 seq num value or -1.
} 

inline void SetPacketSequenceNumber(UInt16 newSequenceNumber, void * packetDataPtr)
{
    ((UInt16*)packetDataPtr)[1] = htons(newSequenceNumber);
}


inline UInt32 GetPacketTimeStamp(void * packetDataPtr)
{
    return ntohl( ((UInt32*)packetDataPtr)[1]);
}

inline void SetPacketTimeStamp(UInt32 newTimeStamp, void * packetDataPtr)
{
    ((UInt32*)packetDataPtr)[1] = htonl(newTimeStamp);
}

inline UInt32 CalculatePauseTimeStamp(UInt32 timescale, SInt64 totalPauseTime, UInt32 currentTimeStamp)
{
    SInt64 pauseTime = (SInt64) ( (Float64) timescale * ( ( (Float64) totalPauseTime) / 1000.0));     
    UInt32 pauseTimeStamp = (UInt32) (pauseTime + currentTimeStamp);

    return pauseTimeStamp;
}

UInt32 SetPausetimeTimeStamp(FileSession *fileSessionPtr, QTSS_Object theRTPStream, UInt32 currentTimeStamp)
{ 
    if (false == fileSessionPtr->fAdjustPauseTime || fileSessionPtr->fTotalPauseTime == 0)
        return currentTimeStamp;

    UInt32 timeScale = 0;
    UInt32 theLen = sizeof(timeScale);
    (void) QTSS_GetValue(theRTPStream, qtssRTPStrTimescale, 0, (void*)&timeScale, &theLen);    
    if (theLen != sizeof(timeScale) || timeScale == 0)
        return currentTimeStamp;

    UInt32 pauseTimeStamp = CalculatePauseTimeStamp( timeScale,  fileSessionPtr->fTotalPauseTime, currentTimeStamp);
    if (pauseTimeStamp != currentTimeStamp)
        SetPacketTimeStamp(pauseTimeStamp, fileSessionPtr->fPacketStruct.packetData);

    return pauseTimeStamp;
}


UInt32 WriteSDPHeader(FILE* sdpFile, iovec *theSDPVec, SInt16 *ioVectorIndex, StrPtrLen *sdpHeader)
{

    Assert (ioVectorIndex != NULL);
    Assert (theSDPVec != NULL);
    Assert (sdpHeader != NULL);
    Assert (*ioVectorIndex < sNumSDPVectors); // if adding an sdp param you need to increase sNumSDPVectors
    
    SInt16 theIndex = *ioVectorIndex;
    *ioVectorIndex += 1;

    theSDPVec[theIndex].iov_base =  sdpHeader->Ptr;
    theSDPVec[theIndex].iov_len = sdpHeader->Len;
    
    if (sdpFile !=NULL)
        ::fwrite(theSDPVec[theIndex].iov_base,theSDPVec[theIndex].iov_len,sizeof(char),sdpFile);
    
    return theSDPVec[theIndex].iov_len;
}



QTSS_Error  QTSSFileModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParamBlock)
{
    switch (inRole)
    {
        case QTSS_Register_Role:
            return Register(&inParamBlock->regParams);
        case QTSS_Initialize_Role:
            return Initialize(&inParamBlock->initParams);
        case QTSS_RereadPrefs_Role:
            return RereadPrefs();
        case QTSS_RTSPRequest_Role:
            return ProcessRTSPRequest(&inParamBlock->rtspRequestParams);
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
    (void)QTSS_AddRole(QTSS_RTSPRequest_Role);
    (void)QTSS_AddRole(QTSS_ClientSessionClosing_Role);
    (void)QTSS_AddRole(QTSS_RereadPrefs_Role);

    // Add text messages attributes
    static char*        sSeekToNonexistentTimeName  = "QTSSFileModuleSeekToNonExistentTime";
    static char*        sNoSDPFileFoundName         = "QTSSFileModuleNoSDPFileFound";
    static char*        sBadQTFileName              = "QTSSFileModuleBadQTFile";
    static char*        sFileIsNotHintedName        = "QTSSFileModuleFileIsNotHinted";
    static char*        sExpectedDigitFilenameName  = "QTSSFileModuleExpectedDigitFilename";
    static char*        sTrackDoesntExistName       = "QTSSFileModuleTrackDoesntExist";
    
    (void)QTSS_AddStaticAttribute(qtssTextMessagesObjectType, sSeekToNonexistentTimeName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssTextMessagesObjectType, sSeekToNonexistentTimeName, &sSeekToNonexistentTimeErr);

    (void)QTSS_AddStaticAttribute(qtssTextMessagesObjectType, sNoSDPFileFoundName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssTextMessagesObjectType, sNoSDPFileFoundName, &sNoSDPFileFoundErr);

    (void)QTSS_AddStaticAttribute(qtssTextMessagesObjectType, sBadQTFileName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssTextMessagesObjectType, sBadQTFileName, &sBadQTFileErr);

    (void)QTSS_AddStaticAttribute(qtssTextMessagesObjectType, sFileIsNotHintedName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssTextMessagesObjectType, sFileIsNotHintedName, &sFileIsNotHintedErr);

    (void)QTSS_AddStaticAttribute(qtssTextMessagesObjectType, sExpectedDigitFilenameName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssTextMessagesObjectType, sExpectedDigitFilenameName, &sExpectedDigitFilenameErr);

    (void)QTSS_AddStaticAttribute(qtssTextMessagesObjectType, sTrackDoesntExistName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssTextMessagesObjectType, sTrackDoesntExistName, &sTrackDoesntExistErr);
    
    // Add an RTP session attribute for tracking FileSession objects
    static char*        sFileSessionName    = "QTSSFileModuleSession";
    (void)QTSS_AddStaticAttribute(qtssClientSessionObjectType, sFileSessionName, NULL, qtssAttrDataTypeVoidPointer);
    (void)QTSS_IDForAttr(qtssClientSessionObjectType, sFileSessionName, &sFileSessionAttr);
    
    static char*        sFileSessionPlayCountName   = "QTSSFileModulePlayCount";
    (void)QTSS_AddStaticAttribute(qtssClientSessionObjectType, sFileSessionPlayCountName, NULL, qtssAttrDataTypeUInt32);
    (void)QTSS_IDForAttr(qtssClientSessionObjectType, sFileSessionPlayCountName, &sFileSessionPlayCountAttrID);
    
    static char*        sFileSessionBufferDelayName = "QTSSFileModuleSDPBufferDelay";
    (void)QTSS_AddStaticAttribute(qtssClientSessionObjectType, sFileSessionBufferDelayName, NULL, qtssAttrDataTypeFloat32);
    (void)QTSS_IDForAttr(qtssClientSessionObjectType, sFileSessionBufferDelayName, &sFileSessionBufferDelayAttrID);
    
     static char*        sRTPStreamLastSentPacketSeqNumName   = "QTSSFileModuleLastSentPacketSeqNum";
    (void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sRTPStreamLastSentPacketSeqNumName, NULL, qtssAttrDataTypeUInt16);
    (void)QTSS_IDForAttr(qtssRTPStreamObjectType, sRTPStreamLastSentPacketSeqNumName, &sRTPStreamLastSentPacketSeqNumAttrID);
   

    static char*        sRTPStreamLastPacketSeqNumName   = "QTSSFileModuleLastPacketSeqNum";
    (void)QTSS_AddStaticAttribute(qtssRTPStreamObjectType, sRTPStreamLastPacketSeqNumName, NULL, qtssAttrDataTypeUInt16);
    (void)QTSS_IDForAttr(qtssRTPStreamObjectType, sRTPStreamLastPacketSeqNumName, &sRTPStreamLastPacketSeqNumAttrID);

    // Tell the server our name!
    static char* sModuleName = "QTSSFileModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    return QTSS_NoErr;
}

QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{
    QTRTPFile::Initialize();
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);
	QTSS3GPPModuleUtils::Initialize(inParams);

    sPrefs = QTSSModuleUtils::GetModulePrefsObject(inParams->inModule);
    sServerPrefs = inParams->inPrefs;
    sServer = inParams->inServer;
        
    // Read our preferences
    RereadPrefs();
    
    // Report to the server that this module handles DESCRIBE, SETUP, PLAY, PAUSE, and TEARDOWN
    static QTSS_RTSPMethod sSupportedMethods[] = { qtssDescribeMethod, qtssSetupMethod, qtssTeardownMethod, qtssPlayMethod, qtssPauseMethod };
    QTSSModuleUtils::SetupSupportedMethods(inParams->inServer, sSupportedMethods, 5);

    return QTSS_NoErr;
}

void BuildPrefBasedHeaders()
{
    //build the sdp that looks like: \r\ne=http://streaming.apple.com\r\ne=qts@apple.com.
    static StrPtrLen sUHeader("u=");
    static StrPtrLen sEHeader("e=");
    static StrPtrLen sHTTP("http://");
    static StrPtrLen sAdmin("admin@");

    // Get the default DNS name of the server
    StrPtrLen theDefaultDNS;
    (void)QTSS_GetValuePtr(sServer, qtssSvrDefaultDNSName, 0, (void**)&theDefaultDNS.Ptr, &theDefaultDNS.Len);
    
    //-------- URL Header
    StrPtrLen sdpURL;
    sdpURL.Ptr = QTSSModuleUtils::GetStringAttribute(sPrefs, "sdp_url", "");
    sdpURL.Len = ::strlen(sdpURL.Ptr);
    
    UInt32 sdpURLLen = sdpURL.Len;
    if (sdpURLLen == 0)
        sdpURLLen = theDefaultDNS.Len + sHTTP.Len + 1;
        
    sURLHeader.Delete();
    sURLHeader.Len = sdpURLLen + 10;
    sURLHeader.Ptr = NEW char[sURLHeader.Len];
    StringFormatter urlFormatter(sURLHeader);
    urlFormatter.Put(sUHeader);
    if (sdpURL.Len == 0)
    {
        urlFormatter.Put(sHTTP);
        urlFormatter.Put(theDefaultDNS);
        urlFormatter.PutChar('/');
    }
    else
        urlFormatter.Put(sdpURL);
    
    sURLHeader.Len = (UInt32)urlFormatter.GetCurrentOffset();


    //-------- Email Header
    StrPtrLen adminEmail;
    adminEmail.Ptr = QTSSModuleUtils::GetStringAttribute(sPrefs, "admin_email", "");
    adminEmail.Len = ::strlen(adminEmail.Ptr);
    
    UInt32 adminEmailLen = adminEmail.Len;
    if (adminEmailLen == 0)
        adminEmailLen = theDefaultDNS.Len + sAdmin.Len; 
        
    sEmailHeader.Delete();
    sEmailHeader.Len = (sEHeader.Len * 2) + adminEmailLen + 10;
    sEmailHeader.Ptr = NEW char[sEmailHeader.Len];
    StringFormatter sdpFormatter(sEmailHeader);
    sdpFormatter.Put(sEHeader);
    
    if (adminEmail.Len == 0)
    {
        sdpFormatter.Put(sAdmin);
        sdpFormatter.Put(theDefaultDNS);
    }
    else
        sdpFormatter.Put(adminEmail);
        
    sEmailHeader.Len = (UInt32)sdpFormatter.GetCurrentOffset();
    
    
    sdpURL.Delete();
    adminEmail.Delete();
}

QTSS_Error RereadPrefs()
{
    
    QTSSModuleUtils::GetAttribute(sPrefs, "flow_control_probe_interval",    qtssAttrDataTypeUInt32,
                                &sFlowControlProbeInterval, &sDefaultFlowControlProbeInterval, sizeof(sFlowControlProbeInterval));

    QTSSModuleUtils::GetAttribute(sPrefs, "max_allowed_speed",  qtssAttrDataTypeFloat32,
                                &sMaxAllowedSpeed, &sDefaultMaxAllowedSpeed, sizeof(sMaxAllowedSpeed));
                                
// File Cache prefs     
                    
    sEnableSharedBuffers = true;
    QTSSModuleUtils::GetIOAttribute(sPrefs, "enable_shared_file_buffers", qtssAttrDataTypeBool16, &sEnableSharedBuffers,  sizeof(sEnableSharedBuffers));

    sEnablePrivateBuffers = false;
    QTSSModuleUtils::GetIOAttribute(sPrefs, "enable_private_file_buffers", qtssAttrDataTypeBool16, &sEnablePrivateBuffers, sizeof(sEnablePrivateBuffers));

    sSharedBufferInc = 8;
    QTSSModuleUtils::GetIOAttribute(sPrefs, "num_shared_buffer_increase_per_session", qtssAttrDataTypeUInt32,&sSharedBufferInc, sizeof(sSharedBufferInc));
                            
    sSharedBufferUnitKSize = 256;
    QTSSModuleUtils::GetIOAttribute(sPrefs, "shared_buffer_unit_k_size", qtssAttrDataTypeUInt32, &sSharedBufferUnitKSize, sizeof(sSharedBufferUnitKSize));

    sPrivateBufferUnitKSize = 256;
    QTSSModuleUtils::GetIOAttribute(sPrefs, "private_buffer_unit_k_size", qtssAttrDataTypeUInt32, &sPrivateBufferUnitKSize, sizeof(sPrivateBufferUnitKSize));

    sSharedBufferUnitSize = 1;
    QTSSModuleUtils::GetIOAttribute(sPrefs, "num_shared_buffer_units_per_buffer", qtssAttrDataTypeUInt32,&sSharedBufferUnitSize, sizeof(sSharedBufferUnitSize));

    sPrivateBufferUnitSize = 1;
    QTSSModuleUtils::GetIOAttribute(sPrefs, "num_private_buffer_units_per_buffer", qtssAttrDataTypeUInt32,&sPrivateBufferUnitSize, sizeof(sPrivateBufferUnitSize));
                                
    sSharedBufferMaxUnits = 8;
    QTSSModuleUtils::GetIOAttribute(sPrefs, "max_shared_buffer_units_per_buffer", qtssAttrDataTypeUInt32, &sSharedBufferMaxUnits, sizeof(sSharedBufferMaxUnits));
                                
    sPrivateBufferMaxUnits = 8;
    QTSSModuleUtils::GetIOAttribute(sPrefs, "max_private_buffer_units_per_buffer", qtssAttrDataTypeUInt32, &sPrivateBufferMaxUnits, sizeof(sPrivateBufferMaxUnits));

    sAddClientBufferDelaySecs = 0;
    QTSSModuleUtils::GetIOAttribute(sPrefs, "add_seconds_to_client_buffer_delay", qtssAttrDataTypeFloat32, &sAddClientBufferDelaySecs, sizeof(sAddClientBufferDelaySecs));

    sRecordMovieFileSDP = false;
    QTSSModuleUtils::GetIOAttribute(sPrefs, "record_movie_file_sdp", qtssAttrDataTypeBool16, &sRecordMovieFileSDP, sizeof(sRecordMovieFileSDP));

    sEnableMovieFileSDP = false;
    QTSSModuleUtils::GetIOAttribute(sPrefs, "enable_movie_file_sdp", qtssAttrDataTypeBool16, &sEnableMovieFileSDP, sizeof(sEnableMovieFileSDP));
    
    sPlayerCompatibility = true;
    QTSSModuleUtils::GetIOAttribute(sPrefs, "enable_player_compatibility", qtssAttrDataTypeBool16, &sPlayerCompatibility, sizeof(sPlayerCompatibility));

    sAdjustMediaBandwidthPercent = 50;
    QTSSModuleUtils::GetIOAttribute(sPrefs, "compatibility_adjust_sdp_media_bandwidth_percent", qtssAttrDataTypeUInt32, &sAdjustMediaBandwidthPercent, sizeof(sAdjustMediaBandwidthPercent));

    sAdjustRTPStartTimeMilli = 500;
    QTSSModuleUtils::GetIOAttribute(sPrefs, "compatibility_adjust_rtp_start_time_milli", qtssAttrDataTypeSInt64, &sAdjustRTPStartTimeMilli, sizeof(sAdjustRTPStartTimeMilli));

    sAllowInvalidHintRefs = false;
    QTSSModuleUtils::GetIOAttribute(sPrefs, "allow_invalid_hint_track_refs", qtssAttrDataTypeBool16, &sAllowInvalidHintRefs,  sizeof(sAllowInvalidHintRefs));

    if (sAdjustMediaBandwidthPercent > 100)
        sAdjustMediaBandwidthPercent = 100;
        
    if (sAdjustMediaBandwidthPercent < 1)
        sAdjustMediaBandwidthPercent = 1;
        
    UInt32 len = sizeof(sDisableThinning);
    (void) QTSS_GetValue(sServerPrefs, qtssPrefsDisableThinning, 0, (void*)&sDisableThinning, &len);

    len = sizeof(sDefaultStreamingQuality);
    (void) QTSS_GetValue(sServerPrefs, qtssPrefsDefaultStreamQuality, 0, (void*)&sDefaultStreamingQuality, &len);

    QTSS3GPPModuleUtils::ReadPrefs();
    
    BuildPrefBasedHeaders();
    
    return QTSS_NoErr;
}

QTSS_Error ProcessRTSPRequest(QTSS_StandardRTSP_Params* inParamBlock)
{
    QTSS_RTSPMethod* theMethod = NULL;
    UInt32 theMethodLen = 0;
    if ((QTSS_GetValuePtr(inParamBlock->inRTSPRequest, qtssRTSPReqMethod, 0,
            (void**)&theMethod, &theMethodLen) != QTSS_NoErr) || (theMethodLen != sizeof(QTSS_RTSPMethod)))
    {
        Assert(0);
        return QTSS_RequestFailed;
    }
    
    QTSS_Error err = QTSS_NoErr;
    switch (*theMethod)
    {
        case qtssDescribeMethod:
            err = DoDescribe(inParamBlock);
            break;
        case qtssSetupMethod:
            err = DoSetup(inParamBlock);
            break;
        case qtssPlayMethod:
            err = DoPlay(inParamBlock);
            break;
        case qtssTeardownMethod:
            (void)QTSS_Teardown(inParamBlock->inClientSession);
            (void)QTSS_SendStandardRTSPResponse(inParamBlock->inRTSPRequest, inParamBlock->inClientSession, 0);
            break;
        case qtssPauseMethod:
        {    (void)QTSS_Pause(inParamBlock->inClientSession);
            (void)QTSS_SendStandardRTSPResponse(inParamBlock->inRTSPRequest, inParamBlock->inClientSession, 0);
                
            FileSession** theFile = NULL;
            UInt32 theLen = 0;
            QTSS_Error theErr = QTSS_GetValuePtr(inParamBlock->inClientSession, sFileSessionAttr, 0, (void**)&theFile, &theLen);
            if ((theErr != QTSS_NoErr) || (theLen != sizeof(FileSession*)))
                return QTSS_RequestFailed;

            (**theFile).fPaused = true;
            (**theFile).fLastPauseTime = OS::Milliseconds();

            break;
        }
        default:
            break;
    }
    if (err != QTSS_NoErr)
        (void)QTSS_Teardown(inParamBlock->inClientSession);

    return QTSS_NoErr;
}

Bool16 isSDP(QTSS_StandardRTSP_Params* inParamBlock)
{
	Bool16 sdpSuffix = false;
	
    char* path = NULL;
    UInt32 len = 0;
	QTSS_LockObject(inParamBlock->inRTSPRequest);
    QTSS_Error theErr = QTSS_GetValuePtr(inParamBlock->inRTSPRequest, qtssRTSPReqLocalPath, 0, (void**)&path, &len);
    Assert(theErr == QTSS_NoErr);
	
    if (sSDPSuffix.Len <= len)
	{
		StrPtrLen thePath(&path[len - sSDPSuffix.Len],sSDPSuffix.Len);
		sdpSuffix = thePath.Equal(sSDPSuffix);
	}
	
	QTSS_UnlockObject(inParamBlock->inRTSPRequest);
	
	return sdpSuffix;
}

QTSS_Error DoDescribe(QTSS_StandardRTSP_Params* inParamBlock)
{
    if (isSDP(inParamBlock))
    {
        StrPtrLen pathStr;
        (void)QTSS_LockObject(inParamBlock->inRTSPRequest);
        (void)QTSS_GetValuePtr(inParamBlock->inRTSPRequest, qtssRTSPReqFilePath, 0, (void**)&pathStr.Ptr, &pathStr.Len);
        QTSS_Error err = QTSSModuleUtils::SendErrorResponse(inParamBlock->inRTSPRequest, qtssClientNotFound, sNoSDPFileFoundErr, &pathStr);
        (void)QTSS_UnlockObject(inParamBlock->inRTSPRequest);
        return err;
    }
    
    //
    // Get the FileSession for this DESCRIBE, if any.
    UInt32 theLen = sizeof(FileSession*);
    FileSession*    theFile = NULL;
    QTSS_Error      theErr = QTSS_NoErr;
    Bool16          pathEndsWithSDP = false;
    static StrPtrLen sSDPSuffix(".sdp");
    SInt16 vectorIndex = 1;
    ResizeableStringFormatter theFullSDPBuffer(NULL,0);
    StrPtrLen bufferDelayStr;
    char tempBufferDelay[64];
    StrPtrLen theSDPData;
        
    (void)QTSS_GetValue(inParamBlock->inClientSession, sFileSessionAttr, 0, (void*)&theFile, &theLen);
    // Generate the complete file path
    UInt32 thePathLen = 0;
    OSCharArrayDeleter thePath(QTSSModuleUtils::GetFullPath(inParamBlock->inRTSPRequest, qtssRTSPReqFilePath,&thePathLen, &sSDPSuffix));
        
    //first locate the target movie
    thePath.GetObject()[thePathLen - sSDPSuffix.Len] = '\0';//truncate the .sdp added in the GetFullPath call
    StrPtrLen   requestPath(thePath.GetObject(), ::strlen(thePath.GetObject()));
    if (requestPath.Len > sSDPSuffix.Len )
    {   StrPtrLen endOfPath(&requestPath.Ptr[requestPath.Len -  sSDPSuffix.Len], sSDPSuffix.Len);
        if (endOfPath.EqualIgnoreCase(sSDPSuffix)) // it is a .sdp
        {   pathEndsWithSDP = true;
        }
    }
    
    if ( theFile != NULL )  
    {
        //
        // There is already a file for this session. This can happen if there are multiple DESCRIBES,
        // or a DESCRIBE has been issued with a Session ID, or some such thing.
        StrPtrLen   moviePath( theFile->fFile.GetMoviePath() );
                
		// Stop playing because the new file isn't ready yet to send packets.
		// Needs a Play request to get things going. SendPackets on the file is active if not paused.
		(void)QTSS_Pause(inParamBlock->inClientSession);
		(*theFile).fPaused = true;

        //
        // This describe is for a different file. Delete the old FileSession.
        if ( !requestPath.Equal( moviePath ) )
        {
            DeleteFileSession(theFile);
            theFile = NULL;
            
            // NULL out the attribute value, just in case.
            (void)QTSS_SetValue(inParamBlock->inClientSession, sFileSessionAttr, 0, &theFile, sizeof(theFile));
        }
    }

    if ( theFile == NULL )
    {   
        theErr = CreateQTRTPFile(inParamBlock, thePath.GetObject(), &theFile);
        if (theErr != QTSS_NoErr)
            return theErr;
    
        // Store this newly created file object in the RTP session.
        theErr = QTSS_SetValue(inParamBlock->inClientSession, sFileSessionAttr, 0, &theFile, sizeof(theFile));
    }
    
    //replace the sacred character we have trodden on in order to truncate the path.
    thePath.GetObject()[thePathLen - sSDPSuffix.Len] = sSDPSuffix.Ptr[0];

    iovec theSDPVec[sNumSDPVectors];//1 for the RTSP header, 6 for the sdp header, 1 for the sdp body
    ::memset(&theSDPVec[0], 0, sizeof(theSDPVec));
    
    if (sEnableMovieFileSDP)
    {
        // Check to see if there is an sdp file, if so, return that file instead
        // of the built-in sdp. ReadEntireFile allocates memory but if all goes well theSDPData will be managed by the File Session
        (void)QTSSModuleUtils::ReadEntireFile(thePath.GetObject(), &theSDPData);
    }
    OSCharArrayDeleter sdpDataDeleter(theSDPData.Ptr); // Just in case we fail we know to clean up. But we clear the deleter if we succeed.

    if (theSDPData.Len > 0)
    {   
        SDPContainer fileSDPContainer; 
        fileSDPContainer.SetSDPBuffer(&theSDPData);  
        if (!fileSDPContainer.IsSDPBufferValid())
        {    return QTSSModuleUtils::SendErrorResponseWithMessage(inParamBlock->inRTSPRequest, qtssUnsupportedMediaType, &sSDPNotValidMessage);
        }
    
        
        // Append the Last Modified header to be a good caching proxy citizen before sending the Describe
        (void)QTSS_AppendRTSPHeader(inParamBlock->inRTSPRequest, qtssLastModifiedHeader,
                                        theFile->fFile.GetQTFile()->GetModDateStr(), DateBuffer::kDateBufferLen);
        (void)QTSS_AppendRTSPHeader(inParamBlock->inRTSPRequest, qtssCacheControlHeader,
                                        kCacheControlHeader.Ptr, kCacheControlHeader.Len);

        //Now that we have the file data, send an appropriate describe
        //response to the client
        theSDPVec[1].iov_base = theSDPData.Ptr;
        theSDPVec[1].iov_len = theSDPData.Len;

        QTSSModuleUtils::SendDescribeResponse(inParamBlock->inRTSPRequest, inParamBlock->inClientSession,
                                                                &theSDPVec[0], 3, theSDPData.Len);  
    }
    else
    {
        // Before generating the SDP and sending it, check to see if there is an If-Modified-Since
        // date. If there is, and the content hasn't been modified, then just return a 304 Not Modified
        QTSS_TimeVal* theTime = NULL;
        (void) QTSS_GetValuePtr(inParamBlock->inRTSPRequest, qtssRTSPReqIfModSinceDate, 0, (void**)&theTime, &theLen);
        if ((theLen == sizeof(QTSS_TimeVal)) && (*theTime > 0))
        {
            // There is an If-Modified-Since header. Check it vs. the content.
            if (*theTime == theFile->fFile.GetQTFile()->GetModDate())
            {
                theErr = QTSS_SetValue( inParamBlock->inRTSPRequest, qtssRTSPReqStatusCode, 0,
                                        &kNotModifiedStatus, sizeof(kNotModifiedStatus) );
                Assert(theErr == QTSS_NoErr);
                // Because we are using this call to generate a 304 Not Modified response, we do not need
                // to pass in a RTP Stream
                theErr = QTSS_SendStandardRTSPResponse(inParamBlock->inRTSPRequest, inParamBlock->inClientSession, 0);
                Assert(theErr == QTSS_NoErr);
                return QTSS_NoErr;
            }
        }
        
        FILE* sdpFile = NULL;
        if (sRecordMovieFileSDP &&  !pathEndsWithSDP) // don't auto create sdp for an sdp file because it would look like a broadcast
        {   
            sdpFile = ::fopen(thePath.GetObject(),"r"); // see if there already is a .sdp for the movie
            if (sdpFile != NULL) // one already exists don't mess with it
            {   ::fclose(sdpFile);
                sdpFile = NULL;
            }
            else
                sdpFile = ::fopen(thePath.GetObject(),"w"); // create the .sdp
        }
        
        UInt32 totalSDPLength = 0;
        
        //Get filename 
        //StrPtrLen fileNameStr;
        //(void)QTSS_GetValuePtr(inParamBlock->inRTSPRequest, qtssRTSPReqFilePath, 0, (void**)&fileNameStr.Ptr, (UInt32*)&fileNameStr.Len);
        char* fileNameStr = NULL;
        (void)QTSS_GetValueAsString(inParamBlock->inRTSPRequest, qtssRTSPReqFilePath, 0, &fileNameStr);
        QTSSCharArrayDeleter fileNameStrDeleter(fileNameStr);
        	
        //Get IP addr
        StrPtrLen ipStr;
        (void)QTSS_GetValuePtr(inParamBlock->inRTSPSession, qtssRTSPSesLocalAddrStr, 0, (void**)&ipStr.Ptr, &ipStr.Len);


//      
// *** The order of sdp headers is specified and required by rfc 2327
//
// -------- version header 

        theFullSDPBuffer.Put(sVersionHeader);
        theFullSDPBuffer.Put(sEOL);
        
// -------- owner header

        const SInt16 sLineSize = 256;
        char ownerLine[sLineSize]="";
        ownerLine[sLineSize - 1] = 0;
        
        char *ipCstr = ipStr.GetAsCString();
        OSCharArrayDeleter ipDeleter(ipCstr);
        
        // the first number is the NTP time used for the session identifier (this changes for each request)
        // the second number is the NTP date time of when the file was modified (this changes when the file changes)
        qtss_sprintf(ownerLine, "o=StreamingServer %"_64BITARG_"d %"_64BITARG_"d IN IP4 %s", (SInt64) OS::UnixTime_Secs() + 2208988800LU, (SInt64) theFile->fFile.GetQTFile()->GetModDate(),ipCstr);
        Assert(ownerLine[sLineSize - 1] == 0);

        StrPtrLen ownerStr(ownerLine);
        theFullSDPBuffer.Put(ownerStr); 
        theFullSDPBuffer.Put(sEOL); 
        
// -------- session header

        theFullSDPBuffer.Put(sSessionNameHeader);
        theFullSDPBuffer.Put(fileNameStr);
        theFullSDPBuffer.Put(sEOL);
    
// -------- uri header

        theFullSDPBuffer.Put(sURLHeader);
        theFullSDPBuffer.Put(sEOL);

    
// -------- email header

        theFullSDPBuffer.Put(sEmailHeader);
        theFullSDPBuffer.Put(sEOL);

// -------- connection information header
        
        theFullSDPBuffer.Put(sConnectionHeader); 
        theFullSDPBuffer.Put(sEOL);

// -------- time header

        // t=0 0 is a permanent always available movie (doesn't ever change unless we change the code)
        theFullSDPBuffer.Put(sPermanentTimeHeader);
        theFullSDPBuffer.Put(sEOL);
        
// -------- control header

        theFullSDPBuffer.Put(sStaticControlHeader);
        theFullSDPBuffer.Put(sEOL);
        
                
// -------- add buffer delay

        if (sAddClientBufferDelaySecs > 0) // increase the client buffer delay by the preference amount.
        {
            Float32 bufferDelay = 3.0; // the client doesn't advertise it's default value so we guess.
            
            static StrPtrLen sBuffDelayStr("a=x-bufferdelay:");
        
            StrPtrLen delayStr;
            theSDPData.FindString(sBuffDelayStr, &delayStr);
            if (delayStr.Len > 0)
            {
                UInt32 offset = (delayStr.Ptr - theSDPData.Ptr) + delayStr.Len; // step past the string
                delayStr.Ptr = theSDPData.Ptr + offset;
                delayStr.Len = theSDPData.Len - offset;
                StringParser theBufferSecsParser(&delayStr);
                theBufferSecsParser.ConsumeWhitespace();
                bufferDelay = theBufferSecsParser.ConsumeFloat();
            }
            
            bufferDelay += sAddClientBufferDelaySecs;

           
            qtss_sprintf(tempBufferDelay, "a=x-bufferdelay:%.2f",bufferDelay);
            bufferDelayStr.Set(tempBufferDelay); 
                
            theFullSDPBuffer.Put(bufferDelayStr); 
            theFullSDPBuffer.Put(sEOL);
        }
        
 // -------- movie file sdp data

        //now append content-determined sdp ( cached in QTRTPFile )
        int sdpLen = 0;
        theSDPData.Ptr = theFile->fFile.GetSDPFile(&sdpLen);
        theSDPData.Len = sdpLen;

// ----------- Add the movie's sdp headers to our sdp headers
 
		theFullSDPBuffer.Put(theSDPData); 
        StrPtrLen fullSDPBuffSPL(theFullSDPBuffer.GetBufPtr(),theFullSDPBuffer.GetBytesWritten());

// ------------ Check the headers
        SDPContainer rawSDPContainer;
        rawSDPContainer.SetSDPBuffer( &fullSDPBuffSPL );  
        if (!rawSDPContainer.IsSDPBufferValid())
        {    return QTSSModuleUtils::SendErrorResponseWithMessage(inParamBlock->inRTSPRequest, qtssUnsupportedMediaType, &sSDPNotValidMessage);
        }

// ------------ reorder the sdp headers to make them proper.
        Float32 adjustMediaBandwidthPercent = 1.0;
        Bool16 adjustMediaBandwidth = false;
        if (sPlayerCompatibility )
            adjustMediaBandwidth = QTSSModuleUtils::HavePlayerProfile(sServerPrefs, inParamBlock,QTSSModuleUtils::kAdjustBandwidth);
		    		    
		if (adjustMediaBandwidth)
		    adjustMediaBandwidthPercent = (Float32) sAdjustMediaBandwidthPercent / 100.0;
        
        ResizeableStringFormatter buffer;
        SDPContainer* insertMediaLines = QTSS3GPPModuleUtils::Get3GPPSDPFeatureListCopy(buffer);
		SDPLineSorter sortedSDP(&rawSDPContainer,adjustMediaBandwidthPercent,insertMediaLines);
		delete insertMediaLines;
		StrPtrLen *theSessionHeadersPtr = sortedSDP.GetSessionHeaders();
		StrPtrLen *theMediaHeadersPtr = sortedSDP.GetMediaHeaders();
	
//3GPP-BAD // add the bitrate adaptation string to the SDPLineSorter
	//sortedSDP should have a getmedialine[n]
	// findstring in line
	// getline and insert line to media
	/*
	5.3.3.5 The bit-rate adaptation support attribute, Ò3GPP-Adaptation-SupportÓ 
To signal the support of bit-rate adaptation, a media level only SDP attribute is defined in ABNF [53]: 
sdp-Adaptation-line  = "a" "=" "3GPP-Adaptation-Support" ":" report-frequency CRLF 
report-frequency  = NonZeroDIGIT [ DIGIT ] 
NonZeroDIGIT = %x31-39 ;1-9 
A server implementing rate adaptation shall signal the "3GPP-Adaptation-Support" attribute in its SDP. 

*/

	
// ----------- write out the sdp

		totalSDPLength += ::WriteSDPHeader(sdpFile, theSDPVec, &vectorIndex, theSessionHeadersPtr);
        totalSDPLength += ::WriteSDPHeader(sdpFile, theSDPVec, &vectorIndex, theMediaHeadersPtr);
 

// -------- done with SDP processing
         
        if (sdpFile !=NULL)
            ::fclose(sdpFile);
            

        Assert(theSDPData.Len > 0);
        Assert(theSDPVec[2].iov_base != NULL);
        //ok, we have a filled out iovec. Let's send the response!
        
        // Append the Last Modified header to be a good caching proxy citizen before sending the Describe
        (void)QTSS_AppendRTSPHeader(inParamBlock->inRTSPRequest, qtssLastModifiedHeader,
                                        theFile->fFile.GetQTFile()->GetModDateStr(), DateBuffer::kDateBufferLen);
        (void)QTSS_AppendRTSPHeader(inParamBlock->inRTSPRequest, qtssCacheControlHeader,
                                        kCacheControlHeader.Ptr, kCacheControlHeader.Len);
        QTSSModuleUtils::SendDescribeResponse(inParamBlock->inRTSPRequest, inParamBlock->inClientSession,
                                                                        &theSDPVec[0], vectorIndex, totalSDPLength);    
    }
    
    Assert(theSDPData.Ptr != NULL);
    Assert(theSDPData.Len > 0);
    
    //now parse the movie media sdp data. We need to do this in order to extract payload information.
    //The SDP parser object will not take responsibility of the memory (one exception... see above)
    theFile->fSDPSource.Parse(theSDPData.Ptr, theSDPData.Len);
    sdpDataDeleter.ClearObject(); // don't delete theSDPData, theFile has it now.
    
    return QTSS_NoErr;
}

QTSS_Error CreateQTRTPFile(QTSS_StandardRTSP_Params* inParamBlock, char* inPath, FileSession** outFile)
{   
    *outFile = NEW FileSession();
    (*outFile)->fFile.SetAllowInvalidHintRefs(sAllowInvalidHintRefs);
    QTRTPFile::ErrorCode theErr = (*outFile)->fFile.Initialize(inPath);
    if (theErr != QTRTPFile::errNoError)
    {
        delete *outFile;
        *outFile = NULL;

        char* thePathStr = NULL;
        (void)QTSS_GetValueAsString(inParamBlock->inRTSPRequest, qtssRTSPReqFilePath, 0, &thePathStr);
        QTSSCharArrayDeleter thePathStrDeleter(thePathStr);
        StrPtrLen thePath(thePathStr);        
		
        if (theErr == QTRTPFile::errFileNotFound)
            return QTSSModuleUtils::SendErrorResponse(  inParamBlock->inRTSPRequest,
                                                        qtssClientNotFound,
                                                        sNoSDPFileFoundErr,&thePath);
        if (theErr == QTRTPFile::errInvalidQuickTimeFile)
            return QTSSModuleUtils::SendErrorResponse(  inParamBlock->inRTSPRequest,
                                                        qtssUnsupportedMediaType,
                                                        sBadQTFileErr,&thePath);
        if (theErr == QTRTPFile::errNoHintTracks)
            return QTSSModuleUtils::SendErrorResponse(  inParamBlock->inRTSPRequest,
                                                        qtssUnsupportedMediaType,
                                                        sFileIsNotHintedErr,&thePath);
        if (theErr == QTRTPFile::errInternalError)
            return QTSSModuleUtils::SendErrorResponse(  inParamBlock->inRTSPRequest,
                                                        qtssServerInternal,
                                                        sBadQTFileErr,&thePath);

        AssertV(0, theErr);
    }
	
    return QTSS_NoErr;
}


QTSS_Error DoSetup(QTSS_StandardRTSP_Params* inParamBlock)
{

    if (isSDP(inParamBlock))
    {
        StrPtrLen pathStr;
        (void)QTSS_LockObject(inParamBlock->inRTSPRequest);
        (void)QTSS_GetValuePtr(inParamBlock->inRTSPRequest, qtssRTSPReqFilePath, 0, (void**)&pathStr.Ptr, &pathStr.Len);
        QTSS_Error err = QTSSModuleUtils::SendErrorResponse(inParamBlock->inRTSPRequest, qtssClientNotFound, sNoSDPFileFoundErr, &pathStr);
        (void)QTSS_UnlockObject(inParamBlock->inRTSPRequest);
        return err;
    }
    
    //setup this track in the file object 
    FileSession* theFile = NULL;
    UInt32 theLen = sizeof(FileSession*);
    QTSS_Error theErr = QTSS_GetValue(inParamBlock->inClientSession, sFileSessionAttr, 0, (void*)&theFile, &theLen);
    if ((theErr != QTSS_NoErr) || (theLen != sizeof(FileSession*)))
    {
        char* theFullPath = NULL;
        //theErr = QTSS_GetValuePtr(inParamBlock->inRTSPRequest, qtssRTSPReqLocalPath, 0, (void**)&theFullPath, &theLen);
		theErr = QTSS_GetValueAsString(inParamBlock->inRTSPRequest, qtssRTSPReqLocalPath, 0, &theFullPath);
        Assert(theErr == QTSS_NoErr);
        // This is possible, as clients are not required to send a DESCRIBE. If we haven't set
        // anything up yet, set everything up
        theErr = CreateQTRTPFile(inParamBlock, theFullPath, &theFile);
		QTSS_Delete(theFullPath);
        if (theErr != QTSS_NoErr)
            return theErr;

        int theSDPBodyLen = 0;
        char* theSDPData = theFile->fFile.GetSDPFile(&theSDPBodyLen);

        //now parse the sdp. We need to do this in order to extract payload information.
        //The SDP parser object will not take responsibility of the memory (one exception... see above)
        theFile->fSDPSource.Parse(theSDPData, theSDPBodyLen);

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
    
//    QTRTPFile::ErrorCode qtfileErr = theFile->fFile.AddTrack(theTrackID, false); //test for 3gpp monotonic wall clocktime and sequence
    QTRTPFile::ErrorCode qtfileErr = theFile->fFile.AddTrack(theTrackID, true);
    
    //if we get an error back, forward that error to the client
    if (qtfileErr == QTRTPFile::errTrackIDNotFound)
        return QTSSModuleUtils::SendErrorResponse(inParamBlock->inRTSPRequest,
                                                    qtssClientNotFound, sTrackDoesntExistErr);
    else if (qtfileErr != QTRTPFile::errNoError)
        return QTSSModuleUtils::SendErrorResponse(inParamBlock->inRTSPRequest,
                                                    qtssUnsupportedMediaType, sBadQTFileErr);

    // Before setting up this track, check to see if there is an If-Modified-Since
    // date. If there is, and the content hasn't been modified, then just return a 304 Not Modified
    QTSS_TimeVal* theTime = NULL;
    (void) QTSS_GetValuePtr(inParamBlock->inRTSPRequest, qtssRTSPReqIfModSinceDate, 0, (void**)&theTime, &theLen);
    if ((theLen == sizeof(QTSS_TimeVal)) && (*theTime > 0))
    {
        // There is an If-Modified-Since header. Check it vs. the content.
        if (*theTime == theFile->fFile.GetQTFile()->GetModDate())
        {
            theErr = QTSS_SetValue( inParamBlock->inRTSPRequest, qtssRTSPReqStatusCode, 0,
                                            &kNotModifiedStatus, sizeof(kNotModifiedStatus) );
            Assert(theErr == QTSS_NoErr);
            // Because we are using this call to generate a 304 Not Modified response, we do not need
            // to pass in a RTP Stream
            theErr = QTSS_SendStandardRTSPResponse(inParamBlock->inRTSPRequest, inParamBlock->inClientSession, 0);
            Assert(theErr == QTSS_NoErr);
            return QTSS_NoErr;
        }
    }

    //find the payload for this track ID (if applicable)
    StrPtrLen* thePayload = NULL;
    UInt32 thePayloadType = qtssUnknownPayloadType;
    Float32 bufferDelay = (Float32) 3.0; // FIXME need a constant defined for 3.0 value. It is used multiple places

    for (UInt32 x = 0; x < theFile->fSDPSource.GetNumStreams(); x++)
    {
        SourceInfo::StreamInfo* theStreamInfo = theFile->fSDPSource.GetStreamInfo(x);
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
    static UInt32 sNumQualityLevels = 6;  
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
    theFile->fFile.SetTrackCookies(theTrackID, newStream, thePayloadType);
    
    StrPtrLen theHeader;
    theErr = QTSS_GetValuePtr(inParamBlock->inRTSPHeaders, qtssXRTPMetaInfoHeader, 0, (void**)&theHeader.Ptr, &theHeader.Len);
    if (theErr == QTSS_NoErr)
    {
        //
        // If there is an x-RTP-Meta-Info header in the request, mirror that header in the
        // response. We will support any fields supported by the QTFileLib.
        RTPMetaInfoPacket::FieldID* theFields = NEW RTPMetaInfoPacket::FieldID[RTPMetaInfoPacket::kNumFields];
        ::memcpy(theFields, QTRTPFile::GetSupportedRTPMetaInfoFields(), sizeof(RTPMetaInfoPacket::FieldID) * RTPMetaInfoPacket::kNumFields);

        //
        // This function does the work of appending the response header based on the
        // fields we support and the requested fields.
        theErr = QTSSModuleUtils::AppendRTPMetaInfoHeader(inParamBlock->inRTSPRequest, &theHeader, theFields);

        //
        // This returns QTSS_NoErr only if there are some valid, useful fields
        Bool16 isVideo = false;
        if (thePayloadType == qtssVideoPayloadType)
            isVideo = true;
        if (theErr == QTSS_NoErr)
            theFile->fFile.SetTrackRTPMetaInfo(theTrackID, theFields, isVideo);
    }
    
    //
    // Our array has now been updated to reflect the fields requested by the client.
    //send the setup response
    (void)QTSS_AppendRTSPHeader(inParamBlock->inRTSPRequest, qtssLastModifiedHeader,
                                theFile->fFile.GetQTFile()->GetModDateStr(), DateBuffer::kDateBufferLen);
    (void)QTSS_AppendRTSPHeader(inParamBlock->inRTSPRequest, qtssCacheControlHeader,
                                kCacheControlHeader.Ptr, kCacheControlHeader.Len);
    theErr = QTSS_SendStandardRTSPResponse(inParamBlock->inRTSPRequest, newStream, 0);
    Assert(theErr == QTSS_NoErr);
    return QTSS_NoErr;
}



QTSS_Error SetupCacheBuffers(QTSS_StandardRTSP_Params* inParamBlock, FileSession** theFile)
{
    
    UInt32 playCount = 0;
    UInt32 theLen = sizeof(playCount);
    QTSS_Error theErr = QTSS_GetValue(inParamBlock->inClientSession, sFileSessionPlayCountAttrID, 0, (void*)&playCount, &theLen);
    if ( (theErr != QTSS_NoErr) || (theLen != sizeof(playCount)) )
    {
        playCount = 1;
        theErr = QTSS_SetValue(inParamBlock->inClientSession, sFileSessionPlayCountAttrID, 0, &playCount, sizeof(playCount));
        if (theErr != QTSS_NoErr)
            return QTSS_RequestFailed;
    }
    
    if (sEnableSharedBuffers && playCount == 1) // increments num buffers after initialization so do only once per session
        (*theFile)->fFile.AllocateSharedBuffers(sSharedBufferUnitKSize, sSharedBufferInc, sSharedBufferUnitSize,sSharedBufferMaxUnits);
    
    if (sEnablePrivateBuffers) // reinitializes buffers to current location so do every time 
        (*theFile)->fFile.AllocatePrivateBuffers(sSharedBufferUnitKSize, sPrivateBufferUnitSize, sPrivateBufferMaxUnits);

    playCount ++;
    theErr = QTSS_SetValue(inParamBlock->inClientSession, sFileSessionPlayCountAttrID, 0, &playCount, sizeof(playCount));
    if (theErr != QTSS_NoErr)
        return QTSS_RequestFailed;  
        
    return theErr;

}

QTSS_Error DoPlay(QTSS_StandardRTSP_Params* inParamBlock)
{
    QTRTPFile::ErrorCode qtFileErr = QTRTPFile::errNoError;

    if (isSDP(inParamBlock))
    {
        StrPtrLen pathStr;
        (void)QTSS_LockObject(inParamBlock->inRTSPRequest);
        (void)QTSS_GetValuePtr(inParamBlock->inRTSPRequest, qtssRTSPReqFilePath, 0, (void**)&pathStr.Ptr, &pathStr.Len);
        QTSS_Error err = QTSSModuleUtils::SendErrorResponse(inParamBlock->inRTSPRequest, qtssClientNotFound, sNoSDPFileFoundErr, &pathStr);
        (void)QTSS_UnlockObject(inParamBlock->inRTSPRequest);
        return err;
    }

    FileSession** theFile = NULL;
    UInt32 theLen = 0;
    QTSS_Error theErr = QTSS_GetValuePtr(inParamBlock->inClientSession, sFileSessionAttr, 0, (void**)&theFile, &theLen);
    if ((theErr != QTSS_NoErr) || (theLen != sizeof(FileSession*)))
        return QTSS_RequestFailed;

    theErr = SetupCacheBuffers(inParamBlock, theFile);  
    if (theErr != QTSS_NoErr)
        return theErr;
    
    //make sure to clear the next packet the server would have sent!
    (*theFile)->fPacketStruct.packetData = NULL;

    // Set the default quality before playing.
    QTRTPFile::RTPTrackListEntry* thePacketTrack;
    for (UInt32 x = 0; x < (*theFile)->fSDPSource.GetNumStreams(); x++)
    {
         SourceInfo::StreamInfo* theStreamInfo = (*theFile)->fSDPSource.GetStreamInfo(x);
         if (!(*theFile)->fFile.FindTrackEntry(theStreamInfo->fTrackID,&thePacketTrack))
            break;
         //(*theFile)->fFile.SetTrackQualityLevel(thePacketTrack, QTRTPFile::kAllPackets);
         (*theFile)->fFile.SetTrackQualityLevel(thePacketTrack, sDefaultStreamingQuality);
    }


    // How much are we going to tell the client to back up?
    Float32 theBackupTime = 0;

    char* thePacketRangeHeader = NULL;
    theErr = QTSS_GetValuePtr(inParamBlock->inRTSPHeaders, qtssXPacketRangeHeader, 0, (void**)&thePacketRangeHeader, &theLen);
    if (theErr == QTSS_NoErr)
    {
        StrPtrLen theRangeHdrPtr(thePacketRangeHeader, theLen);
        StringParser theRangeParser(&theRangeHdrPtr);
        
        theRangeParser.ConsumeUntil(NULL, StringParser::sDigitMask);
        UInt64 theStartPN = theRangeParser.ConsumeInteger();
        
        theRangeParser.ConsumeUntil(NULL, StringParser::sDigitMask);
        (*theFile)->fStopPN = theRangeParser.ConsumeInteger();

        theRangeParser.ConsumeUntil(NULL, StringParser::sDigitMask);
        (*theFile)->fStopTrackID = theRangeParser.ConsumeInteger();

        qtFileErr = (*theFile)->fFile.SeekToPacketNumber((*theFile)->fStopTrackID, theStartPN);
        (*theFile)->fStartTime = (*theFile)->fFile.GetRequestedSeekTime();
    }
    else
    {
        Float64* theStartTimeP = NULL;
        Float64 currentTime = 0;
        theErr = QTSS_GetValuePtr(inParamBlock->inRTSPRequest, qtssRTSPReqStartTime, 0, (void**)&theStartTimeP, &theLen);
        if ((theErr != QTSS_NoErr) || (theLen != sizeof(Float64)))
        {   // No start time so just start at the last packet ready to send
            // This packet could be somewhere out in the middle of the file.
             currentTime =  (*theFile)->fFile.GetFirstPacketTransmitTime(); 
             theStartTimeP = &currentTime;  
             (*theFile)->fStartTime = currentTime;
        }    

        Float32* theMaxBackupTime = NULL;
        theErr = QTSS_GetValuePtr(inParamBlock->inRTSPRequest, qtssRTSPReqPrebufferMaxTime, 0, (void**)&theMaxBackupTime, &theLen);
        Assert(theMaxBackupTime != NULL);
    
        if (*theMaxBackupTime == -1)
        {
            //
            // If this is an old client (doesn't send the x-prebuffer header) or an mp4 client, 
            // - don't back up to a key frame, and do not adjust the buffer time
            qtFileErr = (*theFile)->fFile.Seek(*theStartTimeP, 0);
            (*theFile)->fStartTime = *theStartTimeP;
           
            //
            // burst out -transmit time packets
            (*theFile)->fAllowNegativeTTs = false;
        }
        else
        {
            qtFileErr = (*theFile)->fFile.Seek(*theStartTimeP, *theMaxBackupTime);
            Float64 theFirstPacketTransmitTime = (*theFile)->fFile.GetFirstPacketTransmitTime();
            theBackupTime = (Float32) ( *theStartTimeP - theFirstPacketTransmitTime);
            
            //
            // For oddly authored movies, there are situations in which the packet
            // transmit time can be before the sample time. In that case, the backup
            // time may exceed the max backup time. In that case, just make the backup
            // time the max backup time.
            if (theBackupTime > *theMaxBackupTime)
                theBackupTime = *theMaxBackupTime;
            //
            // If client specifies that it can do extra buffering (new client), use the first
            // packet transmit time as the start time for this play burst. We don't need to
            // burst any packets because the client can do the extra buffering
			Bool16* overBufferEnabledPtr = NULL;
			theLen = 0;
			theErr = QTSS_GetValuePtr(inParamBlock->inClientSession, qtssCliSesOverBufferEnabled, 0, (void**)&overBufferEnabledPtr, &theLen);	
			if ((theErr == QTSS_NoErr) && (theLen == sizeof(Bool16)) && *overBufferEnabledPtr)
				(*theFile)->fStartTime = *theStartTimeP;
			else
                (*theFile)->fStartTime = *theStartTimeP - theBackupTime;            


            (*theFile)->fAllowNegativeTTs = true;
        }
    }
    
    if (qtFileErr == QTRTPFile::errCallAgain)
    {
        //
        // If we are doing RTP-Meta-Info stuff, we might be asked to get called again here.
        // This is simply because seeking might be a long operation and we don't want to
        // monopolize the CPU, but there is no other reason to wait, so just set a timeout of 0
        theErr = QTSS_SetIdleTimer(1);
        Assert(theErr == QTSS_NoErr);
        return theErr;
    }
    else if (qtFileErr != QTRTPFile::errNoError)
        return QTSSModuleUtils::SendErrorResponse(  inParamBlock->inRTSPRequest,
                                                    qtssClientBadRequest, sSeekToNonexistentTimeErr);
                                                        
    //make sure to clear the next packet the server would have sent!
    (*theFile)->fPacketStruct.packetData = NULL;
    
    // Set the movie duration and size parameters
    Float64 movieDuration = (*theFile)->fFile.GetMovieDuration();
    (void)QTSS_SetValue(inParamBlock->inClientSession, qtssCliSesMovieDurationInSecs, 0, &movieDuration, sizeof(movieDuration));
    
    UInt64 movieSize = (*theFile)->fFile.GetAddedTracksRTPBytes();
    (void)QTSS_SetValue(inParamBlock->inClientSession, qtssCliSesMovieSizeInBytes, 0, &movieSize, sizeof(movieSize));
    
    UInt32 bitsPerSecond =  (*theFile)->fFile.GetBytesPerSecond() * 8;
    (void)QTSS_SetValue(inParamBlock->inClientSession, qtssCliSesMovieAverageBitRate, 0, &bitsPerSecond, sizeof(bitsPerSecond));

    Bool16 adjustPauseTime = kAddPauseTimeToRTPTime; //keep rtp time stamps monotonically increasing
    if ( true == QTSSModuleUtils::HavePlayerProfile( sServerPrefs, inParamBlock,QTSSModuleUtils::kDisablePauseAdjustedRTPTime) )
    	adjustPauseTime = kDontAddPauseTimeToRTPTime;
    
	if (sPlayerCompatibility )  // don't change adjust setting if compatibility is off. 
		(**theFile).fAdjustPauseTime = adjustPauseTime;
	
    if ( (**theFile).fLastPauseTime > 0 )
        (**theFile).fTotalPauseTime += OS::Milliseconds() - (**theFile).fLastPauseTime;

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
        
        UInt16 theSeqNum = 0;
        UInt32 theTimestamp = (*theFile)->fFile.GetSeekTimestamp(*theTrackID); // this is the base timestamp need to add in paused time.        

        Assert(theRef != NULL);

        if ((**theFile).fAdjustPauseTime)
        {
            UInt32* theTimescale = NULL;
            QTSS_GetValuePtr(*theRef, qtssRTPStrTimescale, 0,  (void**)&theTimescale, &theLen);
            if (theLen != 0) // adjust the timestamps to reflect paused time else leave it alone we can't calculate the timestamp without a timescale.
            {
                UInt32 pauseTimeStamp = CalculatePauseTimeStamp( *theTimescale,  (*theFile)->fTotalPauseTime, (UInt32) theTimestamp);
                if (pauseTimeStamp != theTimestamp)
                      theTimestamp = pauseTimeStamp;
            }
        }
        
	    theSeqNum = (*theFile)->fFile.GetNextTrackSequenceNumber(*theTrackID);       
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
    
    //Tell the QTRTPFile whether repeat packets are wanted based on the transport
    // we don't care if it doesn't set (i.e. this is a meta info session)
     (void)  (*theFile)->fFile.SetDropRepeatPackets(allTracksReliable);// if alltracks are reliable then drop repeat packets.
        
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
    // Record the requested stop time, if there is one
    (*theFile)->fStopTime = -1;
    theLen = sizeof((*theFile)->fStopTime);
    theErr = QTSS_GetValue(inParamBlock->inRTSPRequest, qtssRTSPReqStopTime, 0, &(*theFile)->fStopTime, &theLen);
    
    //
    // Append x-Prebuffer header if provided & nonzero prebuffer needed
    if (theBackupTime > 0)
    {
        char prebufferBuf[32];
        qtss_sprintf(prebufferBuf, "time=%.5f", theBackupTime);
        StrPtrLen backupTimePtr(prebufferBuf);
        (void)QTSS_AppendRTSPHeader(inParamBlock->inRTSPRequest, qtssXPreBufferHeader,
                                    backupTimePtr.Ptr, backupTimePtr.Len);
    
    }

    // add the range header.
    {
        char rangeHeader[64];
        if (-1 == (*theFile)->fStopTime)
           (*theFile)->fStopTime = (*theFile)->fFile.GetMovieDuration();
           
        qtss_snprintf(rangeHeader,sizeof(rangeHeader) -1, "npt=%.5f-%.5f", (*theFile)->fStartTime, (*theFile)->fStopTime);
        rangeHeader[sizeof(rangeHeader) -1] = 0;
        
        StrPtrLen rangeHeaderPtr(rangeHeader);
        (void)QTSS_AppendRTSPHeader(inParamBlock->inRTSPRequest, qtssRangeHeader,
                                    rangeHeaderPtr.Ptr, rangeHeaderPtr.Len);
    
    }
    (void)QTSS_SendStandardRTSPResponse(inParamBlock->inRTSPRequest, inParamBlock->inClientSession, qtssPlayRespWriteTrackInfo);

    SInt64 adjustRTPStreamStartTimeMilli = 0;
    if (sPlayerCompatibility && QTSSModuleUtils::HavePlayerProfile(sServerPrefs, inParamBlock,QTSSModuleUtils::kDelayRTPStreamsUntilAfterRTSPResponse))
        adjustRTPStreamStartTimeMilli = sAdjustRTPStartTimeMilli;

   //Tell the server to start playing this movie. We do want it to send RTCP SRs, but
    //we DON'T want it to write the RTP header
    (*theFile)->fPaused = false;
    theErr = QTSS_Play(inParamBlock->inClientSession, inParamBlock->inRTSPRequest, qtssPlayFlagsSendRTCP);
    if (theErr != QTSS_NoErr)
       return theErr;

    // Set the adjusted play time. SendPackets can get called between QTSS_Play and
    // setting fAdjustedPlayTime below. 
    SInt64* thePlayTime = NULL;
    theErr = QTSS_GetValuePtr(inParamBlock->inClientSession, qtssCliSesPlayTimeInMsec, 0, (void**)&thePlayTime, &theLen);
    Assert(theErr == QTSS_NoErr);
    Assert(thePlayTime != NULL);
    Assert(theLen == sizeof(SInt64));
    if (thePlayTime != NULL)
        (*theFile)->fAdjustedPlayTime = adjustRTPStreamStartTimeMilli + *thePlayTime - ((SInt64)((*theFile)->fStartTime * 1000) );
 
    return QTSS_NoErr;
}

QTSS_Error SendPackets(QTSS_RTPSendPackets_Params* inParams)
{
    static const UInt32 kQualityCheckIntervalInMsec = 250;  // v331=v107
    FileSession** theFile = NULL;
    UInt32 theLen = 0;
    QTSS_Error theErr = QTSS_GetValuePtr(inParams->inClientSession, sFileSessionAttr, 0, (void**)&theFile, &theLen);
    Assert(theErr == QTSS_NoErr);
    Assert(theLen == sizeof(FileSession*));
    bool isBeginningOfWriteBurst = true;
    QTSS_Object theStream = NULL;


    if ( theFile == NULL || (*theFile)->fStartTime == -1 || (*theFile)->fPaused == true ) //something is wrong
    {
        Assert( theFile != NULL );
        Assert( (*theFile)->fStartTime != -1 );
        Assert( (*theFile)->fPaused != true );

        inParams->outNextPacketTime = qtssDontCallSendPacketsAgain;    
        return QTSS_NoErr;
    }
    
    if ( (*theFile)->fAdjustedPlayTime == 0 ) // this is system milliseconds
    {
        Assert( (*theFile)->fAdjustedPlayTime != 0 );
        inParams->outNextPacketTime = kQualityCheckIntervalInMsec;    
        return QTSS_NoErr;
    }
    
    
    QTRTPFile::RTPTrackListEntry* theLastPacketTrack = (*theFile)->fFile.GetLastPacketTrack();
    
    while (true)
    {   
        if ((*theFile)->fPacketStruct.packetData == NULL)
        { 
            Float64 theTransmitTime = (*theFile)->fFile.GetNextPacket((char**)&(*theFile)->fPacketStruct.packetData, &(*theFile)->fNextPacketLen);
            if ( QTRTPFile::errNoError != (*theFile)->fFile.Error() )
            {
                QTSS_CliSesTeardownReason reason = qtssCliSesTearDownUnsupportedMedia;
                (void) QTSS_SetValue(inParams->inClientSession, qtssCliTeardownReason, 0, &reason, sizeof(reason));
                (void)QTSS_Teardown(inParams->inClientSession);
                return QTSS_RequestFailed;
            }
           theLastPacketTrack = (*theFile)->fFile.GetLastPacketTrack();

			if (theLastPacketTrack == NULL)
				break;

            theStream = (QTSS_Object)theLastPacketTrack->Cookie1;
			Assert(theStream != NULL);
			if (theStream == NULL)
				return 0;


            //
            // Check to see if we should stop playing now
            
            if (((*theFile)->fStopTime != -1) && (theTransmitTime > (*theFile)->fStopTime))
            {
                // We should indeed stop playing
                (void)QTSS_Pause(inParams->inClientSession);
                inParams->outNextPacketTime = qtssDontCallSendPacketsAgain;

                (**theFile).fPaused = true;
                (**theFile).fLastPauseTime = OS::Milliseconds();
  
                return QTSS_NoErr;
            }
            if (((*theFile)->fStopTrackID != 0) && ((*theFile)->fStopTrackID == theLastPacketTrack->TrackID) && (theLastPacketTrack->HTCB->fCurrentPacketNumber > (*theFile)->fStopPN))
            {
                // We should indeed stop playing
                (void)QTSS_Pause(inParams->inClientSession);
                inParams->outNextPacketTime = qtssDontCallSendPacketsAgain;

                (**theFile).fPaused = true;
                (**theFile).fLastPauseTime = OS::Milliseconds();

                return QTSS_NoErr;
            }
            
            //
            // Find out what our play speed is. Send packets out at the specified rate,
            // and do so by altering the transmit time of the packet based on the Speed rate.
            Float64 theOffsetFromStartTime = theTransmitTime - (*theFile)->fStartTime;
            theTransmitTime = (*theFile)->fStartTime + (theOffsetFromStartTime / (*theFile)->fSpeed);
            
            //
            // correct for first packet xmit times that are < 0
            if (( theTransmitTime < 0.0 ) && ( !(*theFile)->fAllowNegativeTTs ))
                theTransmitTime = 0.0;
            
            (*theFile)->fPacketStruct.packetTransmitTime = (*theFile)->fAdjustedPlayTime + ((SInt64)(theTransmitTime * 1000));
  
        }
        
        //We are done playing all streams!
        if ((*theFile)->fPacketStruct.packetData == NULL)
        {
            //TODO not quite good to the last drop -- we -really- should guarantee this, also reflector
            // a write of 0 len to QTSS_Write will flush any buffered data if we're sending over tcp
            //(void)QTSS_Write((QTSS_Object)(*theFile)->fFile.GetLastPacketTrack()->Cookie1, NULL, 0, NULL, qtssWriteFlagsIsRTP);
            inParams->outNextPacketTime = qtssDontCallSendPacketsAgain;
            return QTSS_NoErr;
        }
        
        //we have a packet that needs to be sent now
        Assert(theLastPacketTrack != NULL);

        //If the stream is video, we need to make sure that QTRTPFile knows what quality level we're at
        
        if ( (!sDisableThinning) && (inParams->inCurrentTime > ((*theFile)->fLastQualityCheck + kQualityCheckIntervalInMsec) ) )
        {
            QTSS_RTPPayloadType thePayloadType = (QTSS_RTPPayloadType)theLastPacketTrack->Cookie2;
            if (thePayloadType == qtssVideoPayloadType)
            {
                (*theFile)->fLastQualityCheck = inParams->inCurrentTime;

				theStream = (QTSS_Object)theLastPacketTrack->Cookie1;
				Assert(theStream != NULL);
				if (theStream == NULL)
					return 0;

                // Get the current quality level in the stream, and this stream's TrackID.
                UInt32* theQualityLevel = 0;
                theErr = QTSS_GetValuePtr(theStream, qtssRTPStrQualityLevel, 0, (void**)&theQualityLevel, &theLen);
                Assert(theErr == QTSS_NoErr);
                Assert(theQualityLevel != NULL);
                Assert(theLen == sizeof(UInt32));

                (*theFile)->fFile.SetTrackQualityLevel(theLastPacketTrack, *theQualityLevel);
            }
        }

        // Send the packet!
        QTSS_WriteFlags theFlags = qtssWriteFlagsIsRTP;
        if (isBeginningOfWriteBurst)
            theFlags |= qtssWriteFlagsWriteBurstBegin;

        theStream = (QTSS_Object)theLastPacketTrack->Cookie1;
		Assert(theStream != NULL);
		if (theStream == NULL)
			return 0;

        //adjust the timestamp so it reflects paused time.
        void* packetDataPtr =  (*theFile)->fPacketStruct.packetData;
        UInt32 currentTimeStamp = GetPacketTimeStamp(packetDataPtr);
        UInt32 pauseTimeStamp = SetPausetimeTimeStamp(*theFile, theStream, currentTimeStamp);
        
  		UInt16 curSeqNum = GetPacketSequenceNumber(theStream);
        (void) QTSS_SetValue(theStream, sRTPStreamLastPacketSeqNumAttrID, 0, &curSeqNum, sizeof(curSeqNum));
 
		theErr = QTSS_Write(theStream, &(*theFile)->fPacketStruct, (*theFile)->fNextPacketLen, NULL, theFlags);
        
        isBeginningOfWriteBurst = false;
        
        if ( theErr == QTSS_WouldBlock )
        { 

            if (currentTimeStamp != pauseTimeStamp) // reset the packet time stamp so we adjust it again when we really do send it
               SetPacketTimeStamp(currentTimeStamp, packetDataPtr);
           
            //
            // In the case of a QTSS_WouldBlock error, the packetTransmitTime field of the packet struct will be set to
            // the time to wakeup, or -1 if not known.
            // If the time to wakeup is not given by the server, just give a fixed guess interval
            if ((*theFile)->fPacketStruct.suggestedWakeupTime == -1)
                inParams->outNextPacketTime = sFlowControlProbeInterval;    // for buffering, try me again in # MSec
            else
            {
                Assert((*theFile)->fPacketStruct.suggestedWakeupTime > inParams->inCurrentTime);
                inParams->outNextPacketTime = (*theFile)->fPacketStruct.suggestedWakeupTime - inParams->inCurrentTime;
            }
            
            //qtss_printf("Call again: %qd\n", inParams->outNextPacketTime);
                
            return QTSS_NoErr;
        }
        else
        {

          (void) QTSS_SetValue(theStream, sRTPStreamLastSentPacketSeqNumAttrID, 0, &curSeqNum, sizeof(curSeqNum));
          (*theFile)->fPacketStruct.packetData = NULL;
        }
    }
    
    return QTSS_NoErr;
}

QTSS_Error DestroySession(QTSS_ClientSessionClosing_Params* inParams)
{
    FileSession** theFile = NULL;
    UInt32 theLen = 0;
    QTSS_Error theErr = QTSS_GetValuePtr(inParams->inClientSession, sFileSessionAttr, 0, (void**)&theFile, &theLen);
    if ((theErr != QTSS_NoErr) || (theLen != sizeof(FileSession*)) || (theFile == NULL))
        return QTSS_RequestFailed;

    //
    // Tell the ClientSession how many samples we skipped because of stream thinning
    UInt32 theNumSkippedSamples = (*theFile)->fFile.GetNumSkippedSamples();
    (void)QTSS_SetValue(inParams->inClientSession, qtssCliSesFramesSkipped, 0, &theNumSkippedSamples, sizeof(theNumSkippedSamples));
    
    DeleteFileSession(*theFile);
    return QTSS_NoErr;
}

void    DeleteFileSession(FileSession* inFileSession)
{   
    delete inFileSession;
}
