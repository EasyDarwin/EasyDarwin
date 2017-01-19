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
 // QTAtom_dref:
 //   The 'dref' QTAtom class.


 // -------------------------------------
 // Includes
 //
#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include <string.h>

#ifndef __Win32__
#include <sys/types.h>
#include <netinet/in.h>
#endif

#include "QTFile.h"

#include "QTAtom.h"
#include "QTAtom_dref.h"


// -------------------------------------
// Macros
//
#define DEBUG_PRINT(s) if(fDebug) qtss_printf s
#define DEEP_DEBUG_PRINT(s) if(fDeepDebug) qtss_printf s



// -------------------------------------
// Constants
//
const int       drefPos_VersionFlags = 0;
const int       drefPos_NumRefs = 4;
const int       drefPos_RefTable = 8;

const int       drefRefPos_Size = 0;
const int       drefRefPos_Type = 4;
const int       drefRefPos_VersionFlags = 8;
const int       drefRefPos_Data = 12;



// -------------------------------------
// Mac alias constants and typedefs
//

enum {
	kEndMark = -1,            /* -1 end of variable info */
	kAbsPath = 2,            /* 2  absolute path name */
	kMaxMark = 10            /* End Marker */
};

typedef struct
{
	short           what;           /* what kind of information */
	short           len;            /* length of variable data */
	char            data[1];        /* actual data */
} varInfo;

typedef struct
{
	char            private1[130];
	short           nlvlFrom;       /* # of levels from fromFile/toFile until a common */
	short           nlvlTo;         /* ancestor directory is found */
	char            private2[16];
	varInfo         vdata[1];       /* variable length info */
} AliasRecordPriv;


// -------------------------------------
// Constructors and destructors
//
QTAtom_dref::QTAtom_dref(QTFile * File, QTFile::AtomTOCEntry * TOCEntry, bool Debug, bool DeepDebug)
	: QTAtom(File, TOCEntry, Debug, DeepDebug),
	fNumRefs(0), fRefs(nullptr)
{
}

QTAtom_dref::~QTAtom_dref()
{
	//+ 6.16.1999 rt fix memory leak
	for (UInt32 curRef = 0; curRef < fNumRefs; curRef++)
	{
		delete[] fRefs[curRef].Data;
		delete fRefs[curRef].FCB;
	}
	//-end

	//
	// Free our variables.
	delete[] fRefs;
}



// -------------------------------------
// Initialization functions
//
bool QTAtom_dref::Initialize()
{
	// Temporary vars
	UInt32      tempInt32;

	// General vars
	UInt64      refPos;


	//
	// Parse this atom's fields.
	ReadInt32(drefPos_VersionFlags, &tempInt32);
	fVersion = (UInt8)((tempInt32 >> 24) & 0x000000ff);
	fFlags = tempInt32 & 0x00ffffff;

	ReadInt32(drefPos_NumRefs, &fNumRefs);

	//
	// Read in all of the refs.
	if (fNumRefs > 0) {
		//
		// Allocate our ref table.
		fRefs = new DataRefEntry[fNumRefs];
		if (fRefs == nullptr)
			return false;

		//
		// Read them all in..
		refPos = drefPos_RefTable;
		for (UInt32 CurRef = 0; CurRef < fNumRefs; CurRef++) {
			//
			// Set up the entry.
			fRefs[CurRef].Flags = 0x0;
			fRefs[CurRef].ReferenceType = FOUR_CHARS_TO_INT('N', 'U', 'L', 'L'); // nullptr
			fRefs[CurRef].DataLength = 0;
			fRefs[CurRef].Data = nullptr;

			fRefs[CurRef].IsEntryInitialized = false;
			fRefs[CurRef].IsFileOpen = false;
			fRefs[CurRef].FCB = nullptr;


			//
			// Get the flags and type.
			ReadInt32(refPos + drefRefPos_VersionFlags, &tempInt32);
			fRefs[CurRef].Flags = tempInt32 & 0x00ffffff;

			ReadInt32(refPos + drefRefPos_Type, &tempInt32);
			fRefs[CurRef].ReferenceType = tempInt32;

			//
			// We're done if this is a self-referencing atom.
			if (fRefs[CurRef].Flags & flagSelfRef) {
				fRefs[CurRef].IsEntryInitialized = true;
				continue;
			}


			//
			// Get all of the data.
			ReadInt32(refPos + drefRefPos_Size, &tempInt32);
			fRefs[CurRef].DataLength = tempInt32 - 12 /* skip the header */;

			fRefs[CurRef].Data = new char[fRefs[CurRef].DataLength];
			if (fRefs[CurRef].Data == nullptr) {
				//
				// De-initialize this entry.
				fRefs[CurRef].DataLength = 0;
				fRefs[CurRef].IsEntryInitialized = false;
			}
			else {
				//
				// Read the entry data.
				ReadBytes(refPos + drefRefPos_Data, fRefs[CurRef].Data, fRefs[CurRef].DataLength);

				//
				// Configure the rest of the entry.
				fRefs[CurRef].FCB = new QTFile_FileControlBlock();
				if (fRefs[CurRef].FCB == nullptr)
					fRefs[CurRef].IsEntryInitialized = false;
				else
					fRefs[CurRef].IsEntryInitialized = true;
				fRefs[CurRef].IsFileOpen = false;
			}

			//
			// Skip over this mini-atom.
			refPos += fRefs[CurRef].DataLength + 12 /* account for the header */;
		}
	}

	//
	// This atom has been successfully read in.
	return true;
}



// -------------------------------------
// Read functions.
//
bool QTAtom_dref::Read(UInt32 RefID, UInt64 Offset, char * const Buffer, UInt32 Length, QTFile_FileControlBlock * FCB)
{
	// General vars
	DataRefEntry        *Entry;


	//
	// Validate that this ref exists.
	if ((RefID == 0) || (RefID > fNumRefs))
		return false;

	//  qtss_printf("QTAtom_dref::Read Offset = %qd, Length = %" _S32BITARG_ " \n", Offset, Length);
		//
		// If this data reference is in the movie file itself, then we can forward
		// the request directly to QTFile.
	if (IsRefInThisFile(RefID))
		return fFile->Read(Offset, Buffer, Length, FCB);


	//
	// The ref is not in this file; see if we have already opened an FCB for
	// the file and use that, otherwise we need to process the ref.
	Entry = &fRefs[RefID - 1];

	//
	// Abort if the entry was not initialized.
	if (!Entry->IsEntryInitialized)
		return false;

	//
	// Open the file (after parsing the data reference) if it is not open
	// already.
	if (!Entry->IsFileOpen) {
		// General vars
		char        *AliasPath;


		//
		// We only support parsing alias types.
		if (Entry->ReferenceType != FOUR_CHARS_TO_INT('a', 'l', 'i', 's')) //alis
			return false;

		//
		// Parse the alias.
		if ((AliasPath = ResolveAlias(Entry->Data, Entry->DataLength)) == nullptr)
			return false;

		//
		// Create a path which does *not* contain the current movie's name.
		char * p;
		char * MoviePath = fFile->GetMoviePath();
		char * NewPath = new char[strlen(MoviePath) + strlen(AliasPath) + 1];
		//char * NewPath = (char *)calloc(1, strlen(MoviePath) + strlen(AliasPath));
		if ((p = strrchr(MoviePath, QT_PATH_SEPARATOR)) == nullptr) {
			memcpy(NewPath, AliasPath, strlen(AliasPath) + 1);
		}
		else {
			int MoviePathClippedLength = (p - MoviePath) + 1;
			memcpy(NewPath, MoviePath, MoviePathClippedLength);
			memcpy(NewPath + MoviePathClippedLength, AliasPath, strlen(AliasPath) + 1);
		}


		//
		// Do the open.
		Entry->FCB->Set(NewPath);
		if (!Entry->FCB->IsValid()) {
			delete[] NewPath;
			//free(NewPath);
			return false;
		}

		Entry->IsFileOpen = true;
		delete[] NewPath;
		//free(NewPath);
	}

	//
	// Do the read.
	return fFile->Read(Offset, Buffer, Length, Entry->FCB);
}



// -------------------------------------
// Debugging functions
//
void QTAtom_dref::DumpAtom()
{
	DEBUG_PRINT(("QTAtom_dref::DumpAtom - Dumping atom.\n"));
	for (UInt32 CurRef = 1; CurRef <= fNumRefs; CurRef++)
		DEBUG_PRINT(("QTAtom_dref::DumpAtom - ..Ref #%"   _U32BITARG_   " is in file: %s\n", CurRef, IsRefInThisFile(CurRef) ? "yes" : "no"));
}



// -------------------------------------
// Protected member functions.
//
char * QTAtom_dref::ResolveAlias(char * const AliasData, UInt32 /* AliasDataLength */)
{
	// General vars
	AliasRecordPriv     *Alias = (AliasRecordPriv *)AliasData;
	varInfo             *AliasVarInfo;

	char                *path, *pathStart;
	int                 pathLength;


	//
	// Verify that we have a relative path.
	if ((Alias->nlvlTo == -1) || (Alias->nlvlFrom == -1))
		return nullptr;

	//
	// Search for the absolute pathname in this alias.
	AliasVarInfo = Alias->vdata;
	for (int loopCount = kMaxMark - 1; loopCount >= 0; loopCount--) {
		//
		// Break out of the loop if this is a match/the end of the alias.
		if (((short)ntohs(AliasVarInfo->what) == kAbsPath) || ((short)ntohs(AliasVarInfo->what) == kEndMark))
			break;

		//
		// Otherwise we need to move to the next data unit.
		AliasVarInfo = (varInfo *)((char *)AliasVarInfo + ((ntohs(AliasVarInfo->len) + 1) & ~1) + 4 /* header size */);
	}


	//
	// Now that we have the path, we need to strip off the absolute portions
	// of it so that we can get at it from our current (relative) root.
	AliasVarInfo->data[ntohs(AliasVarInfo->len)] = '\0';

	pathStart = path = AliasVarInfo->data;
	path += ntohs(AliasVarInfo->len);
	int i = ntohs(Alias->nlvlTo);
	pathLength = -1;
	while (i && (path > pathStart)) {
		if (*path-- == ':')
			i--;
		pathLength++;
	}

	if (i == 1)
		pathLength += 2;    // fell out of loop; we're relative to root
	else
		path += 2;          // points past separator character


	//
	// Return the alias.
	return path;
}
