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
 // $Id: QTAtom_stss.cpp,v 1.2 2006/03/29 00:47:00 murata Exp $
 //
 // QTAtom_stss:
 //   The 'stss' QTAtom class.


 // -------------------------------------
 // Includes
 //
#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#ifndef __Win32__
#include <sys/types.h>
#include <netinet/in.h>
#endif

#include "QTFile.h"

#include "QTAtom.h"
#include "QTAtom_stss.h"
#include "OSMemory.h"


// -------------------------------------
// Constants
//
const int       stssPos_VersionFlags = 0;
const int       stssPos_NumEntries = 4;
const int       stssPos_SampleTable = 8;



// -------------------------------------
// Macros
//
#define DEBUG_PRINT(s) if(fDebug) qtss_printf s
#define DEEP_DEBUG_PRINT(s) if(fDeepDebug) qtss_printf s



// -------------------------------------
// Constructors and destructors
//
QTAtom_stss::QTAtom_stss(QTFile * File, QTFile::AtomTOCEntry * TOCEntry, Bool16 Debug, Bool16 DeepDebug)
	: QTAtom(File, TOCEntry, Debug, DeepDebug),
	fNumEntries(0), fSyncSampleTable(NULL), fTable(NULL), fTableSize(0)
{
}

QTAtom_stss::~QTAtom_stss()
{
	//
	// Free our variables.
#if MMAP_TABLES
	if (fSyncSampleTable != NULL)
		this->UnMap(fSyncSampleTable, fTableSize);
#else
	if (fSyncSampleTable != NULL)
		delete[] fSyncSampleTable;
#endif
}



// -------------------------------------
// Initialization functions
//
Bool16 QTAtom_stss::Initialize()
{
	Bool16      initSucceeds = false;
	UInt32      tempInt32;


	//
	// Parse this atom's fields.
	initSucceeds = ReadInt32(stssPos_VersionFlags, &tempInt32);
	fVersion = (UInt8)((tempInt32 >> 24) & 0x000000ff);
	fFlags = tempInt32 & 0x00ffffff;

	if (initSucceeds)
	{
		initSucceeds = ReadInt32(stssPos_NumEntries, &fNumEntries);

		//
		// Validate the size of the sample table.
		if ((UInt32)(fNumEntries * 4) != (fTOCEntry.AtomDataLength - 8))
			return false;


#if 0// MMAP_TABLES needs fixing should be page aligned and maybe on a 64bit system the whole file should be mapped.
		fTableSize = (fNumEntries * 4);
		fSyncSampleTable = this->MemMap(stssPos_SampleTable, fTableSize);
		fTable = (UInt32 *)fSyncSampleTable;
		if (fSyncSampleTable == NULL)
			return false;

#else
		//
		// Read in the sync sample table.
		fSyncSampleTable = NEW char[(fNumEntries * 4) + 1];
		if (fSyncSampleTable == NULL)
			return false;

		if (((PointerSizedInt)fSyncSampleTable & (PointerSizedInt)0x3) == 0)
			fTable = (UInt32 *)fSyncSampleTable;
		else
			fTable = (UInt32 *)(((PointerSizedInt)fSyncSampleTable + 4) & ~((PointerSizedInt)0x3));

		initSucceeds = ReadBytes(stssPos_SampleTable, (char *)fTable, fNumEntries * 4);

#endif

		if (initSucceeds)
		{
			// This atom has been successfully read in.
			// sample offsets are in network byte order on disk, convert them to host order
			UInt32      sampleIndex = 0;

			// convert each sample to host order
			// NOTE - most other Atoms handle byte order conversions in
			// the accessor function.  For efficiency reasons it's converted
			// to host order here for sync samples.

			for (sampleIndex = 0; sampleIndex < fNumEntries; sampleIndex++)
			{
				fTable[sampleIndex] = ntohl(fTable[sampleIndex]);

			}

		}



	}

	return initSucceeds;
}



// -------------------------------------
// Accessors
//
void QTAtom_stss::PreviousSyncSample(UInt32 SampleNumber, UInt32 *SyncSampleNumber)
{
	//
	// We assume that we won't find an answer
	*SyncSampleNumber = SampleNumber;

	//
	// Scan the table until we find a sample number greater than our current
	// sample number; then return that.
	for (UInt32 CurEntry = 0; CurEntry < fNumEntries; CurEntry++) {
		//
		// Take this entry if it is before (or equal to) our current entry.
		if (fTable[CurEntry] <= SampleNumber)
			*SyncSampleNumber = fTable[CurEntry];
	}
}

void QTAtom_stss::NextSyncSample(UInt32 SampleNumber, UInt32 *SyncSampleNumber)
{
	//
	// We assume that we won't find an answer
	*SyncSampleNumber = SampleNumber + 1;

	//
	// Scan the table until we find a sample number greater than our current
	// sample number; then return that.
	for (UInt32 CurEntry = 0; CurEntry < fNumEntries; CurEntry++) {
		//
		// Take this entry if it is greater than our current entry.
		if (fTable[CurEntry] > SampleNumber) {
			*SyncSampleNumber = fTable[CurEntry];
			break;
		}
	}
}


// -------------------------------------
// Debugging functions
//
void QTAtom_stss::DumpAtom()
{
	DEBUG_PRINT(("QTAtom_stss::DumpAtom - Dumping atom.\n"));
	DEBUG_PRINT(("QTAtom_stss::DumpAtom - ..Number of sync sample entries: %" _S32BITARG_ "\n", fNumEntries));
}

void QTAtom_stss::DumpTable()
{
	//
	// Print out a header.
	qtss_printf("-- Sync Sample table -----------------------------------------------------------\n");
	qtss_printf("\n");
	qtss_printf("  Entry Num.   Sample Num\n");
	qtss_printf("  ----------   ----------\n");

	//
	// Print the table.
	for (UInt32 CurEntry = 1; CurEntry <= fNumEntries; CurEntry++) {
		//
		// Print out a listing.
		qtss_printf("  %10"   _U32BITARG_   " : %10"   _U32BITARG_   "\n", CurEntry, fTable[CurEntry - 1]);
	}
}
