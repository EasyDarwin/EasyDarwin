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
//
// QTFile_FileControlBlock:
//   All the per-client stuff for QTFile.


#ifndef _QTFILE_FILECONTROLBLOCK_H_
#define _QTFILE_FILECONTROLBLOCK_H_

//
// Includes
#include "OSHeaders.h"
#include "OSFileSource.h"

#if DSS_USE_API_CALLBACKS
#include "QTSS.h"
#endif

#if DSS_USE_API_CALLBACKS
    #define FILE_SOURCE QTSS_Object
#else
    #define FILE_SOURCE OSFileSource
#endif

//
// Class state cookie
class QTFile_FileControlBlock {

 public:
    //
    // Constructor and destructor.
    QTFile_FileControlBlock(void);
    virtual ~QTFile_FileControlBlock(void);
    
    //Sets this object to reference this file
    void Set(char *inPath);
        
    //Advise: this advises the OS that we are going to be reading soon from the
    //following position in the file
    // void Advise(OSFileSource *dflt, UInt64 advisePos, UInt32 adviseAmt);
    
    Bool16 Read(FILE_SOURCE *dflt, UInt64 inPosition, void* inBuffer, UInt32 inLength);

    Bool16 ReadInternal(FILE_SOURCE *dataFD, UInt64 inPosition, void* inBuffer, UInt32 inLength, UInt32 *inReadLenPtr = NULL);

    //
    // Buffer management functions
    void AdjustDataBufferBitRate(UInt32 inUnitSizeInK = 32, UInt32 inFileBitRate = 32768, UInt32 inNumBuffSizeUnits = 0, UInt32 inMaxBitRateBuffSizeInBlocks = 8);
    void AdjustDataBuffers(UInt32 inBlockSizeKBits = 32, UInt32 inBlockCountPerBuff = 1);
    void EnableCacheBuffers(Bool16 enabled) {fCacheEnabled = enabled;}
    
    // QTSS_ErrorCode Close();
    
    Bool16 IsValid()
        {
#if DSS_USE_API_CALLBACKS
            return fDataFD != NULL;
#else       
            return fDataFD.IsValid();
#endif
        }

    
private:
    //
    // File descriptor for this control block
    FILE_SOURCE fDataFD;

    enum
    {   
        kMaxDefaultBlocks           = 8,
        kDataBufferUnitSizeExp      = 15,   // 32Kbytes
        kBlockByteSize = ( 1 << kDataBufferUnitSizeExp)
    };
    //
    // Data buffer cache
    char                *fDataBufferPool;

    UInt32              fDataBufferSize;
    UInt64              fDataBufferPosStart, fDataBufferPosEnd;

    char                *fCurrentDataBuffer, *fPreviousDataBuffer;
    UInt32              fCurrentDataBufferLength, fPreviousDataBufferLength;
    
    UInt32              fNumBlocksPerBuff;
    UInt32              fNumBuffs;
    Bool16              fCacheEnabled;
};

#endif //_QTFILE_FILECONTROLBLOCK_H_
