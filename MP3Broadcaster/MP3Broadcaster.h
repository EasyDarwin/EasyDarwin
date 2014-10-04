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

#ifndef __MP3Broadcaster_H__
#define __MP3Broadcaster_H__

#include "OSHeaders.h"
#include "TCPSocket.h"
#include "SocketUtils.h"
#include "PickerFromFile.h"
#include "MP3BroadcasterLog.h"

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

class MP3Broadcaster
{
public:
    MP3Broadcaster(char* ipaddr, int port, char* config, char* playList, char* workingDir, Bool16 useICY = false);
    ~MP3Broadcaster(){}
    
    //void SetBroadcastAddr(char* addrString);
    //void SetConfigPath(char* path);
    //void SetBitRate(int bitRate);
    //void SetPlayList(char* path);
    
    //Bool16 CheckConfig();
    int  ConnectToServer();
    void PreFlightOrBroadcast( bool preflight, bool daemonize, bool showMovieList, bool currentMovie, bool checkMP3s, const char* errorlog);
        
    Bool16    IsValid() { return (Bool16) mValid; }
    
    char* GetPIDFilePath() { return mPIDFile; }
        
        void Cleanup(bool signalHandler = false);
        
private:
    static Bool16 ConfigSetter( const char* paramName, const char* paramValue[], void * userData );
    static Bool16 SetEnabled( const char* value, Bool16* field);
    static void PrintPlaylistElement(PLDoubleLinkedListNode<SimplePlayListElement> *node,void *file);

    void    CreateWorkingFilePath(char* extension, char* result);
    bool    FileCreateAndCheckAccess(char *theFileName);
    void    CreateCurrentAndUpcomingFiles();
    void    UpdatePlaylistFiles(PlaylistPicker *picker,PlaylistPicker *insertPicker);
    void    UpdateCurrentFile(char *thePick);
    void    ShowPlaylistElements(PlaylistPicker *picker,FILE *file);
    //char* GetBroadcastDirPath(const char * setupFilePath);
    PlaylistPicker* MakePickerFromConfig();
    int     SendXAudioCastHeaders();
    void    ShowSetupParams();
    void    RemoveFiles();
    char*   MapErrorToString(int error);

    Bool16  mValid;
    
    char    mIPAddr[256];
    int     mPort;
    int     mBitRate;
    int     mFrequency;
    char    mPlayListPath[PATH_MAX];
    char    mWorkingDirPath[PATH_MAX];
    char    mCurrentFile[PATH_MAX];
    char    mUpcomingFile[PATH_MAX];
    char    mReplaceFile[PATH_MAX];
    char    mStopFile[PATH_MAX];
    char    mInsertFile[PATH_MAX];
    char    mLogFile[PATH_MAX];
    char    mPIDFile[PATH_MAX];
    
    char    mPlayMode[256];
    int     mUpcomingSongsListSize;
    int     mRecentSongsListSize;
    char    mName[256];
    char    mGenre[256];
    char    mPassword[256];
    char    mURL[PATH_MAX];
    char    mMountPoint[PATH_MAX];
    
    Bool16  mLogging;
    Bool16  mShowCurrent;
    Bool16  mShowUpcoming;
    
	SInt32  mNumErrors;
	SInt32  mNumWarnings;
	bool    mPreflight;
	bool    mCleanupDone;
        
    PlaylistPicker* mTempPicker;
    int     mElementCount;
    
    TCPSocket mSocket;
    MP3BroadcasterLog* mLog;
	Bool16    mUseICY;

    static MP3Broadcaster* sBroadcaster;

};

#endif

