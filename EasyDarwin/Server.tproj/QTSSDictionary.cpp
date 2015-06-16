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
    File:       QTSSDictionary.cpp

    Contains:   Implementation of object defined in QTSSDictionary.h
                    

*/


#include "QTSSDictionary.h"

#include <string.h>
#include <stdio.h>

#include "MyAssert.h"
#include "OSMemory.h"
#include "QTSSDataConverter.h"

#include <errno.h>



QTSSDictionary::QTSSDictionary(QTSSDictionaryMap* inMap, OSMutex* inMutex) 
:   fAttributes(NULL), fInstanceAttrs(NULL), fInstanceArraySize(0),
    fMap(inMap), fInstanceMap(NULL), fMutexP(inMutex), fMyMutex(false), fLocked(false)
{
    if (fMap != NULL)
        fAttributes = NEW DictValueElement[inMap->GetNumAttrs()];
	if (fMutexP == NULL)
	{
		fMyMutex = true;
		fMutexP = NEW OSMutex();
	}
}

QTSSDictionary::~QTSSDictionary()
{
    if (fMap != NULL)
        this->DeleteAttributeData(fAttributes, fMap->GetNumAttrs(), fMap);
    if (fAttributes != NULL)
        delete [] fAttributes;
    this->DeleteAttributeData(fInstanceAttrs, fInstanceArraySize, fInstanceMap);
    delete [] fInstanceAttrs;
    delete fInstanceMap;
	if (fMyMutex)
		delete fMutexP;
}

QTSSDictionary* QTSSDictionary::CreateNewDictionary(QTSSDictionaryMap* inMap, OSMutex* inMutex)
{
    return NEW QTSSDictionary(inMap, inMutex);
}

QTSS_Error QTSSDictionary::GetValuePtr(QTSS_AttributeID inAttrID, UInt32 inIndex,
                                            void** outValueBuffer, UInt32* outValueLen,
                                            Bool16 isInternal)
{
    // Check first to see if this is a static attribute or an instance attribute
    QTSSDictionaryMap* theMap = fMap;
    DictValueElement* theAttrs = fAttributes;
    if (QTSSDictionaryMap::IsInstanceAttrID(inAttrID))
    {
        theMap = fInstanceMap;
        theAttrs = fInstanceAttrs;
    }

    if (theMap == NULL)
        return QTSS_AttrDoesntExist;
    
    SInt32 theMapIndex = theMap->ConvertAttrIDToArrayIndex(inAttrID);

    if (theMapIndex < 0)
        return QTSS_AttrDoesntExist;
    if (theMap->IsRemoved(theMapIndex))
        return QTSS_AttrDoesntExist;
    if ((!isInternal) && (!theMap->IsPreemptiveSafe(theMapIndex)) && !this->IsLocked())
        return QTSS_NotPreemptiveSafe;
    // An iterated attribute cannot have a param retrieval function
    if ((inIndex > 0) && (theMap->GetAttrFunction(theMapIndex) != NULL))
        return QTSS_BadIndex;
    // Check to make sure the index parameter is legal
    if ((inIndex > 0) && (inIndex >= theAttrs[theMapIndex].fNumAttributes))
        return QTSS_BadIndex;
        
        
    // Retrieve the parameter
    char* theBuffer = theAttrs[theMapIndex].fAttributeData.Ptr;
    *outValueLen = theAttrs[theMapIndex].fAttributeData.Len;
    
	Bool16 cacheable = theMap->IsCacheable(theMapIndex);
	if ( (theMap->GetAttrFunction(theMapIndex) != NULL) && ((cacheable && (*outValueLen == 0)) || !cacheable) )
    {
        // If function is cacheable: 
		// If the parameter doesn't have a value assigned yet, and there is an attribute
        // retrieval function provided, invoke that function now.
		// If function is *not* cacheable:
		// always call the function
		
        theBuffer = (char*)theMap->GetAttrFunction(theMapIndex)(this, outValueLen);

        //If the param retrieval function didn't return an explicit value for this attribute,
        //refetch the parameter out of the array, in case the function modified it.
        
        if (theBuffer == NULL)
        {
            theBuffer = theAttrs[theMapIndex].fAttributeData.Ptr;
            *outValueLen = theAttrs[theMapIndex].fAttributeData.Len;
        }
        
    }
#if DEBUG
    else
        // Make sure we aren't outside the bounds of attribute memory
        Assert(theAttrs[theMapIndex].fAllocatedLen >=
            (theAttrs[theMapIndex].fAttributeData.Len * (theAttrs[theMapIndex].fNumAttributes)));
#endif

    // Return an error if there is no data for this attribute
    if (*outValueLen == 0)
        return QTSS_ValueNotFound;
            
    theBuffer += theAttrs[theMapIndex].fAttributeData.Len * inIndex;
    *outValueBuffer = theBuffer;
        
    // strings need an extra dereference - moved it up
    if ((theMap->GetAttrType(theMapIndex) == qtssAttrDataTypeCharArray) && (theAttrs[theMapIndex].fNumAttributes > 1))
        {
            char** string = (char**)theBuffer;
            *outValueBuffer = *string;
            //*outValueLen = strlen(*string) + 1;
            *outValueLen = strlen(*string);
    }
    
    return QTSS_NoErr;
}



QTSS_Error QTSSDictionary::GetValue(QTSS_AttributeID inAttrID, UInt32 inIndex,
                                            void* ioValueBuffer, UInt32* ioValueLen)
{
    // If there is a mutex, lock it and get a pointer to the proper attribute
    OSMutexLocker locker(fMutexP);

    void* tempValueBuffer = NULL;
    UInt32 tempValueLen = 0;
    QTSS_Error theErr = this->GetValuePtr(inAttrID, inIndex, &tempValueBuffer, &tempValueLen, true);
    if (theErr != QTSS_NoErr)
        return theErr;
        
    if (theErr == QTSS_NoErr)
    {
        // If caller provided a buffer that's too small for this attribute, report that error
        if (tempValueLen > *ioValueLen)
            theErr = QTSS_NotEnoughSpace;
            
        // Only copy out the attribute if the buffer is big enough
        if ((ioValueBuffer != NULL) && (theErr == QTSS_NoErr))
            ::memcpy(ioValueBuffer, tempValueBuffer, tempValueLen);
            
        // Always set the ioValueLen to be the actual length of the attribute.
        *ioValueLen = tempValueLen;
    }

    return QTSS_NoErr;
}

QTSS_Error QTSSDictionary::GetValueAsString(QTSS_AttributeID inAttrID, UInt32 inIndex, char** outString)
{
    void* tempValueBuffer;
    UInt32 tempValueLen = 0;

    if (outString == NULL)  
        return QTSS_BadArgument;
        
    OSMutexLocker locker(fMutexP);
    QTSS_Error theErr = this->GetValuePtr(inAttrID, inIndex, &tempValueBuffer,
                                            &tempValueLen, true);
    if (theErr != QTSS_NoErr)
        return theErr;
        
    QTSSDictionaryMap* theMap = fMap;
    if (QTSSDictionaryMap::IsInstanceAttrID(inAttrID))
        theMap = fInstanceMap;

    if (theMap == NULL)
        return QTSS_AttrDoesntExist;
    
    SInt32 theMapIndex = theMap->ConvertAttrIDToArrayIndex(inAttrID);
    Assert(theMapIndex >= 0);
    
    *outString = QTSSDataConverter::ValueToString(tempValueBuffer, tempValueLen, theMap->GetAttrType(theMapIndex));
    return QTSS_NoErr;
}



QTSS_Error QTSSDictionary::CreateObjectValue(QTSS_AttributeID inAttrID, UInt32* outIndex,
                                        QTSSDictionary** newObject, QTSSDictionaryMap* inMap, UInt32 inFlags)
{
    // Check first to see if this is a static attribute or an instance attribute
    QTSSDictionaryMap* theMap = fMap;
    DictValueElement* theAttrs = fAttributes;
    if (QTSSDictionaryMap::IsInstanceAttrID(inAttrID))
    {
        theMap = fInstanceMap;
        theAttrs = fInstanceAttrs;
    }
    
    if (theMap == NULL)
        return QTSS_AttrDoesntExist;
    
    SInt32 theMapIndex = theMap->ConvertAttrIDToArrayIndex(inAttrID);
    
    // If there is a mutex, make this action atomic.
    OSMutexLocker locker(fMutexP);
    
    if (theMapIndex < 0)
        return QTSS_AttrDoesntExist;
    if ((!(inFlags & kDontObeyReadOnly)) && (!theMap->IsWriteable(theMapIndex)))
        return QTSS_ReadOnly;
    if (theMap->IsRemoved(theMapIndex))
        return QTSS_AttrDoesntExist;
    if (theMap->GetAttrType(theMapIndex) != qtssAttrDataTypeQTSS_Object)
        return QTSS_BadArgument;
        
    UInt32 numValues = theAttrs[theMapIndex].fNumAttributes;

    // if normal QTSSObjects have been added, then we can't add a dynamic one
    if (!theAttrs[theMapIndex].fIsDynamicDictionary && (numValues > 0))
        return QTSS_ReadOnly;

    QTSSDictionary* oldDict = NULL;
    *outIndex = numValues;  // add the object into the next spot

    UInt32 len = sizeof(QTSSDictionary*);
    QTSSDictionary* dict = CreateNewDictionary(inMap, fMutexP);
    
    // kind of a hack to avoid the check in SetValue
    theAttrs[theMapIndex].fIsDynamicDictionary = false;
    QTSS_Error err = SetValue(inAttrID, *outIndex, &dict, len, inFlags);
    if (err != QTSS_NoErr)
    {
        delete dict;
        return err;
    }
    
    if (oldDict != NULL)
    {
        delete oldDict;
    }
    
    theAttrs[theMapIndex].fIsDynamicDictionary = true;
    *newObject = dict;
    
    return QTSS_NoErr;
}

QTSS_Error QTSSDictionary::SetValue(QTSS_AttributeID inAttrID, UInt32 inIndex,
                                        const void* inBuffer,  UInt32 inLen,
                                        UInt32 inFlags)
{
    // Check first to see if this is a static attribute or an instance attribute
    QTSSDictionaryMap* theMap = fMap;
    DictValueElement* theAttrs = fAttributes;
    if (QTSSDictionaryMap::IsInstanceAttrID(inAttrID))
    {
        theMap = fInstanceMap;
        theAttrs = fInstanceAttrs;
    }
    
    if (theMap == NULL)
        return QTSS_AttrDoesntExist;
    
    SInt32 theMapIndex = theMap->ConvertAttrIDToArrayIndex(inAttrID);
    
    // If there is a mutex, make this action atomic.
    OSMutexLocker locker(fMutexP);
    
    if (theMapIndex < 0)
        return QTSS_AttrDoesntExist;
    if ((!(inFlags & kDontObeyReadOnly)) && (!theMap->IsWriteable(theMapIndex)))
        return QTSS_ReadOnly;
    if (theMap->IsRemoved(theMapIndex))
        return QTSS_AttrDoesntExist;
    if (theAttrs[theMapIndex].fIsDynamicDictionary)
        return QTSS_ReadOnly;
    
    UInt32 numValues = theAttrs[theMapIndex].fNumAttributes;

    QTSS_AttrDataType dataType = theMap->GetAttrType(theMapIndex);
    UInt32 attrLen = inLen;
    if (dataType == qtssAttrDataTypeCharArray)
    {
        if (inIndex > 0)
            attrLen = sizeof(char*);    // value just contains a pointer
        
        if ((numValues == 1) && (inIndex == 1))
        {
            // we're adding a second value, so we need to change the storage from directly
            // storing the string to an array of string pointers
          
                // creating new memory here just to create a null terminated string
                // instead of directly using the old storage as the old storage didn't 
                // have its string null terminated
                        UInt32 tempStringLen = theAttrs[theMapIndex].fAttributeData.Len;
                        char* temp = NEW char[tempStringLen + 1];
                        ::memcpy(temp, theAttrs[theMapIndex].fAttributeData.Ptr, tempStringLen);
                        temp[tempStringLen] = '\0';
                        delete [] theAttrs[theMapIndex].fAttributeData.Ptr;
                        
            //char* temp = theAttrs[theMapIndex].fAttributeData.Ptr;
            
            theAttrs[theMapIndex].fAllocatedLen = 16 * sizeof(char*);
            theAttrs[theMapIndex].fAttributeData.Ptr = NEW char[theAttrs[theMapIndex].fAllocatedLen];
            theAttrs[theMapIndex].fAttributeData.Len = sizeof(char*);
            // store off original string as first value in array
            *(char**)theAttrs[theMapIndex].fAttributeData.Ptr = temp;
            // question: why isn't theAttrs[theMapIndex].fAllocatedInternally set to true?
	    theAttrs[theMapIndex].fAllocatedInternally = true;
        }
    }
    else
    {
        // If this attribute is iterated, this new value
        // must be the same size as all the others.
        if (((inIndex > 0) || (numValues > 1))
                 &&(theAttrs[theMapIndex].fAttributeData.Len != 0) && (inLen != theAttrs[theMapIndex].fAttributeData.Len))
            return QTSS_BadArgument;
    }
    
    //
    // Can't put empty space into the array of values
    if (inIndex > numValues)
        return QTSS_BadIndex;
        
    if ((attrLen * (inIndex + 1)) > theAttrs[theMapIndex].fAllocatedLen)
    {
        // We need to reallocate this buffer.
        UInt32 theLen;
        
        if (inIndex == 0)
            theLen = attrLen;   // most attributes are single valued, so allocate just enough space
        else
            theLen = 2 * (attrLen * (inIndex + 1));// Allocate twice as much as we need
        char* theNewBuffer = NEW char[theLen];
        if (inIndex > 0)
        {
            // Copy out the old attribute data
            ::memcpy(theNewBuffer, theAttrs[theMapIndex].fAttributeData.Ptr,
                        theAttrs[theMapIndex].fAllocatedLen);
        }
        
        // Now get rid of the old stuff. Delete the buffer
        // if it was already allocated internally
        if (theAttrs[theMapIndex].fAllocatedInternally)
            delete [] theAttrs[theMapIndex].fAttributeData.Ptr;
        
        // Finally, update this attribute structure with all the new values.
        theAttrs[theMapIndex].fAttributeData.Ptr = theNewBuffer;
        theAttrs[theMapIndex].fAllocatedLen = theLen;
        theAttrs[theMapIndex].fAllocatedInternally = true;
    }
        
    // At this point, we should always have enough space to write what we want
    Assert(theAttrs[theMapIndex].fAllocatedLen >= (attrLen * (inIndex + 1)));
    
    // Copy the new data to the right place in our data buffer
    void *attributeBufferPtr;
    if ((dataType != qtssAttrDataTypeCharArray) || ((numValues < 2) && (inIndex == 0)))
    {
        attributeBufferPtr = theAttrs[theMapIndex].fAttributeData.Ptr + (inLen * inIndex);
        theAttrs[theMapIndex].fAttributeData.Len = inLen;
    }
    else
    {
            //attributeBufferPtr = NEW char[inLen];
            // allocating one extra so that we can null terminate the string
            attributeBufferPtr = NEW char[inLen + 1];
                char* tempBuffer = (char*)attributeBufferPtr;
                tempBuffer[inLen] = '\0';
                
        //char** valuePtr = (char**)theAttrs[theMapIndex].fAttributeData.Ptr + (inLen * inIndex);
        // The offset should be (attrLen * inIndex) and not (inLen * inIndex) 
        char** valuePtr = (char**)(theAttrs[theMapIndex].fAttributeData.Ptr + (attrLen * inIndex));
        if (inIndex < numValues)    // we're replacing an existing string
            delete [] *valuePtr;
        *valuePtr = (char*)attributeBufferPtr;
    }
    
    ::memcpy(attributeBufferPtr, inBuffer, inLen);
    

    // Set the number of attributes to be proper
    if (inIndex >= theAttrs[theMapIndex].fNumAttributes)
    {
        //
        // We should never have to increment num attributes by more than 1
        Assert(theAttrs[theMapIndex].fNumAttributes == inIndex);
        theAttrs[theMapIndex].fNumAttributes++;
    }

    //
    // Call the completion routine
    if (((fMap == NULL) || fMap->CompleteFunctionsAllowed()) && !(inFlags & kDontCallCompletionRoutine))
        this->SetValueComplete(theMapIndex, theMap, inIndex, attributeBufferPtr, inLen);
    
    return QTSS_NoErr;
}


QTSS_Error QTSSDictionary::SetValuePtr(QTSS_AttributeID inAttrID,
                                        const void* inBuffer,  UInt32 inLen,
                                        UInt32 inFlags)
{
    // Check first to see if this is a static attribute or an instance attribute
    QTSSDictionaryMap* theMap = fMap;
    DictValueElement* theAttrs = fAttributes;
    if (QTSSDictionaryMap::IsInstanceAttrID(inAttrID))
    {
        theMap = fInstanceMap;
        theAttrs = fInstanceAttrs;
    }
    
    if (theMap == NULL)
        return QTSS_AttrDoesntExist;
    
    SInt32 theMapIndex = theMap->ConvertAttrIDToArrayIndex(inAttrID);
    
    // If there is a mutex, make this action atomic.
    OSMutexLocker locker(fMutexP);
    
    if (theMapIndex < 0)
        return QTSS_AttrDoesntExist;
    if ((!(inFlags & kDontObeyReadOnly)) && (!theMap->IsWriteable(theMapIndex)))
        return QTSS_ReadOnly;
    if (theMap->IsRemoved(theMapIndex))
        return QTSS_AttrDoesntExist;
    if (theAttrs[theMapIndex].fIsDynamicDictionary)
        return QTSS_ReadOnly;
    
    UInt32 numValues = theAttrs[theMapIndex].fNumAttributes;
    if ((numValues > 0) || (theAttrs[theMapIndex].fAttributeData.Ptr != NULL))
        return QTSS_BadArgument;    // you can only set the pointer if you haven't done set value

    theAttrs[theMapIndex].fAttributeData.Ptr = (char*) inBuffer;
    theAttrs[theMapIndex].fAttributeData.Len = inLen;
    theAttrs[theMapIndex].fAllocatedLen = inLen;
    
    // This function assumes there is only one value and that it isn't allocated internally
    theAttrs[theMapIndex].fNumAttributes = 1;
        
        return QTSS_NoErr;
}

QTSS_Error QTSSDictionary::RemoveValue(QTSS_AttributeID inAttrID, UInt32 inIndex, UInt32 inFlags)
{
    // Check first to see if this is a static attribute or an instance attribute
    QTSSDictionaryMap* theMap = fMap;
    DictValueElement* theAttrs = fAttributes;
    if (QTSSDictionaryMap::IsInstanceAttrID(inAttrID))
    {
        theMap = fInstanceMap;
        theAttrs = fInstanceAttrs;
    }
    
    if (theMap == NULL)
        return QTSS_AttrDoesntExist;
    
    SInt32 theMapIndex = theMap->ConvertAttrIDToArrayIndex(inAttrID);
    
    // If there is a mutex, make this action atomic.
    OSMutexLocker locker(fMutexP);
    
    if (theMapIndex < 0)
        return QTSS_AttrDoesntExist;
    if ((!(inFlags & kDontObeyReadOnly)) && (!theMap->IsWriteable(theMapIndex)))
        return QTSS_ReadOnly;
    if (theMap->IsRemoved(theMapIndex))
        return QTSS_AttrDoesntExist;
    if ((theMap->GetAttrFunction(theMapIndex) != NULL) && (inIndex > 0))
        return QTSS_BadIndex;
        
    UInt32 numValues = theAttrs[theMapIndex].fNumAttributes;

    UInt32 theValueLen = theAttrs[theMapIndex].fAttributeData.Len;

    if (theAttrs[theMapIndex].fIsDynamicDictionary)
    {
        // this is an internally allocated dictionary, so we need to desctruct it
        Assert(theMap->GetAttrType(theMapIndex) == qtssAttrDataTypeQTSS_Object);
        Assert(theValueLen == sizeof(QTSSDictionary*));
        QTSSDictionary* dict = *(QTSSDictionary**)(theAttrs[theMapIndex].fAttributeData.Ptr + (theValueLen * inIndex));
        delete dict;
    }
    
    QTSS_AttrDataType dataType = theMap->GetAttrType(theMapIndex);
    if ((dataType == qtssAttrDataTypeCharArray) && (numValues > 1))
    {
        // we need to delete the string
        char* str = *(char**)(theAttrs[theMapIndex].fAttributeData.Ptr + (theValueLen * inIndex));
        delete [] str;
    }

    //
    // If there are values after this one in the array, move them.
    if (inIndex + 1 < theAttrs[theMapIndex].fNumAttributes) 
    {	
        ::memmove(  theAttrs[theMapIndex].fAttributeData.Ptr + (theValueLen * inIndex),
                theAttrs[theMapIndex].fAttributeData.Ptr + (theValueLen * (inIndex + 1)),
                theValueLen * ( (theAttrs[theMapIndex].fNumAttributes) - inIndex - 1));
    } // else this is the last in the array so just truncate.
    //
    // Update our number of values
    theAttrs[theMapIndex].fNumAttributes--;
    if (theAttrs[theMapIndex].fNumAttributes == 0)
        theAttrs[theMapIndex].fAttributeData.Len = 0;

    if ((dataType == qtssAttrDataTypeCharArray) && (theAttrs[theMapIndex].fNumAttributes == 1))
    {
        // we only have one string left, so we don't need the extra pointer
        char* str = *(char**)(theAttrs[theMapIndex].fAttributeData.Ptr);
        delete theAttrs[theMapIndex].fAttributeData.Ptr;
        theAttrs[theMapIndex].fAttributeData.Ptr = str;
        theAttrs[theMapIndex].fAttributeData.Len = strlen(str);
        theAttrs[theMapIndex].fAllocatedLen = strlen(str);
    }

    //
    // Call the completion routine
    if (((fMap == NULL) || fMap->CompleteFunctionsAllowed()) && !(inFlags & kDontCallCompletionRoutine))
        this->RemoveValueComplete(theMapIndex, theMap, inIndex);
        
    return QTSS_NoErr;
}

UInt32  QTSSDictionary::GetNumValues(QTSS_AttributeID inAttrID)
{
    // Check first to see if this is a static attribute or an instance attribute
    QTSSDictionaryMap* theMap = fMap;
    DictValueElement* theAttrs = fAttributes;
    if (QTSSDictionaryMap::IsInstanceAttrID(inAttrID))
    {
        theMap = fInstanceMap;
        theAttrs = fInstanceAttrs;
    }

    if (theMap == NULL)
        return 0;

    SInt32 theMapIndex = theMap->ConvertAttrIDToArrayIndex(inAttrID);
    if (theMapIndex < 0)
        return 0;

    return theAttrs[theMapIndex].fNumAttributes;
}

void    QTSSDictionary::SetNumValues(QTSS_AttributeID inAttrID, UInt32 inNumValues)
{
    // Check first to see if this is a static attribute or an instance attribute
    QTSSDictionaryMap* theMap = fMap;
    DictValueElement* theAttrs = fAttributes;
    if (QTSSDictionaryMap::IsInstanceAttrID(inAttrID))
    {
        theMap = fInstanceMap;
        theAttrs = fInstanceAttrs;
    }

    if (theMap == NULL)
        return;

    SInt32 theMapIndex = theMap->ConvertAttrIDToArrayIndex(inAttrID);
    if (theMapIndex < 0)
        return;

    UInt32 numAttributes = theAttrs[theMapIndex].fNumAttributes;
    // this routine can only be ever used to reduce the number of values        
    if (inNumValues >= numAttributes || numAttributes == 0)
        return;

    QTSS_AttrDataType dataType = theMap->GetAttrType(theMapIndex);
    if (theAttrs[theMapIndex].fIsDynamicDictionary || (dataType == qtssAttrDataTypeCharArray))
    {         
        // getting rid of dictionaries or strings is tricky, so it's easier to call remove value
        for (UInt32 removeCount = numAttributes - inNumValues; removeCount > 0; removeCount--)
        {	// the delete index passed to RemoveValue is always the last in the array.
            this->RemoveValue(inAttrID, theAttrs[theMapIndex].fNumAttributes - 1, kDontObeyReadOnly);
        }
    }
    else
    {
        theAttrs[theMapIndex].fNumAttributes = inNumValues;
        if (inNumValues == 0)
            theAttrs[theMapIndex].fAttributeData.Len = 0;
    }
}

void    QTSSDictionary::SetVal( QTSS_AttributeID inAttrID,
                                    void* inValueBuffer,
                                    UInt32 inBufferLen)
{ 
    Assert(inAttrID >= 0);
    Assert(fMap);
    Assert((UInt32)inAttrID < fMap->GetNumAttrs());
    fAttributes[inAttrID].fAttributeData.Ptr = (char*)inValueBuffer;
    fAttributes[inAttrID].fAttributeData.Len = inBufferLen;
    fAttributes[inAttrID].fAllocatedLen = inBufferLen;
    
    // This function assumes there is only one value and that it isn't allocated internally
    fAttributes[inAttrID].fNumAttributes = 1;
}

void    QTSSDictionary::SetEmptyVal(QTSS_AttributeID inAttrID, void* inBuf, UInt32 inBufLen)
{
    Assert(inAttrID >= 0);
    Assert(fMap);
    Assert((UInt32)inAttrID < fMap->GetNumAttrs());
    fAttributes[inAttrID].fAttributeData.Ptr = (char*)inBuf;
    fAttributes[inAttrID].fAllocatedLen = inBufLen;

#if !ALLOW_NON_WORD_ALIGN_ACCESS
    //if (((UInt32) inBuf % 4) > 0)
    //  qtss_printf("bad align by %d\n",((UInt32) inBuf % 4) );
    Assert( ((PointerSizedInt) inBuf % 4) == 0 );
#endif
}


QTSS_Error  QTSSDictionary::AddInstanceAttribute(   const char* inAttrName,
                                                    QTSS_AttrFunctionPtr inFuncPtr,
                                                    QTSS_AttrDataType inDataType,
                                                    QTSS_AttrPermission inPermission )
{
    if ((fMap != NULL) && !fMap->InstanceAttrsAllowed())
        return QTSS_InstanceAttrsNotAllowed;
        
    OSMutexLocker locker(fMutexP);

    //
    // Check to see if this attribute exists in the static map. If it does,
    // we can't add it as an instance attribute, so return an error
    QTSSAttrInfoDict* throwAway = NULL;
    QTSS_Error theErr;
    if (fMap != NULL)
    {
        theErr = fMap->GetAttrInfoByName(inAttrName, &throwAway);
        if (theErr == QTSS_NoErr)
            return QTSS_AttrNameExists;
    }
    
    if (fInstanceMap == NULL)
    {
        UInt32 theFlags = QTSSDictionaryMap::kAllowRemoval | QTSSDictionaryMap::kIsInstanceMap;
        if ((fMap == NULL) || fMap->CompleteFunctionsAllowed())
            theFlags |= QTSSDictionaryMap::kCompleteFunctionsAllowed;
            
        fInstanceMap = new QTSSDictionaryMap( 0, theFlags );
    }
    
    //
    // Add the attribute into the Dictionary Map.
    theErr = fInstanceMap->AddAttribute(inAttrName, inFuncPtr, inDataType, inPermission);
    if (theErr != QTSS_NoErr)
        return theErr;
    
    //
    // Check to see if our DictValueElement array needs to be reallocated   
    if (fInstanceMap->GetNumAttrs() >= fInstanceArraySize)
    {
        UInt32 theNewArraySize = fInstanceArraySize * 2;
        if (theNewArraySize == 0)
            theNewArraySize = QTSSDictionaryMap::kMinArraySize;
        Assert(theNewArraySize > fInstanceMap->GetNumAttrs());
        
        DictValueElement* theNewArray = NEW DictValueElement[theNewArraySize];
        if (fInstanceAttrs != NULL)
        {
            ::memcpy(theNewArray, fInstanceAttrs, sizeof(DictValueElement) * fInstanceArraySize);

            //
            // Delete the old instance attr structs, this does not delete the actual attribute memory
            delete [] fInstanceAttrs;
        }
        fInstanceAttrs = theNewArray;
        fInstanceArraySize = theNewArraySize;
    }
    return QTSS_NoErr;
}
QTSS_Error  QTSSDictionary::RemoveInstanceAttribute(QTSS_AttributeID inAttr)
{
    OSMutexLocker locker(fMutexP);

    if (fInstanceMap != NULL)
    {   
        QTSS_Error theErr = fInstanceMap->CheckRemovePermission(inAttr);
        if (theErr != QTSS_NoErr)
            return theErr;

        this->SetNumValues(inAttr,(UInt32) 0); // make sure to set num values to 0 since it is a deleted attribute
        fInstanceMap->RemoveAttribute(inAttr);
    }
    else
        return QTSS_BadArgument;
    
    //
    // Call the completion routine
    SInt32 theMapIndex = fInstanceMap->ConvertAttrIDToArrayIndex(inAttr);
    this->RemoveInstanceAttrComplete(theMapIndex, fInstanceMap);
    
    return QTSS_NoErr;
}

QTSS_Error QTSSDictionary::GetAttrInfoByIndex(UInt32 inIndex, QTSSAttrInfoDict** outAttrInfoDict)
{
    if (outAttrInfoDict == NULL)
        return QTSS_BadArgument;
        
    OSMutexLocker locker(fMutexP);

    UInt32 numInstanceValues = 0;
    UInt32 numStaticValues = 0;
    
    if (fMap != NULL)
        numStaticValues = fMap->GetNumNonRemovedAttrs();
    
    if (fInstanceMap != NULL)
        numInstanceValues = fInstanceMap->GetNumNonRemovedAttrs();
    
    if (inIndex >= (numStaticValues + numInstanceValues))
        return QTSS_AttrDoesntExist;
    
    if ( (numStaticValues > 0)  && (inIndex < numStaticValues) )
        return fMap->GetAttrInfoByIndex(inIndex, outAttrInfoDict);

	Assert(fInstanceMap != NULL);
	return fInstanceMap->GetAttrInfoByIndex(inIndex - numStaticValues, outAttrInfoDict);

}

QTSS_Error QTSSDictionary::GetAttrInfoByID(QTSS_AttributeID inAttrID, QTSSAttrInfoDict** outAttrInfoDict)
{
    if (outAttrInfoDict == NULL)
        return QTSS_BadArgument;
        
    if (QTSSDictionaryMap::IsInstanceAttrID(inAttrID))
    {
        OSMutexLocker locker(fMutexP);

        if (fInstanceMap != NULL)
            return fInstanceMap->GetAttrInfoByID(inAttrID, outAttrInfoDict);
    }
    else
        if (fMap != NULL) return fMap->GetAttrInfoByID(inAttrID, outAttrInfoDict);
            
    return QTSS_AttrDoesntExist;
}

QTSS_Error QTSSDictionary::GetAttrInfoByName(const char* inAttrName, QTSSAttrInfoDict** outAttrInfoDict)
{
    QTSS_Error theErr = QTSS_AttrDoesntExist;
    if (outAttrInfoDict == NULL)
        return QTSS_BadArgument;
        
    // Retrieve the Dictionary Map for this object type
    if (fMap != NULL)
        theErr = fMap->GetAttrInfoByName(inAttrName, outAttrInfoDict);
    
    if (theErr == QTSS_AttrDoesntExist)
    {
        OSMutexLocker locker(fMutexP);
        if (fInstanceMap != NULL)
            theErr = fInstanceMap->GetAttrInfoByName(inAttrName, outAttrInfoDict);
    }
    return theErr;
}

void QTSSDictionary::DeleteAttributeData(DictValueElement* inDictValues,
                                         UInt32 inNumValues,
                                         QTSSDictionaryMap* theMap)
{
    for (UInt32 x = 0; x < inNumValues; x++)
    {
        if (inDictValues[x].fAllocatedInternally) {
            if ((theMap->GetAttrType(x) == qtssAttrDataTypeCharArray) &&
                (inDictValues[x].fNumAttributes > 1)) {
                UInt32 z = 0;
                for (char **y = (char **) (inDictValues[x].fAttributeData.Ptr);
                           z < inDictValues[x].fNumAttributes; z++)
                    delete [] y[z];
            }
            delete [] inDictValues[x].fAttributeData.Ptr;
	}
    }
}



QTSSAttrInfoDict::AttrInfo  QTSSAttrInfoDict::sAttributes[] =
{
    /* 0 */ { "qtssAttrName",       NULL,       qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 1 */ { "qtssAttrID",         NULL,       qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 2 */ { "qtssAttrDataType",   NULL,       qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 3 */ { "qtssAttrPermissions",NULL,       qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe }
};

QTSSAttrInfoDict::QTSSAttrInfoDict()
: QTSSDictionary(QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kAttrInfoDictIndex)), fID(qtssIllegalAttrID)
{}

QTSSAttrInfoDict::~QTSSAttrInfoDict() {}



QTSSDictionaryMap*      QTSSDictionaryMap::sDictionaryMaps[kNumDictionaries + kNumDynamicDictionaryTypes];
UInt32                  QTSSDictionaryMap::sNextDynamicMap = kNumDictionaries;

void QTSSDictionaryMap::Initialize()
{
    //
    // Have to do this one first because this dict map is used by all the other
    // dict maps.
    sDictionaryMaps[kAttrInfoDictIndex]     = new QTSSDictionaryMap(qtssAttrInfoNumParams);

    // Setup the Attr Info attributes before constructing any other dictionaries
    for (UInt32 x = 0; x < qtssAttrInfoNumParams; x++)
        sDictionaryMaps[kAttrInfoDictIndex]->SetAttribute(x, QTSSAttrInfoDict::sAttributes[x].fAttrName,
                                                            QTSSAttrInfoDict::sAttributes[x].fFuncPtr,
                                                            QTSSAttrInfoDict::sAttributes[x].fAttrDataType,
                                                            QTSSAttrInfoDict::sAttributes[x].fAttrPermission);

    sDictionaryMaps[kServerDictIndex]       = new QTSSDictionaryMap(qtssSvrNumParams, QTSSDictionaryMap::kCompleteFunctionsAllowed);
    sDictionaryMaps[kPrefsDictIndex]        = new QTSSDictionaryMap(qtssPrefsNumParams, QTSSDictionaryMap::kInstanceAttrsAllowed | QTSSDictionaryMap::kCompleteFunctionsAllowed);
    sDictionaryMaps[kTextMessagesDictIndex] = new QTSSDictionaryMap(qtssMsgNumParams);
    sDictionaryMaps[kServiceDictIndex]      = new QTSSDictionaryMap(0);
    sDictionaryMaps[kRTPStreamDictIndex]    = new QTSSDictionaryMap(qtssRTPStrNumParams);
	sDictionaryMaps[kClientSessionDictIndex]= new QTSSDictionaryMap(qtssCliSesNumParams, QTSSDictionaryMap::kCompleteFunctionsAllowed);
    sDictionaryMaps[kRTSPSessionDictIndex]  = new QTSSDictionaryMap(qtssRTSPSesNumParams);
    sDictionaryMaps[kRTSPRequestDictIndex]  = new QTSSDictionaryMap(qtssRTSPReqNumParams);
    sDictionaryMaps[kRTSPHeaderDictIndex]   = new QTSSDictionaryMap(qtssNumHeaders);
    sDictionaryMaps[kFileDictIndex]         = new QTSSDictionaryMap(qtssFlObjNumParams);
    sDictionaryMaps[kModuleDictIndex]       = new QTSSDictionaryMap(qtssModNumParams);
    sDictionaryMaps[kModulePrefsDictIndex]  = new QTSSDictionaryMap(0, QTSSDictionaryMap::kInstanceAttrsAllowed | QTSSDictionaryMap::kCompleteFunctionsAllowed);
    sDictionaryMaps[kQTSSUserProfileDictIndex] = new QTSSDictionaryMap(qtssUserNumParams);
    sDictionaryMaps[kQTSSConnectedUserDictIndex] = new QTSSDictionaryMap(qtssConnectionNumParams);
    sDictionaryMaps[k3GPPRequestDictIndex] = new QTSSDictionaryMap(qtss3GPPRequestNumParams);
    sDictionaryMaps[k3GPPStreamDictIndex] = new QTSSDictionaryMap(qtss3GPPStreamNumParams);
    sDictionaryMaps[k3GPPClientSessionDictIndex] = new QTSSDictionaryMap(qtss3GPPCliSesNumParams);
    sDictionaryMaps[k3GPPRTSPSessionDictIndex] = new QTSSDictionaryMap(qtss3GPPRTSPSessNumParams);

}

QTSSDictionaryMap::QTSSDictionaryMap(UInt32 inNumReservedAttrs, UInt32 inFlags)
:   fNextAvailableID(inNumReservedAttrs), fNumValidAttrs(inNumReservedAttrs),fAttrArraySize(inNumReservedAttrs), fFlags(inFlags)
{
    if (fAttrArraySize < kMinArraySize)
        fAttrArraySize = kMinArraySize;
    fAttrArray = NEW QTSSAttrInfoDict*[fAttrArraySize];
    ::memset(fAttrArray, 0, sizeof(QTSSAttrInfoDict*) * fAttrArraySize);
}

QTSS_Error QTSSDictionaryMap::AddAttribute( const char* inAttrName,
                                            QTSS_AttrFunctionPtr inFuncPtr,
                                            QTSS_AttrDataType inDataType,
                                            QTSS_AttrPermission inPermission)
{
    if (inAttrName == NULL || ::strlen(inAttrName) > QTSS_MAX_ATTRIBUTE_NAME_SIZE)
        return QTSS_BadArgument;

    for (UInt32 count = 0; count < fNextAvailableID; count++)
    {
        if  (::strcmp(&fAttrArray[count]->fAttrInfo.fAttrName[0], inAttrName) == 0)
        {   // found the name in the dictionary
            if (fAttrArray[count]->fAttrInfo.fAttrPermission & qtssPrivateAttrModeRemoved )
            { // it is a previously removed attribute
                if (fAttrArray[count]->fAttrInfo.fAttrDataType == inDataType)
                { //same type so reuse the attribute
                    QTSS_AttributeID attrID = fAttrArray[count]->fID; 
                    this->UnRemoveAttribute(attrID); 
                    fAttrArray[count]->fAttrInfo.fFuncPtr = inFuncPtr; // reset
                    fAttrArray[count]->fAttrInfo.fAttrPermission = inPermission;// reset
                    return QTSS_NoErr; // nothing left to do. It is re-added.
                }
                
                // a removed attribute with the same name but different type--so keep checking 
                continue;
            }
            // an error, an active attribute with this name exists
            return QTSS_AttrNameExists;
        }
    }

    if (fAttrArraySize == fNextAvailableID)
    {
        // If there currently isn't an attribute array, or if the current array
        // is full, allocate a new array and copy all the old stuff over to the new array.
        
        UInt32 theNewArraySize = fAttrArraySize * 2;
        if (theNewArraySize == 0)
            theNewArraySize = kMinArraySize;
        
        QTSSAttrInfoDict** theNewArray = NEW QTSSAttrInfoDict*[theNewArraySize];
        ::memset(theNewArray, 0, sizeof(QTSSAttrInfoDict*) * theNewArraySize);
        if (fAttrArray != NULL)
        {
            ::memcpy(theNewArray, fAttrArray, sizeof(QTSSAttrInfoDict*) * fAttrArraySize);
            delete [] fAttrArray;
        }
        fAttrArray = theNewArray;
        fAttrArraySize = theNewArraySize;
    }
    
    QTSS_AttributeID theID = fNextAvailableID;
    fNextAvailableID++;
    fNumValidAttrs++;
    if (fFlags & kIsInstanceMap)
        theID |= 0x80000000; // Set the high order bit to indicate this is an instance attr

    // Copy the information into the first available element
    // Currently, all attributes added in this fashion are always writeable
    this->SetAttribute(theID, inAttrName, inFuncPtr, inDataType, inPermission); 
    return QTSS_NoErr;
}

void QTSSDictionaryMap::SetAttribute(   QTSS_AttributeID inID, 
                                        const char* inAttrName,
                                        QTSS_AttrFunctionPtr inFuncPtr,
                                        QTSS_AttrDataType inDataType,
                                        QTSS_AttrPermission inPermission )
{
    UInt32 theIndex = QTSSDictionaryMap::ConvertAttrIDToArrayIndex(inID);
    UInt32 theNameLen = ::strlen(inAttrName);
    Assert(theNameLen < QTSS_MAX_ATTRIBUTE_NAME_SIZE);
    Assert(fAttrArray[theIndex] == NULL);
    
    fAttrArray[theIndex] = NEW QTSSAttrInfoDict;
    
    //Copy the information into the first available element
    fAttrArray[theIndex]->fID = inID;
        
    ::strcpy(&fAttrArray[theIndex]->fAttrInfo.fAttrName[0], inAttrName);
    fAttrArray[theIndex]->fAttrInfo.fFuncPtr = inFuncPtr;
    fAttrArray[theIndex]->fAttrInfo.fAttrDataType = inDataType; 
    fAttrArray[theIndex]->fAttrInfo.fAttrPermission = inPermission;
    
    fAttrArray[theIndex]->SetVal(qtssAttrName, &fAttrArray[theIndex]->fAttrInfo.fAttrName[0], theNameLen);
    fAttrArray[theIndex]->SetVal(qtssAttrID, &fAttrArray[theIndex]->fID, sizeof(fAttrArray[theIndex]->fID));
    fAttrArray[theIndex]->SetVal(qtssAttrDataType, &fAttrArray[theIndex]->fAttrInfo.fAttrDataType, sizeof(fAttrArray[theIndex]->fAttrInfo.fAttrDataType));
    fAttrArray[theIndex]->SetVal(qtssAttrPermissions, &fAttrArray[theIndex]->fAttrInfo.fAttrPermission, sizeof(fAttrArray[theIndex]->fAttrInfo.fAttrPermission));
}

QTSS_Error  QTSSDictionaryMap::CheckRemovePermission(QTSS_AttributeID inAttrID)
{
    SInt32 theIndex = this->ConvertAttrIDToArrayIndex(inAttrID);
    if (theIndex < 0)
        return QTSS_AttrDoesntExist;
    
    if (0 == (fAttrArray[theIndex]->fAttrInfo.fAttrPermission & qtssAttrModeDelete))
         return QTSS_BadArgument;

    if (!(fFlags & kAllowRemoval))
        return QTSS_BadArgument;
    
    return QTSS_NoErr;
}

QTSS_Error  QTSSDictionaryMap::RemoveAttribute(QTSS_AttributeID inAttrID)
{
    SInt32 theIndex = this->ConvertAttrIDToArrayIndex(inAttrID);
    if (theIndex < 0)
        return QTSS_AttrDoesntExist;
    
    Assert(fFlags & kAllowRemoval);
    if (!(fFlags & kAllowRemoval))
        return QTSS_BadArgument;
    
    //qtss_printf("QTSSDictionaryMap::RemoveAttribute arraySize=%"_U32BITARG_" numNonRemove= %"_U32BITARG_" fAttrArray[%"_U32BITARG_"]->fAttrInfo.fAttrName=%s\n",this->GetNumAttrs(), this->GetNumNonRemovedAttrs(), theIndex,fAttrArray[theIndex]->fAttrInfo.fAttrName);
    //
    // Don't actually touch the attribute or anything. Just flag the
    // it as removed.
    fAttrArray[theIndex]->fAttrInfo.fAttrPermission |= qtssPrivateAttrModeRemoved;
    fNumValidAttrs--;
    Assert(fNumValidAttrs < 1000000);
    return QTSS_NoErr;
}

QTSS_Error  QTSSDictionaryMap::UnRemoveAttribute(QTSS_AttributeID inAttrID)
{
    if (this->ConvertAttrIDToArrayIndex(inAttrID) == -1)
        return QTSS_AttrDoesntExist;
    
    SInt32 theIndex = this->ConvertAttrIDToArrayIndex(inAttrID);
    if (theIndex < 0)
        return QTSS_AttrDoesntExist;
        
    fAttrArray[theIndex]->fAttrInfo.fAttrPermission &= ~qtssPrivateAttrModeRemoved;
    
    fNumValidAttrs++;
    return QTSS_NoErr;
}

QTSS_Error  QTSSDictionaryMap::GetAttrInfoByName(const char* inAttrName, QTSSAttrInfoDict** outAttrInfoObject,
                                                    Bool16 returnRemovedAttr)
{
    if (outAttrInfoObject == NULL)
        return QTSS_BadArgument;

    for (UInt32 count = 0; count < fNextAvailableID; count++)
    {
        if (::strcmp(&fAttrArray[count]->fAttrInfo.fAttrName[0], inAttrName) == 0)
        {
            if ((fAttrArray[count]->fAttrInfo.fAttrPermission & qtssPrivateAttrModeRemoved) && (!returnRemovedAttr))
                continue;
                
            *outAttrInfoObject = fAttrArray[count];
            return QTSS_NoErr;
        }   
    }
    return QTSS_AttrDoesntExist;
}

QTSS_Error  QTSSDictionaryMap::GetAttrInfoByID(QTSS_AttributeID inID, QTSSAttrInfoDict** outAttrInfoObject)
{
    if (outAttrInfoObject == NULL)
        return QTSS_BadArgument;

    SInt32 theIndex = this->ConvertAttrIDToArrayIndex(inID);
    if (theIndex < 0)
        return QTSS_AttrDoesntExist;
    
    if (fAttrArray[theIndex]->fAttrInfo.fAttrPermission & qtssPrivateAttrModeRemoved)
        return QTSS_AttrDoesntExist;
        
    *outAttrInfoObject = fAttrArray[theIndex];
    return QTSS_NoErr;
}

QTSS_Error  QTSSDictionaryMap::GetAttrInfoByIndex(UInt32 inIndex, QTSSAttrInfoDict** outAttrInfoObject)
{
    if (outAttrInfoObject == NULL)
        return QTSS_BadArgument;
    if (inIndex >= this->GetNumNonRemovedAttrs())
        return QTSS_AttrDoesntExist;
        
    UInt32 actualIndex = inIndex;
    UInt32 max = this->GetNumAttrs();
    if (fFlags & kAllowRemoval)
    {
        // If this dictionary map allows attributes to be removed, then
        // the iteration index and array indexes won't line up exactly, so
        // we have to iterate over the whole map all the time
        actualIndex = 0;
        for (UInt32 x = 0; x < max; x++)
        {   if (fAttrArray[x] && (fAttrArray[x]->fAttrInfo.fAttrPermission & qtssPrivateAttrModeRemoved) )
            {   continue;
            }
                
            if (actualIndex == inIndex)
            {   actualIndex = x;
                break;
            }
            actualIndex++;
        }
    }
    //qtss_printf("QTSSDictionaryMap::GetAttrInfoByIndex arraySize=%"_U32BITARG_" numNonRemove= %"_U32BITARG_" fAttrArray[%"_U32BITARG_"]->fAttrInfo.fAttrName=%s\n",this->GetNumAttrs(), this->GetNumNonRemovedAttrs(), actualIndex,fAttrArray[actualIndex]->fAttrInfo.fAttrName);
    Assert(actualIndex < fNextAvailableID);
    Assert(!(fAttrArray[actualIndex]->fAttrInfo.fAttrPermission & qtssPrivateAttrModeRemoved));
    *outAttrInfoObject = fAttrArray[actualIndex];
    return QTSS_NoErr;
}

QTSS_Error  QTSSDictionaryMap::GetAttrID(const char* inAttrName, QTSS_AttributeID* outID)
{
    if (outID == NULL)
        return QTSS_BadArgument;

    QTSSAttrInfoDict* theAttrInfo = NULL;
    QTSS_Error theErr = this->GetAttrInfoByName(inAttrName, &theAttrInfo);
    if (theErr == QTSS_NoErr)
        *outID = theAttrInfo->fID;

    return theErr;
}

UInt32  QTSSDictionaryMap::GetMapIndex(QTSS_ObjectType inType)
{
     if (inType < sNextDynamicMap)
        return inType;
     
     switch (inType)
     {
        case qtssRTPStreamObjectType:       return kRTPStreamDictIndex;
        case qtssClientSessionObjectType:   return kClientSessionDictIndex;
        case qtssRTSPSessionObjectType:     return kRTSPSessionDictIndex;
        case qtssRTSPRequestObjectType:     return kRTSPRequestDictIndex;
        case qtssRTSPHeaderObjectType:      return kRTSPHeaderDictIndex;
        case qtssServerObjectType:          return kServerDictIndex;
        case qtssPrefsObjectType:           return kPrefsDictIndex;
        case qtssTextMessagesObjectType:    return kTextMessagesDictIndex;
        case qtssFileObjectType:            return kFileDictIndex;
        case qtssModuleObjectType:          return kModuleDictIndex;
        case qtssModulePrefsObjectType:     return kModulePrefsDictIndex;
        case qtssAttrInfoObjectType:        return kAttrInfoDictIndex;
        case qtssUserProfileObjectType:     return kQTSSUserProfileDictIndex;
        case qtssConnectedUserObjectType:   return kQTSSConnectedUserDictIndex;
        
        case qtss3GPPStreamObjectType:          return k3GPPStreamDictIndex;
        case qtss3GPPClientSessionObjectType:   return k3GPPClientSessionDictIndex;
        case qtss3GPPRTSPObjectType:            return k3GPPRTSPSessionDictIndex;
        case qtss3GPPRequestObjectType:         return k3GPPRequestDictIndex;
 
       
        default:                            return kIllegalDictionary;
     }
     return kIllegalDictionary;
}

QTSS_ObjectType QTSSDictionaryMap::CreateNewMap()
{
    if (sNextDynamicMap == kNumDictionaries + kNumDynamicDictionaryTypes)
        return 0;
        
    sDictionaryMaps[sNextDynamicMap] = new QTSSDictionaryMap(0);
    QTSS_ObjectType result = (QTSS_ObjectType)sNextDynamicMap;
    sNextDynamicMap++;
    
    return result;
}
