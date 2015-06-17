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
    File:       QTSSModuleUtils.h

    Contains:   Utility routines for modules to use.
                    
*/


#ifndef _QTSS_MODULE_UTILS_H_
#define _QTSS_MODULE_UTILS_H_

#include <stdlib.h>
#include "SafeStdLib.h"
#include "QTSS.h"
#include "StrPtrLen.h"
#include "RTPMetaInfoPacket.h"

class QTSSModuleUtils
{
    public:
        // compatibiltiy features for certain players
        
        enum    {  
                    kRequiresRTPInfoSeqAndTime  = 0, 
                    kAdjustBandwidth            = 1,
                    kDisablePauseAdjustedRTPTime= 2,
                    kDelayRTPStreamsUntilAfterRTSPResponse = 3,
                    kDisable3gppRateAdaptation =4,
                    kAdjust3gppTargetTime = 5,
                    kDisableThinning =6,
                };
    
      
        static void     Initialize( QTSS_TextMessagesObject inMessages,
                                    QTSS_ServerObject inServer,
                                    QTSS_StreamRef inErrorLog);
    
        // Read the complete contents of the file at inPath into the StrPtrLen.
        // This function allocates memory for the file data.
        static QTSS_Error   ReadEntireFile(char* inPath, StrPtrLen* outData, QTSS_TimeVal inModDate = -1, QTSS_TimeVal* outModDate = NULL);

        // If your module supports RTSP methods, call this function from your QTSS_Initialize
        // role to tell the server what those methods are.
        static void     SetupSupportedMethods(  QTSS_Object inServer,
                                                QTSS_RTSPMethod* inMethodArray,
                                                UInt32 inNumMethods);
                                                
        // Using a message out of the text messages dictionary is a common
        // way to log errors to the error log. Here is a function to
        // make that process very easy.
        
        static void     LogError(   QTSS_ErrorVerbosity inVerbosity,
                                    QTSS_AttributeID inTextMessage,
                                    UInt32 inErrNumber,
                                    char* inArgument = NULL,
                                    char* inArg2 = NULL);
                                    
        static void   LogErrorStr( QTSS_ErrorVerbosity inVerbosity, char* inMessage);
        static void   LogPrefErrorStr( QTSS_ErrorVerbosity inVerbosity, char*  preference, char* inMessage);
     
        // This function constructs a C-string of the full path to the file being requested.
        // You may opt to append an optional suffix, or pass in NULL. You are responsible
        // for disposing this memory

        static char* GetFullPath(   QTSS_RTSPRequestObject inRequest,
                                    QTSS_AttributeID whichFileType,
                                    UInt32* outLen,
                                    StrPtrLen* suffix = NULL);

        //
        // This function does 2 things:
        // 1.   Compares the enabled fields in the field ID array with the fields in the
        //      x-RTP-Meta-Info header. Turns off the fields in the array that aren't in the request.
        //
        // 2.   Appends the x-RTP-Meta-Info header to the response, using the proper
        //      fields from the array, as well as the IDs provided in the array
        static QTSS_Error   AppendRTPMetaInfoHeader( QTSS_RTSPRequestObject inRequest,
                                                        StrPtrLen* inRTPMetaInfoHeader,
                                                        RTPMetaInfoPacket::FieldID* inFieldIDArray);

        // This function sends an error to the RTSP client. You must provide a
        // status code for the error, and a text message ID to describe the error.
        //
        // It always returns QTSS_RequestFailed.

        static QTSS_Error   SendErrorResponse(  QTSS_RTSPRequestObject inRequest,
                                                        QTSS_RTSPStatusCode inStatusCode,
                                                        QTSS_AttributeID inTextMessage,
                                                        StrPtrLen* inStringArg = NULL);
														
		// This function sends an error to the RTSP client. You don't have to provide
		// a text message ID, but instead you need to provide the error message in a
		// string
		// 
		// It always returns QTSS_RequestFailed
		static QTSS_Error	SendErrorResponseWithMessage( QTSS_RTSPRequestObject inRequest,
														QTSS_RTSPStatusCode inStatusCode,
														StrPtrLen* inErrorMessageStr);

        // Sends and HTTP 1.1 error message with an error message in HTML if errorMessage != NULL.
        // The session must be flagged by KillSession set to true to kill.
        // Use the QTSS_RTSPStatusCodes for the inStatusCode, for now they are the same as HTTP.
        //
		// It always returns QTSS_RequestFailed
        static QTSS_Error	SendHTTPErrorResponse( QTSS_RTSPRequestObject inRequest,
													QTSS_SessionStatusCode inStatusCode,
                                                    Bool16 inKillSession,
                                                    char *errorMessage);

        //Modules most certainly don't NEED to use this function, but it is awfully handy
        //if they want to take advantage of it. Using the SDP data provided in the iovec,
        //this function sends a standard describe response.
        //NOTE: THE FIRST ENTRY OF THE IOVEC MUST BE EMPTY!!!!
        static void SendDescribeResponse(QTSS_RTSPRequestObject inRequest,
                                                    QTSS_ClientSessionObject inSession,
                                                    iovec* describeData,
                                                    UInt32 inNumVectors,
                                                    UInt32 inTotalLength);

                
                // Called by SendDescribeResponse to coalesce iovec to a buffer
                // Allocates memory - remember to delete it!
                static char* CoalesceVectors(iovec* inVec, UInt32 inNumVectors, UInt32 inTotalLength);
                                                                                                                                                    
        //
        // SEARCH FOR A SPECIFIC MODULE OBJECT                          
        static QTSS_ModulePrefsObject GetModuleObjectByName(const StrPtrLen& inModuleName);
        
        //
        // GET MODULE PREFS OBJECT
        static QTSS_ModulePrefsObject GetModulePrefsObject(QTSS_ModuleObject inModObject);
        
        // GET MODULE ATTRIBUTES OBJECT
        static QTSS_Object GetModuleAttributesObject(QTSS_ModuleObject inModObject);
        
        //
        // GET ATTRIBUTE
        //
        // This function retrieves an attribute 
        // (from any QTSS_Object, including the QTSS_ModulePrefsObject)
        // with the specified name and type
        // out of the specified object.
        //
        // Caller should pass in a buffer for ioBuffer that is large enough
        // to hold the attribute value. inBufferLen should be set to the length
        // of this buffer.
        //
        // Pass in a buffer containing a default value to use for the attribute
        // in the inDefaultValue parameter. If the attribute isn't found, or is
        // of the wrong type, the default value will be copied into ioBuffer.
        // Also, this function adds the default value to object if it is not
        // found or is of the wrong type. If no default value is provided, the
        // attribute is still added but no value is assigned to it.
        //
        // Pass in NULL for the default value or 0 for the default value length if it is not known.
        //
        // This function logs an error if there was a default value provided.
        static void GetAttribute(QTSS_Object inObject, char* inAttributeName, QTSS_AttrDataType inType,
                            void* ioBuffer, void* inDefaultValue, UInt32 inBufferLen);
                            
        static void GetIOAttribute(QTSS_Object inObject, char* inAttributeName, QTSS_AttrDataType inType,
                            void* ioDefaultResultBuffer, UInt32 inBufferLen);
        //
        // GET STRING ATTRIBUTE
        //
        // Does the same thing as GetAttribute, but does it for string attribute. Returns a newly
        // allocated buffer with the attribute value inside it.
        //
        // Pass in NULL for the default value or an empty string if the default is not known.
        static char* GetStringAttribute(QTSS_Object inObject, char* inAttributeName, char* inDefaultValue);

        //
        // GET ATTR ID
        //
        // Given an attribute in an object, returns its attribute ID
        // or qtssIllegalAttrID if it isn't found.
        static QTSS_AttributeID GetAttrID(QTSS_Object inObject, char* inAttributeName);
        
        //
        //
        //
        /// Get the type of request. Returns qtssActionFlagsNoFlags on failure.
        //  Result is a bitmap of flags
        //
        static QTSS_ActionFlags GetRequestActions(QTSS_RTSPRequestObject theRTSPRequest);
 
        static char* GetLocalPath_Copy(QTSS_RTSPRequestObject theRTSPRequest);
        static char* GetMoviesRootDir_Copy(QTSS_RTSPRequestObject theRTSPRequest);
        static QTSS_UserProfileObject GetUserProfileObject(QTSS_RTSPRequestObject theRTSPRequest);
        static QTSS_AttrRights GetRights(QTSS_UserProfileObject theUserProfileObject);
        static char* GetExtendedRights(QTSS_UserProfileObject theUserProfileObject, UInt32 index);
       
        static char*  GetUserName_Copy(QTSS_UserProfileObject inUserProfile);
        static char** GetGroupsArray_Copy(QTSS_UserProfileObject inUserProfile, UInt32 *outNumGroupsPtr);
        static Bool16 UserInGroup(QTSS_UserProfileObject inUserProfile, char* inGroupName, UInt32 inGroupNameLen);

        static void SetEnableRTSPErrorMsg(Bool16 enable) {QTSSModuleUtils::sEnableRTSPErrorMsg = enable; }
        
        static QTSS_AttributeID CreateAttribute(QTSS_Object inObject, char* inAttributeName, QTSS_AttrDataType inType, void* inDefaultValue, UInt32 inBufferLen);
  
        static Bool16 AddressInList(QTSS_Object inObject, QTSS_AttributeID listID, StrPtrLen *theAddressPtr);
  
        static void SetMisingPrefLogVerbosity(QTSS_ErrorVerbosity verbosityLevel) { QTSSModuleUtils::sMissingPrefVerbosity = verbosityLevel;}
        static QTSS_ErrorVerbosity GetMisingPrefLogVerbosity() { return QTSSModuleUtils::sMissingPrefVerbosity;}
  
        static Bool16 FindStringInAttributeList(QTSS_Object inObject, QTSS_AttributeID listID, StrPtrLen *inStrPtr);

        static Bool16 HavePlayerProfile(QTSS_PrefsObject inPrefObjectToCheck, QTSS_StandardRTSP_Params* inParams, UInt32 feature);
        
        static QTSS_Error AuthorizeRequest(QTSS_RTSPRequestObject theRTSPRequest, Bool16* allowed, Bool16*haveUser,Bool16 *authContinue);
        
         
    private:
    
        //
        // Used in the implementation of the above functions
        static QTSS_AttributeID CheckAttributeDataType(QTSS_Object inObject, char* inAttributeName, QTSS_AttrDataType inType, void* inDefaultValue, UInt32 inBufferLen);    

        static QTSS_TextMessagesObject  sMessages;
        static QTSS_ServerObject        sServer;
        static QTSS_StreamRef           sErrorLog;
        static Bool16                   sEnableRTSPErrorMsg;
        static QTSS_ErrorVerbosity      sMissingPrefVerbosity;
};


class IPComponentStr
{
    public:
    enum { kNumComponents = 4 };
    
    StrPtrLen   fAddressComponent[kNumComponents];
    Bool16      fIsValid;
    static IPComponentStr sLocalIPCompStr;

    IPComponentStr() : fIsValid(false) {}
    IPComponentStr(char *theAddress);
    IPComponentStr(StrPtrLen *sourceStrPtr);
    
inline  StrPtrLen*  GetComponent(UInt16 which);
        Bool16      Equal(IPComponentStr *testAddressPtr);
        Bool16      Set(StrPtrLen *theAddressStrPtr);
        Bool16      Valid() { return fIsValid; }
inline  Bool16      IsLocal();

};


Bool16  IPComponentStr::IsLocal()
{
    if (this->Equal(&sLocalIPCompStr))
        return true;
    
    return false;
}

StrPtrLen* IPComponentStr::GetComponent(UInt16 which) 
{
   if (which < IPComponentStr::kNumComponents) 
        return &fAddressComponent[which]; 
   
   Assert(0);
   return NULL; 
}

#endif //_QTSS_MODULE_UTILS_H_
