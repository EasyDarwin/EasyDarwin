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
// QTAtom_hinf:
//   The 'hinf' QTAtom class.

#ifndef QTAtom_hinf_H
#define QTAtom_hinf_H


//
// Includes
#include "QTFile.h"
#include "QTAtom.h"


//
// QTAtom class
class QTAtom_hinf : public QTAtom {

public:
    //
    // Constructors and destructor.
                        QTAtom_hinf(QTFile * File, QTFile::AtomTOCEntry * Atom,
                               Bool16 Debug = false, Bool16 DeepDebug = false);
    virtual             ~QTAtom_hinf(void);


    //
    // Initialization functions.
    virtual Bool16      Initialize(void);

    //
    // Accessors.
    inline  UInt64      GetTotalRTPBytes(void) { return  fTotalRTPBytes32 ? (UInt64) fTotalRTPBytes32 :  fTotalRTPBytes64 ; }
    inline  UInt64      GetTotalRTPPackets(void) { return  fTotalRTPPackets32 ? (UInt64) fTotalRTPPackets32 : fTotalRTPPackets64; }

    inline  UInt64      GetTotalPayLoadBytes(void) { return  fTotalPayLoadBytes32 ? (UInt64) fTotalPayLoadBytes32 : fTotalPayLoadBytes64; }

    inline  UInt64      GetMaxDataRate(void)        { return  fMaxDataRate64; }
    inline  UInt64      GetTotalMediaBytes(void)    { return  fTotalMediaBytes64; }
    inline  UInt64      GetTotalImmediateBytes(void)    { return  fTotalImmediateBytes64; }
    inline  UInt64      GetRepeatBytes(void)        { return  fTotalRepeatBytes64; }
    
    inline  UInt32      GetMinTransTime(void)       { return  fMinTransTime32; }
    inline  UInt32      GetMaxTransTime(void)       { return  fMaxTransTime32; }
    inline  UInt32      GetMaxPacketSizeBytes(void) { return  fMaxPacketSizeBytes32; }
    inline  UInt32      GetMaxPacketDuration(void)  { return  fMaxPacketDuration32; }
    inline  UInt32      GetPayLoadID(void)          { return  fPayloadID;}
    inline  char*       GetPayLoadStr(void)         { return  (char*) fPayloadStr;}

    //
    // Debugging functions.
    virtual void        DumpAtom(void);


protected:
    //
    // Protected member variables.
    UInt32      fTotalRTPBytes32; //totl
    UInt64      fTotalRTPBytes64; //trpy

    UInt32      fTotalRTPPackets32; //nump
    UInt64      fTotalRTPPackets64; //npck
    
    UInt32      fTotalPayLoadBytes32; //tpay
    UInt64      fTotalPayLoadBytes64; //tpyl
    UInt64      fMaxDataRate64; //maxr
    UInt64      fTotalMediaBytes64; //dmed
    UInt64      fTotalImmediateBytes64; //dimm  
    UInt64      fTotalRepeatBytes64; //drep
    
    UInt32      fMinTransTime32; //tmin
    UInt32      fMaxTransTime32; //tmax
    UInt32      fMaxPacketSizeBytes32; //pmax
    UInt32      fMaxPacketDuration32; //dmax
    UInt32      fPayloadID;//payt
    char        fPayloadStr[262];//payt
};

#endif // QTAtom_hinf_H
