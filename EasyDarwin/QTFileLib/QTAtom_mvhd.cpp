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
 // QTAtom_mvhd:
 //   The 'mvhd' QTAtom class.


 // -------------------------------------
 // Includes
 //
#include <stdio.h>
#include <time.h>

#include "QTFile.h"

#include "QTAtom.h"
#include "QTAtom_mvhd.h"



// -------------------------------------
// Constants
//
const int       mvhdPos_VersionFlags = 0;
const int       mvhdPos_CreationTime = 4;
const int       mvhdPos_ModificationTime = 8;
const int       mvhdPos_TimeScale = 12;
const int       mvhdPos_Duration = 16;
const int       mvhdPos_PreferredRate = 20;
const int       mvhdPos_PreferredVolume = 24;
const int       mvhdPos_a = 36;
const int       mvhdPos_b = 40;
const int       mvhdPos_u = 44;
const int       mvhdPos_c = 48;
const int       mvhdPos_d = 52;
const int       mvhdPos_v = 56;
const int       mvhdPos_x = 60;
const int       mvhdPos_y = 64;
const int       mvhdPos_w = 68;
const int       mvhdPos_PreviewTime = 72;
const int       mvhdPos_PreviewDuration = 76;
const int       mvhdPos_PosterTime = 80;
const int       mvhdPos_SelectionTime = 84;
const int       mvhdPos_SelectionDuration = 88;
const int       mvhdPos_CurrentTime = 92;
const int       mvhdPos_NextTrackID = 96;



const int       mvhdPosV1_CreationTime = 4; //+4 bytes
const int       mvhdPosV1_ModificationTime = 12; //+4 bytes
const int       mvhdPosV1_TimeScale = 20;
const int       mvhdPosV1_Duration = 24; //+4 bytes

const int       mvhdPosV1_PreferredRate = 20 + 12;
const int       mvhdPosV1_PreferredVolume = 24 + 12;
const int       mvhdPosV1_a = 36 + 12;
const int       mvhdPosV1_b = 40 + 12;
const int       mvhdPosV1_u = 44 + 12;
const int       mvhdPosV1_c = 48 + 12;
const int       mvhdPosV1_d = 52 + 12;
const int       mvhdPosV1_v = 56 + 12;
const int       mvhdPosV1_x = 60 + 12;
const int       mvhdPosV1_y = 64 + 12;
const int       mvhdPosV1_w = 68 + 12;
const int       mvhdPosV1_PreviewTime = 72 + 12;
const int       mvhdPosV1_PreviewDuration = 76 + 12;
const int       mvhdPosV1_PosterTime = 80 + 12;
const int       mvhdPosV1_SelectionTime = 84 + 12;
const int       mvhdPosV1_SelectionDuration = 88 + 12;
const int       mvhdPosV1_CurrentTime = 92 + 12;
const int       mvhdPosV1_NextTrackID = 96 + 12;

// -------------------------------------
// Macros
//
#define DEBUG_PRINT(s) if(fDebug) qtss_printf s
#define DEEP_DEBUG_PRINT(s) if(fDeepDebug) qtss_printf s



// -------------------------------------
// Constructors and destructors
//
QTAtom_mvhd::QTAtom_mvhd(QTFile * File, QTFile::AtomTOCEntry * TOCEntry, Bool16 Debug, Bool16 DeepDebug)
	: QTAtom(File, TOCEntry, Debug, DeepDebug)
{
}

QTAtom_mvhd::~QTAtom_mvhd()
{
}



// -------------------------------------
// Initialization functions
//
Bool16 QTAtom_mvhd::Initialize()
{
	// Temporary vars
	UInt32      tempInt32;

	//
	// Parse this atom's fields.
	ReadInt32(mvhdPos_VersionFlags, &tempInt32);
	fVersion = (UInt8)((tempInt32 >> 24) & 0x000000ff);
	fFlags = tempInt32 & 0x00ffffff;

	if (0 == fVersion)
	{
		// Verify that this atom is the correct length.
		if (fTOCEntry.AtomDataLength != 100)
		{
			DEEP_DEBUG_PRINT(("QTAtom_mvhd::Initialize failed. Expected AtomDataLength == 100 version: %d AtomDataLength: %" _64BITARG_ "u\n", fVersion, fTOCEntry.AtomDataLength));
			return false;
		}

		ReadInt32To64(mvhdPos_CreationTime, &fCreationTime);
		ReadInt32To64(mvhdPos_ModificationTime, &fModificationTime);
		ReadInt32(mvhdPos_TimeScale, &fTimeScale);
		ReadInt32To64(mvhdPos_Duration, &fDuration);

		ReadInt32(mvhdPos_PreferredRate, &fPreferredRate);
		ReadInt16(mvhdPos_PreferredVolume, &fPreferredVolume);

		ReadInt32(mvhdPos_a, &fa);
		ReadInt32(mvhdPos_b, &fb);
		ReadInt32(mvhdPos_u, &fu);
		ReadInt32(mvhdPos_c, &fc);
		ReadInt32(mvhdPos_d, &fd);
		ReadInt32(mvhdPos_v, &fv);
		ReadInt32(mvhdPos_x, &fx);
		ReadInt32(mvhdPos_y, &fy);
		ReadInt32(mvhdPos_w, &fw);

		ReadInt32(mvhdPos_PreviewTime, &fPreviewTime);
		ReadInt32(mvhdPos_PreviewDuration, &fPreviewDuration);
		ReadInt32(mvhdPos_PosterTime, &fPosterTime);
		ReadInt32(mvhdPos_SelectionTime, &fSelectionTime);
		ReadInt32(mvhdPos_SelectionDuration, &fSelectionDuration);
		ReadInt32(mvhdPos_CurrentTime, &fCurrentTime);
		ReadInt32(mvhdPos_NextTrackID, &fNextTrackID);
	}
	else if (1 == fVersion)
	{
		// Verify that this atom is the correct length.
		if (fTOCEntry.AtomDataLength != 112)
		{
			DEEP_DEBUG_PRINT(("QTAtom_mvhd::Initialize failed. Expected AtomDataLength = 112 version: %d AtomDataLength: %" _64BITARG_ "u\n", fVersion, fTOCEntry.AtomDataLength));
			return false;
		}

		ReadInt64(mvhdPosV1_CreationTime, &fCreationTime);
		ReadInt64(mvhdPosV1_ModificationTime, &fModificationTime);
		ReadInt32(mvhdPosV1_TimeScale, &fTimeScale);
		ReadInt64(mvhdPosV1_Duration, &fDuration);

		ReadInt32(mvhdPosV1_PreferredRate, &fPreferredRate);
		ReadInt16(mvhdPosV1_PreferredVolume, &fPreferredVolume);

		ReadInt32(mvhdPosV1_a, &fa);
		ReadInt32(mvhdPosV1_b, &fb);
		ReadInt32(mvhdPosV1_u, &fu);
		ReadInt32(mvhdPosV1_c, &fc);
		ReadInt32(mvhdPosV1_d, &fd);
		ReadInt32(mvhdPosV1_v, &fv);
		ReadInt32(mvhdPosV1_x, &fx);
		ReadInt32(mvhdPosV1_y, &fy);
		ReadInt32(mvhdPosV1_w, &fw);

		ReadInt32(mvhdPosV1_PreviewTime, &fPreviewTime);
		ReadInt32(mvhdPosV1_PreviewDuration, &fPreviewDuration);
		ReadInt32(mvhdPosV1_PosterTime, &fPosterTime);
		ReadInt32(mvhdPosV1_SelectionTime, &fSelectionTime);
		ReadInt32(mvhdPosV1_SelectionDuration, &fSelectionDuration);
		ReadInt32(mvhdPosV1_CurrentTime, &fCurrentTime);
		ReadInt32(mvhdPosV1_NextTrackID, &fNextTrackID);
	}
	else
	{
		DEEP_DEBUG_PRINT(("QTAtom_mvhd::Initialize failed. Version unsupported: %d\n", fVersion));
		return false;
	}


	//
	// This atom has been successfully read in.
	return true;
}



// -------------------------------------
// Debugging functions
//
void QTAtom_mvhd::DumpAtom()
{
	// Temporary vars
	time_t      unixCreationTime = (time_t)fCreationTime + (time_t)QT_TIME_TO_LOCAL_TIME;
	char buffer[kTimeStrSize];
	struct tm  timeResult;
	DEBUG_PRINT(("QTAtom_mvhd::DumpAtom - Dumping atom.\n"));
	DEBUG_PRINT(("QTAtom_mvhd::DumpAtom - ..Version: %d.\n", (int)fVersion));
	DEBUG_PRINT(("QTAtom_mvhd::DumpAtom - ..Creation date: %s", qtss_asctime(qtss_gmtime(&unixCreationTime, &timeResult), buffer, sizeof(buffer))));
	DEBUG_PRINT(("QTAtom_mvhd::DumpAtom - ..Movie duration: %.2f seconds\n", GetDurationInSeconds()));
}
