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


// -------------------------------------
// Includes
//
#include <stdio.h>
#include <time.h>
#include "SafeStdLib.h"
#include "QTFile.h"

#include "QTAtom.h"
#include "QTAtom_mdhd.h"



// -------------------------------------
// Constants
//
const int       mdhdPos_VersionFlags        =  0;
const int       mdhdPos_CreationTime        =  4;
const int       mdhdPos_ModificationTime    =  8;
const int       mdhdPos_TimeScale           = 12;
const int       mdhdPos_Duration            = 16;
const int       mdhdPos_Language            = 20;
const int       mdhdPos_Quality             = 22;

const int       mdhdPosV1_CreationTime      =  4;
const int       mdhdPosV1_ModificationTime  = 12;
const int       mdhdPosV1_TimeScale         = 20;
const int       mdhdPosV1_Duration          = 24;
const int       mdhdPosV1_Language          = 20 + 12;
const int       mdhdPosV1_Quality           = 22 + 12;



// -------------------------------------
// Macros
//
#define DEBUG_PRINT(s) if(fDebug) qtss_printf s
#define DEEP_DEBUG_PRINT(s) if(fDeepDebug) qtss_printf s



// -------------------------------------
// Constructors and destructors
//
QTAtom_mdhd::QTAtom_mdhd(QTFile * File, QTFile::AtomTOCEntry * TOCEntry, bool Debug, bool DeepDebug)
    : QTAtom(File, TOCEntry, Debug, DeepDebug)
{
}

QTAtom_mdhd::~QTAtom_mdhd()
{
}



// -------------------------------------
// Initialization functions
//
bool QTAtom_mdhd::Initialize()
{
    // Temporary vars
    UInt32      tempInt32;

    //
    // Parse this atom's fields.
    ReadInt32(mdhdPos_VersionFlags, &tempInt32);
    fVersion = (UInt8)((tempInt32 >> 24) & 0x000000ff);
    fFlags = tempInt32 & 0x00ffffff;

    if (0 == fVersion)
    {
        // Verify that this atom is the correct length.
        if( fTOCEntry.AtomDataLength != 24 )
        {
            DEEP_DEBUG_PRINT(("QTAtom_mdhd::Initialize failed. Expected AtomDataLength == 24 version: %d AtomDataLength: %" _64BITARG_ "u\n",fVersion, fTOCEntry.AtomDataLength));
            return false;
        }

        ReadInt32To64(mdhdPos_CreationTime, &fCreationTime);
        ReadInt32To64(mdhdPos_ModificationTime, &fModificationTime);
        ReadInt32(mdhdPos_TimeScale, &fTimeScale);
        ReadInt32To64(mdhdPos_Duration, &fDuration);
        ReadInt16(mdhdPos_Language, &fLanguage);
        ReadInt16(mdhdPos_Quality, &fQuality);
    }
    else if (1 == fVersion)
    {
        // Verify that this atom is the correct length.
        if( fTOCEntry.AtomDataLength != 36 )
        {
            DEEP_DEBUG_PRINT(("QTAtom_mdhd::Initialize failed. Expected AtomDataLength == 36 version: %d AtomDataLength: %" _64BITARG_ "u\n",fVersion, fTOCEntry.AtomDataLength));
            return false;
        }

        ReadInt64(mdhdPosV1_CreationTime, &fCreationTime);
        ReadInt64(mdhdPosV1_ModificationTime, &fModificationTime);
        ReadInt32(mdhdPosV1_TimeScale, &fTimeScale);
        ReadInt64(mdhdPosV1_Duration, &fDuration);
        ReadInt16(mdhdPosV1_Language, &fLanguage);
        ReadInt16(mdhdPosV1_Quality, &fQuality);
    }
    else
    {
        DEEP_DEBUG_PRINT(("QTAtom_mdhd::Initialize  failed. Version unsupported: %d\n",fVersion));
        return false;
    }

    //
    // Compute the reciprocal of the timescale.
    fTimeScaleRecip = 1 / (Float64)fTimeScale;
    
    //
    // This atom has been successfully read in.
    return true;
}



// -------------------------------------
// Debugging functions
//
void QTAtom_mdhd::DumpAtom()
{
    // Temporary vars
    time_t      unixCreationTime = (time_t)fCreationTime + (time_t)QT_TIME_TO_LOCAL_TIME;

    char buffer[kTimeStrSize];
    struct tm  timeResult;
    DEBUG_PRINT(("QTAtom_mdhd::DumpAtom - Dumping atom.\n"));
    DEBUG_PRINT(("QTAtom_mdhd::DumpAtom - ..Version: %d.\n", (int) fVersion));
    DEBUG_PRINT(("QTAtom_mdhd::DumpAtom - ..Creation date: %s", qtss_asctime(qtss_gmtime(&unixCreationTime, &timeResult),buffer,sizeof(buffer))));
}
