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
// $Id: QTAtom_stss.h,v 1.1 2006/01/05 13:20:36 murata Exp $
//
// QTAtom_stss:
//   The 'stss' QTAtom class.

#ifndef QTAtom_stss_H
#define QTAtom_stss_H


//
// Includes
#include "QTFile.h"
#include "QTAtom.h"
#include "MyAssert.h"


//
// QTAtom class
class QTAtom_stss : public QTAtom {

public:
    //
    // Constructors and destructor.
                        QTAtom_stss(QTFile * File, QTFile::AtomTOCEntry * Atom,
                               Bool16 Debug = false, Bool16 DeepDebug = false);
    virtual             ~QTAtom_stss(void);


    //
    // Initialization functions.
    virtual Bool16      Initialize(void);
    
    //
    // Accessors.
            void        PreviousSyncSample(UInt32 SampleNumber, UInt32 *SyncSampleNumber);
            void        NextSyncSample(UInt32 SampleNumber, UInt32 *SyncSampleNumber);
            inline Bool16       IsSyncSample(UInt32 SampleNumber, UInt32 inCursor)
            {
                Assert(inCursor <= fNumEntries);
                for (UInt32 curEntry = inCursor; curEntry < fNumEntries; curEntry++)
                {
                    if (fTable[curEntry] == SampleNumber)
                        return true;
                    else if (fTable[curEntry] > SampleNumber)
                        return false;
                }
                return false;
            }


    //
    // Debugging functions.
    virtual void        DumpAtom(void);
    virtual void        DumpTable(void);


protected:
    //
    // Protected member variables.
    UInt8       fVersion;
    UInt32      fFlags; // 24 bits in the low 3 bytes

    UInt32      fNumEntries;
    char        *fSyncSampleTable;
    UInt32      *fTable; // longword-aligned version of the above
    UInt32      fTableSize;
};

#endif // QTAtom_stss_H
