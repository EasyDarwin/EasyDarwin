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
	 File:       osfile.cpp

	 Contains:   simple file abstraction




 */

#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#ifndef __Win32__
#include <unistd.h>
#endif

#include "OSFileSource.h"
#include "OSMemory.h"
#include "OSThread.h"
#include "OSQueue.h"
#include "OSHeaders.h"

#define FILE_SOURCE_DEBUG 0
#define FILE_SOURCE_BUFFTEST 0
#define TEST_TIME 0

#if TEST_TIME
static SInt64 startTime = 0;
static SInt64 durationTime = 0;
static SInt32 sReadCount = 0;
static SInt32 sByteCount = 0;
static bool sMovie = false;

#endif


#if READ_LOG
extern UInt32 xTrackID;
void OSFileSource::SetLog(const char *inPath)
{
	fFilePath[0] = 0;
	::strcpy(fFilePath, inPath);

	if (fFile != -1 && fFileLog == nullptr)
	{
		::strcat(fFilePath, inPath);
		::strcat(fFilePath, ".readlog");
		fFileLog = ::fopen(fFilePath, "w+");
		if (fFileLog && IsValid())
		{
			qtss_fprintf(fFileLog, "%s", "QTFILE_READ_LOG\n");
			qtss_fprintf(fFileLog, "size: %qu\n", GetLength());
			qtss_printf("OSFileSource::SetLog=%s\n", fFilePath);

		}
		::fclose(fFileLog);
	}
}
#else
void OSFileSource::SetLog(const char *inPath)
{

#if FILE_SOURCE_DEBUG
	qtss_printf("OSFileSource::SetLog=%s\n", inPath);
#endif

}
#endif



FileBlockBuffer::~FileBlockBuffer()
{
	if (fDataBuffer != nullptr)
	{
		Assert(fDataBuffer[fBufferSize] == 0);

#if FILE_SOURCE_DEBUG
		::memset((char *)fDataBuffer, 0, fBufferSize);
		qtss_printf("FileBlockBuffer::~FileBlockBuffer delete %" _U32BITARG_ " this=%" _U32BITARG_ "\n", fDataBuffer, this);
#endif
		delete fDataBuffer;
		fDataBuffer = nullptr;
		fArrayIndex = -1;
	}
	else
		Assert(false);
}

void FileBlockBuffer::AllocateBuffer(UInt32 buffSize)
{
	fBufferSize = buffSize;
	fDataBuffer = new char[buffSize + 1];
	fDataBuffer[buffSize] = 0;

#if FILE_SOURCE_DEBUG
	this->CleanBuffer();
	qtss_printf("FileBlockBuffer::FileBlockBuffer allocate buff ptr =%" _U32BITARG_ " len=%" _U32BITARG_ " this=%"   _U32BITARG_   "\n", fDataBuffer, buffSize, this);
#endif

}

void FileBlockBuffer::TestBuffer()
{

#if FILE_SOURCE_BUFFTEST    
	if (fDataBuffer != nullptr)
		Assert(fDataBuffer[fBufferSize] == 0);
#endif

}

void FileBlockPool::MarkUsed(FileBlockBuffer* inBuffPtr)
{
	if (nullptr == inBuffPtr)
		return;

	if (fQueue.GetTail() != inBuffPtr->GetQElem()) // Least Recently Used tail is last accessed
	{
		fQueue.Remove(inBuffPtr->GetQElem());
		fQueue.EnQueue(inBuffPtr->GetQElem()); // put on tail
	}
}

FileBlockBuffer* FileBlockPool::GetBufferElement(UInt32 bufferSizeBytes)
{
	FileBlockBuffer* theNewBuf = nullptr;
	if (fNumCurrentBuffers < fMaxBuffers)
	{
#if FILE_SOURCE_DEBUG
		qtss_printf("FileBlockPool::GetBufferElement new element fNumCurrentBuffers=%" _U32BITARG_ " fMaxBuffers=%"   _U32BITARG_   " fBufferUnitSizeBytes=%"   _U32BITARG_   " bufferSizeBytes=%"   _U32BITARG_   "\n", fNumCurrentBuffers, fMaxBuffers, fBufferUnitSizeBytes, bufferSizeBytes);
#endif
		theNewBuf = new FileBlockBuffer();
		theNewBuf->AllocateBuffer(bufferSizeBytes);
		fNumCurrentBuffers++;
		theNewBuf->fQElem.SetEnclosingObject(theNewBuf);
		fQueue.EnQueue(theNewBuf->GetQElem()); // put on tail
		Assert(theNewBuf != nullptr);
		return theNewBuf;
	}

	OSQueueElem* theElem = fQueue.DeQueue(); // get head

	Assert(theElem != nullptr);

	if (theElem == nullptr)
		return nullptr;

	theNewBuf = (FileBlockBuffer*)theElem->GetEnclosingObject();
	Assert(theNewBuf != nullptr);
	//qtss_printf("FileBlockPool::GetBufferElement reuse buffer theNewBuf=%" _U32BITARG_ " fDataBuffer=%"   _U32BITARG_   " fArrayIndex=%" _S32BITARG_ "\n",theNewBuf,theNewBuf->fDataBuffer,theNewBuf->fArrayIndex);

	return theNewBuf;

}

void FileBlockPool::DeleteBlockPool()
{

	FileBlockBuffer* buffer = nullptr;
	OSQueueElem* theElem = fQueue.DeQueue();
	while (theElem != nullptr)
	{
		buffer = (FileBlockBuffer *)theElem->GetEnclosingObject();
		delete buffer;
		theElem = fQueue.DeQueue();
	}

	fMaxBuffers = 1;
	fNumCurrentBuffers = 0;
	fBufferUnitSizeBytes = kBufferUnitSize;
}

FileBlockPool::~FileBlockPool()
{
	this->DeleteBlockPool();
}


void FileMap::AllocateBufferMap(UInt32 inUnitSizeInK, UInt32 inNumBuffSizeUnits, UInt32 inBufferIncCount, UInt32 inMaxBitRateBuffSizeInBlocks, UInt64 fileLen, UInt32 inBitRate)
{

	if (fFileMapArray != nullptr && fNumBuffSizeUnits == inNumBuffSizeUnits && inBufferIncCount == fBlockPool.GetMaxBuffers())
		return;

	if (inUnitSizeInK < 1)
		inUnitSizeInK = 1;

	fBlockPool.SetBufferUnitSize(inUnitSizeInK);

	if (inBitRate == 0) // just use the maximum possible size
		inBitRate = inMaxBitRateBuffSizeInBlocks * fBlockPool.GetBufferUnitSizeBytes();

	if (inNumBuffSizeUnits == 0) // calculate the buffer size ourselves
	{
		inNumBuffSizeUnits = inBitRate / fBlockPool.GetBufferUnitSizeBytes();

		if (inNumBuffSizeUnits > inMaxBitRateBuffSizeInBlocks) // max is 8 * buffUnit Size (32k) = 256K
		{
			inNumBuffSizeUnits = inMaxBitRateBuffSizeInBlocks;
		}
	} //else the inNumBuffSizeUnits is explicitly defined so just use that value

	if (inNumBuffSizeUnits < 1)
		inNumBuffSizeUnits = 1;

	this->DeleteMap();
	fBlockPool.DeleteBlockPool();

	fNumBuffSizeUnits = inNumBuffSizeUnits;
	fDataBufferSize = fBlockPool.GetBufferUnitSizeBytes() * inNumBuffSizeUnits;

	fBlockPool.SetMaxBuffers(inBufferIncCount);
	fBlockPool.SetBuffIncValue(inBufferIncCount);

	fMapArraySize = (fileLen / fDataBufferSize) + 1;
	fFileMapArray = new FileBlockBuffer *[(SInt32)(fMapArraySize + 1)];

	this->Clean(); // required because fFileMapArray's array is used to store buffer pointers.
#if FILE_SOURCE_DEBUG
	qtss_printf("FileMap::AllocateBufferMap shared buffers fFileMapArray=%" _U32BITARG_ " fDataBufferSize= %"   _U32BITARG_   " fMapArraySize=%"   _U32BITARG_   " fileLen=%qu \n", fFileMapArray, fDataBufferSize, fMapArraySize, fileLen);
#endif

}

void FileMap::DeleteOldBuffs()
{
	while (fBlockPool.GetNumCurrentBuffers() > fBlockPool.GetMaxBuffers()) // delete any old buffers
	{
		FileBlockBuffer* theElem = fBlockPool.GetBufferElement(fDataBufferSize);
		fFileMapArray[theElem->fArrayIndex] = nullptr;
		delete theElem;
		fBlockPool.DecCurBuffers();
	}
}

char* FileMap::GetBuffer(SInt64 buffIndex, bool* outFillBuff)
{
	Assert(outFillBuff != nullptr);
	*outFillBuff = true; // we are re-using or just created a buff

	this->DeleteOldBuffs();
	Assert(buffIndex < (SInt32)fMapArraySize);

	FileBlockBuffer *theElem = fFileMapArray[buffIndex];
	if (nullptr == theElem)
	{
#if FILE_SOURCE_DEBUG
		qtss_printf("FileMap::GetBuffer call fBlockPool.GetBufferElement(); buffIndex=%" _S32BITARG_ "\n", buffIndex);
#endif

		theElem = fBlockPool.GetBufferElement(fDataBufferSize);
		Assert(theElem);
	}

	fBlockPool.MarkUsed(theElem); // must happen here after getting a pre-allocated or used buffer.

	if (theElem->fArrayIndex == buffIndex) // found a pre-allocated and filled buffer
	{
#if FILE_SOURCE_DEBUG
		//qtss_printf("FileMap::GetBuffer pre-allocated buff buffIndex=%" _S32BITARG_ "\n",buffIndex);
#endif

		*outFillBuff = false;
		return theElem->fDataBuffer;
	}

	if (theElem->fArrayIndex >= 0)
	{
		fFileMapArray[theElem->fArrayIndex] = nullptr; // reset the old map location
	}
	fFileMapArray[buffIndex] = theElem; // a new buffer
	theElem->fArrayIndex = buffIndex; // record the index

#if FILE_SOURCE_DEBUG
	theElem->CleanBuffer();
#endif

	return theElem->fDataBuffer;

}

void FileMap::Clean()
{
	if (fFileMapArray != nullptr)
		::memset((char *)fFileMapArray, 0, (SInt32)(sizeof(FileBlockBuffer *) * fMapArraySize));
}

void FileMap::DeleteMap()
{
	if (nullptr == fFileMapArray)
		return;

#if FILE_SOURCE_DEBUG
	qtss_printf("FileMap::DeleteMap fFileMapArray=%" _U32BITARG_ " fMapArraySize=%" _S32BITARG_ " \n", fFileMapArray, fMapArraySize);
	this->Clean();
#endif

	delete fFileMapArray;
	fFileMapArray = nullptr;

}

void OSFileSource::Set(const char* inPath)
{
	Close();

#if __Win32__
	fFile = open(inPath, O_RDONLY | O_BINARY);
#elif __linux__
	fFile = open(inPath, O_RDONLY | O_LARGEFILE);
#else
	fFile = open(inPath, O_RDONLY);
#endif

	if (fFile != -1)
	{
		struct stat buf;
		::memset(&buf, sizeof(buf), 0);
		if (::fstat(fFile, &buf) >= 0)
		{
			fLength = buf.st_size;
			fModDate = buf.st_mtime;
			if (fModDate < 0)
				fModDate = 0;
#ifdef __Win32__
			fIsDir = buf.st_mode & _S_IFDIR;
#else
			fIsDir = S_ISDIR(buf.st_mode);
#endif
			this->SetLog(inPath);
		}
		else
			this->Close();
	}
}

void OSFileSource::Advise(UInt64, UInt32)
{
	// does nothing on platforms other than MacOSXServer
}

OS_Error OSFileSource::FillBuffer(char* ioBuffer, char* buffStart, SInt32 buffIndex)
{
	UInt32 buffSize = fFileMap.GetMaxBufSize();
	UInt64 startPos = (UInt64)buffIndex * (UInt64)buffSize;
	UInt32 readLen = 0;

	OS_Error theErr = this->ReadFromPos(startPos, buffStart, buffSize, &readLen);

	fFileMap.SetIndexBuffFillSize(buffIndex, readLen);
	fFileMap.TestBuffer(buffIndex);

	return theErr;
}

#if FILE_SOURCE_BUFFTEST
static SInt32 sBuffCount = 1;
#endif

OS_Error OSFileSource::Read(UInt64 inPosition, void* inBuffer, UInt32 inLength, UInt32* outRcvLen)
{

	if ((!fFileMap.Initialized())
		|| (!fCacheEnabled)
		|| (fFileMap.GetBuffIndex(inPosition + inLength) > fFileMap.GetMaxBuffIndex())
		)
		return  this->ReadFromPos(inPosition, inBuffer, inLength, outRcvLen);

	return  this->ReadFromCache(inPosition, inBuffer, inLength, outRcvLen);
}


OS_Error OSFileSource::ReadFromCache(UInt64 inPosition, void* inBuffer, UInt32 inLength, UInt32* outRcvLen)
{
	OSMutexLocker locker(&fMutex);

	if (!fFileMap.Initialized() || !fCacheEnabled)
	{
		Assert(0);
	}

	Assert(outRcvLen != nullptr);
	*outRcvLen = 0;

	if (inPosition >= fLength) // eof
		return OS_NoErr;

	SInt64 buffIndex = fFileMap.GetBuffIndex(inPosition);
	SInt64 buffSize = 0;
	SInt64 maxBuffSize = fFileMap.GetMaxBufSize();
	SInt64 endIndex = fFileMap.GetBuffIndex(inPosition + inLength);
	SInt64 maxIndex = fFileMap.GetMaxBuffIndex();
	SInt64 buffPos = inPosition - fFileMap.GetBuffOffset(buffIndex);
	SInt64 buffOffsetLen = 0;
	char* buffStart = nullptr;
	SInt64 buffCopyLen = inLength;
	SInt64 bytesToCopy = inLength;
	char* buffOut = (char*)inBuffer;
	bool fillBuff = true;
	char* buffOffset = nullptr;

#if FILE_SOURCE_BUFFTEST
	char testBuff[inLength + 1];
	buffOut = (char*)testBuff;
	sBuffCount++;
	::memset(inBuffer, 0, inLength);
	::memset(testBuff, 0, inLength);
#endif

	if (buffIndex > endIndex || endIndex > maxIndex)
	{
#if FILE_SOURCE_DEBUG

		qtss_printf("OSFileSource::ReadFromCache bad index: buffIndex=%" _S32BITARG_ " endIndex=%" _S32BITARG_ " maxIndex=%" _S32BITARG_ "\n", buffIndex, endIndex, maxIndex);
		qtss_printf("OSFileSource::ReadFromCache inPosition =%qu buffSize = %"   _U32BITARG_   " index=%" _S32BITARG_ "\n", inPosition, fFileMap.GetMaxBufSize(), buffIndex);
#endif
		Assert(0);
	}

	while (buffIndex <= endIndex && buffIndex <= maxIndex)
	{
#if FILE_SOURCE_DEBUG
		qtss_printf("OSFileSource::ReadFromCache inPosition =%qu buffSize = %" _U32BITARG_ " index=%" _S32BITARG_ "\n", inPosition, fFileMap.GetMaxBufSize(), buffIndex);
#endif

		buffStart = fFileMap.GetBuffer(buffIndex, &fillBuff);
		Assert(buffStart != nullptr);

		if (fillBuff)
		{
			OS_Error theErr = this->FillBuffer((char *)inBuffer, (char *)buffStart, (SInt32)buffIndex);
			if (theErr != OS_NoErr)
				return theErr;

		}


		buffSize = fFileMap.GetBuffSize(buffIndex);
		buffOffset = &buffStart[buffPos];

		if ((buffPos == 0) &&
			(bytesToCopy <= maxBuffSize) &&
			(buffSize < bytesToCopy)
			) // that's all there is in the file
		{

#if FILE_SOURCE_DEBUG
			qtss_printf("OSFileSource::ReadFromCache end of file reached buffIndex=%" _U32BITARG_ " buffSize = %" _S32BITARG_ " bytesToCopy=%"   _U32BITARG_   "\n", buffIndex, buffSize, bytesToCopy);
#endif
			Assert(buffSize <= (SInt64)kUInt32_Max);
			::memcpy(buffOut, buffOffset, (UInt32)buffSize);
			*outRcvLen += (UInt32)buffSize;
			break;
		}

		buffOffsetLen = buffSize - buffPos;
		if (buffCopyLen >= buffOffsetLen)
			buffCopyLen = buffOffsetLen;

		Assert(buffCopyLen <= buffSize);

		::memcpy(buffOut, buffOffset, (UInt32)buffCopyLen);
		buffOut += buffCopyLen;
		*outRcvLen += (UInt32)buffCopyLen;
		bytesToCopy -= buffCopyLen;
		Assert(bytesToCopy >= 0);

		buffCopyLen = bytesToCopy;
		buffPos = 0;
		buffIndex++;

	}

#if FILE_SOURCE_DEBUG
	//qtss_printf("OSFileSource::ReadFromCache inLength= %" _U32BITARG_ " *outRcvLen=%" _U32BITARG_ "\n",inLength, *outRcvLen);
#endif

#if FILE_SOURCE_BUFFTEST    
	{   UInt32 outLen = 0;
	OS_Error theErr = this->ReadFromPos(inPosition, inBuffer, inLength, &outLen);

	Assert(*outRcvLen == outLen);
	if (*outRcvLen != outLen)
		qtss_printf("OSFileSource::ReadFromCache *outRcvLen != outLen *outRcvLen=%" _U32BITARG_ " outLen=%" _U32BITARG_ "\n", *outRcvLen, outLen);

	for (int i = 0; i < inLength; i++)
	{
		if (((char*)inBuffer)[i] != testBuff[i])
		{
			qtss_printf("OSFileSource::ReadFromCache byte pos %d of %" _U32BITARG_ " failed len=%" _U32BITARG_ " inPosition=%qu sBuffCount=%" _S32BITARG_ "\n", i, inLength, outLen, inPosition, sBuffCount);
			break;
		}
	}
	}
#endif

	return OS_NoErr;
}

OS_Error OSFileSource::ReadFromDisk(void* inBuffer, UInt32 inLength, UInt32* outRcvLen)
{
#if FILE_SOURCE_BUFFTEST
	qtss_printf("OSFileSource::Read inLength=%"   _U32BITARG_   " fFile=%d\n", inLength, fFile);
#endif

#if __Win32__
	if (_lseeki64(fFile, fPosition, SEEK_SET) == -1)
		return OSThread::GetErrno();
#else
	if (lseek(fFile, fPosition, SEEK_SET) == -1)
		return OSThread::GetErrno();
#endif


	int rcvLen = ::read(fFile, (char*)inBuffer, inLength);
	if (rcvLen == -1)
		return OSThread::GetErrno();

	if (outRcvLen != nullptr)
		*outRcvLen = rcvLen;

	fPosition += rcvLen;
	fReadPos = fPosition;

	return OS_NoErr;
}

OS_Error OSFileSource::ReadFromPos(UInt64 inPosition, void* inBuffer, UInt32 inLength, UInt32* outRcvLen)
{
#if TEST_TIME
	{
		startTime = OS::Milliseconds();
		sReadCount++;
		if (outRcvLen)
			*outRcvLen = 0;
		qtss_printf("OSFileSource::Read sReadCount = %" _S32BITARG_ " totalbytes=%" _S32BITARG_ " readsize=%"   _U32BITARG_   "\n", sReadCount, sByteCount, inLength);
	}
#endif

	this->Seek(inPosition);
	OS_Error err = this->ReadFromDisk(inBuffer, inLength, outRcvLen);

#if READ_LOG
	if (fFileLog)
	{
		fFileLog = ::fopen(fFilePath, "a");
		if (fFileLog)
		{
			qtss_fprintf(fFileLog, "read: %qu %"   _U32BITARG_   " %"   _U32BITARG_   "\n", inPosition, *outRcvLen, xTrackID);
			::fclose(fFileLog);
		}
	}

#endif
#if TEST_TIME
	{
		durationTime += OS::Milliseconds() - startTime;
		sByteCount += *outRcvLen;
	}
#endif

	return err;
}

void OSFileSource::SetTrackID(UInt32 trackID)
{
#if READ_LOG
	fTrackID = trackID;
	//  qtss_printf("OSFileSource::SetTrackID = %"   _U32BITARG_   " this=%"   _U32BITARG_   "\n",fTrackID,(UInt32) this);
#endif
}


void OSFileSource::Close()
{
	if ((fFile != -1) && (fShouldClose))
	{
		::close(fFile);

#if READ_LOG
		if (0 && fFileLog != nullptr)
		{
			::fclose(fFileLog);
			fFileLog = nullptr;
			fFilePath[0] = 0;
		}
#endif
	}

	fFile = -1;
	fModDate = 0;
	fLength = 0;
	fPosition = 0;
	fReadPos = 0;

#if TEST_TIME   
	if (fShouldClose)
	{
		sMovie = 0;
		//      qtss_printf("OSFileSource::Close sReadCount = %" _S32BITARG_ " totalbytes=%" _S32BITARG_ "\n",sReadCount,sByteCount);
		//      qtss_printf("OSFileSource::Close durationTime = %qd\n",durationTime);
	}
#endif

}
