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
 
 
#include <stdlib.h>
#include "SafeStdLib.h" 
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef __MacOSX__
#include "getopt.h"
#include <unistd.h>
#endif


 #include "RTPFileDefs.h"
 #include "QTRTPFile.h"
 #include "QTHintTrack.h"
 #include "OSHeaders.h"


 class QTRTPGenFile : public QTRTPFile
 {
    public:
    
        QTRTPGenFile() : QTRTPFile() {}
        virtual ~QTRTPGenFile() {}
        
        // Accessors
        UInt32      GetNumHintTracks() { return fNumHintTracks; }
        RTPTrackListEntry* GetFirstTrack() { return fFirstTrack; }
 };
 
 
UInt8* WriteTempFile(QTRTPFile* inQTRTPFile, int inTempFile, UInt32* outNumBlocks);



int main(int argc, char *argv[])
{

    if (argc <= 1)
    {
        qtss_printf("Usage: rtpfilegen *hintedqtfilename*\n");
        exit(0);
    }
    
    char* theFileName = argv[1];
    
    QTRTPFile::Initialize();
    
    // Init our qt file
    QTRTPGenFile theFile;
    QTRTPFile::ErrorCode initErr = theFile.Initialize(theFileName);
    switch (initErr)
    {
        case QTRTPFile::errFileNotFound:
            qtss_printf("Movie file not found\n"); exit(0);
        case QTRTPFile::errInvalidQuickTimeFile:
            qtss_printf("File is not a hinted quicktime file\n"); exit(0);
        case QTRTPFile::errNoHintTracks:
            qtss_printf("File has no hint tracks\n"); exit(0);
        case QTRTPFile::errInternalError:
            qtss_printf("Internal error\n"); exit(0);
        
        case QTRTPFile::errNoError:
        case QTRTPFile::errTrackIDNotFound:
        case QTRTPFile::errCallAgain:
            //noops
            break;
    }

    // Open our output file
    char* outputFileName = new char[::strlen(theFileName) + 5];
    ::strcpy(outputFileName, theFileName);
    ::strcat(outputFileName, ".rtp");
    int theOutFile = open(outputFileName, O_WRONLY | O_CREAT | O_TRUNC);
    if (theOutFile == -1)
    {   qtss_printf("Failed to open output file at %s.\n", outputFileName); exit(0); }
        
    // Open a temp file
    char* tempFileName = new char[::strlen(theFileName) + 6];
    ::strcpy(tempFileName, theFileName);
    ::strcat(tempFileName, ".temp");
    int theTempFile = open(tempFileName, O_WRONLY | O_CREAT | O_TRUNC);
    if (theTempFile == -1)
    {   qtss_printf("Failed to open temp file at %s.\n", tempFileName); exit(0); }
    
    // Setup all our tracks
    UInt32 theMaxTrackID = 0; // These variables are needed to write the file header
    UInt32 theNumTracks = 0;
    QTRTPFile::RTPTrackListEntry* curTrack = theFile.GetFirstTrack();
    while (curTrack != NULL)
    {
        if (theMaxTrackID < curTrack->TrackID)
            theMaxTrackID = curTrack->TrackID;
        
        theNumTracks++;
        (void)theFile.AddTrack(curTrack->TrackID);
        // Make the cookie be the track ID, so we know what the track ID is when 
        // we are retreiving packets
        theFile.SetTrackCookies(curTrack->TrackID, (void*) NULL, curTrack->TrackID);
        
        curTrack = curTrack->NextTrack;
    }
    
    // Seek to time 0
    (void)theFile.Seek(0);
    
    // Write status
    qtss_printf("Generating RTP packets to temp file at %s...\n", tempFileName);
    
    // Write out all the packets to the temp file
    UInt32 theNumBlocks = 0;
    UInt8* theBlockArray = WriteTempFile(&theFile, theTempFile, &theNumBlocks);
    ::close(theTempFile);
    
    // Write status
    qtss_printf("Generating RTP file at %s. Copying packets...\n", outputFileName);

    // Get SDP
    int theSDPLen = 0;
    char* theSDPFile = theFile.GetSDPFile(&theSDPLen);
    
    // Write file header
    RTPFileHeader theHeader;
    theHeader.fSDPLen = theSDPLen;
    theHeader.fNumTracks = theFile.GetNumHintTracks();
    theHeader.fMaxTrackID = theMaxTrackID;
    theHeader.fBlockMapSize = theNumBlocks;
    theHeader.fDataStartPos = theNumBlocks + theSDPLen + sizeof(RTPFileHeader) +
                                        (sizeof(RTPFileTrackInfo) * theNumTracks);
    theHeader.fMovieDuration = theFile.GetMovieDuration();
    
    int theErr = ::write(theOutFile, &theHeader, sizeof(theHeader));
    if (theErr != sizeof(theHeader))
    {   qtss_printf("Write operation failed on output file\n"); exit(0); }
    
    // Write the SDP data
    theErr = ::write(theOutFile, theSDPFile, theSDPLen);
    if (theErr != theSDPLen)
    {   qtss_printf("Write operation failed on output file\n"); exit(0); }
        
    // Write track info
    curTrack = theFile.GetFirstTrack();
    while (curTrack != NULL)
    {
        RTPFileTrackInfo theTrackInfo;
        theTrackInfo.fID = curTrack->TrackID;
        theTrackInfo.fTimescale = curTrack->HintTrack->GetRTPTimescale();
        theTrackInfo.fBytesInTrack = curTrack->HintTrack->GetTotalRTPBytes();
        theTrackInfo.fDuration = curTrack->HintTrack->GetDurationInSeconds();

        theErr = ::write(theOutFile, &theTrackInfo, sizeof(theTrackInfo));
        if (theErr != sizeof(theTrackInfo))
        {   qtss_printf("Write operation failed on output file\n"); exit(0); }

        curTrack = curTrack->NextTrack;
    }
    
    // Write the block map
    UInt32 numWritten = ::write(theOutFile, theBlockArray, theNumBlocks);
    if (numWritten != theNumBlocks)
    {   qtss_printf("Write operation failed on output file\n"); exit(0); }
    
    //Copy the temp file into the out file
    theTempFile = open(tempFileName, O_RDONLY);
    if (theTempFile == -1)
    {   qtss_printf("Failed to open temp file for reading at %s.\n", tempFileName); exit(0); }
    
    while (true)
    {
        char copyBuffer[kBlockSize];
    
        // Read a block out of the temp file
        theErr = ::read(theTempFile, &copyBuffer[0], kBlockSize);
        if (theErr <= 0)
            break;
            
        int lenWritten = ::write(theOutFile, &copyBuffer[0], theErr);
        if (theErr != lenWritten)
        {   qtss_printf("Write operation failed on output file\n"); exit(0); }
    }
    
    qtss_printf("RTP file generation complete\n");
    
    // Delete the temp file
    (void)::unlink(tempFileName);
    
    // close the output file
    ::close(theOutFile);
    
    // We're done!
}


// Returns the complete block map
UInt8* WriteTempFile(QTRTPFile* inQTRTPFile, int inTempFile, UInt32* outNumBlocks)
{
    int theErr = -1;
    UInt32 theNumPackets = 0;
    
    // Write all packets to the temp file. Pad out blocks appropriately
    UInt32 offsetInBlock = 0;
    
    // As we write out packets, build our block map
    UInt8* theBlockMap = new UInt8[1024];
    UInt32 curBlockMapSize = 1024;
    UInt32 curBlockMapIndex = 0;
    Float64 theLastBlockTime = 0;
    
    while (true)
    {
        RTPFilePacket thePacketHeader;
        char* thePacketData = NULL;
        int thePacketLength = 0;
        
        thePacketHeader.fTransmitTime = inQTRTPFile->GetNextPacket(&thePacketData, &thePacketLength);
        if (thePacketData == NULL)
            // We're done with all tracks
            break;
                    
        thePacketHeader.fPacketLength = thePacketLength;
        thePacketHeader.fTrackID = inQTRTPFile->GetLastPacketTrack()->Cookie2;

        // Make sure there is enough room in the current block for this
        // packet plus a header for the next packet. If not, move onto the next block
        if ((offsetInBlock + thePacketHeader.fPacketLength + (2 * sizeof(RTPFilePacket))) > kBlockSize)
        {
            // Write out a pad packet
            RTPFilePacket pad;
            pad.fTrackID |= kPaddingBit;
            
            theErr = ::write(inTempFile, &pad, sizeof(pad));
            if (theErr < (int) sizeof(pad))
            {   qtss_printf("Write operation failed on temp file\n"); exit(0); }
                
            offsetInBlock += sizeof(pad);
            SInt32 spaceRemaining = kBlockSize - offsetInBlock;
            Assert(spaceRemaining >= 0);
            
            // Fill out the rest of this block with crap
            char* dumpBuf = new char[spaceRemaining];
            ::memset(dumpBuf, 0xFF, spaceRemaining);//Fill with FFs so we can debug easier
            theErr = ::write(inTempFile, dumpBuf, spaceRemaining);
            if (theErr < spaceRemaining)
            {   qtss_printf("Write operation failed on temp file\n"); exit(0); }
                
            delete [] dumpBuf;
            
            offsetInBlock = 0;
        }
        
        // If this is the first packet in a block, we should mark the time in the
        // Write the packet header and the packet data
        if (offsetInBlock == 0)
        {
            // What is the offset between the first packet in the last block and this block?
            Float64 theOffset = thePacketHeader.fTransmitTime - theLastBlockTime;
            if (theOffset < 0)
                theOffset = 0; //This may happen with the stupid negative times
            theLastBlockTime = thePacketHeader.fTransmitTime;
            if (theLastBlockTime < 0)
                theLastBlockTime = 0; //This may happen with the stupid negative times
                
            Assert(theOffset < 255);
            theBlockMap[curBlockMapIndex] = (UInt8)theOffset;

            // We may need to reallocate the block array
            curBlockMapIndex++;
            if (curBlockMapIndex == curBlockMapSize)
            {
                UInt8* newBlockArray = new UInt8[curBlockMapSize * 2];
                ::memcpy(newBlockArray, theBlockMap, curBlockMapSize);
                curBlockMapSize *= 2;
                
                delete [] theBlockMap;
                theBlockMap = newBlockArray;
            }
        }

        theNumPackets++;
        theErr = ::write(inTempFile, &thePacketHeader, sizeof(thePacketHeader));
        if (theErr < (int) sizeof(thePacketHeader))
        {   qtss_printf("Write operation failed on temp file\n"); exit(0); }

        theErr = ::write(inTempFile, thePacketData, thePacketLength);
        if (theErr < thePacketLength)
        {   qtss_printf("Write operation failed on temp file\n"); exit(0); }
        
        // update our block offset
        offsetInBlock += thePacketLength + sizeof(thePacketHeader);
    }
    
    qtss_printf("Finished writing packets. Wrote %"_U32BITARG_" packets to temp file.\n", theNumPackets);
    *outNumBlocks = curBlockMapIndex-1;
    return theBlockMap;
}
