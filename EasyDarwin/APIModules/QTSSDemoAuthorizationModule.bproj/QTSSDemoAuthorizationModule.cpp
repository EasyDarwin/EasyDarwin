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
    File:       QTSSDemoAuthorizationModule.cpp

    Contains:   Implementation of QTSSDemoAuthorizationModule, which demonstrates an authorization method using 
                client IP addresses for access control.  Connections from IP addresses not included in the "IPAccessList"
                preference will be immediately sent an RTSP "401 Unauthorized" error.
				
                Example of preferences allowing only connections from loopback (or any address in the 127.0.0.X network):
                <MODULE NAME="QTSSDemoAuthorizationModule" >
                    <PREF NAME="enabled" TYPE="Bool16" >true</PREF>
                    <PREF NAME="IPAccessList" >127.0.0.*</PREF>
                </MODULE>
*/

#include "QTSSDemoAuthorizationModule.h"
#include "defaultPaths.h"
#include "StrPtrLen.h"
#include "QTSSModuleUtils.h"
#include "OSArrayObjectDeleter.h"
#include "SafeStdLib.h"
#include "QTSSMemoryDeleter.h"
#include "StringParser.h"
#include "OSMemory.h"

// STATIC DATA
static QTSS_ServerObject        sServer         = NULL;
static QTSS_ModuleObject        sModule         = NULL;
static QTSS_ModulePrefsObject	sModulePrefs    = NULL;

const UInt32 kBuffLen = 512;

// Module description and version
static char*            sDescription        = "Demonstrates an authorization method using client IP addresses for access control";
static UInt32           sVersion            = 0x00010000;

// FUNCTION PROTOTYPES

static QTSS_Error QTSSDemoAuthorizationModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error Register(QTSS_Register_Params* inParams);
static QTSS_Error Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error Shutdown();
static QTSS_Error RereadPrefs();
static QTSS_Error Authenticate();
static QTSS_Error Authorize(QTSS_StandardRTSP_Params* inParams);
static Bool16 AcceptSession(QTSS_RTSPSessionObject inRTSPSession);

// Module preferences and their defaults
static Bool16               sEnabled            = false;
static Bool16               kDefaultEnabled     = false;
static char*                sIPAccessList       = NULL;
static QTSS_AttributeID     sIPAccessListID     = qtssIllegalAttrID;

// FUNCTION IMPLEMENTATIONS

QTSS_Error QTSSDemoAuthorizationModule_Main(void* inPrivateArgs)
{
     return _stublibrary_main(inPrivateArgs, QTSSDemoAuthorizationModuleDispatch);
}


QTSS_Error  QTSSDemoAuthorizationModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
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
            return Authenticate();
        case QTSS_RTSPAuthorize_Role:
            return Authorize(&inParams->rtspRequestParams);
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
    (void)QTSS_AddRole(QTSS_RTSPAuthorize_Role);

    // Tell the server our name
    static char* sModuleName = "QTSSDemoAuthorizationModule";
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

    delete [] sIPAccessList;
    sIPAccessList = QTSSModuleUtils::GetStringAttribute(sModulePrefs, "IPAccessList", "127.0.0.*");
    sIPAccessListID = QTSSModuleUtils::GetAttrID(sModulePrefs, "IPAccessList");
    
    return QTSS_NoErr;
}

// This is not necessary, but could be used to perform Authentication Role actions
QTSS_Error Authenticate()
{
    return QTSS_NoErr;
}

QTSS_Error Authorize(QTSS_StandardRTSP_Params* inParams)
{
    QTSS_Error              theErr = QTSS_NoErr;
    QTSS_RTSPRequestObject  theRTSPRequest = inParams->inRTSPRequest;
    QTSS_RTSPSessionObject  theRTSPSession = inParams->inRTSPSession;
    QTSS_ActionFlags noAction = ~qtssActionFlagsRead; //only handle read

    
    QTSS_ActionFlags authorizeAction = QTSSModuleUtils::GetRequestActions(theRTSPRequest);
    Bool16 allowRequest = false;

    if ((authorizeAction & noAction) != 0)
        return QTSS_NoErr;
        
    allowRequest = AcceptSession(theRTSPSession);

    if( (theErr != QTSS_NoErr) || (allowRequest == false) ) //handle it
    {     
        //Access denied
        Bool16 allowed = false;
        (void) QTSSModuleUtils::SendErrorResponse(theRTSPRequest, qtssClientUnAuthorized, 0);
        (void) QTSSModuleUtils::AuthorizeRequest(theRTSPRequest, &allowed, &allowed, &allowed);
    } else {
        //Access granted
        Bool16 allowed = true;
        (void) QTSSModuleUtils::AuthorizeRequest(theRTSPRequest, &allowed, &allowed, &allowed);
    }

    return QTSS_NoErr;
}

Bool16 AcceptSession(QTSS_RTSPSessionObject inRTSPSession)
{   
    char remoteAddress[20] = {0};
    StrPtrLen theClientIPAddressStr(remoteAddress,sizeof(remoteAddress));
    QTSS_Error err = QTSS_GetValue(inRTSPSession, qtssRTSPSesRemoteAddrStr, 0, (void*)theClientIPAddressStr.Ptr, &theClientIPAddressStr.Len);
    if (err != QTSS_NoErr) return false;

    if  (QTSSModuleUtils::AddressInList(sModulePrefs, sIPAccessListID, &theClientIPAddressStr))
        return true;
    return false;
}