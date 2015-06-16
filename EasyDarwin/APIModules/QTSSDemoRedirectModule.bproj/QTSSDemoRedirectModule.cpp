/*
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Copyright (c) 2008 Apple Inc.  All Rights Reserved.
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
    File:       QTSSDemoRedirectModule.cpp

    Contains:   Implementation of QTSSDemoRedirectModule
*/

#include "QTSSDemoRedirectModule.h"
#include "StrPtrLen.h"
#include "QTSSModuleUtils.h"
#include "SafeStdLib.h"
#include "QTSSMemoryDeleter.h"

// STATIC DATA
static QTSS_ServerObject        sServer         = NULL;
static QTSS_ModuleObject        sModule         = NULL;
static QTSS_ModulePrefsObject	sModulePrefs    = NULL;
static StrPtrLen sRedirect("RTSP/1.0 302 Found\r\nServer: QTSS/6.0\r\nCSeq: 1\r\nConnection: Close");
static StrPtrLen sLocation("\r\nLocation: ");
static StrPtrLen sRedirectEnd("\r\n\r\n");
static const UInt32 kBuffSize = 512;

// Module description and version
static char*            sDescription        = "Demonstrates a way to redirect file requests based on client bandwidth headers";
static UInt32           sVersion            = 0x00010000;

// FUNCTION PROTOTYPES

static QTSS_Error QTSSDemoRedirectModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error Register(QTSS_Register_Params* inParams);
static QTSS_Error Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error Shutdown();
static QTSS_Error RereadPrefs();
static QTSS_Error Authenticate(QTSS_StandardRTSP_Params* inParams);

// Module preferences and their defaults
static Bool16               sEnabled            = false;
static Bool16               kDefaultEnabled     = true;
static char*                sRedirectUrl        = NULL;
static char*                sRedirectUrlDefault = "sample";

// FUNCTION IMPLEMENTATIONS

QTSS_Error QTSSDemoRedirectModule_Main(void* inPrivateArgs)
{
     return _stublibrary_main(inPrivateArgs, QTSSDemoRedirectModuleDispatch);
}


QTSS_Error  QTSSDemoRedirectModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
   switch (inRole)
    {  
       case QTSS_Register_Role:
            return Register(&inParams->regParams);
        case QTSS_Initialize_Role:
            return Initialize(&inParams->initParams);
        case QTSS_RereadPrefs_Role:
            return RereadPrefs();
        case QTSS_RTSPAuthenticate_Role:
            return Authenticate(&inParams->rtspRequestParams);
        case QTSS_Shutdown_Role:
            return Shutdown();
    }
    return QTSS_NoErr;
}


QTSS_Error Register(QTSS_Register_Params* inParams)
{
    // Do role & attribute setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_RereadPrefs_Role);
    (void)QTSS_AddRole(QTSS_RTSPAuthenticate_Role);

    // Tell the server our name
    static char* sModuleName = "QTSSDemoRedirectModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    return QTSS_NoErr;
}


QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{

    // Setup module utils
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);

    // Get the server object
    sServer = inParams->inServer;
    
    // Get our prefs object
    sModule = inParams->inModule;
    sModulePrefs = QTSSModuleUtils::GetModulePrefsObject(sModule);

    // Set our version and description
    (void)QTSS_SetValue(sModule, qtssModDesc, 0, sDescription, ::strlen(sDescription));   
    (void)QTSS_SetValue(sModule, qtssModVersion, 0, &sVersion, sizeof(sVersion)); 

    RereadPrefs();
    return QTSS_NoErr;
}

QTSS_Error Shutdown()
{
    return QTSS_NoErr;
}

QTSS_Error RereadPrefs()
{
    QTSSModuleUtils::GetAttribute(sModulePrefs, "enabled", qtssAttrDataTypeBool16, 
        &sEnabled, &kDefaultEnabled, sizeof(sEnabled));

    sRedirectUrl = QTSSModuleUtils::GetStringAttribute(sModulePrefs, "url_to_redirect", sRedirectUrlDefault);                           

    return QTSS_NoErr;
}


QTSS_Error Authenticate(QTSS_StandardRTSP_Params* inParams)
{

    if  ( (NULL == inParams) || (NULL == inParams->inRTSPRequest) ) {
        return QTSS_RequestFailed;
    }
    
    QTSS_RTSPRequestObject  theRTSPRequest = inParams->inRTSPRequest;
    QTSS_Error theErr = QTSS_NoErr;
        
    char *movieUrlTrunc = NULL;
    theErr = QTSS_GetValueAsString(theRTSPRequest,qtssRTSPReqTruncAbsoluteURL, 0, &movieUrlTrunc);
    QTSSCharArrayDeleter movieUrlTruncDeleter(movieUrlTrunc);
    if (theErr != QTSS_NoErr)
        return QTSS_RequestFailed;

    char *movieUrlFilename = NULL;
    theErr = QTSS_GetValueAsString(theRTSPRequest,qtssRTSPReqFileName, 0, &movieUrlFilename);
    QTSSCharArrayDeleter movieUrlFilenameDeleter(movieUrlFilename);
    if (theErr != QTSS_NoErr)
        return QTSS_RequestFailed;
    
    // Return unless the requested filename matches the redirect URL
    if (::strncmp(movieUrlFilename, sRedirectUrl,  (::strlen(movieUrlFilename) <= ::strlen(sRedirectUrl)) 
            ? ::strlen(sRedirectUrl) : ::strlen(movieUrlFilename)))
        return QTSS_NoErr;
    
    // Get the client's bandwidth header
    UInt32 bandwidthBits = 0;
    UInt32 len = sizeof(bandwidthBits);
	(void)QTSS_GetValue(theRTSPRequest, qtssRTSPReqBandwidthBits, 0, (void *)&bandwidthBits, &len);

    // Prepare a filename extension to use for redirection.
    // These numbers match the "Streaming Speed" options in the QuickTime Player 7 settings.
    // The extension will be appended to the requested URL.
    char theNewFilename[kBuffSize];
    ::memset(theNewFilename, 0, kBuffSize);
    
    if (bandwidthBits <= 0) {
        qtss_snprintf(theNewFilename, sizeof(theNewFilename), "%s/%s_50kbit.3gp", movieUrlTrunc, movieUrlFilename);
    } else if (bandwidthBits <= 28000) {
        qtss_snprintf(theNewFilename, sizeof(theNewFilename), "%s/%s_50kbit.3gp", movieUrlTrunc, movieUrlFilename);
    } else if (bandwidthBits <= 56000) {
        qtss_snprintf(theNewFilename, sizeof(theNewFilename), "%s/%s_50kbit.3gp", movieUrlTrunc, movieUrlFilename);
    } else if (bandwidthBits <= 112000) {
        qtss_snprintf(theNewFilename, sizeof(theNewFilename), "%s/%s_100kbit.mov", movieUrlTrunc, movieUrlFilename);
    } else if (bandwidthBits <= 256000) {
        qtss_snprintf(theNewFilename, sizeof(theNewFilename), "%s/%s_100kbit.mov", movieUrlTrunc, movieUrlFilename);
    } else if (bandwidthBits <= 384000) {
        qtss_snprintf(theNewFilename, sizeof(theNewFilename), "%s/%s_300kbit.mov", movieUrlTrunc, movieUrlFilename);
    } else if (bandwidthBits <= 512000) {
        qtss_snprintf(theNewFilename, sizeof(theNewFilename), "%s/%s_300kbit.mov", movieUrlTrunc, movieUrlFilename);
    } else if (bandwidthBits <= 768000) {
        qtss_snprintf(theNewFilename, sizeof(theNewFilename), "%s/%s_300kbit.mov", movieUrlTrunc, movieUrlFilename);
    } else if (bandwidthBits <= 1000000) {
        qtss_snprintf(theNewFilename, sizeof(theNewFilename), "%s/%s_h264_1mbit.mp4", movieUrlTrunc, movieUrlFilename);
    } else if (bandwidthBits <= 1500000) {
        qtss_snprintf(theNewFilename, sizeof(theNewFilename), "%s/%s_h264_1mbit.mp4", movieUrlTrunc, movieUrlFilename);
    } else {
        qtss_snprintf(theNewFilename, sizeof(theNewFilename), "%s/%s_h264_1mbit.mp4", movieUrlTrunc, movieUrlFilename);
    }
    
    // In order to send the redirect, we need to get a QTSS_StreamRef
    // so we can send data to the client. Get the QTSS_StreamRef out of the request.
    QTSS_StreamRef* theStreamRef = NULL;
    UInt32 strRefLen = 0;
        
    theErr = QTSS_GetValuePtr(theRTSPRequest, qtssRTSPReqStreamRef, 0,
        (void**)&theStreamRef, &strRefLen);
    if (( QTSS_NoErr != theErr ) || ( sizeof(QTSS_StreamRef) != strRefLen) ) {
        return QTSS_RequestFailed;
    }

    // Send the redirect
    UInt32 theLenWritten = 0;
    
    (void)QTSS_Write(*theStreamRef, sRedirect.Ptr, sRedirect.Len, &theLenWritten, 0);
    (void)QTSS_Write(*theStreamRef, sLocation.Ptr, sLocation.Len, &theLenWritten, 0);
    (void)QTSS_Write(*theStreamRef, theNewFilename, ::strlen(theNewFilename), &theLenWritten, 0);
    (void)QTSS_Write(*theStreamRef, sRedirectEnd.Ptr, sRedirectEnd.Len, &theLenWritten, 0);
    (void)QTSS_Flush(*theStreamRef);
     
    return QTSS_NoErr;
}