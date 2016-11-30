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

#ifndef QTAtom_H
#define QTAtom_H


//
// Includes
#include "OSHeaders.h"

#include "QTFile.h"


//
// QTAtom class
class QTAtom {

public:
	//
	// Constructors and destructor.
	QTAtom(QTFile * File, QTFile::AtomTOCEntry * Atom,
		bool Debug = false, bool DeepDebug = false);
	virtual             ~QTAtom();


	//
	// Initialization functions.
	virtual bool      Initialize() { return true; }

	static SInt64   NTOH64(SInt64 networkOrdered)
	{
#if BIGENDIAN
		return networkOrdered;
#else
		return (SInt64)((UInt64)(networkOrdered << 56) | (UInt64)(((UInt64)0x00ff0000 << 32) & (networkOrdered << 40))
			| (UInt64)(((UInt64)0x0000ff00 << 32) & (networkOrdered << 24)) | (UInt64)(((UInt64)0x000000ff << 32) & (networkOrdered << 8))
			| (UInt64)(((UInt64)0x00ff0000 << 8) & (networkOrdered >> 8)) | (UInt64)((UInt64)0x00ff0000 & (networkOrdered >> 24))
			| (UInt64)((UInt64)0x0000ff00 & (networkOrdered >> 40)) | (UInt64)((UInt64)0x00ff & (networkOrdered >> 56)));
#endif
	}

	//
	// Read functions.
	bool      ReadBytes(UInt64 Offset, char* Buffer, UInt32 Length);
	bool      ReadInt8(UInt64 Offset, UInt8* Datum);
	bool      ReadInt16(UInt64 Offset, UInt16* Datum);
	bool      ReadInt32(UInt64 Offset, UInt32* Datum);
	bool      ReadInt64(UInt64 Offset, UInt64* Datum);
	bool      ReadInt32To64(UInt64 Offset, UInt64* Datum);
	bool		ReadInt32To64Signed(UInt64 Offset, SInt64* Datum);

	bool      ReadSubAtomBytes(const char* AtomPath, char* Buffer, UInt32 Length);
	bool      ReadSubAtomInt8(const char* AtomPath, UInt8* Datum);
	bool      ReadSubAtomInt16(const char* AtomPath, UInt16* Datum);
	bool      ReadSubAtomInt32(const char* AtomPath, UInt32* Datum);
	bool      ReadSubAtomInt64(const char* AtomPath, UInt64* Datum);

	char*       MemMap(UInt64 Offset, UInt32 Length);
	bool      UnMap(char* memPtr, UInt32 Length);
	//
	// Debugging functions.
	virtual void        DumpAtom() {}


protected:
	//
	// Protected member variables.
	bool              fDebug;
	bool				fDeepDebug;
	QTFile*				fFile;

	QTFile::AtomTOCEntry fTOCEntry;
};

#endif // QTAtom_H
