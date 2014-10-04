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

#ifndef __MP3FileBroadcaster_H__
#define __MP3FileBroadcaster_H__

#include "TCPSocket.h"
#include "MP3MetaInfoUpdater.h"
#include "OSFileSource.h"

class MP3FileBroadcaster
{
public:
    MP3FileBroadcaster(TCPSocket* socket, int bitrate, int frequency, int bufferSize = 7000);
    ~MP3FileBroadcaster();
    
    void SetInfoUpdater(MP3MetaInfoUpdater* updater) { mUpdater = updater; }
    
    int PlaySong(char *fileName, char *currentFile, bool preflight = false, bool fastpreflight = false);
    
    enum
    {
        kBadFileFormat = 1,
        kWrongFrequency = 2,
        kWrongBitRate = 3,
        kConnectionError = 4,
        kCouldntOpenFile = 5
    };
    
    char* GetTitle() { return mTitle; }
    char* GetArtist() { return mArtist; }
    char* GetAlbum() { return mAlbum; }
    char* GetSong() { return mSong; }
    
private:
    void CheckForTags();
    bool ReadV1Tags();
    bool ReadV2_2Tags();
    bool ReadV2_3Tags();
    
    void UpdateMetaInfo();
    
    bool CheckHeaders(unsigned char * buffer);
    bool ParseHeader(unsigned char* buffer, int* bitRate, int* frequency, int* recordSize);
    int CountFrames(unsigned char* buffer, UInt32 length, int* leftOver);
    
    bool ConvertUTF16toASCII(char* sourceStr,int sourceSize, char* dest, int destSize);

    TCPSocket*  mSocket;
    int         mBitRate;
    int         mFrequency;
    bool        mIsMPEG2;
    int         mBufferSize;
    int         mDelay;
    UInt64      mNumFramesSent;
    UInt64      mBroadcastStartTime;
    unsigned char* mBuffer;
    MP3MetaInfoUpdater* mUpdater;
    
    int         mDesiredBitRate;
    int         mDesiredFrequency;
    
    OSFileSource mFile;
    int         mStartByte;
    
    char        mTitle[256];
    char        mArtist[256];
    char        mAlbum[256];
    char        mSong[780];
};

#endif
