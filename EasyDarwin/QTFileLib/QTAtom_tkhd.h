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
 // $Id: QTAtom_tkhd.h,v 1.1 2006/01/05 13:20:36 murata Exp $
 //
 // QTAtom_tkhd:
 //   The 'tkhd' QTAtom class.

#ifndef QTAtom_tkhd_H
#define QTAtom_tkhd_H


//
// Includes
#include "OSHeaders.h"

#include "QTFile.h"
#include "QTAtom.h"


//
// QTAtom class
class QTAtom_tkhd : public QTAtom {
	//
	// Class constants
	enum {
		flagEnabled = 0x00000001,
		flagInMovie = 0x00000002,
		flagInPreview = 0x00000004,
		flagInPoster = 0x00000008
	};


public:
	//
	// Constructors and destructor.
	QTAtom_tkhd(QTFile * File, QTFile::AtomTOCEntry * Atom,
		Bool16 Debug = false, Bool16 DeepDebug = false);
	virtual             ~QTAtom_tkhd();


	//
	// Initialization functions.
	virtual Bool16      Initialize();

	//
	// Accessors.
	inline  UInt32      GetTrackID() { return fTrackID; }
	inline  UInt32      GetFlags() { return fFlags; }
	inline  UInt64      GetCreationTime() { return fCreationTime; }
	inline  UInt64      GetModificationTime() { return fModificationTime; }
	inline  UInt64      GetDuration() { return fDuration; }


	//
	// Debugging functions.
	virtual void        DumpAtom();


protected:
	//
	// Protected member variables.
	UInt8       fVersion;
	UInt32      fFlags; // 24 bits in the low 3 bytes
	UInt64      fCreationTime, fModificationTime;
	UInt32      fTrackID;
	UInt32      freserved1;
	UInt64      fDuration;
	UInt32      freserved2, freserved3;
	UInt16      fLayer, fAlternateGroup;
	UInt16      fVolume;
	UInt16      freserved4;
	UInt32      fa, fb, fu, fc, fd, fv, fx, fy, fw;
	UInt32      fTrackWidth, fTrackHeight;
};

#endif // QTAtom_tkhd_H
