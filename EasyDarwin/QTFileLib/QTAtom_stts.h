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
// $Id: QTAtom_stts.h,v 1.1 2006/01/05 13:20:36 murata Exp $
//
// QTAtom_stts:
//   The 'stts' QTAtom class.

#ifndef QTAtom_stts_H
#define QTAtom_stts_H


//
// Includes
#include "QTFile.h"
#include "QTAtom.h"


//
// Class state cookie
class QTAtom_stts_SampleTableControlBlock {

public:
    //
    // Constructor and destructor.
                        QTAtom_stts_SampleTableControlBlock(void);
    virtual             ~QTAtom_stts_SampleTableControlBlock(void);
    
    //
    // Reset function
            void        Reset(void);

    //
    // MT->SN Sample table cache
    UInt32              fMTtSN_CurEntry;
    UInt32              fMTtSN_CurMediaTime, fMTtSN_CurSample;
    
    //
    /// SN->MT Sample table cache
    UInt32              fSNtMT_CurEntry;
    UInt32              fSNtMT_CurMediaTime, fSNtMT_CurSample;
    
    UInt32              fGetSampleMediaTime_SampleNumber;
    UInt32              fGetSampleMediaTime_MediaTime;

};


//
// QTAtom class
class QTAtom_stts : public QTAtom {

public:
    //
    // Constructors and destructor.
                        QTAtom_stts(QTFile * File, QTFile::AtomTOCEntry * Atom,
                               Bool16 Debug = false, Bool16 DeepDebug = false);
    virtual             ~QTAtom_stts(void);


    //
    // Initialization functions.
    virtual Bool16      Initialize(void);
    
    //
    // Accessors.
            Bool16      MediaTimeToSampleNumber(UInt32 MediaTime, UInt32 * SampleNumber,
                                                QTAtom_stts_SampleTableControlBlock * STCB);
            Bool16      SampleNumberToMediaTime(UInt32 SampleNumber, UInt32 * MediaTime,
                                                QTAtom_stts_SampleTableControlBlock * STCB);


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
    char        *fTimeToSampleTable;
    UInt32      fTableSize;
    
};

//
// Class state cookie
class QTAtom_ctts_SampleTableControlBlock {

public:
    //
    // Constructor and destructor.
                        QTAtom_ctts_SampleTableControlBlock(void);
    virtual             ~QTAtom_ctts_SampleTableControlBlock(void);
    
    //
    // Reset function
            void        Reset(void);

    //
    // MT->SN Sample table cache
    UInt32              fMTtSN_CurEntry;
    UInt32              fMTtSN_CurMediaTime, fMTtSN_CurSample;
    
    //
    /// SN->MT Sample table cache
    UInt32              fSNtMT_CurEntry;
    UInt32              fSNtMT_CurMediaTime, fSNtMT_CurSample;
    
    UInt32              fGetSampleMediaTime_SampleNumber;
    UInt32              fGetSampleMediaTime_MediaTime;

};


//
// QTAtom class
class QTAtom_ctts : public QTAtom {

public:
    //
    // Constructors and destructor.
                        QTAtom_ctts(QTFile * File, QTFile::AtomTOCEntry * Atom,
                               Bool16 Debug = false, Bool16 DeepDebug = false);
    virtual             ~QTAtom_ctts(void);


    //
    // Initialization functions.
    virtual Bool16      Initialize(void);
    
    //
    // Accessors.
            Bool16      MediaTimeToSampleNumber(UInt32 MediaTime, UInt32 * SampleNumber,
                                                QTAtom_ctts_SampleTableControlBlock * STCB);
            Bool16      SampleNumberToMediaTimeOffset(UInt32 SampleNumber, UInt32 * MediaTimeOffset,
                                                QTAtom_ctts_SampleTableControlBlock * STCB);


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
    char        *fTimeToSampleTable;
    
};

#endif // QTAtom_stts_H
