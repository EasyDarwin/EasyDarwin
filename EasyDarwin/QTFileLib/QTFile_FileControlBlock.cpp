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
 // QTFile:
 //   The central point of control for a file in the QuickTime File Format.


 // -------------------------------------
 // Includes
 //
#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include <string.h>

#include "OSMutex.h"

#include "QTFile.h"


// -------------------------------------
// Macros
//
//#define DEBUG_PRINT(s) if(fDebug) qtss_printf s
//#define DEEP_DEBUG_PRINT(s) if(fDeepDebug) qtss_printf s


// -------------------------------------
// Class state cookie
//

QTFile_FileControlBlock::QTFile_FileControlBlock()
	: fDataFD(NULL), fDataBufferPool(NULL),
	fDataBufferSize(0), fDataBufferPosStart(0), fDataBufferPosEnd(0),
	fCurrentDataBuffer(NULL), fPreviousDataBuffer(NULL),
	fCurrentDataBufferLength(0), fPreviousDataBufferLength(0),
	fNumBlocksPerBuff(1), fNumBuffs(1),
	fCacheEnabled(false)

{
}

QTFile_FileControlBlock::~QTFile_FileControlBlock()
{
	if (fDataBufferPool != NULL)
		delete[] fDataBufferPool;
#if DSS_USE_API_CALLBACKS
	(void)QTSS_CloseFileObject(fDataFD);
#endif

}


void QTFile_FileControlBlock::Set(char * DataPath)
{
#if DSS_USE_API_CALLBACKS
	(void)QTSS_OpenFileObject(DataPath, qtssOpenFileReadAhead, &fDataFD);
#else
	fDataFD.Set(DataPath);
#endif

}

bool QTFile_FileControlBlock::ReadInternal(FILE_SOURCE *dataFD, UInt64 inPosition, void* inBuffer, UInt32 inLength, UInt32 *inReadLenPtr)
{
	UInt32 readLen = 0;
	if (NULL != inReadLenPtr)
		*inReadLenPtr = 0;

#if DSS_USE_API_CALLBACKS
	QTSS_Error  theErr = QTSS_Seek(*dataFD, inPosition);
	if (theErr == QTSS_NoErr)
		theErr = QTSS_Read(*dataFD, inBuffer, inLength, &readLen);
	if (theErr != QTSS_NoErr)
		return false;
#else
	if (dataFD->Read(inPosition, inBuffer, inLength, &readLen) != OS_NoErr)
		return false;
#endif
	if (NULL != inReadLenPtr)
		*inReadLenPtr = readLen;

	if (inReadLenPtr == NULL && readLen != inLength) //external reads expect false if it fails to read all the requested data.
		return false;

	return true;
}


bool QTFile_FileControlBlock::Read(FILE_SOURCE *dflt, UInt64 inPosition, void* inBuffer, UInt32 inLength)
{
	// Temporary vars
	UInt32 rcSize;

	// General vars
	FILE_SOURCE     *dataFD;

	// success or failure
	bool result = false;

	// Get the file descriptor.  If the FCB is NULL, or the descriptor in
	// the FCB is -1, then we need to use the class' descriptor.
	if (this->IsValid())
		dataFD = &fDataFD;
	else
		dataFD = dflt;

	if (
		(!fCacheEnabled) ||    // file control block caching disabled
		(inLength > fDataBufferSize) ||   // too big for this cache
		(inPosition < fDataBufferPosStart) // backing up
		)
	{
		//if ( !fCacheEnabled) qtss_printf("QTFile_FileControlBlock::Read  cache not enabled\n");
		//if ( inLength > fDataBufferSize) qtss_printf("QTFile_FileControlBlock::Read  read too big for cache len=%"   _U32BITARG_   " max%"   _U32BITARG_   "\n",inLength,fDataBufferSize);
		//if ( inPosition < fDataBufferPosStart) qtss_printf("QTFile_FileControlBlock::Read  backing up skipping cache missed by =%"   _U32BITARG_   " bytes\n", fDataBufferPosStart - inPosition);
		result = this->ReadInternal(dataFD, inPosition, inBuffer, inLength);
		goto done;
	}


	// Is the requested block of data in our data buffer?  If not, read in the
	// section of the file where this piece of data is.
	if ((inPosition < fDataBufferPosStart) || ((inPosition + inLength) > fDataBufferPosEnd))
	{
		// If this is a forward-moving, contiguous read, then we can keep the
		// current buffer around.

		if ((fCurrentDataBufferLength != 0)
			&& ((fDataBufferPosEnd - fCurrentDataBufferLength) <= inPosition)
			&& ((fDataBufferPosEnd + fDataBufferSize) >= (inPosition + inLength))
			)
		{
			// Temporary vars
			char        *TempDataBuffer;

			//qtss_printf("QTFile_FileControlBlock::Read forward read inPosition=%" _64BITARG_ "u fPreviousDataBuffer=%" _64BITARG_ "u start=%" _64BITARG_ "u\n",inPosition,fDataBufferPosStart,fDataBufferPosEnd);

			// First, demote the current buffer.
			fDataBufferPosStart += fPreviousDataBufferLength;
			TempDataBuffer = fPreviousDataBuffer;

			fPreviousDataBuffer = fCurrentDataBuffer;
			fPreviousDataBufferLength = fCurrentDataBufferLength;

			fCurrentDataBuffer = TempDataBuffer;
			fCurrentDataBufferLength = 0;

			//
			// Then, fill the now-current buffer with data.
			if (!this->ReadInternal(dataFD, fDataBufferPosEnd, fCurrentDataBuffer, fDataBufferSize, &rcSize))
				goto done;

			//Assert(rcSize == fDataBufferSize);
			fCurrentDataBufferLength = (UInt32)rcSize;
			fDataBufferPosEnd += fCurrentDataBufferLength;
		}
		else
		{
			//qtss_printf("QTFile_FileControlBlock::Read not a contiguous forward read inPosition=%" _64BITARG_ "u fPreviousDataBuffer=%" _64BITARG_ "u missed=%" _64BITARG_ "d\n ",inPosition,fDataBufferPosStart,(fDataBufferPosStart > inPosition) ? fDataBufferPosStart-inPosition: inPosition - fDataBufferPosStart);

			// We need to play with our current and previous data buffers in
			// order to skip around while reading.
			fCurrentDataBuffer = fDataBufferPool;
			fCurrentDataBufferLength = 0;

			fPreviousDataBuffer = (char *)fDataBufferPool + fDataBufferSize;
			fPreviousDataBufferLength = 0;

			fDataBufferPosStart = inPosition;

			if (!this->ReadInternal(dataFD, fDataBufferPosStart, fCurrentDataBuffer, fDataBufferSize, &rcSize))
				goto done;

			//Assert(rcSize == fDataBufferSize);
			fCurrentDataBufferLength = (UInt32)rcSize;
			fDataBufferPosEnd = fDataBufferPosStart + fCurrentDataBufferLength;
		}

	}

	//
	// Copy the data out of our buffer(s).
	{
		// General vars
		UInt64      ReadLength = inLength;
		UInt64      ReadOffset = inPosition - fDataBufferPosStart;

		//
		// Figure out if doing a continuous copy would cause us to cross a
		// buffer boundary.
		if ((inPosition < (fDataBufferPosStart + fPreviousDataBufferLength))
			&& ((ReadOffset + ReadLength) > fPreviousDataBufferLength)
			)
		{
			// Temporary vars
			char    *pBuffer = (char *)inBuffer;

			//
			// Read the first part of the block.
			ReadLength = fDataBufferSize - ReadOffset;
			if (ReadLength <= (fPreviousDataBufferLength - ReadOffset))
				::memcpy(pBuffer, fPreviousDataBuffer + ReadOffset, (UInt32)ReadLength);
			else
				goto done;

			pBuffer += ReadLength;

			//
			// Read the last part of the block.
			ReadLength = inLength - ReadLength;
			if (ReadLength <= fCurrentDataBufferLength)
			{
				::memcpy(pBuffer, fCurrentDataBuffer, (UInt32)ReadLength);
				result = true;
			}
			//
			// Or maybe this is a continuous copy out of the old buffer.
		}
		else if (inPosition < (fDataBufferPosEnd - fCurrentDataBufferLength))
		{
			if (ReadLength <= (fPreviousDataBufferLength - ReadOffset))
			{
				::memcpy(inBuffer, fPreviousDataBuffer + ReadOffset, (UInt32)ReadLength);

				result = true;
			}
		}
		else
		{
			ReadOffset -= fPreviousDataBufferLength;
			if (ReadLength <= (fCurrentDataBufferLength - ReadOffset))
			{
				::memcpy(inBuffer, fCurrentDataBuffer + ReadOffset, (UInt32)ReadLength);
				result = true;
			}

		}
	}

done:
	// We're done.
	return result;
}


void QTFile_FileControlBlock::AdjustDataBufferBitRate(UInt32 inUnitSizeInK, UInt32 inFileBitRate, UInt32 inNumBuffSizeUnits, UInt32 inMaxBitRateBuffSizeInBlocks)
{
	if (!fCacheEnabled)
		return;

	// General vars
	UInt32  newDataBufferSizeInUnits = inNumBuffSizeUnits;
	UInt32  newDataBufferSize = 0;
	UInt32  newUnitSizeBytes = 0;

	if (inUnitSizeInK < 1)
		inUnitSizeInK = 32;

	newUnitSizeBytes = inUnitSizeInK * 1024;

	if (inMaxBitRateBuffSizeInBlocks < 1)
		inMaxBitRateBuffSizeInBlocks = kMaxDefaultBlocks;

	if (inFileBitRate == 0) // set the maximum
		inFileBitRate = inMaxBitRateBuffSizeInBlocks * newUnitSizeBytes;

	if (inNumBuffSizeUnits < 1) // calculate if not set to a given number
	{
		newDataBufferSizeInUnits = (inFileBitRate + newUnitSizeBytes) / newUnitSizeBytes; // 32k bytes of buffer for every 32k bits in rate
		if (newDataBufferSizeInUnits > inMaxBitRateBuffSizeInBlocks) // Limit the buffer size value to a reasonable maximum. should be pref
			newDataBufferSizeInUnits = inMaxBitRateBuffSizeInBlocks;
	}

	newDataBufferSize = newDataBufferSizeInUnits * kBlockByteSize;

	//qtss_printf("QTFile_FileControlBlock::AdjustDataBuffer private buffers NewDataBufferSizeInUnits =%"   _U32BITARG_   " NewDataBufferSize = %"   _U32BITARG_   "\n",newDataBufferSizeInUnits,newDataBufferSize);

	// Free the old buffer.
	delete[] fDataBufferPool;
	fDataBufferSize = newDataBufferSize;
	fDataBufferPool = new char[2 * newDataBufferSize]; // 2 contiguous buffers
	fCurrentDataBuffer = fDataBufferPool;
	fPreviousDataBuffer = (char *)fDataBufferPool + newDataBufferSize;

	fDataBufferPosStart = 0;
	fDataBufferPosEnd = 0;
	fCurrentDataBufferLength = 0;
	fPreviousDataBuffer = 0;

}
