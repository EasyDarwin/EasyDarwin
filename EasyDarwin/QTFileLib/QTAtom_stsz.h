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
 // $Id: QTAtom_stsz.h,v 1.1 2006/01/05 13:20:36 murata Exp $
 //
 // QTAtom_stsz:
 //   The 'stsz' QTAtom class.

#ifndef QTAtom_stsz_H
#define QTAtom_stsz_H


//
// Includes
#include "QTFile.h"
#include "QTAtom.h"


//
// QTAtom class
class QTAtom_stsz : public QTAtom {

public:
	//
	// Constructors and destructor.
	QTAtom_stsz(QTFile * File, QTFile::AtomTOCEntry * Atom,
		Bool16 Debug = false, Bool16 DeepDebug = false);
	virtual             ~QTAtom_stsz();


	//
	// Initialization functions.
	virtual Bool16      Initialize();

	//
	// Accessors.
	inline  Bool16      SampleSize(UInt32 SampleNumber, UInt32 *Size = NULL) \
	{   if (fCommonSampleSize) {
		\
			if (Size != NULL) \
				*Size = fCommonSampleSize; \
				return true; \
	}
	else if (SampleNumber && (SampleNumber <= fNumEntries)) {
		\
			if (Size != NULL) \
				*Size = ntohl(fTable[SampleNumber - 1]); \
				return true; \
	}
	else \
		return false; \
	};

	Bool16      SampleRangeSize(UInt32 firstSampleNumber, UInt32 lastSampleNumber, UInt32 *sizePtr);

	//
	// Debugging functions.
	virtual void        DumpAtom();
	virtual void        DumpTable();

	inline  UInt32      GetNumEntries() { return fNumEntries; }
	inline  UInt32      GetCommonSampleSize() { return fCommonSampleSize; }

protected:
	//
	// Protected member variables.
	UInt8       fVersion;
	UInt32      fFlags; // 24 bits in the low 3 bytes
	UInt32      fCommonSampleSize;
	UInt32      fNumEntries;
	char        *fSampleSizeTable;
	UInt32      *fTable; // longword-aligned version of the above
};

#endif // QTAtom_stsz_H
