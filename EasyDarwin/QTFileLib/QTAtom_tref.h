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
 // $Id: QTAtom_tref.h,v 1.1 2006/01/05 13:20:36 murata Exp $
 //
 // QTAtom_tref:
 //   The 'tref' QTAtom class.

#ifndef QTAtom_tref_H
#define QTAtom_tref_H

//
// Includes
#ifndef __Win32__
#include <netinet/in.h>
#endif

#include "QTFile.h"
#include "QTAtom.h"


//
// QTAtom class
class QTAtom_tref : public QTAtom {

public:
	//
	// Constructors and destructor.
	QTAtom_tref(QTFile * File, QTFile::AtomTOCEntry * Atom,
		bool Debug = false, bool DeepDebug = false);
	virtual             ~QTAtom_tref();


	//
	// Initialization functions.
	virtual bool      Initialize();

	//
	// Accessors.
	inline  UInt32      GetNumReferences() { return (UInt32)fNumEntries; }
	inline  bool      TrackReferenceToTrackID(UInt32 TrackReference, UInt32 * TrackID = NULL) \
	{   if (TrackReference < fNumEntries) {
		\
			if (TrackID != NULL) \
				*TrackID = ntohl(fTable[TrackReference]); \
				return true; \
	}
	else \
		return false; \
	}


	//
	// Debugging functions.
	virtual void        DumpAtom();


protected:
	//
	// Protected member variables.
	UInt64      fNumEntries;
	char        *fTrackReferenceTable;
	UInt32      *fTable; // longword-aligned version of the above
};

#endif // QTAtom_tref_H
