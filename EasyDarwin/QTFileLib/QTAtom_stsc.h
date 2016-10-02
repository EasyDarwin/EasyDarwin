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
 // $Id: QTAtom_stsc.h,v 1.1 2006/01/05 13:20:36 murata Exp $
 //
 // QTAtom_stsc:
 //   The 'stsc' QTAtom class.

#ifndef QTAtom_stsc_H
#define QTAtom_stsc_H


//
// Includes
#include "QTFile.h"
#include "QTAtom.h"


//
// Class state cookie
class QTAtom_stsc_SampleTableControlBlock {

public:
	//
	// Constructor and destructor.
	QTAtom_stsc_SampleTableControlBlock();
	virtual             ~QTAtom_stsc_SampleTableControlBlock();

	//
	// Reset function
	void        Reset();

	//
	// Sample table cache
	UInt32              fCurEntry;
	UInt32              fCurSample;
	UInt32              fLastFirstChunk, fLastSamplesPerChunk, fLastSampleDescription;


	UInt32  fLastFirstChunk_GetChunkFirstLastSample;
	UInt32  fLastSamplesPerChunk_GetChunkFirstLastSample;
	UInt32  fLastTotalSamples_GetChunkFirstLastSample;

	UInt32 fCurEntry_GetChunkFirstLastSample;
	UInt32 chunkNumber_GetChunkFirstLastSample;
	UInt32 firstSample_GetChunkFirstLastSample;
	UInt32 lastSample_GetChunkFirstLastSample;


	UInt32 fFirstSampleNumber_SampleToChunkInfo;
	UInt32 fFirstSamplesPerChunk_SampleToChunkInfo;
	UInt32 fFirstChunkNumber_SampleToChunkInfo;
	UInt32 fFirstSampleDescriptionIndex_SampleToChunkInfo;
	UInt32 fFirstSampleOffsetInChunk_SampleToChunkInfo;

	UInt32  fCurEntry_SampleToChunkInfo;
	UInt32  fCurSample_SampleToChunkInfo;
	UInt32  fLastFirstChunk_SampleToChunkInfo;
	UInt32  fLastSamplesPerChunk_SampleToChunkInfo;
	UInt32  fLastSampleDescription_SampleToChunkInfo;

	UInt32  fGetSampleInfo_SampleNumber;
	UInt32  fGetSampleInfo_Length;
	UInt32  fGetSampleInfo_SampleDescriptionIndex;
	UInt64  fGetSampleInfo_Offset;
	UInt32  fGetSampleInfo_LastChunk;
	UInt32  fGetSampleInfo_LastChunkOffset;

	UInt32 fGetSizeOfSamplesInChunk_chunkNumber;
	UInt32 fGetSizeOfSamplesInChunk_firstSample;
	UInt32 fGetSizeOfSamplesInChunk_lastSample;
	UInt32 fGetSizeOfSamplesInChunk_size;


};


//
// QTAtom class
class QTAtom_stsc : public QTAtom {

public:
	//
	// Constructors and destructor.
	QTAtom_stsc(QTFile * File, QTFile::AtomTOCEntry * Atom,
		bool Debug = false, bool DeepDebug = false);
	virtual             ~QTAtom_stsc();


	//
	// Initialization functions.
	virtual bool      Initialize();

	//
	// Accessors.

	bool             GetChunkFirstLastSample(UInt32 chunkNumber, UInt32 *firstSample, UInt32 *lastSample, QTAtom_stsc_SampleTableControlBlock *STCB);

	bool             SampleToChunkInfo(UInt32 SampleNumber,
		UInt32 *samplesPerChunk = NULL,
		UInt32 *ChunkNumber = NULL,
		UInt32 *SampleDescriptionIndex = NULL,
		UInt32 *SampleOffsetInChunk = NULL,
		QTAtom_stsc_SampleTableControlBlock * STCB = NULL);


	inline  bool      SampleNumberToChunkNumber(UInt32 SampleNumber, UInt32 *ChunkNumber = NULL, UInt32 *SampleDescriptionIndex = NULL, UInt32 *SampleOffsetInChunk = NULL,
		QTAtom_stsc_SampleTableControlBlock * STCB = NULL)
	{
		return SampleToChunkInfo(SampleNumber, NULL /*samplesPerChunk*/, ChunkNumber, SampleDescriptionIndex, SampleOffsetInChunk, STCB);
	}

	UInt32  GetChunkFirstSample(UInt32 chunkNumber);
	//
	// Debugging functions.
	virtual void        DumpAtom();
	virtual void        DumpTable();


protected:
	//
	// Protected member variables.
	UInt8       fVersion;
	UInt32      fFlags; // 24 bits in the low 3 bytes

	UInt32      fNumEntries;
	char        *fSampleToChunkTable;
	UInt32      fTableSize;
};

#endif // QTAtom_stsc_H
