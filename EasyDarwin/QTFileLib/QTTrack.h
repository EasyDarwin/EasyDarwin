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

#ifndef QTTrack_H
#define QTTrack_H


//
// Includes
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


//
// External classes
class QTFile;
class QTFile_FileControlBlock;
class QTAtom_stsc_SampleTableControlBlock;
class QTAtom_stts_SampleTableControlBlock;


//
// QTTrack class
class QTTrack {

public:
	//
	// Class error codes
	enum ErrorCode {
		errNoError = 0,
		errInvalidQuickTimeFile = 1,
		errParamError = 2,
		errIsSkippedPacket = 3,
		errInternalError = 100
	};


public:
	//
	// Constructors and destructor.
	QTTrack(QTFile * File, QTFile::AtomTOCEntry * trakAtom,
		bool Debug = false, bool DeepDebug = false);
	virtual             ~QTTrack();


	//
	// Initialization functions.
	virtual ErrorCode   Initialize();

	//
	// Accessors.
	inline  bool      IsInitialized() { return fIsInitialized; }

	inline  const char *GetTrackName() { return (fTrackName ? fTrackName : ""); }
	inline  UInt32      GetTrackID() { return fTrackHeaderAtom->GetTrackID(); }
	inline  UInt64      GetCreationTime() { return fTrackHeaderAtom->GetCreationTime(); }
	inline  UInt64      GetModificationTime() { return fTrackHeaderAtom->GetModificationTime(); }
	inline  SInt64      GetDuration() { return (SInt64)fTrackHeaderAtom->GetDuration(); }
	inline  Float64     GetTimeScale() { return fMediaHeaderAtom->GetTimeScale(); }
	inline  Float64     GetTimeScaleRecip() { return fMediaHeaderAtom->GetTimeScaleRecip(); }
	inline  Float64     GetDurationInSeconds() { return GetDuration() / (Float64)GetTimeScale(); }
	inline  UInt64      GetFirstEditMovieTime(void)
	{
		if (fEditListAtom != NULL) return fEditListAtom->FirstEditMovieTime();
		else return 0;
	}
	inline  UInt32      GetFirstEditMediaTime() { return fFirstEditMediaTime; }

	//
	// Sample functions
	bool              GetSizeOfSamplesInChunk(UInt32 chunkNumber, UInt32 * const sizePtr, UInt32 * const firstSampleNumPtr, UInt32 * const lastSampleNumPtr, QTAtom_stsc_SampleTableControlBlock * stcbPtr);

	inline  bool      GetChunkFirstLastSample(UInt32 chunkNumber, UInt32 *firstSample, UInt32 *lastSample,
		QTAtom_stsc_SampleTableControlBlock *STCB)
	{
		return fSampleToChunkAtom->GetChunkFirstLastSample(chunkNumber, firstSample, lastSample, STCB);
	}


	inline  bool      SampleToChunkInfo(UInt32 SampleNumber, UInt32 *samplesPerChunk, UInt32 *ChunkNumber, UInt32 *SampleDescriptionIndex, UInt32 *SampleOffsetInChunk,
		QTAtom_stsc_SampleTableControlBlock * STCB)
	{
		return fSampleToChunkAtom->SampleToChunkInfo(SampleNumber, samplesPerChunk, ChunkNumber, SampleDescriptionIndex, SampleOffsetInChunk, STCB);
	}


	inline  bool      SampleNumberToChunkNumber(UInt32 SampleNumber, UInt32 *ChunkNumber, UInt32 *SampleDescriptionIndex, UInt32 *SampleOffsetInChunk,
		QTAtom_stsc_SampleTableControlBlock * STCB)
	{
		return fSampleToChunkAtom->SampleNumberToChunkNumber(SampleNumber, ChunkNumber, SampleDescriptionIndex, SampleOffsetInChunk, STCB);
	}


	inline  UInt32      GetChunkFirstSample(UInt32 chunkNumber)
	{
		return fSampleToChunkAtom->GetChunkFirstSample(chunkNumber);
	}

	inline  bool      ChunkOffset(UInt32 ChunkNumber, UInt64 *Offset = NULL)
	{
		return fChunkOffsetAtom->ChunkOffset(ChunkNumber, Offset);
	}

	inline  bool      SampleSize(UInt32 SampleNumber, UInt32 *Size = NULL)
	{
		return fSampleSizeAtom->SampleSize(SampleNumber, Size);
	}

	inline  bool      SampleRangeSize(UInt32 firstSample, UInt32 lastSample, UInt32 *sizePtr = NULL)
	{
		return fSampleSizeAtom->SampleRangeSize(firstSample, lastSample, sizePtr);
	}

	bool      GetSampleInfo(UInt32 SampleNumber, UInt32 * const Length, UInt64 * const Offset, UInt32 * const SampleDescriptionIndex,
		QTAtom_stsc_SampleTableControlBlock * STCB);

	bool      GetSample(UInt32 SampleNumber, char * Buffer, UInt32 * Length, QTFile_FileControlBlock * FCB,
		QTAtom_stsc_SampleTableControlBlock * STCB);

	inline  bool      GetSampleMediaTime(UInt32 SampleNumber, UInt32 * const MediaTime,
		QTAtom_stts_SampleTableControlBlock * STCB)
	{
		return fTimeToSampleAtom->SampleNumberToMediaTime(SampleNumber, MediaTime, STCB);
	}

	inline  bool      GetSampleNumberFromMediaTime(UInt32 MediaTime, UInt32 * const SampleNumber,
		QTAtom_stts_SampleTableControlBlock * STCB)
	{
		return fTimeToSampleAtom->MediaTimeToSampleNumber(MediaTime, SampleNumber, STCB);
	}


	inline  void        GetPreviousSyncSample(UInt32 SampleNumber, UInt32 * SyncSampleNumber)
	{
		if (fSyncSampleAtom != NULL) fSyncSampleAtom->PreviousSyncSample(SampleNumber, SyncSampleNumber);
		else *SyncSampleNumber = SampleNumber;
	}

	inline  void        GetNextSyncSample(UInt32 SampleNumber, UInt32 * SyncSampleNumber)
	{
		if (fSyncSampleAtom != NULL) fSyncSampleAtom->NextSyncSample(SampleNumber, SyncSampleNumber);
		else *SyncSampleNumber = SampleNumber + 1;
	}

	inline bool           IsSyncSample(UInt32 SampleNumber, UInt32 SyncSampleCursor)
	{
		if (fSyncSampleAtom != NULL) return fSyncSampleAtom->IsSyncSample(SampleNumber, SyncSampleCursor);
		else return true;
	}
	//
	// Read functions.
	inline  bool      Read(UInt32 SampleDescriptionID, UInt64 Offset, char * const Buffer, UInt32 Length,
		QTFile_FileControlBlock * FCB = NULL)
	{
		return fDataReferenceAtom->Read(fSampleDescriptionAtom->SampleDescriptionToDataReference(SampleDescriptionID), Offset, Buffer, Length, FCB);
	}

	inline bool       GetSampleMediaTimeOffset(UInt32 SampleNumber, UInt32 *mediaTimeOffset, QTAtom_ctts_SampleTableControlBlock * STCB)
	{
		if (fCompTimeToSampleAtom)
			return fCompTimeToSampleAtom->SampleNumberToMediaTimeOffset(SampleNumber, mediaTimeOffset, STCB);
		else
			return false;
	}
	//
	// Debugging functions.
	virtual void        DumpTrack();
	inline  void        DumpSampleToChunkTable() { fSampleToChunkAtom->DumpTable(); }
	inline  void        DumpChunkOffsetTable() { fChunkOffsetAtom->DumpTable(); }
	inline  void        DumpSampleSizeTable() { fSampleSizeAtom->DumpTable(); }
	inline  void        DumpTimeToSampleTable() { fTimeToSampleAtom->DumpTable(); }
	inline  void        DumpCompTimeToSampleTable() { if (fCompTimeToSampleAtom) fCompTimeToSampleAtom->DumpTable(); else printf("*** no ctts table ****\n"); }


protected:
	//
	// Protected member variables.
	bool              fDebug, fDeepDebug;
	QTFile              *fFile;
	QTFile::AtomTOCEntry fTOCEntry;

	bool              fIsInitialized;

	QTAtom_tkhd         *fTrackHeaderAtom;
	char                *fTrackName;

	QTAtom_mdhd         *fMediaHeaderAtom;

	QTAtom_elst         *fEditListAtom;
	QTAtom_dref         *fDataReferenceAtom;

	QTAtom_stts         *fTimeToSampleAtom;
	QTAtom_ctts         *fCompTimeToSampleAtom;
	QTAtom_stsc         *fSampleToChunkAtom;
	QTAtom_stsd         *fSampleDescriptionAtom;
	QTAtom_stco         *fChunkOffsetAtom;
	QTAtom_stsz         *fSampleSizeAtom;
	QTAtom_stss         *fSyncSampleAtom;

	UInt32              fFirstEditMediaTime;
};

#endif // QTTrack_H
