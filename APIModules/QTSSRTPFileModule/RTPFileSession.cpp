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
/*
    File:       RTPFileSession.cpp

    Contains:   
                    
    
    
*/

#define RTPFILESESSIONDEBUG 0

#include "RTPFileSession.h"
#include "OSMemory.h"

static OSRefTable sOpenFileMap;


RTPFileSession::RTPFileSession()
:   fFileSource(NULL),
    fFileLength(0),
    fCurrentPosition(0),
    fFile(NULL),
    fTrackInfo(NULL),
    fNumTracksEnabled(0),
    fReadBuffer(NULL),
    fReadBufferOffset(0),
    fDataBuffer(NULL),
    fDataBufferSize(0),
    fDataBufferLen(0),
    fCurrentPacket(NULL),
    fAddedTracksRTPBytes(0)
{}

RTPFileSession::~RTPFileSession()
{
    // Check to see if we should destroy this file
    OSMutexLocker locker (sOpenFileMap.GetMutex());

#if RTPFILESESSIONDEBUG
    qtss_printf("Dropping refcount on file\n");
#endif
    if (fFile == NULL)
        return;
        
    sOpenFileMap.Release(fFile->GetRef());
    if (fFile->GetRef()->GetRefCount() == 0)
    {
#if RTPFILESESSIONDEBUG
        qtss_printf("Refcount dropped to 0. Deleting file\n");
#endif
        sOpenFileMap.UnRegister(fFile->GetRef());
        delete fFile;
    }   

    // Delete our data buffer
    delete [] fDataBuffer;
    delete [] fTrackInfo;
}

RTPFileSession::ErrorCode   RTPFileSession::Initialize(StrPtrLen& inFilePath, Float32 inBufferSeconds)
{
    Assert(fFile == NULL);
    
    // Check to see if this file is already open
    OSMutexLocker locker(sOpenFileMap.GetMutex());
    OSRef* theFileRef = sOpenFileMap.Resolve((StrPtrLen*)&inFilePath);

    if (theFileRef == NULL)
    {
        //qtss_printf("Didn't find file in map. Creating new one\n");
        fFile = NEW RTPFile();
        ErrorCode theErr = fFile->Initialize(inFilePath);
        if (theErr != errNoError)
        {
            delete fFile;
            fFile = NULL;
            return theErr;
        }
        
        OS_Error osErr = sOpenFileMap.Register(fFile->GetRef());
        Assert(osErr == OS_NoErr);

        //unless we do this, the refcount won't increment (and we'll delete the session prematurely
        OSRef* debug = sOpenFileMap.Resolve((StrPtrLen*)&inFilePath);
        Assert(debug == fFile->GetRef());
    }   
    else
    {
        //qtss_printf("Found file. Refcounting.\n");
        fFile = (RTPFile*)theFileRef->GetObject();
    }
    
    //Open the file no matter what
    //fFileSource.Set(inFilePath.Ptr);
    //Assert(fFileSource.GetLength() > 0);
    QTSS_Error theErr = QTSS_OpenFileObject(inFilePath.Ptr, 0, &fFileSource);
    Assert(theErr == QTSS_NoErr);
    
    //
    // Get the file length
    UInt32 theLen = sizeof(fFileLength);
    theErr = QTSS_GetValue(fFileSource, qtssFlObjLength, 0, &fFileLength, &theLen);
    Assert(theErr == QTSS_NoErr);
    Assert(theLen == sizeof(fFileLength));
    
    // Allocate our data buffer
    fDataBufferSize = this->PowerOf2Floor((UInt32)(inBufferSeconds * fFile->GetBytesPerSecond()));

    // Check to see if the size is out of range. If so, adjust it
    if (fDataBufferSize > kMaxDataBufferSize)
        fDataBufferSize = kMaxDataBufferSize;
    if (fDataBufferSize < kBlockSize)
        fDataBufferSize = kBlockSize;
        
    fReadBuffer = fDataBuffer = NEW UInt8[fDataBufferSize];
    
    // Allocate a buffer of TrackInfos
    fTrackInfo = NEW RTPFileSessionTrackInfo[fFile->GetMaxTrackNumber() + 1];
    ::memset(fTrackInfo, 0, fFile->GetMaxTrackNumber() * sizeof(RTPFileSessionTrackInfo));
    return errNoError;
}

RTPFileSession::ErrorCode RTPFileSession::AddTrack(UInt32 trackID)
{
    if (fFile->TrackExists(trackID))
    {
        if (!fTrackInfo[trackID].fEnabled)
        {
            fTrackInfo[trackID].fEnabled = true;
            fAddedTracksRTPBytes += fFile->GetTrackBytes(trackID);
            fNumTracksEnabled++;
        }
        else
            return errTrackAlreadyAdded;
    }
    else
        return errTrackDoesntExist;
    return errNoError;
}


RTPFileSession::ErrorCode   RTPFileSession::Seek(Float64 inTime)
{
    if ((inTime < 0) || (inTime > fFile->GetMovieDuration()))
        return errSeekToNonexistentTime;
    
    UInt64 theBlockLocation = fFile->GetBlockLocation(inTime);
    Assert(theBlockLocation >= fFile->fHeader.fDataStartPos);
    Assert(theBlockLocation < fFileLength);
    
    // Seek to the right file location.
    //fFileSource.Seek(theBlockLocation);
    QTSS_Error theErr = QTSS_Seek(fFileSource, theBlockLocation);
    Assert(theErr == QTSS_NoErr);
    fCurrentPosition = theBlockLocation;
    
    // Read the file data
    this->ReadAndAdvise();
    
    for (UInt32 x = 0; x <= fFile->GetMaxTrackNumber(); x++)
        fTrackInfo[x].fMarked = false;
    
    //
    // We need to find out what the first packet is for each enabled track.
    // So scan ahead until we find the very first packet we need to send.
    // At that point, "freeze" the current block in memory, and that position,
    // because that's the position we'll be starting from when GetNextPacket gets
    // called. In order to "freeze" we store lots of info about the current position
    // on the stack with the variables defined below.
    //
    // Keep on going until we find the first packets for all the enabled tracks,
    // and if that involves traversing multiple blocks, keep those blocks in a temporary
    // buffer, allowing us to easily go back to the start point when done.
    
    UInt8* theStartPos = NULL;
    UInt32 origDataBufferLen = 0;
    UInt64 currentFileOffset = 0;
    UInt32 tracksFound = 0;
    
    // Needed to call GetNextPacket
    UInt8* thePacketP = NULL;
    UInt32 thePacketLength = 0;
    void* theCookie = NULL;
    
    while (tracksFound < fNumTracksEnabled)
    {
        Float64 theTransmitTime = this->GetNextPacket(&thePacketP, &thePacketLength, &theCookie);
        if (thePacketP == NULL)
        {
            Assert(tracksFound > 0);
            break; // We're at the end of the file!
        }
        // Ignore < 0 timed packets
        RTPFilePacket* thePacket = (RTPFilePacket*)thePacketP;
        Assert((thePacket - 1)->fTransmitTime == theTransmitTime);
        if (theTransmitTime < 0)
            theTransmitTime = 0;

        UInt32 theTrackID = (thePacket - 1)->fTrackID;
        
        if ((theTransmitTime >= inTime) && (!fTrackInfo[theTrackID].fMarked) &&
            (fTrackInfo[theTrackID].fEnabled))
        {
            // This is the first packet for this track after our fCurrentPtr mark.
            // Record the first seq # and timestamp of the packet
            UInt16* theSeqNumPtr = (UInt16*)thePacketP;
            UInt32* theTimestampPtr = (UInt32*)thePacketP;
            
            fTrackInfo[theTrackID].fSeekSeqNumber = theSeqNumPtr[1];
            fTrackInfo[theTrackID].fSeekTimestamp = theTimestampPtr[1];
            fTrackInfo[theTrackID].fMarked = true;
            
            if (tracksFound == 0)
            {
                //
                // If this is the first packet that we're going to send (for all
                // streams), then mark the position, and make sure that if we
                // need to dump this buffer to find first packets for other tracks,
                // we'll be able to come back to this very place so we can start streaming.
                fReadBuffer = NULL;
                theStartPos = (UInt8*)(thePacket-1);
                origDataBufferLen = fDataBufferLen;
                currentFileOffset = fCurrentPosition;
            }

            tracksFound++;
        }
    }
    
    if (fReadBuffer != NULL)
    {
        // We had to skip ahead in the file. Restore everything to
        // the way it was when we found the first packet, so GetNextPacket
        // will work fine.
        delete [] fReadBuffer;
        fReadBuffer = fDataBuffer;
        Assert(origDataBufferLen > 0);
        fDataBufferLen = origDataBufferLen;
        Assert(currentFileOffset > 0);
        //fFileSource.Seek(currentFileOffset);
        theErr = QTSS_Seek(fFileSource, theBlockLocation);
        Assert(theErr == QTSS_NoErr);
    }
    
    // Start at the first packet we need to send.
    Assert(theStartPos != NULL);
    fCurrentPacket = theStartPos;
    
    return errNoError;
}

UInt16  RTPFileSession::GetNextTrackSequenceNumber(UInt32 inTrackID)
{
    Assert(inTrackID <= fFile->GetMaxTrackNumber());
    Assert(fTrackInfo[inTrackID].fMarked);
    return fTrackInfo[inTrackID].fSeekSeqNumber;
}

UInt32  RTPFileSession::GetSeekTimestamp(UInt32 inTrackID)
{
    Assert(inTrackID <= fFile->GetMaxTrackNumber());
    Assert(fTrackInfo[inTrackID].fMarked);
    return fTrackInfo[inTrackID].fSeekTimestamp;
}


Float64 RTPFileSession::GetNextPacket(UInt8** outPacket, UInt32* outPacketLength, void** outCookie)
{
    Bool16 isValidPacket = false;
    RTPFilePacket* thePacket = NULL;
    
    // Loop until we find a legal packet
    while (!isValidPacket)
    {
        // If we are between blocks, read the next block
        if (fCurrentPacket == NULL)
        {
            if (fCurrentPosition == fFileLength)
            {
            
#if RTPFILESESSIONDEBUG
                qtss_printf("RTPFileSession::GetNextPacket fCurrentPosition == fFileLength quit\n");
#endif
                *outPacket = NULL;
                return -1;
            }
            
            this->ReadAndAdvise();
        }
        Assert(fCurrentPacket != NULL);
        thePacket = (RTPFilePacket*)fCurrentPacket;
        
        if (thePacket->fTrackID & kPaddingBit)
        {
            // We hit a padding packet, move the fCurrentPacket pointer to the next block
            fReadBufferOffset += kBlockSize;
            fReadBufferOffset &= kBlockMask; //Rounds down to the nearest block size
            
#if RTPFILESESSIONDEBUG
            qtss_printf("Found a pad packet. Moving on\n");
#endif
            // Check to make sure we aren't at the end of the buffer
            if (fReadBufferOffset >= fDataBufferLen)
                fCurrentPacket = NULL;
            else
                fCurrentPacket = fDataBuffer + fReadBufferOffset;
        }
        else if (!fTrackInfo[thePacket->fTrackID].fEnabled)
        {
            // This is a valid packet, but track not enabled, so skip it
            Assert(thePacket->fTrackID <= fFile->GetMaxTrackNumber());
            this->SkipToNextPacket(thePacket);
        }
        else
            // This is a valid packet, and the track is enabled
            isValidPacket = true;
    }
    
    // We must have a valid packet here
    Assert(thePacket != NULL);
    Assert(thePacket->fTrackID <= fFile->GetMaxTrackNumber());
    
    // Set the return values
    *outPacket = (UInt8*)(thePacket + 1);
    *outPacketLength = thePacket->fPacketLength;
    *outCookie = fTrackInfo[thePacket->fTrackID].fCookie;
    
    // Set the packet's SSRC
    UInt32* ssrcPtr = (UInt32*)*outPacket;
    ssrcPtr[2] = fTrackInfo[thePacket->fTrackID].fSSRC;
    
    Float64 transmitTime = thePacket->fTransmitTime;
    
    this->SkipToNextPacket(thePacket);
    return transmitTime;    
}

void RTPFileSession::SkipToNextPacket(RTPFilePacket* inCurPacket)
{
    // Skip over this packet
    Assert(inCurPacket->fPacketLength < 1500);
    fReadBufferOffset += inCurPacket->fPacketLength;
    fCurrentPacket += (inCurPacket->fPacketLength + sizeof(RTPFilePacket));
    
    // Check to see if we need to read more data
    if (fReadBufferOffset >= fDataBufferLen)
    {
#if RTPFILESESSIONDEBUG
        qtss_printf("In SkipToNextPacket. Out of data\n");
#endif
        fCurrentPacket = NULL;
    }
}

void RTPFileSession::ReadAndAdvise()
{
    if (fReadBuffer == NULL)
    {
        // In some situations, callers of this function may not want the fDataBuffer
        // to be disturbed (see Seek()). If that's the case, the caller will set
        // fReadBuffer to NULL, and we must allocate it here
        fReadBuffer = NEW UInt8[fDataBufferSize];
    }
        
    // Read the next block. There should always be at least one packet
    // here, as we have a valid block in the block table.
#if RTPFILESESSIONDEBUG
    //qtss_printf("Moving onto next block. File loc: %qd\n",fFileSource.GetCurOffset());
#endif
    fDataBufferLen = 0;
    //(void)fFileSource.Read(fDataBuffer, fDataBufferSize, &fDataBufferLen);
    QTSS_Error theErr = QTSS_Read(fFileSource, fDataBuffer, fDataBufferSize, &fDataBufferLen);
    Assert(theErr == QTSS_NoErr);
    Assert(fDataBufferLen > sizeof(RTPFilePacket));
    fCurrentPosition += fDataBufferLen;
    
    // Now do an advise for the next block, if this block isn't the last.
    if (fCurrentPosition < fFileLength)
    {
        Assert(fDataBufferLen == fDataBufferSize);
        //fFileSource.Advise(fFileSource.GetCurOffset(), fDataBufferSize);
        theErr = QTSS_Advise(fFileSource, fCurrentPosition, fDataBufferSize);
    }
    fReadBufferOffset = 0;
    fCurrentPacket = fDataBuffer;
}

UInt32 RTPFileSession::PowerOf2Floor(UInt32 inNumToFloor)
{
    UInt32 retVal = 0x10000000;
    while (retVal > 0)
    {
        if (retVal & inNumToFloor)
            return retVal;
        else
            retVal >>= 1;
    }
    return retVal;
}


RTPFile::RTPFile()
:   fTrackInfo(NULL),
    fBlockMap(NULL),
    fBytesPerSecond(0),
    fMaxTrackNumber(0)
{}

RTPFile::~RTPFile()
{
    delete [] fFilePath.Ptr;
    delete [] fSDPData.Ptr;
    delete [] fTrackInfo;
    delete [] fBlockMap;
}

RTPFileSession::ErrorCode   RTPFile::Initialize(const StrPtrLen& inFilePath)
{
    //OSFileSource theFile(inFilePath.Ptr);
    QTSS_Object theFile = NULL;
    QTSS_Error theErr = QTSS_OpenFileObject(inFilePath.Ptr, 0, &theFile);
    if (theErr != QTSS_NoErr)
        return RTPFileSession::errFileNotFound;
    
    // Copy the path.
    fFilePath.Ptr = NEW char[inFilePath.Len + 1];
    ::memcpy(fFilePath.Ptr, inFilePath.Ptr, inFilePath.Len);
    fFilePath.Len = inFilePath.Len;
    
    // Setup our osref
    fRef.Set(fFilePath, this);
    
    // Read the header
    //OS_Error theErr = theFile.Read(&fHeader, sizeof(fHeader));
    UInt32 theLengthRead = 0;
    theErr = QTSS_Read(theFile, &fHeader, sizeof(fHeader), &theLengthRead);
    Assert(theErr == QTSS_NoErr);
    Assert(theLengthRead == sizeof(fHeader));
    
    // Read the SDP data
    fSDPData.Len = fHeader.fSDPLen;
    fSDPData.Ptr = NEW char[fSDPData.Len + 1];
    //theErr = theFile.Read(fSDPData.Ptr, fSDPData.Len);
    theErr = QTSS_Read(theFile, fSDPData.Ptr, fSDPData.Len, &theLengthRead);
    Assert(theErr == QTSS_NoErr);
    Assert(theLengthRead == fSDPData.Len);
    
    // Parse the SDP Information
    fSourceInfo.Parse(fSDPData.Ptr, fSDPData.Len);

    // Read the track info
    fTrackInfo = NEW RTPFileTrackInfo[fHeader.fNumTracks];
    //theErr = theFile.Read(fTrackInfo, sizeof(RTPFileTrackInfo) * fHeader.fNumTracks);
    theErr = QTSS_Read(theFile, fTrackInfo, sizeof(RTPFileTrackInfo) * fHeader.fNumTracks, &theLengthRead);
    Assert(theErr == QTSS_NoErr);
    Assert(theLengthRead == sizeof(RTPFileTrackInfo) * fHeader.fNumTracks);
    
    // Create and read the block map
    fBlockMap = NEW UInt8[fHeader.fBlockMapSize];
    //theErr = theFile.Read(fBlockMap, fHeader.fBlockMapSize);
    theErr = QTSS_Read(theFile, fBlockMap, fHeader.fBlockMapSize, &theLengthRead);
    Assert(theErr == QTSS_NoErr);
    Assert(theLengthRead == fHeader.fBlockMapSize);
    
    // Calculate bit rate of all the tracks combined
    Float64 totalBytes = 0;
    for (UInt32 x = 0; x < fHeader.fNumTracks; x++)
        totalBytes += (Float64)fTrackInfo[x].fBytesInTrack;
    totalBytes /= fHeader.fMovieDuration;
    fBytesPerSecond = (UInt32)totalBytes;
    
    //Get the max track number
    fMaxTrackNumber = 0;
    for (UInt32 y = 0; y < fHeader.fNumTracks; y++)
        if (fTrackInfo[y].fID > fMaxTrackNumber)
            fMaxTrackNumber = fTrackInfo[y].fID;
            
    (void)QTSS_CloseFileObject(theFile);
    return RTPFileSession::errNoError;
}

SInt64      RTPFile::GetBlockLocation(Float64 inTimeInSecs)
{
    if (inTimeInSecs == 0)
        return fHeader.fDataStartPos;
    
    UInt32 theTime = 0;
    UInt32 x = 0;
    for ( ; x < fHeader.fBlockMapSize ; x++)
    {
        theTime += fBlockMap[x];
        if (theTime >= (UInt32)inTimeInSecs)
        {
            if ((theTime > (UInt32)inTimeInSecs) && (x > 0))
                // If we've moved too far, go back to the previous block
                x--;
            return fHeader.fDataStartPos + (kBlockSize * x);
        }
    }
    return fHeader.fDataStartPos + (kBlockSize * x);// The requested time must be in the last block (or nonexistent).
}


Float64     RTPFile::GetTrackDuration(UInt32 inTrackID)
{
    for (UInt32 x = 0; x < fHeader.fNumTracks; x++)
    {
        if (fTrackInfo[x].fID == inTrackID)
            return fTrackInfo[x].fDuration;
    }
    Assert(0);
    return -1;
}


UInt32      RTPFile::GetTrackTimeScale(UInt32 inTrackID)
{
    for (UInt32 x = 0; x < fHeader.fNumTracks; x++)
    {
        if (fTrackInfo[x].fID == inTrackID)
            return fTrackInfo[x].fTimescale;
    }
    Assert(0);
    return 0;
}

UInt64      RTPFile::GetTrackBytes(UInt32 inTrackID)
{
    for (UInt32 x = 0; x < fHeader.fNumTracks; x++)
    {
        if (fTrackInfo[x].fID == inTrackID)
            return fTrackInfo[x].fBytesInTrack;
    }
    Assert(0);
    return 0;
}

Bool16      RTPFile::TrackExists(UInt32 inTrackID)
{
    for (UInt32 x = 0; x < fHeader.fNumTracks; x++)
    {
        if (fTrackInfo[x].fID == inTrackID)
            return true;
    }
    return false;
}

