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

/* contributions by emil@popwire.com  
*/

#ifndef playlist_broadcaster_H
#define playlist_broadcaster_H

#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include <string.h>
#include <errno.h>
#include "OSHeaders.h"
#include "playlist_utils.h"
#include "playlist_elements.h"
#include "playlist_lists.h"
#include "playlist_parsers.h"
#include "QTRTPFile.h"
#include "PLBroadcastDef.h"


#ifndef __Win32__
    #include <sys/types.h>
    #include <fcntl.h>
#endif

static PlayListUtils gUtils;


class QTFileBroadcaster 
{

protected:
    
    QTRTPFile               *fRTPFilePtr ;
    SDPFileParser           fStreamSDPParser;
    SDPFileParser           *fMovieSDPParser;
    SocketList              fSocketlist;
    MediaStreamList         fMediaStreamList;
    int                     fBasePort;
    bool                    fDebug;
    bool                    fDeepDebug;
    bool                    *fQuitImmediatePtr;

// transmit time trackers
    Float64                 fLastTransmitTime;
    
    SInt64                  fStreamStartTime;
    SInt64                  fMovieStartTime;
    SInt64                  fMovieEndTime;
    SInt64                  fMovieIntervalTime;
    SInt64                  fMovieTimeDiffMilli;
    
    bool                    fMovieStart;
    Float64                 fSendTimeOffset;
    Float64                 fMovieDuration;
    int                     fMovieTracks;
    int                     fMappedMovieTracks;
    UInt64                  fNumMoviesPlayed;
        
    PayLoad *               FindPayLoad(short id, ArrayList<PayLoad> *PayLoadListPtr);
    bool                    CompareRTPMaps(TypeMap *movieMediaTypePtr, TypeMap *streamMediaTypePtr, short id);
    bool                    CompareMediaTypes(TypeMap *movieMediaTypePtr, TypeMap *streamMediaTypePtr);
    UInt32                  GetSDPTracks(QTRTPFile *newRTPFilePtr);
    int                     SetUpAMovie(char *movieFileName);
    int                     AddTrackAndStream(QTRTPFile *newRTPFilePtr);
    int                     MapMovieToStream();
    int                     Play(char *mTimeFile);
    Float64                 SleepInterval(Float64 sleepTime) { return Sleep( (Float64) (PlayListUtils::Milliseconds() - fMovieStartTime) + sleepTime); };

    Float64                 Sleep(Float64 transmitTime);
    void                    SetDebug(bool debug) {fDebug = debug;};
    void                    SetDeepDebug(bool debug) {fDeepDebug = debug;};
    PLBroadcastDef          *fBroadcastDefPtr;  
public:
        QTFileBroadcaster();
        ~QTFileBroadcaster();
    
    
static  int     EvalErrorCode(QTRTPFile::ErrorCode err);
        int     SetUp(PLBroadcastDef *broadcastDefPtr, bool *quitImmediatePtr);
        int     PlayMovie(char *movieFileName, char *currentFile);
        int     GetMovieTrackCount() { return fMovieTracks; };
        int     GetMappedMovieTrackCount() { return fMappedMovieTracks; };
        void    MilliSleep(SInt32 sleepTimeMilli);
        bool    fPlay;
        bool    fSend;
    
    enum {  eClientBufferSecs = 0,
            eMaxPacketQLen = 200
         };
    
    enum ErrorID 
    {   // General errors
         eNoErr             = 0
        ,eParam                     
        ,eMem                       
        ,eInternalError     
        ,eFailedBind
        
        // Setup Errors
        ,eNoAvailableSockets 
        ,eSDPFileNotFound   
        ,eSDPDestAddrInvalid 
        ,eSDPFileInvalid    
        ,eSDPFileNoMedia     
        ,eSDPFileNoPorts    
        ,eSDPFileInvalidPort 
        ,eSDPFileInvalidName 
        ,eSDPFileInvalidTTL
        
        // eMem also
        
        // Play Errors,
        ,eMovieFileNotFound         
        ,eMovieFileNoHintedTracks   
        ,eMovieFileNoSDPMatches     
        ,eMovieFileInvalid          
        ,eMovieFileInvalidName          
        
        ,eNetworkConnectionError    
        ,eNetworkRequestError       
        ,eNetworkConnectionStopped  
        ,eNetworkAuthorization      
        ,eNetworkNotSupported       
        ,eNetworkSDPFileNameInvalidMissing 
        ,eNetworkSDPFileNameInvalidBadPath
        ,eNetworkConnectionFailed
        
        ,eDescriptionInvalidDestPort
    };
    
    char	fCurrentMovieName[256];
    char	fCurrentMovieCopyright[256];
    char	fCurrentMovieComment[256];
    char	fCurrentMovieAuthor[256];
    char	fCurrentMovieArtist[256];
    char	fCurrentMovieAlbum[256];
    
private:

};

#endif //playlist_broadcaster_H
