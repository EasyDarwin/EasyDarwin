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
    File:       QTSSODSAuthModule.cpp

    Contains:   Implementation of QTSSDSAuthModule, a modified version of the AuthenticateRequestModule
                is sample code. 
                    


*/

#include "QTSSDSAuthModule.h"


#include "defaultPaths.h"
#include "DSAccessChecker.h"
#include "StrPtrLen.h"
#include "QTSSModuleUtils.h"
#include "OSArrayObjectDeleter.h"
#include "SafeStdLib.h"
#include "QTSSMemoryDeleter.h"
#include "QTSS_Private.h"
#include "OS.h"

//#define SACL 1
#if OSX_SACL
extern "C"
{
    #include <membershipPriv.h>
}
#include <membership.h>
#include <errno.h>
#endif

// ATTRIBUTES

// STATIC DATA

const UInt32 kBuffLen = 512;
#define MODPREFIX_ "modDSAuth_"
#define AUTHDEBUG 0
#define debug_printf if (AUTHDEBUG) qtss_printf


static QTSS_ModulePrefsObject   sPrefs = NULL;
static QTSS_PrefsObject         sServerPrefs = NULL;
static OSMutex*                 sAuthMutex   = NULL;
static Bool16                   sDefaultAuthenticationEnabled   = true;
static Bool16                   sAuthenticationEnabled          = true;
static char*                    sDefaultAccessFileName = "qtaccess";
static char*                    sAccessFileName = NULL;
static Bool16                   sAllowGuestDefaultEnabled = true;
static Bool16                   sDefaultGuestEnabled = true;


// FUNCTION PROTOTYPES

static QTSS_Error QTSSDSAuthModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error Register();
static QTSS_Error Initialize(QTSS_Initialize_Params* inParams);
static QTSS_Error Shutdown();
static QTSS_Error RereadPrefs();
static QTSS_Error AuthenticateRTSPRequest(QTSS_RTSPAuth_Params* inParams);
static QTSS_Error Authorize(QTSS_StandardRTSP_Params* inParams);
static Bool16 AuthenticateRequest(QTSS_StandardRTSP_Params* inParams, const char* pathBuff, const char* movieRootDir, StrPtrLen* ioRealmName, Bool16* foundUserPtr);


static int    check_sacl(const char *inUser);
#define kSACLNotAuthorized  0
#define kSACLAuthorized     1
#define kSACLUnknownUser    2
#define kSACLAnyUser        3

// FUNCTION IMPLEMENTATIONS


QTSS_Error QTSSDSAuthModule_Main(void* inPrivateArgs)
{
printf("QTSSDSAuthModule_Main\n");
#if OSX_SACL
	printf("QTSSDSAuthModule_Main OSX_SACL\n");
#endif
#if OSX_OD_API
	printf("QTSSDSAuthModule_Main OSX_OD_API\n");
#endif

     return _stublibrary_main(inPrivateArgs, QTSSDSAuthModuleDispatch);
}


QTSS_Error  QTSSDSAuthModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
   switch (inRole)
    {  
       case QTSS_Register_Role:
            return Register();
        case QTSS_Initialize_Role:
            return Initialize(&inParams->initParams);
        case QTSS_RereadPrefs_Role:
            return RereadPrefs();
        case QTSS_RTSPAuthenticate_Role:
             if (sAuthenticationEnabled)
                 return AuthenticateRTSPRequest(&inParams->rtspAthnParams);
        case QTSS_RTSPAuthorize_Role:
             if (sAuthenticationEnabled)
                 return Authorize(&inParams->rtspRequestParams);
        case QTSS_Shutdown_Role:
            return Shutdown();
    }
    return QTSS_NoErr;
}


QTSS_Error Register()
{
    // Do role & attribute setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_RereadPrefs_Role);
    (void)QTSS_AddRole(QTSS_RTSPAuthenticate_Role);
    (void)QTSS_AddRole(QTSS_RTSPAuthorize_Role);
        
    return QTSS_NoErr;
}

QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{
    // Setup module utils
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);
    sPrefs = QTSSModuleUtils::GetModulePrefsObject(inParams->inModule);
    sServerPrefs = inParams->inPrefs;
    sAuthMutex = new OSMutex();

    RereadPrefs();
    return QTSS_NoErr;
}

QTSS_Error Shutdown()
{
    return QTSS_NoErr;
}

char* GetCheckedFileName()
{
    char        *result = NULL;
    static char *badChars = "/'\"";
    char        theBadCharMessage[] = "' '";
    char        *theBadChar = NULL;
    result = QTSSModuleUtils::GetStringAttribute(sPrefs, MODPREFIX_"dsaccessfilename", sDefaultAccessFileName);
    StrPtrLen searchStr(result);
    
    theBadChar = strpbrk(searchStr.Ptr, badChars);
    if ( theBadChar!= NULL) 
    {
        theBadCharMessage[1] = theBadChar[0];
        QTSSModuleUtils::LogErrorStr(qtssWarningVerbosity,MODPREFIX_"found invalid DS access file name in prefs");
                
        delete[] result;
        result = new char[::strlen(sDefaultAccessFileName) + 2];
        ::strcpy(result, sDefaultAccessFileName);   
    }
    return result;
}

QTSS_Error RereadPrefs()
{
    OSMutexLocker locker(sAuthMutex);
    QTSSModuleUtils::GetAttribute(sPrefs, MODPREFIX_"enabled",    qtssAttrDataTypeBool16,
                      &sAuthenticationEnabled, &sDefaultAuthenticationEnabled, sizeof(sAuthenticationEnabled));

   QTSSModuleUtils::GetAttribute(sServerPrefs,"enable_allow_guest_default", qtssAttrDataTypeBool16, 
                                           &sAllowGuestDefaultEnabled,(void *)&sDefaultGuestEnabled, sizeof(sAllowGuestDefaultEnabled));

    delete [] sAccessFileName;
    sAccessFileName = GetCheckedFileName();
    return QTSS_NoErr;
}


Bool16 AuthenticateRequest(QTSS_StandardRTSP_Params* inParams, 
                const char* pathBuff, 
                const char* movieRootDir,
                StrPtrLen* ioRealmName,
                Bool16* foundUserPtr)
{
    if (foundUserPtr)
        *foundUserPtr = false;
 
    if (ioRealmName) //Set Value to Empty for now use whatever is set by access file or the default
    {
        ioRealmName->Ptr[0] =  '\0';
        ioRealmName->Len = 0;        
    }
    QTSS_Error theErr = QTSS_NoErr;
    
    char passwordBuff[kBuffLen];
    StrPtrLen passwordStr(passwordBuff, kBuffLen -1);
    
    char nameBuff[kBuffLen];
    StrPtrLen nameStr(nameBuff, kBuffLen -1);

    theErr = QTSS_GetValue (inParams->inRTSPRequest,qtssRTSPReqUserName,0, (void *) nameStr.Ptr, &nameStr.Len);
    if ( (QTSS_NoErr != theErr) || (nameStr.Len >= kBuffLen) ) 
    {
        debug_printf("QTSSDSAuthModule:AuthenticateRequest() Username Error - %"_S32BITARG_"\n", theErr);
        return false;    
    }           
    theErr = QTSS_GetValue (inParams->inRTSPRequest,qtssRTSPReqUserPassword,0, (void *) passwordStr.Ptr, &passwordStr.Len);
    if ( (QTSS_NoErr != theErr) || (passwordStr.Len >= kBuffLen) )
    {
        debug_printf("QTSSDSAuthModule:AuthenticateRequest() Password Error - %"_S32BITARG_"\n", theErr);
        return false;        
    }
    nameBuff[nameStr.Len] = '\0';
    passwordBuff[passwordStr.Len] = '\0';

    //
    // Use the name and password to check access
    DSAccessChecker accessChecker;
    if ( !accessChecker.CheckPassword( nameBuff, passwordBuff) )
    {
         return false;
    }
    
    if (foundUserPtr)
        *foundUserPtr = true;
  
     
    return true;
}



QTSS_Error AuthenticateRTSPRequest(QTSS_RTSPAuth_Params* inParams)
{
    OSMutexLocker locker(sAuthMutex);

    QTSS_RTSPRequestObject  theRTSPRequest = inParams->inRTSPRequest;
    QTSS_AuthScheme authScheme = qtssAuthNone;
    
    debug_printf("QTSSDSAuthModule:AuthenticateRTSPRequest start\n");

    if  ( (NULL == inParams) || (NULL == inParams->inRTSPRequest) )
    {
        debug_printf("QTSSDSAuthModule:AuthenticateRTSPRequest inParams NULL\n");
        return QTSS_RequestFailed;
    }
    
    
    // Get the user profile object from the request object
    QTSS_UserProfileObject theUserProfile = NULL;
    UInt32 len = sizeof(QTSS_UserProfileObject);
    QTSS_Error theErr = QTSS_GetValue(theRTSPRequest, qtssRTSPReqUserProfile, 0, (void*)&theUserProfile, &len);
    Assert(len == sizeof(QTSS_UserProfileObject));
    if (theErr != QTSS_NoErr)
    {
        debug_printf("QTSSDSAuthModule:AuthenticateRTSPRequest - username error is %"_S32BITARG_"\n", theErr);
        return theErr;
    }    
    char*   nameBuff = NULL;
    theErr = QTSS_GetValueAsString(theUserProfile, qtssUserName, 0, &nameBuff);
    debug_printf("QTSSDSAuthModule:AuthenticateRTSPRequest - username is %s\n", nameBuff);
    OSCharArrayDeleter usernameBufDeleter(nameBuff);
    if (theErr != QTSS_NoErr)
    {
        debug_printf("QTSSDSAuthModule:AuthenticateRTSPRequest - theUserProfile nameBuff error is %"_S32BITARG_"\n", theErr);
    }    


    len = sizeof(authScheme);
    theErr = QTSS_GetValue(theRTSPRequest, qtssRTSPReqAuthScheme, 0, (void*)&authScheme, &len);
  
    if (theErr != QTSS_NoErr)
        return theErr;
        
	DSAccessChecker accessChecker;
	Bool16 allowed = true;
	Bool16 foundUser = true;
	Bool16 authHandled = true;

    if ( authScheme == qtssAuthDigest)
    {
        debug_printf("QTSSDSAuthModule:AuthenticateRTSPRequest - authScheme = qtssAuthDigest\n");

        char*   challengeBuff = NULL;
        (void) QTSS_GetValueAsString(theRTSPRequest, qtssRTSPReqDigestChallenge, 0, &challengeBuff);
        OSCharArrayDeleter challengeDeleter(challengeBuff);
        debug_printf("QTSSDSAuthModule:AuthenticateRTSPRequest - Server Challenge =%s\n",challengeBuff);
    
        char*   responseBuff = NULL;
        (void) QTSS_GetValueAsString(theRTSPRequest, qtssRTSPReqDigestResponse, 0, &responseBuff);
        OSCharArrayDeleter responseDeleter(responseBuff);
        
        char*   methodBuff = NULL;
        (void) QTSS_GetValueAsString(theRTSPRequest, qtssRTSPReqMethodStr, 0, &methodBuff);
        OSCharArrayDeleter methodDeleter(methodBuff);
        debug_printf("QTSSDSAuthModule:AuthenticateRTSPRequest - Server Method =%s\n",methodBuff);

        debug_printf("QTSSDSAuthModule:AuthenticateRTSPRequest - username is %s challenge=%s response=%s method=%s\n", nameBuff, challengeBuff, responseBuff, methodBuff);
        if ( false == accessChecker.CheckDigest(nameBuff, challengeBuff, responseBuff, methodBuff) )
        {    debug_printf("QTSSDSAuthModule CheckDigest returned false\n");
        }
        else
        {   debug_printf("QTSSDSAuthModule CheckDigest returned true\n");
            (void) QTSSModuleUtils::AuthorizeRequest(theRTSPRequest,&allowed,&foundUser,&authHandled);
        }
    
    }
    if ( authScheme == qtssAuthBasic)
    {
        debug_printf("QTSSDSAuthModule:AuthenticateRTSPRequest - authScheme = qtssAuthBasic\n");

        
        char passwordBuff[kBuffLen];
        StrPtrLen passwordStr(passwordBuff, kBuffLen -1);
        
        char nameBuff[kBuffLen];
        StrPtrLen nameStr(nameBuff, kBuffLen -1);
    
        theErr = QTSS_GetValue (inParams->inRTSPRequest,qtssRTSPReqUserName,0, (void *) nameStr.Ptr, &nameStr.Len);
        if ( (QTSS_NoErr != theErr) || (nameStr.Len >= kBuffLen) ) 
        {
            debug_printf("QTSSDSAuthModule:AuthenticateRequest() Username Error - %"_S32BITARG_"\n", theErr);
            return false;    
        }           
        theErr = QTSS_GetValue (inParams->inRTSPRequest,qtssRTSPReqUserPassword,0, (void *) passwordStr.Ptr, &passwordStr.Len);
        if ( (QTSS_NoErr != theErr) || (passwordStr.Len >= kBuffLen) )
        {
            debug_printf("QTSSDSAuthModule:AuthenticateRequest() Password Error - %"_S32BITARG_"\n", theErr);
        }
        nameBuff[nameStr.Len] = '\0';
        passwordBuff[passwordStr.Len] = '\0';
        debug_printf("QTSSDSAuthModule:AuthenticateRTSPRequest - username is %s\n", nameBuff);
        debug_printf("QTSSDSAuthModule:AuthenticateRTSPRequest - password is %s\n", passwordBuff);
        if ( !accessChecker.CheckPassword(nameBuff, passwordBuff) )
        {   debug_printf("QTSSDSAuthModule CheckPassword returned false\n");
        }
        else
        {
            debug_printf("QTSSDSAuthModule CheckPassword returned true\n");
            (void) QTSSModuleUtils::AuthorizeRequest(theRTSPRequest,&allowed,&foundUser,&authHandled);
        }
   
    }
        
    return QTSS_NoErr;
}



int    check_sacl(const char *inUser)
{

#if OSX_SACL
    int    mbrErr = ENOENT;
    int    isMember = 0;
   	uuid_t user_uuid;

    uuid_t uu;
    mbrErr = mbr_uid_to_uuid(geteuid(), uu);
    if (0 == mbrErr)
    {    
        mbrErr = mbr_check_service_membership(uu, "qtss", &isMember);
        if (ENOENT == mbrErr) //no acl exists so allow any user.
            return kSACLAnyUser;
    }
    
    if( (mbrErr = mbr_user_name_to_uuid(inUser, user_uuid)) != 0)
    {
            return kSACLUnknownUser;
    } 
    
    if((mbrErr = mbr_check_service_membership(user_uuid, "qtss", &isMember)) != 0)
    {
        if(mbrErr == ENOENT){    // no ACL exists
            return kSACLAuthorized;    
        } else {
            return kSACLNotAuthorized;
        }
    }

    if(isMember == kSACLAuthorized)
    {
        return kSACLAuthorized;
    } 
    
    
    return kSACLNotAuthorized;
#else
    return kSACLAuthorized;
#endif
}



QTSS_Error Authorize(QTSS_StandardRTSP_Params* inParams)
{

    OSMutexLocker locker(sAuthMutex);


    QTSS_RTSPRequestObject  theRTSPRequest = inParams->inRTSPRequest;
 
    if  ( (NULL == inParams) || (NULL == inParams->inRTSPRequest) )
    {
        debug_printf("QTSSDSAuthModule - Authorize inParams: Error");
        return QTSS_RequestFailed;
    }
        
    //get the local file path
    char*   pathBuffStr = NULL;
    QTSS_Error theErr = QTSS_GetValueAsString(theRTSPRequest, qtssRTSPReqLocalPath, 0, &pathBuffStr);
    QTSSCharArrayDeleter pathBuffDeleter(pathBuffStr);
    if (theErr != QTSS_NoErr)
    {
        debug_printf("QTSSDSAuthModule - Authorize [QTSS_GetValueAsString]: Error %"_S32BITARG_"", theErr);
        return QTSS_RequestFailed;  
    }
    //get the root movie directory
    char*   movieRootDirStr = NULL;
    theErr = QTSS_GetValueAsString(theRTSPRequest,qtssRTSPReqRootDir, 0, &movieRootDirStr);
    OSCharArrayDeleter movieRootDeleter(movieRootDirStr);
    if (theErr != QTSS_NoErr)
    {
        debug_printf("QTSSDSAuthModule - Authorize[QTSS_GetValueAsString]: Error %"_S32BITARG_"", theErr);
        return false;
    }
    //check if this user is allowed to see this movie
    
    DSAccessFile accessFile;
    Bool16 allowNoAccessFiles = sAllowGuestDefaultEnabled; //no access files allowed means allowing guest access (unknown users)
    Bool16 allowAnyUser = false;
    QTSS_ActionFlags noAction = ~qtssActionFlagsRead; //only handle read
    QTSS_ActionFlags authorizeAction =  QTSSModuleUtils::GetRequestActions(theRTSPRequest);
    Bool16 authorized =false;
    Bool16 saclUser = false;
 
    char *name = NULL;
    (void) QTSS_GetValueAsString (theRTSPRequest,qtssRTSPReqUserName,0, &name);
    OSCharArrayDeleter nameDeleter(name);
    if (sAllowGuestDefaultEnabled) // if guest access is on, sacls are ignored.
    {
        authorized =  true;
    }
    else
    {   int result = check_sacl(name);
        
        switch (result)
        {
            case kSACLAuthorized: authorized = true;	
            break;
            
            case kSACLUnknownUser: authorized = false;	//set this to true to allow file based and other non-directory service users access, when SACLs are enabled in the system for QTSS.
            break;
            
            case kSACLNotAuthorized: authorized = false;	
            break;
            
            case kSACLAnyUser: authorized = true;
            break;
        
            default: authorized = false;	
       }

          
         debug_printf("QTSSDSAuthModule:Authorize sacl_check result=%d for %s authorized = %d\n",result,  name, authorized);
         if (false == authorized)
            saclUser = true;
    }

    Bool16 foundUser = false;
    Bool16 passwordOK = false; //::AuthenticateRequest(inParams, pathBuffStr, movieRootDirStr, &sRealmNameStr, &foundUser);
    if (authorized) //have to be authorized by sacls or guest first before qtaccess file checks can allow or disallow.
    {
       theErr = accessFile.AuthorizeRequest(inParams,allowNoAccessFiles, noAction, authorizeAction,&authorized,  &allowAnyUser);
       debug_printf("QTSSDSAuthModule:Authorize AuthorizeRequest() returned authorized=%d allowAnyUser=%d\n", authorized, allowAnyUser);

    }
    
    debug_printf("QTSSDSAuthModule:Authorize AuthenticateRequest() returned passwordOK=%d foundUser=%d authorized=%d allowAnyUser=%d\n", passwordOK ,foundUser, authorized,allowAnyUser);

    Bool16 allowRequest = authorized;
    Bool16 authHandled = true;

    if(!(authorizeAction & qtssActionFlagsRead)) //not for us
    {
        debug_printf("QTSSDSAuthModule:Authorize(qtssActionFlagsRead) not handled do nothing.\n");
    }
    else if (allowRequest)
    {
        debug_printf("QTSSDSAuthModule:Authorize() succeeded.\n");
        theErr = QTSSModuleUtils::AuthorizeRequest(theRTSPRequest, &allowRequest, &foundUser, &authHandled);
        debug_printf("QTSSDSAuthModule:Authorize allowRequest=%d founduser=%d authHandled=%d\n", allowRequest, foundUser, authHandled);
    }
    else //request denied
    {
         debug_printf("QTSSDSAuthModule:Authorize() failed.\n");
         foundUser = saclUser;
         authHandled = true;
         theErr = QTSSModuleUtils::AuthorizeRequest(theRTSPRequest, &allowRequest, &foundUser, &authHandled);
         debug_printf("QTSSDSAuthModule:Authorize allowRequest=%d founduser=%d authHandled=%d saclUser=%d\n", allowRequest, foundUser, authHandled,saclUser);
    }


  return theErr;
}
