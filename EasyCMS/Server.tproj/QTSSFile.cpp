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
	Copyleft (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
/*
    File:       QTSSFile.h

    Contains:    
*/

#include "QTSSFile.h"
#include "QTSServerInterface.h"

QTSSAttrInfoDict::AttrInfo  QTSSFile::sAttributes[] = 
{   /*fields:   fAttrName, fFuncPtr, fAttrDataType, fAttrPermission */
    /* 0 */ { "qtssFlObjStream",                NULL,   qtssAttrDataTypeQTSS_StreamRef, qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 1 */ { "qtssFlObjFileSysModuleName",     NULL,   qtssAttrDataTypeCharArray,      qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 2 */ { "qtssFlObjLength",                NULL,   qtssAttrDataTypeUInt64,         qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeWrite },
    /* 3 */ { "qtssFlObjPosition",              NULL,   qtssAttrDataTypeUInt64,         qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 4 */ { "qtssFlObjModDate",               NULL,   qtssAttrDataTypeUInt64,         qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeWrite }
};

void    QTSSFile::Initialize()
{
    for (UInt32 x = 0; x < qtssFlObjNumParams; x++)
        QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kFileDictIndex)->
            SetAttribute(x, sAttributes[x].fAttrName, sAttributes[x].fFuncPtr, sAttributes[x].fAttrDataType, sAttributes[x].fAttrPermission);
}

QTSSFile::QTSSFile()
:   QTSSDictionary(QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kFileDictIndex)),
    fModule(NULL),
    fPosition(0),
    fLength(0),
    fModDate(0)
{
    fThisPtr = this;
    //
    // The stream is just a pointer to this thing
    this->SetVal(qtssFlObjStream, &fThisPtr, sizeof(fThisPtr));
    this->SetVal(qtssFlObjLength, &fLength, sizeof(fLength));
    this->SetVal(qtssFlObjPosition, &fPosition, sizeof(fPosition));
    this->SetVal(qtssFlObjModDate, &fModDate, sizeof(fModDate));
}

QTSS_Error  QTSSFile::Open(char* inPath, QTSS_OpenFileFlags inFlags)
{
    //
    // Because this is a role being executed from inside a callback, we need to
    // make sure that QTSS_RequestEvent will not work.
    Task* curTask = NULL;
    QTSS_ModuleState* theState = (QTSS_ModuleState*)OSThread::GetMainThreadData();
    if (OSThread::GetCurrent() != NULL)
        theState = (QTSS_ModuleState*)OSThread::GetCurrent()->GetThreadData();
        
    if (theState != NULL)
        curTask = theState->curTask;
    
    QTSS_RoleParams theParams;
    theParams.openFileParams.inPath = inPath;
    theParams.openFileParams.inFlags = inFlags;
    theParams.openFileParams.inFileObject = this;

    QTSS_Error theErr = QTSS_FileNotFound;
    UInt32 x = 0;
    
    for ( ; x < QTSServerInterface::GetNumModulesInRole(QTSSModule::kOpenFilePreProcessRole); x++)
    {
        theErr = QTSServerInterface::GetModule(QTSSModule::kOpenFilePreProcessRole, x)->CallDispatch(QTSS_OpenFilePreProcess_Role, &theParams);
        if (theErr != QTSS_FileNotFound)
        {
            fModule = QTSServerInterface::GetModule(QTSSModule::kOpenFilePreProcessRole, x);
            break;
        }
    }
    
    if (theErr == QTSS_FileNotFound)
    {
        // None of the prepreprocessors claimed this file. Invoke the default file handler
        if (QTSServerInterface::GetNumModulesInRole(QTSSModule::kOpenFileRole) > 0)
        {
            fModule = QTSServerInterface::GetModule(QTSSModule::kOpenFileRole, 0);
            theErr = QTSServerInterface::GetModule(QTSSModule::kOpenFileRole, 0)->CallDispatch(QTSS_OpenFile_Role, &theParams);

        }
    }
    
    //
    // Reset the curTask to what it was before this role started
    if (theState != NULL)
        theState->curTask = curTask;

    return theErr;
}

void    QTSSFile::Close()
{
    Assert(fModule != NULL);
    
    QTSS_RoleParams theParams;
    theParams.closeFileParams.inFileObject = this;
    (void)fModule->CallDispatch(QTSS_CloseFile_Role, &theParams);
}


QTSS_Error  QTSSFile::Read(void* ioBuffer, UInt32 inBufLen, UInt32* outLengthRead)
{
    Assert(fModule != NULL);
    UInt32 theLenRead = 0;

    //
    // Invoke the owning QTSS API module. Setup a param block to do so.
    QTSS_RoleParams theParams;
    theParams.readFileParams.inFileObject = this;
    theParams.readFileParams.inFilePosition = fPosition;
    theParams.readFileParams.ioBuffer = ioBuffer;
    theParams.readFileParams.inBufLen = inBufLen;
    theParams.readFileParams.outLenRead = &theLenRead;
    
    QTSS_Error theErr = fModule->CallDispatch(QTSS_ReadFile_Role, &theParams);
    
    fPosition += theLenRead;
    if (outLengthRead != NULL)
        *outLengthRead = theLenRead;
        
    return theErr;
}
                                                            
QTSS_Error  QTSSFile::Seek(UInt64 inNewPosition)
{
    UInt64* theFileLength = NULL;
    UInt32 theParamLength = 0;
    
    (void)this->GetValuePtr(qtssFlObjLength, 0, (void**)(void*)&theFileLength, &theParamLength);
    
    if (theParamLength != sizeof(UInt64))
        return QTSS_RequestFailed;
        
    if (inNewPosition > *theFileLength)
        return QTSS_RequestFailed;
        
    fPosition = inNewPosition;
    return QTSS_NoErr;
}
        
QTSS_Error  QTSSFile::Advise(UInt64 inPosition, UInt32 inAdviseSize)
{
    Assert(fModule != NULL);

    //
    // Invoke the owning QTSS API module. Setup a param block to do so.
    QTSS_RoleParams theParams;
    theParams.adviseFileParams.inFileObject = this;
    theParams.adviseFileParams.inPosition = inPosition;
    theParams.adviseFileParams.inSize = inAdviseSize;

    return fModule->CallDispatch(QTSS_AdviseFile_Role, &theParams);
}

QTSS_Error  QTSSFile::RequestEvent(QTSS_EventType inEventMask)
{
    Assert(fModule != NULL);

    //
    // Invoke the owning QTSS API module. Setup a param block to do so.
    QTSS_RoleParams theParams;
    theParams.reqEventFileParams.inFileObject = this;
    theParams.reqEventFileParams.inEventMask = inEventMask;

    return fModule->CallDispatch(QTSS_RequestEventFile_Role, &theParams);
}
