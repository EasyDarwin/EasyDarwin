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

#include "MP3MetaInfoUpdater.h"
#include "StringTranslator.h"

MP3MetaInfoUpdater::MP3MetaInfoUpdater(char* password, char* mountPoint, UInt32 addr, UInt16 port)
    : mPassword(NULL),
    mMountPoint(NULL),
    mSocket(NULL, 0),
    mAddr(addr),
    mPort(port),
    mFirstTime(true)
{
    mPassword = new char[strlen(password) + 1];
    strcpy(mPassword, password);
    mMountPoint = new char[strlen(mountPoint) + 1];
    strcpy(mMountPoint, mountPoint);
}

MP3MetaInfoUpdater::~MP3MetaInfoUpdater()
{
    delete [] mPassword;
    delete [] mMountPoint;
    SendStopRequest();
    mCond.Signal();
}

void MP3MetaInfoUpdater::Entry()
{
    while(!IsStopRequested())
    {
        mMutex.Lock();
        mCond.Wait(&mMutex);
        mMutex.Unlock();
        if (!IsStopRequested())
        {
            if (mFirstTime)
            {
                Sleep(3000);        // give the stream a chance to get established (icecast isn't happy otherwise)
                mFirstTime = false;
            }
            DoUpdateMetaInfo();
        }
    }
}

void MP3MetaInfoUpdater::RequestMetaInfoUpdate(char* song)
{
    char temp[600];
    strcpy(temp, song);
    StringTranslator::EncodeURL(temp, strlen(temp) + 1, mSong, sizeof(mSong));
    mCond.Signal();
}

void MP3MetaInfoUpdater::DoUpdateMetaInfo()
{
    mSocket.Open();
    int err = mSocket.Connect(mAddr, mPort);
    
    if (!err)
    {
        UInt32 len;
        char* buffer = new char[100 + strlen(mSong) + strlen(mPassword) + strlen(mMountPoint)];
        qtss_sprintf(buffer, "GET /admin.cgi?mode=updinfo&pass=%s&mount=%s&song=%s HTTP/1.0\r\nUser-Agent: Darwin MP3Broadcaster\r\n\r\n",
                            mPassword, mMountPoint, mSong);
        
        mSocket.Send(buffer, strlen(buffer), &len);
        delete [] buffer;
    }
    
    mSocket.Cleanup();
}
