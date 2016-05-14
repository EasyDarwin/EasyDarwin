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
	Copyleft (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       QTSSModuleUtils.cpp

    Contains:   Implements utility routines defined in QTSSModuleUtils.h.
                    
*/

#include "QTSSModuleUtils.h"
#include "QTSS_Private.h"

#include "StrPtrLen.h"
#include "OSArrayObjectDeleter.h"
#include "OSMemory.h"
#include "MyAssert.h"
#include "StringFormatter.h"
#include "ResizeableStringFormatter.h"
#include "QTAccessFile.h"
#include "StringParser.h"
#include "SafeStdLib.h"
#ifndef __Win32__
#include <netinet/in.h>
#endif

#ifdef __solaris__
#include <limits.h>
#endif

QTSS_TextMessagesObject     QTSSModuleUtils::sMessages = NULL;
QTSS_ServerObject           QTSSModuleUtils::sServer = NULL;
QTSS_StreamRef              QTSSModuleUtils::sErrorLog = NULL;
QTSS_ErrorVerbosity         QTSSModuleUtils::sMissingPrefVerbosity = qtssMessageVerbosity;

void    QTSSModuleUtils::Initialize(QTSS_TextMessagesObject inMessages,
                                    QTSS_ServerObject inServer,
                                    QTSS_StreamRef inErrorLog)
{
    sMessages = inMessages;
    sServer = inServer;
    sErrorLog = inErrorLog;
}

QTSS_Error QTSSModuleUtils::ReadEntireFile(char* inPath, StrPtrLen* outData, QTSS_TimeVal inModDate, QTSS_TimeVal* outModDate)
{   
    
    QTSS_Object theFileObject = NULL;
    QTSS_Error theErr = QTSS_NoErr;
    
    outData->Ptr = NULL;
    outData->Len = 0;
    
    do { 

        // Use the QTSS file system API to read the file
        theErr = QTSS_OpenFileObject(inPath, 0, &theFileObject);
        if (theErr != QTSS_NoErr)
            break;
    
        UInt32 theParamLen = 0;
        QTSS_TimeVal* theModDate = NULL;
        theErr = QTSS_GetValuePtr(theFileObject, qtssFlObjModDate, 0, (void**)(void*)&theModDate, &theParamLen);
        Assert(theParamLen == sizeof(QTSS_TimeVal));
        if(theParamLen != sizeof(QTSS_TimeVal))
            break;
        if(outModDate != NULL)
            *outModDate = (QTSS_TimeVal)*theModDate;

        if(inModDate != -1) {   
            // If file hasn't been modified since inModDate, don't have to read the file
            if(*theModDate <= inModDate)
                break;
        }
        
        theParamLen = 0;
        UInt64* theLength = NULL;
        theErr = QTSS_GetValuePtr(theFileObject, qtssFlObjLength, 0, (void**)(void*)&theLength, &theParamLen);
        if (theParamLen != sizeof(UInt64))
            break;
        
		if (*theLength > kSInt32_Max)
			break;

        // Allocate memory for the file data
        outData->Ptr = NEW char[ (SInt32) (*theLength + 1) ];
        outData->Len = (SInt32) *theLength;
        outData->Ptr[outData->Len] = 0;
    
        // Read the data
        UInt32 recvLen = 0;
        theErr = QTSS_Read(theFileObject, outData->Ptr, outData->Len, &recvLen);
        if (theErr != QTSS_NoErr)
        {
            outData->Delete();
            break;
        }   
        Assert(outData->Len == recvLen);
    
    }while(false);
    
    // Close the file
    if(theFileObject != NULL) {
        theErr = QTSS_CloseFileObject(theFileObject);
    }
    
    return theErr;
}

void    QTSSModuleUtils::LogError(  QTSS_ErrorVerbosity inVerbosity,
                                    QTSS_AttributeID inTextMessage,
                                    UInt32 /*inErrNumber*/,
                                    char* inArgument,
                                    char* inArg2)
{
    static char* sEmptyArg = "";
    
    if (sMessages == NULL)
        return;
        
    // Retrieve the specified text message from the text messages dictionary.
    
    StrPtrLen theMessage;
    (void)QTSS_GetValuePtr(sMessages, inTextMessage, 0, (void**)(void*)&theMessage.Ptr, &theMessage.Len);
    if ((theMessage.Ptr == NULL) || (theMessage.Len == 0))
        (void)QTSS_GetValuePtr(sMessages, qtssMsgNoMessage, 0, (void**)(void*)&theMessage.Ptr, &theMessage.Len);

    if ((theMessage.Ptr == NULL) || (theMessage.Len == 0))
        return;
    
    // qtss_sprintf and ::strlen will crash if inArgument is NULL
    if (inArgument == NULL)
        inArgument = sEmptyArg;
    if (inArg2 == NULL)
        inArg2 = sEmptyArg;
    
    // Create a new string, and put the argument into the new string.
    
    UInt32 theMessageLen = theMessage.Len + ::strlen(inArgument) + ::strlen(inArg2);

    OSCharArrayDeleter theLogString(NEW char[theMessageLen + 1]);
    qtss_sprintf(theLogString.GetObject(), theMessage.Ptr, inArgument, inArg2);
    Assert(theMessageLen >= ::strlen(theLogString.GetObject()));
    
    (void)QTSS_Write(sErrorLog, theLogString.GetObject(), ::strlen(theLogString.GetObject()),
                        NULL, inVerbosity);
}

void QTSSModuleUtils::LogErrorStr( QTSS_ErrorVerbosity inVerbosity, char* inMessage) 
{  	
	if (inMessage == NULL) return;  
	(void)QTSS_Write(sErrorLog, inMessage, ::strlen(inMessage), NULL, inVerbosity);
}


void QTSSModuleUtils::LogPrefErrorStr( QTSS_ErrorVerbosity inVerbosity, char*  preference, char* inMessage)
{  	
	if (inMessage == NULL || preference == NULL) 
	{  Assert(0);
	   return;  
	}
	char buffer[1024];
	
	qtss_snprintf(buffer,sizeof(buffer), "Server preference %s %s",  preference, inMessage);
   
	(void)QTSS_Write(sErrorLog, buffer, ::strlen(buffer), NULL, inVerbosity);
}

QTSS_ModulePrefsObject QTSSModuleUtils::GetModulePrefsObject(QTSS_ModuleObject inModObject)
{
    QTSS_ModulePrefsObject thePrefsObject = NULL;
    UInt32 theLen = sizeof(thePrefsObject);
    QTSS_Error theErr = QTSS_GetValue(inModObject, qtssModPrefs, 0, &thePrefsObject, &theLen);
    Assert(theErr == QTSS_NoErr);
    
    return thePrefsObject;
}

QTSS_Object QTSSModuleUtils::GetModuleAttributesObject(QTSS_ModuleObject inModObject)
{
    QTSS_Object theAttributesObject = NULL;
    UInt32 theLen = sizeof(theAttributesObject);
    QTSS_Error theErr = QTSS_GetValue(inModObject, qtssModAttributes, 0, &theAttributesObject, &theLen);
    Assert(theErr == QTSS_NoErr);
    
    return theAttributesObject;
}

QTSS_ModulePrefsObject QTSSModuleUtils::GetModuleObjectByName(const StrPtrLen& inModuleName)
{
    QTSS_ModuleObject theModule = NULL;
    UInt32 theLen = sizeof(theModule);
    
    for (int x = 0; QTSS_GetValue(sServer, qtssSvrModuleObjects, x, &theModule, &theLen) == QTSS_NoErr; x++)
    {
        Assert(theModule != NULL);
        Assert(theLen == sizeof(theModule));
        
        StrPtrLen theName;
        QTSS_Error theErr = QTSS_GetValuePtr(theModule, qtssModName, 0, (void**)(void*)&theName.Ptr, &theName.Len);
        Assert(theErr == QTSS_NoErr);
        
        if (inModuleName.Equal(theName))
            return theModule;
            
#if DEBUG
        theModule = NULL;
        theLen = sizeof(theModule);
#endif
    }
    return NULL;
}

void    QTSSModuleUtils::GetAttribute(QTSS_Object inObject, char* inAttributeName, QTSS_AttrDataType inType, 
                                                void* ioBuffer, void* inDefaultValue, UInt32 inBufferLen)
{
    //
    // Check to make sure this attribute is the right type. If it's not, this will coerce
    // it to be the right type. This also returns the id of the attribute
    QTSS_AttributeID theID = QTSSModuleUtils::CheckAttributeDataType(inObject, inAttributeName, inType, inDefaultValue, inBufferLen);

    //
    // Get the attribute value.
    QTSS_Error theErr = QTSS_GetValue(inObject, theID, 0, ioBuffer, &inBufferLen);
    
    //
    // Caller should KNOW how big this attribute is
    Assert(theErr != QTSS_NotEnoughSpace);
    
    if (theErr != QTSS_NoErr)
    {
        //
        // If we couldn't get the attribute value for whatever reason, just use the
        // default if it was provided.
        ::memcpy(ioBuffer, inDefaultValue, inBufferLen);

        if (inBufferLen > 0)
        {
            //
            // Log an error for this pref only if there was a default value provided.
            char* theValueAsString = NULL;
            theErr = QTSS_ValueToString(inDefaultValue, inBufferLen, inType, &theValueAsString);
            Assert(theErr == QTSS_NoErr);
            OSCharArrayDeleter theValueStr(theValueAsString);
            QTSSModuleUtils::LogError(  sMissingPrefVerbosity, 
                                        qtssServerPrefMissing,
                                        0,
                                        inAttributeName,
                                        theValueStr.GetObject());
        }
        
        //
        // Create an entry for this attribute                           
        QTSSModuleUtils::CreateAttribute(inObject, inAttributeName, inType, inDefaultValue, inBufferLen);
    }
}

char*   QTSSModuleUtils::GetStringAttribute(QTSS_Object inObject, char* inAttributeName, char* inDefaultValue)
{
    UInt32 theDefaultValLen = 0;
    if (inDefaultValue != NULL)
        theDefaultValLen = ::strlen(inDefaultValue);
    
    //
    // Check to make sure this attribute is the right type. If it's not, this will coerce
    // it to be the right type
    QTSS_AttributeID theID = QTSSModuleUtils::CheckAttributeDataType(inObject, inAttributeName, qtssAttrDataTypeCharArray, inDefaultValue, theDefaultValLen);

    char* theString = NULL;
    (void)QTSS_GetValueAsString(inObject, theID, 0, &theString);
    if (theString != NULL)
        return theString;
    
    //
    // If we get here the attribute must be missing, so create it and log
    // an error.
    
    QTSSModuleUtils::CreateAttribute(inObject, inAttributeName, qtssAttrDataTypeCharArray, inDefaultValue, theDefaultValLen);
    
    //
    // Return the default if it was provided. Only log an error if the default value was provided
    if (theDefaultValLen > 0)
    {
        QTSSModuleUtils::LogError(  sMissingPrefVerbosity,
                                    qtssServerPrefMissing,
                                    0,
                                    inAttributeName,
                                    inDefaultValue);
    }
    
    if (inDefaultValue != NULL)
    {
        //
        // Whether to return the default value or not from this function is dependent
        // solely on whether the caller passed in a non-NULL pointer or not.
        // This ensures that if the caller wants an empty-string returned as a default
        // value, it can do that.
        theString = NEW char[theDefaultValLen + 1];
        ::strcpy(theString, inDefaultValue);
        return theString;
    }
    return NULL;
}

void    QTSSModuleUtils::GetIOAttribute(QTSS_Object inObject, char* inAttributeName, QTSS_AttrDataType inType,
                            void* ioDefaultResultBuffer, UInt32 inBufferLen)
{
    char *defaultBuffPtr = NEW char[inBufferLen];
    ::memcpy(defaultBuffPtr,ioDefaultResultBuffer,inBufferLen);
    QTSSModuleUtils::GetAttribute(inObject, inAttributeName, inType, ioDefaultResultBuffer, defaultBuffPtr, inBufferLen);
    delete [] defaultBuffPtr;

}
                            

QTSS_AttributeID QTSSModuleUtils::GetAttrID(QTSS_Object inObject, char* inAttributeName)
{
    //
    // Get the attribute ID of this attribute.
    QTSS_Object theAttrInfo = NULL;
    QTSS_Error theErr = QTSS_GetAttrInfoByName(inObject, inAttributeName, &theAttrInfo);
    if (theErr != QTSS_NoErr)
        return qtssIllegalAttrID;

    QTSS_AttributeID theID = qtssIllegalAttrID; 
    UInt32 theLen = sizeof(theID);
    theErr = QTSS_GetValue(theAttrInfo, qtssAttrID, 0, &theID, &theLen);
    Assert(theErr == QTSS_NoErr);

    return theID;
}

QTSS_AttributeID QTSSModuleUtils::CheckAttributeDataType(QTSS_Object inObject, char* inAttributeName, QTSS_AttrDataType inType, void* inDefaultValue, UInt32 inBufferLen)
{
    //
    // Get the attribute type of this attribute.
    QTSS_Object theAttrInfo = NULL;
    QTSS_Error theErr = QTSS_GetAttrInfoByName(inObject, inAttributeName, &theAttrInfo);
    if (theErr != QTSS_NoErr)
        return qtssIllegalAttrID;

    QTSS_AttrDataType theAttributeType = qtssAttrDataTypeUnknown;
    UInt32 theLen = sizeof(theAttributeType);
    theErr = QTSS_GetValue(theAttrInfo, qtssAttrDataType, 0, &theAttributeType, &theLen);
    Assert(theErr == QTSS_NoErr);
    
    QTSS_AttributeID theID = qtssIllegalAttrID; 
    theLen = sizeof(theID);
    theErr = QTSS_GetValue(theAttrInfo, qtssAttrID, 0, &theID, &theLen);
    Assert(theErr == QTSS_NoErr);

    if (theAttributeType != inType)
    {
        char* theValueAsString = NULL;
        theErr = QTSS_ValueToString(inDefaultValue, inBufferLen, inType, &theValueAsString);
        Assert(theErr == QTSS_NoErr);
        OSCharArrayDeleter theValueStr(theValueAsString);
        QTSSModuleUtils::LogError(  qtssWarningVerbosity,
                                    qtssServerPrefWrongType,
                                    0,
                                    inAttributeName,
                                    theValueStr.GetObject());
                                    
        theErr = QTSS_RemoveInstanceAttribute( inObject, theID );
        Assert(theErr == QTSS_NoErr);
        return  QTSSModuleUtils::CreateAttribute(inObject, inAttributeName, inType, inDefaultValue, inBufferLen);
    }
    return theID;
}

QTSS_AttributeID QTSSModuleUtils::CreateAttribute(QTSS_Object inObject, char* inAttributeName, QTSS_AttrDataType inType, void* inDefaultValue, UInt32 inBufferLen)
{
    QTSS_Error theErr = QTSS_AddInstanceAttribute(inObject, inAttributeName, NULL, inType);
    Assert((theErr == QTSS_NoErr) || (theErr == QTSS_AttrNameExists));
    
    QTSS_AttributeID theID = QTSSModuleUtils::GetAttrID(inObject, inAttributeName);
    Assert(theID != qtssIllegalAttrID);
        
    //
    // Caller can pass in NULL for inDefaultValue, in which case we don't add the default
    if (inDefaultValue != NULL)
    {
        theErr = QTSS_SetValue(inObject, theID, 0, inDefaultValue, inBufferLen);
        Assert(theErr == QTSS_NoErr);
    }
    return theID;
}


Bool16 QTSSModuleUtils::AddressInList(QTSS_Object inObject, QTSS_AttributeID listID, StrPtrLen *inAddressPtr)
{
    StrPtrLenDel strDeleter;
    char*   theAttributeString = NULL;    
    IPComponentStr inAddress(inAddressPtr);
    IPComponentStr addressFromList;
    
    if (!inAddress.Valid())
        return false;

    UInt32 numValues = 0;
    (void) QTSS_GetNumValues(inObject, listID, &numValues);
    
    for (UInt32 index = 0; index < numValues; index ++)
    { 
        strDeleter.Delete();
        (void) QTSS_GetValueAsString(inObject, listID, index, &theAttributeString);
        strDeleter.Set(theAttributeString);
 
        addressFromList.Set(&strDeleter);
        if (addressFromList.Equal(&inAddress))
            return true;
    }

    return false;
}

Bool16 QTSSModuleUtils::FindStringInAttributeList(QTSS_Object inObject, QTSS_AttributeID listID, StrPtrLen *inStrPtr)
{
    StrPtrLenDel tempString;
     
    if (NULL == inStrPtr || NULL == inStrPtr->Ptr || 0 == inStrPtr->Len)
        return false;

    UInt32 numValues = 0;
    (void) QTSS_GetNumValues(inObject, listID, &numValues);
    
    for (UInt32 index = 0; index < numValues; index ++)
    { 
        tempString.Delete();
        (void) QTSS_GetValueAsString(inObject, listID, index, &tempString.Ptr);
        tempString.Set(tempString.Ptr);
		if (tempString.Ptr == NULL)
			return false;
			
        if (tempString.Equal(StrPtrLen("*",1)))
            return true;
        
        if (inStrPtr->FindString(tempString.Ptr))
            return true;
            
   }

    return false;
}

IPComponentStr IPComponentStr::sLocalIPCompStr("127.0.0.*");

IPComponentStr::IPComponentStr(char *theAddressPtr)
{
    StrPtrLen sourceStr(theAddressPtr);
     (void) this->Set(&sourceStr);    
}

IPComponentStr::IPComponentStr(StrPtrLen *sourceStrPtr)
{
    (void) this->Set(sourceStrPtr);    
}

Bool16 IPComponentStr::Set(StrPtrLen *theAddressStrPtr)
{
    fIsValid = false;
   
    StringParser IP_Paser(theAddressStrPtr);
    StrPtrLen *piecePtr = &fAddressComponent[0];

    while (IP_Paser.GetDataRemaining() > 0) 
    {
        IP_Paser.ConsumeUntil(piecePtr,'.');    
        if (piecePtr->Len == 0) 
            break;
        
        IP_Paser.ConsumeLength(NULL, 1);
        if (piecePtr == &fAddressComponent[IPComponentStr::kNumComponents -1])
        {
           fIsValid = true;
           break;
        }
        
        piecePtr++;
    };
     
    return fIsValid;
}


Bool16 IPComponentStr::Equal(IPComponentStr *testAddressPtr)
{
    if (testAddressPtr == NULL) 
        return false;
    
    if ( !this->Valid() || !testAddressPtr->Valid() )
        return false;

    for (UInt16 component= 0 ; component < IPComponentStr::kNumComponents ; component ++)
    {
        StrPtrLen *allowedPtr = this->GetComponent(component);
        StrPtrLen *testPtr = testAddressPtr->GetComponent(component);
                       
        if ( testPtr->Equal("*") || allowedPtr->Equal("*") )
            continue;
            
        if (!testPtr->Equal(*allowedPtr) ) 
            return false; 
    };  
    
    return true;
}


