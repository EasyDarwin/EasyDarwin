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
// QTFile:
//   The central point of control for a file in the QuickTime File Format.


// -------------------------------------
// Includes
//

#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include <string.h>

#ifndef __Win32__
#include <netinet/in.h>
#endif

#include "OSMutex.h"

#include "QTFile.h"

#include "QTAtom.h"
#include "QTAtom_mvhd.h"
#include "QTAtom_tkhd.h"

#include "QTTrack.h"
#include "QTHintTrack.h"
#include "OSMemory.h"
#if MMAP_TABLES
#include <sys/mman.h>
#endif

#if DSS_USE_API_CALLBACKS
#include "QTSS.h" // When inside the server, we need to use the QTSS API file system callbacks
#endif


// -------------------------------------
// Macros
//
#define DEBUG_PRINT(s) if(fDebug) qtss_printf s
#define DEEP_DEBUG_PRINT(s) if(fDeepDebug) qtss_printf s



// -------------------------------------
// Constructors and destructors
//
QTFile::QTFile(Bool16 Debug, Bool16 DeepDebug)
    : fDebug(Debug), fDeepDebug(DeepDebug),
      fNextTOCID(1),
#if DSS_USE_API_CALLBACKS
    fMovieFD(NULL),
    fOSFileSourceFD(NULL),
#endif
    fCacheBuffersSet(false),
    fTOC(NULL), fTOCOrdHead(NULL), fTOCOrdTail(NULL),
    fNumTracks(0),
    fFirstTrack(NULL), fLastTrack(NULL),
    fMovieHeaderAtom(NULL), 
    fFile(-1)
{
}

QTFile::~QTFile(void)
{
    //
    // Free our track list (and the associated tracks)
    TrackListEntry *TrackEntry = fFirstTrack,
                   *NextTrackEntry = TrackEntry ? TrackEntry->NextTrack : NULL;
    while( TrackEntry != NULL ) {
        //
        // Delete this track entry and move to the next one.
        if( TrackEntry->Track != NULL )
            delete TrackEntry->Track;
        delete TrackEntry;
        
        TrackEntry = NextTrackEntry;
        if( TrackEntry != NULL )
            NextTrackEntry = TrackEntry->NextTrack;
    }
    
    //
    // Free our variables.
    if( fMovieHeaderAtom != NULL )
        delete fMovieHeaderAtom;
    
    //
    // Free our table of contents
    AtomTOCEntry *TOCEntry = fTOCOrdHead,
                 *NextTOCEntry = TOCEntry ? TOCEntry->NextOrdAtom : NULL;
    while( TOCEntry != NULL ) {
        //
        // Delete this track entry and move to the next one.
        delete TOCEntry;
        
        TOCEntry = NextTOCEntry;
        if( TOCEntry != NULL )
            NextTOCEntry = TOCEntry->NextOrdAtom;
    }
    
    //
    // Delete our mutexen.
    if( fReadMutex != NULL )
        delete fReadMutex;
    
    //
    // Free our path.
    if( fMoviePath != NULL )
        //free(fMoviePath);
        delete [] fMoviePath;

#if DSS_USE_API_CALLBACKS
    (void)QTSS_CloseFileObject(fMovieFD);
#endif
#if MMAP_TABLES
    ::close(fFile);
#endif
}



// -------------------------------------
// Public functions
//

//
// Open a movie file and generate the atom table of contents.
QTFile::ErrorCode QTFile::Open(const char * MoviePath)
{
    // General vars
    AtomTOCEntry    *TOCEntry;


    //
    // Create our mutexen.
    fReadMutex = NEW OSMutex();
    if( fReadMutex == NULL )
        return errInternalError;


    //
    // Attempt to open the movie file.
    DEBUG_PRINT(("QTFile::Open - Opening movie.\n"));
    
    fMoviePath = NEW char[strlen(MoviePath) + 1];
    ::strcpy(fMoviePath, MoviePath);

#if MMAP_TABLES
    fFile = open(fMoviePath, O_RDONLY);
#endif


#if DSS_USE_API_CALLBACKS
    QTSS_Error theErr = QTSS_OpenFileObject(fMoviePath, qtssOpenFileReadAhead, &fMovieFD);
    if (theErr != QTSS_NoErr)
        return errFileNotFound;
    
    QTSS_AttrInfoObject attrInfoObject;
    QTSS_Error error = QTSS_GetAttrInfoByName(fMovieFD, "QTSSPosixFileSysModuleOSFileSource", &attrInfoObject);
    if (QTSS_NoErr == error)
    {
        QTSS_AttributeID fdID;
        UInt32 len = sizeof(fdID);
        error = QTSS_GetValue(attrInfoObject, qtssAttrID, 0, &fdID, &len);

        if (theErr == QTSS_NoErr && len > 0)        
        {   len = sizeof(fOSFileSourceFD);
            error = QTSS_GetValue(fMovieFD, fdID, 0, &fOSFileSourceFD, &len);
            if (theErr != QTSS_NoErr || len == 0)      
                fOSFileSourceFD = NULL;
        }
    }
        
#else
    fMovieFD.Set(MoviePath);
    if( !fMovieFD.IsValid() )
        return errFileNotFound;
#endif
    
    //
    // We have a file, generate the mod date str
    fModDateBuffer.Update(this->GetModDate());
    
    //
    // Generate the table of contents for this movie.
    DEBUG_PRINT(("QTFile::Open - Generating Atom TOC.\n"));
    if( !GenerateAtomTOC() )
        return errInvalidQuickTimeFile;
    

    //
    // Find the Movie Header atom and read it in.
    DEBUG_PRINT(("QTFile::Open - Reading movie header.\n"));
    if( !FindTOCEntry("moov:mvhd", &TOCEntry) )
        return errInvalidQuickTimeFile;
    
    fMovieHeaderAtom = NEW QTAtom_mvhd(this, TOCEntry, fDebug, fDeepDebug);
    if( fMovieHeaderAtom == NULL )
        return errInternalError;
    if( !fMovieHeaderAtom->Initialize() )
        return errInvalidQuickTimeFile;
    
    if(fDeepDebug) fMovieHeaderAtom->DumpAtom();


    //
    // Create QTTrack objects for all of the tracks in this movie.  (Although
    // this does incur some extra resource usage where tracks are not used,
    // they can always A) be disposed of later, or B) be ignored as their use
    // of system resources is exceptionally minimal.)
    // NOTE that the tracks are *not* initialized here.  That is done when they
    // are actually used; either directly or by a QTHintTrack.
    DEBUG_PRINT(("QTFile::Open - Loading tracks.\n"));
    TOCEntry = NULL;
    while( FindTOCEntry("moov:trak", &TOCEntry, TOCEntry) ) {
        // General vars
        TrackListEntry  *ListEntry;


        //
        // Allocate space for this list entry.
        ListEntry = NEW TrackListEntry();
        if( ListEntry == NULL )
            return errInternalError;

        //
        // Make a hint track if that's what this is.
        if( FindTOCEntry(":tref:hint", NULL, TOCEntry) ) {
            ListEntry->Track = NEW QTHintTrack(this, TOCEntry, fDebug, fDeepDebug);
            ListEntry->IsHintTrack = true;
        } else {
            ListEntry->Track = NEW QTTrack(this, TOCEntry, fDebug, fDeepDebug);
            ListEntry->IsHintTrack = false;
        }
        if( ListEntry->Track == NULL ) {
            delete ListEntry;
            return errInternalError;
        }

        QTTrack* theTrack = NULL;
        if (FindTrack(ListEntry->Track->GetTrackID(), &theTrack))
        {
            //
            // A track with this track ID already exists. Ignore other tracks with
            // identical track IDs.
            delete ListEntry->Track;
            delete ListEntry;
            continue;
        }

        //
        // Add this track object to our track list.
        ListEntry->TrackID = ListEntry->Track->GetTrackID();

        ListEntry->NextTrack = NULL;

        if( fFirstTrack == NULL ) {
            fFirstTrack = fLastTrack = ListEntry;
        } else {
            fLastTrack->NextTrack = ListEntry;
            fLastTrack = ListEntry;
        }
        
        //
        // One more track..
        fNumTracks++;
    }
    
    
    //
    // The file has been successfully opened.
    DEBUG_PRINT(("QTFile::Open - Finished loading.\n"));
    return errNoError;
}

char *QTFile::GetModDateStr()
{
    return fModDateBuffer.GetDateBuffer();
}

void QTFile::AllocateBuffers(UInt32 inUnitSizeInK, UInt32 inBufferInc, UInt32 inBufferSizeUnits, UInt32 inMaxBitRateBuffSizeInBlocks, UInt32 inBitrate)
{

#if DSS_USE_API_CALLBACKS
    if (fOSFileSourceFD != NULL)
    {
        if (!fCacheBuffersSet)
        {
            fCacheBuffersSet = true;    
            fOSFileSourceFD->AllocateFileCache(inUnitSizeInK, inBufferSizeUnits, inBufferInc,inMaxBitRateBuffSizeInBlocks, inBitrate);
            fOSFileSourceFD->EnableFileCache(fCacheBuffersSet);
        }
        else
        {    
            fOSFileSourceFD->IncMaxBuffers();
        }
    }

#else
    if (!fCacheBuffersSet)
    {
        fCacheBuffersSet = true;    
        fMovieFD.AllocateFileCache(inUnitSizeInK, inBufferSizeUnits, inBufferInc,inMaxBitRateBuffSizeInBlocks, inBitrate);
        fMovieFD.EnableFileCache(fCacheBuffersSet);
    }
    else
    {    
        fMovieFD.IncMaxBuffers();
    }
    
#endif

}


//
// Table of Contents functions.
Bool16 QTFile::FindTOCEntry(const char * AtomPath, AtomTOCEntry **TOCEntry, AtomTOCEntry *LastFoundTOCEntry)
{
    // General vars
    AtomTOCEntry    *Atom, *CurParent;
    const char      *pCurAtomType = AtomPath;

    UInt32          RootTOCID = 0;


    DEEP_DEBUG_PRINT(("QTFile::FindTOCEntry - Searching for \"%s\".\n", AtomPath));

    //
    // If we were given a LastFoundTOCEntry to start from, then we need to
    // find that before we can do the real search.
    if( LastFoundTOCEntry != NULL ) {
        for( Atom = fTOCOrdHead; ; Atom = Atom->NextOrdAtom ) {
            //
            // If it's NULL, then something is seriously wrong.
            if( Atom == NULL )
                return false;

            //
            // Check for matches.
            if( Atom->TOCID == LastFoundTOCEntry->TOCID )
                break;
        }

        //
        // Is this a root search or a rooted search?
        if( *pCurAtomType == ':' ) {
            DEEP_DEBUG_PRINT(("QTFile::FindTOCEntry - ..Rooting search at [%03"_U32BITARG_"].\n", LastFoundTOCEntry->TOCID));

            RootTOCID = LastFoundTOCEntry->TOCID;
            pCurAtomType++;
            Atom = Atom->FirstChild;
        } else {
            //
            // "Wind up" the list to get our new search path.
            for( CurParent = Atom->Parent; CurParent != NULL; CurParent = CurParent->Parent )
                pCurAtomType += (4 + 1);

            //
            // Move to the next atom.
            Atom = Atom->NextAtom;

            if( Atom != NULL ) {
                DEEP_DEBUG_PRINT(("QTFile::FindTOCEntry - ..Starting search at [%03"_U32BITARG_"] '%c%c%c%c'.  Search path is \"%s\"\n",
                    Atom->TOCID,
                    (char)((Atom->AtomType & 0xff000000) >> 24),
                    (char)((Atom->AtomType & 0x00ff0000) >> 16),
                    (char)((Atom->AtomType & 0x0000ff00) >> 8),
                    (char)((Atom->AtomType & 0x000000ff)),
                    pCurAtomType));
            }
        }
    } else {
        //
        // Start at the head..
        Atom = fTOC;
    }

    //
    // Recurse through our table of contents until we find this path.
    while( Atom != NULL ) { // already initialized by the above
        DEEP_DEBUG_PRINT(("QTFile::FindTOCEntry - ..Comparing against [%03"_U32BITARG_" '%c%c%c%c'\n",
            Atom->TOCID,
            (char)((Atom->AtomType & 0xff000000) >> 24),
            (char)((Atom->AtomType & 0x00ff0000) >> 16),
            (char)((Atom->AtomType & 0x0000ff00) >> 8),
            (char)((Atom->AtomType & 0x000000ff))));

        //
        // Is this a match?
        if( memcmp(&Atom->beAtomType, pCurAtomType, 4) == 0 ) {
            //
            // Skip to the delimiter.
            pCurAtomType += 4;

            DEEP_DEBUG_PRINT(("QTFile::FindTOCEntry - ....Found match for '%c%c%c%c'; search path is \"%s\"\n",
                (char)((Atom->AtomType & 0xff000000) >> 24),
                (char)((Atom->AtomType & 0x00ff0000) >> 16),
                (char)((Atom->AtomType & 0x0000ff00) >> 8),
                (char)((Atom->AtomType & 0x000000ff)),
                pCurAtomType));

            //
            // Did we finish matching?
            if( *pCurAtomType == '\0' ) {
                //
                // Here's the atom.
                DEEP_DEBUG_PRINT(("QTFile::FindTOCEntry - ..Matched atom path.\n"));
                if( TOCEntry != NULL )
                    *TOCEntry = Atom;

                return true;
            }

            //
            // Not done yet; descend.
            pCurAtomType++;
            Atom = Atom->FirstChild;
            continue;
        }

        //
        // If there is no next atom, but we have a parent, then move up and
        // continue the search. (this is necessary if A) the file is wacky,
        // or B) we were given a LastFoundTOCEntry)  Do not, however, leave
        // the realm of the RootTOCID (if we have one).
        while( (Atom->NextAtom == NULL) && (Atom->Parent != NULL) ) {
            //
            // Do not leave the realm of the RootTOCID (if we have one).
            if( RootTOCID && (RootTOCID == Atom->Parent->TOCID) ) {
                DEEP_DEBUG_PRINT(("QTFile::FindTOCEntry - ....Hit RootTOCID; aborting ascension.\n"));
                break;
            }

            //
            // Move up.
            pCurAtomType -= (4 + 1);
            Atom = Atom->Parent;

            DEEP_DEBUG_PRINT(("QTFile::FindTOCEntry - ....Failed match; ascending to parent.  Search path is \"%s\".\n", pCurAtomType));
        }

        //
        // No match; keep going.
        Atom = Atom->NextAtom;
    }

    //
    // Couldn't find a match..
    DEEP_DEBUG_PRINT(("QTFile::FindTOCEntry - ..Match failed.\n"));
    return false;
}


//
// Track List functions.
Bool16 QTFile::NextTrack(QTTrack **Track, QTTrack *LastFoundTrack)
{
    // General vars
    TrackListEntry      *ListEntry;


    //
    // Return the first track if requested.
    if( LastFoundTrack == NULL ) {
        if( fFirstTrack != NULL ) {
            *Track = fFirstTrack->Track;
            return true;
        } else {
            return false;
        }
    }

    //
    // Find LastTrack and return the one after it.
    for( ListEntry = fFirstTrack; ListEntry != NULL; ListEntry = ListEntry->NextTrack ) {
        //
        // Check for matches.
        if( ListEntry->TrackID == LastFoundTrack->GetTrackID() ) {
            //
            // Is there a next track?
            if( ListEntry->NextTrack != NULL ) {
                *Track = ListEntry->NextTrack->Track;
                return true;
            } else {
                return false;
            }
        }
    }

    //
    // This should never happen, but..
    return false;
}

Bool16 QTFile::FindTrack(UInt32 TrackID, QTTrack **Track)
{
    // General vars
    TrackListEntry      *ListEntry;


    //
    // Find the specified track.
    for( ListEntry = fFirstTrack; ListEntry != NULL; ListEntry = ListEntry->NextTrack ) {
        //
        // Check for matches.
        if( ListEntry->TrackID == TrackID ) {
            *Track = ListEntry->Track;
            return true;
        }
    }

    //
    // The search failed.
    return false;
}

Bool16 QTFile::IsHintTrack(QTTrack *Track)
{
    // General vars
    TrackListEntry      *ListEntry;


    //
    // Find the specified track.
    for( ListEntry = fFirstTrack; ListEntry != NULL; ListEntry = ListEntry->NextTrack ) {
        //
        // Check for matches.
        if( ListEntry->Track == Track )
            return ListEntry->IsHintTrack;
    }

    //
    // The search failed.  Can this actually happen?
    return false;
}



//
// Accessors
Float64 QTFile::GetTimeScale(void)
{
   if (fMovieHeaderAtom == NULL)
        return 0.0;

    return fMovieHeaderAtom->GetTimeScale();
}

Float64 QTFile::GetDurationInSeconds(void)
{
   if (fMovieHeaderAtom == NULL)
        return 0.0;

    return fMovieHeaderAtom->GetDurationInSeconds();
}

SInt64 QTFile::GetModDate()
{
#if DSS_USE_API_CALLBACKS
    SInt64 theTime = 0;
    UInt32 theLen = sizeof(SInt64);
    (void)QTSS_GetValue(fMovieFD, qtssFlObjModDate, 0, (void*)&theTime, &theLen);
    return theTime;
#else
    time_t theTime = fMovieFD.GetModDate();
    return (SInt64)theTime * 1000;
#endif
}

//
// Read functions.
Bool16 QTFile::Read(UInt64 Offset, char * const Buffer, UInt32 Length, QTFile_FileControlBlock * FCB)
{
    // General vars
    OSMutexLocker   ReadMutex(fReadMutex);
    Bool16 rv = false;
    UInt32 gotlen = 0;

    if( FCB )
        rv = FCB->Read(&fMovieFD,Offset,Buffer,Length);
    else
    {
#if DSS_USE_API_CALLBACKS
        QTSS_Error theErr = QTSS_Seek(fMovieFD, Offset);
        if (theErr == QTSS_NoErr)
            theErr = QTSS_Read(fMovieFD, Buffer, Length, &gotlen);
        if ((theErr == QTSS_NoErr) && (gotlen == Length))
            rv = true;
#else
        if ((fMovieFD.Read(Offset,Buffer,Length,&gotlen) == OS_NoErr) &&
            (gotlen == Length))
            rv = true;
#endif
    }
    return rv;
}




// -------------------------------------
// Protected functions
//
Bool16 QTFile::GenerateAtomTOC(void)
{
    // General vars
    OSType          AtomType;
    UInt32          atomLength;
    UInt64          BigAtomLength;

    UInt64          CurPos;
    UInt32		    CurAtomHeaderSize;

    AtomTOCEntry    *NewTOCEntry = NULL,
                    *CurParent = NULL, *LastTOCEntry = NULL;
    Bool16 hasMoovAtom = false;
    Bool16 hasBigAtom = false;


    //
    // Scan through all of the atoms in this movie, generating a TOC entry
    // for each one.
    CurPos = 0;
    while( Read(CurPos, (char *)&atomLength, 4) ) {

        //
        // Swap the AtomLength for little-endian machines.
        CurPos += 4;
        atomLength = ntohl(atomLength);
        BigAtomLength = (UInt64) atomLength;
        hasBigAtom = false;

        //
        // Is AtomLength zero?  If so, and we're in a 'udta' atom, then all
        // is well (this is the end of a 'udta' atom).  Leave this level of
        // siblings as the 'udta' atom is obviously over.
        if( (BigAtomLength == 0) && CurParent && (CurParent->AtomType == FOUR_CHARS_TO_INT('u', 'd', 't', 'a')) ) {
            //
            // Do no harm..
            DEEP_DEBUG_PRINT(("QTFile::GenerateAtomTOC - Found End-of-udta marker.\n"));
            LastTOCEntry = CurParent;
            CurParent = CurParent->Parent;

            //
            // Keep moving up.
            goto lbl_SkipAtom;

        //
        // Is the AtomLength zero?  If so, this is a "QT atom" which needs
        // some additional work before it can be processed.
        } 
        else if( BigAtomLength == 0 ) 
        { 
            //
            // This is a QT atom; skip the (rest of the) reserved field and
            // the lock count field.
            CurPos += 22;

            //
            // Read the size and the type of this atom.
            if( !Read(CurPos, (char *)&atomLength, 4) )
                return false;
            CurPos += 4;
            BigAtomLength =  (UInt64) ntohl(atomLength);
            
            if( !Read(CurPos, (char *)&AtomType, 4) )
                return false;
            CurPos += 4;
            AtomType = ntohl(AtomType);

            //
            // Skip over the rest of the fields.
            CurPos += 12;
            
            //
            // Set the header size to that of a QT atom.
            CurAtomHeaderSize = 10 + 16 + 4 + 4 + 4 + 2 + 2 + 4;

        //
        // This is a normal atom; get the atom type.
        } 
        else  // This is a normal atom; get the atom type.
        {
            if( !Read(CurPos, (char *)&AtomType, 4) )
                break;

            CurPos += 4;
            CurAtomHeaderSize = 4 + 4; // AtomLength + AtomType
            AtomType = ntohl(AtomType);

            if ( atomLength == 1 ) //large size atom
            {
                if( !Read(CurPos, (char *)&BigAtomLength, 8) )  
                    break;
                BigAtomLength = QTAtom::NTOH64(BigAtomLength);
                CurPos += 8;
                CurAtomHeaderSize += 8; // AtomLength + AtomType + big atom length
                hasBigAtom = true;
            }
             
            if (AtomType == FOUR_CHARS_TO_INT('u', 'u', 'i', 'd'))
            {
                static const int sExtendedTypeSize = 16;
                UInt8 usertype[sExtendedTypeSize + 1]; //sExtendedTypeSize for the type + 1 for 0 terminator.
                usertype[sExtendedTypeSize] = 0;
                if( !Read(CurPos, (char *)usertype, 16) ) // read and just throw it away we don't need to store
                    return false;
    
                DEEP_DEBUG_PRINT(("QTFile::GenerateAtomTOC - Found 'uuid' extended type name= %s.\n",usertype));
                CurPos += sExtendedTypeSize;
                CurAtomHeaderSize += sExtendedTypeSize;
            }

        }


        if (0 == BigAtomLength)
        {
            DEEP_DEBUG_PRINT(("QTFile::GenerateAtomTOC - Bail atom is bad. type= '%c%c%c%c'; pos=%"_64BITARG_"u; length=%"_64BITARG_"u; header=%"_U32BITARG_".\n",
            (char)((AtomType & 0xff000000) >> 24),
            (char)((AtomType & 0x00ff0000) >> 16),
            (char)((AtomType & 0x0000ff00) >> 8),
            (char)((AtomType & 0x000000ff)),
            CurPos - CurAtomHeaderSize, BigAtomLength, CurAtomHeaderSize));         
            
            return false;
        }

        if (hasBigAtom)
        {    DEEP_DEBUG_PRINT(("QTFile::GenerateAtomTOC - Found 64 bit atom '%c%c%c%c'; pos=%"_64BITARG_"u; length=%"_64BITARG_"u; header=%"_U32BITARG_".\n",
            (char)((AtomType & 0xff000000) >> 24),
            (char)((AtomType & 0x00ff0000) >> 16),
            (char)((AtomType & 0x0000ff00) >> 8),
            (char)((AtomType & 0x000000ff)),
            CurPos - CurAtomHeaderSize, BigAtomLength, CurAtomHeaderSize));
        }
        else
        {    DEEP_DEBUG_PRINT(("QTFile::GenerateAtomTOC - Found 32 bit atom '%c%c%c%c'; pos=%"_64BITARG_"u; length=%"_64BITARG_"u; header=%"_U32BITARG_".\n",
            (char)((AtomType & 0xff000000) >> 24),
            (char)((AtomType & 0x00ff0000) >> 16),
            (char)((AtomType & 0x0000ff00) >> 8),
            (char)((AtomType & 0x000000ff)),
            CurPos - CurAtomHeaderSize, BigAtomLength, CurAtomHeaderSize));
        }

        if ((AtomType == FOUR_CHARS_TO_INT('m', 'o', 'o', 'v')) && (hasMoovAtom))
        {
            //
            // Skip over any additional 'moov' atoms once we find one.
            CurPos += BigAtomLength - CurAtomHeaderSize;
            continue;
        }
        else if (AtomType == FOUR_CHARS_TO_INT('m', 'o', 'o', 'v'))
        {
           hasMoovAtom = true;
        }
        else if (!hasMoovAtom)
        {
            CurPos += BigAtomLength - CurAtomHeaderSize;
            continue;
        }

        //
        // Create a TOC entry for this atom.
        NewTOCEntry = NEW AtomTOCEntry();
        if( NewTOCEntry == NULL )
            return false;

        NewTOCEntry->TOCID = fNextTOCID++;

        NewTOCEntry->AtomType = AtomType;
        NewTOCEntry->beAtomType = htonl(AtomType);

        NewTOCEntry->AtomDataPos = CurPos;
        NewTOCEntry->AtomDataLength = BigAtomLength - CurAtomHeaderSize;
        NewTOCEntry->AtomHeaderSize = CurAtomHeaderSize;

        NewTOCEntry->NextOrdAtom = NULL;

        NewTOCEntry->PrevAtom = LastTOCEntry;
        NewTOCEntry->NextAtom = NULL;

        if( NewTOCEntry->PrevAtom )
            NewTOCEntry->PrevAtom->NextAtom = NewTOCEntry;

        NewTOCEntry->Parent = CurParent;
        NewTOCEntry->FirstChild = NULL;

        LastTOCEntry = NewTOCEntry;

        //
        // Make this entry the head of the TOC list if necessary.
        if( fTOC == NULL ) {
            DEEP_DEBUG_PRINT(("QTFile::GenerateAtomTOC - Placing this atom at the head of the TOC.\n"));
            fTOC = NewTOCEntry;
            fTOCOrdHead = fTOCOrdTail = NewTOCEntry;
        } else {
            fTOCOrdTail->NextOrdAtom = NewTOCEntry;
            fTOCOrdTail = NewTOCEntry;
        }

        //
        // Make this the first child if we have one.
        if( CurParent && (CurParent->FirstChild == NULL) ) {
            DEEP_DEBUG_PRINT(("QTFile::GenerateAtomTOC - ..This atom is the first child of our new parent.\n"));
            CurParent->FirstChild = NewTOCEntry;
        }


        //
        // Figure out if we have to descend into this entry and do so.
        switch( NewTOCEntry->AtomType ) 
        {
            case FOUR_CHARS_TO_INT('m', 'o', 'o', 'v'): //moov
            case FOUR_CHARS_TO_INT('c', 'l', 'i', 'p'): //clip
            case FOUR_CHARS_TO_INT('t', 'r', 'a', 'k'): //trak
            case FOUR_CHARS_TO_INT('m', 'a', 't', 't'): //matt
            case FOUR_CHARS_TO_INT('e', 'd', 't', 's'): //edts
            case FOUR_CHARS_TO_INT('t', 'r', 'e', 'f'): //tref
            case FOUR_CHARS_TO_INT('m', 'd', 'i', 'a'): //mdia
            case FOUR_CHARS_TO_INT('m', 'i', 'n', 'f'): //minf
            case FOUR_CHARS_TO_INT('d', 'i', 'n', 'f'): //dinf
            case FOUR_CHARS_TO_INT('s', 't', 'b', 'l'): //stbl
            case FOUR_CHARS_TO_INT('u', 'd', 't', 'a'): /* can appear anywhere */ //udta
            case FOUR_CHARS_TO_INT('h', 'n', 't', 'i'): //hnti
            case FOUR_CHARS_TO_INT('h', 'i', 'n', 'f'): //hinf
            {
                //
                // All of the above atoms need to be descended into.  Set up
                // our variables to descend into this atom.
    
                if (NewTOCEntry->AtomDataLength > 0) // maybe it should be greater than some number such as header size?
                {
                    DEEP_DEBUG_PRINT(("QTFile::GenerateAtomTOC - ..Creating a new parent.\n"));
                    CurParent = NewTOCEntry;
                    LastTOCEntry = NULL;
                    continue; // skip the level checks below
                }
                else
                {
                    DEEP_DEBUG_PRINT(("QTFile::GenerateAtomTOC - Empty atom.\n"));
                }
                
            }
            break;
        }


        //
        // Skip over this atom's data.
        CurPos += NewTOCEntry->AtomDataLength;

        //
        // Would continuing to the next atom cause us to leave this level?
        // If so, move up a level and move on.  Keep doing this until we find
        // a level we can continue on.
lbl_SkipAtom:
        while( CurParent && ((LastTOCEntry->AtomDataPos - LastTOCEntry->AtomHeaderSize) + (LastTOCEntry->AtomDataLength + LastTOCEntry->AtomHeaderSize)) >= ((CurParent->AtomDataPos - CurParent->AtomHeaderSize) + (CurParent->AtomDataLength + CurParent->AtomHeaderSize)) ) {
            DEEP_DEBUG_PRINT(("QTFile::GenerateAtomTOC - End of this parent's ('%c%c%c%c') children.\n",
                (char)((CurParent->AtomType & 0xff000000) >> 24),
                (char)((CurParent->AtomType & 0x00ff0000) >> 16),
                (char)((CurParent->AtomType & 0x0000ff00) >> 8),
                (char)((CurParent->AtomType & 0x000000ff))));
            LastTOCEntry = CurParent;
            CurParent = CurParent->Parent;
        }
    }


    if (!this->ValidTOC()) // make sure we were able to read all the atoms.
        return false;



    //
    // The TOC has been successfully read in.
    return true;
}


char *QTFile::MapFileToMem(UInt64 offset, UInt32 length)
{
#if MMAP_TABLES
    char*  mappedMem = (char *) mmap( NULL,
                            (size_t) length,
                            PROT_READ,
                            0,
                            fFile,
                            (off_t) offset);
                            
            if(mappedMem == MAP_FAILED )
            {  //printf("MAP_FAILED\n");
               mappedMem = NULL;
            }
    return mappedMem;
#else
    return NULL;
#endif

}

int QTFile::UnmapMem(char* memPtr, UInt32 length)
{
#if MMAP_TABLES
    return munmap( (caddr_t) memPtr, (size_t) length);
#else
    return 0;
#endif
}


// -------------------------------------
// Debugging functions.
void QTFile::DumpAtomTOC(void)
{
    // General vars
    AtomTOCEntry    *Atom;
    char            Indent[128] = { '\0' };


    // Display all of the atoms.
    DEBUG_PRINT(("QTFile::DumpAtomTOC - Dumping TOC.\n"));
    for( Atom = fTOC; Atom != NULL; ) 
    {
    
        // Print out this atom.     
        DEBUG_PRINT(("%s[%03"_U32BITARG_"] AtomType=%c%c%c%c; AtomDataPos=%"_64BITARG_"u; AtomDataLength=%"_64BITARG_"u\n",
        Indent,
        Atom->TOCID,
        (char)((Atom->AtomType & 0xff000000) >> 24),
        (char)((Atom->AtomType & 0x00ff0000) >> 16),
        (char)((Atom->AtomType & 0x0000ff00) >> 8),
        (char)((Atom->AtomType & 0x000000ff)),
        Atom->AtomDataPos, Atom->AtomDataLength));
        
        
        // Descend into this atom's children if it has any.
        if( Atom->FirstChild != NULL ) {
            Atom = Atom->FirstChild;
            strcat(Indent, "  ");
            continue;
        }
        

        // Are we at the end of a sibling list?  If so, move up a level.  Keep
        // moving up until we find a non-NULL atom.
        while( Atom && (Atom->NextAtom == NULL) ) 
        {
            Atom = Atom->Parent;
            Indent[strlen(Indent) - 2] = '\0';
        }
        

        // Next atom..
        if( Atom != NULL ) // could be NULL if we just moved up to a NULL parent
            Atom = Atom->NextAtom;
    }
}
