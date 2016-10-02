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
 // $Id: QTAtom_tkhd.cpp,v 1.1 2006/01/05 13:20:36 murata Exp $
 //
 // QTAtom_tkhd:
 //   The 'tkhd' QTAtom class.


 // -------------------------------------
 // Includes
 //
#include <stdio.h>
#include <time.h>

#include "QTFile.h"

#include "QTAtom.h"
#include "QTAtom_tkhd.h"



// -------------------------------------
// Constants
//
const int       tkhdPos_VersionFlags = 0;
const int       tkhdPos_CreationTime = 4;
const int       tkhdPos_ModificationTime = 8;
const int       tkhdPos_TrackID = 12;
const int       tkhdPos_Duration = 20;
const int       tkhdPos_Layer = 32;
const int       tkhdPos_AlternateGroup = 34;
const int       tkhdPos_Volume = 36;
const int       tkhdPos_a = 40;
const int       tkhdPos_b = 44;
const int       tkhdPos_u = 48;
const int       tkhdPos_c = 52;
const int       tkhdPos_d = 56;
const int       tkhdPos_v = 60;
const int       tkhdPos_x = 64;
const int       tkhdPos_y = 68;
const int       tkhdPos_w = 72;
const int       tkhdPos_TrackWidth = 76;
const int       tkhdPos_TrackHeight = 80;

const int       tkhdPosV1_CreationTime = 4;
const int       tkhdPosV1_ModificationTime = 12;
const int       tkhdPosV1_TrackID = 20;
const int       tkhdPosV1_Duration = 28;
const int       tkhdPosV1_Layer = 44;
const int       tkhdPosV1_AlternateGroup = 34 + 12;
const int       tkhdPosV1_Volume = 36 + 12;
const int       tkhdPosV1_a = 40 + 12;
const int       tkhdPosV1_b = 44 + 12;
const int       tkhdPosV1_u = 48 + 12;
const int       tkhdPosV1_c = 52 + 12;
const int       tkhdPosV1_d = 56 + 12;
const int       tkhdPosV1_v = 60 + 12;
const int       tkhdPosV1_x = 64 + 12;
const int       tkhdPosV1_y = 68 + 12;
const int       tkhdPosV1_w = 72 + 12;
const int       tkhdPosV1_TrackWidth = 76 + 12;
const int       tkhdPosV1_TrackHeight = 80 + 12;


// -------------------------------------
// Macros
//
#define DEBUG_PRINT(s) if(fDebug) qtss_printf s
#define DEEP_DEBUG_PRINT(s) if(fDeepDebug) qtss_printf s



// -------------------------------------
// Constructors and destructors
//
QTAtom_tkhd::QTAtom_tkhd(QTFile * File, QTFile::AtomTOCEntry * TOCEntry, bool Debug, bool DeepDebug)
	: QTAtom(File, TOCEntry, Debug, DeepDebug)
{
}

QTAtom_tkhd::~QTAtom_tkhd()
{
}



// -------------------------------------
// Initialization functions
//
bool QTAtom_tkhd::Initialize()
{
	// Temporary vars
	UInt32      tempInt32;

	//
	// Parse this atom's fields.
	ReadInt32(tkhdPos_VersionFlags, &tempInt32);
	fVersion = (UInt8)((tempInt32 >> 24) & 0x000000ff);
	fFlags = tempInt32 & 0x00ffffff;

	if (0 == fVersion)
	{
		// Verify that this atom is the correct length.
		if (fTOCEntry.AtomDataLength != 84)
		{
			DEEP_DEBUG_PRINT(("QTAtom_tkhd::Initialize failed. Expected AtomDataLength == 84 version: %d AtomDataLength: %" _64BITARG_ "u\n", fVersion, fTOCEntry.AtomDataLength));
			return false;
		}

		ReadInt32To64(tkhdPos_CreationTime, &fCreationTime);
		ReadInt32To64(tkhdPos_ModificationTime, &fModificationTime);
		ReadInt32(tkhdPos_TrackID, &fTrackID);
		ReadInt32To64(tkhdPos_Duration, &fDuration);
		ReadInt16(tkhdPos_AlternateGroup, &fAlternateGroup);
		ReadInt16(tkhdPos_Volume, &fVolume);

		ReadInt32(tkhdPos_a, &fa);
		ReadInt32(tkhdPos_b, &fb);
		ReadInt32(tkhdPos_u, &fu);
		ReadInt32(tkhdPos_c, &fc);
		ReadInt32(tkhdPos_d, &fd);
		ReadInt32(tkhdPos_v, &fv);
		ReadInt32(tkhdPos_x, &fx);
		ReadInt32(tkhdPos_y, &fy);
		ReadInt32(tkhdPos_w, &fw);

		ReadInt32(tkhdPos_TrackWidth, &fTrackWidth);
		ReadInt32(tkhdPos_TrackHeight, &fTrackHeight);
	}
	else if (1 == fVersion)
	{
		// Verify that this atom is the correct length.
		if (fTOCEntry.AtomDataLength != 96)
		{
			DEEP_DEBUG_PRINT(("QTAtom_tkhd::Initialize failed. Expected AtomDataLength == 96 version: %d AtomDataLength: %" _64BITARG_ "u\n", fVersion, fTOCEntry.AtomDataLength));
			return false;
		}

		ReadInt64(tkhdPosV1_CreationTime, &fCreationTime);
		ReadInt64(tkhdPosV1_ModificationTime, &fModificationTime);
		ReadInt32(tkhdPosV1_TrackID, &fTrackID);
		ReadInt64(tkhdPosV1_Duration, &fDuration);
		ReadInt16(tkhdPosV1_AlternateGroup, &fAlternateGroup);
		ReadInt16(tkhdPosV1_Volume, &fVolume);

		ReadInt32(tkhdPosV1_a, &fa);
		ReadInt32(tkhdPosV1_b, &fb);
		ReadInt32(tkhdPosV1_u, &fu);
		ReadInt32(tkhdPosV1_c, &fc);
		ReadInt32(tkhdPosV1_d, &fd);
		ReadInt32(tkhdPosV1_v, &fv);
		ReadInt32(tkhdPosV1_x, &fx);
		ReadInt32(tkhdPosV1_y, &fy);
		ReadInt32(tkhdPosV1_w, &fw);

		ReadInt32(tkhdPosV1_TrackWidth, &fTrackWidth);
		ReadInt32(tkhdPosV1_TrackHeight, &fTrackHeight);
	}
	else
	{
		DEEP_DEBUG_PRINT(("QTAtom_tkhd::Initialize failed. Version unsupported: %d", fVersion));
		return false;
	}


	//
	// This atom has been successfully read in.
	return true;
}



// -------------------------------------
// Debugging functions
//
void QTAtom_tkhd::DumpAtom()
{
	// Temporary vars
	time_t      unixCreationTime = (time_t)fCreationTime + (time_t)QT_TIME_TO_LOCAL_TIME;
	char buffer[kTimeStrSize];
	struct tm  timeResult;

	DEBUG_PRINT(("QTAtom_tkhd::DumpAtom - Dumping atom.\n"));
	DEBUG_PRINT(("QTAtom_tkhd::DumpAtom - ..Version: %d.\n", (int)fVersion));
	DEBUG_PRINT(("QTAtom_tkhd::DumpAtom - ..Track ID: %" _S32BITARG_ "\n", fTrackID));
	DEBUG_PRINT(("QTAtom_tkhd::DumpAtom - ..Flags:%s%s%s%s\n", (fFlags & flagEnabled) ? " Enabled" : "", (fFlags & flagInMovie) ? " InMovie" : "", (fFlags & flagInPreview) ? " InPreview" : "", (fFlags & flagInPoster) ? " InPoster" : ""));
	DEBUG_PRINT(("QTAtom_tkhd::DumpAtom - ..Creation date: %s", qtss_asctime(qtss_gmtime(&unixCreationTime, &timeResult), buffer, sizeof(buffer))));
}
