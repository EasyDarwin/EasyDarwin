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
// QTAtom_stsc:
//   The 'stsc' QTAtom class.


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
#include "QTAtom_stsc.h"
#include "OSMemory.h"


// -------------------------------------
// Constants
//
const int       stscPos_VersionFlags        =  0;
const int       stscPos_NumEntries          =  4;
const int       stscPos_SampleTable         =  8;



// -------------------------------------
// Macros
//
#define DEBUG_PRINT(s) if(fDebug) qtss_printf s
#define DEEP_DEBUG_PRINT(s) if(fDeepDebug) qtss_printf s



// -------------------------------------
// Class state cookie
//
QTAtom_stsc_SampleTableControlBlock::QTAtom_stsc_SampleTableControlBlock(void)
{
    Reset();
}

QTAtom_stsc_SampleTableControlBlock::~QTAtom_stsc_SampleTableControlBlock(void)
{
}

void QTAtom_stsc_SampleTableControlBlock::Reset(void)
{
    fCurEntry = 0;
    fCurSample = 1;
    fLastFirstChunk = 1;
    fLastSamplesPerChunk = 1;
    fLastSampleDescription = 0;
    
    fLastFirstChunk_GetChunkFirstLastSample = 1;
    fLastSamplesPerChunk_GetChunkFirstLastSample = 1;
    fLastTotalSamples_GetChunkFirstLastSample = 0;  
    fCurEntry_GetChunkFirstLastSample = 0;
    chunkNumber_GetChunkFirstLastSample = 0;
    firstSample_GetChunkFirstLastSample = 0;
    lastSample_GetChunkFirstLastSample = 0;


    fCurEntry_SampleToChunkInfo = 0;
    fCurSample_SampleToChunkInfo = 1;
    fLastFirstChunk_SampleToChunkInfo = 1;
    fLastSamplesPerChunk_SampleToChunkInfo = 1;
    fLastSampleDescription_SampleToChunkInfo = 0;
    
    fFirstSampleNumber_SampleToChunkInfo = 0;
    fFirstSamplesPerChunk_SampleToChunkInfo = 0;
    fFirstChunkNumber_SampleToChunkInfo = 0;
    fFirstSampleDescriptionIndex_SampleToChunkInfo = 0;
    fFirstSampleOffsetInChunk_SampleToChunkInfo = 0;
    
    fGetSampleInfo_SampleNumber = 0;
    fGetSampleInfo_Length = 0;
    fGetSampleInfo_SampleDescriptionIndex = 0;
    fGetSampleInfo_Offset = 0;
    fGetSampleInfo_LastChunk = 0;
    fGetSampleInfo_LastChunkOffset = 0;
    
    fGetSizeOfSamplesInChunk_chunkNumber = 0;
    fGetSizeOfSamplesInChunk_firstSample = 0;
    fGetSizeOfSamplesInChunk_lastSample = 0;
    fGetSizeOfSamplesInChunk_size = 0;
    

}



// -------------------------------------
// Constructors and destructors
//
QTAtom_stsc::QTAtom_stsc(QTFile * File, QTFile::AtomTOCEntry * TOCEntry, Bool16 Debug, Bool16 DeepDebug)
    : QTAtom(File, TOCEntry, Debug, DeepDebug),
      fNumEntries(0), fSampleToChunkTable(NULL), fTableSize(0)
{
}

QTAtom_stsc::~QTAtom_stsc(void)
{
    //
    // Free our variables.
#if MMAP_TABLES
    if( fSampleToChunkTable != NULL )
        this->UnMap(fSampleToChunkTable, fTableSize);
#else
    if( fSampleToChunkTable != NULL )
        delete[] fSampleToChunkTable;
#endif

}


// -------------------------------------
// Initialization functions
//
Bool16 QTAtom_stsc::Initialize(void)
{
    // Temporary vars
    UInt32      tempInt32;


    //
    // Parse this atom's fields.
    ReadInt32(stscPos_VersionFlags, &tempInt32);
    fVersion = (UInt8)((tempInt32 >> 24) & 0x000000ff);
    fFlags = tempInt32 & 0x00ffffff;

    ReadInt32(stscPos_NumEntries, &fNumEntries);

    //
    // Validate the size of the sample table.
    if( (UInt32)(fNumEntries * 12) != (fTOCEntry.AtomDataLength - 8) )
        return false;

    //
    // Read in the sample-to-chunk table.
#if MMAP_TABLES
    fTableSize = fNumEntries * 12;
    fSampleToChunkTable = this->MemMap(stscPos_SampleTable, fTableSize);
    if( fSampleToChunkTable == NULL )
        return false;
#else
    fSampleToChunkTable = NEW char[fNumEntries * 12];
    if( fSampleToChunkTable == NULL )
        return false;
    ReadBytes(stscPos_SampleTable, fSampleToChunkTable, fNumEntries * 12);
#endif
    
    //
    // This atom has been successfully read in.
    return true;
}



// -------------------------------------
// Accessors
//

Bool16 QTAtom_stsc::GetChunkFirstLastSample(UInt32 chunkNumber, UInt32 *firstSample, UInt32 *lastSample,  QTAtom_stsc_SampleTableControlBlock *STCB)
{
    // Temporary state var
    QTAtom_stsc_SampleTableControlBlock *tempSTCB = NULL;
    

    // General vars
    UInt32      prevFirstChunk = 0, thisFirstChunk = 0;
    UInt32      totalSamples = 0;
    UInt32      numChunks = 0;
    UInt32      numSamplesInChunks = 0;
    UInt32      prevSamplesPerChunk = 0;
    UInt32      samplesPerChunk = 0;
    
//  qtss_printf("GetChunkFirstLastSample chunk = %d STCB->chunkNumber_GetChunkFirstLastSample= %d \n",chunkNumber,STCB->chunkNumber_GetChunkFirstLastSample);
     

    if( STCB == NULL )
    {
//      qtss_printf(" QTAtom_stsc::GetChunkFirstLastSample (NULL == STCB) \n");
        tempSTCB = NEW QTAtom_stsc_SampleTableControlBlock;
        STCB = tempSTCB;

    }    
        
    if ( (STCB->chunkNumber_GetChunkFirstLastSample == chunkNumber) && (STCB->lastSample_GetChunkFirstLastSample > 0) )
    {
//      qtss_printf("GetChunkFirstLastSample cache hit chunk = %d\n",chunkNumber);
        if (firstSample) *firstSample = STCB->firstSample_GetChunkFirstLastSample;
        if (lastSample) *lastSample = STCB->lastSample_GetChunkFirstLastSample;
        goto GetChunkFirstLastSample_Done;
        
    }

    if (STCB->fCurEntry_GetChunkFirstLastSample > chunkNumber)
    {
//      qtss_printf(" QTAtom_stsc::GetChunkFirstLastSample missed Cache Loop \n");
        STCB->fLastFirstChunk_GetChunkFirstLastSample = 1;
        STCB->fLastSamplesPerChunk_GetChunkFirstLastSample = 1;
        STCB->fLastTotalSamples_GetChunkFirstLastSample = 0;    
        STCB->fCurEntry_GetChunkFirstLastSample = 0;
        STCB->chunkNumber_GetChunkFirstLastSample = 0;
        STCB->firstSample_GetChunkFirstLastSample = 0;
        STCB->lastSample_GetChunkFirstLastSample = 0;
    }   
     
    if (STCB->fCurEntry_GetChunkFirstLastSample > 0)
    {
//      qtss_printf("GetChunkFirstLastSample cached loop chunk start = %d look for chunk = %"_S32BITARG_"\n",STCB->fLastFirstChunk_GetChunkFirstLastSample,chunkNumber);
        samplesPerChunk = STCB->fLastSamplesPerChunk_GetChunkFirstLastSample;
        totalSamples = STCB->fLastTotalSamples_GetChunkFirstLastSample;
        thisFirstChunk = STCB->fLastFirstChunk_GetChunkFirstLastSample;
    }

    
    //
    // Linearly search through the sample table until we find the chunk
    // which contains the given sample.
    
    if (fNumEntries == 1)
    {   
        memcpy(&samplesPerChunk, fSampleToChunkTable + (STCB->fCurEntry_GetChunkFirstLastSample * 12) + 4, 4);
        samplesPerChunk = ntohl(samplesPerChunk);
    
        prevSamplesPerChunk = ((chunkNumber -1 ) * samplesPerChunk);
        totalSamples = chunkNumber * samplesPerChunk;
    }
    else
    for( ; STCB->fCurEntry_GetChunkFirstLastSample < fNumEntries; STCB->fCurEntry_GetChunkFirstLastSample++ ) 
    {
        // Copy this entry's fields.
        prevSamplesPerChunk = samplesPerChunk;
        prevFirstChunk = thisFirstChunk;

        memcpy(&thisFirstChunk, fSampleToChunkTable + (STCB->fCurEntry_GetChunkFirstLastSample * 12) + 0, 4);
        thisFirstChunk = ntohl(thisFirstChunk);
        memcpy(&samplesPerChunk, fSampleToChunkTable + (STCB->fCurEntry_GetChunkFirstLastSample * 12) + 4, 4);
        samplesPerChunk = ntohl(samplesPerChunk);
        
        if (prevSamplesPerChunk == 0)  
            prevSamplesPerChunk = samplesPerChunk;
        
        if ( chunkNumber < thisFirstChunk ) // found chunk in group
        {   
//          qtss_printf("found chunk in group numEntries = %"_S32BITARG_" this chunk = %"_S32BITARG_" \n",fNumEntries, chunkNumber);
            numSamplesInChunks = (chunkNumber - prevFirstChunk) * prevSamplesPerChunk;
            totalSamples += numSamplesInChunks;
            prevSamplesPerChunk = totalSamples - prevSamplesPerChunk;
            break;
        }
        
        if ( chunkNumber == thisFirstChunk ) // found chunk 
        {
            numSamplesInChunks = samplesPerChunk;
            totalSamples += numSamplesInChunks;
            prevSamplesPerChunk = totalSamples - samplesPerChunk;
            break;
        }
        
         
        numChunks = chunkNumber - prevFirstChunk;
        numSamplesInChunks = numChunks * samplesPerChunk;
        totalSamples += numSamplesInChunks;

        //
        // We have yet to find the sample; update our CurSample counter
        // and move on.
        STCB->fLastFirstChunk_GetChunkFirstLastSample = thisFirstChunk;
        STCB->fLastSamplesPerChunk_GetChunkFirstLastSample = samplesPerChunk;
        STCB->fLastTotalSamples_GetChunkFirstLastSample = totalSamples;

    }
    
    if (firstSample) *firstSample = prevSamplesPerChunk + 1;
    if (lastSample) *lastSample = totalSamples;
    
//  qtss_printf("Get Chunk %"_S32BITARG_" First %"_S32BITARG_" Last %"_S32BITARG_" prevSamplesPerChunk %"_S32BITARG_" samplesPerChunk %"_S32BITARG_"\n",chunkNumber,*firstSample,*lastSample,prevSamplesPerChunk,samplesPerChunk);

    STCB->chunkNumber_GetChunkFirstLastSample = chunkNumber;
    STCB->firstSample_GetChunkFirstLastSample =  prevSamplesPerChunk + 1;
    STCB->lastSample_GetChunkFirstLastSample = totalSamples;
    
GetChunkFirstLastSample_Done:
    delete tempSTCB;
    return true;
}


UInt32 QTAtom_stsc::GetChunkFirstSample(UInt32 chunkNumber)
{
    // Temporary state var
    QTAtom_stsc_SampleTableControlBlock localSTCB;
    QTAtom_stsc_SampleTableControlBlock *STCB = &localSTCB;
    
    // General vars
    UInt32      prevFirstChunk = 0, thisFirstChunk = 0, sampleDescription = 0;
    UInt32      totalSamples = 1;
    UInt32      numChunks = 0;
    UInt32      numSamplesInChunks = 0;
    UInt32      thisChunk = 0;
    UInt32      prevSamplesPerChunk = 0;
    UInt32      samplesPerChunk = 0;
    
    
    //
    // Linearly search through the sample table until we find the chunk
    // which contains the given sample.
    for( ; STCB->fCurEntry < fNumEntries; STCB->fCurEntry++ ) 
    {
        // Copy this entry's fields.
        prevSamplesPerChunk = samplesPerChunk;
        prevFirstChunk = thisFirstChunk;

        memcpy(&thisFirstChunk, fSampleToChunkTable + (STCB->fCurEntry * 12) + 0, 4);
        thisFirstChunk = ntohl(thisFirstChunk);
        memcpy(&samplesPerChunk, fSampleToChunkTable + (STCB->fCurEntry * 12) + 4, 4);
        samplesPerChunk = ntohl(samplesPerChunk);
        memcpy(&sampleDescription, fSampleToChunkTable + (STCB->fCurEntry * 12) + 8, 4);
        sampleDescription = ntohl(sampleDescription);
        
        thisChunk = thisFirstChunk;
        numChunks = thisFirstChunk - prevFirstChunk;
        
        if ( chunkNumber <= thisFirstChunk ) // found chunk in group
        {
            numChunks = chunkNumber - prevFirstChunk;
            numSamplesInChunks = numChunks * prevSamplesPerChunk;
            totalSamples += numSamplesInChunks;
            break;
        }
        
        
        numChunks = thisFirstChunk - prevFirstChunk;
        numSamplesInChunks = numChunks * prevSamplesPerChunk;
        totalSamples += numSamplesInChunks;

        //
        // We have yet to find the sample; update our CurSample counter
        // and move on.
        STCB->fLastFirstChunk = thisFirstChunk;
        STCB->fLastSampleDescription = sampleDescription;
        STCB->fLastSamplesPerChunk = samplesPerChunk;
    }
    

    return totalSamples;
}


//UInt32 gstscCacheCount = 0;
//UInt32 gstscCallCount = 0;


Bool16 QTAtom_stsc::SampleToChunkInfo(UInt32 SampleNumber,  UInt32 *samplesPerChunk,  UInt32 *ChunkNumber, UInt32 *SampleDescriptionIndex, UInt32 *SampleOffsetInChunk, QTAtom_stsc_SampleTableControlBlock * STCB) 
{
    // Temporary state var
    QTAtom_stsc_SampleTableControlBlock *tempSTCB = NULL;
    
    // General vars
    UInt32      NewCurSample;
    UInt32      FirstChunk = 0, SamplesPerChunk = 0, SampleDescription = 0;
    
    Bool16      missedCache = false;

    if (STCB == NULL ) 
    {
//      qtss_printf(" QTAtom_stsc::SampleToChunkInfo (NULL == STCB) \n");
        tempSTCB = NEW QTAtom_stsc_SampleTableControlBlock;
        STCB = tempSTCB;
    }
    
    
    UInt32  aChunkNumber = 0;
    UInt32  aSampleDescriptionIndex = 0;
    UInt32  aSampleOffsetInChunk = 0;
    UInt32  aSamplesPerChunk = 0;

/*
    gstscCallCount ++;
    if (gstscCallCount == 1000)
    {
        qtss_printf("QTAtom_stsc::SampleToChunkInfo #calls = %"_S32BITARG_", cache hits = %"_S32BITARG_" \n",gstscCallCount, gstscCacheCount);
        gstscCacheCount = 0;
        gstscCallCount = 0;

    }
*/
    
    //
    // Use a temporary STCB if we weren't given one.  We cannot use a default
    // STCB as we would have no way to know when we are seeking around in the
    // movie.

//  qtss_printf("------ QTAtom_stsc::SampleToChunkInfo SampleNumber = %"_S32BITARG_"\n",SampleNumber);

    if (STCB->fFirstSampleNumber_SampleToChunkInfo == SampleNumber)
    {
//      qtss_printf("------ QTAtom_stsc::SampleToChunkInfo cache hit SampleNumber = %"_S32BITARG_"\n",SampleNumber);

//      gstscCacheCount ++;
        aChunkNumber = STCB->fFirstChunkNumber_SampleToChunkInfo;
        aSampleDescriptionIndex = STCB->fFirstSampleDescriptionIndex_SampleToChunkInfo;
        aSamplesPerChunk = STCB->fFirstSamplesPerChunk_SampleToChunkInfo;
        aSampleOffsetInChunk = STCB->fFirstSampleOffsetInChunk_SampleToChunkInfo;

        if( ChunkNumber != NULL )
            *ChunkNumber = aChunkNumber;
        if( SampleDescriptionIndex != NULL )
            *SampleDescriptionIndex = aSampleDescriptionIndex;
        if( SampleOffsetInChunk != NULL )
            *SampleOffsetInChunk = aSampleOffsetInChunk;
        if (NULL != samplesPerChunk) 
             *samplesPerChunk = aSamplesPerChunk;

        goto done;
    }
//  qtss_printf("QTAtom_stsc::SampleToChunkInfo missed cache SampleNumber = %"_S32BITARG_"\n",SampleNumber);

    //
    // Assume that this sample came out of the last chunk.
    aChunkNumber = STCB->fLastFirstChunk_SampleToChunkInfo + ((SampleNumber - STCB->fCurSample_SampleToChunkInfo) / STCB->fLastSamplesPerChunk_SampleToChunkInfo) ;
    aSampleDescriptionIndex = STCB->fLastSampleDescription_SampleToChunkInfo;
    aSampleOffsetInChunk = SampleNumber - (STCB->fCurSample_SampleToChunkInfo + ((aChunkNumber - STCB->fLastFirstChunk_SampleToChunkInfo) * STCB->fLastSamplesPerChunk_SampleToChunkInfo));
    aSamplesPerChunk = STCB->fLastSamplesPerChunk_SampleToChunkInfo;
    
    if( ChunkNumber != NULL )
        *ChunkNumber = aChunkNumber;
    if( SampleDescriptionIndex != NULL )
        *SampleDescriptionIndex = aSampleDescriptionIndex;
    if( SampleOffsetInChunk != NULL )
        *SampleOffsetInChunk = aSampleOffsetInChunk;
    if (NULL != samplesPerChunk) 
         *samplesPerChunk = aSamplesPerChunk;
         
    //
    // Linear search through the sample table until we find the chunk
    // which contains the given sample.

    if (STCB->fCurSample_SampleToChunkInfo > SampleNumber) // we missed the cache start over
    {   
        missedCache = true;
//      qtss_printf("missed loop Cache!! STCB = %"_S32BITARG_" STCB->fCurSample_SampleToChunkInfo = %"_S32BITARG_"  > SampleNumber = %"_S32BITARG_" \n",STCB, STCB->fCurSample_SampleToChunkInfo, SampleNumber);
        STCB->fCurEntry_SampleToChunkInfo = 0;
        STCB->fCurSample_SampleToChunkInfo = 1;
        STCB->fLastFirstChunk_SampleToChunkInfo = 1;
        STCB->fLastSamplesPerChunk_SampleToChunkInfo = 1;
        STCB->fLastSampleDescription_SampleToChunkInfo = 0;
    }



    for(; STCB->fCurEntry_SampleToChunkInfo < fNumEntries; STCB->fCurEntry_SampleToChunkInfo++ ) {
        //
        // Copy this entry's fields.
        memcpy(&FirstChunk, fSampleToChunkTable + (STCB->fCurEntry_SampleToChunkInfo * 12) + 0, 4);
        FirstChunk = ntohl(FirstChunk);
        memcpy(&SamplesPerChunk, fSampleToChunkTable + (STCB->fCurEntry_SampleToChunkInfo * 12) + 4, 4);
        SamplesPerChunk = ntohl(SamplesPerChunk);
        memcpy(&SampleDescription, fSampleToChunkTable + (STCB->fCurEntry_SampleToChunkInfo * 12) + 8, 4);
        SampleDescription = ntohl(SampleDescription);
        
        //
        // Check to see if the sample was actually in the last chunk and
        // return if it was.
        NewCurSample = STCB->fCurSample_SampleToChunkInfo +  (FirstChunk - STCB->fLastFirstChunk_SampleToChunkInfo) * STCB->fLastSamplesPerChunk_SampleToChunkInfo ;
        if( SampleNumber < NewCurSample )
        {
    
            if( ChunkNumber != NULL )
                *ChunkNumber = aChunkNumber;
            if( SampleDescriptionIndex != NULL )
                *SampleDescriptionIndex = aSampleDescriptionIndex;
            if( SampleOffsetInChunk != NULL )
                *SampleOffsetInChunk = aSampleOffsetInChunk;
            if (NULL != samplesPerChunk) 
                 *samplesPerChunk = aSamplesPerChunk;
        
            if (SampleNumber == 1 || missedCache)
            {
//                  qtss_printf("QTAtom_stsc::SampleToChunkInfo (SampleNumber == %"_S32BITARG_") \n",SampleNumber);
                    STCB->fFirstChunkNumber_SampleToChunkInfo = aChunkNumber;
                    STCB->fFirstSampleDescriptionIndex_SampleToChunkInfo = aSampleDescriptionIndex;
                    STCB->fFirstSamplesPerChunk_SampleToChunkInfo = aSamplesPerChunk;
                    STCB->fFirstSampleOffsetInChunk_SampleToChunkInfo = aSampleOffsetInChunk;
                    STCB->fFirstSampleNumber_SampleToChunkInfo = SampleNumber;
            }
//          qtss_printf("GetSample Info in for loop returning offset %"_S32BITARG_" \n",aSampleOffsetInChunk);
            goto done;
        }   

        STCB->fCurSample_SampleToChunkInfo = NewCurSample;
        
        //
        // Assume that the sample will be in this chunk.
        

        aChunkNumber = FirstChunk + ((SampleNumber - STCB->fCurSample_SampleToChunkInfo) / SamplesPerChunk) ;
        aSampleDescriptionIndex = SampleDescription;
        aSampleOffsetInChunk = SampleNumber - (STCB->fCurSample_SampleToChunkInfo + ((aChunkNumber - FirstChunk) * SamplesPerChunk));
        aSamplesPerChunk = SamplesPerChunk;  
        
        //
        // We have yet to find the sample; update our CurSample counter
        // and move on.
        

        STCB->fLastFirstChunk_SampleToChunkInfo = FirstChunk;
        STCB->fLastSampleDescription_SampleToChunkInfo = SampleDescription;
        STCB->fLastSamplesPerChunk_SampleToChunkInfo = SamplesPerChunk;


    }

//  qtss_printf("GetSample Info fall out of loop returning offset%"_S32BITARG_" \n",aSampleOffsetInChunk);
    //
    // Falling out of the loop means that the sample is in the last chunk of
    // the table.
    
    if( ChunkNumber != NULL )
        *ChunkNumber = aChunkNumber;
    if( SampleDescriptionIndex != NULL )
        *SampleDescriptionIndex = aSampleDescriptionIndex;
    if( SampleOffsetInChunk != NULL )
        *SampleOffsetInChunk = aSampleOffsetInChunk;
    if (NULL != samplesPerChunk) 
         *samplesPerChunk = aSamplesPerChunk;

    if (SampleNumber == 1 || missedCache)
    {
        STCB->fFirstChunkNumber_SampleToChunkInfo = aChunkNumber;
        STCB->fFirstSampleDescriptionIndex_SampleToChunkInfo = aSampleDescriptionIndex;
        STCB->fFirstSamplesPerChunk_SampleToChunkInfo = aSamplesPerChunk;
        STCB->fFirstSampleOffsetInChunk_SampleToChunkInfo = aSampleOffsetInChunk;
        STCB->fFirstSampleNumber_SampleToChunkInfo = SampleNumber;
    }

    
done:
    delete tempSTCB;
    return true;
}



// -------------------------------------
// Debugging functions
//
void QTAtom_stsc::DumpAtom(void)
{
    DEBUG_PRINT(("QTAtom_stsc::DumpAtom - Dumping atom.\n"));
    DEBUG_PRINT(("QTAtom_stsc::DumpAtom - ..Number of STC entries: %"_S32BITARG_"\n", fNumEntries));
}

void QTAtom_stsc::DumpTable(void)
{
    // General vars
    UInt32      FirstChunk = 0, SamplesPerChunk = 0, SampleDescription = 0;


    //
    // Print out a header.
    qtss_printf("-- Sample-to-Chunk table -------------------------------------------------------\n");
    qtss_printf("\n");
    qtss_printf("  Sample Num   FirstChunk  Samples/Chunk  Sample Description\n");
    qtss_printf("  ----------   ----------  -------------  ------------------\n");
    
    //
    // Print the table.
    for( UInt32 CurEntry = 0; CurEntry < fNumEntries; CurEntry++ ) {
        //
        // Copy this entry's fields.
        memcpy(&FirstChunk, fSampleToChunkTable + (CurEntry * 12) + 0, 4);
        FirstChunk = ntohl(FirstChunk);
        memcpy(&SamplesPerChunk, fSampleToChunkTable + (CurEntry * 12) + 4, 4);
        SamplesPerChunk = ntohl(SamplesPerChunk);
        memcpy(&SampleDescription, fSampleToChunkTable + (CurEntry * 12) + 8, 4);
        SampleDescription = ntohl(SampleDescription);
        
        //
        // Print out a listing.
        qtss_printf("  %10"_U32BITARG_" : %10"_U32BITARG_"   %10"_U32BITARG_"        %10"_U32BITARG_"\n",
               CurEntry+1, FirstChunk, SamplesPerChunk, SampleDescription);
    }
}
