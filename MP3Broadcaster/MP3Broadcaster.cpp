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

#include "MP3Broadcaster.h"
#include "MP3MetaInfoUpdater.h"
#include "StringTranslator.h"

#include "defaultPaths.h"

#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include <signal.h>
#include <sys/stat.h>
#ifndef __Win32__
    #include <unistd.h>
#endif


#ifndef __Win32__
        #include <netdb.h>
    #if defined (__solaris__) || defined (__osf__) || defined (__hpux__)
        #include "daemon.h"
    #else
        #ifndef __FreeBSD__
            #include <sys/sysctl.h>
        #endif
    #endif
#endif


#include <errno.h>

#include "OSHeaders.h"
#include "OS.h"
#include "OSMemory.h"
#include "Task.h"
#include "TimeoutTask.h"
#include "OSArrayObjectDeleter.h"
#include "ConfParser.h"
#include "MP3FileBroadcaster.h"
// must now inlcude this from the project level using the -i switch in the compiler
#ifndef __MacOSX__
    #include "revision.h"
#endif


MP3Broadcaster* MP3Broadcaster::sBroadcaster = NULL;

//#include "MyAssert.h"

MP3Broadcaster::MP3Broadcaster(char* ipaddr, int port, char* config, char* playList, char* workingDir, Bool16 useICY) :
    mValid(true),
    mPort(8000),
    mBitRate(0),
    mFrequency(0),
    mUpcomingSongsListSize(7),
    mRecentSongsListSize(0),
    mLogging(0),
    mShowCurrent(true),
    mShowUpcoming(true),
    mNumErrors(0),
    mNumWarnings(0),
    mPreflight(false),
    mCleanupDone(false),
    mTempPicker(NULL),
    mSocket(NULL, 0),
    mLog(NULL),
    mUseICY(useICY)
{
    sBroadcaster = this;
    
    strcpy(mIPAddr, "128.0.0.1");
    strcpy(mPlayListPath, DEFAULTPATHS_ETC_DIR "mp3playlist.ply");
    strcpy(mWorkingDirPath, DEFAULTPATHS_ETC_DIR);
    strcpy(mPlayMode, "sequential");
    strcpy(mMountPoint, "/");
    strcpy(mGenre, "Pop");
    strcpy(mURL, "");
    strcpy(mPIDFile, "");

    //see if there is a defaults File.
    //if there is load it and over-ride the other defaults
    int len = ::strlen(config) + 10;
    char *defaultFileName = new char[len];
    qtss_snprintf(defaultFileName, len, "%s%s", config, ".def");
    (void) ::ParseConfigFile( false, defaultFileName, ConfigSetter, this ); //ignore if no defaults file
    delete [] defaultFileName;
 
    int err = ::ParseConfigFile( false, config, ConfigSetter, this );
    if (err)
    {
        mValid = false;
        return;
    }
    
    if (ipaddr != NULL)
    {
        // override config file
        // size limit for any IP addr is 255
        strncpy(mIPAddr, ipaddr, 255);
    }
    
    if (port != 0)
    {
        // override config file
        mPort = port;
    }
    
    if (playList)
    {
        // override config file
        // size limit for any playlist string is PATH_MAX - 1
        strncpy(mPlayListPath, playList, PATH_MAX-1);
    }
    
    if (workingDir)
    {
        // override config file
        // size limit for any working path is PATH_MAX - extension - 1
        strncpy(mWorkingDirPath, playList, PATH_MAX-12);
    }
    
    CreateWorkingFilePath(".current", mCurrentFile);
    CreateWorkingFilePath(".upcoming", mUpcomingFile);
    CreateWorkingFilePath(".replacelist", mReplaceFile);
    CreateWorkingFilePath(".stoplist", mStopFile);
    CreateWorkingFilePath(".insertlist", mInsertFile);
    CreateWorkingFilePath("mp3_broadcast.log", mLogFile);
}

Bool16 MP3Broadcaster::ConfigSetter( const char* paramName, const char* paramValue[], void* userData )
{
    // return true if set fails
    MP3Broadcaster* thisPtr = (MP3Broadcaster*)userData;

    if (!::strcmp( "destination_ip_address", paramName) )
    {   
        if (strlen(paramValue[0]) >= sizeof(thisPtr->mIPAddr))
            return true;
        
        strcpy(thisPtr->mIPAddr, paramValue[0]);
        return false;
    }
    else if (!::strcmp( "destination_base_port", paramName) )
    {
        thisPtr->mPort = atoi(paramValue[0]);

        return false;
    }
    else if (!::strcmp( "max_upcoming_list_size", paramName) )
    {
        if ( ::atoi( paramValue[0] )  < 0 )
            return true;
            
        thisPtr->mUpcomingSongsListSize = ::atoi( paramValue[0] );
        return false;
    }
    else if (!::strcmp( "play_mode", paramName) )
    {
        if (strlen(paramValue[0]) >= sizeof(thisPtr->mPlayMode))
            return true;
        
        strcpy(thisPtr->mPlayMode, paramValue[0]);
        return false;
    }   
    else if (!::strcmp( "recent_songs_list_size", paramName) )
    {
        if ( ::atoi( paramValue[0] )  < 0 )
            return true;
            
        thisPtr->mRecentSongsListSize = ::atoi( paramValue[0] );
        return false;
    }
    else if (!::strcmp( "playlist_file", paramName) )
    {
        if (strlen(paramValue[0]) >= sizeof(thisPtr->mPlayListPath))
            return true;
        
        strcpy(thisPtr->mPlayListPath, paramValue[0]);
        return false;
    }
    else if (!::strcmp( "working_dir", paramName) )
    {
        if (strlen(paramValue[0]) >= sizeof(thisPtr->mWorkingDirPath))
            return true;
        
        strcpy(thisPtr->mWorkingDirPath, paramValue[0]);
        return false;
    }
    else if (!::strcmp( "logging", paramName) )
    {
        return SetEnabled(paramValue[0], &thisPtr->mLogging);
    }
    else if (!::strcmp( "show_current", paramName) )
    {
        return SetEnabled(paramValue[0], &thisPtr->mShowCurrent);
    }
    else if (!::strcmp( "show_upcoming", paramName) )
    {
        return SetEnabled(paramValue[0], &thisPtr->mShowUpcoming);
    }
    else if (!::strcmp( "use_icy", paramName) )
    {
        return SetEnabled(paramValue[0], &thisPtr->mUseICY);
    }
    else if (!::strcmp( "broadcast_name", paramName) )
    {
        if (strlen(paramValue[0]) >= sizeof(thisPtr->mName))
            return true;
        
        strcpy(thisPtr->mName, paramValue[0]);
        return false;
    }
    else if (!::strcmp( "broadcast_password", paramName) )
    {
        if (strlen(paramValue[0]) >= sizeof(thisPtr->mPassword))
            return true;
        
        strcpy(thisPtr->mPassword, paramValue[0]);
        return false;
    }
    else if (!::strcmp( "broadcast_genre", paramName) )
    {
        if (strlen(paramValue[0]) >= sizeof(thisPtr->mGenre))
            return true;
        
        strcpy(thisPtr->mGenre, paramValue[0]);
        return false;
    }
    else if (!::strcmp( "broadcast_url", paramName) )
    {
        if (strlen(paramValue[0]) >= sizeof(thisPtr->mURL))
            return true;
        
        strcpy(thisPtr->mURL, paramValue[0]);
        return false;
    }
    else if (!::strcmp( "broadcast_bitrate", paramName) )
    {
        if ( ::atoi( paramValue[0] )  < 0 )
            return true;
            
        thisPtr->mBitRate = ::atoi( paramValue[0] );
        return false;
    }
    else if (!::strcmp( "broadcast_sample_rate", paramName) )
    {
        if ( ::atoi( paramValue[0] )  < -1 )
            return true;
            
        thisPtr->mFrequency = ::atoi( paramValue[0] );
        return false;
    }
    else if (!::strcmp( "broadcast_mount_point", paramName) )
    {
        if (strlen(paramValue[0]) >= sizeof(thisPtr->mMountPoint))
            return true;
        // Make sure the mountpoint always begins with a '/' character.
        // If its missing prepend it to the mountpoint name for them.
        thisPtr->mMountPoint[0] = '\0';
        if (*paramValue[0] != '/')
            strcpy(thisPtr->mMountPoint, "/");
        strcat(thisPtr->mMountPoint, paramValue[0]);
        return false;
    }
    else if (!::strcmp( "pid_file", paramName) )
    {
        if (strlen(paramValue[0]) >= sizeof(thisPtr->mPIDFile))
            return true;
        
        strcpy(thisPtr->mPIDFile, paramValue[0]);
        return false;
    }
    else if (!::strcmp( "max_err_file_k_size", paramName) )
    {
        if ( !paramValue[0] || !::strlen(paramValue[0]) )
             return true;
			
        UInt32 setvalue = kSInt32_Max;
        int maxValue = ::atoi( paramValue[0] );
        if (maxValue >= 0)
            setvalue = (UInt32) maxValue;
		
        qtss_setmaxprintfcharsinK( (UInt32) setvalue);
        return false;
    }
    
    return true;
}

Bool16 MP3Broadcaster::SetEnabled( const char* value, Bool16* field)
{
    if ( ::strcmp( "enabled", value) && ::strcmp( "disabled", value) )
        return true;
        
    *field = !strcmp( "enabled", value );
        
    return false;
}

void MP3Broadcaster::CreateWorkingFilePath(char* extension, char* result)
{
    if (strlen(mWorkingDirPath) + strlen(extension) >= PATH_MAX)
        result[0] = 0;
    else
    {
        strcpy(result, mWorkingDirPath);
        strcat(result, extension);
    }
}

void MP3Broadcaster::CreateCurrentAndUpcomingFiles()
{
    if (mShowCurrent) 
    {   
        if(FileCreateAndCheckAccess(mCurrentFile))
        {   /* error */
            mLog->LogInfo( "MP3Broadcaster Error: Failed to create current broadcast file" );
        }
    }


    if (mShowUpcoming) 
    {       
        if(FileCreateAndCheckAccess(mUpcomingFile))
        {   /* error */
            mLog->LogInfo( "MP3Broadcaster Error: Failed to create upcoming broadcast file" );
        }
    }
}

void MP3Broadcaster::UpdatePlaylistFiles(PlaylistPicker *picker,PlaylistPicker *insertPicker)
{
    if ( (NULL == picker) || (NULL == insertPicker) ) 
        return;
        
    /* if .stoplist file exists - prepare to stop broadcast */
    if(!access(mStopFile, R_OK))
    {
        picker->CleanList();
        PopulatePickerFromFile(picker, mStopFile, "", NULL);

        mTempPicker->CleanList();

        remove(mStopFile);
        picker->mStopFlag = true;
    }

    /* if .replacelist file exists - replace current playlist */
    if(!access(mReplaceFile, R_OK))
    {
        picker->CleanList();     
        PopulatePickerFromFile(picker, mReplaceFile, "", NULL);
        
        mTempPicker->CleanList();
        
        remove(mReplaceFile);
        picker->mStopFlag = false;
    }

    /* if .insertlist file exists - insert into current playlist */
    if(!access(mInsertFile, R_OK))
    {
        insertPicker->CleanList();
        mTempPicker->CleanList();

        PopulatePickerFromFile(insertPicker, mInsertFile, "", NULL);
        remove(mInsertFile);
        picker->mStopFlag = false;
    }


                // write upcoming playlist to .upcoming file 
    if (mShowUpcoming) 
    {
        FILE *upcomingFile = fopen(mUpcomingFile, "w");
        if(upcomingFile)
        {
            mElementCount = 0;
        
            if (!::strcmp(mPlayMode, "weighted_random")) 
                qtss_fprintf(upcomingFile,"#random play - upcoming list not supported\n");
            else
            {   qtss_fprintf(upcomingFile,"*PLAY-LIST*\n");
                ShowPlaylistElements(insertPicker,upcomingFile);
                ShowPlaylistElements(picker,upcomingFile);
                if (    picker->GetNumMovies() == 0 
                        && !picker->mStopFlag 
                        && 0 != ::strcmp(mPlayMode, "sequential") 
                    )
                {   picker->CleanList();
                    PopulatePickerFromFile(picker,mPlayListPath,"",NULL);
                    ShowPlaylistElements(picker,upcomingFile);
                    mTempPicker->CleanList();
                    PopulatePickerFromFile(mTempPicker,mPlayListPath,"",NULL);
                }
                
                if  (   mElementCount <= mUpcomingSongsListSize 
                        && 0 == ::strcmp(mPlayMode, "sequential_looped")
                    )
                {   if (mTempPicker->GetNumMovies() == 0)
                    {   mTempPicker->CleanList();
                        PopulatePickerFromFile(mTempPicker,mPlayListPath,"",NULL);
                    }
                    //sElementCount can be zero if the playlist contains no paths to valid files
                    while ( (mElementCount != 0) && mElementCount <= mUpcomingSongsListSize )
                        ShowPlaylistElements(mTempPicker,upcomingFile);
                }
            }
            fclose(upcomingFile);
        }   
    }
    else
    {   
        if (    picker->GetNumMovies() == 0 
                && !picker->mStopFlag 
                && ::strcmp(mPlayMode, "sequential") 
            )
        {   picker->CleanList();
            PopulatePickerFromFile(picker,mPlayListPath,"",NULL);
        }       
    }
}

void MP3Broadcaster::UpdateCurrentFile(char *thePick)
{
    if (NULL == thePick)
        return;
        
    // save currently playing song to .current file 
    if (mShowCurrent) 
    {   FILE *currentFile = fopen(mCurrentFile, "w");
        if(currentFile)
        {
            qtss_fprintf(currentFile,"u=%s\n",thePick);

            fclose(currentFile);
        }   
    }
}


void MP3Broadcaster::PrintPlaylistElement(PLDoubleLinkedListNode<SimplePlayListElement> *node,void *file)
{   
    sBroadcaster->mElementCount ++;
    if (sBroadcaster->mElementCount <= sBroadcaster->mUpcomingSongsListSize)    
    {   
        char* thePick = node->fElement->mElementName;
        qtss_fprintf((FILE*)file,"%s\n", thePick);
    }
}

void MP3Broadcaster::ShowPlaylistElements(PlaylistPicker *picker,FILE *file)
{
    if (mElementCount > mUpcomingSongsListSize)     
        return;
        
    UInt32  x;
    for (x= 0;x<picker->GetNumBuckets();x++)
    {   
        picker->GetBucket(x)->ForEach(PrintPlaylistElement,file);
    }
}

void MP3Broadcaster::PreFlightOrBroadcast( bool preflight, bool daemonize, bool showMovieList, bool currentMovie, bool checkMP3s, const char* errorlog)
{
    PlaylistPicker*     picker = NULL;
    PlaylistPicker*     insertPicker = NULL;
    MP3FileBroadcaster  fileBroadcaster(&mSocket, mBitRate, mFrequency);
    MP3MetaInfoUpdater* metaInfoUpdater = NULL;
    
    SInt32                moviePlayCount;
    char*               thePick = NULL;
    SInt32                numMovieErrors;
   	bool				didAtLeastOneMoviePlay = false;
    int err;
        
        mPreflight = preflight;
        
    if ( preflight )
        ShowSetupParams();

    if (preflight)
        picker = new PlaylistPicker(false);             // make sequential picker, no looping
    else
    {   
        picker = MakePickerFromConfig(); // make  picker according to parms
        mTempPicker =  new PlaylistPicker(false);
        insertPicker = new PlaylistPicker(false);
        insertPicker->mRemoveFlag = true;
    }
    
    // initial call uses empty string for path, NULL for loop detection list
    (void)PopulatePickerFromFile( picker, mPlayListPath, "", NULL );
    
    if ( preflight )
    {
        if ( picker->mNumToPickFrom == 1 )
            qtss_printf( "\nThere is one movie in the Playlist.\n\n" );
        else
            qtss_printf( "\nThere are (%li) movies in the Playlist.\n\n", (SInt32) picker->mNumToPickFrom );
    }   
    
    if ( picker->mNumToPickFrom == 0 )
    {   
        qtss_printf( "- MP3Broadcaster setup failed: There are no movies to play.\n" );
        mNumErrors++;
        goto bail;
    }
    

    // check that we have enough movies to cover the recent movies list.
    if ( preflight )
    {
        if (  !strcmp( mPlayMode, "weighted_random" ) ) // this implies "random" play
        {   
            if ( mRecentSongsListSize >=  picker->mNumToPickFrom )
            {
                mRecentSongsListSize = picker->mNumToPickFrom - 1;
                qtss_printf("- PlaylistBroadcaster Warning:\n  The recent_movies_list_size setting is greater than \n");
                qtss_printf("  or equal to the number of movies in the playlist.\n" );
                                mNumWarnings++;
            }
        }
    }
    
    // create the log file
    mLog = new MP3BroadcasterLog( mWorkingDirPath,  mLogFile, mLogging );
    
//  if ( !PreflightTrackerFileAccess( R_OK | W_OK ) )
//      goto bail;

    if (!preflight)
    {
        err = ConnectToServer();
        if (err)
        {       
                        if (err == EACCES)
                            qtss_printf("- MP3Broadcaster: Disconnected from Server. Bad password or mount point\n  Exiting.\n" );
                        else
                            qtss_printf("- MP3Broadcaster: Couldn't connect to server\n  Exiting.\n" );
            mNumErrors++;
            goto bail;
        }
    }
    
    //Unless the Debug command line option is set, daemonize the process at this point
    if (daemonize)
    {   
#ifndef __Win32__
        qtss_printf("- MP3Broadcaster: Started in background.\n");
        
        // keep the same working directory..
#ifdef __sgi__
		if (::_daemonize(_DF_NOCHDIR, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO) != 0)
#else
        if (::daemon( 1, 0 ) != 0)
#endif
        {
            qtss_printf("- MP3Broadcaster:  System error (%i).\n", errno);
            mNumErrors++;
            goto bail;
        }

#endif      
    }
    
        if (daemonize && (errorlog != NULL))
    {   
                freopen(errorlog, "a", stdout);
        ::setvbuf(stdout, (char *)NULL, _IONBF, 0);
    }
        
        if (!preflight)
        {
                metaInfoUpdater = new MP3MetaInfoUpdater(mPassword, mMountPoint, mSocket.GetRemoteAddr(), mPort);
                metaInfoUpdater->Start();
                fileBroadcaster.SetInfoUpdater(metaInfoUpdater);
    }
        
        // ^ daemon must be called before we Open the log and tracker since we'll
    // get a new pid, our files close,  ( does SIGTERM get sent? )
    
    if (( mLog ) && ( mLogging ))
        mLog->EnableLog( false ); // don't append ".log" to name for PLB
    
    if ( mLogging && !mLog->IsLogEnabled() )
    {
        if (  mLog->LogDirName() && *mLog->LogDirName() )
            qtss_printf("- MP3Broadcaster: The log file failed to open.\n  ( path: %s/%s )\n  Exiting.\n", mLog->LogDirName(), mLog->LogFileName() );
        else
            qtss_printf("- MP3Broadcaster: The log file failed to open.\n  ( path: %s )\n  Exiting.\n", mLog->LogFileName() );
        
        mNumErrors++;
        goto bail;
    }
    
    
    if (mPIDFile[0] != 0)
    {
        if(!FileCreateAndCheckAccess(mPIDFile))
        {
            FILE *pidFile = fopen(mPIDFile, "w");
            if(pidFile)
            {
                qtss_fprintf(pidFile,"%d\n",getpid());
                fclose(pidFile);
            }   
        }
    }
    
//  AddOurPIDToTracker( bcastSetupFilePath ); // <-- exits on failure

    if ( !preflight )
        mLog->LogInfo( "MP3Broadcaster started." );
    else
        mLog->LogInfo( "MP3Broadcaster preflight started." );

    if(!preflight)
    {   
        CreateCurrentAndUpcomingFiles();
        SendXAudioCastHeaders();
    }
        
    if (!preflight)
    {
        // check the frequency of the first song
        fileBroadcaster.PlaySong( picker->GetFirstFile(), NULL, true, true );
    }
    
    moviePlayCount = 0;
    numMovieErrors = 0;
 	didAtLeastOneMoviePlay = false;
           
    while (true)
    {   
    
        if (!showMovieList && !preflight)
            UpdatePlaylistFiles(picker, insertPicker);
             
        if (NULL != insertPicker)
            thePick = insertPicker->PickOne(); 
            
        if (NULL == thePick && (NULL != picker))
            thePick = picker->PickOne();
                
        if ( (thePick != NULL) && (!preflight && showMovieList) )
        {
            // display the picks in debug mode, but not preflight
            qtss_printf( "[%li] %s picked\n", moviePlayCount, thePick );
        }
        
        
        if ( !showMovieList )
        {
            int playError;
                
            if(!preflight)
            {   UpdateCurrentFile(thePick);
                /* if playlist is about to run out repopulate it */
                if  (   !::strcmp(mPlayMode, "sequential_looped") )
                {   
                    if(NULL == thePick &&!picker->mStopFlag)
                    {   
                        if (picker->GetNumMovies() == 0) 
                            break;
                        else
                            continue;
                    }
                        
                }
            }

            if (thePick == NULL)
                break;
            
            ++moviePlayCount;
            
            if(!preflight)
            {
            
                SInt64 startTime = OS::Milliseconds();
                SInt64 endTime = 0;
                playError = fileBroadcaster.PlaySong( thePick, mCurrentFile );
                endTime = OS::Milliseconds();
                
				//were we able to actually play the movie?
				didAtLeastOneMoviePlay = didAtLeastOneMoviePlay || (playError == 0);
				
				//ok, we've reached the end of the current playlist
				if (picker->GetNumMovies() == 0)
				{
					//If we determine that every one of the movies resulted in an error, then bail
					if (!didAtLeastOneMoviePlay)
					{
						qtss_printf("Quitting:  Playlist contains no valid files.\n");
						mLog->LogInfo( "Quitting:  Playlist contains no valid files.\n" );
						goto bail;
					}
					else
					{
						didAtLeastOneMoviePlay = false;
					}
				}
                
                mLog->LogMediaData(thePick, fileBroadcaster.GetTitle(), 
                                            fileBroadcaster.GetArtist(), 
                                            fileBroadcaster.GetAlbum(),
                                            (UInt32) ((endTime - startTime)/1000L),
                                            playError);
            }
            else
            {
                playError = fileBroadcaster.PlaySong( thePick, NULL, preflight, !checkMP3s );
            }
            
            if (playError == MP3FileBroadcaster::kConnectionError)
            {
                // do something
                mNumErrors++;
                goto bail;
            }
            
            if ( !preflight && (playError != 0))
            {    
                 qtss_printf("File %s : ", thePick);
                 mLog->LogMediaError( thePick, MapErrorToString(playError), NULL );
            }
            else if (playError != 0)
            {
                qtss_printf("File %s : ", thePick);
                MapErrorToString(playError);
                numMovieErrors++;
                mNumWarnings++;
            }
        }
        

        delete [] thePick;
        thePick = NULL;
    } //while (true)
    
    remove(mCurrentFile);
    remove(mUpcomingFile);  

    if ( preflight )
    {
        char    str[256];   
        qtss_printf( " - "  );
        if (numMovieErrors == 1)
            strcpy(str, "MP3Broadcaster found one problem MP3 file.");
        else
            qtss_sprintf( str, "MP3Broadcaster found %li problem MP3 files." , numMovieErrors );
        qtss_printf( "%s\n", str );
        if (mLog) mLog->LogInfo( str );
        
        if (numMovieErrors == moviePlayCount)
        {
            qtss_printf("There are no valid MP3s to play\n");
            mNumErrors++;
        }
    }
    
bail:

    delete picker;

    if (metaInfoUpdater)
        delete metaInfoUpdater;
        
        Cleanup();

#ifndef __Win32__
/*
    if ( sgTrackingSucceeded )
    {
        // remove ourselves from the tracker
        // this is the "normal" remove, also signal handlers
        // may remove us.
        
        BCasterTracker  tracker( sgTrackerFilePath );
        
        tracker.RemoveByProcessID( getpid() );
        tracker.Save();
    }
*/
#endif
}

PlaylistPicker* MP3Broadcaster::MakePickerFromConfig()
{
    // construct a PlaylistPicker object using options set
    
    PlaylistPicker *picker = NULL;
    
    if ( !::strcmp( mPlayMode, "weighted_random" ) )
    {
        picker = new PlaylistPicker( 10, mRecentSongsListSize );
        
    }
    else if ( !::strcmp( mPlayMode, "sequential_looped" ) )
    {           
        picker = new PlaylistPicker(true);
        picker->mRemoveFlag = true;
    }
    else if ( !::strcmp( mPlayMode, "sequential" ) )
    {           
        picker = new PlaylistPicker(false);
        picker->mRemoveFlag = true;
    }
    
    return picker;
}

bool MP3Broadcaster::FileCreateAndCheckAccess(char *theFileName)
{
    FILE *theFile;


#ifndef __Win32__
    if(access(theFileName, F_OK)){
        /* file does not exist  - create and set rights */
        theFile = ::fopen(theFileName, "w+");
        if(theFile){
            /* make sure everybody has r/w access to file */
            (void)::chmod(theFileName, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
            (void)::fclose(theFile);
        }else return 1;
    }else{
        /* file exists - check rights */
        if(access(theFileName, R_OK|W_OK))return 2;
    }

#else

    theFile = ::fopen(theFileName,"a");

    if (theFile) ::fclose(theFile);

#endif


    return 0;
}

int MP3Broadcaster::ConnectToServer()
{
    UInt32 addr;
    addr = SocketUtils::ConvertStringToAddr(mIPAddr);
    if (addr == INADDR_NONE)
    {
        struct hostent* theHostent = ::gethostbyname(mIPAddr);      
        if (theHostent != NULL)
            addr = ntohl(*(UInt32*)(theHostent->h_addr_list[0]));
        else
            qtss_printf("Couldn't resolve address %s\n", mIPAddr);
    }
        
    OS_Error err = mSocket.Open();
    err = mSocket.Connect(addr, mPort);
        
    if (err == 0)
    {
        char buffer1[512];
        char buffer2[512];
        UInt32 len;

        StringTranslator::EncodeURL(mMountPoint, strlen(mMountPoint) + 1, buffer1, sizeof(buffer1));
        if (strlen(buffer1) + strlen(mPassword) + 12 <= 512)
        {
            if (mUseICY)
            {
                // in the ICY protocol there is no mountpoint
                // the reflector assumes a default mountpoint of "/"
                qtss_sprintf(buffer2, "%s\n", mPassword);
            }
            else
            {
                // if the mountpoint does not start with a "/" we prepend one for the
                // reflector before we send the broadcast request.
                if (buffer1[0] == '/')
                    qtss_sprintf(buffer2, "SOURCE %s %s\n\n", mPassword, buffer1);
                else
                    qtss_sprintf(buffer2, "SOURCE %s /%s\n\n", mPassword, buffer1);
            }
            mSocket.Send(buffer2, strlen(buffer2), &len);
        }
        else return -1;
                
                char buffer3[3];
                len = 0;
                mSocket.Read(buffer3, 2, &len);
                buffer3[2] = '\0';
                if (::strcmp(buffer3, "OK") != 0)
                   err = EACCES; 
        }
        
    return err;
}

int MP3Broadcaster::SendXAudioCastHeaders()
{
    char buffer[1024];
    char temp[256];
    UInt32 len;
    
    if (mUseICY)
    {
        qtss_sprintf(buffer, "icy-name:%s\n", mName);
        qtss_sprintf(temp, "icy-genre:%s\n", mGenre);
        strcat(buffer, temp);
        qtss_sprintf(temp, "icy-pub:%s\n", "0");
        strcat(buffer, temp);
        qtss_sprintf(temp, "icy-url:%s\n", mURL);
        strcat(buffer, temp);
    }
    else
    {
        qtss_sprintf(buffer, "x-audiocast-name:%s\n", mName);
        qtss_sprintf(temp, "x-audiocast-genre:%s\n", mGenre);
        strcat(buffer, temp);
        qtss_sprintf(temp, "x-audiocast-public:%s\n", "0");
        strcat(buffer, temp);
        qtss_sprintf(temp, "x-audiocast-description:%s\n", "");
        strcat(buffer, temp);
    }    
    mSocket.Send(buffer, strlen(buffer), &len);
    
    return 0;
}

void MP3Broadcaster::ShowSetupParams()
{
    qtss_printf( "\n" );
    qtss_printf( "Configuration Settings\n" );
    qtss_printf( "----------------------------\n" );
    qtss_printf( "Destination address %s:%d\n", mIPAddr, mPort );
    qtss_printf( "MP3 bitrate %d\n", mBitRate );
    qtss_printf( "play_mode  %s\n", mPlayMode );
    qtss_printf( "recent_movies_list_size  %d\n", mRecentSongsListSize );
    qtss_printf( "playlist_file  %s\n", mPlayListPath );
    qtss_printf( "working_dir  %s\n", mWorkingDirPath );
    qtss_printf( "logging  %d\n", mLogging );
    qtss_printf( "log_file  %s\n", mLogFile );
    qtss_printf( "max_upcoming_list_size  %d\n", mUpcomingSongsListSize );
    qtss_printf( "show_current  %d\n", mShowCurrent );
    qtss_printf( "show_upcoming  %d\n", mShowUpcoming );
    qtss_printf( "use_icy  %d\n", mUseICY );
    qtss_printf( "broadcast_name \"%s\"\n", mName);
    qtss_printf( "broadcast_genre \"%s\"\n", mGenre);
    qtss_printf( "broadcast_mount_point \"%s\"\n", mMountPoint);
    qtss_printf( "broadcast_password \"XXXXX\"\n");
    qtss_printf( "max_err_file_k_size %"_U32BITARG_"\n", qtss_getmaxprintfcharsinK());
    qtss_printf( "\n" );
}

void MP3Broadcaster::RemoveFiles()
{
    if (mPIDFile[0] != 0)
    {
        remove(mPIDFile);
    }

    remove(mStopFile);
    remove(mReplaceFile);
    remove(mInsertFile);
    remove(mCurrentFile);
    remove(mUpcomingFile);  
}

char* MP3Broadcaster::MapErrorToString(int error)
{
    char* result = NULL;
        
    if (error == MP3FileBroadcaster::kBadFileFormat)
        result = "Bad file format.";
    else if (error == MP3FileBroadcaster::kWrongFrequency)
        result = "Encoded at wrong frequency.";
    else if (error == MP3FileBroadcaster::kWrongBitRate)
        result = "Doesn't use desired bit rate.";
    else if (error == MP3FileBroadcaster::kCouldntOpenFile)
        result = "Couldn't open file.";
        
    if (result != NULL)
        qtss_printf("%s\n", result);
    
    return result;
}

void MP3Broadcaster::Cleanup(bool signalHandler)
{
        if (mCleanupDone)
            return;
            
        mCleanupDone = true;
        
        if (signalHandler)
        {
            mNumErrors++;
            qtss_printf("- MP3Broadcaster: Disconnected from Server while playing. Exiting.\n");
        }
        
        if (mPreflight)
    {
        qtss_printf("Warnings: %"_S32BITARG_"\n", mNumWarnings);
        qtss_printf("Errors: %"_S32BITARG_"\n", mNumErrors);
    }
        else
        {
            qtss_printf("Broadcast Warnings: %"_S32BITARG_"\n", mNumWarnings);
            qtss_printf("Broadcast Errors: %"_S32BITARG_"\n", mNumErrors);
        }
    
    RemoveFiles();
        
        if (mLog)
        {
            if ( mPreflight )
                mLog->LogInfo( "MP3Broadcaster preflight finished." );
            else
                mLog->LogInfo( "MP3Broadcaster finished." );
    }
        
        //  mLog = NULL; // protect the interrupt handler and just let it die don't delete because it is a task thread
}
