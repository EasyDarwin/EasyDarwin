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
	 File:       QTSSDictionary.h

	 Contains:   Definitions of two classes: QTSSDictionary and QTSSDictionaryMap.
				 Collectively, these classes implement the "dictionary" APIs in QTSS
				 API. A QTSSDictionary corresponds to a QTSS_Object,
				 a QTSSDictionaryMap corresponds to a QTSS_ObjectType.

	 Created: Tue, Mar 2, 1999 @ 4:23 PM
 */



#ifndef _QTSSDICTIONARY_H_
#define _QTSSDICTIONARY_H_

#include <stdlib.h>
#include "SafeStdLib.h"
#include "QTSS.h"
#include "OSHeaders.h"
#include "OSMutex.h"
#include "StrPtrLen.h"
#include "MyAssert.h"
#include "QTSSStream.h"

class QTSSDictionary;
class QTSSDictionaryMap;
class QTSSAttrInfoDict;

#define __DICTIONARY_TESTING__ 0

//
// Function prototype for attr functions
typedef void* (*QTSS_AttrFunctionPtr)(QTSSDictionary*, UInt32*);

class QTSSDictionary : public QTSSStream
{
public:

	//
	// CONSTRUCTOR / DESTRUCTOR

	QTSSDictionary(QTSSDictionaryMap* inMap, OSMutex* inMutex = NULL);
	virtual ~QTSSDictionary();

	//
	// QTSS API CALLS

	// Flags used by internal callers of these routines
	enum
	{
		kNoFlags = 0,
		kDontObeyReadOnly = 1,
		kDontCallCompletionRoutine = 2
	};

	// This version of GetValue copies the element into a buffer provided by the caller
	// Returns:     QTSS_BadArgument, QTSS_NotPreemptiveSafe (if attribute is not preemptive safe),
	//              QTSS_BadIndex (if inIndex is bad)
	QTSS_Error GetValue(QTSS_AttributeID inAttrID, UInt32 inIndex, void* ioValueBuffer, UInt32* ioValueLen);


	//This version of GetValue returns a pointer to the internal buffer for the attribute.
	//Only usable if the attribute is preemptive safe.
	//
	// Returns:     Same as above, but also QTSS_NotEnoughSpace, if value is too big for buffer.
	QTSS_Error GetValuePtr(QTSS_AttributeID inAttrID, UInt32 inIndex, void** outValueBuffer, UInt32* outValueLen)
	{
		return GetValuePtr(inAttrID, inIndex, outValueBuffer, outValueLen, false);
	}

	// This version of GetValue converts the value to a string before returning it. Memory for
	// the string is allocated internally.
	//
	// Returns: QTSS_BadArgument, QTSS_BadIndex, QTSS_ValueNotFound
	QTSS_Error GetValueAsString(QTSS_AttributeID inAttrID, UInt32 inIndex, char** outString);

	// Returns:     QTSS_BadArgument, QTSS_ReadOnly (if attribute is read only),
	//              QTSS_BadIndex (attempt to set indexed parameter with param retrieval)
	QTSS_Error SetValue(QTSS_AttributeID inAttrID, UInt32 inIndex,
		const void* inBuffer, UInt32 inLen, UInt32 inFlags = kNoFlags);

	// Returns:     QTSS_BadArgument, QTSS_ReadOnly (if attribute is read only),
	QTSS_Error SetValuePtr(QTSS_AttributeID inAttrID,
		const void* inBuffer, UInt32 inLen, UInt32 inFlags = kNoFlags);

	// Returns:     QTSS_BadArgument, QTSS_ReadOnly (if attribute is read only),
	QTSS_Error CreateObjectValue(QTSS_AttributeID inAttrID, UInt32* outIndex,
		QTSSDictionary** newObject, QTSSDictionaryMap* inMap = NULL,
		UInt32 inFlags = kNoFlags);

	// Returns:     QTSS_BadArgument, QTSS_ReadOnly, QTSS_BadIndex
	QTSS_Error RemoveValue(QTSS_AttributeID inAttrID, UInt32 inIndex, UInt32 inFlags = kNoFlags);

	// Utility routine used by the two external flavors of GetValue
	QTSS_Error GetValuePtr(QTSS_AttributeID inAttrID, UInt32 inIndex,
		void** outValueBuffer, UInt32* outValueLen,
		Bool16 isInternal);

	//
	// ACCESSORS

	QTSSDictionaryMap*  GetDictionaryMap() { return fMap; }

	// Returns the Instance dictionary map for this dictionary. This may return NULL
	// if there are no instance attributes in this dictionary
	QTSSDictionaryMap*  GetInstanceDictMap() { return fInstanceMap; }

	// Returns the number of values associated with a given attribute
	UInt32              GetNumValues(QTSS_AttributeID inAttrID);
	void                SetNumValues(QTSS_AttributeID inAttrID, UInt32 inNumValues);

	// Meant only for internal server use. Does no error checking,
	// doesn't invoke the param retrieval function.
	StrPtrLen*  GetValue(QTSS_AttributeID inAttrID)
	{
		return &fAttributes[inAttrID].fAttributeData;
	}

	OSMutex*    GetMutex() { return fMutexP; }

	void		SetLocked(Bool16 inLocked) { fLocked = inLocked; }
	Bool16		IsLocked() { return fLocked; }

	//
	// GETTING ATTRIBUTE INFO
	QTSS_Error GetAttrInfoByIndex(UInt32 inIndex, QTSSAttrInfoDict** outAttrInfoDict);
	QTSS_Error GetAttrInfoByName(const char* inAttrName, QTSSAttrInfoDict** outAttrInfoDict);
	QTSS_Error GetAttrInfoByID(QTSS_AttributeID inAttrID, QTSSAttrInfoDict** outAttrInfoDict);


	//
	// INSTANCE ATTRIBUTES

	QTSS_Error  AddInstanceAttribute(const char* inAttrName,
		QTSS_AttrFunctionPtr inFuncPtr,
		QTSS_AttrDataType inDataType,
		QTSS_AttrPermission inPermission);

	QTSS_Error  RemoveInstanceAttribute(QTSS_AttributeID inAttr);
	//
	// MODIFIERS

	// These functions are meant to be used by the server when it is setting up the
	// dictionary attributes. They do no error checking.

	// They don't set fNumAttributes & fAllocatedInternally.
	void    SetVal(QTSS_AttributeID inAttrID, void* inValueBuffer, UInt32 inBufferLen);
	void    SetVal(QTSS_AttributeID inAttrID, StrPtrLen* inNewValue)
	{
		this->SetVal(inAttrID, inNewValue->Ptr, inNewValue->Len);
	}

	// Call this if you want to assign empty storage to an attribute
	void    SetEmptyVal(QTSS_AttributeID inAttrID, void* inBuf, UInt32 inBufLen);

#if __DICTIONARY_TESTING__
	static void Test(); // API test for these objects
#endif

protected:

	// Derived classes can provide a completion routine for some dictionary functions
	virtual void    RemoveValueComplete(UInt32 /*inAttrIndex*/, QTSSDictionaryMap* /*inMap*/, UInt32 /*inValueIndex*/) {}

	virtual void    SetValueComplete(UInt32 /*inAttrIndex*/, QTSSDictionaryMap* /*inMap*/,
		UInt32 /*inValueIndex*/, void* /*inNewValue*/, UInt32 /*inNewValueLen*/)
	{
	}
	virtual void    RemoveInstanceAttrComplete(UInt32 /*inAttrindex*/, QTSSDictionaryMap* /*inMap*/) {}

	virtual QTSSDictionary* CreateNewDictionary(QTSSDictionaryMap* inMap, OSMutex* inMutex);

private:

	struct DictValueElement
	{
		// This stores all necessary information for each attribute value.

		DictValueElement() : fAllocatedLen(0), fNumAttributes(0),
			fAllocatedInternally(false), fIsDynamicDictionary(false)
		{
		}

		// Does not delete! You Must call DeleteAttributeData for that
		~DictValueElement() {}

		StrPtrLen   fAttributeData; // The data
		UInt32      fAllocatedLen;  // How much space do we have allocated?
		UInt32      fNumAttributes; // If this is an iterated attribute, how many?
		Bool16      fAllocatedInternally; //Should we delete this memory?
		Bool16      fIsDynamicDictionary; //is this a dictionary object?
	};

	DictValueElement*   fAttributes;
	DictValueElement*   fInstanceAttrs;
	UInt32              fInstanceArraySize;
	QTSSDictionaryMap*  fMap;
	QTSSDictionaryMap*  fInstanceMap;
	OSMutex*            fMutexP;
	Bool16				fMyMutex;
	Bool16				fLocked;

	void DeleteAttributeData(DictValueElement* inDictValues,
		UInt32 inNumValues, QTSSDictionaryMap* theMap);
};


class QTSSAttrInfoDict : public QTSSDictionary
{
public:

	struct AttrInfo
	{
		// This is all the relevent information for each dictionary
		// attribute.
		char                    fAttrName[QTSS_MAX_ATTRIBUTE_NAME_SIZE + 1];
		QTSS_AttrFunctionPtr    fFuncPtr;
		QTSS_AttrDataType       fAttrDataType;
		QTSS_AttrPermission     fAttrPermission;
	};

	QTSSAttrInfoDict();
	virtual ~QTSSAttrInfoDict();

private:

	AttrInfo fAttrInfo;
	QTSS_AttributeID fID;

	static AttrInfo sAttributes[];

	friend class QTSSDictionaryMap;

};

class QTSSDictionaryMap
{
public:

	//
	// This must be called before using any QTSSDictionary or QTSSDictionaryMap functionality
	static void Initialize();

	// Stores all meta-information for attributes

	// CONSTRUCTOR FLAGS
	enum
	{
		kNoFlags = 0,
		kAllowRemoval = 1,
		kIsInstanceMap = 2,
		kInstanceAttrsAllowed = 4,
		kCompleteFunctionsAllowed = 8
	};

	//
	// CONSTRUCTOR / DESTRUCTOR

	QTSSDictionaryMap(UInt32 inNumReservedAttrs, UInt32 inFlags = kNoFlags);
	~QTSSDictionaryMap()
	{
		for (UInt32 i = 0; i < fAttrArraySize; i++)
			delete fAttrArray[i];
		delete[] fAttrArray;
	}

	//
	// QTSS API CALLS

	// All functions either return QTSS_BadArgument or QTSS_NoErr
	QTSS_Error      AddAttribute(const char* inAttrName,
		QTSS_AttrFunctionPtr inFuncPtr,
		QTSS_AttrDataType inDataType,
		QTSS_AttrPermission inPermission);

	//
	// Marks this attribute as removed
	QTSS_Error  RemoveAttribute(QTSS_AttributeID inAttrID);
	QTSS_Error  UnRemoveAttribute(QTSS_AttributeID inAttrID);
	QTSS_Error  CheckRemovePermission(QTSS_AttributeID inAttrID);

	//
	// Searching / Iteration. These never return removed attributes
	QTSS_Error  GetAttrInfoByName(const char* inAttrName, QTSSAttrInfoDict** outAttrInfoDict, Bool16 returnRemovedAttr = false);
	QTSS_Error  GetAttrInfoByID(QTSS_AttributeID inID, QTSSAttrInfoDict** outAttrInfoDict);
	QTSS_Error  GetAttrInfoByIndex(UInt32 inIndex, QTSSAttrInfoDict** outAttrInfoDict);
	QTSS_Error  GetAttrID(const char* inAttrName, QTSS_AttributeID* outID);

	//
	// PRIVATE ATTR PERMISSIONS
	enum
	{
		qtssPrivateAttrModeRemoved = 0x80000000
	};

	//
	// CONVERTING attribute IDs to array indexes. Returns -1 if inAttrID doesn't exist
	inline SInt32                   ConvertAttrIDToArrayIndex(QTSS_AttributeID inAttrID);

	static Bool16           IsInstanceAttrID(QTSS_AttributeID inAttrID)
	{
		return (inAttrID & 0x80000000) != 0;
	}

	// ACCESSORS

	// These functions do no error checking. Be careful.

	// Includes removed attributes
	UInt32          GetNumAttrs() { return fNextAvailableID; }
	UInt32          GetNumNonRemovedAttrs() { return fNumValidAttrs; }

	Bool16                  IsPreemptiveSafe(UInt32 inIndex)
	{
		Assert(inIndex < fNextAvailableID); return (Bool16)(fAttrArray[inIndex]->fAttrInfo.fAttrPermission & qtssAttrModePreempSafe);
	}

	Bool16                  IsWriteable(UInt32 inIndex)
	{
		Assert(inIndex < fNextAvailableID); return (Bool16)(fAttrArray[inIndex]->fAttrInfo.fAttrPermission & qtssAttrModeWrite);
	}

	Bool16                  IsCacheable(UInt32 inIndex)
	{
		Assert(inIndex < fNextAvailableID); return (Bool16)(fAttrArray[inIndex]->fAttrInfo.fAttrPermission & qtssAttrModeCacheable);
	}

	Bool16                  IsRemoved(UInt32 inIndex)
	{
		Assert(inIndex < fNextAvailableID); return (Bool16)(fAttrArray[inIndex]->fAttrInfo.fAttrPermission & qtssPrivateAttrModeRemoved);
	}

	QTSS_AttrFunctionPtr    GetAttrFunction(UInt32 inIndex)
	{
		Assert(inIndex < fNextAvailableID); return fAttrArray[inIndex]->fAttrInfo.fFuncPtr;
	}

	char*                   GetAttrName(UInt32 inIndex)
	{
		Assert(inIndex < fNextAvailableID); return fAttrArray[inIndex]->fAttrInfo.fAttrName;
	}

	QTSS_AttributeID        GetAttrID(UInt32 inIndex)
	{
		Assert(inIndex < fNextAvailableID); return fAttrArray[inIndex]->fID;
	}

	QTSS_AttrDataType       GetAttrType(UInt32 inIndex)
	{
		Assert(inIndex < fNextAvailableID); return fAttrArray[inIndex]->fAttrInfo.fAttrDataType;
	}

	Bool16                  InstanceAttrsAllowed() { return (Bool16)(fFlags & kInstanceAttrsAllowed); }
	Bool16                  CompleteFunctionsAllowed() { return (Bool16)(fFlags & kCompleteFunctionsAllowed); }

	// MODIFIERS

	// Sets this attribute ID to have this information

	void        SetAttribute(QTSS_AttributeID inID,
		const char* inAttrName,
		QTSS_AttrFunctionPtr inFuncPtr,
		QTSS_AttrDataType inDataType,
		QTSS_AttrPermission inPermission);


	//
	// DICTIONARY MAPS

	// All dictionary maps are stored here, and are accessable
	// through these routines

	// This enum allows all QTSSDictionaryMaps to be stored in an array 
	enum
	{
		kServerDictIndex = 0,
		kPrefsDictIndex = 1,
		kTextMessagesDictIndex = 2,
		kServiceDictIndex = 3,

		kFileDictIndex = 4,
		kModuleDictIndex = 5,
		kModulePrefsDictIndex = 6,
		kAttrInfoDictIndex = 7,
		kQTSSUserProfileDictIndex = 8,
		kQTSSConnectedUserDictIndex = 9,

		kNumDictionaries = 10,

		kNumDynamicDictionaryTypes = 500,
		kIllegalDictionary = kNumDynamicDictionaryTypes + kNumDictionaries
	};

	// This function converts a QTSS_ObjectType to an index
	static UInt32                   GetMapIndex(QTSS_ObjectType inType);

	// Using one of the above predefined indexes, this returns the corresponding map
	static QTSSDictionaryMap*       GetMap(UInt32 inIndex)
	{
		Assert(inIndex < kNumDynamicDictionaryTypes + kNumDictionaries); return sDictionaryMaps[inIndex];
	}

	static QTSS_ObjectType          CreateNewMap();

private:

	//
	// Repository for dictionary maps

	static QTSSDictionaryMap*       sDictionaryMaps[kNumDictionaries + kNumDynamicDictionaryTypes];
	static UInt32                   sNextDynamicMap;

	enum
	{
		kMinArraySize = 20
	};

	UInt32                          fNextAvailableID;
	UInt32                          fNumValidAttrs;
	UInt32                          fAttrArraySize;
	QTSSAttrInfoDict**              fAttrArray;
	UInt32                          fFlags;

	friend class QTSSDictionary;
};

inline SInt32   QTSSDictionaryMap::ConvertAttrIDToArrayIndex(QTSS_AttributeID inAttrID)
{
	SInt32 theIndex = inAttrID & 0x7FFFFFFF;
	if ((theIndex < 0) || (theIndex >= (SInt32)fNextAvailableID))
		return -1;
	else
		return theIndex;
}


#endif
