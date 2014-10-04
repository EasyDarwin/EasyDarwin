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

#include "MP3FileBroadcaster.h"
#include <fcntl.h>
//#include <unistd.h>
#include <stdio.h>

#include "OS.h"
#include "OSThread.h"

int gBitRateArray[] = { 0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0 };
int gFrequencyArray[] = { 44100, 48000, 32000, 0 };

int gBitRateArrayv2[] = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0 };
int gFrequencyArrayv2[] = { 22050, 24000, 16000, 0 };
int gFrequencyArrayv2_5[] = { 11025, 12000, 8000, 0 };

MP3FileBroadcaster::MP3FileBroadcaster(TCPSocket* socket, int bitrate, int frequency, int bufferSize) :
    mSocket(socket),
    mBitRate(bitrate),
    mBufferSize(bufferSize),
    mNumFramesSent(0),
    mBroadcastStartTime(0),
    mBuffer(NULL),
    mUpdater(NULL),
    mDesiredBitRate(bitrate),
    mDesiredFrequency(frequency)
{
    mBuffer = new unsigned char[bufferSize];
    mTitle[0] = 0;
    mArtist[0] = 0;
    mSong[0] = 0;
}

MP3FileBroadcaster::~MP3FileBroadcaster()
{
    delete [] mBuffer;
}

int MP3FileBroadcaster::PlaySong(char *fileName, char *currentFile, bool preflight, bool fastpreflight)
{
    UInt32 length, lengthSent;
    
    if (mBuffer == NULL)
        return -1;
    
    mFile.Set(fileName);
    if (!mFile.IsValid())
        return kCouldntOpenFile;
    
    CheckForTags();
    
    if (strlen(mTitle) == 0)
    {
        char* temp = fileName+strlen(fileName);
        while ((temp > fileName) && (*(temp-1) != kPathDelimiterChar))
            temp--;
            
        ::strncpy(mTitle, temp,sizeof(mTitle) -1);
        mTitle[sizeof(mTitle) -1] = 0;	
    }
    
    if (strlen(mArtist) != 0 && strlen(mAlbum) != 0)
        qtss_sprintf(mSong, "%s - %s (%s)", mTitle, mArtist, mAlbum);
    else if (strlen(mArtist) != 0)
        qtss_sprintf(mSong, "%s - %s", mTitle, mArtist);
    else
        ::strcpy(mSong, mTitle);
    
    if (preflight)
        qtss_printf("Preflighting %s\n", mSong);
    
    // skip all the padding at the beginning of the file
    mFile.Seek(mStartByte);
    bool done = false;

    OS_Error err = mFile.Read(mBuffer, mBufferSize, &length);
    if (err != OS_NoErr)
        return -1;

    for(UInt32 i = 0; i<length-1000; i++)
    {
        if ((mBuffer[i] == 0xff) && CheckHeaders(mBuffer + i) )
        {
            mStartByte += i;
            done = true;
            break;
        }
    }
        
	if (!done)
    {
        mFile.Close();
        return kBadFileFormat;
    }

    if (mDesiredBitRate && (mBitRate != mDesiredBitRate))
    {
		qtss_printf("File %s : ",fileName);
		qtss_printf("Bitrate = %dkbits, frequency = %dKHz\n", mBitRate, mFrequency/1000);

        mFile.Close();
        return kWrongBitRate;
    }
        
    if (mDesiredFrequency == -1)
    {
        mDesiredFrequency = mFrequency;
        qtss_printf("Setting required frequency to %dKHz\n", mFrequency/1000);
    }
    
    if (mDesiredFrequency && (mFrequency != mDesiredFrequency))
    {
    	qtss_printf("File %s : ",fileName);
    	qtss_printf("Bitrate = %dkbits, frequency = %dKHz\n", mBitRate, mFrequency/1000);

        mFile.Close();
        return kWrongFrequency;
    }
    
    if (mUpdater)
        mUpdater->RequestMetaInfoUpdate(mSong);

    mFile.Seek(mStartByte);
    
    if (mBroadcastStartTime == 0)
        mBroadcastStartTime = OS::Milliseconds();
    //unused UInt64 startTime = OS::Milliseconds();
    int totalBytes = 0;
    //unused int numFrames = 0; // amount of play time this buffer represents
    int leftOver = 0;   // we may have a partial buffer left over from last read
    SInt64 properElapsedTime;
    while(true)
    {
        if (!preflight)
        {
            // the time length each frame represents depends on the frequency
            int numSamplesPerFrame = mIsMPEG2 ? 576 : 1152; // these are MP3 standards
            properElapsedTime = mNumFramesSent * numSamplesPerFrame * 1000 / mFrequency; // frequency is samples per second
            SInt64 nextSendTime = mBroadcastStartTime + properElapsedTime;

            SInt64 currentTime = OS::Milliseconds();
            if (nextSendTime > currentTime)
                OSThread::Sleep( (UInt32) (nextSendTime - currentTime));
        }
        
        length = 0;
        err = mFile.Read(mBuffer + leftOver, mBufferSize - leftOver, &length);
        if ((err != OS_NoErr) || (length == 0))
            break;

        length += leftOver;
        mNumFramesSent += CountFrames(mBuffer, length, &leftOver);
        
        if (!preflight)
        {
            OS_Error err = mSocket->Send((char*)mBuffer, length - leftOver, &lengthSent);
            if (err != 0)
            {
                mFile.Close();
                return kConnectionError;
            }
        }
            
        totalBytes += length;
        if (leftOver > 0)
            ::memcpy(mBuffer, mBuffer+length-leftOver, leftOver);
            
        if (preflight && fastpreflight && (totalBytes > 20 * 1024))
            break;  // just check first 20K of file
    }
    
//  UInt64 elapsed = OS::Milliseconds() - startTime;
//  UInt64 rate = (UInt64)totalBytes * 8 * 1000 / elapsed;
//  qtss_printf("Sent %d bytes in %qd milliseconds = %qd bits per second\n", totalBytes, elapsed, rate);
    
    if ((mDesiredFrequency == 0) || preflight)
    {
        // if we aren't fixing the frequency, then we need to time each song seperately
        mBroadcastStartTime = OS::Milliseconds();
        mNumFramesSent = 0;
    }
    
    mFile.Close();
    
    return 0;
}

void MP3FileBroadcaster::CheckForTags()
{
    mArtist[0] = 0;
    mTitle[0] = 0;
    mAlbum[0] = 0;
    mStartByte = 0;
    
    if (ReadV2_3Tags())
        return;
        
    if (ReadV2_2Tags())
        return;
        
    if (ReadV1Tags())
        return;
    
    return; 
}

bool MP3FileBroadcaster::ReadV1Tags()
{
    char buffer[128] = "";
    UInt32 length = 0;
    int i;
    
    mFile.Seek(mFile.GetLength()-128);
    OS_Error err = mFile.Read(buffer, sizeof(buffer), &length);
    if ((err != OS_NoErr) || (length != 128))
        return false;
        
    if (strncmp(buffer, "TAG", 3))
        return false;
        
    // Song Title
    // stored as space padded 30 byte buffer (no null termination)
    memcpy(mTitle, buffer+3, 30);
    for (i = 29; i>=0; i--)
        if (mTitle[i] != ' ')
        {
            mTitle[i+1] = 0;
            break;
        }
        
    // Artist Name
    // stored as space padded 30 byte buffer (no null termination)
    memcpy(mArtist, buffer+33, 30);
    for (i = 29; i>=0; i--)
        if (mArtist[i] != ' ')
        {
            mArtist[i+1] = 0;
            break;
        }

    // Album Title
    // stored as space padded 30 byte buffer (no null termination)
    memcpy(mAlbum, buffer+63, 30);
    for (i = 29; i>=0; i--)
        if (mAlbum[i] != ' ')
        {
            mAlbum[i+1] = 0;
            break;
        }

    return true;
}

bool MP3FileBroadcaster::ReadV2_2Tags()
{
    char buffer[1024] = "";
    UInt32 length = 0;
    
    mFile.Seek(0);
    OS_Error err = mFile.Read(buffer, sizeof(buffer), &length);
    if (err)
        return false;
        
    if (length < 4) // tag header size
        return false;
        
    if (strncmp(buffer, "ID3", 3))
        return false;
    
    if (buffer[3] != 2)
        return false;
        
    // we have a valid v2.2 tag header
    char* ptr = buffer + 10;
    
    // the total length of tags is encoded in this strange way to avoid being
    // interpreted as an MP3 "sync" flag (don't use the top bit of each byte).
    int totalTagLen = buffer[6]*2097152+buffer[7]*16384+buffer[8]*128+buffer[9];
    mStartByte = totalTagLen;   // skip tags when streaming
    
    // OK, I'm being lazy here, but if someone can't find a way to put the song
    // title and artist in the first 1K of header then they're just being plain mean.
    if (totalTagLen > 1024) totalTagLen = 1024;

    while (ptr-buffer < totalTagLen)
    {
        if (*ptr == 0)
            break;
            
        // next three bytes are length, so go two bytes, copy 4 and mask off one
        int fieldLen = ntohl(OS::GetUInt32FromMemory((UInt32*)(ptr+2))) & 0x00ffffff;
        
        if (!strncmp(ptr, "TP1", 3))    // Artist
        {
            int len = fieldLen;
            if (len > 255) len = 255;
            if (ptr[6] == 0)
            {
                ::memcpy(mArtist, ptr+7, len-1);    // skip encoding byte
                mArtist[len-1] = 0;
            }
            else
                ConvertUTF16toASCII(ptr+7, len-1, mArtist, sizeof(mArtist));
        }

        if (!strncmp(ptr, "TT2", 3))    // Title
        {
            int len = fieldLen;
            if (len > 255) len = 255;
            if (ptr[6] == 0)
            {
                ::memcpy(mTitle, ptr+7, len-1); // skip encoding byte
                mTitle[len-1] = 0;
            }
            else
                ConvertUTF16toASCII(ptr+7, len-1, mTitle, sizeof(mTitle));
        }
        
        if (!strncmp(ptr, "TAL", 3))    // Album
        {
            int len = fieldLen;
            if (len > 255) len = 255;
            if (ptr[6] == 0)
            {
                ::memcpy(mAlbum, ptr+7, len-1); // skip encoding byte
                mAlbum[len-1] = 0;
            }
            else
                ConvertUTF16toASCII(ptr+7, len-1, mAlbum, sizeof(mAlbum));
        }
        
        if ((strlen(mTitle) > 0) && (strlen(mArtist) > 0) && (strlen(mAlbum) > 0))
            break;  // we found the tags we need
        
        ptr += fieldLen + 6;    // skip field and header
    }
    
    return true;
}

bool MP3FileBroadcaster::ReadV2_3Tags()
{
    char buffer[1024];
    UInt32 length;
    
    mFile.Seek(0);
    OS_Error err = mFile.Read(buffer, sizeof(buffer), &length);
    if (err) 
        return false;
        
    if (strncmp(buffer, "ID3", 3))
        return false;
    
    if (buffer[3] != 3)
        return false;
        
    // we have a valid v2.3 tag header
    char* ptr = buffer + 10;
    
    // skip extended header if it exists
    if ((buffer[4] & 0x40) != 0)
        ptr += 10;
        
    // if any other flags are set (like "unsychronization"), bail.
    // these can be supported in the future.
    if (((buffer[4] & 0xbf) != 0) || (buffer[5] != 0))
        return false;
    
    // the total length of tags is encoded in this strange way to avoid being
    // interpreted as an MP3 "sync" flag (don't use the top bit of each byte).
    int totalTagLen = buffer[6]*2097152+buffer[7]*16384+buffer[8]*128+buffer[9];
    mStartByte = totalTagLen;   // skip tags when streaming
    
    // OK, I'm being lazy here, but if someone can't find a way to put the song
    // title and artist in the first 1K of header then they're just being plain mean.
    if (totalTagLen > 1024) totalTagLen = 1024;

    while (ptr-buffer < totalTagLen)
    {
        if (*ptr == 0)
            break;
            
        int fieldLen = ntohl(OS::GetUInt32FromMemory((UInt32*)(ptr+4)));
        
        // should check compression and encryption flags for these fields, but I
        // wouldn't really expect them to be set for title or artist
        
        if (!::strncmp(ptr, "TPE1", 4)) // Artist
        {
            int len = fieldLen;
            if (len > 255) len = 255;
            if (ptr[10] == 0)
            {
                ::memcpy(mArtist, ptr+11, len-1);   // skip encoding byte
                mArtist[len-1] = 0;
            }
            else
                ConvertUTF16toASCII(ptr+11, len-1, mArtist, sizeof(mArtist));
        }

        if (!strncmp(ptr, "TIT2", 4))   // Title
        {
            int len = fieldLen;
            if (len > 255) len = 255;
            if (ptr[10] == 0)
            {
                ::memcpy(mTitle, ptr+11, len-1);    // skip encoding byte
                mTitle[len-1] = 0;
            }
            else
                ConvertUTF16toASCII(ptr+11, len-1, mTitle, sizeof(mTitle));
        }
        
        if (!strncmp(ptr, "TALB", 4))   // Album
        {
            int len = fieldLen;
            if (len > 255) len = 255;
            if (ptr[10] == 0)
            {
                ::memcpy(mAlbum, ptr+11, len-1);    // skip encoding byte
                mAlbum[len-1] = 0;
            }
            else
                ConvertUTF16toASCII(ptr+11, len-1, mAlbum, sizeof(mAlbum));
        }
        
        if ((::strlen(mTitle) > 0) && (::strlen(mArtist) > 0) && (::strlen(mAlbum) > 0))
            break;  // we found the tags we need
        
        ptr += fieldLen + 10;   // skip field and header
    }
    
    return true;
}

bool MP3FileBroadcaster::CheckHeaders(unsigned char * buffer)
{
    int bitRate, bitRate2;
    int frequency, frequency2;
    int recordSize;
    
    if (!ParseHeader(buffer, &bitRate, &frequency, &recordSize))
        return false;
    
    if (!ParseHeader(buffer + recordSize, &bitRate2, &frequency2, &recordSize))
        return false;
        
    if (frequency != frequency2)
        return false;
        
    mBitRate = bitRate;
    mFrequency = frequency;

    return true;
}

bool MP3FileBroadcaster::ParseHeader(unsigned char* buffer, int* bitRate, int* frequency, int* recordSize)
{
    if ((buffer[0] != 0xff) || ((buffer[1] & 0xe6) != 0xe2))
        return false;       // not a valid MP3 header (not valid or not layer 3)
        
    int version = (buffer[1] & 0x18) >> 3;
    
    mIsMPEG2 = (version != 3);
    
    if (mIsMPEG2)
        *bitRate = gBitRateArrayv2[buffer[2] >> 4];     // MPEG2 
    else
        *bitRate = gBitRateArray[buffer[2] >> 4];       // MPEG 1

    if (version == 3)
        *frequency = gFrequencyArray[(buffer[2] & 0x0a) >> 2];
    else if (version == 2)
        *frequency = gFrequencyArrayv2[(buffer[2] & 0x0a) >> 2];
    else
        *frequency = gFrequencyArrayv2_5[(buffer[2] & 0x0a) >> 2];
    
    if ((*bitRate == 0) || (*frequency == 0))
        return false;
        
    int pad = (buffer[2] & 0x02) >> 1;
    
    *recordSize = 144000 * *bitRate / *frequency;   // standard MP3 calculation

    // MPEG 2 (and 2.5) frames encode half the number of samples
    if (mIsMPEG2)
        *recordSize /= 2;
        
    *recordSize += pad;
    
    return true;
}

int MP3FileBroadcaster::CountFrames(unsigned char* buffer, UInt32 length, int* leftOver)
{
    int bitRate;
    int frequency;
    int recordSize;

    UInt32 offset = 0;
    int numFrames = 0;
    while ( offset < length)
    {
        if (length - offset < 4)
            break;  // we don't have a whole header left, so move on
        
        if (!ParseHeader(buffer + offset, &bitRate, &frequency, &recordSize))
        {
            // Oops, we lost our stream, so advance byte by byte looking for a frame header
            offset++;
            continue;
        }
        
        if ( ((UInt32) recordSize + offset) > length)
            break;  // we don't have the whole frame in this buffer.
        
        numFrames++;
        
        offset += recordSize;
    }
    
    // usually there will be a partial frame left over.  We leave this for next time.
    *leftOver = length - offset;
    
    return numFrames;
}

// We really should be converting from Unicode to Latin-1, but the conversion of the high byte characters isn't
// easy.  If I find some code or tables to do this I can include it later.
bool MP3FileBroadcaster::ConvertUTF16toASCII(char* sourceStr,int sourceSize, char* dest, int destSize)
{
    unsigned short *sourceStart = (unsigned short *)sourceStr;
    unsigned short *sourceEnd = (unsigned short *)(sourceStart + sourceSize);
    unsigned char *targetStart = (unsigned char *)dest;
    unsigned char *targetEnd = targetStart + destSize;
    
    bool result = true;
    unsigned short *source = sourceStart;
    unsigned char *target = targetStart;
    unsigned short ch;
    bool doSwap = false;
    ch = *source++;
    Assert((ch == 0xfffe) || (ch == 0xfeff));
    if (ch != 0xfeff)
        doSwap = true;

    while ((source < sourceEnd) && (target <= targetEnd))
    {
        ch = *source++;
        if (doSwap)
        {
            unsigned short low = (ch & 0xff) << 8;
            ch = (ch >> 8) | low;
        }

        if (ch < 0x80)
        {
            *target = (UInt8) ch;
            target++;
        }
    }

    if (target <= targetEnd)
        *target = 0;
    else
        result = false;

    return result;
}
