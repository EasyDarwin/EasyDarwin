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
    File:       AdminElementNode.cpp

    Contains:   Implements Admin Elements
                    
    
    
*/


#ifndef __Win32__
    #include <unistd.h>     /* for getopt() et al */
#endif

#include <stdio.h>      /* for //qtss_printf */
#include <stdlib.h>     /* for getloadavg & other useful stuff */
#include <time.h>
#include "QTSS.h"
#include "QTSSAdminModule.h"
#include "OSArrayObjectDeleter.h"
#include "StringParser.h"
#include "StrPtrLen.h"
#include "QTSSModuleUtils.h"
#include "OSHashTable.h"
#include "OSMutex.h"
#include "StrPtrLen.h"
#include "OSRef.h"
#include "AdminElementNode.h"
#include "OSMemory.h"
//#include "OSHeaders.h"

static char* sParameterDelimeter = ";";
static char* sListDelimeter = ",";
static char* sAccess = "a=";
static char* sType = "t=";
static StrPtrLen sDoAllSPL("*");
static StrPtrLen sDoAllIndexIteratorSPL(":");

#define MEMORYDEBUGGING 0
#if MEMORYDEBUGGING
static SInt32 sMaxPtrs = 10000;
static void * sPtrArray[10000];
static char * sSourceArray[10000];
#endif

Bool16  ElementNode_DoAll(StrPtrLen* str)
{   
    Assert(str); 
    Bool16 isIterator = false;
    
    if ( str->Equal(sDoAllSPL) || str->Equal(sDoAllIndexIteratorSPL)) 
        isIterator = true;
        
    return isIterator;
}

void ElementNode_InitPtrArray()
{
#if MEMORYDEBUGGING
    memset(sPtrArray, 0, sizeof(sPtrArray));
    memset(sSourceArray, 0, sizeof(sSourceArray));
#endif
}

void ElementNode_InsertPtr(void *ptr, char * src)
{
#if MEMORYDEBUGGING
    if (ptr == NULL)
        return;

    for (SInt32 index = 0; index < sMaxPtrs; index ++)
    {
        if (sPtrArray[index] == NULL)
        {   sPtrArray[index] =ptr;
            sSourceArray[index] = src;
            //qtss_printf("%s INSERTED ptr=%p countPtrs=%"_S32BITARG_"\n",src, ptr, ElementNode_CountPtrs());  
            return;
        }   
    }
    
    qtss_printf("ElementNode_InsertPtr no space in ptr array\n");
    Assert(0);
#endif
}

Bool16 ElementNode_FindPtr(void *ptr, char * src)
{   // use for validating duplicates at some point
#if MEMORYDEBUGGING
    if (ptr == NULL)
        return false;
        
    for (SInt32 index = 0; index < sMaxPtrs; index ++)
    {   if (sPtrArray[index] == ptr)
            return true;
    }
    
#endif
    return false;
}

void ElementNode_RemovePtr(void *ptr, char * src)
{   
#if MEMORYDEBUGGING
    if (ptr == NULL)
        return;
        
    SInt16 foundCount = 0;
    for (SInt32 index = 0; index < sMaxPtrs; index ++)
    {
        if (sPtrArray[index] == ptr)
        {   
            sPtrArray[index] = NULL;
            sSourceArray[index] = NULL;
            //qtss_printf("%s REMOVED ptr countPtrs=%"_S32BITARG_"\n",src,ElementNode_CountPtrs());
            foundCount ++; // use for validating duplicates at some point
            return;
        }   
    }
    
    if (foundCount == 0)
    {   qtss_printf("PTR NOT FOUND ElementNode_RemovePtr %s ptr=%p countPtrs=%"_S32BITARG_"\n",src,ptr,ElementNode_CountPtrs());
        Assert(0);
    }
#endif
}

SInt32 ElementNode_CountPtrs()
{
#if MEMORYDEBUGGING
    SInt32 count = 0;
    for (SInt32 index = 0; index < sMaxPtrs; index ++)
    {
        if (sPtrArray[index] != NULL)
            count ++;
    }
    
    return count;
#else
    return 0;
#endif
}

void ElementNode_ShowPtrs()
{
#if MEMORYDEBUGGING
    for (SInt32 index = 0; index < sMaxPtrs; index ++)
    {
        if (sPtrArray[index] != NULL)
            qtss_printf("ShowPtrs ptr=%p source=%s\n", sPtrArray[index],sSourceArray[index]); 
    }
#endif
}

void PRINT_STR(StrPtrLen *spl)
{
    
    if (spl && spl->Ptr && spl->Ptr[0] != 0)
    {   char buff[1024] = {0};
        memcpy (buff,spl->Ptr, spl->Len);
        qtss_printf("%s len=%"_U32BITARG_"\n",buff,spl->Len);
    }
    else
    {   qtss_printf("(null)\n");
    }
}

void COPYBUFFER(char *dest,char *src,SInt8 size)
{
    if ( (dest != NULL) && (src != NULL) && (size > 0) ) 
        memcpy (dest, src, size);
    else
        Assert(0);
};

char* NewCharArrayCopy(StrPtrLen *theStringPtr)
{
    char* newArray = NULL;
    if (theStringPtr != NULL)
    {
        newArray = NEW char[theStringPtr->Len + 1];  
        if (newArray != NULL) 
        {   memcpy(newArray, theStringPtr->Ptr,theStringPtr->Len);
            newArray[theStringPtr->Len] = 0;
        }
    }
    return newArray;
}



ElementNode::ElementNode()
{
    fDataSource     = NULL;
    fNumFields      = 0;
    fPathLen        = 0;
    fInitialized    = false;

    fFieldIDs       = NULL;
    fFieldDataPtrs  = NULL;
    fFieldOSRefPtrs = NULL;
    fParentNodePtr  = NULL;
    fElementMap     = NULL;
    fSelfPtr        = NULL;
    fPathBuffer[0]=0;
    fPathSPL.Set(fPathBuffer,0);
    fIsTop = false;
    fDataFieldsType = eDynamic;

};

void ElementNode::Initialize(SInt32 index, ElementNode *parentPtr, QueryURI *queryPtr, StrPtrLen *currentSegmentPtr,QTSS_Initialize_Params *initParams,QTSS_Object nodeSource, DataFieldsType dataFieldsType)
{   
    //qtss_printf("------ ElementNode::Initialize ---------\n");
    
    if (!fInitialized)
    {
        SetParentNode(parentPtr);
        SetSource(nodeSource);
        
        SetNodeInfo(parentPtr->GetNodeInfoPtr(index));
        SetNodeName(parentPtr->GetName(index));
        SetMyElementDataPtr(parentPtr->GetElementDataPtr(index));

        fDataFieldsType = dataFieldsType;
        
        StrPtrLen nextSegment;
        StrPtrLen nextnextSegment;
        (void) queryPtr->NextSegment(currentSegmentPtr, &nextSegment);      
        (void) queryPtr->NextSegment(&nextSegment, &nextnextSegment);
        Bool16 forceAll = nextSegment.Equal(sDoAllIndexIteratorSPL) | nextnextSegment.Equal(sDoAllIndexIteratorSPL);
        
        if (GetFields() == NULL)
            InitializeAllFields(true, NULL, nodeSource,queryPtr,currentSegmentPtr,forceAll);
            
        fInitialized = true;
    }
    SetupNodes(queryPtr,currentSegmentPtr, initParams);
    
};


ElementNode::~ElementNode()
{
    
    //qtss_printf("ElementNode::~ElementNode delete %s Element Node # fields = %"_U32BITARG_"\n",GetNodeName(), fNumFields);
    
    for(SInt32 index = 0; !IsStopItem(index) ; index ++)
    {
        OSRef *theRefPtr = GetOSRef(index);
        if (theRefPtr != NULL )
        {
            //qtss_printf("deleting hash entry of %s \n", GetName(index));
            SetOSRef(index,NULL);
            (fElementMap->GetHashTable())->Remove(theRefPtr);
            delete (OSRef*) theRefPtr;  ElementNode_RemovePtr(theRefPtr,"ElementNode::~ElementNode OSRef *");
        }
        
        char *dataPtr = GetElementDataPtr(index);
        if (dataPtr != NULL)
        {
            SetElementDataPtr(index,NULL,IsNodeElement(index));
        }
    }
    
    delete fElementMap;  ElementNode_RemovePtr(fElementMap,"ElementNode::~ElementNode fElementMap");
    
    fElementMap = NULL;
    
    UInt32 i = 0;
    for (i = 0; i < GetNumFields(); i ++)
    {   SetElementDataPtr(i,NULL, IsNodeElement(i)) ;
        fFieldDataPtrs[i] = NULL;
    }
    delete fFieldDataPtrs; ElementNode_RemovePtr(fFieldDataPtrs,"ElementNode::~ElementNode fFieldDataPtrs");
    fFieldDataPtrs = NULL;
    
    for (i = 0; i < GetNumFields(); i ++)
    {   delete fFieldOSRefPtrs[i]; ElementNode_RemovePtr(fFieldOSRefPtrs[i],"ElementNode::~ElementNode fFieldOSRefPtrs");
        fFieldOSRefPtrs[i] = NULL;
    }
    delete fFieldOSRefPtrs;  ElementNode_RemovePtr(fFieldOSRefPtrs,"ElementNode::~ElementNode fFieldOSRefPtrs");
    fFieldOSRefPtrs = NULL;
            
    if (fDataFieldsType == eDynamic)
    {   delete fFieldIDs;  ElementNode_RemovePtr(fFieldIDs,"ElementNode::~ElementNode fFieldIDs");
        fFieldIDs = NULL;
    }
        
    SetNodeName(NULL);

};

QTSS_Error ElementNode::AllocateFields(UInt32 numFields)
{
    //qtss_printf("-------- ElementNode::AllocateFields ----------\n");
    //qtss_printf("ElementNode::AllocateFields numFields=%"_U32BITARG_"\n",numFields);
    
    QTSS_Error err = QTSS_NotEnoughSpace;
    
    Assert(GetNumFields() == 0);
    SetNumFields(numFields);

    if (numFields > 0) do
    {   
        Assert(fFieldIDs == NULL);
        fFieldIDs = NEW ElementNode::ElementDataFields[numFields]; ElementNode_InsertPtr(fFieldIDs,"ElementNode::AllocateFields fFieldIDs array");
        Assert(fFieldIDs != NULL);
        if (fFieldIDs == NULL) break;
        memset(fFieldIDs, 0, numFields * sizeof(ElementNode::ElementDataFields));
        
        Assert(fElementMap == NULL);
        fElementMap = NEW OSRefTable();  ElementNode_InsertPtr(fElementMap,"ElementNode::AllocateFields fElementMap OSRefTable");
        Assert(fElementMap != NULL);
        if (fElementMap == NULL) break;
        
        Assert(fFieldDataPtrs == NULL);
        fFieldDataPtrs = NEW char*[numFields]; ElementNode_InsertPtr(fFieldDataPtrs,"ElementNode::AllocateFields fFieldDataPtrs array");
        Assert(fFieldDataPtrs != NULL);
        if (fFieldDataPtrs == NULL) break;
        memset(fFieldDataPtrs, 0, numFields * sizeof(char*));
            
        Assert(fFieldOSRefPtrs == NULL);
        fFieldOSRefPtrs = NEW OSRef*[numFields];  ElementNode_InsertPtr(fFieldOSRefPtrs,"ElementNode::AllocateFields fFieldDataPtrs array");
        Assert(fFieldOSRefPtrs != NULL);
        if (fFieldOSRefPtrs == NULL) break;
        memset(fFieldOSRefPtrs, 0, numFields * sizeof(OSRef*));
        
        err = QTSS_NoErr;
    } while (false);
    
    return err; 
};




void ElementNode::SetFields(UInt32 i, QTSS_Object attrInfoObject)
{
    //qtss_printf("------- ElementNode::SetFields -------- \n");

    UInt32 ioLen = 0;
    QTSS_Error err = QTSS_NoErr;
    if(fFieldIDs[i].fFieldName[0] != 0)
        return;
        
    if(fFieldIDs[i].fFieldName[0] == 0)
    {
        fFieldIDs[i].fFieldLen = eMaxAttributeNameSize;
        err = QTSS_GetValue (attrInfoObject, qtssAttrName,0, &fFieldIDs[i].fFieldName,&fFieldIDs[i].fFieldLen);
        Assert(err == QTSS_NoErr);  
        if (fFieldIDs[i].fFieldName != NULL)
            fFieldIDs[i].fFieldName[fFieldIDs[i].fFieldLen] = 0;
    }
    
    ioLen = sizeof(fFieldIDs[i].fAPI_ID);
    err = QTSS_GetValue (attrInfoObject, qtssAttrID,0, &fFieldIDs[i].fAPI_ID, &ioLen);
    Assert(err == QTSS_NoErr);  
    
    ioLen = sizeof(fFieldIDs[i].fAPI_Type);
    err = QTSS_GetValue (attrInfoObject, qtssAttrDataType,0, &fFieldIDs[i].fAPI_Type, &ioLen);
    Assert(err == QTSS_NoErr);  
    if (fFieldIDs[i].fAPI_Type == 0 || err != QTSS_NoErr)
    {
        //qtss_printf("QTSS_GetValue err = %"_S32BITARG_" attrInfoObject=%"_U32BITARG_" qtssAttrDataType = %"_U32BITARG_" \n",err, (UInt32)  attrInfoObject, (UInt32) fFieldIDs[i].fAPI_Type);
    }
    
    if (fFieldIDs[i].fAPI_Type == qtssAttrDataTypeQTSS_Object)
        fFieldIDs[i].fFieldType = eNode;
        
    ioLen = sizeof(fFieldIDs[i].fAccessPermissions);
    err = QTSS_GetValue (attrInfoObject, qtssAttrPermissions,0, &fFieldIDs[i].fAccessPermissions, &ioLen);
    Assert(err == QTSS_NoErr);  
    
    fFieldIDs[i].fAccessData[0] = 0;
    if (fFieldIDs[i].fAccessPermissions & qtssAttrModeRead)
    {   strcat(fFieldIDs[i].fAccessData, "r");
    }
    
    if (fFieldIDs[i].fAccessPermissions & qtssAttrModeWrite && fFieldIDs[i].fAPI_Type != qtssAttrDataTypeQTSS_Object)
    {   strcat(fFieldIDs[i].fAccessData, "w");
    }

    if (fFieldIDs[i].fAccessPermissions & qtssAttrModeInstanceAttrAllowed && fFieldIDs[i].fAPI_Type == qtssAttrDataTypeQTSS_Object)
    {   strcat(fFieldIDs[i].fAccessData, "w");
    }

    if (GetMyFieldType() != eNode && GetNumFields() > 1)
    {   strcat(fFieldIDs[i].fAccessData, "d");
    }
    if (fFieldIDs[i].fAccessPermissions & qtssAttrModeDelete)
    {   strcat(fFieldIDs[i].fAccessData, "d");
    }
    

    if (fFieldIDs[i].fAccessPermissions & qtssAttrModePreempSafe)
    {   strcat(fFieldIDs[i].fAccessData, "p");
    }

    fFieldIDs[i].fAccessLen = strlen(fFieldIDs[i].fAccessData);
    
    //qtss_printf("ElementNode::SetFields name=%s api_id=%"_S32BITARG_" \n",fFieldIDs[i].fFieldName, fFieldIDs[i].fAPI_ID);
    //DebugShowFieldDataType(i);    
};


ElementNode* ElementNode::CreateArrayAttributeNode(UInt32 index, QTSS_Object source, QTSS_Object attributeInfo, UInt32 arraySize )
{
    //qtss_printf("------- ElementNode::CreateArrayAttributeNode --------\n");
    //qtss_printf("ElementNode::CreateArrayAttributeNode name = %s index = %"_U32BITARG_" arraySize =%"_U32BITARG_" \n",fFieldIDs[index].fFieldName, index,arraySize);

    ElementDataFields* fieldPtr = NULL;
    SetFields(index, attributeInfo);
    fFieldIDs[index].fFieldType = eArrayNode;
    
    ElementNode* nodePtr = NEW ElementNode(); ElementNode_InsertPtr(nodePtr,"ElementNode::CreateArrayAttributeNode ElementNode*");
    this->SetElementDataPtr(index,(char *) nodePtr, true);
    Assert(nodePtr!=NULL);
    if (NULL == nodePtr) return NULL;
    
    nodePtr->SetSource(source); // the node's API source
    nodePtr->AllocateFields(arraySize); 
    
    if (this->GetNodeInfoPtr(index) == NULL)
    {   //qtss_printf("ElementNode::CreateArrayAttributeNode index = %"_U32BITARG_" this->GetNodeInfoPtr(index) == NULL \n",index);
    }
    nodePtr->SetNodeInfo(this->GetNodeInfoPtr(index));
    
    for (UInt32 i = 0; !nodePtr->IsStopItem(i); i++)
    {   
        fieldPtr = nodePtr->GetElementFieldPtr(i);
        Assert(fieldPtr != NULL);
        
        nodePtr->SetFields(i, attributeInfo);
        
        fieldPtr->fIndex = i; // set the API attribute index

        // set the name of the field to the array index 
        fieldPtr->fFieldName[0]= 0; 
        qtss_sprintf(fieldPtr->fFieldName,"%"_U32BITARG_,i);
        fieldPtr->fFieldLen = ::strlen(fieldPtr->fFieldName);
        
        if (fieldPtr->fAPI_Type != qtssAttrDataTypeQTSS_Object)
        {
            //qtss_printf("ElementNode::CreateArrayAttributeNode array field index = %"_U32BITARG_" name = %s api Source = %"_U32BITARG_" \n", (UInt32)  i,fieldPtr->fFieldName, (UInt32) source);
            fieldPtr->fAPISource = source; // the attribute's source is the same as node source
        }
        else 
        {   fieldPtr->fFieldType = eNode;
            // this is an array of objects so record each object as the source for a new node
            UInt32 sourceLen = sizeof(fieldPtr->fAPISource);
            QTSS_Error err = QTSS_GetValue (source,fieldPtr->fAPI_ID,fieldPtr->fIndex, &fieldPtr->fAPISource, &sourceLen);
            Warn(err == QTSS_NoErr);
            if (err != QTSS_NoErr)
            {   //qtss_printf("Error Getting Value for %s type = qtssAttrDataTypeQTSS_Object err = %"_U32BITARG_"\n", fieldPtr->fFieldName,err);
                fieldPtr->fAPISource = NULL;
            }
            
            QTSS_AttributeID id;
            Bool16 foundFilteredAttribute = GetFilteredAttributeID(GetMyName(),nodePtr->GetMyName(), &id);
            if (foundFilteredAttribute)
            {   GetFilteredAttributeName(fieldPtr,id);
            }

            //qtss_printf("ElementNode::CreateArrayAttributeNode array field index = %"_U32BITARG_" name = %s api Source = %"_U32BITARG_" \n", i,fieldPtr->fFieldName, (UInt32) fieldPtr->fAPISource);
        }

        nodePtr->fElementMap->Register(nodePtr->GetOSRef(i));
    }
    nodePtr->SetNodeName(GetName(index));
    nodePtr->SetParentNode(this);
    nodePtr->SetSource(source);
    nodePtr->fInitialized = true;
    
    return nodePtr;
    
}

void ElementNode::InitializeAllFields(Bool16 allocateFields, QTSS_Object defaultAttributeInfo, QTSS_Object source, QueryURI *queryPtr, StrPtrLen *currentSegmentPtr , Bool16 forceAll =false)
{
    //qtss_printf("------- ElementNode::InitializeAllFields -------- \n");
    
    QTSS_Error err = QTSS_NoErr;
    QTSS_Object theAttributeInfo;
    
    if (allocateFields)
    {           
        UInt32 numFields = this->CountAttributes(source);
        err = AllocateFields( numFields);
        //qtss_printf("ElementNode::InitializeAllFields AllocateFields numFields =  %"_U32BITARG_" error = %"_S32BITARG_" \n",numFields, err);
    }
    
    if (err == QTSS_NoErr)
    {   
        UInt32 numValues = 0;

        for (UInt32 i = 0; !IsStopItem(i); i++)
        {   
            if (defaultAttributeInfo == NULL)
            {   err = QTSS_GetAttrInfoByIndex(source, i, &theAttributeInfo);
                Assert(err == QTSS_NoErr);          
                if (err != QTSS_NoErr)
                {   //qtss_printf("QTSS_GetAttrInfoByIndex returned err = %"_U32BITARG_" \n",err);
                }
            }
            else
            {   theAttributeInfo = defaultAttributeInfo; 
            }
            
            SetFields(i, theAttributeInfo);         
            
            if ((SInt32) fFieldIDs[i].fAPI_ID < 0)
            {   //qtss_printf("ElementNode::InitializeAllFields name = %s index = %"_S32BITARG_" numValues =%"_U32BITARG_" \n",fFieldIDs[i].fFieldName, (SInt32) fFieldIDs[i].fAPI_ID,numValues);
            }
            numValues = this->CountValues(source, fFieldIDs[i].fAPI_ID);
            //qtss_printf("ElementNode::InitializeAllFields name = %s index = %"_U32BITARG_" numValues =%"_U32BITARG_" \n",fFieldIDs[i].fFieldName, fFieldIDs[i].fAPI_ID,numValues);

            QTSS_AttributeID id;
            Bool16 foundFilteredAttribute = GetFilteredAttributeID(GetMyName(),GetName(i), &id);
            
            StrPtrLen nextSegment;
            (void) queryPtr->NextSegment(currentSegmentPtr, &nextSegment);

            if (forceAll || nextSegment.Equal(sDoAllIndexIteratorSPL) || queryPtr->IndexParam() || numValues > 1 || foundFilteredAttribute)
            {       
                ElementNode *nodePtr = CreateArrayAttributeNode(i, source, theAttributeInfo,numValues);
                Assert(nodePtr != NULL);
                /*
                if (NULL == nodePtr) 
                {   //qtss_printf("ElementNode::InitializeAllFields(NULL == CreateArrayAttributeNode  nodePtr\n");
                }
                if (NULL == GetElementDataPtr(i)) 
                {   //qtss_printf("ElementNode::InitializeAllFields(NULL == GetElementDataPtr (i=%"_U32BITARG_") nodePtr=%"_U32BITARG_" \n",i, (UInt32) nodePtr);
                }
                */
                    
            }
            else
            {
                //qtss_printf("ElementNode::InitializeAllFields field index = %"_U32BITARG_" name = %s api Source = %"_U32BITARG_" \n", i,fFieldIDs[i].fFieldName, (UInt32) source);
            }
            
            err = fElementMap->Register(GetOSRef(i));
            if (err != QTSS_NoErr)
            {   //qtss_printf("ElementNode::InitializeAllFields  Register returned err = %"_U32BITARG_" field = %s node=%s \n",err,GetName(i),GetMyName());
            }
            Assert(err == QTSS_NoErr);  
        }
    }
};


void ElementNode::SetNodeInfo(ElementDataFields *nodeInfoPtr)
{
    if (nodeInfoPtr == NULL)
    {
        //qtss_printf("---- SetNodeInfo nodeInfoPtr = NULL \n");
    }
    else
    {
        //qtss_printf("---- SetNodeInfo nodeInfoPtr name = %s \n",nodeInfoPtr->fFieldName);
        fSelfPtr = nodeInfoPtr;
    }
};


void ElementNode::SetNodeName(char *namePtr)
{
    if (namePtr == NULL)
    {   delete fNodeNameSPL.Ptr; ElementNode_RemovePtr(fNodeNameSPL.Ptr,"ElementNode::SetNodeName char* ");
        fNodeNameSPL.Set(NULL, 0);
        return;
    }
    
    if (fNodeNameSPL.Ptr != NULL) 
    {   delete fNodeNameSPL.Ptr; ElementNode_RemovePtr(fNodeNameSPL.Ptr,"ElementNode::SetNodeName char* ");
        fNodeNameSPL.Set(NULL, 0);
    }
    //qtss_printf(" ElementNode::SetNodeName new NodeName = %s \n",namePtr);
    int len = ::strlen(namePtr);    
    fNodeNameSPL.Ptr = NEW char[len + 1]; ElementNode_InsertPtr(fNodeNameSPL.Ptr,"ElementNode::SetNodeName ElementNode* chars");
    fNodeNameSPL.Len = len; 
    memcpy(fNodeNameSPL.Ptr,namePtr,len);
    fNodeNameSPL.Ptr[len] = 0;
};

ElementNode::ElementDataFields *ElementNode::GetElementFieldPtr(SInt32 index) 
{ 
    ElementNode::ElementDataFields *resultPtr = NULL; 
    Assert (fFieldIDs != NULL);
    Assert ((index >= 0) && (index < (SInt32) fNumFields));
    if ((index >= 0) && (index < (SInt32) fNumFields)) 
        resultPtr = &fFieldIDs[index]; 
    return resultPtr; 
}

char *ElementNode::GetElementDataPtr(SInt32 index) 
{ 
    char *resultPtr = NULL; 
    Assert((index >= 0) && (index < (SInt32) fNumFields));
    if (fInitialized && (fFieldDataPtrs != NULL) && (index >= 0) && (index < (SInt32) fNumFields)) 
    {   resultPtr = fFieldDataPtrs[index]; 
    }
    return resultPtr; 
}

void ElementNode::SetElementDataPtr(SInt32 index,char *data, Bool16 isNode) 
{ 
    //qtss_printf("------ElementNode::SetElementDataPtr----- \n");
    //qtss_printf("ElementNode::SetElementDataPtr index = %"_S32BITARG_" fNumFields = %"_S32BITARG_" \n", index,fNumFields);
    Assert  ((index >= 0) && (index < (SInt32) fNumFields));
    if      ((index >= 0) && (index < (SInt32) fNumFields)) 
    {   //Assert(fFieldDataPtrs[index] == NULL);
        if (fDataFieldsType != eStatic)
        {   
            if (isNode)
            {   delete (ElementNode*) fFieldDataPtrs[index];ElementNode_RemovePtr(fFieldDataPtrs[index],"ElementNode::SetElementDataPtr ElementNode* fFieldDataPtrs");
            }
            else
            {   delete fFieldDataPtrs[index]; ElementNode_RemovePtr(fFieldDataPtrs[index],"ElementNode::SetElementDataPtr char* fFieldDataPtrs");
            }
        }
        fFieldDataPtrs[index] = data; 
        //qtss_printf("ElementNode::SetElementDataPtr index = %"_S32BITARG_" \n", index);
    }
}



inline void ElementNode::DebugShowFieldDataType(SInt32 /*index*/)
{
    //char field[100];
    //field[0] = ' ';
    //char* typeStringPtr = GetAPI_TypeStr(index);
    //qtss_printf("debug: %s=%s\n",GetName(index),typeStringPtr);
        
}

inline void ElementNode::DebugShowFieldValue(SInt32 /*index*/)
{
    //qtss_printf("debug: %s=%s\n",GetName(index),GetElementDataPtr(index));
}

ElementNode::ElementDataFields *ElementNode::GetNodeInfoPtr(SInt32 index) 
{ 
    ElementNode::ElementDataFields *resultPtr = GetElementFieldPtr(index);
    Assert (resultPtr != NULL);
    
    if ((resultPtr != NULL) && ((eNode != resultPtr->fFieldType) && (eArrayNode != resultPtr->fFieldType) )) 
        resultPtr = NULL; 
    return resultPtr; 
}       


void ElementNode::SetUpSingleNode(QueryURI *queryPtr,  StrPtrLen *currentSegmentPtr, StrPtrLen *nextSegmentPtr, SInt32 index, QTSS_Initialize_Params *initParams) 
{
    //qtss_printf("--------ElementNode::SetUpSingleNode ------------\n");
    if (queryPtr && currentSegmentPtr && nextSegmentPtr&& initParams) do
    {
        if (!queryPtr->RecurseParam() && (nextSegmentPtr->Len == 0) ) break;
    
        ElementNode::ElementDataFields *theNodePtr = GetNodeInfoPtr(index); 
        if (NULL == theNodePtr) 
        {
            //qtss_printf(" ElementNode::SetUpSingleNode (NULL == theNodePtr(%"_S32BITARG_")) name=%s \n",index,GetName(index));
            break;
        }
        
        if (!IsNodeElement(index) ) 
        {
            //qtss_printf(" ElementNode::SetUpSingleNode (apiType != qtssAttrDataTypeQTSS_Object) \n");
            break;
        }
        
        // filter unnecessary nodes     
        char *nodeName = GetName(index); 
        if (nodeName != NULL)
        {   StrPtrLen nodeNameSPL(nodeName);
            if  (   (!nodeNameSPL.Equal(*nextSegmentPtr) && !ElementNode_DoAll(nextSegmentPtr))
                    &&
                    !(queryPtr->RecurseParam() && (nextSegmentPtr->Len == 0))
                )
            {
                //qtss_printf(" ElementNode::SetUpSingleNode SPL TEST SKIP NodeElement= %s\n",GetName(index));
                //qtss_printf("ElementNode::SetUpAllNodes skip nextSegmentPtr=");PRINT_STR(nextSegmentPtr);
                break;
            }
                
        }
        
        ElementNode *nodePtr = NULL;
        nodePtr = (ElementNode *) GetElementDataPtr(index);     
        if (nodePtr == NULL)
        {   
            //qtss_printf("ElementNode::SetUpSingleNode %s nodePtr == NULL make NEW nodePtr index = %"_S32BITARG_"\n", GetMyName(),index);
            nodePtr = NEW ElementNode(); ElementNode_InsertPtr(nodePtr,"ElementNode::SetUpSingleNode ElementNode* NEW ElementNode() ");     
            SetElementDataPtr(index,(char *) nodePtr, true); 
        }
        
        if (nodePtr != NULL)
        {
            StrPtrLen tempSegment;
            ( void)queryPtr->NextSegment(nextSegmentPtr, &tempSegment);
            currentSegmentPtr = nextSegmentPtr;
            nextSegmentPtr = &tempSegment;
            

            if (!nodePtr->fInitialized)
            {
                //qtss_printf("ElementNode::SetUpSingleNode Node !fInitialized -- Initialize %s\n",GetName(index));
                //qtss_printf("ElementNode::SetUpSingleNode GetValue source = %"_U32BITARG_" name = %s id = %"_U32BITARG_" \n",(UInt32)  GetSource(),(UInt32)  GetName(index),(UInt32) GetAPI_ID(index));
                
                ElementDataFields* fieldPtr = GetElementFieldPtr(index);
                if (fieldPtr != NULL && fieldPtr->fAPI_Type == qtssAttrDataTypeQTSS_Object)
                {   UInt32 sourceLen = sizeof(fieldPtr->fAPISource);
                    (void) QTSS_GetValue (GetSource(),fieldPtr->fAPI_ID,fieldPtr->fIndex, &fieldPtr->fAPISource, &sourceLen);
                }
            
                QTSS_Object theSourceObject = GetAPISource(index);
                //Warn(theSourceObject != NULL);

                nodePtr->Initialize(index, this, queryPtr,nextSegmentPtr,initParams, theSourceObject, eDynamic);
                nodePtr->SetUpAllElements(queryPtr, currentSegmentPtr,nextSegmentPtr, initParams);
                fInitialized = true;
    
                break;

            }
            else
            {
                nodePtr->SetUpAllElements(queryPtr, currentSegmentPtr,nextSegmentPtr, initParams);      
            }
        }
        
    } while (false);
    
    return;
}

Bool16 ElementNode::SetUpOneDataField( UInt32 index)
{
    //qtss_printf("----ElementNode::SetUpOneDataField----\n");       
    //qtss_printf(" ElementNode::SetUpOneDataField parent = %s field name=%s\n",GetNodeName(), GetName(index));  
    
    QTSS_AttributeID inID = GetAPI_ID(index); 
    Bool16 isNodeResult = IsNodeElement(index);
    char *testPtr =  GetElementDataPtr(index);
    //Warn(NULL == testPtr);
    if (NULL != testPtr) 
    {   //qtss_printf(" ElementNode::SetUpOneDataField skip field already setup parent = %s field name=%s\n",GetNodeName(), GetName(index)); 
        return isNodeResult;
    }
    
    if (!isNodeResult)
    {
        //qtss_printf("ElementNode::SetUpOneDataField %s Source=%"_U32BITARG_" Field index=%"_U32BITARG_" API_ID=%"_U32BITARG_" value index=%"_U32BITARG_"\n",GetName(index),GetSource(), index,inID,GetAttributeIndex(index));  
        SetElementDataPtr(index, NewIndexElement (GetSource() , inID, GetAttributeIndex(index)), false);
    }
    else
    {
        //qtss_printf("ElementNode::SetUpOneDataField %s Source=%"_U32BITARG_" Field index=%"_U32BITARG_" API_ID=%"_U32BITARG_" value index=%"_U32BITARG_"\n",GetName(index),(UInt32) GetSource(),(UInt32)  index,(UInt32) inID,(UInt32) GetAttributeIndex(index));  
        //qtss_printf("ElementNode::SetUpOneDataField %s IsNodeElement index = %"_U32BITARG_"\n",GetName(index),(UInt32)  index);   
        //DebugShowFieldDataType(index);
    }

    DebugShowFieldValue( index);

    return isNodeResult;
}

void ElementNode::SetUpAllElements(QueryURI *queryPtr, StrPtrLen *currentSegmentPtr,StrPtrLen *nextSegmentPtr,  QTSS_Initialize_Params *initParams) 
{
    //qtss_printf("---------ElementNode::SetUpAllElements------- \n");

    for(SInt32 index = 0; !IsStopItem(index); index ++)
    {
        SetUpSingleElement(queryPtr, currentSegmentPtr,nextSegmentPtr, index,initParams);
    }
}




void ElementNode::SetUpSingleElement(QueryURI *queryPtr, StrPtrLen *currentSegmentPtr,StrPtrLen *nextSegmentPtr, SInt32 index, QTSS_Initialize_Params *initParams) 
{
    //qtss_printf("---------ElementNode::SetUpSingleElement------- \n");
    StrPtrLen indexNodeNameSPL;
    GetNameSPL(index,&indexNodeNameSPL);
    if  (   (queryPtr->RecurseParam() && (nextSegmentPtr->Len == 0))
            || 
            (indexNodeNameSPL.Equal(*nextSegmentPtr) || ElementNode_DoAll(nextSegmentPtr)) 
        ) // filter unnecessary elements        
    {
    
        Bool16 isNode = SetUpOneDataField((UInt32) index);      
        if (isNode)
        {
            //qtss_printf("ElementNode::SetUpSingleElement isNode=true calling SetUpSingleNode \n");
            SetUpSingleNode(queryPtr,currentSegmentPtr, nextSegmentPtr, index, initParams);
        }
    }
    else
    {   //qtss_printf("ElementNode::SetUpSingleElement filter element=%s\n",GetName(index));
    }
}


void ElementNode::SetUpAllNodes(QueryURI *queryPtr, StrPtrLen *currentSegmentPtr, StrPtrLen *nextSegmentPtr, QTSS_Initialize_Params *initParams) 
{
    //qtss_printf("--------ElementNode::SetUpAllNodes------- \n");
    for(SInt32 index = 0; !IsStopItem(index); index ++)
    {
        if (!queryPtr->RecurseParam() && (nextSegmentPtr->Len == 0) ) break;
        
        //qtss_printf("ElementNode::SetUpAllNodes index = %"_S32BITARG_" nextSegmentPtr=", index);PRINT_STR(nextSegmentPtr);
        StrPtrLen indexNodeNameSPL;
        GetNameSPL(index,&indexNodeNameSPL);
        if  (   IsNodeElement(index) 
                &&
                (   (queryPtr->RecurseParam() && (nextSegmentPtr->Len == 0))
                    || 
                    (indexNodeNameSPL.Equal(*nextSegmentPtr) || ElementNode_DoAll(nextSegmentPtr)) 
                )
            ) // filter unnecessary nodes       
            SetUpSingleNode(queryPtr, currentSegmentPtr, nextSegmentPtr, index, initParams);
        else
        {
            //qtss_printf("ElementNode::SetUpAllNodes skip index = %"_S32BITARG_" indexNodeName=", index);PRINT_STR(&indexNodeNameSPL);
            //qtss_printf("ElementNode::SetUpAllNodes skip nextSegmentPtr=");PRINT_STR(nextSegmentPtr);
        }
    }
}

QTSS_Error ElementNode::GetAttributeSize (QTSS_Object inObject, QTSS_AttributeID inID, UInt32 inIndex, UInt32* outLenPtr)
{
    return QTSS_GetValue (inObject, inID, inIndex, NULL, outLenPtr);
}

char *ElementNode::NewIndexElement (QTSS_Object inObject, QTSS_AttributeID inID, UInt32 inIndex)
{   
    QTSS_Error err = QTSS_NoErr;
    char *resultPtr = NULL;

    Assert(inObject != NULL);
    
    if (inObject != NULL)
    {   err = QTSS_GetValueAsString (inObject, inID, inIndex, &resultPtr); ElementNode_InsertPtr(resultPtr,"ElementNode::NewIndexElement QTSS_GetValueAsString ");
        if (err != QTSS_NoErr)
        {   //qtss_printf("ElementNode::NewIndexElement QTSS_GetValueAsString object= %p id=%"_U32BITARG_" index=%"_U32BITARG_" err= %"_S32BITARG_" \n",inObject,inID, inIndex, err);
        }
    }
    return resultPtr;
}


inline  SInt32 ElementNode::ResolveSPLKeyToIndex(StrPtrLen *keyPtr)
{   
    SInt32 index = -1; 
    OSRef* osrefptr = NULL; 
    PointerSizedInt object = 0;
            
    if (fElementMap != NULL && keyPtr != NULL && keyPtr->Len > 0) 
    {   osrefptr = fElementMap->Resolve(keyPtr);
        if (osrefptr != NULL) 
        {   object = (PointerSizedInt) osrefptr->GetObject();
            index =  (SInt32) object; 
        }
    }

    return index;   
}


UInt32 ElementNode::CountAttributes(QTSS_Object source)
{
    //qtss_printf("------ElementNode::CountAttributes-------\n");
    //qtss_printf("ElementNode::CountAttributes SOURCE = %"_U32BITARG_" \n", (UInt32) source);

    UInt32 numFields = 0;

    (void) QTSS_GetNumAttributes (source, &numFields);
    
    //qtss_printf("ElementNode::CountAttributes %s = %"_U32BITARG_" \n",GetNodeName() ,numFields);

    return numFields;
}

UInt32 ElementNode::CountValues(QTSS_Object source, UInt32 apiID)
{
    //qtss_printf("------ElementNode::CountValues-------\n");
    UInt32 numFields = 0;
    
    (void) QTSS_GetNumValues (source, apiID, &numFields);

    //qtss_printf("ElementNode::CountValues %s = %"_U32BITARG_" \n",GetNodeName() ,numFields);

    return numFields;
}



OSRef* ElementNode::GetOSRef(SInt32 index)
{   
    StrPtrLen   theName;
    OSRef*      resultPtr = NULL;
    
    resultPtr = fFieldOSRefPtrs[index];
//      Assert(resultPtr != NULL);
    if (resultPtr == NULL)
    {   
        fFieldOSRefPtrs[index] = NEW OSRef(); Assert(fFieldOSRefPtrs[index] != NULL); ElementNode_InsertPtr(fFieldOSRefPtrs[index],"ElementNode::GetOSRef NEW OSRef() fFieldOSRefPtrs ");   
        GetNameSPL(index,&theName); Assert(theName.Len != 0);
        //qtss_printf("ElementNode::GetOSRef index = %"_S32BITARG_" name = %s \n", index, theName.Ptr);
        fFieldOSRefPtrs[index]->Set(theName,(void *) index);
        if (0 != theName.Len && NULL != theName.Ptr) //return the ptr else NULL
            resultPtr = fFieldOSRefPtrs[index];
    }
    
    
    return resultPtr;
}

void ElementNode::SetOSRef(SInt32 index, OSRef* refPtr)
{
    Assert  ((index >= 0) && (index < (SInt32) fNumFields));
    if      (fInitialized && (index >= 0) && (index < (SInt32) fNumFields)) 
        fFieldOSRefPtrs[index] = refPtr; 
}

void ElementNode::GetFullPath(StrPtrLen *resultPtr)
{
    //qtss_printf("ElementNode::GetFullPath this node name %s \n",GetNodeName());

    Assert(fPathSPL.Ptr != NULL);   
    
    if (fPathSPL.Len != 0)
    {   
        resultPtr->Set(fPathSPL.Ptr,fPathSPL.Len);
        //qtss_printf("ElementNode::GetFullPath has path=%s\n",resultPtr->Ptr);
        return;
    }   
    
    ElementNode *parentPtr = GetParentNode();
    if (parentPtr != NULL)
    {
        StrPtrLen parentPath;
        parentPtr->GetFullPath(&parentPath);
        memcpy(fPathSPL.Ptr,parentPath.Ptr,parentPath.Len);
        fPathSPL.Ptr[parentPath.Len] = 0;
        fPathSPL.Len = parentPath.Len;
    }
    
    UInt32 nodeNameLen = GetNodeNameLen();
    if (nodeNameLen > 0)
    {
        fPathSPL.Len += nodeNameLen + 1;
        Assert(fPathSPL.Len < kmaxPathlen);
        if (fPathSPL.Len < kmaxPathlen)
        {   strcat(fPathSPL.Ptr, GetNodeName());
            strcat(fPathSPL.Ptr,"/"); 
            fPathSPL.Len = strlen(fPathSPL.Ptr);
        }
     }

    resultPtr->Set(fPathSPL.Ptr,fPathSPL.Len);
    //qtss_printf("ElementNode::GetFullPath element=%s received full path=%s \n",GetMyName(),resultPtr->Ptr);
}

void ElementNode::RespondWithSelfAdd(QTSS_StreamRef inStream, QueryURI *queryPtr)
{
    static char *nullErr = "(null)";
    Bool16 nullData = false;
    QTSS_Error err = QTSS_NoErr;
    char messageBuffer[1024] = "";
    StrPtrLen bufferSPL(messageBuffer);
    
    //qtss_printf("ElementNode::RespondWithSelfAdd NODE = %s index = %"_S32BITARG_" \n",GetNodeName(), (SInt32) index);
    
    if (!fInitialized) 
    {   //qtss_printf("ElementNode::RespondWithSelfAdd not Initialized EXIT\n");
        return;
    }
    if (NULL == queryPtr) 
    {   //qtss_printf("ElementNode::RespondWithSelfAdd NULL == queryPtr EXIT\n");
        return;
    }
    
    if (NULL == inStream) 
    {   //qtss_printf("ElementNode::RespondWithSelfAdd NULL == inStream EXIT\n");
        return;
    }
    
    char *dataPtr = GetMyElementDataPtr();
    if (NULL == dataPtr) 
    {   //qtss_printf("ElementNode::RespondWithSelfAdd NULL == dataPtr EXIT\n");
        dataPtr = nullErr;
        nullData = true;
    }
    
    queryPtr->SetQueryHasResponse();    



#if CHECKACCESS
/*
    StrPtrLen *accessParamsPtr=queryPtr->GetAccess();
    if (accessParamsPtr == NULL)
    {
            UInt32 result = 400;
            qtss_sprintf(messageBuffer,  "Attribute Access is required");
            (void) queryPtr->EvalQuery(&result, messageBuffer);
            return;
    }   
    
    accessFlags = queryPtr->GetAccessFlags();   
    if (0 == (accessFlags & qtssAttrModeWrite)) 
    {
            UInt32 result = 400;
            qtss_sprintf(messageBuffer,  "Attribute must have write access");
            (void) queryPtr->EvalQuery(&result, messageBuffer);
            return;
    }   
*/
#endif


    StrPtrLen* valuePtr = queryPtr->GetValue();
    OSCharArrayDeleter value(NewCharArrayCopy(valuePtr));
    if (!valuePtr || !valuePtr->Ptr)
    {   UInt32 result = 400;
        qtss_sprintf(messageBuffer,  "Attribute value is required");
        (void) queryPtr->EvalQuery(&result, messageBuffer);
        return;
    }   


    StrPtrLen *typePtr = queryPtr->GetType();
    OSCharArrayDeleter dataType(NewCharArrayCopy(typePtr));
    if (!typePtr || !typePtr->Ptr)
    {   UInt32 result = 400;
        qtss_sprintf(messageBuffer,  "Attribute type is required");
        (void) queryPtr->EvalQuery(&result, messageBuffer);
        return;
    }   

    QTSS_AttrDataType attrDataType = qtssAttrDataTypeUnknown;
    if (typePtr && typePtr->Len > 0)
    {   
        err = QTSS_TypeStringToType(dataType.GetObject(), &attrDataType);
        Assert(err == QTSS_NoErr);  
        //qtss_printf("ElementNode::RespondWithSelfAdd theType=%s typeID=%"_U32BITARG_" \n",dataType.GetObject(), attrDataType);
    }

    //qtss_printf("ElementNode::RespondWithSelfAdd theValue= %s theType=%s typeID=%"_U32BITARG_" \n",value.GetObject(), typePtr->Ptr, attrDataType);
    char valueBuff[2048] = "";
    UInt32 len = 2048;
    err = QTSS_StringToValue(value.GetObject(),attrDataType, valueBuff, &len);
    if (err) 
    {   UInt32 result = 400;
        qtss_sprintf(messageBuffer,  "QTSS_Error=%"_S32BITARG_" from ElementNode::RespondWithSelfAdd QTSS_ConvertStringToType",err);
        (void) queryPtr->EvalQuery(&result, messageBuffer);
        return;
    }

    if (GetMyFieldType() != eNode)
    {   UInt32 result = 500;
        qtss_sprintf(messageBuffer,  "Internal error");
        (void) queryPtr->EvalQuery(&result, messageBuffer);
        return;
    }

    StrPtrLen *namePtr = queryPtr->GetName();
    OSCharArrayDeleter nameDeleter(NewCharArrayCopy(namePtr));
    if (!namePtr || !namePtr->Ptr || namePtr->Len == 0)
    {   UInt32 result = 400;
        qtss_sprintf(messageBuffer,  "Missing name for attribute");
        (void) queryPtr->EvalQuery(&result, messageBuffer);
        return;
    }   
    
    err = QTSS_AddInstanceAttribute(GetSource(),nameDeleter.GetObject(), NULL, attrDataType);
    //qtss_printf("QTSS_AddInstanceAttribute(source=%"_U32BITARG_", name=%s, NULL, %d, %"_U32BITARG_")\n",GetSource(),nameDeleter.GetObject(),attrDataType,accessFlags);
    if (err) 
    {   UInt32 result = 400;
        if (err == QTSS_AttrNameExists)
        {   qtss_sprintf(messageBuffer,  "The name %s already exists QTSS_Error=%"_S32BITARG_" from QTSS_AddInstanceAttribute",nameDeleter.GetObject(),err);
        }
        else
        {   qtss_sprintf(messageBuffer,  "QTSS_Error=%"_S32BITARG_" from QTSS_AddInstanceAttribute",err);
        }
        (void) queryPtr->EvalQuery(&result, messageBuffer);
        return;
    }
    QTSS_Object attrInfoObject;
    err = QTSS_GetAttrInfoByName(GetSource(), nameDeleter.GetObject(), &attrInfoObject);
    if (err) 
    {   UInt32 result = 400;
        qtss_sprintf(messageBuffer,  "QTSS_Error=%"_S32BITARG_" from QTSS_GetAttrInfoByName",err);
        (void) queryPtr->EvalQuery(&result, messageBuffer);
        return;
    }
    QTSS_AttributeID attributeID = 0;
    UInt32 attributeLen = sizeof(attributeID);
    err = QTSS_GetValue (attrInfoObject, qtssAttrID,0, &attributeID, &attributeLen);
    if (err) 
    {   UInt32 result = 400;
        qtss_sprintf(messageBuffer,  "QTSS_Error=%"_S32BITARG_" from QTSS_GetValue",err);
        (void) queryPtr->EvalQuery(&result, messageBuffer);
        return;
    }
    
    err = QTSS_SetValue (GetSource(), attributeID, 0, valueBuff, len);
    if (err) 
    {   UInt32 result = 400;
        qtss_sprintf(messageBuffer,  "QTSS_Error=%"_S32BITARG_" from QTSS_SetValue",err);
        (void) queryPtr->EvalQuery(&result, messageBuffer);
        return;
    }
    
}

void ElementNode::RespondWithSelf(QTSS_StreamRef inStream, QueryURI *queryPtr)
{
    //qtss_printf("ElementNode::RespondWithSelf = %s \n",GetNodeName());

    static char *nullErr = "(null)";
    if (QueryURI::kADDCommand == queryPtr->GetCommandID())
    {
        if (GetMyFieldType() == eArrayNode)
        {   RespondToAdd(inStream, 0,queryPtr);
        }
        else
        {   RespondWithSelfAdd(inStream,queryPtr);
        }
        
        return;
        
    }
    
    if (QueryURI::kDELCommand == queryPtr->GetCommandID())
    {   GetParentNode()->RespondToDel(inStream, GetMyKey(),queryPtr,true);
        return; 
    }
    
        
    if (GetNodeName() == NULL) 
    {   //qtss_printf("ElementNode::RespondWithSelf Node = %s is Uninitialized no name so LEAVE\n",GetNodeName() );
        return;
    }
    
    if (!fInitialized) 
    {   //qtss_printf("ElementNode::RespondWithSelf not Initialized EXIT\n");
        return;
    }
    
    if (NULL == queryPtr) 
    {   //qtss_printf("ElementNode::RespondWithSelf NULL == queryPtr EXIT\n");
        return;
    }

    if (queryPtr->fNumFilters > 0)
    {           
        Bool16 foundFilter = false;
        StrPtrLen*  theFilterPtr;
        for (SInt32 count = 0; count < queryPtr->fNumFilters; count ++)
        {
            theFilterPtr = queryPtr->GetFilter(count);
            if (theFilterPtr && theFilterPtr->Equal(StrPtrLen(GetMyName())) ) 
            {   foundFilter = true;
                //qtss_printf("ElementNode::RespondWithSelf found filter = ");PRINT_STR(theFilterPtr);
                break;
            }
        }
        if (!foundFilter) return;
    }
        
    StrPtrLen bufferSPL;

    UInt32 parameters = queryPtr->GetParamBits();
    parameters &= ~QueryURI::kRecurseParam; // clear recurse flag
    parameters &= ~QueryURI::kDebugParam; // clear verbose flag
    parameters &= ~QueryURI::kIndexParam; // clear index flag
    
    
    Bool16 isVerbosePath = 0 != (parameters & QueryURI::kVerboseParam);
    if (isVerbosePath) 
    {   
        parameters &= ~QueryURI::kVerboseParam; // clear verbose flag
        GetFullPath(&bufferSPL);
        (void)QTSS_Write(inStream, bufferSPL.Ptr, ::strlen(bufferSPL.Ptr), NULL, 0);
        //qtss_printf("ElementNode::RespondWithSelf Path=%s \n",bufferSPL.Ptr);
    }
    
    if (IsNodeElement())
    {   if (!isVerbosePath) // this node name already in path
        {
            (void)QTSS_Write(inStream, GetNodeName(), GetNodeNameLen(), NULL, 0);
            (void)QTSS_Write(inStream, "/", 1, NULL, 0);
            //qtss_printf("ElementNode::RespondWithSelf %s/ \n",GetNodeName());
        }
    }
    else
    {   //qtss_printf(" ****** ElementNode::RespondWithSelf NOT a node **** \n");
        (void)QTSS_Write(inStream, GetNodeName(), GetNodeNameLen(), NULL, 0);
        (void)QTSS_Write(inStream, "=", 1, NULL, 0);
        //qtss_printf(" %s=",GetNodeName());
            
        char *dataPtr = GetMyElementDataPtr();
        if (dataPtr == NULL)
        {
            (void)QTSS_Write(inStream, nullErr, ::strlen(nullErr), NULL, 0);
        }
        else
        {
            (void)QTSS_Write(inStream, dataPtr, ::strlen(dataPtr), NULL, 0);
        }
        //qtss_printf(" %s",buffer);
    
    }
    
    if (parameters)
    {   (void)QTSS_Write(inStream, sParameterDelimeter, 1, NULL, 0);
        //qtss_printf(" %s",sParameterDelimeter);
    }
    
    if (parameters & QueryURI::kAccessParam)
    {   
        (void)QTSS_Write(inStream, sAccess, 2, NULL, 0);
        //qtss_printf(" %s",sAccess);
        (void)QTSS_Write(inStream, GetMyAccessData(),  GetMyAccessLen(), NULL, 0);
        //qtss_printf("%s",GetMyAccessData());
        parameters &= ~QueryURI::kAccessParam; // clear access flag
        
        if (parameters)
        {   (void)QTSS_Write(inStream, sListDelimeter, 1, NULL, 0);
            //qtss_printf("%s",sListDelimeter);
        }
    }
    
    if (parameters & QueryURI::kTypeParam)
    {   
        (void)QTSS_Write(inStream, sType, 2, NULL, 0);
        //qtss_printf(" %s",sType);
        char *theTypeString = GetMyAPI_TypeStr();
        if (theTypeString == NULL)
        {
            (void)QTSS_Write(inStream, nullErr, ::strlen(nullErr), NULL, 0);
        }
        else
        {
            (void)QTSS_Write(inStream,theTypeString,strlen(theTypeString), NULL, 0);
            //qtss_printf("%s",theTypeString);
        }
        
        parameters &= ~QueryURI::kTypeParam; // clear type flag
        
        if (parameters)
        {   (void)QTSS_Write(inStream, sListDelimeter, 1, NULL, 0);
            //qtss_printf("%s",sListDelimeter);
        }
    }
    queryPtr->SetQueryHasResponse();
    (void)QTSS_Write(inStream, "\n", 1, NULL, 0);
    //qtss_printf("\n");
    
}

void    ElementNode::RespondToAdd(QTSS_StreamRef inStream, SInt32 index,QueryURI *queryPtr)
{
    char messageBuffer[1024] = "";

    //qtss_printf("ElementNode::RespondToAdd NODE = %s index = %"_S32BITARG_" \n",GetNodeName(), (SInt32) index);
    if (GetNumFields() == 0) 
    {
        UInt32 result = 405;
        qtss_sprintf(messageBuffer,  "Attribute does not allow adding. Action not allowed");
        (void) queryPtr->EvalQuery(&result, messageBuffer);
        //qtss_printf("ElementNode::RespondToAdd error = %s \n",messageBuffer);
        return;
    }   
    
    if (GetFieldType(index) == eNode)
    {   RespondWithSelfAdd(inStream, queryPtr);
        return;
    }
        
    static char *nullErr = "(null)";
    Bool16 nullData = false;
    QTSS_Error err = QTSS_NoErr;
    StrPtrLen bufferSPL(messageBuffer);
    
    
    if (!fInitialized) 
    {   //qtss_printf("ElementNode::RespondToAdd not Initialized EXIT\n");
        return;
    }
    if (NULL == queryPtr) 
    {   //qtss_printf("ElementNode::RespondToAdd NULL == queryPtr EXIT\n");
        return;
    }
    
    if (NULL == inStream) 
    {   //qtss_printf("ElementNode::RespondToAdd NULL == inStream EXIT\n");
        return;
    }
    
    char *dataPtr = GetElementDataPtr(index);
    if (NULL == dataPtr) 
    {   //qtss_printf("ElementNode::RespondToAdd NULL == dataPtr EXIT\n");
        //  return;
        dataPtr = nullErr;
        nullData = true;
    }
    
    queryPtr->SetQueryHasResponse();    

    
    UInt32 accessFlags = 0;
    StrPtrLen *accessParamsPtr=queryPtr->GetAccess();
    if (accessParamsPtr != NULL)
        accessFlags = queryPtr->GetAccessFlags();
    else
        accessFlags = GetAccessPermissions(index);
    
    
    
    StrPtrLen* valuePtr = queryPtr->GetValue();
    OSCharArrayDeleter value(NewCharArrayCopy(valuePtr));
    if (!valuePtr || !valuePtr->Ptr)
    {   UInt32 result = 400;
        qtss_sprintf(messageBuffer,  "No value found");
        (void) queryPtr->EvalQuery(&result, messageBuffer);
        return;
    }   

    //qtss_printf("ElementNode::RespondToAdd theValue= %s theType=%s typeID=%"_U32BITARG_" \n",value.GetObject(), GetAPI_TypeStr(index), GetAPI_Type(index));
    char valueBuff[2048] = "";
    UInt32 len = 2048;
    err = QTSS_StringToValue(value.GetObject(),GetAPI_Type(index), valueBuff, &len);
    if (err) 
    {   UInt32 result = 400;
        qtss_sprintf(messageBuffer,  "QTSS_Error=%"_S32BITARG_" from QTSS_ConvertStringToType",err);
        (void) queryPtr->EvalQuery(&result, messageBuffer);
        return;
    }

    if (GetFieldType(index) != eNode)
    {   
        OSCharArrayDeleter typeDeleter(NewCharArrayCopy(queryPtr->GetType()));
        StrPtrLen theQueryType(typeDeleter.GetObject());

        if (typeDeleter.GetObject())
        {
            StrPtrLen attributeString(GetAPI_TypeStr(index));
            if (!attributeString.Equal(theQueryType))
            {   UInt32 result = 400;
                qtss_sprintf(messageBuffer,  "Type %s does not match attribute type %s",typeDeleter.GetObject(), attributeString.Ptr);
                (void) queryPtr->EvalQuery(&result, messageBuffer);
                return;
            }   
        }
        
        QTSS_Object source = GetSource();

        UInt32 tempBuff;
        UInt32 attributeLen = sizeof(tempBuff);
        (void) QTSS_GetValue (source, GetAPI_ID(index), 0, &tempBuff, &attributeLen);
        if (attributeLen != len)
        {   UInt32 result = 400;
            qtss_sprintf(messageBuffer,  "Data length %"_U32BITARG_" does not match attribute len %"_U32BITARG_"",len, attributeLen);
            (void) queryPtr->EvalQuery(&result, messageBuffer);
            return;
        }

        
        UInt32 numValues = 0;
        err = QTSS_GetNumValues (source,  GetAPI_ID(index), &numValues);
        if (err) 
        {   UInt32 result = 400;
            qtss_sprintf(messageBuffer,  "QTSS_Error=%"_S32BITARG_" from QTSS_GetNumValues",err);
            (void) queryPtr->EvalQuery(&result, messageBuffer);
            return;
        }
        
        //qtss_printf("ElementNode::RespondToAdd QTSS_SetValue object=%"_U32BITARG_" attrID=%"_U32BITARG_", index = %"_U32BITARG_" valuePtr=%"_U32BITARG_" valuelen =%"_U32BITARG_" \n",GetSource(),GetAPI_ID(index), GetAttributeIndex(index), valueBuff,len);
        err = QTSS_SetValue (source, GetAPI_ID(index), numValues, valueBuff, len);
        if (err) 
        {   UInt32 result = 400;
            qtss_sprintf(messageBuffer,  "QTSS_Error=%"_S32BITARG_" from QTSS_SetValue",err);
            (void) queryPtr->EvalQuery(&result, messageBuffer);
            return;
        }

    }
    
}

void    ElementNode::RespondToSet(QTSS_StreamRef inStream, SInt32 index,QueryURI *queryPtr)
{
    static char *nullErr = "(null)";
    Bool16 nullData = false;
    QTSS_Error err = QTSS_NoErr;
    char messageBuffer[1024] = "";
    StrPtrLen bufferSPL(messageBuffer);
    
    //qtss_printf("ElementNode::RespondToSet NODE = %s index = %"_S32BITARG_" \n",GetNodeName(), (SInt32) index);
    
    if (!fInitialized) 
    {   //qtss_printf("ElementNode::RespondToSet not Initialized EXIT\n");
        return;
    }
    if (NULL == queryPtr) 
    {   //qtss_printf("ElementNode::RespondToSet NULL == queryPtr EXIT\n");
        return;
    }
    
    if (NULL == inStream) 
    {   //qtss_printf("ElementNode::RespondToSet NULL == inStream EXIT\n");
        return;
    }
    
    char *dataPtr = GetElementDataPtr(index);
    if (NULL == dataPtr) 
    {   //qtss_printf("ElementNode::RespondToSet NULL == dataPtr EXIT\n");
        //  return;
        dataPtr = nullErr;
        nullData = true;
    }
    
    queryPtr->SetQueryHasResponse();    

    OSCharArrayDeleter typeDeleter(NewCharArrayCopy(queryPtr->GetType()));
    StrPtrLen theQueryType(typeDeleter.GetObject());

    if (theQueryType.Len > 0)
    {   StrPtrLen attributeString(GetAPI_TypeStr(index));
        if (!attributeString.Equal(theQueryType))
        {   UInt32 result = 400;
            qtss_sprintf(messageBuffer,  "Type %s does not match attribute type %s",typeDeleter.GetObject(), attributeString.Ptr);
            (void) queryPtr->EvalQuery(&result, messageBuffer);
            return;
        }   
    }

    if (0 == (GetAccessPermissions(index) & qtssAttrModeWrite)) 
    {
        UInt32 result = 400;
        qtss_sprintf(messageBuffer,  "Attribute is read only. Action not allowed");
        (void) queryPtr->EvalQuery(&result, messageBuffer);
        return;
    }

    if (GetFieldType(index) == eNode)
    {   
        UInt32 result = 400;
        qtss_sprintf(messageBuffer,  "Set of type %s not allowed",typeDeleter.GetObject());
        //qtss_printf("ElementNode::RespondToSet (GetFieldType(index) == eNode) %s\n",messageBuffer);
        (void) queryPtr->EvalQuery(&result, messageBuffer);
        return;
    }
    else do 
    {   
        
        StrPtrLen* valuePtr = queryPtr->GetValue();
        if (!valuePtr || !valuePtr->Ptr) break;
    
        char valueBuff[2048] = "";
        UInt32 len = 2048;
        OSCharArrayDeleter value(NewCharArrayCopy(valuePtr));
        
        //qtss_printf("ElementNode::RespondToSet valuePtr->Ptr= %s\n",value.GetObject());
        
        err = QTSS_StringToValue(value.GetObject(),GetAPI_Type(index), valueBuff, &len);
        if (err) 
        {   //qtss_sprintf(messageBuffer,  "QTSS_Error=%"_S32BITARG_" from QTSS_ConvertStringToType",err);
            break;
        }
        
        //qtss_printf("ElementNode::RespondToSet QTSS_SetValue object=%"_U32BITARG_" attrID=%"_U32BITARG_", index = %"_U32BITARG_" valuePtr=%"_U32BITARG_" valuelen =%"_U32BITARG_" \n",GetSource(),GetAPI_ID(index), GetAttributeIndex(index), valueBuff,len);
        err = QTSS_SetValue (GetSource(), GetAPI_ID(index), GetAttributeIndex(index), valueBuff, len);
        if (err) 
        {   //qtss_sprintf(messageBuffer,  "QTSS_Error=%"_S32BITARG_" from QTSS_SetValue",err);
            break;
        }   
            
    } while (false);
    
    if (err != QTSS_NoErr)
    {   UInt32 result = 400;
        (void) queryPtr->EvalQuery(&result, messageBuffer);
        //qtss_printf("ElementNode::RespondToSet %s len = %"_U32BITARG_" ",messageBuffer, result);
        return;
    }
    
}

void    ElementNode::RespondToDel(QTSS_StreamRef inStream, SInt32 index,QueryURI *queryPtr,Bool16 delAttribute)
{
    static char *nullErr = "(null)";
    Bool16 nullData = false;
    QTSS_Error err = QTSS_NoErr;
    char messageBuffer[1024] = "";
    StrPtrLen bufferSPL(messageBuffer);
    
    //qtss_printf("ElementNode::RespondToDel NODE = %s index = %"_S32BITARG_" \n",GetNodeName(), (SInt32) index);
    
    if (!fInitialized) 
    {   //qtss_printf("ElementNode::RespondToDel not Initialized EXIT\n");
        return;
    }
    if (NULL == queryPtr) 
    {   //qtss_printf("ElementNode::RespondToDel NULL == queryPtr EXIT\n");
        return;
    }
    
    if (NULL == inStream) 
    {   //qtss_printf("ElementNode::RespondToDel NULL == inStream EXIT\n");
        return;
    }

    //qtss_printf("ElementNode::RespondToDel NODE = %s index = %"_S32BITARG_" \n",GetNodeName(), (SInt32) index);
    if (    GetNumFields() == 0 
        || ( 0 == (GetAccessPermissions(index) & qtssAttrModeDelete) && GetMyFieldType() == eArrayNode && GetNumFields() == 1)  
        || ( 0 == (GetAccessPermissions(index) & qtssAttrModeDelete) && GetMyFieldType() != eArrayNode)  
        ) 
    {
        UInt32 result = 405;
        qtss_sprintf(messageBuffer,  "Attribute does not allow deleting. Action not allowed");
        (void) queryPtr->EvalQuery(&result, messageBuffer);
        //qtss_printf("ElementNode::RespondToDel error = %s \n",messageBuffer);
        return;
    }      
    
    char *dataPtr = GetElementDataPtr(index);
    if (NULL == dataPtr) 
    {   //qtss_printf("ElementNode::RespondToDel NULL == dataPtr EXIT\n");
        //  return;
        dataPtr = nullErr;
        nullData = true;
    }
    
    queryPtr->SetQueryHasResponse();    

    // DMS - Removeable is no longer a permission bit
    //
    //if (GetMyFieldType() != eArrayNode && 0 == (GetAccessPermissions(index) & qtssAttrModeRemoveable)) 
    //{
    //  UInt32 result = 405;
    //  qtss_sprintf(messageBuffer,  "Attribute is not removable. Action not allowed");
    //  (void) queryPtr->EvalQuery(&result, messageBuffer);
    //  return;
    //} 
    
    if (GetMyFieldType() == eArrayNode && !delAttribute)
    {   
        UInt32 result = 500;
        err = QTSS_RemoveValue (GetSource(),GetAPI_ID(index), GetAttributeIndex(index));
        qtss_sprintf(messageBuffer,  "QTSS_Error=%"_S32BITARG_" from QTSS_RemoveValue", err);
        //qtss_printf("ElementNode::RespondToDel QTSS_RemoveValue object=%"_U32BITARG_" attrID=%"_U32BITARG_" index=%"_U32BITARG_" %s\n",GetSource(),GetAPI_ID(index),GetAttributeIndex(index),messageBuffer);
        if (err) 
        {   (void) queryPtr->EvalQuery(&result, messageBuffer);
        }   
    }
    else  
    {   
        //qtss_printf("ElementNode::RespondToDel QTSS_RemoveInstanceAttribute object=%"_U32BITARG_" attrID=%"_U32BITARG_" \n",GetSource(),GetAPI_ID(index));
        err = QTSS_RemoveInstanceAttribute(GetSource(),GetAPI_ID(index));       
        if (err) 
        {
            qtss_sprintf(messageBuffer,  "QTSS_Error=%"_S32BITARG_" from QTSS_RemoveInstanceAttribute",err);
        }   
            
    } 
    
    if (err != QTSS_NoErr)
    {   UInt32 result = 400;
        (void) queryPtr->EvalQuery(&result, messageBuffer);
        //qtss_printf("ElementNode::RespondToDel %s len = %"_U32BITARG_" ",messageBuffer, result);
        return;
    }
    
}

Bool16 ElementNode::IsFiltered(SInt32 index,QueryURI *queryPtr)
{
    Bool16 foundFilter = false;
    StrPtrLen*  theFilterPtr;
    for (SInt32 count = 0; count < queryPtr->fNumFilters; count ++)
    {
        theFilterPtr = queryPtr->GetFilter(count);
        if (theFilterPtr && theFilterPtr->Equal(StrPtrLen(GetName(index))) ) 
        {   foundFilter = true;
            break;
        }
    }
    return foundFilter;
}
    
void ElementNode::RespondToGet(QTSS_StreamRef inStream, SInt32 index,QueryURI *queryPtr)
{
    static char *nullErr = "(null)";
    Bool16 nullData = false;
    
    //qtss_printf("ElementNode::RespondToGet NODE = %s index = %"_S32BITARG_" \n",GetNodeName(), (SInt32) index);
    
    if (!fInitialized) 
    {   //qtss_printf("ElementNode::RespondToGet not Initialized EXIT\n");
        return;
    }
    if (NULL == queryPtr) 
    {   //qtss_printf("ElementNode::RespondToGet NULL == queryPtr EXIT\n");
        return;
    }
    
    if (NULL == inStream) 
    {   //qtss_printf("ElementNode::RespondToGet NULL == inStream EXIT\n");
        return;
    }
    
    char *dataPtr = GetElementDataPtr(index);
    if (NULL == dataPtr) 
    {   //qtss_printf("ElementNode::RespondToGet NULL == dataPtr EXIT\n");
        //  return;
        dataPtr = nullErr;
        nullData = true;
    }
    
    StrPtrLen bufferSPL;
    
    UInt32 parameters = queryPtr->GetParamBits();
    parameters &= ~QueryURI::kRecurseParam; // clear verbose flag
    parameters &= ~QueryURI::kDebugParam; // clear debug flag
    parameters &= ~QueryURI::kIndexParam; // clear index flag
    
    //qtss_printf("ElementNode::RespondToGet QTSS_SetValue object=%"_U32BITARG_" attrID=%"_U32BITARG_", index = %"_U32BITARG_" \n",GetSource(),GetAPI_ID(index), GetAttributeIndex(index));


    if ((parameters & QueryURI::kVerboseParam) ) 
    {   
        parameters &= ~QueryURI::kVerboseParam; // clear verbose flag
        GetFullPath(&bufferSPL);
        (void)QTSS_Write(inStream, bufferSPL.Ptr, ::strlen(bufferSPL.Ptr), NULL, 0);
        //qtss_printf("ElementNode::RespondToGet Path=%s \n",bufferSPL.Ptr);
    }
    

    (void)QTSS_Write(inStream, GetName(index), GetNameLen(index), NULL, 0);
    //qtss_printf("ElementNode::RespondToGet %s:len = %"_U32BITARG_"",GetName(index),(UInt32) GetNameLen(index));
        
    if (IsNodeElement(index))
    {
        (void)QTSS_Write(inStream, "/\"", 1, NULL, 0);
        //qtss_printf(" %s/\"",GetNodeName());
    }
    else
    {
        if (nullData)
        {
            (void)QTSS_Write(inStream, "=", 1, NULL, 0);
            (void)QTSS_Write(inStream, dataPtr, ::strlen(dataPtr), NULL, 0);
        }
        else
        {
            (void)QTSS_Write(inStream, "=\"", 2, NULL, 0);
            (void)QTSS_Write(inStream, dataPtr, ::strlen(dataPtr), NULL, 0);
            (void)QTSS_Write(inStream, "\"", 1, NULL, 0);
        }
    }
    
    //qtss_printf(" %s len = %d ",buffer, ::strlen(buffer));
    //DebugShowFieldDataType(index);
    
    if (parameters)
    {   (void)QTSS_Write(inStream, sParameterDelimeter, 1, NULL, 0);
        //qtss_printf(" %s",sParameterDelimeter);
    }
    
    if (parameters & QueryURI::kAccessParam)
    {   
        (void)QTSS_Write(inStream, sAccess, 2, NULL, 0);
        //qtss_printf(" %s",sAccess);
        (void)QTSS_Write(inStream, GetAccessData(index),  GetAccessLen(index), NULL, 0);
        //qtss_printf("%s",GetAccessData(index));
        parameters &= ~QueryURI::kAccessParam; // clear access flag
        
        if (parameters)
        {   (void)QTSS_Write(inStream, sListDelimeter, 1, NULL, 0);
            //qtss_printf("%s",sListDelimeter);
        }
    }
    
    if (parameters & QueryURI::kTypeParam)
    {   
        (void)QTSS_Write(inStream, sType, 2, NULL, 0);
        //qtss_printf(" %s",sType);
        char* typeStringPtr = GetAPI_TypeStr(index);
        if (typeStringPtr == NULL)
        {
            //qtss_printf("ElementNode::RespondToGet typeStringPtr is NULL for type = %s \n", typeStringPtr);
            (void)QTSS_Write(inStream, nullErr, ::strlen(nullErr), NULL, 0);
        }
        else
        {
            //qtss_printf("ElementNode::RespondToGet type = %s \n", typeStringPtr);
            (void)QTSS_Write(inStream,typeStringPtr,strlen(typeStringPtr), NULL, 0);
        }

        parameters &= ~QueryURI::kTypeParam; // clear type flag
        
        if (parameters)
        {   (void)QTSS_Write(inStream, sListDelimeter, 1, NULL, 0);
            //qtss_printf("%s",sListDelimeter);
        }
    }
    
    
    (void)QTSS_Write(inStream, "\n", 1, NULL, 0);
    //qtss_printf(" %s","\n");
    
    queryPtr->SetQueryHasResponse();

}

void    ElementNode::RespondToKey(QTSS_StreamRef inStream, SInt32 index,QueryURI *queryPtr)
{
    SInt32 command = queryPtr->GetCommandID();
    //qtss_printf("ElementNode::RespondToKey command = %"_S32BITARG_" node =%s index=%"_S32BITARG_"\n",command, GetNodeName(),index);
    
    switch (command)
    {
        case QueryURI::kGETCommand: RespondToGet(inStream,index,queryPtr);
        break;
        
        case QueryURI::kSETCommand: RespondToSet(inStream,index,queryPtr);
        break;
        
        case QueryURI::kADDCommand: RespondToAdd(inStream,index,queryPtr);
        break;
        
        case QueryURI::kDELCommand: RespondToDel(inStream,index,queryPtr,false);
        break;
    }
        
}

void ElementNode::RespondWithNodeName(QTSS_StreamRef inStream, QueryURI * /*unused queryPtr*/) 
{
    
    //qtss_printf("ElementNode::RespondWithNodeName NODE = %s \n",GetNodeName());
    
    if (!fInitialized) 
    {    //qtss_printf("ElementNode::RespondWithNodeName not Initialized EXIT\n");
         return;
    }
        
    StrPtrLen fullPathSPL;
    GetFullPath(&fullPathSPL);
    
    (void)QTSS_Write(inStream, "Container=\"/", ::strlen("Container=\"/"), NULL, 0);
    
    (void)QTSS_Write(inStream, fPathSPL.Ptr, ::strlen(fPathSPL.Ptr), NULL, 0);
    //qtss_printf("ElementNode::RespondWithNodeName Path=%s \n",fPathSPL.Ptr);
    
    (void)QTSS_Write(inStream, "\"", 1, NULL, 0);
    //qtss_printf("\"");     
    
    (void)QTSS_Write(inStream, "\n", 1, NULL, 0);
    //qtss_printf("\n");
    
}
    
void ElementNode::RespondWithSingleElement(QTSS_StreamRef inStream, QueryURI *queryPtr, StrPtrLen *currentSegmentPtr) 
{
        
    //qtss_printf("ElementNode::RespondWithSingleElement Current Node = %s\n",GetNodeName() );
    
    if (!fInitialized) 
    {   
        //qtss_printf("ElementNode::RespondWithSingleElement failed Not Initialized %s\n",GetNodeName() );
        return;
    }

    if (GetNodeName() == NULL) 
    {   
        //qtss_printf("ElementNode::RespondWithSingleElement Node = %s is Uninitialized LEAVE\n",GetNodeName() );
        return;
    }

    Assert(queryPtr != NULL);
    Assert(currentSegmentPtr != NULL);
    Assert(currentSegmentPtr->Ptr != 0);
    Assert(currentSegmentPtr->Len != 0);
    
    SInt32 key = ResolveSPLKeyToIndex(currentSegmentPtr);
    //qtss_printf("ElementNode::RespondWithSingleElement key = %"_S32BITARG_"\n",key);
    //qtss_printf("currentSegmentPtr="); PRINT_STR(currentSegmentPtr);
    
    if (key < 0) 
    {
        //qtss_printf("ElementNode::RespondWithSingleElement key = %"_S32BITARG_" NOT FOUND no ELEMENT\n",key);
        return;
    }
    
    if ((queryPtr == NULL) || (currentSegmentPtr == NULL) || (currentSegmentPtr->Ptr == NULL) || (currentSegmentPtr->Len == 0))
    {
        //qtss_printf("ElementNode::RespondWithSingleElement currentSegmentPtr || queryPtr = NULL\n");
        return;
    }
    
    //add here
    
    StrPtrLen nextSegment;
    ( void)queryPtr->NextSegment(currentSegmentPtr, &nextSegment);

    if ( (nextSegment.Len == 0)  && !queryPtr->RecurseParam() ) // only respond if we are at the end of the path
    {
        //qtss_printf("currentSegmentPtr="); PRINT_STR(currentSegmentPtr);
        //qtss_printf("nextSegment="); PRINT_STR(&nextSegment);
        //qtss_printf("ElementNode::RespondWithSingleElement Current Node = %s Call RespondWithNodeName\n",GetNodeName() );
        if (QueryURI::kGETCommand == queryPtr->GetCommandID())
            RespondWithNodeName( inStream,queryPtr);
    }
            
    if (IsNodeElement(key))
    {
        ElementNode *theNodePtr = (ElementNode *)GetElementDataPtr(key);
        if (theNodePtr) 
        {   
            //qtss_printf("ElementNode::RespondWithSingleElement Current Node = %s Call RespondToQuery\n",GetNodeName() );
            theNodePtr->RespondToQuery(inStream, queryPtr,currentSegmentPtr);
        }
    }
    else
    {
        //qtss_printf("ElementNode::RespondWithSingleElement call RespondToKey\n");
        if ( (queryPtr->fNumFilters > 0) && (QueryURI::kGETCommand == queryPtr->GetCommandID()) )
        {
            StrPtrLen*  theFilterPtr;
            SInt32 index;
            for (SInt32 count = 0; count < queryPtr->fNumFilters; count ++)
            {
                theFilterPtr = queryPtr->GetFilter(count);
                index = ResolveSPLKeyToIndex(theFilterPtr);
                if (index < 0) continue;
                RespondToKey(inStream, index, queryPtr);
                //qtss_printf("ElementNode::RespondWithSingleElement found filter = ");PRINT_STR(theFilterPtr);
                break;
            }
            //qtss_printf("ElementNode::RespondWithSingleElement found filter = ?");PRINT_STR(theFilterPtr);
        }
        else
        {   RespondToKey(inStream, key, queryPtr);
        }
    }
    
}


void ElementNode::RespondWithAllElements(QTSS_StreamRef inStream, QueryURI *queryPtr, StrPtrLen *currentSegmentPtr)
{
    //qtss_printf("ElementNode::RespondWithAllElements %s\n",GetNodeName());
    //qtss_printf("ElementNode::RespondWithAllElements fDataFieldsStop = %d \n",fDataFieldsStop);
    
    if (GetNodeName() == NULL) 
    {   //qtss_printf("ElementNode::RespondWithAllElements %s is Uninitialized LEAVE\n",GetNodeName());
        return;
    }

    if (!fInitialized) 
    {   //qtss_printf("ElementNode::RespondWithAllElements %s is Uninitialized LEAVE\n",GetNodeName());
        return;
    }
    
    StrPtrLen nextSegment;
    ( void)queryPtr->NextSegment(currentSegmentPtr, &nextSegment);

    if ( (nextSegment.Len == 0 || nextSegment.Ptr == 0)  )  // only respond if we are at the end of the path
        if (QueryURI::kGETCommand == queryPtr->GetCommandID())
            RespondWithNodeName( inStream,queryPtr);
        
    if ( (queryPtr->fNumFilters > 0) && (QueryURI::kGETCommand == queryPtr->GetCommandID()) )
    {
        StrPtrLen*  theFilterPtr;
        SInt32 index;
        for (SInt32 count = 0; count < queryPtr->fNumFilters; count ++)
        {
            theFilterPtr = queryPtr->GetFilter(count);
            index = ResolveSPLKeyToIndex(theFilterPtr);
            if (index < 0) continue;
                        
            if ( (nextSegment.Len == 0 || nextSegment.Ptr == 0) )
            {   if ( (!IsNodeElement(index)) || (IsNodeElement(index) && queryPtr->RecurseParam() ))    // only respond if we are at the end of the path 
                    RespondToKey(inStream, index, queryPtr);    
            }

            //qtss_printf("ElementNode::RespondWithAllElements found filter = ");PRINT_STR(theFilterPtr);
        }
    }
    else
    {
        UInt32 index = 0;
        for ( index = 0; !IsStopItem(index) ;index++)
        {
            //qtss_printf("RespondWithAllElements = %d \n",index);
            //qtss_printf("ElementNode::RespondWithAllElements nextSegment="); PRINT_STR(&nextSegment);
            
            if ( (nextSegment.Len == 0 || nextSegment.Ptr == 0) )
            {   if ( (!IsNodeElement(index)) || (IsNodeElement(index) && queryPtr->RecurseParam() ) )   // only respond if we are at the end of the path
                    RespondToKey(inStream, index, queryPtr);    
            }
    
        }
    }   
    
    UInt32 index = 0;
    for ( index = 0; !IsStopItem(index);index++)
    {
        
        if  (IsNodeElement(index)) 
        {   
            //qtss_printf("ElementNode::RespondWithAllElements FoundNode\n");
            //qtss_printf("ElementNode::RespondWithAllElements currentSegmentPtr="); PRINT_STR(currentSegmentPtr);
            //qtss_printf("ElementNode::RespondWithAllElements nextSegment="); PRINT_STR(&nextSegment);
            ElementNode *theNodePtr = (ElementNode *)GetElementDataPtr(index);
            
            if (theNodePtr ) 
            {   
                //qtss_printf("ElementNode::RespondWithAllElements Current Node = %s Call RespondToQuery\n",GetNodeName() );
                theNodePtr->RespondToQuery(inStream, queryPtr,&nextSegment);
            }
            else
            {
                //qtss_printf("ElementNode::RespondWithAllElements Current Node index= %"_U32BITARG_" NULL = %s\n",index,GetName(index));
            
            }           
        }
    }
}



void ElementNode::RespondWithAllNodes(QTSS_StreamRef inStream, QueryURI *queryPtr, StrPtrLen *currentSegmentPtr) 
{
    
    //qtss_printf("ElementNode::RespondWithAllNodes %s\n",GetNodeName());

    if (!fInitialized) 
    {   //qtss_printf("ElementNode::RespondWithAllNodes %s is Uninitialized LEAVE\n",GetNodeName());
        return;
    }
    
    if (GetNodeName() == NULL) 
    {   //qtss_printf("ElementNode::RespondWithAllNodes %s is Uninitialized LEAVE\n",GetNodeName());
        return;
    }
    
    StrPtrLen nextSegment;
    ( void)queryPtr->NextSegment(currentSegmentPtr, &nextSegment);  
    
    for(SInt32 index = 0; !IsStopItem(index); index ++)
    {
        if (!queryPtr->RecurseParam() && (currentSegmentPtr->Len == 0) ) 
        {   Assert(0);
            break;
        }
        

        if  (IsNodeElement(index)) 
        {   
            //qtss_printf("ElementNode::RespondWithAllNodes FoundNode\n");
            //qtss_printf("ElementNode::RespondWithAllNodes currentSegmentPtr="); PRINT_STR(currentSegmentPtr);
            ElementNode *theNodePtr = (ElementNode *)GetElementDataPtr(index);
            //qtss_printf("ElementNode::RespondWithAllNodes nextSegment="); PRINT_STR(&nextSegment);
            if (theNodePtr) 
            {   
                //qtss_printf("ElementNode::RespondWithAllNodes Current Node = %s Call RespondToQuery\n",GetNodeName() );
                theNodePtr->RespondToQuery(inStream, queryPtr,currentSegmentPtr);           
            }
        }
    }
}

void ElementNode::RespondToQuery(QTSS_StreamRef inStream, QueryURI *queryPtr,StrPtrLen *currentPathPtr)
{
    //qtss_printf("----- ElementNode::RespondToQuery ------\n");
    //qtss_printf("ElementNode::RespondToQuery NODE = %s\n",GetNodeName());

    Assert(NULL != queryPtr);
    Assert(NULL != currentPathPtr);

    if (!fInitialized) 
    {   //qtss_printf("ElementNode::RespondToQuery %s is Uninitialized LEAVE\n",GetNodeName());
        return;
    
    }
        
    if (GetNodeName() == NULL) 
    {   //qtss_printf("ElementNode::RespondToQuery %s is Uninitialized LEAVE\n",GetNodeName());
        return;
    }
        
    
    Bool16 recurse = queryPtr->RecurseParam() ;
    Bool16 doAllNext = false;
    Bool16 doAllNextNext = false;
    StrPtrLen nextSegment;
    StrPtrLen nextnextSegment;
    StrPtrLen nextnextnextSegment;
    
    
    if (queryPtr && currentPathPtr) do
    {   
        ( void)queryPtr->NextSegment(currentPathPtr, &nextSegment);
        ( void)queryPtr->NextSegment(&nextSegment, &nextnextSegment);
        ( void)queryPtr->NextSegment(&nextnextSegment, &nextnextnextSegment);
            
        //qtss_printf("ElementNode::RespondToQuery currentPathPtr="); PRINT_STR( currentPathPtr);
        //qtss_printf("ElementNode::RespondToQuery nextSegment="); PRINT_STR( &nextSegment);
        //qtss_printf("ElementNode::RespondToQuery nextnextSegment="); PRINT_STR( &nextnextSegment);

         // recurse param is set and this is the end of the path
        if  (recurse && ( (0 == currentPathPtr->Len) || (0 == nextSegment.Len) ) )
        {   // admin 
            //qtss_printf("ElementNode::RespondToQuery 1)RespondToQuery -> RespondWithAllElements ") ;PRINT_STR( GetNodeNameSPL());
            RespondWithAllElements(inStream, queryPtr, &nextSegment); 
            break;                          
        }

         // recurse param is not set and this is the end of the path
        if  ( (!recurse && ( (0 == currentPathPtr->Len) || (0 == nextSegment.Len) ) )
            )
        {   // admin 
            //qtss_printf("ElementNode::RespondToQuery 2)RespondToQuery -> RespondWithSelf ") ;PRINT_STR( GetNodeNameSPL());
            if (fIsTop)
                (void)QTSS_Write(inStream, "Container=\"/\"\n", ::strlen("Container=\"/\"\n"), NULL, 0);

            RespondWithSelf(inStream, queryPtr);
            break;                          
        }


        doAllNext = ElementNode_DoAll(&nextSegment);    
        doAllNextNext = ElementNode_DoAll(&nextnextSegment);    
        
        if  (   doAllNext && (0 == nextnextSegment.Len) )
        {   // admin/*
            //qtss_printf("ElementNode::RespondToQuery 3)RespondToQuery -> RespondWithAllElements ");PRINT_STR( &nextSegment);
            RespondWithAllElements(inStream, queryPtr, &nextSegment);       
            break;                          
        }
        
        if  (   doAllNext && doAllNextNext) 
        {   // admin/*/*
            //qtss_printf("ElementNode::RespondToQuery 4)RespondToQuery -> RespondWithAllNodes ");PRINT_STR( currentPathPtr);
            RespondWithAllNodes(inStream, queryPtr, &nextSegment); 
            break;                          
        }
                

        if  (   doAllNext && (nextnextSegment.Len > 0) )
        {   // admin/*/attribute
            //qtss_printf("ElementNode::RespondToQuery 5)RespondToQuery -> RespondWithAllNodes  ");PRINT_STR( &nextSegment);
            RespondWithAllNodes(inStream, queryPtr, &nextSegment); 
            break;                          
        }
        
        // admin/attribute
        //qtss_printf("ElementNode::RespondToQuery 6)RespondToQuery -> RespondWithSingleElement ");PRINT_STR( &nextSegment);
        RespondWithSingleElement(inStream, queryPtr,&nextSegment);

    } while (false);

    if (QueryURI::kGETCommand != queryPtr->GetCommandID() && (!queryPtr->fIsPref))
    {   queryPtr->fIsPref = IsPreferenceContainer(GetMyName(),NULL);
    }
    //qtss_printf("ElementNode::RespondToQuery LEAVE\n");
}


void ElementNode::SetupNodes(QueryURI *queryPtr,StrPtrLen *currentPathPtr,QTSS_Initialize_Params *initParams)
{
    //qtss_printf("----- ElementNode::SetupNodes ------ NODE = %s\n", GetNodeName()); 
    //qtss_printf("ElementNode::SetupNodes currentPathPtr ="); PRINT_STR(currentPathPtr);
    if (fSelfPtr == NULL) 
    {   //qtss_printf("******* ElementNode::SetupNodes (fSelfPtr == NULLL) \n");
    }
    Assert(NULL != queryPtr);
    Assert(NULL != currentPathPtr);
    
    if (queryPtr && currentPathPtr) do
    {   
        Bool16 doAll = false;
        StrPtrLen nextSegment;

        ( void)queryPtr->NextSegment(currentPathPtr, &nextSegment);
        doAll = ElementNode_DoAll(&nextSegment);
        
        StrPtrLen *thisNamePtr = GetNodeNameSPL();
        //qtss_printf("ElementNode::SetupNodes thisNamePtr="); PRINT_STR(thisNamePtr);

        if (    ( (doAll) && (currentPathPtr->Equal(*thisNamePtr) || ElementNode_DoAll(currentPathPtr)) )
             || (queryPtr->RecurseParam())
            )
        {
            SetUpAllElements(queryPtr,currentPathPtr, &nextSegment, initParams);
            break;
        }
                 
        SInt32 index = ResolveSPLKeyToIndex(&nextSegment);
        if (index < 0) 
        {   
            //qtss_printf("ElementNode::SetupNodes FAILURE ResolveSPLKeyToIndex = %d NODE = %s\n", index, GetNodeName());
            break;
        }

        SetUpAllNodes(queryPtr, currentPathPtr, &nextSegment, initParams); 
    
        if (NULL == GetElementDataPtr(index))
        {   //qtss_printf("ElementNode::SetupNodes call SetUpSingleElement index=%"_U32BITARG_" nextSegment=");PRINT_STR( &nextSegment);
            SetUpSingleElement(queryPtr,currentPathPtr, &nextSegment, index, initParams);
        }
        
        
    } while (false);

}

void ElementNode::GetFilteredAttributeName(ElementDataFields* fieldPtr, QTSS_AttributeID theID)
{   
    fieldPtr->fFieldLen = 0;
    char *theName = NULL;
    (void) QTSS_GetValueAsString (fieldPtr->fAPISource, theID,0, &theName);
    OSCharArrayDeleter nameDeleter(theName); 
    if (theName != NULL )
    {   UInt32 len = strlen(theName);
        if (len < eMaxAttributeNameSize)
        {   memcpy(fieldPtr->fFieldName, theName, len); 
            fieldPtr->fFieldName[len] = 0;
            fieldPtr->fFieldLen = len;
        }
    }
}

Bool16 ElementNode::GetFilteredAttributeID(char *parentName, char *nodeName, QTSS_AttributeID* foundID)
{
    Bool16 found = false;
    
    if (0 == strcmp("server", parentName))
    {
        if (0 == strcmp("qtssSvrClientSessions", nodeName) )
        {   if (foundID)
                *foundID = qtssCliSesCounterID;
            found = true;
        }
        
        if (0 == strcmp("qtssSvrModuleObjects", nodeName))
        {   if (foundID) 
                *foundID = qtssModName;
            found = true;
        }
    }           
    return found;
};

Bool16 ElementNode::IsPreferenceContainer(char *nodeName, QTSS_AttributeID* foundID)
{
     Bool16 found = false;
    if (foundID) *foundID = 0;
    //qtss_printf(" ElementNode::IsPreferenceContainer name = %s \n",nodeName);
    if (0 == strcmp("qtssSvrPreferences", nodeName) )
    {   if (foundID) *foundID = qtssCliSesCounterID;
        found = true;
    }
    
    if (0 == strcmp("qtssModPrefs", nodeName))
    {   if (foundID) *foundID = qtssModName;
        found = true;
    }
        
    return found;
};

ElementNode::ElementDataFields AdminClass::sAdminSelf[] = // special case of top of tree
{   // key, API_ID,     fIndex,     Name,       Name_Len,       fAccessPermissions, Access, access_Len, fAPI_Type, fFieldType ,fAPISource
    {0,     0,          0,          "admin",    strlen("admin"),    qtssAttrModeRead, "r", strlen("r"),0,   ElementNode::eNode, NULL    }
};

ElementNode::ElementDataFields AdminClass::sAdminFieldIDs[] = 
{   // key, API_ID,     fIndex,     Name,       Name_Len,       fAccessPermissions, Access, access_Len, fAPI_Type, fFieldType ,fAPISource
    {0,     0,          0,          "server",   strlen("server"),qtssAttrModeRead,  "r", strlen("r"),qtssAttrDataTypeQTSS_Object,   ElementNode::eNode, NULL    }
};



void AdminClass::Initialize(QTSS_Initialize_Params *initParams, QueryURI *queryPtr) 
{   

    //qtss_printf("----- Initialize AdminClass ------\n");
    
    SetParentNode(NULL);
    SetNodeInfo(&sAdminSelf[0]);// special case of this node as top of tree so it sets self
    Assert(NULL != GetMyName());
    SetNodeName(GetMyName());
    SetSource(NULL);
    StrPtrLen *currentPathPtr = queryPtr->GetRootID();
    UInt32 numFields = 1;
    SetNumFields(numFields);
    fFieldIDs = sAdminFieldIDs;
    fDataFieldsType = eStatic;
    fPathBuffer[0]=0;
    fPathSPL.Set(fPathBuffer,0);
    fIsTop = true;
    fInitialized = true;
    do
    {   
        Assert(fElementMap == NULL);
        fElementMap = NEW OSRefTable();  ElementNode_InsertPtr(fElementMap,"AdminClass::Initialize ElementNode* fElementMap ");
        Assert(fElementMap != NULL);
        if (fElementMap == NULL) break;
        
        Assert(fFieldDataPtrs == NULL);
        fFieldDataPtrs = NEW char*[numFields];  ElementNode_InsertPtr(fFieldDataPtrs,"AdminClass::Initialize ElementNode* fFieldDataPtrs array ");
        Assert(fFieldDataPtrs != NULL);
        if (fFieldDataPtrs == NULL) break;
        memset(fFieldDataPtrs, 0, numFields * sizeof(char*));
            
        Assert(fFieldOSRefPtrs == NULL);
        fFieldOSRefPtrs = NEW OSRef *[numFields]; ElementNode_InsertPtr(fFieldOSRefPtrs,"AdminClass::Initialize ElementNode* fFieldOSRefPtrs array ");
        Assert(fFieldOSRefPtrs != NULL);
        if (fFieldOSRefPtrs == NULL) break;
        memset(fFieldOSRefPtrs, 0, numFields * sizeof(OSRef*));
        
        QTSS_Error err = fElementMap->Register(GetOSRef(0));        
        Assert(err == QTSS_NoErr);
    } while (false);
    
    if (queryPtr && currentPathPtr) do
    {   
        StrPtrLen nextSegment;
        if (!queryPtr->NextSegment(currentPathPtr, &nextSegment)) break;

        SetupNodes(queryPtr,currentPathPtr,initParams);
    } while(false);


};

void AdminClass::SetUpSingleNode(QueryURI *queryPtr,  StrPtrLen *currentSegmentPtr, StrPtrLen *nextSegmentPtr, SInt32 index, QTSS_Initialize_Params *initParams) 
{
    //qtss_printf("-------- AdminClass::SetUpSingleNode ---------- \n");
    switch (index)
    {
        case eServer:
            //qtss_printf("AdminClass::SetUpSingleNode case eServer\n");
            fNodePtr =  NEW ElementNode();  ElementNode_InsertPtr(fNodePtr, "AdminClass::SetUpSingleNode ElementNode * NEW ElementNode()");
            SetElementDataPtr(index,(char *) fNodePtr, true); 
            if (fNodePtr)
            {   fNodePtr->Initialize(index, this, queryPtr,nextSegmentPtr,initParams, initParams->inServer, eDynamic);
            }
        break;
    };
    
}
void AdminClass::SetUpSingleElement(QueryURI *queryPtr, StrPtrLen *currentSegmentPtr,StrPtrLen *nextSegmentPtr, SInt32 index, QTSS_Initialize_Params *initParams) 
{
    //qtss_printf("---------AdminClass::SetUpSingleElement------- \n");
    SetUpSingleNode(queryPtr,currentSegmentPtr, nextSegmentPtr, index, initParams);
}

AdminClass::~AdminClass() 
{   //qtss_printf("AdminClass::~AdminClass() \n");
    delete (ElementNode*) fNodePtr;ElementNode_RemovePtr(fNodePtr,"AdminClass::~AdminClass ElementNode* fNodePtr");
    fNodePtr = NULL;
}

