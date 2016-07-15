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
	 File:       AdminElements.h

	 Contains:   implements various Admin Elements class


 */
#ifndef _ADMINELEMENTNODE_H_
#define _ADMINELEMENTNODE_H_



#ifndef __Win32__
#include <unistd.h>     /* for getopt() et al */
#endif

#include <stdio.h>      /* for //qtss_printf */
#include "OSArrayObjectDeleter.h"
#include "StrPtrLen.h"
#include "OSRef.h"
#include "AdminQuery.h"

void PRINT_STR(StrPtrLen *spl);
void COPYBUFFER(char *dest, char *src, SInt8 size);

void ElementNode_InitPtrArray();
void ElementNode_InsertPtr(void *ptr, char * src);
void ElementNode_RemovePtr(void *ptr, char * src);
SInt32 ElementNode_CountPtrs();
void ElementNode_ShowPtrs();

class ClientSession {
public:
	ClientSession(void) : fRTSPSessionID(0), fBitrate(0), fPacketLossPercent(0), fBytesSent(0), fTimeConnected(0) {};
	~ClientSession() { };
	UInt32 fRTSPSessionID;
	char fIPAddressStr[32];
	char fURLBuffer[512];
	UInt32 fBitrate;
	Float32 fPacketLossPercent;
	UInt64 fBytesSent;
	UInt64 fTimeConnected;

};


class ElementNode
{
public:
	enum { eMaxAccessSize = 32, eMaxAttributeNameSize = 63, eMaxAPITypeSize = 63, eMaxAttrIDSize = sizeof(UInt32) };

	enum { eData = 0, eArrayNode, eNode };


#define kEmptyRef (OSRef *)NULL
#define kEmptyData (char *)NULL


	enum { kFirstIndexItem = 0 };


	typedef enum
	{
		eStatic = 0,
		eDynamic = 1,
	} DataFieldsType;

	struct ElementDataFields
	{
		UInt32                  fKey;
		UInt32                  fAPI_ID;
		UInt32                  fIndex;

		char                    fFieldName[eMaxAttributeNameSize + 1];
		UInt32                  fFieldLen;

		QTSS_AttrPermission     fAccessPermissions;
		char                    fAccessData[eMaxAccessSize + 1];
		UInt32                  fAccessLen;

		UInt32                  fAPI_Type;
		UInt32                  fFieldType;

		QTSS_Object             fAPISource;

	};

	SInt32                  fDataFieldsStop;

	UInt32  CountElements();

	SInt32  GetMyStopItem() { Assert(fSelfPtr); return fDataFieldsStop; };
	UInt32  GetMyKey() { Assert(fSelfPtr); return fSelfPtr->fKey; };
	char*   GetMyName() { Assert(fSelfPtr); return fSelfPtr->fFieldName; };
	UInt32  GetMyNameLen() { Assert(fSelfPtr); return fSelfPtr->fFieldLen; };
	UInt32  GetMyAPI_ID() { Assert(fSelfPtr); return fSelfPtr->fAPI_ID; };
	UInt32  GetMyIndex() { Assert(fSelfPtr); return fSelfPtr->fIndex; };

	UInt32  GetMyAPI_Type() { Assert(fSelfPtr); return fSelfPtr->fAPI_Type; };
	char*   GetMyAPI_TypeStr() { Assert(fSelfPtr); char* theTypeString = NULL; (void)QTSS_TypeToTypeString(GetMyAPI_Type(), &theTypeString); return theTypeString; };
	UInt32  GetMyFieldType() { Assert(fSelfPtr); return fSelfPtr->fFieldType; };

	char*   GetMyAccessData() { Assert(fSelfPtr); return fSelfPtr->fAccessData; };
	UInt32  GetMyAccessLen() { Assert(fSelfPtr); return fSelfPtr->fAccessLen; };
	UInt32  GetMyAccessPermissions() { Assert(fSelfPtr); return fSelfPtr->fAccessPermissions; };

	void    GetMyNameSPL(StrPtrLen* str) { Assert(str); if (str != NULL) str->Set(fSelfPtr->fFieldName, fSelfPtr->fFieldLen); };
	void    GetMyAccess(StrPtrLen* str) { Assert(str); if (str != NULL) str->Set(fSelfPtr->fAccessData, fSelfPtr->fAccessLen); };
	QTSS_Object GetMySource() {
		Assert(fSelfPtr != NULL);
		//qtss_printf("GetMySource fSelfPtr->fAPISource = %"_U32BITARG_" \n", fSelfPtr->fAPISource); 
		return fSelfPtr->fAPISource;
	};

	Bool16  IsNodeElement() { Assert(this); return (this->GetMyFieldType() == eNode || this->GetMyFieldType() == eArrayNode); }


	Bool16  IsStopItem(SInt32 index) { return index == GetMyStopItem(); };
	UInt32  GetKey(SInt32 index) { return fFieldIDs[index].fKey; };
	char*   GetName(SInt32 index) { return fFieldIDs[index].fFieldName; };
	UInt32  GetNameLen(SInt32 index) { return fFieldIDs[index].fFieldLen; };
	UInt32  GetAPI_ID(SInt32 index) { return fFieldIDs[index].fAPI_ID; };
	UInt32  GetAttributeIndex(SInt32 index) { return fFieldIDs[index].fIndex; };
	UInt32  GetAPI_Type(SInt32 index) { return fFieldIDs[index].fAPI_Type; };
	char*   GetAPI_TypeStr(SInt32 index) { char* theTypeStr = NULL; (void)QTSS_TypeToTypeString(GetAPI_Type(index), &theTypeStr); return theTypeStr; };
	UInt32  GetFieldType(SInt32 index) { return fFieldIDs[index].fFieldType; };
	char*   GetAccessData(SInt32 index) { return fFieldIDs[index].fAccessData; };
	UInt32  GetAccessLen(SInt32 index) { return fFieldIDs[index].fAccessLen; };
	UInt32  GetAccessPermissions(SInt32 index) { return fFieldIDs[index].fAccessPermissions; };
	void    GetNameSPL(SInt32 index, StrPtrLen* str) { if (str != NULL) str->Set(fFieldIDs[index].fFieldName, fFieldIDs[index].fFieldLen); };
	void    GetAccess(SInt32 index, StrPtrLen* str) { if (str != NULL) str->Set(fFieldIDs[index].fAccessData, fFieldIDs[index].fAccessLen); };
	QTSS_Object GetAPISource(SInt32 index) { return fFieldIDs[index].fAPISource; };
	Bool16  IsNodeElement(SInt32 index) { return (GetFieldType(index) == eNode || GetFieldType(index) == eArrayNode); }

	enum
	{
		eAPI_ID = 0,
		eAPI_Name = 1,
		eAccess = 2,
		ePath = 3,
		eType = 4,
		eNumAttributes = 5
	};

	ElementNode();
	void Initialize(SInt32 index, ElementNode *parentPtr, QueryURI *queryPtr, StrPtrLen *currentSegmentPtr, QTSS_Initialize_Params *initParams, QTSS_Object nodeSource, DataFieldsType dataFieldsType);
	virtual ~ElementNode();

	void            SetNodeName(char *namePtr);
	char *          GetNodeName() { return fNodeNameSPL.Ptr; };
	UInt32          GetNodeNameLen() { return fNodeNameSPL.Len; };
	StrPtrLen*      GetNodeNameSPL() { return &fNodeNameSPL; };

	void            SetParentNode(ElementNode *parentPtr) { fParentNodePtr = parentPtr; };
	ElementNode*    GetParentNode() { return fParentNodePtr; };
	void            GetFullPath(StrPtrLen *resultPtr);

	OSRef*  GetOSRef(SInt32 index);
	void    SetOSRef(SInt32 index, OSRef* refPtr);
	SInt32  ResolveSPLKeyToIndex(StrPtrLen *keyPtr);
	virtual Bool16  SetUpOneDataField(UInt32 index);

	ElementDataFields   *GetElementFieldPtr(SInt32 index);
	char                *GetElementDataPtr(SInt32 index);
	void                SetElementDataPtr(SInt32 index, char * data, Bool16 isNode);
	void                SetMyElementDataPtr(char * data) { fSelfDataPtr = data; }
	char*               GetMyElementDataPtr() { return fSelfDataPtr; }
	Bool16              IsFiltered(SInt32 index, QueryURI *queryPtr);

	ElementDataFields   *GetNodeInfoPtr(SInt32 index);

	void    SetNodeInfo(ElementDataFields *nodeInfo);
	void    SetSource(void * dataSource) { fDataSource = dataSource; };
	void *  GetSource() {
		QTSS_Object source = GetMySource();
		if (source != NULL)
			return source;
		else
		{   //qtss_printf("GetSource return fDataSource = %"_U32BITARG_" \n",fDataSource);
			return fDataSource;
		}
	};

	virtual void    SetUpSingleNode(QueryURI *queryPtr, StrPtrLen *currentSegmentPtr, StrPtrLen *nextSegmentPtr, SInt32 index, QTSS_Initialize_Params *initParams);
	virtual void    SetUpAllNodes(QueryURI *queryPtr, StrPtrLen *currentSegmentPtr, StrPtrLen *nextSegmentPtr, QTSS_Initialize_Params *initParams);

	virtual void    SetUpSingleElement(QueryURI *queryPtr, StrPtrLen *currentSegmentPtr, StrPtrLen *nextSegmentPtr, SInt32 index, QTSS_Initialize_Params *initParams);
	virtual void    SetUpAllElements(QueryURI *queryPtr, StrPtrLen *currentSegmentPtr, StrPtrLen *nextSegmentPtr, QTSS_Initialize_Params *initParams);
	virtual void    SetupNodes(QueryURI *queryPtr, StrPtrLen *currentPathPtr, QTSS_Initialize_Params *initParams);


	void    RespondWithSelfAdd(QTSS_StreamRef inStream, QueryURI *queryPtr);
	void    RespondToAdd(QTSS_StreamRef inStream, SInt32 index, QueryURI *queryPtr);
	void    RespondToSet(QTSS_StreamRef inStream, SInt32 index, QueryURI *queryPtr);
	void    RespondToGet(QTSS_StreamRef inStream, SInt32 index, QueryURI *queryPtr);
	void    RespondToDel(QTSS_StreamRef inStream, SInt32 index, QueryURI *queryPtr, Bool16 delAttribute);
	void    RespondToKey(QTSS_StreamRef inStream, SInt32 index, QueryURI *queryPtr);

	void    RespondWithNodeName(QTSS_StreamRef inStream, QueryURI *queryPtr);
	void    RespondWithSelf(QTSS_StreamRef inStream, QueryURI *queryPtr);
	void    RespondWithSingleElement(QTSS_StreamRef inStream, QueryURI *queryPtr, StrPtrLen *currentSegmentPtr);
	void    RespondWithAllElements(QTSS_StreamRef inStream, QueryURI *queryPtr, StrPtrLen *currentSegmentPtr);
	void    RespondWithAllNodes(QTSS_StreamRef inStream, QueryURI *queryPtr, StrPtrLen *currentSegmentPtr);
	void    RespondToQuery(QTSS_StreamRef inStream, QueryURI *queryPtr, StrPtrLen *currentPathPtr);

	UInt32  CountAttributes(QTSS_Object source);
	UInt32  CountValues(QTSS_Object source, UInt32 apiID);

	QTSS_Error      AllocateFields(UInt32 numFields);
	void            InitializeAllFields(Bool16 allocateFields, QTSS_Object defaultAttributeInfo, QTSS_Object source, QueryURI *queryPtr, StrPtrLen *currentSegmentPtr, Bool16 forceAll);
	void            InitializeSingleField(StrPtrLen *currentSegmentPtr);
	void            SetFields(UInt32 i, QTSS_Object attrInfoObject);
	ElementNode*    CreateArrayAttributeNode(UInt32 index, QTSS_Object source, QTSS_Object attributeInfo, UInt32 arraySize);

	QTSS_Error      GetAttributeSize(QTSS_Object inObject, QTSS_AttributeID inID, UInt32 inIndex, UInt32* outLenPtr);
	char*           NewIndexElement(QTSS_Object inObject, QTSS_AttributeID inID, UInt32 inIndex);
	UInt32          GetNumFields() { return fNumFields; };
	void            SetNumFields(UInt32 numFields) { fNumFields = numFields; fDataFieldsStop = numFields; };

	ElementDataFields*  GetFields() { return fFieldIDs; };
	void                SetFields(ElementDataFields *fieldsPtr) { fFieldIDs = fieldsPtr; };
	void                SetFieldsType(DataFieldsType fDataFieldsType) { this->fDataFieldsType = fDataFieldsType; };

	static void GetFilteredAttributeName(ElementDataFields* fieldPtr, QTSS_AttributeID theID);
	static Bool16 GetFilteredAttributeID(char *parentName, char *nodeName, QTSS_AttributeID* foundID);
	static Bool16 IsPreferenceContainer(char *nodeName, QTSS_AttributeID* foundID);

	enum { kmaxPathlen = 1048 };
	char                fPathBuffer[kmaxPathlen];
	StrPtrLen           fPathSPL;
	StrPtrLen           fNodeNameSPL;

	QTSS_Object         fDataSource;
	SInt32              fNumFields;
	SInt32              fPathLen;
	Bool16              fInitialized;

	ElementDataFields*  fFieldIDs;
	ElementDataFields*  fSelfPtr;
	DataFieldsType      fDataFieldsType;
	char*               fSelfDataPtr;


	char**              fFieldDataPtrs;
	OSRef**             fFieldOSRefPtrs;
	ElementNode*        fParentNodePtr;
	OSRefTable*         fElementMap;

	Bool16              fIsTop;

private:

	inline void DebugShowFieldDataType(SInt32 index);
	inline void DebugShowFieldValue(SInt32 index);


};

class AdminClass : public ElementNode
{
public:
	QueryURI *fQueryPtr;
	ElementNode *fNodePtr;

	void SetUpSingleElement(QueryURI *queryPtr, StrPtrLen *currentSegmentPtr, StrPtrLen *nextSegmentPtr, SInt32 index, QTSS_Initialize_Params *initParams);
	void SetUpSingleNode(QueryURI *queryPtr, StrPtrLen *currentSegmentPtr, StrPtrLen *nextSegmentPtr, SInt32 index, QTSS_Initialize_Params *initParams);
	void Initialize(QTSS_Initialize_Params *initParams, QueryURI *queryPtr);
	AdminClass() :fQueryPtr(NULL), fNodePtr(NULL) {};
	~AdminClass();
	static ElementNode::ElementDataFields sAdminSelf[];
	static ElementNode::ElementDataFields sAdminFieldIDs[];
	enum
	{
		eServer = 0,
		eNumAttributes
	};
};




#endif // _ADMINELEMENTNODE_H_
