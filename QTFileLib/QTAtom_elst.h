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
// QTAtom_elst:
//   The 'elst' QTAtom class.

#ifndef QTAtom_elst_H
#define QTAtom_elst_H


//
// Includes
#include "QTFile.h"
#include "QTAtom.h"


//
// External classes
class QTFile_FileControlBlock;


//
// QTAtom class
class QTAtom_elst : public QTAtom {
    //
    // Class typedefs.
    struct EditListEntry {
        // Edit information
        UInt64          EditDuration;
        SInt64          StartingMediaTime;
        UInt32          EditMediaRate;
    };


public:
    //
    // Constructors and destructor.
                        QTAtom_elst(QTFile * File, QTFile::AtomTOCEntry * Atom,
                               Bool16 Debug = false, Bool16 DeepDebug = false);
    virtual             ~QTAtom_elst(void);


    //
    // Initialization functions.
    virtual Bool16      Initialize(void);

    //
    // Accessors.
    inline  UInt64      FirstEditMovieTime(void) { return fFirstEditMovieTime; }


    //
    // Debugging functions.
    virtual void        DumpAtom(void);


protected:
    //
    // Protected member variables.
    UInt8       fVersion;
    UInt32      fFlags; // 24 bits in the low 3 bytes

    UInt32          fNumEdits;
    EditListEntry   *fEdits;
    
    UInt64      fFirstEditMovieTime;
};

#endif // QTAtom_elst_H
