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
 // QTAtom_mdhd:
 //   The 'mdhd' QTAtom class.

#ifndef QTAtom_mdhd_H
#define QTAtom_mdhd_H


//
// Includes
#include "OSHeaders.h"

#include "QTFile.h"
#include "QTAtom.h"


//
// QTAtom class
class QTAtom_mdhd : public QTAtom {

public:
	//
	// Constructors and destructor.
	QTAtom_mdhd(QTFile * File, QTFile::AtomTOCEntry * Atom,
		bool Debug = false, bool DeepDebug = false);
	virtual             ~QTAtom_mdhd();


	//
	// Initialization functions.
	virtual bool      Initialize();

	//
	// Accessors.
	inline  Float64     GetTimeScale() { return (Float64)fTimeScale; }
	inline  Float64     GetTimeScaleRecip() { return fTimeScaleRecip; }

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
	UInt32      fTimeScale;
	UInt64      fDuration;
	UInt16      fLanguage;
	UInt16      fQuality;

	Float64     fTimeScaleRecip;
};

#endif // QTAtom_mdhd_H
