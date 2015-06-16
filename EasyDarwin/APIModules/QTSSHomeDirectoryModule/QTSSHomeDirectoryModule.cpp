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
    File:       QTSSHomeDirectoryModule.cpp

    Contains:   Module that expands ~ in request URLs to home directories
	    
 */
    
#include <stdio.h>      
#include <stdlib.h>
#include "SafeStdLib.h"
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "OSMemory.h"
#include "StringParser.h"
#include "ResizeableStringFormatter.h"
#include "QTSSModuleUtils.h"
#include "OSArrayObjectDeleter.h"
#include "DirectoryInfo.h"
#include "QTSSMemoryDeleter.h"

#include "QTSSHomeDirectoryModule.h"

#define	HOME_DIRECTORY_MODULE_DEBUGGING 0

// STATIC DATA
static QTSS_ServerObject		sServer						= NULL;
static QTSS_ModuleObject        sModule						= NULL;
static QTSS_ModulePrefsObject	sPrefs						= NULL;

// Attributes
static char*					sIsFirstRequestName			= "QTSSHomeDirectoryModuleIsFirstRequest";
static QTSS_AttributeID         sIsFirstRequestAttr			= qtssIllegalAttrID;
static char*					sRequestHomeDirAttrName			= "QTSSHomeDirectoryModuleHomeDir";
static QTSS_AttributeID         sRequestHomeDirAttr				= qtssIllegalAttrID;
static char*					sSessionHomeDirAttrName			= "QTSSHomeDirectoryModuleHomeDir";
static QTSS_AttributeID         sSessionHomeDirAttr				= qtssIllegalAttrID;

// Module description and version
static char*					sDescription				= "Provides support for streaming from users' home directories";
static UInt32                   sVersion 					= 0x00010000;

// Module preferences and their defaults
static Bool16					sEnabled					= false;
static Bool16					kDefaultEnabled				= false;
static char*					sMoviesDirectory			= NULL; // Streaming dir in user's home dir
static char*					kDefaultMoviesDirectory		= "/Sites/Streaming/";
static UInt32                   sMaxNumConnsPerHomeDir		= 0; //Max conns. allowed per home directory
static UInt32                   kDefaultMaxNumConnsPerHomeDir	= 0; // 0 => no limit enforced
static UInt32                   sMaxBWInKbpsPerHomeDir			= 0; // Max BW allowed per home directory
static UInt32                   kDefaultMaxBWInKbpsPerHomeDir	= 0; // 0 => no limit enforced

enum { kDefaultHomeDirSize = 64 };
static Bool16 					sTrue						= true;

static OSRefTable*	sDirectoryInfoMap = NULL;

// FUNCTIONS
static QTSS_Error	QTSSHomeDirectoryDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error	Register(QTSS_Register_Params* inParams);
static QTSS_Error	Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error	RereadPrefs();
static QTSS_Error	RewriteRequestFilePathAndRootDir(QTSS_StandardRTSP_Params* inParams);
static void			RewriteRootDir(QTSS_StandardRTSP_Params* inParams, StrPtrLen* inHomeDir);
static QTSS_Error	AuthorizeRequest(QTSS_StandardRTSP_Params* inParams);
static QTSS_Error	RemoveClientSessionFromMap(QTSS_ClientSessionClosing_Params* inParams);

// Helper functions
static Bool16		ExpandPath(const StrPtrLen *inPathPtr, StrPtrLen *outPathPtr);

// FUNCTION IMPLEMENTATIONS
QTSS_Error QTSSHomeDirectoryModule_Main(void* inPrivateArgs)
{
    return _stublibrary_main(inPrivateArgs, QTSSHomeDirectoryDispatch);
}


QTSS_Error QTSSHomeDirectoryDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
    switch (inRole)
    {
        case QTSS_Register_Role:
            return Register(&inParams->regParams);
        case QTSS_Initialize_Role:
            return Initialize(&inParams->initParams);
        case QTSS_RereadPrefs_Role:
            return RereadPrefs();
        case QTSS_RTSPRoute_Role:
			{
				if (!sEnabled) 
					break;
				return RewriteRequestFilePathAndRootDir(&inParams->rtspRouteParams);
			} 
		case QTSS_RTSPAuthorize_Role:
			{
				if (!sEnabled) 
					break;
				return AuthorizeRequest(&inParams->rtspAuthParams);
			}
		case QTSS_ClientSessionClosing_Role:
			{
				if (!sEnabled) 
					break;
				return RemoveClientSessionFromMap(&inParams->clientSessionClosingParams);
			}
    }
    return QTSS_NoErr;
}


QTSS_Error Register(QTSS_Register_Params* inParams)
{
    // Do role & attribute setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_RereadPrefs_Role);
	(void)QTSS_AddRole(QTSS_RTSPRoute_Role);
    (void)QTSS_AddRole(QTSS_RTSPAuthorize_Role);
	(void)QTSS_AddRole(QTSS_ClientSessionClosing_Role);
    
	// Add an RTSP session attribute to track if the request is the first request of the session
	// so that the bandwidth/# of connections quota check can be done only if it is the first request
	(void)QTSS_AddStaticAttribute(qtssRTSPSessionObjectType, sIsFirstRequestName, NULL, qtssAttrDataTypeBool16);
	(void)QTSS_IDForAttr(qtssRTSPSessionObjectType, sIsFirstRequestName, &sIsFirstRequestAttr);
		
	// Add an RTSP request object attribute for tracking the home directory it is being served from
    (void)QTSS_AddStaticAttribute(qtssRTSPRequestObjectType, sRequestHomeDirAttrName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssRTSPRequestObjectType, sRequestHomeDirAttrName, &sRequestHomeDirAttr);
	
	// Add an RTP session object attribute for tracking the home directory it is being served from
    (void)QTSS_AddStaticAttribute(qtssClientSessionObjectType, sSessionHomeDirAttrName, NULL, qtssAttrDataTypeCharArray);
    (void)QTSS_IDForAttr(qtssClientSessionObjectType, sSessionHomeDirAttrName, &sSessionHomeDirAttr);


    // Tell the server our name!
    static char* sModuleName = "QTSSHomeDirectoryModule";
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
    sPrefs = QTSSModuleUtils::GetModulePrefsObject(sModule);
	
	// Set our version and description
	(void)QTSS_SetValue(sModule, qtssModDesc, 0, sDescription, ::strlen(sDescription));   
    (void)QTSS_SetValue(sModule, qtssModVersion, 0, &sVersion, sizeof(sVersion)); 
	
    RereadPrefs();
	
	sDirectoryInfoMap = NEW OSRefTable();
	
	return QTSS_NoErr;
}


QTSS_Error RereadPrefs()
{
	QTSSModuleUtils::GetAttribute(sPrefs, "enabled", qtssAttrDataTypeBool16, 
								&sEnabled, &kDefaultEnabled, sizeof(sEnabled));
    delete [] sMoviesDirectory;
    sMoviesDirectory = QTSSModuleUtils::GetStringAttribute(sPrefs, "movies_directory", kDefaultMoviesDirectory);
	QTSSModuleUtils::GetAttribute(sPrefs, "max_num_conns_per_home_directory", qtssAttrDataTypeUInt32, 
								&sMaxNumConnsPerHomeDir, &kDefaultMaxNumConnsPerHomeDir, sizeof(sMaxNumConnsPerHomeDir));
	QTSSModuleUtils::GetAttribute(sPrefs, "max_bandwidth_kbps_per_home_directory", qtssAttrDataTypeUInt32, 
								&sMaxBWInKbpsPerHomeDir, &kDefaultMaxBWInKbpsPerHomeDir, sizeof(sMaxBWInKbpsPerHomeDir));
	return QTSS_NoErr;
}


QTSS_Error RewriteRequestFilePathAndRootDir(QTSS_StandardRTSP_Params* inParams)
{
	QTSS_RTSPRequestObject theRequest = inParams->inRTSPRequest;
	
	char* requestPathStr;
	(void)QTSS_GetValueAsString(theRequest, qtssRTSPReqFilePath, 0, &requestPathStr);
    QTSSCharArrayDeleter requestPathStrDeleter(requestPathStr);
	StrPtrLen theRequestPath(requestPathStr);
	StringParser theRequestPathParser(&theRequestPath);
	
	// request path begins with a '/' for ex. /mysample.mov or /~joe/mysample.mov
	theRequestPathParser.Expect('/');
	
	if (theRequestPathParser.PeekFast() == '~')
	{	
		theRequestPathParser.Expect('~');
		
		StrPtrLen theFirstPath;
		theRequestPathParser.ConsumeUntil(&theFirstPath, '/');
		if (theFirstPath.Len != 0) // if the URI is /~/mysample.mov --> do nothing
		{
			StrPtrLen theHomeDir;
			// ExpandPath allocates memory, so delete it before exiting
			if (ExpandPath(&theFirstPath, &theHomeDir))
			{
				// Rewrite the qtssRTSPReqRootDir attribute
				RewriteRootDir(inParams, &theHomeDir);
				
				StrPtrLen theStrippedRequestPath;
				theRequestPathParser.ConsumeLength(&theStrippedRequestPath, theRequestPathParser.GetDataRemaining());
				(void) QTSS_SetValue(theRequest, qtssRTSPReqFilePath, 0, theStrippedRequestPath.Ptr, theStrippedRequestPath.Len);
			}
			theHomeDir.Delete();
		}
	}

	return QTSS_NoErr;
}

void RewriteRootDir(QTSS_StandardRTSP_Params* inParams, StrPtrLen* inHomeDir)
{
    QTSS_RTSPRequestObject theRequest = inParams->inRTSPRequest;
	QTSS_ClientSessionObject theSession = inParams->inClientSession;
	QTSS_Error theErr = QTSS_NoErr;
	
	Assert(inHomeDir->Len != 0);
	if (inHomeDir->Len == 0)
		return;
		
	char homeDirFormatterBuf[kDefaultHomeDirSize] = "";
	ResizeableStringFormatter theHomeDirFormatter(homeDirFormatterBuf, kDefaultHomeDirSize);
	theHomeDirFormatter.Put(inHomeDir->Ptr, inHomeDir->Len);
	// Append a path separator if the movies directory doesn't begin with it
	if ((::strlen(sMoviesDirectory) != 0) && (sMoviesDirectory[0] != '/'))
		theHomeDirFormatter.PutChar('/');
	// Append the movies_directory
	theHomeDirFormatter.Put(sMoviesDirectory);
	
	StrPtrLen theRootDir(theHomeDirFormatter.GetBufPtr(), theHomeDirFormatter.GetBytesWritten());
	
	// Set qtssRTSPReqRootDir
	(void)QTSS_SetValue(theRequest, qtssRTSPReqRootDir, 0, theRootDir.Ptr, theRootDir.Len);
	
	// Set the client session's home dir attribute
	(void)QTSS_SetValue(theSession, sSessionHomeDirAttr, 0, theRootDir.Ptr, theRootDir.Len);

	// If the request is a PLAY, then add this to the session list for the home directory
	QTSS_RTSPMethod *theMethod = NULL;
	UInt32 theLen = 0;
	(void)QTSS_GetValuePtr(theRequest, qtssRTSPReqMethod, 0, (void **)&theMethod, &theLen);
	if (*theMethod == qtssPlayMethod)
	{
		OSRef* theDirectoryInfoRef = sDirectoryInfoMap->Resolve(&theRootDir);
		if (theDirectoryInfoRef == NULL)
		{
			DirectoryInfo* newDirInfo = NEW DirectoryInfo(&theRootDir);
			theErr = sDirectoryInfoMap->Register(newDirInfo->GetRef());
			Assert(theErr == QTSS_NoErr);
			theDirectoryInfoRef = sDirectoryInfoMap->Resolve(&theRootDir);
			Assert (theDirectoryInfoRef == newDirInfo->GetRef());
		}
		
		DirectoryInfo* theDirInfo = (DirectoryInfo *)theDirectoryInfoRef->GetObject();
		theDirInfo->AddSession(&theSession);
		sDirectoryInfoMap->Release(theDirectoryInfoRef);
	} 

#if HOME_DIRECTORY_MODULE_DEBUGGING
	char* newRequestPath = NULL;
	theErr = QTSS_GetValueAsString (theRequest, qtssRTSPReqFilePath, 0, &newRequestPath);
	qtss_printf("QTSSHomeDirectoryModule::RewriteRootDir qtssRTSPReqFilePath = %s\n", newRequestPath);
	QTSS_Delete(newRequestPath);

	char* newRequestRootDir = NULL;
	theErr = QTSS_GetValueAsString (theRequest, qtssRTSPReqRootDir, 0, &newRequestRootDir);
	qtss_printf("QTSSHomeDirectoryModule::RewriteRootDir qtssRTSPReqRootDir = %s\n", newRequestRootDir);
	QTSS_Delete(newRequestRootDir);
#endif

}


QTSS_Error AuthorizeRequest(QTSS_StandardRTSP_Params* inParams)
{
    // Only do anything if this is the first request	
	Bool16 *isFirstRequest	= NULL;
	UInt32 theLen = sizeof(isFirstRequest);
    (void)QTSS_GetValuePtr(inParams->inRTSPSession, sIsFirstRequestAttr, 0, (void**)&isFirstRequest, &theLen);
    if (isFirstRequest != NULL)
        return QTSS_NoErr;
	
	OSRef *theDirectoryInfoRef = NULL;
	DirectoryInfo *theDirInfo = NULL;
	StrPtrLen theHomeDir;
	
	// if no limits are imposed, do nothing
	if ((sMaxNumConnsPerHomeDir == 0) && (sMaxBWInKbpsPerHomeDir == 0))
		goto end_authorize;
		
	// Get this client session's home dir
	(void)QTSS_GetValuePtr(inParams->inClientSession, sSessionHomeDirAttr, 0, (void**)&theHomeDir.Ptr, &theHomeDir.Len);
	if ((theHomeDir.Ptr == NULL) || (theHomeDir.Len == 0)) // If it's NULL it's not served out of the home dir
		goto end_authorize;
	
	// Get the sessions for this home dir
	theDirectoryInfoRef = sDirectoryInfoMap->Resolve(&theHomeDir); 
	if (theDirectoryInfoRef == NULL) // No sessions exist yet for this homeDir
		goto end_authorize;
	
	theDirInfo = (DirectoryInfo *)(theDirectoryInfoRef->GetObject());

	if ((sMaxNumConnsPerHomeDir != 0) && ((theDirInfo->NumSessions()) >= sMaxNumConnsPerHomeDir))
		QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssClientNotEnoughBandwidth, qtssMsgTooManyClients);
	else if((sMaxBWInKbpsPerHomeDir != 0) && ((theDirInfo->CurrentTotalBandwidthInKbps()) >= sMaxBWInKbpsPerHomeDir))
		QTSSModuleUtils::SendErrorResponse(inParams->inRTSPRequest, qtssClientNotEnoughBandwidth, qtssMsgTooMuchThruput);
		
	sDirectoryInfoMap->Release(theDirectoryInfoRef);
	
end_authorize:	
	// Mark the request so we'll know subsequent ones aren't the first.
    (void)QTSS_SetValue(inParams->inRTSPSession, sIsFirstRequestAttr, 0, &sTrue, sizeof(sTrue));
	
	return QTSS_NoErr;
}

QTSS_Error RemoveClientSessionFromMap(QTSS_ClientSessionClosing_Params* inParams)
{
	QTSS_ClientSessionObject theSession = inParams->inClientSession;
	
	StrPtrLen theHomeDir;    
    (void)QTSS_GetValuePtr (theSession, sSessionHomeDirAttr, 0, (void**)&theHomeDir.Ptr, &theHomeDir.Len);
    if ((theHomeDir.Ptr != NULL) && (theHomeDir.Len != 0))
	{
		OSRef* theDirectoryInfoRef = sDirectoryInfoMap->Resolve(&theHomeDir);
		if (theDirectoryInfoRef != NULL)
		{
			DirectoryInfo *theDirInfo = (DirectoryInfo *)theDirectoryInfoRef->GetObject();
			theDirInfo->RemoveSession(&theSession);
			sDirectoryInfoMap->Release(theDirectoryInfoRef);
			(void)sDirectoryInfoMap->TryUnRegister(theDirectoryInfoRef);
		}
	}
	
	return QTSS_NoErr;
}

// Expand path expands the ~ or ~username to the home directory of the user
// It allocates memory for the outPathPtr->Ptr, so the caller should delete it
// inPathPtr has the tilde stripped off, so inPathPtr->Len == 0 is valid
Bool16 ExpandPath(const StrPtrLen *inPathPtr, StrPtrLen *outPathPtr)
{    
    Assert(inPathPtr != NULL);
    Assert(outPathPtr != NULL);

	char *newPath = NULL;
   
	if (inPathPtr->Len != 0)
	{
        char* inPathStr = inPathPtr->GetAsCString();
        struct passwd *passwdStruct = getpwnam(inPathStr);
        delete [] inPathStr;
        if (passwdStruct != NULL)
            newPath = passwdStruct->pw_dir;
    }


/*
	if (inPathPtr->Len == 0)
	{
		newPath = getenv("HOME");
        
		if (newPath == NULL)
        {
            struct passwd *passwdStruct = getpwuid(getuid());
            if (passwdStruct != NULL)
                newPath = passwdStruct->pw_dir;
        }
	}
*/

    if (newPath == NULL)
        return false;

    UInt32 newPathLen = ::strlen(newPath);
    char* newPathStr = NEW char[newPathLen + 1];
    ::strcpy(newPathStr, newPath);
    
    outPathPtr->Ptr = newPathStr;
    outPathPtr->Len = newPathLen;
    
    return true;
}
