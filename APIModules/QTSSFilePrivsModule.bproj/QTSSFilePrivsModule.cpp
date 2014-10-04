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
    File:       QTSSFilePrivsModule.cpp

    Contains:   Implementation of QTSSFilePrivsModule. 
                    
    
    
*/

#include "QTSSFilePrivsModule.h"


#include "OSArrayObjectDeleter.h"
#include "QTSS_Private.h"
#include "StrPtrLen.h"
#include "OSMemory.h"
#include "MyAssert.h"
#include "StringFormatter.h"
#include "StrPtrLen.h"
#include "StringParser.h"
#include "QTSSModuleUtils.h"
#include "base64.h"
#include "OS.h"

#ifndef __MW_
    #include <sys/errno.h>
    #include <pwd.h>
    #include <grp.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/stat.h>
#endif



// STATIC DATA

static StrPtrLen        sDefaultRealm("WWW Streaming Server"); // testing only
static StrPtrLen        sSDPSuffix(".sdp");
static char*            sRootUserPtr                    = "root";

const UInt32 kMaxPathLen = 512; 

static OSMutex*         sUserMutex              = NULL;

// FUNCTION PROTOTYPES

QTSS_Error  QTSSFilePrivsModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
QTSS_Error  Register(QTSS_Register_Params* inParams);
QTSS_Error  Initialize(QTSS_Initialize_Params* inParams);
QTSS_Error  Shutdown();
QTSS_Error  RereadPrefs();
QTSS_Error  AuthenticateRTSPRequest(QTSS_StandardRTSP_Params* inParams);
Bool16      QTSSAuthorize(QTSS_StandardRTSP_Params* inParams, const char* pathBuff);
Bool16      CheckWorldAccess(const char* pathBuff);
Bool16      CheckPassword(QTSS_StandardRTSP_Params* inParams);
Bool16      FileExists(char* pathBuff);
UInt32      GetPathParentDestructive(const StrPtrLen *thePathPtr, StrPtrLen *resultPathPtr, UInt32 maxLen);


// FUNCTION IMPLEMENTATIONS

QTSS_Error QTSSFilePrivsModule_Main(void* inPrivateArgs)
{
    return _stublibrary_main(inPrivateArgs, QTSSFilePrivsModuleDispatch);
}


QTSS_Error  QTSSFilePrivsModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
    switch (inRole)
    {
        case QTSS_Register_Role:
            return Register(&inParams->regParams);
        break;
        
        case QTSS_Initialize_Role:
            return Initialize(&inParams->initParams);
        break;
        
        case QTSS_RereadPrefs_Role:
            return RereadPrefs();
        break;
            
        case QTSS_RTSPAuthorize_Role:
                return AuthenticateRTSPRequest(&inParams->rtspRequestParams);
        break;
            
        case QTSS_Shutdown_Role:
            return Shutdown();
        break;
    }
    
    return QTSS_NoErr;
}


QTSS_Error Register(QTSS_Register_Params* inParams)
{

    // Do role & attribute setup
    (void)QTSS_AddRole(QTSS_Initialize_Role);
    (void)QTSS_AddRole(QTSS_RereadPrefs_Role);
    (void)QTSS_AddRole(QTSS_RTSPAuthorize_Role);
        
    
    // Tell the server our name!
        static char* sModuleName = "QTSSFilePrivsModule";
    ::strcpy(inParams->outModuleName, sModuleName);

    return QTSS_NoErr;
}


QTSS_Error Initialize(QTSS_Initialize_Params* inParams)
{
    // Setup module utils
    QTSSModuleUtils::Initialize(inParams->inMessages, inParams->inServer, inParams->inErrorLogStream);
    sUserMutex = NEW OSMutex();

    RereadPrefs();

    return QTSS_NoErr;
}

QTSS_Error Shutdown()
{
    return QTSS_NoErr;
}

QTSS_Error RereadPrefs()
{
    return QTSS_NoErr;
}


UInt32 GetPathParentDestructive(const StrPtrLen *thePathPtr, StrPtrLen *resultPathPtr, UInt32 maxLen)
{
    
    if (resultPathPtr && thePathPtr)
    {
        StrPtrLen thePath = *thePathPtr;

        while ( (thePath.Len > 0) && (thePath.Ptr[thePath.Len -1] == '/') )
        {   
            thePath.Len--;
        }
        
        while ( (thePath.Len > 0) && (thePath.Ptr[thePath.Len -1] != '/') )
        {   
            thePath.Len--;
        }   
        
        if (thePath.Len < maxLen)
        {
            memcpy (resultPathPtr->Ptr,thePath.Ptr, thePath.Len);
            resultPathPtr->Len = thePath.Len;
            resultPathPtr->Ptr[thePath.Len] = 0;
            //qtss_printf("new dir =%s \n",resultPathPtr->Ptr);
        }
            
    }
    
    return resultPathPtr->Len;
}

Bool16 IsUserMember(uid_t userID, gid_t groupID)
{
    struct passwd* user = getpwuid(userID);
    struct group* group = getgrgid(groupID);
    
    if ((user == NULL) || (group == NULL))
        return false;
        
    if (user->pw_gid == groupID) return true;
    
    int i = 0;
    while (group->gr_mem[i] != NULL)
    {
        if (!strcmp(user->pw_name, group->gr_mem[i]))
            return true;
        i++;
    }
    
    return false;
}

Bool16 CheckFileAccess(struct passwd    *passwdStructPtr, StrPtrLen */*nameStrPtr*/, char* pathBuff)
{
    Bool16 result = false;

    /*
    
         if the user is the owner then
            if owner access then succeed
            else fail
        if user is in group then
            if group access then succeed
            else fail
            
        if guest is on then
            succed 
            else fail
        
    */
    
    do // once only check for the current entity
    {
        struct stat statData;
        
        int statResult =  stat(pathBuff,&statData);
            
        if (0 != statResult)
        {   
            //qtss_printf("no access to file =%s\n",pathBuff);
            result = true; // let access error be handled by server
            break;
        }   
    

        if ( statData.st_uid == (UInt16) passwdStructPtr->pw_uid) // check if owner
        {
            if  (  (statData.st_mode & 0400 ) != 0 ) // has owner read access
            {   //qtss_printf("owner access to file =%s\n",pathBuff); 
                result = true;
                break;
            }
            else
            {   //qtss_printf("no owner access to file =%s\n",pathBuff); 
                result = false;
                break;
            
            }
        }
        
        
        if (statData.st_gid == (UInt16) passwdStructPtr->pw_gid)  // check if user's default group owns
        {
            if  ( (statData.st_mode & 0040) != 0 ) // has group read access
            {   //qtss_printf("user default group has access to file =%s\n",pathBuff); 
                result = true;
                break;
            }
            else
            {   //qtss_printf("user default group has no access to file =%s\n",pathBuff); 
                result = false;
                break;
            }
        
        }

        if (IsUserMember(passwdStructPtr->pw_uid, statData.st_gid)) // check if user in group
        {
            if  ( (statData.st_mode & 0040) != 0 ) // has group read access
            {   //qtss_printf("user member group has access to file =%s\n",pathBuff); 
                result = true;
                break;
            }
            else
            {   //qtss_printf("user member group has no access to file =%s\n",pathBuff); 
                result = false;
                break;
            }
        
        }
        
        if  ( (statData.st_mode & 0004) != 0 ) // has world read access
        {       
            //qtss_printf("world has access to file =%s\n",pathBuff); 
            result = true;
            break;
            
        }
        
        //qtss_printf("world has no read access to file =%s\n",pathBuff); 
        result = false;
                        
    } while (false);
    
    
    if (!result) 
    {   
        //qtss_printf("CheckFileAccess failed for %s on file %s\n",nameStrPtr->Ptr, pathBuff);
    }
    else
    {
        //qtss_printf("success on file %s for %s\n",pathBuff, nameStrPtr->Ptr);
    }
    
    return result;
}   

Bool16 CheckDirAccess(struct passwd *passwdStructPtr, StrPtrLen */*nameStrPtr*/, char* pathBuff)
{
    Bool16 result = true;

    char searchBuffer[kMaxPathLen] = {};
    StrPtrLen searchPath(searchBuffer, kMaxPathLen -1);
    StrPtrLen thePath(pathBuff);
    
/*
    for each directory in path until fail
         if the user is the owner then
            if owner access then succeed-continue
            else fail - stop
            
        if user is in group then
            if group access then succeed-continue
            else fail - stop
            
        if guest access then
            succeed - continue
            else fail - stop
*/

    if (thePath.Len <= searchPath.Len)
    {   
        memcpy(searchBuffer,thePath.Ptr, thePath.Len);
    
        while ( (true == result) && (0 != GetPathParentDestructive(&searchPath, &searchPath, kMaxPathLen)) ) // loop until fail or have checked full directory path and file
        {
            result = false;
                        
            do // once only check for the current entity
            {
                struct stat statData;
                
                int statResult =  stat(searchBuffer,&statData);
                    
                if (0 != statResult)
                {   
                    //qtss_printf("no access to path =%s\n",searchBuffer);
                    result = true; // let the error be handled in the server
                    break; //  allow
                }   
            
        

                if ( statData.st_uid == (UInt16) passwdStructPtr->pw_uid) // check if owner
                {
                    if  (  (statData.st_mode & 0100 ) != 0 ) // has owner search access
                    {   //qtss_printf("owner search access to directory =%s\n",pathBuff); 
                        result = true;
                        break;
                    }
                    else
                    {   //qtss_printf("no owner search access to directory =%s\n",pathBuff); 
                        result = false;
                        break;
                    
                    }
                }
                
                
                if (statData.st_gid == (UInt16) passwdStructPtr->pw_gid)  // check if user's default group owns
                {
                    if  ( (statData.st_mode & 0010) != 0 ) // has group search access
                    {   //qtss_printf("user default group has search access to directory =%s\n",pathBuff); 
                        result = true;
                        break;
                    }
                    else
                    {   //qtss_printf("user default group has no search access to directory =%s\n",pathBuff); 
                        result = false;
                        break;
                    }
                
                }

                if (IsUserMember(passwdStructPtr->pw_uid, statData.st_gid)) // check if user in group
                {
                    if  ( (statData.st_mode & 0010) != 0 ) // has group search access
                    {   //qtss_printf("user member group has search access to directory =%s\n",pathBuff); 
                        result = true;
                        break;
                    }
                    else
                    {   //qtss_printf("user member group has no search access to directory =%s\n",pathBuff); 
                        result = false;
                        break;
                    }
                
                }
                
                if  ( (statData.st_mode & 0001) != 0 ) // has world search access
                {       
                    //qtss_printf("world has search access to directory =%s\n",pathBuff); 
                    result = true;
                    break;
                    
                }
                
                //qtss_printf("world has no search access to directory =%s\n",pathBuff); 
                result = false;

                                
            } while (false);
            
        } 

    }
    
    
    if (!result) 
    {   
        //qtss_printf("CheckDirAccess failed for %s on file %s\n",nameStrPtr->Ptr, searchBuffer);
    }
    else
    {
        //qtss_printf("success on file %s for %s\n",searchBuffer, nameStrPtr->Ptr);
    }
    
    return result;
}   


Bool16 FileExists(char* pathBuff)
{
    struct stat statData;
    Bool16 result = true;
    
    if (0 != stat(pathBuff,&statData) ) // something wrong
    {
        if ( OSThread::GetErrno() == ENOENT )   // doesn't exist
            result = false;
    }
    
    return result;

}



Bool16 CheckWorldFileAccess(char* pathBuff)
{

    Bool16 result = false;
    struct stat statData;
                            
    do // once only check for the current entity
    {
        //qtss_printf("stat on %s \n",pathBuff);
        if (0 != stat(pathBuff,&statData))
        {   
            //qtss_printf("no access to path =%s\n",pathBuff);
            result = true; // let the server deal with this one
            break;
        }           
            
        //qtss_printf("statData.st_mode = %x \n",statData.st_mode);  
        if (0 == (statData.st_mode & 0004) ) // world read access
        {   
            //qtss_printf("no world access to path =%s\n",pathBuff);
            break;
        }

        result = true;
            
    
    } while (false);

    
    return result;
}  

Bool16 CheckWorldDirAccess(char* pathBuff)
{

    Bool16 result = true;
    struct stat statData;
    char searchBuffer[kMaxPathLen] = {};
    StrPtrLen searchPath(searchBuffer, kMaxPathLen -1);
    StrPtrLen thePath(pathBuff);
    
    if (thePath.Len <= searchPath.Len)
    {   
        memcpy(searchBuffer,thePath.Ptr, thePath.Len);
    
        while ( (true == result) && (0 != GetPathParentDestructive(&searchPath, &searchPath, kMaxPathLen)) ) // loop until fail or have checked full directory path and file
        {
            result = false;
                        
            do // once only check for the current entity
            {
                //qtss_printf("stat on %s \n",searchBuffer);
                if (0 != stat(searchBuffer,&statData))
                {   
                    //qtss_printf("no world access to path =%s\n",searchBuffer);
                    result = true; // let the server deal with this one
                    break;
                }           
                    
                //qtss_printf("statData.st_mode = %x \n",statData.st_mode);  
                if (0 == (statData.st_mode & 0001) )
                {   
                    //qtss_printf("no world access to path =%s\n",searchBuffer);
                    result = false;
                    break;
                }
        
                result = true;
                    
            
            } while (false);
        } ;
    }

    return result;
}  

   
Bool16 QTSSAuthorize(QTSS_StandardRTSP_Params* inParams, char* pathBuff)
{
    QTSS_Error theErr = QTSS_NoErr;
    Bool16 result = false;
    
    const int kBuffLen = 256;
    char passwordBuff[kBuffLen] = {};
    char nameBuff[kBuffLen] = {};
    StrPtrLen nameStr(nameBuff, kBuffLen -1); 
    StrPtrLen passwordStr(passwordBuff, kBuffLen -1); 
    Bool16  noUserName = false;
    Bool16  noPassword = false;
    Bool16  isSpecialGuest = false;
    
    do
    {

        theErr = QTSS_GetValue (inParams->inRTSPRequest,qtssRTSPReqUserName,0, (void *) nameStr.Ptr, &nameStr.Len);
        //qtss_printf("GetValue qtssRTSPReqUserName err =%"_S32BITARG_" \n",theErr);
        
        if ( (theErr != QTSS_NoErr) || (nameStr.Len == 0) || (nameStr.Ptr == NULL) || (*nameStr.Ptr == '\0'))
        {
            //qtss_printf ("no user name\n");
            noUserName = true;
        }
            
        //qtss_printf("RTSPRequest dictionary name =%s  len = %"_S32BITARG_"\n",nameStr.Ptr, nameStr.Len);

        theErr = QTSS_GetValue (inParams->inRTSPRequest,qtssRTSPReqUserPassword,0, (void *) passwordStr.Ptr, &passwordStr.Len);
        //qtss_printf("GetValue qtssRTSPReqUserName err =%"_S32BITARG_" \n",theErr);
        if ( (theErr != QTSS_NoErr) || (passwordStr.Len == 0) || (passwordStr.Ptr == NULL) || (*passwordStr.Ptr == '\0'))
        {
            //qtss_printf ("no Password\n");
            noPassword = true;
        }
        //qtss_printf("RTSPRequest dictionary password =%s len = %"_S32BITARG_" \n",passwordStr.Ptr, passwordStr.Len);

        if (noUserName && noPassword) isSpecialGuest = true;
        
        if (isSpecialGuest) // no name and no password means guest
        {
            //qtss_printf ("User is guest check for access\n");
            
            result = CheckWorldDirAccess(pathBuff);
            if (true == result)
                result = CheckWorldFileAccess(pathBuff);
            
            break; // no further processing on guest
        }
        
        if (0 == strcasecmp(nameStr.Ptr, sRootUserPtr) )
        {   //qtss_printf("user is root no root access to file =%s\n",pathBuff); // must log
            result = false; // don't allow
            break;
        }

        struct passwd   *passwdStructPtr = getpwnam(nameStr.Ptr);
        if (NULL == passwdStructPtr) 
        {   
            //qtss_printf("failed to find name =%s\n",passwordStr.Ptr);
            break;
        }
        
        char *theCryptedPassword = crypt(passwordStr.Ptr, passwdStructPtr->pw_passwd);
        if ( 0 != strcmp(passwdStructPtr->pw_passwd, theCryptedPassword ) )
        {   
            //qtss_printf("failed to match name to password =%s\n",passwordStr.Ptr);
            break;
        }
        
        result = CheckDirAccess(passwdStructPtr, &nameStr, pathBuff);
        if (!result) break;
        
        result = CheckFileAccess(passwdStructPtr, &nameStr, pathBuff);
        if (!result) break;

        //qtss_printf("QTSSAuthorize: user %s is authorized for %s\n",nameStr.Ptr,pathBuff);
        
        
    } while (false);
    
    if (!result) 
    {   //qtss_printf("QTSSAuthorize: user %s is un authorized for %s\n",nameStr.Ptr,pathBuff);
    }
    
    return result;
}

Bool16 FileOrSDPFileExists(char *pathBuff, UInt32 *pathLen, const UInt32 maxLen, QTSS_Error *theErrPtr)
{
    Bool16 result = false;
    
    do
    {
        if (FileExists(pathBuff)) 
        {   
            //qtss_printf("file exists =%s\n",pathBuff);
            result = true;
            break; // file is there
        }
        
        if ( (*pathLen + sSDPSuffix.Len + 1) >= maxLen)
        {   
            //qtss_printf("buffer too small for path\n");
            if (theErrPtr) *theErrPtr = ENAMETOOLONG; // don't allow request to succed we can't authorize
            break;
        }
        
        strcat(pathBuff, sSDPSuffix.Ptr);
        //qtss_printf("sdp path =%s\n",pathBuff);
        
        if (FileExists(pathBuff)) 
        {
            //qtss_printf("sdp file exists =%s\n",pathBuff);
            result = true; 
            break; // sdp file is there     
        }
        
    } while (false);
    
    return result;
    
}



QTSS_Error AuthenticateRTSPRequest(QTSS_StandardRTSP_Params* inParams)
{
    QTSS_Error theErr = QTSS_NoErr;
    OSMutexLocker locker(sUserMutex);
    
/*

    Notes:
            pathBuff is cleared so Getting the path should leave a zero terminated string.
            
            Order is important.
            1) Get the path 
            2) See if the path exists or an sdp path based on the path exists
            3) if world access on the path is available, allow the request
            4) if non-world access on the path or file check the name and password on the path and file

*/

    do 
    {           
        if  ( (NULL == inParams) || (NULL == inParams->inRTSPRequest) )
        {
            theErr = QTSS_RequestFailed;
            break;
        }
        
        UInt32 pathLen = kMaxPathLen -1;
        char pathBuff[kMaxPathLen] = {};    
        QTSS_RTSPRequestObject  theRTSPRequest = inParams->inRTSPRequest;
        
        theErr = QTSS_GetValue (theRTSPRequest,qtssRTSPReqLocalPath,0, (void *) pathBuff, &pathLen);
        if ( (theErr != QTSS_NoErr) || (0 == pathLen) )
        {   
            //qtss_printf("path not found in request\n");
            break; // Bail on the request. The Server will handle the error
        }

        Bool16 fileOk = FileOrSDPFileExists(pathBuff, &pathLen, kMaxPathLen, &theErr);
        if (!fileOk) 
        {   
            //qtss_printf("file not found\n");
            break; // Do nothing. The Server will handle the error
        }


        Bool16 allowRequest = QTSSAuthorize(inParams, pathBuff);
        if (allowRequest) 
        {   
            //qtss_printf("user is authorized\n");
            break; // Do nothing. Allow the request (the default behavior)
        }
            
        // We are not allowing the request so pass false back to the server.
        theErr = QTSS_SetValue(theRTSPRequest,qtssRTSPReqUserAllowed, 0, &allowRequest, sizeof(allowRequest));
        if (theErr != QTSS_NoErr) break;
        
        #if 0 // test setting a specific realm this is not used in this module
            theErr = QTSS_SetValue(theRTSPRequest,qtssRTSPReqURLRealm, 0, sDefaultRealm.Ptr, sDefaultRealm.Len);
            if (theErr != QTSS_NoErr) break;
        #endif
    
    } while (false); 
    
    if (theErr)
    {
        Assert(0);
        theErr = QTSS_RequestFailed;
    }
    

    return theErr;
}





