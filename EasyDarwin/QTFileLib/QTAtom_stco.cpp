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
 // QTAtom_stco:
 //   The 'stco' QTAtom class.


 // -------------------------------------
 // Includes
 //
#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include "QTFile.h"

#include "QTAtom.h"
#include "QTAtom_stco.h"


// -------------------------------------
// Constants
//
const int       stcoPos_VersionFlags = 0;
const int       stcoPos_NumEntries = 4;
const int       stcoPos_SampleTable = 8;



// -------------------------------------
// Macros
//
#define DEBUG_PRINT(s) if(fDebug) qtss_printf s
#define DEEP_DEBUG_PRINT(s) if(fDeepDebug) qtss_printf s



// -------------------------------------
// Constructors and destructors
//
QTAtom_stco::QTAtom_stco(QTFile * File, QTFile::AtomTOCEntry * TOCEntry, UInt16 offSetSize, bool Debug, bool DeepDebug)
	: QTAtom(File, TOCEntry, Debug, DeepDebug),
	fNumEntries(0), fOffSetSize(offSetSize), fChunkOffsetTable(NULL), fTable(NULL)
{
}

QTAtom_stco::~QTAtom_stco()
{
	//
	// Free our variables.
	if (fChunkOffsetTable != NULL)
		delete[] fChunkOffsetTable;
}



// -------------------------------------
// Initialization functions
//
bool QTAtom_stco::Initialize()
{
	// Temporary vars
	UInt32      tempInt32;


	//
	// Parse this atom's fields.
	ReadInt32(stcoPos_VersionFlags, &tempInt32);
	fVersion = (UInt8)((tempInt32 >> 24) & 0x000000ff);
	fFlags = tempInt32 & 0x00ffffff;

	ReadInt32(stcoPos_NumEntries, &fNumEntries);

	//
	// Validate the size of the sample table.
	if ((UInt32)(fNumEntries * fOffSetSize) != (fTOCEntry.AtomDataLength - 8))
		return false;

	//
	// Read in the chunk offset table.
	fChunkOffsetTable = new char[(fNumEntries * fOffSetSize) + 1];
	if (fChunkOffsetTable == NULL)
		return false;

	if (((PointerSizedInt)fChunkOffsetTable & (PointerSizedInt)0x3) == 0)
		fTable = (void *)fChunkOffsetTable;
	else
		fTable = (void *)(((PointerSizedInt)fChunkOffsetTable + 4) & ~((PointerSizedInt)0x3));

	ReadBytes(stcoPos_SampleTable, (char *)fTable, fNumEntries * fOffSetSize);

	//
	// This atom has been successfully read in.
	return true;
}



// -------------------------------------
// Debugging functions
//
void QTAtom_stco::DumpAtom()
{
	DEBUG_PRINT(("QTAtom_stco::DumpAtom - Dumping atom.\n"));
	DEBUG_PRINT(("QTAtom_stco::DumpAtom - ..Number of chunk offset entries: %" _S32BITARG_ "\n", fNumEntries));
}

void QTAtom_stco::DumpTable()
{
	//
	// Print out a header.
	qtss_printf("-- Chunk Offset table ----------------------------------------------------------\n");
	qtss_printf("\n");
	qtss_printf("  Chunk Num.     Offset\n");
	qtss_printf("  ----------   ----------\n");

	//
	// Print the table.
	UInt64 offset = 0;
	for (UInt32 CurEntry = 1; CurEntry <= fNumEntries; CurEntry++)
	{
		if (ChunkOffset(CurEntry, &offset))
			qtss_printf("  %10"   _U32BITARG_   ": %" _64BITARG_ "u\n", CurEntry, offset);
		else
			qtss_printf("  %10"   _U32BITARG_   ": QTAtom_stco::DumpTable ChunkOffset error\n", CurEntry);
	}
}
