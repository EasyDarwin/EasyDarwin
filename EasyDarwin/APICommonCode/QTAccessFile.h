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
    File:       QTAccessFile.h

    Contains:   This object contains an interface for finding and parsing qtaccess files.
                

*/
#ifndef _QT_ACCESS_FILE_H_
#define _QT_ACCESS_FILE_H_

#include <stdlib.h>
#include "SafeStdLib.h"
#include "QTSS.h"
#include "StrPtrLen.h"
#include "OSHeaders.h"
#include "StringParser.h"
#include "OSMutex.h"

class QTAccessFile
{
    public:
        static UInt8 sWhitespaceAndGreaterThanMask[];
        static void Initialize();
        
        static char * GetUserNameCopy(QTSS_UserProfileObject inUserProfile);

        //GetGroupsArrayCopy 
        //
        // GetGroupsArrayCopy allocates outGroupCharPtrArray. Caller must "delete [] outGroupCharPtrArray" when done.
        static char*  GetAccessFile_Copy( const char* movieRootDir, const char* dirPath);

        //over ride these in a sub class
        virtual bool HaveUser(char *userName, void* extraDataPtr);
        virtual bool HaveGroups( char** groupArray, UInt32 numGroups, void* extraDataPtr);
        virtual bool HaveRealm(   char *userName, StrPtrLen* ioRealmNameStr, void *extraData );
        virtual bool TestUser(StrPtrLen* accessUser, char *userName,void *extraDataPtr );
        virtual bool TestGroup( StrPtrLen* accessGroup, char *userName, char**groupArray, UInt32 numGroups, void *extraDataPtr);
        virtual bool TestExtraData( StrPtrLen* wordPtr, StringParser* lineParserPtr, void* extraDataPtr);
        virtual void   GetRealm(StrPtrLen* accessRealm, StrPtrLen* ioRealmNameStr, char *userName,void *extraDataPtr );
        virtual bool ValidUser(char* userName, void* extraDataPtr) { return false; };

        //AccessAllowed
        //
        // This routine is used to get the Realm to send back to a user and to check if a user has access
        // userName: may be null.
        // accessFileBufPtr:If accessFileBufPtr is NULL or contains a NULL PTR or 0 LEN then false is returned
        // ioRealmNameStr:  ioRealmNameStr and ioRealmNameStr->Ptr may be null. 
        //                  To get a returned ioRealmNameStr value the ioRealmNameStr and ioRealmNameStr->Ptr must be non-NULL
        //                  valid pointers. The ioRealmNameStr.Len should be set to the ioRealmNameStr->Ptr's allocated len.
        // numGroups:       The number of groups in the groupArray. Use GetGroupsArrayCopy to create the groupArray.
        bool AccessAllowed (   char *userName, char**groupArray, UInt32 numGroups, 
                                        StrPtrLen *accessFileBufPtr,QTSS_ActionFlags inFlags,StrPtrLen* ioRealmNameStr,
                                        bool* outAllowAnyUserPtr,
                                        void *extraDataPtr = NULL
                                    );

        static void SetAccessFileName(const char *inQTAccessFileName); //makes a copy and stores it
        static char* GetAccessFileName() { return sQTAccessFileName; }; // a reference. Don't delete!
        
        // allocates memory for outUsersFilePath and outGroupsFilePath - remember to delete
        // returns the auth scheme
        static QTSS_AuthScheme FindUsersAndGroupsFilesAndAuthScheme(char* inAccessFilePath, QTSS_ActionFlags inAction, char** outUsersFilePath, char** outGroupsFilePath);
                
        QTSS_Error AuthorizeRequest(QTSS_StandardRTSP_Params* inParams, bool allowNoAccessFiles, QTSS_ActionFlags noAction, QTSS_ActionFlags authorizeAction,  bool *outAuthorizedPtr, bool* outAllowAnyUserPtr = NULL);
        virtual ~QTAccessFile() {};
        
    private:    
        static char* sQTAccessFileName; // managed by the QTAccess module
        static bool sAllocatedName;
        static OSMutex* sAccessFileMutex;
        static char* sAccessValidUser;
        static char* sAccessAnyUser;
        

};

class DSAccessFile : public QTAccessFile
{
   public:
        virtual   ~DSAccessFile() {}
        virtual bool HaveGroups( char** groupArray, UInt32 numGroups, void* extraDataPtr) { return true; }
        virtual bool TestGroup( StrPtrLen* accessGroup, char *userName, char**groupArray, UInt32 numGroups, void *extraDataPtr)
        {   StrPtrLenDel deleter( accessGroup->GetAsCString() );
            return this->CheckGroupMembership(userName, deleter.Ptr );
        }
       virtual bool ValidUser(char* userName, void* extraDataPtr);
	   bool CheckGroupMembership(const char* inUsername, const char* inGroupName);

};


#endif //_QT_ACCESS_FILE_H_

