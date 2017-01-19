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
 // $Id: QTAtom_stts.cpp,v 1.2 2006/03/29 00:47:00 murata Exp $
 //
 // QTAtom_stts:
 //   The 'stts' QTAtom class.


 // -------------------------------------
 // Includes
 //
#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include <string.h>
#ifndef __Win32__
#include <sys/types.h>
#include <netinet/in.h>
#endif

#include "QTFile.h"

#include "QTAtom.h"
#include "QTAtom_stts.h"


// -------------------------------------
// Constants
//
const int       sttsPos_VersionFlags = 0;
const int       sttsPos_NumEntries = 4;
const int       sttsPos_SampleTable = 8;



// -------------------------------------
// Macros
//
#define DEBUG_PRINT(s) if(fDebug) qtss_printf s
#define DEEP_DEBUG_PRINT(s) if(fDeepDebug) qtss_printf s



// -------------------------------------
// Class state cookie
//
QTAtom_stts_SampleTableControlBlock::QTAtom_stts_SampleTableControlBlock()
{
	Reset();
}

QTAtom_stts_SampleTableControlBlock::~QTAtom_stts_SampleTableControlBlock()
{
}

void QTAtom_stts_SampleTableControlBlock::Reset()
{
	fMTtSN_CurEntry = 0;
	fMTtSN_CurMediaTime = 0;
	fMTtSN_CurSample = 1;

	fSNtMT_CurEntry = 0;
	fSNtMT_CurMediaTime = 0;
	fSNtMT_CurSample = 1;

	fGetSampleMediaTime_SampleNumber = 0;
	fGetSampleMediaTime_MediaTime = 0;

}



// -------------------------------------
// Constructors and destructors
//
QTAtom_stts::QTAtom_stts(QTFile * File, QTFile::AtomTOCEntry * TOCEntry, bool Debug, bool DeepDebug)
	: QTAtom(File, TOCEntry, Debug, DeepDebug),
	fNumEntries(0), fTimeToSampleTable(nullptr), fTableSize(0)
{
}

QTAtom_stts::~QTAtom_stts()
{
	//
	// Free our variables.
#if MMAP_TABLES
	if (fTimeToSampleTable != nullptr)
		this->UnMap(fTimeToSampleTable, fTableSize);
#else
	if (fTimeToSampleTable != nullptr)
		delete[] fTimeToSampleTable;
#endif


}



// -------------------------------------
// Initialization functions
//
bool QTAtom_stts::Initialize()
{
	// Temporary vars
	UInt32      tempInt32;


	//
	// Parse this atom's fields.
	ReadInt32(sttsPos_VersionFlags, &tempInt32);
	fVersion = (UInt8)((tempInt32 >> 24) & 0x000000ff);
	fFlags = tempInt32 & 0x00ffffff;

	ReadInt32(sttsPos_NumEntries, &fNumEntries);

	//
	// Validate the size of the sample table.
	if ((UInt32)(fNumEntries * 8) != (fTOCEntry.AtomDataLength - 8))
		return false;

	//
	// Read in the time-to-sample table.

#if MMAP_TABLES
	fTableSize = (fNumEntries * 8);
	fTimeToSampleTable = this->MemMap(sttsPos_SampleTable, fTableSize);
	if (fTimeToSampleTable == nullptr)
		return false;
#else
	fTimeToSampleTable = new char[fNumEntries * 8];
	if (fTimeToSampleTable == nullptr)
		return false;

	ReadBytes(sttsPos_SampleTable, fTimeToSampleTable, fNumEntries * 8);
#endif

	//
	// This atom has been successfully read in.
	return true;
}



// -------------------------------------
// Accessors
//
bool QTAtom_stts::MediaTimeToSampleNumber(UInt32 MediaTime, UInt32 * SampleNumber, QTAtom_stts_SampleTableControlBlock * STCB)
{
	// General vars
	UInt32      SampleCount, SampleDuration;
	QTAtom_stts_SampleTableControlBlock *tempSTCB = nullptr;
	bool      result = false;
	//
	// Use the default STCB if one was not passed in to us.
	if (STCB == nullptr)
	{
		//      qtss_printf("QTAtom_stts::MediaTimeToSampleNumber  ( STCB == nullptr ) \n");
		tempSTCB = new QTAtom_stts_SampleTableControlBlock;
		STCB = tempSTCB;
	}
	//
	// Reconfigure the STCB if necessary.
	if (MediaTime < STCB->fMTtSN_CurMediaTime)
	{
		//      qtss_printf(" QTAtom_stts::MediaTimeToSampleNumber RESET \n");
		STCB->Reset();
	}
	//
	// Linearly search through the sample table until we find the sample
	// which fits inside the given media time.
	for (; STCB->fMTtSN_CurEntry < fNumEntries; STCB->fMTtSN_CurEntry++) {
		//
		// Copy this sample count and duration.
		memcpy(&SampleCount, fTimeToSampleTable + (STCB->fMTtSN_CurEntry * 8), 4);
		SampleCount = ntohl(SampleCount);
		memcpy(&SampleDuration, fTimeToSampleTable + (STCB->fMTtSN_CurEntry * 8) + 4, 4);
		SampleDuration = ntohl(SampleDuration);

		//
		// Can we skip over this entry?
		if (STCB->fMTtSN_CurMediaTime + (SampleCount * SampleDuration) < MediaTime) {
			STCB->fMTtSN_CurMediaTime += SampleCount * SampleDuration;
			STCB->fMTtSN_CurSample += SampleCount;
			continue;
		}

		//
		// Locate and return the sample which is/begins right before the
		// given media time.
		if (SampleNumber != nullptr)
		{
			*SampleNumber = STCB->fMTtSN_CurSample;
			if (SampleDuration > 0)
				*SampleNumber += (MediaTime - STCB->fMTtSN_CurMediaTime) / SampleDuration;
			result = true;
			break;
		}
	}

	delete tempSTCB;

	return result;
}

bool QTAtom_stts::SampleNumberToMediaTime(UInt32 SampleNumber, UInt32 * MediaTime, QTAtom_stts_SampleTableControlBlock * STCB)
{
	// General vars
	UInt32      SampleCount, SampleDuration;
	//
	// Use the default STCB if one was not passed in to us.
	Assert(STCB != nullptr);

	if (STCB->fGetSampleMediaTime_SampleNumber == SampleNumber)
	{
		//      qtss_printf("QTTrack::GetSampleMediaTime cache hit SampleNumber %" _S32BITARG_ " \n", SampleNumber);
		*MediaTime = STCB->fGetSampleMediaTime_MediaTime;
		return true;
	}


	//
	// Reconfigure the STCB if necessary.
	if (SampleNumber < STCB->fSNtMT_CurSample)
	{
		//      qtss_printf(" QTAtom_stts::SampleNumberToMediaTime reset \n");
		STCB->Reset();
	}
	//
	// Linearly search through the sample table until we find the sample
	// which fits inside the given media time.
	for (; STCB->fSNtMT_CurEntry < fNumEntries; STCB->fSNtMT_CurEntry++) {
		//
		// Copy this sample count and duration.
		memcpy(&SampleCount, fTimeToSampleTable + (STCB->fSNtMT_CurEntry * 8), 4);
		SampleCount = ntohl(SampleCount);
		memcpy(&SampleDuration, fTimeToSampleTable + (STCB->fSNtMT_CurEntry * 8) + 4, 4);
		SampleDuration = ntohl(SampleDuration);

		//
		// Can we skip over this entry?
		if (STCB->fSNtMT_CurSample + SampleCount < SampleNumber) {
			STCB->fSNtMT_CurMediaTime += SampleCount * SampleDuration;
			STCB->fSNtMT_CurSample += SampleCount;
			continue;
		}

		//
		// Return the sample time at the beginning of this sample.
		if (MediaTime != nullptr)
			*MediaTime = STCB->fSNtMT_CurMediaTime + ((SampleNumber - STCB->fSNtMT_CurSample) * SampleDuration);

		STCB->fGetSampleMediaTime_SampleNumber = SampleNumber;
		STCB->fGetSampleMediaTime_MediaTime = *MediaTime;

		return true;
	}

	//
	// No match; return false.
	return false;
}



// -------------------------------------
// Debugging functions
//
void QTAtom_stts::DumpAtom()
{
	DEBUG_PRINT(("QTAtom_stts::DumpAtom - Dumping atom.\n"));
	DEBUG_PRINT(("QTAtom_stts::DumpAtom - ..Number of TTS entries: %" _S32BITARG_ "\n", fNumEntries));
}

void QTAtom_stts::DumpTable()
{
	//
	// Print out a header.
	qtss_printf("-- Time To Sample table -----------------------------------------------------------\n");
	qtss_printf("\n");
	qtss_printf("  Entry Num.   Sample Count   Sample Duration\n");
	qtss_printf("  ----------   ------------   ---------------\n");

	//
	// Print the table.
	UInt32      SampleCount = 0;
	UInt32      SampleDuration = 0;
	for (UInt32 CurEntry = 0; CurEntry < fNumEntries; CurEntry++)
	{
		//
		// Copy this sample count and duration.
		memcpy(&SampleCount, fTimeToSampleTable + (CurEntry * 8), 4);
		SampleCount = ntohl(SampleCount);
		memcpy(&SampleDuration, fTimeToSampleTable + (CurEntry * 8) + 4, 4);
		SampleDuration = ntohl(SampleDuration);

		// Print out a listing.
		qtss_printf("  %10"   _U32BITARG_   " : %10"   _U32BITARG_   "  %10"   _U32BITARG_   "\n", CurEntry, SampleCount, SampleDuration);
	}
}

// -------------------------------------
// Constants
//
const int       cttsPos_VersionFlags = 0;
const int       cttsPos_NumEntries = 4;
const int       cttsPos_SampleTable = 8;



// -------------------------------------
// Macros
//
#define DEBUG_PRINT(s) if(fDebug) qtss_printf s
#define DEEP_DEBUG_PRINT(s) if(fDeepDebug) qtss_printf s



// -------------------------------------
// Class state cookie
//
QTAtom_ctts_SampleTableControlBlock::QTAtom_ctts_SampleTableControlBlock()
{
	Reset();
}

QTAtom_ctts_SampleTableControlBlock::~QTAtom_ctts_SampleTableControlBlock()
{
}

void QTAtom_ctts_SampleTableControlBlock::Reset()
{
	fMTtSN_CurEntry = 0;
	fMTtSN_CurMediaTime = 0;
	fMTtSN_CurSample = 1;

	fSNtMT_CurEntry = 0;
	fSNtMT_CurMediaTime = 0;
	fSNtMT_CurSample = 1;

	fGetSampleMediaTime_SampleNumber = 0;
	fGetSampleMediaTime_MediaTime = 0;

}



// -------------------------------------
// Constructors and destructors
//
QTAtom_ctts::QTAtom_ctts(QTFile * File, QTFile::AtomTOCEntry * TOCEntry, bool Debug, bool DeepDebug)
	: QTAtom(File, TOCEntry, Debug, DeepDebug),
	fNumEntries(0), fTimeToSampleTable(nullptr)
{
}

QTAtom_ctts::~QTAtom_ctts()
{
	//
	// Free our variables.
	if (fTimeToSampleTable != nullptr)
		delete[] fTimeToSampleTable;
}



// -------------------------------------
// Initialization functions
//
bool QTAtom_ctts::Initialize()
{
	// Temporary vars
	UInt32      tempInt32;


	//
	// Parse this atom's fields.
	ReadInt32(cttsPos_VersionFlags, &tempInt32);
	fVersion = (UInt8)((tempInt32 >> 24) & 0x000000ff);
	fFlags = tempInt32 & 0x00ffffff;

	ReadInt32(cttsPos_NumEntries, &fNumEntries);

	//
	// Validate the size of the sample table.
	if ((UInt32)(fNumEntries * 8) != (fTOCEntry.AtomDataLength - 8))
		return false;

	//
	// Read in the time-to-sample table.
	fTimeToSampleTable = new char[fNumEntries * 8];
	if (fTimeToSampleTable == nullptr)
		return false;

	ReadBytes(cttsPos_SampleTable, fTimeToSampleTable, fNumEntries * 8);

	//
	// This atom has been successfully read in.
	return true;
}



// -------------------------------------
// Accessors
//
bool QTAtom_ctts::MediaTimeToSampleNumber(UInt32 MediaTime, UInt32 * SampleNumber, QTAtom_ctts_SampleTableControlBlock * STCB)
{
	// General vars
	UInt32      SampleCount, SampleDuration;
	QTAtom_ctts_SampleTableControlBlock *tempSTCB = nullptr;
	bool      result = false;
	//
	// Use the default STCB if one was not passed in to us.
	if (STCB == nullptr)
	{
		//      qtss_printf("QTAtom_ctts::MediaTimeToSampleNumber  ( STCB == nullptr ) \n");
		tempSTCB = new QTAtom_ctts_SampleTableControlBlock;
		STCB = tempSTCB;
	}
	//
	// Reconfigure the STCB if necessary.
	if (MediaTime < STCB->fMTtSN_CurMediaTime)
	{
		//      qtss_printf(" QTAtom_ctts::MediaTimeToSampleNumber RESET \n");
		STCB->Reset();
	}
	//
	// Linearly search through the sample table until we find the sample
	// which fits inside the given media time.
	for (; STCB->fMTtSN_CurEntry < fNumEntries; STCB->fMTtSN_CurEntry++) {
		//
		// Copy this sample count and duration.
		memcpy(&SampleCount, fTimeToSampleTable + (STCB->fMTtSN_CurEntry * 8), 4);
		SampleCount = ntohl(SampleCount);
		memcpy(&SampleDuration, fTimeToSampleTable + (STCB->fMTtSN_CurEntry * 8) + 4, 4);
		SampleDuration = ntohl(SampleDuration);

		//
		// Can we skip over this entry?
		if (STCB->fMTtSN_CurMediaTime + (SampleCount * SampleDuration) < MediaTime) {
			STCB->fMTtSN_CurMediaTime += SampleCount * SampleDuration;
			STCB->fMTtSN_CurSample += SampleCount;
			continue;
		}

		//
		// Locate and return the sample which is/begins right before the
		// given media time.
		if (SampleNumber != nullptr)
		{
			*SampleNumber = STCB->fMTtSN_CurSample;
			if (SampleDuration > 0)
				*SampleNumber += (MediaTime - STCB->fMTtSN_CurMediaTime) / SampleDuration;
			result = true;
			break;
		}
	}

	delete tempSTCB;

	return result;
}

bool QTAtom_ctts::SampleNumberToMediaTimeOffset(UInt32 SampleNumber, UInt32 * MediaTimeOffset, QTAtom_ctts_SampleTableControlBlock * STCB)
{
	// General vars
	UInt32      SampleCount, SampleOffset;
	//
	// Use the default STCB if one was not passed in to us.
	Assert(STCB != nullptr);

	if (STCB->fGetSampleMediaTime_SampleNumber == SampleNumber)
	{
		//      qtss_printf("QTTrack::GetSampleMediaTime cache hit SampleNumber %" _S32BITARG_ " \n", SampleNumber);
		*MediaTimeOffset = STCB->fGetSampleMediaTime_MediaTime;
		return true;
	}


	//
	// Reconfigure the STCB if necessary.
	if (SampleNumber < STCB->fSNtMT_CurSample)
	{
		//      qtss_printf(" QTAtom_ctts::SampleNumberToMediaTime reset \n");
		STCB->Reset();
	}
	//
	// Linearly search through the sample table until we find the sample
	// which fits inside the given media time.
	for (; STCB->fSNtMT_CurEntry < fNumEntries; STCB->fSNtMT_CurEntry++) {
		//
		// Copy this sample count and duration.
		memcpy(&SampleCount, fTimeToSampleTable + (STCB->fSNtMT_CurEntry * 8), 4);
		SampleCount = ntohl(SampleCount);
		memcpy(&SampleOffset, fTimeToSampleTable + (STCB->fSNtMT_CurEntry * 8) + 4, 4);
		SampleOffset = ntohl(SampleOffset);

		//
		// Can we skip over this entry?
		if (STCB->fSNtMT_CurSample + SampleCount < SampleNumber) {
			STCB->fSNtMT_CurMediaTime += SampleCount * SampleOffset;
			STCB->fSNtMT_CurSample += SampleCount;
			continue;
		}

		//
		// Return the sample time at the beginning of this sample.
		if (MediaTimeOffset != nullptr)
			*MediaTimeOffset = SampleOffset;

		STCB->fGetSampleMediaTime_SampleNumber = SampleNumber;
		STCB->fGetSampleMediaTime_MediaTime = *MediaTimeOffset;

		return true;
	}

	//
	// No match; return false.
	return false;
}



// -------------------------------------
// Debugging functions
//
void QTAtom_ctts::DumpAtom()
{
	DEBUG_PRINT(("QTAtom_ctts::DumpAtom - Dumping atom.\n"));
	DEBUG_PRINT(("QTAtom_ctts::DumpAtom - ..Number of CTTS entries: %" _S32BITARG_ "\n", fNumEntries));
}

void QTAtom_ctts::DumpTable()
{
	//
	// Print out a header.
	qtss_printf("-- Composition Time To Sample table ----------------------------------------------\n");
	qtss_printf("\n");
	qtss_printf("  Entry Num.   Sample Count   Sample Offset\n");
	qtss_printf("  ----------   ------------   ---------------\n");

	//
	// Print the table.
	UInt32      SampleCount = 0;
	UInt32      SampleOffset = 0;
	for (UInt32 CurEntry = 0; CurEntry < fNumEntries; CurEntry++)
	{
		//
		// Copy this sample count and duration.
		memcpy(&SampleCount, fTimeToSampleTable + (CurEntry * 8), 4);
		SampleCount = ntohl(SampleCount);
		memcpy(&SampleOffset, fTimeToSampleTable + (CurEntry * 8) + 4, 4);
		SampleOffset = ntohl(SampleOffset);

		// Print out a listing.
		qtss_printf("  %10" _U32BITARG_ " : %10" _U32BITARG_ "  %10" _U32BITARG_ "\n", CurEntry, SampleCount, SampleOffset);
	}
}
