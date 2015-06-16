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
// QTHintTrack:
//   The central point of control for a track in a QTFile.

/*

    9.21.99 rtucker - CVS is down, so I might forget this by the time tis back up
    
    
    added:
    
    changed GetPacket to be more efficient in get RTP packets from the hint track
    
    The FastCopyMacros replace calls to memcpy for moving chars, shorts, longs etc.
    
    fixed a bug in B-frame thinning.  would previously discard all packets in any sample
    containing one or more b-frame packets.  seems unlikely this would ever be seen in 
    the real world.
    
*/


// -------------------------------------
// Includes
//
#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include <string.h>

#include "QTFile.h"

#include "QTAtom.h"
#include "QTAtom_hinf.h"
#include "QTAtom_tref.h"

#include "QTHintTrack.h"
#include "OSMutex.h"
#include "FastCopyMacros.h"
#include "MyAssert.h"
#include "OSMemory.h"
#include "OS.h"

// -------------------------------------
// Macros
//

#define TEMP_PRINT_ON 0

#if TEMP_PRINT_ON

#define TEMP_PRINT(s)  qtss_printf( s )
#define TEMP_PRINT_ONE( s, p1 )   qtss_printf( s, p1 )
    #define TEMP_PRINT_TWO( s, p1, p2 )   qtss_printf( s, p1, p2 )

#else

    #define TEMP_PRINT(s)  
    #define TEMP_PRINT_ONE( s, p1 )
    #define TEMP_PRINT_TWO( s, p1, p2 ) 

#endif

#define DEBUG_PRINT(s) if(fDebug) qtss_printf s
#define DEEP_DEBUG_PRINT(s) if(fDeepDebug) qtss_printf s

#define ENABLE_REPEAT_DROPPING 1

#define TESTTIME 0

// -------------------------------------
#if TESTTIME 
#include "OS.h"
#include <sys/time.h>


static Bool16 timerStarted = false;
static Bool16 doneMediaCount = false;
static Bool16 doneHintCount = false;
static SInt64 totalMediaSampleReadTime = 0;
static SInt64 totalHintSampleReadTime = 0;
enum {eMicro = 1000000, eMilli =1000};
#define kMaxPacketCount 10000
static SInt32 mediaPacketCount = 0;
static SInt32 hintPacketCount = 0;
static SInt32 totalMediaLength = 0;
static SInt32 totalHintLength = 0;
static SInt64 totalMediaReadTime = 0;
static SInt64 totalHintReadTime = 0;


SInt64 GetMicroseconds()
{
    return OS::Milliseconds();
}
#endif

// -------------------------------------
// Class state cookie
//
QTHintTrack_HintTrackControlBlock::QTHintTrack_HintTrackControlBlock(QTFile_FileControlBlock * FCB)
    : fFCB(FCB),
    
      fCachedSampleNumber(0),
      fCachedSample(NULL),
      fCachedSampleSize(0), fCachedSampleLength(0),

      fCachedHintTrackSampleNumber(0), fCachedHintTrackSampleOffset(0),
      fCachedHintTrackSample(NULL),
      fCachedHintTrackSampleLength(0),
      fLastPacketNumberFetched(0xFFFF),
      fPointerToNextPacket(NULL),
      
      fRTPMetaInfoFieldArray(NULL),
      fSyncSampleCursor(0),
      
      fCurrentPacketNumber(0),
      fCurrentPacketPosition(0)
{
    fMediaTrackSTSC_STCB = NULL;
    fMediaTrackRefIndex = -2;
}

QTHintTrack_HintTrackControlBlock::~QTHintTrack_HintTrackControlBlock(void)
{
    delete fMediaTrackSTSC_STCB;
    delete []fCachedSample;
    delete []fCachedHintTrackSample;
    
    delete [] fRTPMetaInfoFieldArray;
}

void QTHintTrack_HintTrackControlBlock::Reset()
{
    fSyncSampleCursor = 0;
    fCurrentPacketNumber = 0;
    fCurrentPacketPosition = 0;
}


// -------------------------------------
// Constructors and destructors
//
QTHintTrack::QTHintTrack(QTFile * File, QTFile::AtomTOCEntry * Atom, Bool16 Debug, Bool16 DeepDebug)
    : QTTrack(File, Atom, Debug, DeepDebug),
      fHintInfoAtom(NULL), fHintTrackReferenceAtom(NULL),
      fTrackRefs(NULL),
      fMaxPacketSize(65536),
      fRTPTimescale(0),
      fFirstRTPTimestamp(0),
      fTimestampRandomOffset(0),
      fSequenceNumberRandomOffset(0),
      fHintTrackInitialized(false),
      fHintType(QTHintTrack::kUnknown),
      fFirstTransmitTime(0.0),
      fAllowInvalidHintRefs(false)
{
#if TESTTIME
    qtss_printf(" QTHintTrack initialized \n"); 
    mediaPacketCount = 0;
    hintPacketCount = 0;
    totalMediaSampleReadTime = 0;
    totalHintSampleReadTime = 0;
    totalMediaReadTime = 0;
    totalHintReadTime = 0;
#endif

}

QTHintTrack::~QTHintTrack(void)
{

    //
    // Free our variables
    if( fHintInfoAtom != NULL )
        delete fHintInfoAtom;
    if( fHintTrackReferenceAtom != NULL )
        delete fHintTrackReferenceAtom;
        
    if( fTrackRefs != NULL )
        delete[] fTrackRefs;
}



// -------------------------------------
// Initialization functions
//
QTTrack::ErrorCode QTHintTrack::Initialize(void)
{
    // General vars
    char        *sampleDescription, *pSampleDescription;
    UInt32      sampleDescriptionLength;

    QTFile::AtomTOCEntry    *hinfTOCEntry;

    //
    // Don't initialize more than once.
    if( IsHintTrackInitialized() )
        return errNoError;

    //
    // Initialize the QTTrack class.
    if( QTTrack::Initialize() != errNoError )
        return errInvalidQuickTimeFile;


    //
    // Get the sample description table for this track and verify that it is an
    // RTP track.
    if( !fSampleDescriptionAtom->FindSampleDescription(FOUR_CHARS_TO_INT('r', 't', 'p', ' '), &sampleDescription, &sampleDescriptionLength) )
        return errInvalidQuickTimeFile;

    ::memcpy(&fMaxPacketSize, sampleDescription + 20, 4);
    fMaxPacketSize = ntohl(fMaxPacketSize);
    
    for( pSampleDescription = (sampleDescription + 24);
         pSampleDescription < (sampleDescription + sampleDescriptionLength);
    ) {
        // General vars
        UInt32      entryLength, dataType;
        
        
        //
        // Get the entry length and data type of this entry.
        ::memcpy( &entryLength, pSampleDescription + 0, 4);
        entryLength = ntohl(entryLength);
        
        ::memcpy( &dataType, pSampleDescription + 4, 4);
        dataType = ntohl(dataType);
        
        //
        // Process this data type.
        switch( dataType ) {
            case FOUR_CHARS_TO_INT('t', 'i', 'm', 's'): // tims RTP timescale
                ::memcpy(&fRTPTimescale, pSampleDescription + 8, 4);
                fRTPTimescale = ntohl(fRTPTimescale);
            break;
            
            case FOUR_CHARS_TO_INT('t', 's', 'r', 'o'): // tsro Timestamp random offset
                ::memcpy(&fTimestampRandomOffset, pSampleDescription + 8, 4);
                fTimestampRandomOffset = ntohl(fTimestampRandomOffset);
            break;
            
            case FOUR_CHARS_TO_INT('s', 'n', 'r', 'o'): // snro Sequence number random offset
                ::memcpy(&fSequenceNumberRandomOffset, pSampleDescription + 8, 2);
                fSequenceNumberRandomOffset = ntohs(fSequenceNumberRandomOffset);
            break;
        }
        
        //
        // Next entry..
        pSampleDescription += entryLength;
    }
    
    

    //
    // Load in the hint info atom for this track.
    if( fFile->FindTOCEntry(":udta:hinf", &hinfTOCEntry, &fTOCEntry) ) 
    {
        fHintInfoAtom = NEW QTAtom_hinf(fFile, hinfTOCEntry, fDebug, fDeepDebug);

        if( fHintInfoAtom == NULL )
            return errInternalError;
            
        if( !fHintInfoAtom->Initialize() )
            return errInvalidQuickTimeFile;
    }

    //
    // Load in the hint track reference atom for this track.
    if( !fFile->FindTOCEntry(":tref:hint", &hinfTOCEntry, &fTOCEntry) )
        return errInvalidQuickTimeFile;
    
    fHintTrackReferenceAtom = NEW QTAtom_tref(fFile, hinfTOCEntry, fDebug, fDeepDebug);
    if( fHintTrackReferenceAtom == NULL )
        return errInternalError;
    if( !fHintTrackReferenceAtom->Initialize() )
        return errInvalidQuickTimeFile;
    

    //
    // Allocate space for our track reference table.
    UInt32 numTrackRefs = fHintTrackReferenceAtom->GetNumReferences();
    if (numTrackRefs > QTHintTrack::kMaxHintTrackRefs)
        return errInvalidQuickTimeFile;
       
    fTrackRefs = NEW QTTrack *[numTrackRefs];    
    if( fTrackRefs == NULL )
        return errInternalError;

    ::memset( (void *) fTrackRefs, 0, sizeof(QTTrack*) * numTrackRefs); //initialize ptr array with 0.

    //
    // Locate all of the tracks that we use, but don't initialize them until we
    // actually try to access them.
    for( UInt32 CurRef = 0; CurRef < numTrackRefs; CurRef++ ) 
    {
        // General vars
        UInt32      trackID = 0;
        
        //
        // Get the reference and make sure it's not empty.
        if( !fHintTrackReferenceAtom->TrackReferenceToTrackID(CurRef, &trackID) )
            return errInvalidQuickTimeFile;
               
        //
        // Store away a reference to this track.
        if( !fFile->FindTrack( trackID, &fTrackRefs[CurRef]) )
            if ( !fAllowInvalidHintRefs )
                return errInvalidQuickTimeFile;
    }


    //
    // Calculate the first RTP timestamp for this track.
    if( GetFirstEditMovieTime() > 0 ) 
    {
        UInt64 trackTime = GetFirstEditMovieTime();
        
        trackTime *= fRTPTimescale;
        
        if( fFile->GetTimeScale() > 0.0 )
            trackTime /= (UInt64)fFile->GetTimeScale();
            
        fFirstRTPTimestamp = (UInt32)(trackTime & 0xffffffff);
        
    }
    else
    {
        fFirstRTPTimestamp = 0;
    }

    //
    // This track has been successfully initialiazed.
    fHintTrackInitialized = true;
    
    return errNoError;
}
    


// -------------------------------------
// Accessors.
//
QTTrack::ErrorCode QTHintTrack::GetSDPFileLength(int * length)
{
    QTFile::AtomTOCEntry*   sdpTOCEntry;


    //
    // Locate the 'sdp ' atom for this track.
    if( !fFile->FindTOCEntry(":udta:hnti:sdp ", &sdpTOCEntry, &fTOCEntry) )
        return errInvalidQuickTimeFile;
    
    //
    // Return the length.
    *length = (int) sdpTOCEntry->AtomDataLength;
    
    
    return errNoError;
}

char * QTHintTrack::GetSDPFile(int * length)
{
    QTFile::AtomTOCEntry*   sdpTOCEntry;
    QTAtom*                 sdpAtom;    
    char*                   sdpBuffer;


    //
    // Locate and read in the 'sdp ' atom for this track.
    if( !fFile->FindTOCEntry(":udta:hnti:sdp ", &sdpTOCEntry, &fTOCEntry) )
        return NULL;
    
    sdpAtom = NEW QTAtom(fFile, sdpTOCEntry, fDebug, fDeepDebug);
    if( sdpAtom == NULL )
        return NULL;
    if( !sdpAtom->Initialize() )
        return NULL;
    
    //
    // Allocate a buffer to hold the SDP file.
    *length = (int) sdpTOCEntry->AtomDataLength;
    //sdpBuffer = (char *)malloc(*Length);
    sdpBuffer = NEW char[*length];
    if( sdpBuffer != NULL )
        sdpAtom->ReadBytes(0, sdpBuffer, *length);
    
    //
    // Free the atom and return the buffer.
    delete sdpAtom;
    return sdpBuffer;
}


            
// -------------------------------------
// Sample functions
//


void QTHintTrack::GetSamplePacketHeaderVars( char *samplePacketPtr,char *maxBuffPtr, QTHintTrackRTPHeaderData &hdrData )
{
    // Read in this (packetNumber) packet's header.
    // need to acquire the header info on every pass
    
    MOVE_WORD( hdrData.hintFlags, samplePacketPtr + 8);
    hdrData.hintFlags = ntohs( hdrData.hintFlags );

    MOVE_WORD( hdrData.dataEntryCount, samplePacketPtr + 10);
    hdrData.dataEntryCount = ntohs(hdrData.dataEntryCount);
    
    hdrData.tlvTimestampOffset = 0;
    
    Bool16 tlvOK = false; // reset tlvSize to 0 if the size value or the tlv flag is invalid
    if( hdrData.hintFlags & 0x4 ) do // Extra Information TLV is present
    {
        hdrData.tlvSize = ntohl( *(UInt32*) (samplePacketPtr + 12) );
        char* tlvParser = samplePacketPtr + 16; // start of tlv data
        char* tlvEnd = tlvParser + hdrData.tlvSize;// 1 past the end of tlv data
        
        if (tlvEnd >= maxBuffPtr || tlvParser >= maxBuffPtr || tlvEnd < samplePacketPtr || tlvParser < samplePacketPtr)
        {   
            WarnV(tlvParser < maxBuffPtr, "incorrect tlv data: ignoring"); // error tlv-start past buffer
            WarnV(tlvEnd < maxBuffPtr, "incorrect tlv data: ignoring"); // error tlv-end past buffer
            WarnV(tlvEnd >= samplePacketPtr, "incorrect tlv data: ignoring"); // error tlv-end before start of sample in buffer
            WarnV(tlvParser >= samplePacketPtr, "incorrect tlv data: ignoring"); // error tlv-start before start of sample in buffer
            break;
        }
        
        tlvOK = true; // at this point hdrData.tlvSize is ok. The size is used to calculate the next packet so it better be correct.

        // if there is a TLV, parse out the 1 field we currently know about, the 'rtpo' field
        while (tlvParser + 12 < tlvEnd) // test for the minimum tlv size (size+type+smallest data size)
        {           
            UInt32  fieldSize   = ntohl( *(UInt32*) tlvParser );
            UInt32  theType     = ntohl( *(UInt32*) (tlvParser + 4) );
            UInt32  theData     = ntohl( *(UInt32*) (tlvParser + 8) ); //data can't be smaller 4 and we only know about rtpo data

            if (theType == FOUR_CHARS_TO_INT( 'r', 't', 'p', 'o' )) //'rtpo'
            {
                // This is the RTP timestamp TLV. Record it
                hdrData.tlvTimestampOffset = theData;
            }
            
            tlvParser += fieldSize;// add the field size to current for the next TLV entry
        }

    } while (false);
    
    if (!tlvOK)
    {   hdrData.tlvSize = 0;
    }   

    MOVE_LONG_WORD( hdrData.relativePacketTransmissionTime, samplePacketPtr );  
    hdrData.relativePacketTransmissionTime = ntohl(hdrData.relativePacketTransmissionTime);
    
    MOVE_WORD( hdrData.rtpHeaderBits, samplePacketPtr + 4);
    MOVE_WORD( hdrData.rtpSequenceNumber, samplePacketPtr + 6);
    hdrData.rtpSequenceNumber = ntohs(hdrData.rtpSequenceNumber);       

}


QTTrack::ErrorCode QTHintTrack::GetSamplePacketPtr( char ** samplePacketPtr, UInt32 sampleNumber, UInt16 packetNumber
                                                    , QTHintTrackRTPHeaderData  &hdrData, QTHintTrack_HintTrackControlBlock& htcb)
{
    // get a pointer to the packetNumber # in sampleNumber #, from the QTHintTrack_HintTrackControlBlock htcb
    // return a pointer to the data in samplePacketPtr ( past the header info we read into QTHintTrackRTPHeaderData )
    // and fill in the QTHintTrackRTPHeaderData fields

    // you must insure that the sampleNumber you want is already cached
    // use GetSamplePtr to do this
    
    // on exit fLastPacketNumberFetched = packetNumber
    //      fPointerToNextPacket points to 1 past the end of the current packet
    
    
    Assert( sampleNumber == htcb.fCachedSampleNumber );
    
    if( htcb.fLastPacketNumberFetched != 0xFFFF &&  packetNumber == htcb.fLastPacketNumberFetched + 1 )
    {       
        // we're still looking at the same cached sample, and we just want the next packet in that sample       
        Assert( htcb.fPointerToNextPacket >= htcb.fCachedSample );
        
        char* maxBuffPtr = (char*)(htcb.fCachedSample + htcb.fCachedSampleLength);
        Assert( htcb.fPointerToNextPacket < maxBuffPtr );
    
        this->GetSamplePacketHeaderVars( htcb.fPointerToNextPacket,maxBuffPtr,  hdrData );
        
        // adjust fPointerToNextPacket past current header data
        // return this in samplePacketPtr, it points to the hint data table now.
        htcb.fPointerToNextPacket += (4 + 2 + 2 + 2 + 2) + hdrData.tlvSize;
        *samplePacketPtr = htcb.fPointerToNextPacket;
        
        // now adjust fPointerToNextPacket to point at the header of the next packet
        htcb.fPointerToNextPacket += hdrData.dataEntryCount * 16;
            
        // validate the buffer
        htcb.fLastPacketNumberFetched++;
        Assert( htcb.fPointerToNextPacket <= (char*)( htcb.fCachedSample + htcb.fCachedSampleLength ) );
        
        if( htcb.fPointerToNextPacket > ( htcb.fCachedSample + htcb.fCachedSampleLength ) )
            return errInvalidQuickTimeFile;
        
    }
    else
    {
        Assert( sampleNumber == htcb.fCachedSampleNumber );
                
        char* pSampleBuffer = htcb.fCachedSample;       
        char* pSampleBufferEnd = (char*) ( pSampleBuffer + htcb.fCachedSampleLength );
        pSampleBuffer += 4;

        // Loop through the sample until we find the packet that we want.       
        for( UInt16 curPacket = 0; curPacket != packetNumber; curPacket++ ) 
        {
        
            this->GetSamplePacketHeaderVars( pSampleBuffer,pSampleBufferEnd, hdrData );

            // Adjust (and check) pSampleBuffer)
            pSampleBuffer += (4 + 2 + 2 + 2 + 2) + hdrData.tlvSize;
            if( pSampleBuffer > pSampleBufferEnd )
                return errInvalidQuickTimeFile;
            
            // Skip over the data table entries if this is *not* the packet that
            // we want.
            if( (curPacket + 1) != packetNumber ) 
            {
                pSampleBuffer += hdrData.dataEntryCount * 16;
                if( pSampleBuffer > pSampleBufferEnd )
                    return errInvalidQuickTimeFile;
            }
        }
        
        
        htcb.fPointerToNextPacket = pSampleBuffer + (hdrData.dataEntryCount * 16);
        htcb.fLastPacketNumberFetched = packetNumber;
        
    #if DEBUG
        // validate that sample fits within the buffer provided..
        Assert ( htcb.fPointerToNextPacket <= pSampleBufferEnd );
    #endif
    
        if ( htcb.fPointerToNextPacket > pSampleBufferEnd )
            return errInvalidQuickTimeFile;
        
        *samplePacketPtr = pSampleBuffer;
    }
    
    return errNoError;
}


Bool16 QTHintTrack::GetSamplePtr(UInt32 sampleNumber, char ** samplePtr, UInt32 * length, QTHintTrack_HintTrackControlBlock * htcb)
{
    // General vars
    UInt32      newSampleLength;
    Assert(htcb != NULL);

    // See if this sample is in our cache, returning it out of the cache if it
    // is, fetching and caching it if it is not.
    if( sampleNumber == htcb->fCachedSampleNumber ) 
    {
        *samplePtr = htcb->fCachedSample;
        *length = htcb->fCachedSampleLength;
        return true;
    }
    
    //TEMP_PRINT( "QTHintTrack::GetSamplePtr; sample not cached\n" );
    
    htcb->fLastPacketNumberFetched = 0xFFFF;    // mark to invalid
    htcb->fPointerToNextPacket = NULL;

    //
    // Get the length of the new sample.
    UInt32      sampleDescriptionIndex;
    UInt64      sampleOffset;
    
    if( !this->GetSampleInfo(sampleNumber, &newSampleLength, &sampleOffset, &sampleDescriptionIndex, &htcb->fstscSTCB) )
        return false;
    
    //
    // Create a new (bigger) cache samplePtr if the sample wouldn't fit in the
    // old one.
    if( (htcb->fCachedSample == NULL) || (htcb->fCachedSampleSize < newSampleLength) ) 
    {
        //
        // Free the old cache entry if we had one.
        if( htcb->fCachedSample != NULL ) 
        {
            htcb->fCachedSampleNumber = 0;
            htcb->fCachedSampleSize = 0;
            delete[] htcb->fCachedSample;
        }
        
        //
        // Create a new cache entry.
        htcb->fCachedSampleLength = htcb->fCachedSampleSize = newSampleLength;
        htcb->fCachedSample = NEW char[htcb->fCachedSampleSize];
        if( htcb->fCachedSample == NULL )
            return false;
    }
    

    //
    // Read in the new sample.
    htcb->fCachedSampleLength = newSampleLength;
    
    //- this did another GetSampleInfo and we already have that data...
    //if( !this->GetSample(sampleNumber, htcb->fCachedSample, &htcb->fCachedSampleLength, htcb->fFCB, htcb->fstscSTCB) )
    //  return false;
    
    //
    // Read in the sample
    if( !fDataReferenceAtom->Read( sampleDescriptionIndex, sampleOffset, htcb->fCachedSample, htcb->fCachedSampleLength, htcb->fFCB ) )
        return false;
    //
    // Return the cached sample.
    htcb->fCachedSampleNumber = sampleNumber;
    *samplePtr = htcb->fCachedSample;
    *length = htcb->fCachedSampleLength;
    
    
    return true;
}



// -------------------------------------
// Packet functions
//
QTTrack::ErrorCode QTHintTrack::GetNumPackets(UInt32 sampleNumber, UInt16 * numPackets, QTHintTrack_HintTrackControlBlock * htcb)
{
    char        *buf;
    UInt32      bufLen;
    UInt16      entryCount;
    
    
    //
    // Read this sample and figure out how many packets are in it.
    if( !this->GetSamplePtr(sampleNumber, &buf, &bufLen, htcb) )
        return errInvalidQuickTimeFile;
    
    MOVE_WORD( entryCount, buf );
    //::memcpy(&entryCount, buf, 2);
    
    *numPackets = ntohs(entryCount);

    return errNoError;
}


QTTrack::ErrorCode QTHintTrack::GetSampleData( QTHintTrack_HintTrackControlBlock * htcb, char **buffPtr, char **ppPacketBufOut, UInt32 sampleNumber, UInt16 packetNumber, UInt32 buffOutLen )
{

//  qtss_printf("GetSampleData sampleNumber = %"_U32BITARG_" packetNumber = %"_U32BITARG_" buffOutLen = %"_U32BITARG_" \n",sampleNumber, packetNumber, buffOutLen);
    // General vars
    SInt8       trackRefIndex = 0;
    UInt16      readLength = 0;
    UInt32      mediaSampleNumber = 0;
    UInt32      readOffset = 0;
    UInt16      bytesPerCompressionBlock = 0;
    UInt16      samplesPerCompressionBlock= 0;  // inititialization eliminates a stupid compiler warning :(
    UInt32      sampleDescriptionIndex;
    UInt64      dataOffset;
    char*       pBuf = NULL;
    char*       maxBuffPtr = NULL;
    char*       buffOutPtr = NULL;
    QTTrack     *track = NULL;
    UInt32      samplesPerChunk = 0;
    UInt32      chunkNumber = 0; 
    UInt64      chunkOffset = 0; 
    UInt32      sampleOffsetInChunk = 0; 
    UInt32      sampleLength = 0;
    UInt64      cacheHintSampleLen = 0;
    SInt32      hintMaxRead = 0;
    SInt64      sizeOfSamplesInChunk =0;
    SInt64      endOfSampleInChunk = 0;
    SInt64      sampleFirstPartLength = 0;
    SInt64      remainingLength = 0; 
    Bool16      isOneForOne = false;
    Bool16      isCompressed = false;
    
    QTAtom_stsc_SampleTableControlBlock * mediaTrackSTSC_STCBPtr = NULL;

#if TESTTIME
    Bool16 isMediaSample = false;
    Bool16 isHintSample = false;
    SInt64 startTime = GetMicroseconds();
    SInt64 readStart = 0;
#endif

    if (NULL == ppPacketBufOut || NULL == *ppPacketBufOut) return errInternalError;
    if (NULL == buffPtr || NULL == *buffPtr) return errInternalError;
    if (buffOutLen <= 0) return errInternalError;
    pBuf = *buffPtr;
    maxBuffPtr = *ppPacketBufOut + buffOutLen -1;
    buffOutPtr = *ppPacketBufOut;
    cacheHintSampleLen = buffOutLen;
    //
    // Get the information about this sample
    trackRefIndex = (SInt8)*(pBuf + 1);

    MOVE_WORD( readLength, pBuf + 2);
    cacheHintSampleLen = hintMaxRead = readLength = ntohs(readLength);
    
    MOVE_LONG_WORD( mediaSampleNumber, pBuf + 4);
    mediaSampleNumber = ntohl(mediaSampleNumber);

    MOVE_LONG_WORD( readOffset, pBuf + 8);
    readOffset = ntohl(readOffset);
    
    MOVE_WORD( bytesPerCompressionBlock, pBuf + 12);
    bytesPerCompressionBlock = ntohs(bytesPerCompressionBlock);
    if( bytesPerCompressionBlock == 0 )
        bytesPerCompressionBlock = 1;
    
    MOVE_WORD( samplesPerCompressionBlock, pBuf + 14);
    samplesPerCompressionBlock = ntohs(samplesPerCompressionBlock);
    if( samplesPerCompressionBlock == 0 )
        samplesPerCompressionBlock = 1;

    DEEP_DEBUG_PRINT(("QTHintTrack::GetPacket - ....Sample entry found (sample#=%"_U32BITARG_"; offset=%"_U32BITARG_"; length=%u; BPCB=%u; SPCB=%u)\n", mediaSampleNumber, readOffset, readLength, bytesPerCompressionBlock, samplesPerCompressionBlock));
    
    if( trackRefIndex == -1 ) 
    {
        if (kUnknown == fHintType)
            fHintType = kOptimized;
//      qtss_printf("hint track sample = %"_S32BITARG_" \n", mediaSampleNumber);
        //
        // We're getting data out of the hint track..
        #if TESTTIME
            isHintSample = true;
            totalHintLength += readLength;
        #endif

        track = this;
        
        //
        // ..this might be the Sample Description that is stored in the first
        // few samples of this track.  See if we have it cached and use that
        // if so.  If we do not have this block of data cached, then continue
        // on; we'll cache it later.
        if (   (mediaSampleNumber == htcb->fCachedHintTrackSampleNumber)
                && (readOffset == htcb->fCachedHintTrackSampleOffset)
                && (readLength == htcb->fCachedHintTrackSampleLength)
            ) 
        {
//          qtss_printf("found cached hint sample = %"_S32BITARG_" \n", mediaSampleNumber);
            if ( (char *) (*ppPacketBufOut + readLength -1) > maxBuffPtr )
            {   return errInvalidQuickTimeFile;
            }
                
            ::memcpy( *ppPacketBufOut, htcb->fCachedHintTrackSample, readLength);
            *ppPacketBufOut += readLength;
            
            #if TESTTIME
            if (hintPacketCount < kMaxPacketCount)
            {   totalHintSampleReadTime += (GetMicroseconds() -  startTime);
                hintPacketCount ++;
            } 
            if (hintPacketCount >= kMaxPacketCount)
            {
//                  qtss_printf("hintPacketCount = %"_S32BITARG_" hint get info time = %f hint bytes read = %d read time = %f\n", hintPacketCount, (float) (totalHintSampleReadTime - totalHintReadTime)  / eMilli,totalHintLength, (float)  totalHintReadTime/ eMilli);
                hintPacketCount = 0;
                totalHintSampleReadTime = 0;
                totalHintLength = 0;
                totalHintReadTime = 0;
            }
            #endif
            
            return errNoError;
        }
        
        //
        // ..or this might be in our currently-cached data sample.

        if  (       (mediaSampleNumber == htcb->fCachedSampleNumber)
                && ((readOffset + readLength) <= htcb->fCachedSampleLength)
            ) 
        {
//          qtss_printf("found currently cached sample = %"_S32BITARG_" \n", mediaSampleNumber);
            if ( (char *) (*ppPacketBufOut + readLength -1) > maxBuffPtr)
            {   return errInvalidQuickTimeFile;
            }

            ::memcpy( *ppPacketBufOut, htcb->fCachedSample + readOffset, readLength);
            *ppPacketBufOut += readLength;
            
            #if TESTTIME
            if (hintPacketCount < kMaxPacketCount)
            {   totalHintSampleReadTime += (GetMicroseconds() -  startTime);
                hintPacketCount ++;
            } 
            
            if (hintPacketCount >= kMaxPacketCount) 
            {
//              qtss_printf("hintPacketCount = %"_S32BITARG_" hint get info time = %f hint bytes read = %d read time = %f\n", hintPacketCount, (float) (totalHintSampleReadTime - totalHintReadTime) / eMilli,totalHintLength, (float)  totalHintReadTime/ eMilli);
                hintPacketCount = 0;
                totalHintSampleReadTime = 0;
                totalHintLength = 0;
                totalHintReadTime = 0;
            }
            #endif
        
            return errNoError;
        }

        mediaTrackSTSC_STCBPtr = &htcb->fstscSTCB;
        if( !track->GetSampleInfo(mediaSampleNumber,&sampleLength, &dataOffset, &sampleDescriptionIndex, mediaTrackSTSC_STCBPtr ) )
            return errInvalidQuickTimeFile;

        dataOffset += readOffset; 
        
        if ( (char *) (*ppPacketBufOut + readLength -1) > maxBuffPtr)
        {   return errInvalidQuickTimeFile;
        }

        if( !track->Read(sampleDescriptionIndex, dataOffset, *ppPacketBufOut, readLength, htcb->fFCB) )
            return (errInvalidQuickTimeFile);
        *ppPacketBufOut += readLength;  // point to remainder of buffer;    


    }    // trackRefIndex == -1
    else 
    {   //  trackRefIndex != -1 

        // We're getting data out of a media track..
        if (kUnknown == fHintType || kOptimized == fHintType) // if uninitialized or self-referencing then reset
            fHintType = kUnoptimized;

//      qtss_printf("media track sample = %"_S32BITARG_" \n", mediaSampleNumber);
            
        #if TESTTIME
                isMediaSample = true;
                totalMediaLength += readLength;
        #endif
        
        track = fTrackRefs[trackRefIndex];        
        if( 0 == track )
             return errInvalidQuickTimeFile;

        
        // Initialize this track if we haven't done so yet.
        if( !track->IsInitialized() )
        {   TEMP_PRINT_ONE( "QTHintTrack::GetPacket trackRefIndex %li, not initialized,\n", (SInt32)trackRefIndex );  
            OSMutexLocker theLocker(fFile->GetMutex());
            
            TEMP_PRINT("Initializing media track\n");
            if( track->Initialize() != QTTrack::errNoError )
                return errInvalidQuickTimeFile;
        }

        if (htcb->fMediaTrackRefIndex == -2) // initial value : -1 is hint track and 0 is first index so use -2 as uninitialized
        {   
            htcb->fMediaTrackSTSC_STCB = NEW QTAtom_stsc_SampleTableControlBlock();
            htcb->fMediaTrackRefIndex = trackRefIndex;
        }
        
        if(htcb->fMediaTrackRefIndex == trackRefIndex)
        {   
            mediaTrackSTSC_STCBPtr = htcb->fMediaTrackSTSC_STCB;
        }

        if( !track->GetSampleInfo(mediaSampleNumber,&sampleLength, &dataOffset, &sampleDescriptionIndex, mediaTrackSTSC_STCBPtr ) )
            return errInvalidQuickTimeFile;

        if ( (1 == samplesPerCompressionBlock) && (1 == bytesPerCompressionBlock) )  
                isOneForOne = true;
            

        if  (   (isOneForOne && sampleLength == 1)  // special case  (isOneForOne && sampleLength == 1) //  the data is compressed and the sample's byte offset is really a byte offset into a chunk
                || (!isOneForOne)   // compressed data is normally defined by bytesPerCompressionBlock or samplesPerCompressionBlock 
            )
        {
//          qtss_printf("track = %"_S32BITARG_" sample  = %"_S32BITARG_" is compressed \n",this, mediaSampleNumber);
//          qtss_printf("is compressed bytesPerCompressionBlock = %"_S32BITARG_" samplesPerCompressionBlock = %d sampleLength = %"_S32BITARG_"\n", bytesPerCompressionBlock, samplesPerCompressionBlock, sampleLength);
            isCompressed = true;
        }

            
        //
        // Get the information about this sample and compute an offset.  If the BPCB
        // and SPCB are 1, then we use the standard sample routines to get the location
        // of this sample, otherwise we have to compute it ourselves.

        if (isCompressed)
        {   // Media track sample compressed
        
            UInt32  compressionBlocksToSkip  = (UInt32) ( (Float64) readOffset / (Float64) bytesPerCompressionBlock);
            mediaSampleNumber               += compressionBlocksToSkip * samplesPerCompressionBlock;
            readOffset                      -= compressionBlocksToSkip * bytesPerCompressionBlock; // readoffset should always be 0 after this 
            // start gathering chunk info to check sample length against chunk length
            if( !track->SampleToChunkInfo(mediaSampleNumber, &samplesPerChunk, &chunkNumber, NULL, &sampleOffsetInChunk,mediaTrackSTSC_STCBPtr) )
                return (errInvalidQuickTimeFile);

            if( !track->ChunkOffset(chunkNumber, &chunkOffset) )
                return (errInvalidQuickTimeFile);

            dataOffset  = (UInt64) chunkOffset  + (UInt64) ((Float64) sampleOffsetInChunk * ((Float64)bytesPerCompressionBlock / (Float64)samplesPerCompressionBlock));

            if( !track->GetSizeOfSamplesInChunk(chunkNumber,  (UInt32 *) &sizeOfSamplesInChunk , NULL , NULL , mediaTrackSTSC_STCBPtr) )
                return (errInvalidQuickTimeFile);
        
            sizeOfSamplesInChunk = (UInt32) ( (Float64) sizeOfSamplesInChunk * ((Float64) bytesPerCompressionBlock / (Float64)samplesPerCompressionBlock) );

            endOfSampleInChunk      = sizeOfSamplesInChunk + chunkOffset;
            sampleFirstPartLength   = endOfSampleInChunk - dataOffset; // the first piece length = maxlen - start
            remainingLength         = readLength - sampleFirstPartLength; // the read - first piece is either <= 0 or the remaining amount.                             

            if ( (remainingLength > 0) && (sampleFirstPartLength > 0) ) // this packet is split across chunks
            {
//              qtss_printf("mediaSampleNumber = %"_S32BITARG_" is compressed and split across chunks first part = %"_S32BITARG_" remaining = %"_S32BITARG_"\n",mediaSampleNumber, readLength, remainingLength);
                readLength = (UInt16) sampleFirstPartLength;
            }
            else
            {   // this is still needed. For some movies the compressed split packet calculation doesn't match the simple dataOffset calc below --a problem with sampleOffsetInChunk
//              qtss_printf("compressed but not split across chunks \n");
//              qtss_printf("chunkOffset = %"_S32BITARG_"  ",chunkOffset);
//              qtss_printf("old dataOffset = %qd ",dataOffset);
                remainingLength = 0;
                dataOffset = chunkOffset + readOffset + (UInt64)(sampleOffsetInChunk * ((Float64)bytesPerCompressionBlock / (Float64)samplesPerCompressionBlock));
//              qtss_printf("new dataOffset = %qd \n", dataOffset);
            }
            
            hintMaxRead -= readLength;
            if (hintMaxRead < 0) 
            {   return (errInvalidQuickTimeFile); 
            }
            
            #if TESTTIME
//              qtss_printf("Read mediaSampleNumber = %"_S32BITARG_" dataOffset = %qd  readLength = %"_S32BITARG_" remaining = %"_S32BITARG_"\n",mediaSampleNumber, dataOffset, readLength, remainingLength);
                readStart = GetMicroseconds();
                        
                if ( (char *) (*ppPacketBufOut + readLength -1) > maxBuffPtr)
                {   return errInvalidQuickTimeFile;
                }

                if( !track->Read(sampleDescriptionIndex, dataOffset, *ppPacketBufOut, readLength, htcb->fFCB) )
                    return (errInvalidQuickTimeFile);
                totalMediaReadTime += GetMicroseconds() - readStart;
            #else
//              qtss_printf("Read mediaSampleNumber = %"_S32BITARG_" dataOffset = %qd  readLength = %"_S32BITARG_" remaining = %"_S32BITARG_"\n",mediaSampleNumber, dataOffset, readLength, remainingLength);
    
                if ( (char *) (*ppPacketBufOut + readLength -1) > maxBuffPtr)
                {   return errInvalidQuickTimeFile;
                }
                    
                if( !track->Read(sampleDescriptionIndex, dataOffset, *ppPacketBufOut, readLength, htcb->fFCB) )
                    return (errInvalidQuickTimeFile);
                    
            #endif
    
                    
            *ppPacketBufOut += readLength;  // point to remainder of buffer;    
    
    
    
            while (remainingLength > 0)  // loop if packet is split across more than just two chunks
            {   
//              qtss_printf("mediaSampleNumber = %"_S32BITARG_" remaining read  = %"_S32BITARG_"\n",mediaSampleNumber, remainingLength);
            
                readLength = (UInt16)  remainingLength; // set the read to what is left           
                chunkNumber++; // The rest of the sample is in the next N chunks 
                if( !track->ChunkOffset(chunkNumber, &chunkOffset) ) // Get the Next chunk location
                {   
                    // It seems some movies have sample lengths that overstep the end of the last chunk.
                    // if that happens, truncate?
                    // Below is a commented out work around for bad offsets. It is commented out because if it happens we should report something is wrong with the file.
                    // remainingLength = 0;
                    // break;
                    return (errInvalidQuickTimeFile); 
                }
                                            
                dataOffset = chunkOffset;    // the location of the data starting at the beginning of the chunk             
                if( !track->GetSizeOfSamplesInChunk(chunkNumber,  (UInt32 *) &sizeOfSamplesInChunk , NULL , NULL , mediaTrackSTSC_STCBPtr) )
                    return (errInvalidQuickTimeFile);
                            
                sizeOfSamplesInChunk = (UInt32) ( (Float64) sizeOfSamplesInChunk * ((Float64) bytesPerCompressionBlock / (Float64)samplesPerCompressionBlock) );
                
                if (sizeOfSamplesInChunk < remainingLength) // read in the whole chunk and keep going
                {   
                    remainingLength -= sizeOfSamplesInChunk;
                    readLength = (UInt16) sizeOfSamplesInChunk;
                }
                else
                {   
                    remainingLength =  0; // done reading this packet
                }
                            
                hintMaxRead -= readLength;
                if (hintMaxRead < 0) 
                {   return (errInvalidQuickTimeFile); 
                }
                
                #if TESTTIME
//                  qtss_printf("Read next parts mediaSampleNumber = %"_S32BITARG_" dataOffset = %qd  readLength = %"_S32BITARG_" remaining = %"_S32BITARG_"\n",mediaSampleNumber, dataOffset, readLength, remainingLength);
                    readStart = GetMicroseconds();
                    
                    if ( (char *) (*ppPacketBufOut + readLength -1) > maxBuffPtr)
                    {   return errInvalidQuickTimeFile;
                    }

                    if( !track->Read(sampleDescriptionIndex, dataOffset, *ppPacketBufOut, readLength, htcb->fFCB) )
                           return errInvalidQuickTimeFile;
                    totalMediaReadTime += GetMicroseconds() - readStart;
                #else
                    if ( (char *) (*ppPacketBufOut + readLength -1) > maxBuffPtr)
                    {   return errInvalidQuickTimeFile;
                    }

                    if( !track->Read(sampleDescriptionIndex, dataOffset, *ppPacketBufOut, readLength, htcb->fFCB) )
                    {   return errInvalidQuickTimeFile; 
                    }
                #endif       
                
                    *ppPacketBufOut += readLength;  // point to remainder of buffer;    
            }
    
            
        }
        else
        {   // Media track sample not compressed
            dataOffset += readOffset; 

            if ( (char *) (*ppPacketBufOut + readLength -1) > maxBuffPtr)
            {   return errInvalidQuickTimeFile;
            }


            if( !track->Read(sampleDescriptionIndex, dataOffset, *ppPacketBufOut, readLength, htcb->fFCB) )
                return (errInvalidQuickTimeFile);

            *ppPacketBufOut += readLength;  // point to remainder of buffer;    
        }


        *buffPtr = pBuf;

    }

    // If this chunk of data came out of our hint track; then we should cache
    // it, just in case we want it later.
    if( (trackRefIndex == -1) && (mediaSampleNumber < sampleNumber) ) 
    {

        if( htcb->fCachedHintTrackSample == NULL ) 
        {
//          qtss_printf("create a cache buffer for %"_S32BITARG_" of size %"_S32BITARG_"\n",mediaSampleNumber,cacheHintSampleLen); 
            htcb->fCachedHintTrackSample = NEW char[ (SInt32) cacheHintSampleLen];
            htcb->fCachedHintTrackBufferLength = (SInt32) cacheHintSampleLen;
        
        }
        else if (htcb->fCachedHintTrackBufferLength < cacheHintSampleLen )
        {
//          qtss_printf("reallocate a cache buffer for %"_S32BITARG_" of size %"_S32BITARG_"\n",mediaSampleNumber,cacheHintSampleLen); 
            htcb->fCachedHintTrackSampleNumber = 0;
            htcb->fCachedHintTrackSampleOffset = 0;
            delete[] htcb->fCachedHintTrackSample;
            
            htcb->fCachedHintTrackSample = NEW char[ (SInt32) cacheHintSampleLen];
            htcb->fCachedHintTrackBufferLength = (SInt32) cacheHintSampleLen;
        }
        
        if (htcb->fCachedHintTrackSampleNumber != mediaSampleNumber)
        {
            
            if( htcb->fCachedHintTrackSample != NULL ) 
            {
//              qtss_printf("cache a hint sample sampleNumber %"_S32BITARG_" readLength = %"_S32BITARG_"\n",mediaSampleNumber,cacheHintSampleLen); 
                ::memcpy(htcb->fCachedHintTrackSample, buffOutPtr, (UInt32) cacheHintSampleLen);
                htcb->fCachedHintTrackSampleNumber = mediaSampleNumber;
                htcb->fCachedHintTrackSampleOffset = readOffset;
                htcb->fCachedHintTrackSampleLength = (UInt32) cacheHintSampleLen;
            }
        }
    }
        
    #if TESTTIME
        if (mediaPacketCount < kMaxPacketCount) 
        {   totalMediaSampleReadTime += (GetMicroseconds() -  startTime);
            mediaPacketCount ++;
        }
        if (mediaPacketCount >= kMaxPacketCount)
        {   
//          qtss_printf("mediaPacketCount = %"_S32BITARG_" media get info time = %f media bytes read = %d read time = %f\n", mediaPacketCount, (float) (totalMediaSampleReadTime - totalMediaReadTime) / eMilli, totalMediaLength, (float)  totalMediaReadTime/ eMilli);
            totalMediaSampleReadTime = 0;
            mediaPacketCount = 0;
            totalMediaLength = 0;
            totalMediaReadTime = 0;
        }
        
        if (isHintSample && (hintPacketCount < kMaxPacketCount))
        {   totalHintSampleReadTime += (GetMicroseconds() -  startTime);
            hintPacketCount ++;
        } 
        else if (isHintSample && (hintPacketCount >= kMaxPacketCount) )
        {
//          qtss_printf("hintPacketCount = %"_S32BITARG_" hint get info time = %f hint bytes read = %d read time = %f\n", hintPacketCount, (float) (totalHintSampleReadTime - totalHintReadTime) / eMilli,totalHintLength, (float)  totalHintReadTime/ eMilli);
            hintPacketCount = 0;
            totalHintSampleReadTime = 0;
            totalHintLength = 0;
            totalHintReadTime = 0;
        }
    #endif

    
    return errNoError;
 
        
}


QTTrack::ErrorCode QTHintTrack::GetPacket(UInt32 sampleNumber, UInt16 packetNumber, char * buffer, UInt32 * length
                        , Float64 * transmitTime, Bool16 dropBFrames, Bool16 dropRepeatPackets, UInt32 ssrc, QTHintTrack_HintTrackControlBlock * htcb)
{
    // Temporary vars
    UInt16      tempInt16;
    UInt32      tempInt32;

    UInt16      curEntry;

    // General vars
    UInt32      mediaTime;
    
    char*       buf;
    UInt32      bufLen;
    
    char*       pSampleBuffer;
    char        *pDataTableStart;

    UInt16      entryCount;
    UInt32      rtpTimestamp;
    
    QTHintTrackRTPHeaderData    hdrData;
    char*       pPacketOutBuf;
    UInt32      packetSize;
    QTTrack::ErrorCode  err = errNoError;
    
    Float64 timeScale = 1.0;
    
    Assert(htcb != NULL);
    
    DEEP_DEBUG_PRINT(("QTHintTrack::GetPacket - Building packet #%u in sample %"_U32BITARG_".\n", packetNumber, sampleNumber));

    //
    // Get the RTP timestamp for this sample.
    if( !this->GetSampleMediaTime(sampleNumber, &mediaTime, &htcb->fsttsSTCB) )
        return errInvalidQuickTimeFile;

    if( fRTPTimescale != this->GetTimeScale() )
        timeScale = (Float64) fRTPTimescale * (Float64) GetTimeScaleRecip();
    
    rtpTimestamp = (UInt32) ((Float64) mediaTime * timeScale);
    rtpTimestamp += fFirstRTPTimestamp;

    //
    // Add the first edit's media time.
    // What about other edits?
    mediaTime += this->GetFirstEditMediaTime(); 

    //
    // Read this sample and generate the n'th packet.
    if( !this->GetSamplePtr(sampleNumber, &buf, &bufLen, htcb) )
        return errInvalidQuickTimeFile;
    
    MOVE_WORD( entryCount, (char *)buf + 0);
    entryCount = ntohs(entryCount);
    if( (packetNumber-1) > entryCount )
        return errInvalidQuickTimeFile;
            
    err = this->GetSamplePacketPtr( &pSampleBuffer, sampleNumber, packetNumber, hdrData, *htcb);
    
    if ( err != errNoError )
    {   return err;
    }
    
#if 0 // ignore relativePacketTransmissionTime offsets
	*transmitTime =  ( mediaTime * fMediaHeaderAtom->GetTimeScaleRecip() );
            
#else //keep relativePacketTransmissionTime offsets.  time 0 base the streams independently. All streams start together regardless of their transmission time.

	*transmitTime =  ( mediaTime * fMediaHeaderAtom->GetTimeScaleRecip() )
                    + ( hdrData.relativePacketTransmissionTime * fMediaHeaderAtom->GetTimeScaleRecip() );

#endif

	// remove negative start times so each track's transmit time is now 0 based 
	if ( hdrData.relativePacketTransmissionTime != 0 && (fFirstTransmitTime == 0.0) )
	{	fFirstTransmitTime = *transmitTime * -1;
	}
	*transmitTime += fFirstTransmitTime; // prevent any negative transmission times by making all the streams 0 based.


    if ( hdrData.hintFlags )
    {   //qtss_printf( "QTHintTrack::GetPacket hintFlags %lx\n", (SInt32)hdrData.hintFlags );      
    }
    
    if ( hdrData.hintFlags & kRepeatPacketMask && (dropRepeatPackets))
    {   //qtss_printf( "QTHintTrack::GetPacket repeat packet dropped.\n" );
        return QTTrack::errIsSkippedPacket;
    }


    if (( hdrData.hintFlags & kBFrameBitMask) && (dropBFrames))
    {   TEMP_PRINT( "QTHintTrack::GetPacket bframe packet dropped.\n" );
        return QTTrack::errIsSkippedPacket;
    }
    

    
    DEEP_DEBUG_PRINT(("QTHintTrack::GetPacket - ..rtpTimestamp=%"_U32BITARG_"; rtpSequenceNumber=%u; transmitTime=%.2f\n", rtpTimestamp, hdrData.rtpSequenceNumber, *transmitTime));

    //
    // We found this packet and the start of our data table.
    pDataTableStart = pSampleBuffer;
    
    //
    // Our first pass through the data table is done to compute the size of the
    // RTP packet that we will be generating and to validate the contents of
    // the data table.

    // and now only one pass!!  the 2 loops are merged now
    
    //
    // Now we go through the data table again, but this time we build the
    // packet.
    
    pPacketOutBuf = buffer;

    //
    // Add in the RTP header.
    tempInt16 = hdrData.rtpHeaderBits | ntohs(0x8000) /* v2 RTP header */;
    COPY_WORD(pPacketOutBuf, &tempInt16);
    
    //TEMP_PRINT_ONE( "QTHintTrack::GetPacket rtpHeaderBits %li.\n", (SInt32)rtpHeaderBits );
    pPacketOutBuf += 2;

    tempInt16 = htons(hdrData.rtpSequenceNumber);
    COPY_WORD(pPacketOutBuf, &tempInt16);
    pPacketOutBuf += 2;

    tempInt32 = rtpTimestamp;
    tempInt32 += hdrData.tlvTimestampOffset; // rtpo tlv offset

    tempInt32 = htonl(tempInt32);
    COPY_LONG_WORD(pPacketOutBuf, &tempInt32);
    pPacketOutBuf += 4;

    tempInt32 = htonl(ssrc);
    COPY_LONG_WORD(pPacketOutBuf, &tempInt32);
    pPacketOutBuf += 4;
    
    //
    // Go through each possible field. For each one, see if caller
    // wants the field appended. If so, append the field
    for ( UInt32 fieldCount = 0; fieldCount < RTPMetaInfoPacket::kNumFields; fieldCount++)
    {
        //
        // If there is no field array, don't generate a packet
        if (htcb->fRTPMetaInfoFieldArray == NULL)
            break;
            
        //
        // Check if field should be appended
        if (htcb->fRTPMetaInfoFieldArray[fieldCount] == RTPMetaInfoPacket::kFieldNotUsed)
            continue;
        
        switch (fieldCount)
        {
            case RTPMetaInfoPacket::kPacketPosField:
            {
                SInt64 curPacketPos = OS::HostToNetworkSInt64(htcb->fCurrentPacketPosition);
                this->WriteMetaInfoField(RTPMetaInfoPacket::kPacketPosField, htcb->fRTPMetaInfoFieldArray[fieldCount], &curPacketPos, sizeof(curPacketPos), &pPacketOutBuf);
                break;
            }
            case RTPMetaInfoPacket::kTransTimeField:
            {
                SInt64 transmitTimeInMsec = OS::HostToNetworkSInt64((SInt64)(*transmitTime * 1000));
                this->WriteMetaInfoField(RTPMetaInfoPacket::kTransTimeField, htcb->fRTPMetaInfoFieldArray[fieldCount], &transmitTimeInMsec, sizeof(transmitTimeInMsec), &pPacketOutBuf);
                break;
            }
            
            case RTPMetaInfoPacket::kFrameTypeField:
            {
                UInt16 theFrameType = RTPMetaInfoPacket::kUnknownFrameType;
                
                if (!htcb->fIsVideo)
                    theFrameType = RTPMetaInfoPacket::kUnknownFrameType;
                else if (hdrData.hintFlags & kBFrameBitMask)
                    theFrameType = RTPMetaInfoPacket::kBFrameType;
                else if (this->IsSyncSample(sampleNumber, htcb->fSyncSampleCursor))
                    theFrameType = RTPMetaInfoPacket::kKeyFrameType;
                else
                    theFrameType = RTPMetaInfoPacket::kPFrameType;

                theFrameType = htons(theFrameType);
                this->WriteMetaInfoField(RTPMetaInfoPacket::kFrameTypeField, htcb->fRTPMetaInfoFieldArray[fieldCount], &theFrameType, sizeof(theFrameType), &pPacketOutBuf);
                break;
            }
            case RTPMetaInfoPacket::kPacketNumField:
            {
                SInt64 curPacketNum = OS::HostToNetworkSInt64(htcb->fCurrentPacketNumber);
                this->WriteMetaInfoField(RTPMetaInfoPacket::kPacketNumField, htcb->fRTPMetaInfoFieldArray[fieldCount], &curPacketNum, sizeof(curPacketNum), &pPacketOutBuf);
                break;
            }
            case RTPMetaInfoPacket::kSeqNumField:
            {
                tempInt16 = htons(hdrData.rtpSequenceNumber);
                this->WriteMetaInfoField(RTPMetaInfoPacket::kSeqNumField, htcb->fRTPMetaInfoFieldArray[fieldCount], &tempInt16, sizeof(tempInt16), &pPacketOutBuf);
                break;
            }
            case RTPMetaInfoPacket::kMediaDataField:
            {
                //
                // This field cannot be compressed
                Assert(htcb->fRTPMetaInfoFieldArray[fieldCount] == RTPMetaInfoPacket::kUncompressed);
                
                //
                // We don't have the data yet, so just write in the header
                this->WriteMetaInfoField(RTPMetaInfoPacket::kMediaDataField, htcb->fRTPMetaInfoFieldArray[fieldCount], NULL, 0, &pPacketOutBuf);
                break;
            }
        }
    }
    
    char* endOfMetaInfo = pPacketOutBuf;
    packetSize = endOfMetaInfo - buffer;
    
    DEEP_DEBUG_PRINT(("QTHintTrack::GetPacket - ..Building packet.\n"));    
    TEMP_PRINT_TWO( "QTHintTrack::GetPacket Building packet %li ; hdrData.dataEntryCount %li .\n", (SInt32)packetNumber ,(SInt32)hdrData.dataEntryCount );
    
    for( curEntry = 0; curEntry < hdrData.dataEntryCount; curEntry++ ) 
    {
        //
        // Get the size out of this entry.
        if ( *pSampleBuffer == 0x02 ) 
        {
            // Sample Mode
            MOVE_WORD( tempInt16, pSampleBuffer + 2);
            tempInt16 = ntohs(tempInt16);

            DEEP_DEBUG_PRINT (( "QTHintTrack::GetPacket - ....Sample entry found (size=%u)\n", tempInt16 ) );
            packetSize += tempInt16;

            if( *length < packetSize )
                return errParamError;
                
            err = this->GetSampleData( htcb, &pSampleBuffer, &pPacketOutBuf, sampleNumber, packetNumber, *length);
            if ( err != errNoError )
                return err;

            // GetSampleData increments our out pointer
        }
        else if ( *pSampleBuffer == 0x01 ) 
        {
            // Immediate Data Mode
            DEEP_DEBUG_PRINT (( "QTHintTrack::GetPacket - ....Immediate entry found (size=%u)\n", (UInt16)*(pSampleBuffer + 1) ) );
            packetSize += *(pSampleBuffer + 1);

            if ( *length < packetSize )
                return errParamError;

            //
            // Copy the data straight into the packet.
            // ( it's data <= 16 bytes, padded out to a full 16 byte of header )
            ::memcpy(pPacketOutBuf, pSampleBuffer + 2, *(pSampleBuffer + 1));
            
            // increment our out pointer
            pPacketOutBuf += *(pSampleBuffer + 1);

        }
        else if ( *pSampleBuffer == 0x03 ) 
        {
            
            // Sample Description Data Mode
            MOVE_WORD( tempInt16, pSampleBuffer + 2);
            tempInt16 = ntohs(tempInt16);

            DEEP_DEBUG_PRINT(("QTHintTrack::GetPacket - ....Sample Description entry found (size=%u)\n", tempInt16));
            packetSize += tempInt16;

            if( *length < packetSize )
                return errParamError;
            // guess we don't handle these??
            DEEP_DEBUG_PRINT(("QTHintTrack::GetPacket - ....Sample Description entry found (size=%u)\n", tempInt16));
            Assert(0);
        }
        else if ( *pSampleBuffer == 0x00 ) 
        {   
            // No-Op Data Mode
            DEEP_DEBUG_PRINT(("QTHintTrack::GetPacket - ....No-Op entry found\n"));
        }
        else
        {
//          qtss_printf("Found unknown RTP data table type!\n");
            Assert(0);
        }
        
        //
        // Move to the next entry.
        pSampleBuffer += 16;
    }
    
    DEEP_DEBUG_PRINT(("QTHintTrack::GetPacket - ..Packet length is %"_U32BITARG_" bytes.\n", packetSize));

    *length = packetSize;
    
    //
    // Always track packet number and packet position.
    UInt16 thePacketDataLen = pPacketOutBuf - endOfMetaInfo;
    htcb->fCurrentPacketNumber++;
    htcb->fCurrentPacketPosition += thePacketDataLen;
        
    //
    // If this is RTP-Meta-Info, well then update the fields we haven't updated yet!!!!!!!
    if (htcb->fRTPMetaInfoFieldArray != NULL)
    {
        //
        // If this is an RTP-Meta-Info packet, and there is no 'md' field, we shouldn't
        // send the media data in the packet, so strip it off.
        if (htcb->fRTPMetaInfoFieldArray[RTPMetaInfoPacket::kMediaDataField] == RTPMetaInfoPacket::kFieldNotUsed)
            *length -= thePacketDataLen;
        else
        {
            // If we do have md, make sure to put the right length in the right place
            thePacketDataLen = htons(thePacketDataLen);
            COPY_WORD(endOfMetaInfo - 2, &thePacketDataLen);
        }
    }
    
    //
    // The packet has been generated.
    return err;
}

void QTHintTrack::WriteMetaInfoField(   RTPMetaInfoPacket::FieldIndex inFieldIndex,
                                        RTPMetaInfoPacket::FieldID inFieldID,
                                        void* inFieldData, UInt32 inFieldLen, char** ioBuffer)
{
    if (inFieldID == RTPMetaInfoPacket::kUncompressed)
    {
        //
        // Write an uncompressed field
        RTPMetaInfoPacket::FieldName theName = htons(RTPMetaInfoPacket::GetFieldNameForIndex(inFieldIndex));
        COPY_WORD(*ioBuffer, &theName);
        (*ioBuffer)+=2;
        UInt16 theLen = htons((UInt16)inFieldLen);
        COPY_WORD(*ioBuffer, &theLen);
        (*ioBuffer)+=2;
    }
    else
    {
        //
        // Write a compressed field
        UInt8 theID = (UInt8)inFieldID;
        theID |= 0x80;
        COPY_BYTE(*ioBuffer, &theID); 
        (*ioBuffer)+=1;
        UInt8 theLenByte = (UInt8)inFieldLen;
        COPY_BYTE(*ioBuffer, &theLenByte);
        (*ioBuffer)+=1;
    }

    if (inFieldData != NULL)
    {
        ::memcpy(*ioBuffer, inFieldData, inFieldLen);
        (*ioBuffer)+=inFieldLen;
    }
}





// -------------------------------------
// Debugging functions
//
void QTHintTrack::DumpTrack(void)
{
    //
    // Dump the QTTrack class.
    QTTrack::DumpTrack();

    //
    // Dump the sub-atoms of this track.
    if( fHintInfoAtom != NULL )
        fHintInfoAtom->DumpAtom();
}
