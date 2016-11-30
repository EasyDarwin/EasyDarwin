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
 // QTAtom:
 //   The base-class for atoms in a QuickTime file.


 // -------------------------------------
 // Includes
 //
#include <stdio.h>
#include "SafeStdLib.h"
#include <string.h>

#ifndef __Win32__
#include <sys/types.h>
#include <netinet/in.h>
#endif

#include "QTFile.h"
#include "QTAtom.h"



// -------------------------------------
// Macros
//
#define DEBUG_PRINT(s) if(fDebug) qtss_printf s
#define DEEP_DEBUG_PRINT(s) if(fDeepDebug) qtss_printf s



// -------------------------------------
// Constructors and destructors
//
QTAtom::QTAtom(QTFile * File, QTFile::AtomTOCEntry * Atom, bool Debug, bool DeepDebug)
	: fDebug(Debug), fDeepDebug(DeepDebug),
	fFile(File)
{
	//
	// Make a copy of the TOC entry.
	memcpy(&fTOCEntry, Atom, sizeof(QTFile::AtomTOCEntry));
}

QTAtom::~QTAtom(void)
{
}



// -------------------------------------
// Read functions
//
bool QTAtom::ReadBytes(UInt64 Offset, char * Buffer, UInt32 Length)
{
	//
	// Validate the arguments.
	if ((Offset + Length) > fTOCEntry.AtomDataLength)
		return false;

	//
	// Read and return this data.
	return fFile->Read(fTOCEntry.AtomDataPos + Offset, Buffer, Length);
}

char *QTAtom::MemMap(UInt64 Offset, UInt32 Length)
{
	//
	// Validate the arguments.
	if ((Offset + Length) > fTOCEntry.AtomDataLength)
		return NULL;

	//
	// Read and return this data.
	return fFile->MapFileToMem(fTOCEntry.AtomDataPos + Offset, Length);
}


bool QTAtom::UnMap(char *memPtr, UInt32 Length)
{
	if (-1 == fFile->UnmapMem(memPtr, Length))
		return false;

	return true;
}


bool QTAtom::ReadInt8(UInt64 Offset, UInt8 * Datum)
{
	//
	// Read and return.
	return ReadBytes(Offset, (char *)Datum, 1);
}

bool QTAtom::ReadInt16(UInt64 Offset, UInt16 * Datum)
{
	// General vars
	UInt16      tempDatum;


	//
	// Read and flip.
	if (!ReadBytes(Offset, (char *)&tempDatum, 2))
		return false;

	*Datum = ntohs(tempDatum);
	return true;
}

bool QTAtom::ReadInt32(UInt64 Offset, UInt32 * Datum)
{
	// General vars
	UInt32      tempDatum;


	//
	// Read and flip.
	if (!ReadBytes(Offset, (char *)&tempDatum, 4))
		return false;

	*Datum = ntohl(tempDatum);
	return true;
}

bool QTAtom::ReadInt32To64(UInt64 Offset, UInt64 * Datum)
{
	// General vars
	UInt32      tempDatum;


	//
	// Read and flip.
	if (!ReadBytes(Offset, (char *)&tempDatum, 4))
		return false;

	tempDatum = ntohl(tempDatum);
	*Datum = (UInt64)tempDatum;
	return true;
}

bool QTAtom::ReadInt32To64Signed(UInt64 Offset, SInt64 * Datum)
{
	// General vars
	UInt32		tempDatum;

	//
	// Read and flip.
	if (!ReadBytes(Offset, (char *)&tempDatum, 4))
		return false;

	tempDatum = ntohl(tempDatum);
	*Datum = (SInt64)(SInt32)tempDatum;
	return true;
}



bool QTAtom::ReadInt64(UInt64 Offset, UInt64 * Datum)
{
	// General vars
	UInt64      tempDatum;


	//
	// Read and flip.
	if (!ReadBytes(Offset, (char *)&tempDatum, 8))
		return false;

	*Datum = (UInt64)QTAtom::NTOH64(tempDatum);
	return true;
}


bool QTAtom::ReadSubAtomBytes(const char * AtomPath, char * Buffer, UInt32 Length)
{
	// General vars
	QTFile::AtomTOCEntry    *atomTOCEntry;


	//
	// Find the TOC entry for this sub-atom.
	if (!fFile->FindTOCEntry(AtomPath, &atomTOCEntry, &fTOCEntry))
		return false;

	//
	// Validate the arguments.
	if ((atomTOCEntry->AtomDataPos <= fTOCEntry.AtomDataPos) || ((atomTOCEntry->AtomDataPos + Length) > (fTOCEntry.AtomDataPos + fTOCEntry.AtomDataLength)))
		return false;

	//
	// Read and return this data.
	return fFile->Read(atomTOCEntry->AtomDataPos, Buffer, Length);
}

bool QTAtom::ReadSubAtomInt8(const char * AtomPath, UInt8 * Datum)
{
	//
	// Read and return.
	return ReadSubAtomBytes(AtomPath, (char *)Datum, 1);
}

bool QTAtom::ReadSubAtomInt16(const char * AtomPath, UInt16 * Datum)
{
	// General vars
	UInt16      tempDatum;


	//
	// Read and flip.
	if (!ReadSubAtomBytes(AtomPath, (char *)&tempDatum, 2))
		return false;

	*Datum = ntohs(tempDatum);
	return true;
}

bool QTAtom::ReadSubAtomInt32(const char * AtomPath, UInt32 * Datum)
{
	// General vars
	UInt32      tempDatum;


	//
	// Read and flip.
	if (!ReadSubAtomBytes(AtomPath, (char *)&tempDatum, 4))
		return false;

	*Datum = ntohl(tempDatum);
	return true;
}

bool QTAtom::ReadSubAtomInt64(const char * AtomPath, UInt64 * Datum)
{
	// General vars
	UInt64      tempDatum;


	//
	// Read and flip.
	if (!ReadSubAtomBytes(AtomPath, (char *)&tempDatum, 8))
		return false;

	*Datum = (UInt64)QTAtom::NTOH64(tempDatum);
	return true;
}

