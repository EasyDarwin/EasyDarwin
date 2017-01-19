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
 // QTTrack:
 //   The central point of control for a track in a QTFile.


 // -------------------------------------
 // Includes
 //
#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include <string.h>

#include "QTFile.h"

#include "QTAtom_dref.h"
#include "QTAtom_elst.h"
#include "QTAtom_mdhd.h"
#include "QTAtom_tkhd.h"
#include "QTAtom_stco.h"
#include "QTAtom_stsc.h"
#include "QTAtom_stsd.h"
#include "QTAtom_stss.h"
#include "QTAtom_stsz.h"
#include "QTAtom_stts.h"

#include "QTTrack.h"

// -------------------------------------
// Macros
//
#define DEBUG_PRINT(s) if(fDebug) qtss_printf s
#define DEEP_DEBUG_PRINT(s) if(fDeepDebug) qtss_printf s

// -------------------------------------
// Constructors and destructors
//
QTTrack::QTTrack(QTFile * File, QTFile::AtomTOCEntry * Atom, bool Debug, bool DeepDebug)
	: fDebug(Debug), fDeepDebug(DeepDebug),
	fFile(File),
	fIsInitialized(false),
	fTrackHeaderAtom(NULL),
	fTrackName(NULL),
	fMediaHeaderAtom(NULL),
	fEditListAtom(NULL), fDataReferenceAtom(NULL),
	fTimeToSampleAtom(NULL), fCompTimeToSampleAtom(NULL), fSampleToChunkAtom(NULL), fSampleDescriptionAtom(NULL),
	fChunkOffsetAtom(NULL), fSampleSizeAtom(NULL), fSyncSampleAtom(NULL),
	fFirstEditMediaTime(0)
{
	// Temporary vars
	QTFile::AtomTOCEntry    *tempTOCEntry;

	//
	// Make a copy of the TOC entry.
	memcpy(&fTOCEntry, Atom, sizeof(QTFile::AtomTOCEntry));

	//
	// Load in the track header atom for this track.
	if (!fFile->FindTOCEntry(":tkhd", &tempTOCEntry, &fTOCEntry))
		return;

	fTrackHeaderAtom = new QTAtom_tkhd(fFile, tempTOCEntry, fDebug, fDeepDebug);
	if (fTrackHeaderAtom == NULL)
		return;
	if (!fTrackHeaderAtom->Initialize()) {
		delete fTrackHeaderAtom;
		fTrackHeaderAtom = NULL;
	}

}

QTTrack::~QTTrack()
{
	//
	// Free our variables
	if (fTrackHeaderAtom != NULL)
		delete fTrackHeaderAtom;
	if (fTrackName != NULL)
		delete[] fTrackName;

	if (fMediaHeaderAtom != NULL)
		delete fMediaHeaderAtom;

	if (fEditListAtom != NULL)
		delete fEditListAtom;
	if (fDataReferenceAtom != NULL)
		delete fDataReferenceAtom;

	if (fTimeToSampleAtom != NULL)
		delete fTimeToSampleAtom;
	if (fSampleToChunkAtom != NULL)
		delete fSampleToChunkAtom;
	if (fSampleDescriptionAtom != NULL)
		delete fSampleDescriptionAtom;
	if (fChunkOffsetAtom != NULL)
		delete fChunkOffsetAtom;
	if (fSampleSizeAtom != NULL)
		delete fSampleSizeAtom;
	if (fSyncSampleAtom != NULL)
		delete fSyncSampleAtom;
}



// -------------------------------------
// Initialization functions
//
QTTrack::ErrorCode QTTrack::Initialize()
{
	// Temporary vars
	QTFile::AtomTOCEntry    *tempTOCEntry;


	//
	// Don't initialize more than once.
	if (IsInitialized())
		return errNoError;

	//
	// Make sure that we were able to read in our track header atom.
	if (fTrackHeaderAtom == NULL)
		return errInvalidQuickTimeFile;

	//
	// See if this track has a name and load it in.
	if (fFile->FindTOCEntry(":udta:name", &tempTOCEntry, &fTOCEntry)) {
		fTrackName = new char[(SInt32)(tempTOCEntry->AtomDataLength + 1)];
		if (fTrackName != NULL)
			fFile->Read(tempTOCEntry->AtomDataPos, fTrackName, (UInt32)tempTOCEntry->AtomDataLength);
	}


	//
	// Load in the media header atom for this track.
	if (!fFile->FindTOCEntry(":mdia:mdhd", &tempTOCEntry, &fTOCEntry))
		return errInvalidQuickTimeFile;

	fMediaHeaderAtom = new QTAtom_mdhd(fFile, tempTOCEntry, fDebug, fDeepDebug);
	if (fMediaHeaderAtom == NULL)
		return errInternalError;
	if (!fMediaHeaderAtom->Initialize())
		return errInvalidQuickTimeFile;


	//
	// Load in the edit list atom for this track.
	DEEP_DEBUG_PRINT(("Searching track #%"   _U32BITARG_   " 'elst' atom.\n", GetTrackID()));
	if (fFile->FindTOCEntry(":edts:elst", &tempTOCEntry, &fTOCEntry)) {
		fEditListAtom = new QTAtom_elst(fFile, tempTOCEntry, fDebug, fDeepDebug);
		if (fEditListAtom == NULL)
			return errInternalError;
		if (!fEditListAtom->Initialize())
			return errInvalidQuickTimeFile;

		//
		// Compute the first edit's media time.
		fFirstEditMediaTime = (UInt32)((((Float64)(SInt64)GetFirstEditMovieTime()) / fFile->GetTimeScale()) * GetTimeScale());
	}
	else {
		fEditListAtom = NULL;
	}


	//
	// Load in the data reference atom for this track.
	if (!fFile->FindTOCEntry(":mdia:minf:dinf:dref", &tempTOCEntry, &fTOCEntry))
		return errInvalidQuickTimeFile;

	fDataReferenceAtom = new QTAtom_dref(fFile, tempTOCEntry, fDebug, fDeepDebug);
	if (fDataReferenceAtom == NULL)
		return errInternalError;
	if (!fDataReferenceAtom->Initialize())
		return errInvalidQuickTimeFile;


	//
	// Load in the sample table atoms.
	if (!fFile->FindTOCEntry(":mdia:minf:stbl:stts", &tempTOCEntry, &fTOCEntry))
		return errInvalidQuickTimeFile;

	fTimeToSampleAtom = new QTAtom_stts(fFile, tempTOCEntry, fDebug, fDeepDebug);
	if (fTimeToSampleAtom == NULL)
		return errInternalError;
	if (!fTimeToSampleAtom->Initialize())
		return errInvalidQuickTimeFile;

	if (fFile->FindTOCEntry(":mdia:minf:stbl:ctts", &tempTOCEntry, &fTOCEntry))
	{
		fCompTimeToSampleAtom = new QTAtom_ctts(fFile, tempTOCEntry, fDebug, fDeepDebug);
		if (fCompTimeToSampleAtom == NULL)
			return errInternalError;
		if (!fCompTimeToSampleAtom->Initialize())
			return errInvalidQuickTimeFile;
	}

	if (!fFile->FindTOCEntry(":mdia:minf:stbl:stsc", &tempTOCEntry, &fTOCEntry))
		return errInvalidQuickTimeFile;

	fSampleToChunkAtom = new QTAtom_stsc(fFile, tempTOCEntry, fDebug, fDeepDebug);
	if (fSampleToChunkAtom == NULL)
		return errInternalError;
	if (!fSampleToChunkAtom->Initialize())
		return errInvalidQuickTimeFile;


	if (!fFile->FindTOCEntry(":mdia:minf:stbl:stsd", &tempTOCEntry, &fTOCEntry))
		return errInvalidQuickTimeFile;

	fSampleDescriptionAtom = new QTAtom_stsd(fFile, tempTOCEntry, fDebug, fDeepDebug);
	if (fSampleDescriptionAtom == NULL)
		return errInternalError;
	if (!fSampleDescriptionAtom->Initialize())
		return errInvalidQuickTimeFile;


	UInt16 offSetSize = 0;
	bool coFound = false;

	if (fFile->FindTOCEntry(":mdia:minf:stbl:stco", &tempTOCEntry, &fTOCEntry))
	{
		coFound = true;
		offSetSize = 4;
	}
	else if (fFile->FindTOCEntry(":mdia:minf:stbl:co64", &tempTOCEntry, &fTOCEntry))
	{
		coFound = true;
		offSetSize = 8;
	}

	if (!coFound)
		return errInvalidQuickTimeFile;

	fChunkOffsetAtom = new QTAtom_stco(fFile, tempTOCEntry, offSetSize, fDebug, fDeepDebug);
	if (fChunkOffsetAtom == NULL)
		return errInternalError;
	if (!fChunkOffsetAtom->Initialize())
		return errInvalidQuickTimeFile;


	if (!fFile->FindTOCEntry(":mdia:minf:stbl:stsz", &tempTOCEntry, &fTOCEntry))
		return errInvalidQuickTimeFile;

	fSampleSizeAtom = new QTAtom_stsz(fFile, tempTOCEntry, fDebug, fDeepDebug);
	if (fSampleSizeAtom == NULL)
		return errInternalError;
	if (!fSampleSizeAtom->Initialize())
		return errInvalidQuickTimeFile;

	if (fFile->FindTOCEntry(":mdia:minf:stbl:stss", &tempTOCEntry, &fTOCEntry)) {
		fSyncSampleAtom = new QTAtom_stss(fFile, tempTOCEntry, fDebug, fDeepDebug);
		if (fSyncSampleAtom == NULL)
			return errInternalError;
		if (!fSyncSampleAtom->Initialize())
			return errInvalidQuickTimeFile;
	}
	else {
		fSyncSampleAtom = NULL;
	}


	//
	// This track has been successfully initialiazed.
	fIsInitialized = true;
	return errNoError;
}



// -------------------------------------
// Sample functions
//
bool QTTrack::GetSampleInfo(UInt32 SampleNumber, UInt32 * const Length, UInt64 * const Offset, UInt32 * const SampleDescriptionIndex, QTAtom_stsc_SampleTableControlBlock * STCB)
{

	Assert(STCB != NULL);

	//  qtss_printf("GetSampleInfo QTTrack SampleNumber = %" _S32BITARG_ " \n", SampleNumber);

	if (STCB->fGetSampleInfo_SampleNumber == SampleNumber && STCB->fGetSampleInfo_Length > 0)
	{
		//      qtss_printf("----- GetSampleInfo Cache Hit QTTrack SampleNumber = %" _S32BITARG_ " \n", SampleNumber);

		if (Length) *Length = STCB->fGetSampleInfo_Length;
		if (Offset) *Offset = STCB->fGetSampleInfo_Offset;
		if (SampleDescriptionIndex) *SampleDescriptionIndex = STCB->fGetSampleInfo_SampleDescriptionIndex;

		return true;
	}




	// Temporary vars
	UInt32      sampleLength = 0;
	UInt32      sampleDescriptionIndex = 0;
	// General vars
	UInt32      ChunkNumber, SampleOffsetInChunk;
	UInt64      sampleFileStartOffset = 0;
	UInt64      ChunkOffset = 0;


	// Locate this sample, compute its offset, and get its size.
	if (!SampleNumberToChunkNumber(SampleNumber, &ChunkNumber, &sampleDescriptionIndex, &SampleOffsetInChunk, STCB))
		return false;


	if (!fSampleSizeAtom->SampleSize(SampleNumber, &sampleLength))
		return false;

	if (ChunkNumber == STCB->fGetSampleInfo_LastChunk && (SampleNumber == (STCB->fGetSampleInfo_SampleNumber + 1)))
	{

		sampleFileStartOffset = STCB->fGetSampleInfo_Offset;
		sampleFileStartOffset += STCB->fGetSampleInfo_Length;

	}
	else
	{
		if (!fChunkOffsetAtom->ChunkOffset(ChunkNumber, &ChunkOffset))
			return false;

		// Walk through all of the samples previous to this one, adding up
		// their lengths to figure out what the offset from the start of
		// the chunk to this sample is.


		UInt32  tempSampleLength = fSampleSizeAtom->GetCommonSampleSize();
		sampleFileStartOffset = ChunkOffset;

		if (tempSampleLength > 0) // samples are the same size so just multiply to get size
		{
			sampleFileStartOffset += (tempSampleLength * SampleOffsetInChunk);
		}
		else
		{
			for (UInt32 CurSample = (SampleNumber - SampleOffsetInChunk); CurSample < SampleNumber; CurSample++)
			{
				// Get the length of this sample and add it to our offset.
				if (!fSampleSizeAtom->SampleSize(CurSample, &tempSampleLength))
					return false;
				sampleFileStartOffset += tempSampleLength;
			}
		}


		STCB->fGetSampleInfo_LastChunk = ChunkNumber;
		STCB->fGetSampleInfo_LastChunkOffset = (UInt32)ChunkOffset;
		STCB->fGetSampleInfo_SampleDescriptionIndex = sampleDescriptionIndex;
	}

	STCB->fGetSampleInfo_SampleNumber = SampleNumber;
	STCB->fGetSampleInfo_Length = sampleLength;
	STCB->fGetSampleInfo_Offset = sampleFileStartOffset;

	if (Length != NULL) *Length = sampleLength;
	if (Offset != NULL) *Offset = sampleFileStartOffset;
	if (SampleDescriptionIndex != NULL) *SampleDescriptionIndex = sampleDescriptionIndex;



	//
	// The sample was successfully located.
	return true;
}


bool QTTrack::GetSizeOfSamplesInChunk(UInt32 chunkNumber, UInt32 * const sizePtr, UInt32 * const firstSampleNumPtr, UInt32 * const lastSampleNumPtr, QTAtom_stsc_SampleTableControlBlock * stcbPtr)
{
	UInt32 firstSample = 0;
	UInt32 lastSample = 0;
	UInt32 size = 0;

	if (stcbPtr && stcbPtr->fGetSizeOfSamplesInChunk_chunkNumber == chunkNumber)
	{
		//      qtss_printf("QTTrack::GetSizeOfSamplesInChunk cache hit %" _S32BITARG_ " \n", chunkNumber);
		if (firstSampleNumPtr != NULL) *firstSampleNumPtr = stcbPtr->fGetSizeOfSamplesInChunk_firstSample;
		if (lastSampleNumPtr != NULL) *lastSampleNumPtr = stcbPtr->fGetSizeOfSamplesInChunk_lastSample;
		if (sizePtr != NULL) *sizePtr = stcbPtr->fGetSizeOfSamplesInChunk_size;
		return true;
	}

	bool result = GetChunkFirstLastSample(chunkNumber, &firstSample, &lastSample, stcbPtr);

	if (result && (sizePtr != NULL))
	{
		result = SampleRangeSize(firstSample, lastSample, &size);
	}

	if (firstSampleNumPtr != NULL) *firstSampleNumPtr = firstSample;
	if (lastSampleNumPtr != NULL) *lastSampleNumPtr = lastSample;
	if (sizePtr != NULL) *sizePtr = size;

	if (stcbPtr && result)
	{
		stcbPtr->fGetSizeOfSamplesInChunk_chunkNumber = chunkNumber;
		stcbPtr->fGetSizeOfSamplesInChunk_firstSample = firstSample;
		stcbPtr->fGetSizeOfSamplesInChunk_lastSample = lastSample;
		stcbPtr->fGetSizeOfSamplesInChunk_size = size;
	}

	return result;
}


bool QTTrack::GetSample(UInt32 SampleNumber, char * Buffer, UInt32 * Length, QTFile_FileControlBlock * FCB, QTAtom_stsc_SampleTableControlBlock * STCB)
{
	// General vars
	UInt32      SampleDescriptionIndex;
	UInt64      SampleOffset;

	//
	// Get the location and size of this sample.
	if (!this->GetSampleInfo(SampleNumber, Length, &SampleOffset, &SampleDescriptionIndex, STCB))
		return false;

	//
	// Read in the sample
	if (!fDataReferenceAtom->Read(SampleDescriptionIndex, SampleOffset, Buffer, *Length, FCB))
		return false;

	//
	// The sample was successfully read in.
	return true;
}



// -------------------------------------
// Debugging functions
//
void QTTrack::DumpTrack()
{
	//
	// Dump this track's information.
	DEBUG_PRINT(("QTTrack::DumpTrack - Dumping track.\n"));
	DEBUG_PRINT(("QTTrack::DumpTrack - ..Track name: \"%s\".\n", fTrackName ? fTrackName : "<untitled>"));

	//
	// Dump the sub-atoms of this track.
	if (fTrackHeaderAtom != NULL)
		fTrackHeaderAtom->DumpAtom();

	if (fMediaHeaderAtom != NULL)
		fMediaHeaderAtom->DumpAtom();

	if (fDataReferenceAtom != NULL)
		fDataReferenceAtom->DumpAtom();

	if (fCompTimeToSampleAtom != NULL)
		fCompTimeToSampleAtom->DumpAtom();
	if (fTimeToSampleAtom != NULL)
		fTimeToSampleAtom->DumpAtom();
	if (fSampleToChunkAtom != NULL)
		fSampleToChunkAtom->DumpAtom();
	if (fSampleDescriptionAtom != NULL)
		fSampleDescriptionAtom->DumpAtom();
	if (fChunkOffsetAtom != NULL)
		fChunkOffsetAtom->DumpAtom();
	if (fSampleSizeAtom != NULL)
		fSampleSizeAtom->DumpAtom();
}
