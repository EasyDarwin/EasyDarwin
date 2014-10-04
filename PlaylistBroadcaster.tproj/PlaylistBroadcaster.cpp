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
    8.11.99 rt  - changed references to "PlaylistBroadcaster Setup" to Broadcast Description File

    8.4.99 rt   - changed references to "PlaylistBroadcaster Description" to Broadcast Setup File
                - addded error messages
                - prefilght config file access
                - require log file creation
                
                
    7.27.99 rt  - removed license from about display
                - updated credit names
                - fixed mapping of --stop to 's' from 'l'
                - added about to help
                

    8.2.99 rt   - changed reference to "channel setup" to "PlaylistBroadcaster Description"
                - changed &d's in qtss_printf's to %d
                
*/



#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include <signal.h>
#ifndef __MacOSX__
#include "getopt.h"
#endif

#ifndef __Win32__
    #if defined (__solaris__) || defined (__osf__) || defined (__sgi__) || defined (__hpux__)
        #include "daemon.h"
    #else
        #ifndef __FreeBSD__
            #include <sys/sysctl.h>
        #endif
    #endif
#endif

#ifndef kVersionString
#include "revision.h"
#endif

#include <errno.h>
#include "QTRTPFile.h"

#include "OSHeaders.h"
#include "OS.h"
#include "OSMemory.h"
#include "SocketUtils.h"
#include "Socket.h"
#include "Task.h"
#include "TimeoutTask.h"
#include "BroadcasterSession.h"
#include "OSArrayObjectDeleter.h"
#include "PickerFromFile.h"
#include "PLBroadcastDef.h"
#include "BroadcastLog.h"
#include "StringTranslator.h"

#ifndef __Win32__
    #include "BCasterTracker.h"
#endif

#ifdef __Win32__
    #include "getopt.h"
#endif  

// must now inlcude this from the project level using the -i switch in the compiler
#ifndef __MacOSX__
    #include "revision.h"
#endif

#ifdef __MacOSX__
    #include <sys/stat.h>
#endif

#include "playlist_SDPGen.h"
#include "playlist_broadcaster.h"


#include "MyAssert.h"

/*
    local functions
*/

static void usage();
static void version();
static void help();
static PlaylistPicker*  MakePickerFromConfig( PLBroadcastDef* broadcastParms );
static void SignalEventHandler( int signalID );
static int EvalBroadcasterErr(int err);
static const char* GetMovieFileErrString( int err );

static void RegisterEventHandlers();
static void ShowPlaylistStatus();
static void StopABroadcast( const char* arg );
static bool DoSDPGen( PLBroadcastDef *broadcastParms, bool preflight, bool overWriteSDP, bool generateNewSDP, int* numErrorsPtr, char* refMovie);

static bool AddOurPIDToTracker( const char* bcastSetupFilePath );

static void Cleanup();

/* -- preflighting --*/

static int MyAccess( const char* path, int mode );
static int PreflightSDPFileAccess( const char * sdpPath );
static int PreflightDestinationAddress( const char *destAddress );
static int PreflightBasePort( const char *basePort );
static int PreflightReferenceMovieAccess( const char *refMoviePath );
static bool PreflightTrackerFileAccess( int mode );
static bool PreFlightSetupFile( const char * bcastSetupFilePath );
static int PreflightClientBufferDelay( const char * delay, Float32 *ioDelay);
static void ShowSetupParams(PLBroadcastDef* broadcastParms, const char *file);
static int PreflightDestSDPFileAccess( const char * sdpPath );
static void PreFlightOrBroadcast( const char *bcastSetupFilePath, bool preflight, bool daemonize, bool showMovieList, bool writeCurrentMovieFile, char *destinationIP,bool writeNewSDP, const char* errorFilePath);

/* changed by emil@popwire.com */
static bool FileCreateAndCheckAccess(char *theFileName);
static void PrintPlaylistElement(PLDoubleLinkedListNode<SimplePlayListElement> *node,void *file);
static void ShowPlaylistElements(PlaylistPicker *picker,FILE *file);
static void RemoveFiles(PLBroadcastDef* broadcastParms);
/* ***************************************************** */

/*
    local variables
*/
static char*                sgProgramName; // the actual program name as input at commmand line
static char*                sgTrackerDirPath = "/var/run";
static char*                sgTrackerFilePath = "/var/run/broadcastlist";
static BroadcastLog*        sgLogger = NULL;
static bool             sgTrackingSucceeded = false;
static BroadcasterSession* sBroadcasterSession = NULL;
static  StrPtrLen           sSetupDirPath;
static  PlaylistPicker*     sTempPicker = NULL;
static SInt32               sElementCount = 0;
static SInt32               sMaxUpcomingListSize = 5;
static PLBroadcastDef*      sBroadcastParms = NULL;

enum {maxSDPbuffSize = 10000};
static	char	sSDPBuffer[maxSDPbuffSize] = {0};
static	bool	sQuitImmediate = false;
static  bool    sAnnounceBroadcast = false;
static  bool    sPushOverTCP = false;
static  int		sRTSPClientError = 0;
static  bool    sErrorEvaluted = false;
static  bool    sDeepDebug = false;

static int  sNumWarnings = 0;
static int  sNumErrors = 0;
static bool sPreflight = false;
static bool sCleanupDone = false;
static bool sBurst = false;

//+ main is getting too big.. need to clean up and separate the command actions 
// into a separate file.


int main(int argc, char *argv[]) {

    char    *bcastSetupFilePath = NULL;
    bool    daemonize = true;
    int anOption = 0;
    bool    preflight = false;
    bool    showMovieList = false;
    bool    writeCurrentMovieFile = false;
    int displayedOptions = 0; // count number of command line options we displayed
    bool    needsTracker = false;   // set to true when PLB needs tracker access
    bool    needsLogfile = false;   // set to true when PLB needs tracker access
    char*   destinationIP = NULL;
    bool    writeNewSDP = false;
    char*   errorlog = NULL;    
    extern char* optarg;
    extern int optind;

#ifdef __Win32__        
    //
    // Start Win32 DLLs
    WORD wsVersion = MAKEWORD(1, 1);
    WSADATA wsData;
    (void)::WSAStartup(wsVersion, &wsData);
#endif


    QTRTPFile::Initialize(); // do only once
    OS::Initialize();
    OSThread::Initialize();
    OSMemory::SetMemoryError(ENOMEM);
    Socket::Initialize();
    SocketUtils::Initialize(false);
    
    if (SocketUtils::GetNumIPAddrs() == 0)
    {
        qtss_printf("Network initialization failed. IP must be enabled to run PlaylistBroadcaster\n");
        ::exit(0);
    }

#ifdef __MacOSX__
    (void) ::umask(S_IWOTH); // make sure files are opened with default of owner -rw-rw-r-
#endif


    sgProgramName = argv[0];
#ifdef __Win32__
    while ((anOption = getopt(argc, argv, "vhdcpbDtai:fe:" )) != EOF)
#else
    while ((anOption = getopt(argc, argv, "vhdcpbDls:tai:fe:" )) != EOF)
#endif
    {
        
        switch(anOption)
        {
            case 'b':
                sBurst = true;
                break;
                
            case 'v':
                ::version();
                ::usage();
                return 0;

            case 'h':
                ::help();
                displayedOptions++;
                break;
            
            case 'd':
                daemonize = false;
                break;
                                    
            case 'c':
                writeCurrentMovieFile = true;
                break;              
                
/*
            case 'm':
                showMovieList = true;
                daemonize = false;
                break;
*/
                            
            case 'p':
                preflight = true;
                daemonize = false;
                needsTracker = true;
                needsLogfile = true;
                break;
                
            case 'D':
                sDeepDebug = true;
                daemonize = false;
                break;

#ifndef __Win32__

            
            case 'l':
                // show playlist broadcast status           
                if (!PreflightTrackerFileAccess( R_OK ))    // check for read access. 
                    ::exit(-1);
                ShowPlaylistStatus();               // <- exits() on failure
                displayedOptions++;
                break;
                
            case 's':               
                // stop a playlist broadcast            
                // is there one to kill?
                if (!PreflightTrackerFileAccess( R_OK | W_OK ))     // check for read access. 
                    ::exit(-1);
                StopABroadcast( optarg );                   // <- exits() on failure
                displayedOptions++;
                break;
#endif
            
            case 't':
                sAnnounceBroadcast = true;
                sPushOverTCP = true;
                break;

            case 'a':
                sAnnounceBroadcast = true;
                break;              
            
            case 'i':
                destinationIP = (char*)malloc(strlen(optarg)+1);
                strcpy(destinationIP, optarg);
                qtss_printf("destinationIP =%s\n",destinationIP);
                break;
                
            case 'f':
                writeNewSDP = true;
                break;
                
            case 'e':
                errorlog = (char*)malloc(strlen(optarg)+1);
                strcpy(errorlog, optarg);
                break;
                
            default:
                ::usage();
                ::exit(-1);
        }
    }

    if (argv[optind] != NULL)
    {
        bcastSetupFilePath = (char*)malloc(strlen(argv[optind])+1);
        strcpy(bcastSetupFilePath, argv[optind]);
    }

	sPreflight = preflight;
         
    if (errorlog != NULL)
    {
            if (preflight)
                    freopen(errorlog, "w", stdout);
        else
                freopen(errorlog, "a", stdout);
        ::setvbuf(stdout, (char *)NULL, _IONBF, 0);
    }
        
    // preflight needs a description file
    if ( preflight && !bcastSetupFilePath )
    {   qtss_printf("- PlaylistBroadcaster: Error.  \"Preflight\" requires a broadcast description file.\n" );
        ::usage();
        ::exit(-1);
    }

    // don't complain about lack of description File if we were asked to display some options.  
    if ( displayedOptions > 0 && !bcastSetupFilePath )
        ::exit(0);      // <-- we displayed option info, but have no description file to broadcast
        
    PreFlightOrBroadcast( bcastSetupFilePath, preflight, daemonize, showMovieList, writeCurrentMovieFile,destinationIP,writeNewSDP, errorlog ); // <- exits() on failure -- NOT ANYMORE
    
    return 0;

}

BroadcasterSession *StartBroadcastRTSPSession(PLBroadcastDef *broadcastParms)
{   
    TaskThreadPool::AddThreads(1);
    TimeoutTask::Initialize();
    Socket::StartThread();

    UInt32 inAddr = SocketUtils::ConvertStringToAddr(broadcastParms->mDestAddress);
    
    UInt16 inPort = atoi(broadcastParms->mRTSPPort);
    char * theURL = new char[512];
    StringTranslator::EncodeURL(broadcastParms->mDestSDPFile, strlen(broadcastParms->mDestSDPFile) + 1, theURL, 512);
    BroadcasterSession::BroadcasterType inBroadcasterType;
        
    if (sPushOverTCP)
         inBroadcasterType = BroadcasterSession::kRTSPTCPBroadcasterType;
    else
         inBroadcasterType = BroadcasterSession::kRTSPUDPBroadcasterType;
    
    UInt32 inDurationInSec = 0;
    UInt32 inStartPlayTimeInSec = 0;
    UInt32 inRTCPIntervalInSec = 5; 
    UInt32 inOptionsIntervalInSec = 0;
    UInt32 inHTTPCookie = 1; 
    Bool16 inAppendJunkData = false; 
    UInt32 inReadInterval = 50;
    UInt32 inSockRcvBufSize = 32768;
    StrPtrLen sdpSPL(sSDPBuffer,maxSDPbuffSize);
    sRTSPClientError = QTFileBroadcaster::eNetworkConnectionError;
    sBroadcasterSession = NEW BroadcasterSession(inAddr, 
                            inPort,                 
                            theURL,
                            inBroadcasterType,      // Client type
                            inDurationInSec,        // Movie length
                            inStartPlayTimeInSec,                   // Movie start time
                            inRTCPIntervalInSec,                    // RTCP interval
                            inOptionsIntervalInSec,                 // Options interval
                            inHTTPCookie,   // HTTP cookie
                            inAppendJunkData,
                            inReadInterval, // Interval between data reads  
                            inSockRcvBufSize,
                            &sdpSPL,
                            broadcastParms->mName,
                            broadcastParms->mPassword,
                            sDeepDebug,
                            sBurst);    

    return  sBroadcasterSession;
}

Bool16 AnnounceBroadcast(PLBroadcastDef *broadcastParms,QTFileBroadcaster   *fileBroadcasterPtr)
{   // return true if no Announce required or broadcast is ok.

    if (!sAnnounceBroadcast) return true;

	// if the address is a multicast address then we can't announce the broadcast.
    
    if(SocketUtils::IsMulticastIPAddr(ntohl(inet_addr(broadcastParms->mDestAddress)))) {
        sAnnounceBroadcast = false;
        return true;
    }
    
#if !MACOSXEVENTQUEUE
    ::select_startevents();//initialize the select() implementation of the event queue
#endif

    if (SocketUtils::GetNumIPAddrs() == 0)
    {
        qtss_printf("IP must be enabled to run PlaylistBroadcaster\n");
        //::exit(0); // why exit here? If we return false here, the calling function will take care of the error
                return false;
    }

    broadcastParms->mTheSession = sBroadcasterSession = StartBroadcastRTSPSession(broadcastParms);
    while (!sBroadcasterSession->IsDone() && BroadcasterSession::kBroadcasting != sBroadcasterSession->GetState()) 
    {   OSThread::Sleep(100);
    }
    sRTSPClientError = 0;
    int broadcastErr = sBroadcasterSession->GetRequestStatus();
    
    Bool16 isOK = false;    
    if (broadcastErr != 200) do 
    {
        if (412 == broadcastErr)
        {   
            ::EvalBroadcasterErr(broadcastErr);
            break;  
        }
        
        if (200 != broadcastErr)
        {   //qtss_printf("broadcastErr = %"_S32BITARG_" sBroadcasterSession->GetDeathState()=%"_S32BITARG_" sBroadcasterSession->GetReasonForDying()=%"_S32BITARG_"\n",broadcastErr,sBroadcasterSession->GetDeathState(),sBroadcasterSession->GetReasonForDying());
            if (sBroadcasterSession->GetDeathState() == BroadcasterSession::kSendingAnnounce && sBroadcasterSession->GetReasonForDying() == BroadcasterSession::kConnectionFailed)
                ::EvalBroadcasterErr(QTFileBroadcaster::eNetworkConnectionError);
            else if (sBroadcasterSession->GetDeathState() == BroadcasterSession::kSendingAnnounce && sBroadcasterSession->GetReasonForDying() == BroadcasterSession::kBadSDP)
                 ::EvalBroadcasterErr(QTFileBroadcaster::eSDPFileInvalid);
            else if (401 == broadcastErr)
                ::EvalBroadcasterErr(QTFileBroadcaster::eNetworkAuthorization);
            else if ((500 == broadcastErr) || (400 == broadcastErr) )
                ::EvalBroadcasterErr(QTFileBroadcaster::eNetworkNotSupported);
            else 
                ::EvalBroadcasterErr(QTFileBroadcaster::eNetworkRequestError);

            break;
        }   
        
        if (sBroadcasterSession != NULL && BroadcasterSession::kDiedNormally != sBroadcasterSession->GetReasonForDying())
        {   ::EvalBroadcasterErr(QTFileBroadcaster::eNetworkRequestError);
            break;
        }   
    
        isOK = true;
        
    } while (false);
    else 
        isOK = true;
        
    if (isOK)
    {
        broadcastErr = fileBroadcasterPtr->SetUp(broadcastParms, &sQuitImmediate);
        if (  broadcastErr )
        {   ::EvalBroadcasterErr(broadcastErr);
            qtss_printf( "- Broadcaster setup failed.\n" );
            isOK = false;
            if (sBroadcasterSession != NULL && !sBroadcasterSession->IsDone())
                sBroadcasterSession->TearDownNow();
        }
    }
    
    return isOK;
}

char* GetBroadcastDirPath(const char * setupFilePath)
{
    int len = 2;
    
    if (setupFilePath != NULL)
        len = ::strlen(setupFilePath);

    char *setupDirPath = new char [ len + 1 ];
    
    if ( setupDirPath )
    {   
        strcpy( setupDirPath, setupFilePath );
        char* endOfDirPath = strrchr( setupDirPath, kPathDelimiterChar );
        
        if ( endOfDirPath )
        {
            endOfDirPath++;
            *endOfDirPath= 0;           
            
            int  chDirErr = ::chdir( setupDirPath );            
            if ( chDirErr )
                chDirErr = errno;
                
            //qtss_printf("- PLB DEBUG MSG: setupDirPath==%s\n  chdir err: %i\n", setupDirPath, chDirErr);         
        }
        else
        {   
            setupDirPath[0] = '.';
            setupDirPath[1] = kPathDelimiterChar;
            setupDirPath[2] = 0;
        }
            
    }
    
    //qtss_printf("GetBroadcastDirPath setupDirPath = %s\n",setupDirPath);
    return setupDirPath;
}

void CreateCurrentAndUpcomingFiles(PLBroadcastDef* broadcastParms)
{
    if (!::strcmp(broadcastParms->mShowCurrent, "enabled")) 
    {   if(FileCreateAndCheckAccess(broadcastParms->mCurrentFile))
        {   /* error */
            sgLogger->LogInfo( "PlaylistBroadcaster Error: Failed to create current broadcast file" );
        }
    }


    if (!::strcmp(broadcastParms->mShowUpcoming, "enabled")) 
    {   if(FileCreateAndCheckAccess(broadcastParms->mUpcomingFile))
        {   /* error */
            sgLogger->LogInfo( "PlaylistBroadcaster Error:  Failed to create upcoming broadcast file" );
        }
    }

}

void UpdatePlaylistFiles(PLBroadcastDef* broadcastParms, PlaylistPicker *picker,PlaylistPicker *insertPicker)
{
    if (    (NULL == broadcastParms)
         ||  (NULL == picker)
        ||  (NULL == insertPicker)
        ) return;
        
     if(!access(broadcastParms->mStopFile, R_OK))
    {
        picker->CleanList();
        PopulatePickerFromFile(picker, broadcastParms->mStopFile, "", NULL);

        sTempPicker->CleanList();

        remove(broadcastParms->mStopFile);
        picker->mStopFlag = true;
    }

    /* if .replacelist file exists - replace current playlist */
    if(!access(broadcastParms->mReplaceFile, R_OK))
    {
        picker->CleanList();     
        PopulatePickerFromFile(picker, broadcastParms->mReplaceFile, "", NULL);
        
        sTempPicker->CleanList();
        
        remove(broadcastParms->mReplaceFile);
        picker->mStopFlag = false;
    }

    /* if .insertlist file exists - insert into current playlist */
    if(!access(broadcastParms->mInsertFile, R_OK))
    {
        insertPicker->CleanList();
        sTempPicker->CleanList();

        PopulatePickerFromFile(insertPicker, broadcastParms->mInsertFile, "", NULL);
        remove(broadcastParms->mInsertFile);
        picker->mStopFlag = false;
    }


                // write upcoming playlist to .upcoming file 
    if (!::strcmp(broadcastParms->mShowUpcoming, "enabled")) 
    {
        FILE *upcomingFile = fopen(broadcastParms->mUpcomingFile, "w");
        if(upcomingFile)
        {
            sElementCount = 0;
        
            if (!::strcmp(broadcastParms->mPlayMode, "weighted_random")) 
                qtss_fprintf(upcomingFile,"#random play - upcoming list not supported\n");
            else
            {   qtss_fprintf(upcomingFile,"*PLAY-LIST*\n");
                ShowPlaylistElements(insertPicker,upcomingFile);
                ShowPlaylistElements(picker,upcomingFile);
                if (    picker->GetNumMovies() == 0 
                        && !picker->mStopFlag 
                        && 0 != ::strcmp(broadcastParms->mPlayMode, "sequential") 
                    )
                {   picker->CleanList();
                    PopulatePickerFromFile(picker,broadcastParms->mPlayListFile,"",NULL);
                    ShowPlaylistElements(picker,upcomingFile);
                    sTempPicker->CleanList();
                    PopulatePickerFromFile(sTempPicker,broadcastParms->mPlayListFile,"",NULL);
                }
                
                if  (   sElementCount <= sMaxUpcomingListSize 
                        && 0 == ::strcmp(broadcastParms->mPlayMode, "sequential_looped")
                    )
                {   if (sTempPicker->GetNumMovies() == 0)
                    {   sTempPicker->CleanList();
                        PopulatePickerFromFile(sTempPicker,broadcastParms->mPlayListFile,"",NULL);
                    }
                    //sElementCount can be zero if the playlist contains no paths to valid files
                    while ( (sElementCount != 0) && sElementCount <= sMaxUpcomingListSize )
                        ShowPlaylistElements(sTempPicker,upcomingFile);
                }
            }
            fclose(upcomingFile);
        }   
    }
    else
    {   
        if (    picker->GetNumMovies() == 0 
                && !picker->mStopFlag 
                && ::strcmp(broadcastParms->mPlayMode, "sequential") 
            )
        {   picker->CleanList();
            PopulatePickerFromFile(picker,broadcastParms->mPlayListFile,"",NULL);
        }       
    }       
        
}


void UpdateCurrentFile(PLBroadcastDef* broadcastParms, char *thePick)
{
    if ( (NULL == broadcastParms) || (NULL == thePick) ) 
        return;
        
    // save currently playing song to .current file 
    if (!::strcmp(broadcastParms->mShowCurrent, "enabled")) 
    {   FILE *currentFile = fopen(broadcastParms->mCurrentFile, "w");
        if(currentFile)
        {
            if (sSetupDirPath.EqualIgnoreCase(thePick, sSetupDirPath.Len) || '\\' == thePick[0] || '/' == thePick[0])
                qtss_fprintf(currentFile,"u=%s\n",thePick);
            else
                qtss_fprintf(currentFile,"u=%s%s\n", sSetupDirPath.Ptr,thePick);

            fclose(currentFile);
        }   
    }
                        

    
}


static void PreFlightOrBroadcast( const char *bcastSetupFilePath, bool preflight, bool daemonize, bool showMovieList, bool writeCurrentMovieFile, char *destinationIP,bool writeNewSDP, const char* errorFilePath)
{
    PLBroadcastDef*     broadcastParms = NULL;
    PlaylistPicker*     picker = NULL;
    PlaylistPicker*     insertPicker = NULL;
    
    QTFileBroadcaster   fileBroadcaster;
    int     broadcastErr = 0;
    SInt32    moviePlayCount;
    char*   thePick = NULL;
    int     numMovieErrors;
	bool	didAtLeastOneMoviePlay = false;
    bool    sdpFileCreated = false;
    char    *allocatedIPStr = NULL;
    bool    generateNewSDP = false;
    int     numErrorsBeforeSDPGen = 0;
    char    theUserAgentStr[128];
    char*   thePLBStr = "PlaylistBroadcaster";
       
    RegisterEventHandlers();

    if ( !PreFlightSetupFile( bcastSetupFilePath ) )    // returns true on success and false on failure
        {
            sNumErrors++;
            goto bail;
        }
        
    if (destinationIP != NULL)
    { 
        allocatedIPStr = new char[strlen(destinationIP)+1];
        strcpy(allocatedIPStr,destinationIP);
        generateNewSDP = true;
    }
		
    broadcastParms = new PLBroadcastDef( bcastSetupFilePath, allocatedIPStr,  sDeepDebug);  
    sBroadcastParms = broadcastParms;
    
    if( !broadcastParms )
    {   
        qtss_printf("- PlaylistBroadcaster: Error. Out of memory.\n" );
        sNumErrors++;
        goto bail;
    }

    
    if ( !broadcastParms->ParamsAreValid() )
    {   
        qtss_printf("- PlaylistBroadcaster: Error reading the broadcast description file \"%s\". (bad format or missing file)\n", bcastSetupFilePath );
        broadcastParms->ShowErrorParams();
        sNumErrors++;
        goto bail;
    }
    
    
    if ( preflight || sDeepDebug)
        ShowSetupParams(broadcastParms, bcastSetupFilePath);

    if (NULL == GetBroadcastDirPath(bcastSetupFilePath))
    {   
        qtss_printf("- PlaylistBroadcaster: Error. Out of memory.\n" );
        sNumErrors++;
        goto bail;
    }
    sSetupDirPath.Set(GetBroadcastDirPath(bcastSetupFilePath), strlen(GetBroadcastDirPath(bcastSetupFilePath)));

    if (preflight)
    {
        picker = new PlaylistPicker(false);             // make sequential picker, no looping
    }
    else
    {   
        picker = MakePickerFromConfig( broadcastParms ); // make  picker according to parms
        sTempPicker =  new PlaylistPicker(false);
        insertPicker = new PlaylistPicker(false);
        insertPicker->mRemoveFlag = true;
    }
    
    if ( !picker )
    {   
        qtss_printf("- PlaylistBroadcaster: Error. Out of memory.\n" );
        sNumErrors++;
        goto bail;
    }
    
    if ( preflight || sDeepDebug)
        qtss_printf("\n");
    
    Assert( broadcastParms->mPlayListFile );
    if ( broadcastParms->mPlayListFile )
    {   
        char*   fileName;
        
        fileName = broadcastParms->mPlayListFile;
        // initial call uses empty string for path, NULL for loop detection list
        (void)PopulatePickerFromFile( picker, fileName, "", NULL );
        
        // ignore errors, if we have movies in the list, play them
    }
    
    if ( preflight )
    {
        if ( picker->mNumToPickFrom == 1 )
            qtss_printf( "\nThere is (%li) movie in the Playlist.\n\n", (SInt32) picker->mNumToPickFrom );
        else
            qtss_printf( "\nThere are (%li) movies in the Playlist.\n\n", (SInt32) picker->mNumToPickFrom );
    }   
    
    if ( !picker->mNumToPickFrom )
    {   
        qtss_printf( "- PlaylistBroadcaster setup failed: There are no movies to play.\n" );
        sNumErrors++;
        goto bail;
    }
    

    // check that we have enough movies to cover the recent movies list.
    if ( preflight && broadcastParms->ParamsAreValid() )
    {
        if (  !strcmp( broadcastParms->mPlayMode, "weighted_random" ) ) // this implies "random" play
        {   
            if ( broadcastParms->mLimitPlayQueueLength >=  picker->mNumToPickFrom )
            {
                broadcastParms->mLimitPlayQueueLength = picker->mNumToPickFrom-1;
                qtss_printf("- PlaylistBroadcaster Warning:\n  The recent_movies_list_size setting is greater than \n");
                qtss_printf("  or equal to the number of movies in the playlist.\n" );
            }
        }
    }
    
    // create the log file
    sgLogger = new BroadcastLog( broadcastParms, &sSetupDirPath );
    
    
    Assert( sgLogger != NULL );
    
    if( sgLogger == NULL )
    {   
        qtss_printf("- PlaylistBroadcaster: Error. Out of memory.\n" );
        sNumErrors++;
        goto bail;
    }
    
    numErrorsBeforeSDPGen = sNumErrors;
    sdpFileCreated = DoSDPGen( broadcastParms, preflight, writeNewSDP,generateNewSDP, &sNumErrors, picker->GetFirstFile());
    if( sNumErrors > numErrorsBeforeSDPGen )
        goto bail;
            
    if (!sAnnounceBroadcast)
        broadcastErr = fileBroadcaster.SetUp(broadcastParms, &sQuitImmediate);
    
    if (  broadcastErr )
    {   
        ::EvalBroadcasterErr(broadcastErr);
        qtss_printf( "- Broadcaster setup failed.\n" );
        sNumErrors++;
        goto bail;
    }
    
    if (preflight)
    {    
        fileBroadcaster.fPlay = false;
    }
    if ( !PreflightTrackerFileAccess( R_OK | W_OK ) )
    {
        sNumErrors++;
        goto bail;
    }
       
    //Unless the Debug command line option is set, daemonize the process at this point
    if (daemonize)
    {   
#ifndef __Win32__
        qtss_printf("- PlaylistBroadcaster: Started in background.\n");
        
        // keep the same working directory..
        if (::daemon( 1, 0 ) != 0)
        {
            qtss_printf("- PlaylistBroadcaster:  System error (%i).\n", errno);
            goto bail;
        }

#endif  //__Win32__
    }
        
        // If daemonize, then reopen stdout to the error file 
        if (daemonize && (errorFilePath != NULL))
        {
                freopen(errorFilePath, "a", stdout);
                ::setvbuf(stdout, (char *)NULL, _IONBF, 0);
        }
        

#ifndef __Win32__
    qtss_snprintf(theUserAgentStr, ::strlen(thePLBStr) + 1 + ::strlen(kVersionString) + 1, "%s/%s", thePLBStr, kVersionString);
#else
    _snprintf(theUserAgentStr, ::strlen(thePLBStr) + 1 + ::strlen(kVersionString) + 1, "%s/%s", thePLBStr, kVersionString);
#endif
    RTSPClient::SetUserAgentStr(theUserAgentStr);
        
        if (!preflight && !AnnounceBroadcast(broadcastParms,&fileBroadcaster)) 
    {
        sNumErrors++;
        goto bail;
    }
        
        // ^ daemon must be called before we Open the log and tracker since we'll
    // get a new pid, our files close,  ( does SIGTERM get sent? )
    
    if (( sgLogger != NULL ) && ( sgLogger->WantsLogging() ))
        sgLogger->EnableLog( false ); // don't append ".log" to name for PLB
    
    if ( sgLogger->WantsLogging() && !sgLogger->IsLogEnabled() )
    {
        if (  sgLogger->LogDirName() && *sgLogger->LogDirName() )
            qtss_printf("- PlaylistBroadcaster: The log file failed to open.\n  ( path: %s/%s )\n  Exiting.\n", sgLogger->LogDirName(), sgLogger->LogFileName() );
        else
            qtss_printf("- PlaylistBroadcaster: The log file failed to open.\n  ( path: %s )\n  Exiting.\n", sgLogger->LogFileName() );
        
        sNumErrors++;
        goto bail;
    }
    
    
    if (broadcastParms->mPIDFile != NULL)
    {
        if(!FileCreateAndCheckAccess(broadcastParms->mPIDFile))
        {
            FILE *pidFile = fopen(broadcastParms->mPIDFile, "w");
            if(pidFile)
            {
                qtss_fprintf(pidFile,"%d\n",getpid());
                fclose(pidFile);
            }   
        }
    }
    else if ( !AddOurPIDToTracker( bcastSetupFilePath ) ) // <-- doesn't exit on failure anymore - returns false if failed
        {
            // writes to the broadcast list file only if the pid_file config param doesn't exist
            sNumErrors++;
            goto bail;
        }
        
    if ( !preflight )
        sgLogger->LogInfo( "PlaylistBroadcaster started." );
    else
        sgLogger->LogInfo( "PlaylistBroadcaster preflight started." );

    //+ make the RTP movie broadcaster
    
    if ( !preflight )
    {
        qtss_printf( "\n" );
        qtss_printf( "[pick#] movie path\n" );
        qtss_printf( "----------------------------\n" );
    }
    else
    {       
        qtss_printf( "\n" );
        qtss_printf( "Problems found\n" );
        qtss_printf( "--------------\n" );
    }

    if(!preflight)
        CreateCurrentAndUpcomingFiles(broadcastParms);
    
    moviePlayCount = 0;
    numMovieErrors = 0;
	didAtLeastOneMoviePlay = false;
    sMaxUpcomingListSize = ::atoi( broadcastParms->mMaxUpcomingMovieListSize );
            
    while (true)
    {
    
        if (!showMovieList && !preflight)
        {
            UpdatePlaylistFiles(broadcastParms,  picker, insertPicker);
        }
       
        if (NULL != insertPicker)
            thePick = insertPicker->PickOne(); 
            
        if (NULL == thePick && (NULL != picker))
            thePick = picker->PickOne();
        
        
        if ( (thePick != NULL) && (!preflight || showMovieList) )
        {
            // display the picks in debug mode, but not preflight
            qtss_printf( "[%li] ", moviePlayCount );
            {   
                if (sSetupDirPath.EqualIgnoreCase(thePick, sSetupDirPath.Len) || '\\' == thePick[0] || '/' == thePick[0])
                    qtss_printf("%s picked\n", thePick);
                else
                    qtss_printf("%s%s picked\n", sSetupDirPath.Ptr,thePick);
            }

        }
        
        
        if ( !showMovieList )
        {
            int playError;
                
            if(!preflight)
            {   UpdateCurrentFile(broadcastParms, thePick);
                
                /* if playlist is about to run out repopulate it */
                if  (   !::strcmp(broadcastParms->mPlayMode, "sequential_looped") )
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
                playError = fileBroadcaster.PlayMovie( thePick, broadcastParms->mCurrentFile );
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
						sgLogger->LogInfo( "Quitting:  Playlist contains no valid files.\n" );
              		  	goto bail;
					}
					else
					{
						didAtLeastOneMoviePlay = false;
					}
				}
		
                // log the result of broacasting the picked movie
                sgLogger->LogMediaData( thePick, 
                                        fileBroadcaster.fCurrentMovieName, 
                                        fileBroadcaster.fCurrentMovieCopyright, 
                                        fileBroadcaster.fCurrentMovieComment, 
                                        fileBroadcaster.fCurrentMovieAuthor, 
                                        fileBroadcaster.fCurrentMovieArtist, 
                                        fileBroadcaster.fCurrentMovieAlbum, 
                                        (UInt32) ((endTime - startTime)/1000L),
                                        playError);
            }
            else
            {
                playError = fileBroadcaster.PlayMovie( thePick, NULL );
            }
            
            if (sQuitImmediate) 
            {   
                break;
            }
            
            if (sBroadcasterSession != NULL && sBroadcasterSession->GetReasonForDying() != BroadcasterSession::kDiedNormally)
            {   
                playError = ::EvalBroadcasterErr(QTFileBroadcaster::eNetworkConnectionFailed);
                sNumErrors++;
                goto bail;
            }
                
            if ( playError == 0 && sAnnounceBroadcast)
            {   
                int theErr = sBroadcasterSession->GetRequestStatus();
                if (200 != theErr)
                {   
                    if (401 == theErr)
                        playError = QTFileBroadcaster::eNetworkAuthorization;
                    else if (500 == theErr)
                        playError = QTFileBroadcaster::eNetworkNotSupported;
                    else 
                        playError = QTFileBroadcaster::eNetworkConnectionError;
                        
                    sNumErrors++;
                    goto bail;
                }
            }
            
            if (playError)
            {   
                playError = ::EvalBroadcasterErr(playError);
                
                if (playError == QTFileBroadcaster::eNetworkConnectionError)
                    goto bail;

                qtss_printf("  (file: %s err: %d %s)\n", thePick, playError,GetMovieFileErrString( playError ) );
                sNumWarnings++;
                numMovieErrors++;
            }
            else
            {
                int tracks = fileBroadcaster.GetMovieTrackCount() ;
                int mtracks = fileBroadcaster.GetMappedMovieTrackCount();

                if (tracks != mtracks)
                {   
                    sNumWarnings++;
                    numMovieErrors++;
                    qtss_printf("- PlaylistBroadcaster: Warning, movie tracks do not match the SDP file.\n" );
                    qtss_printf("  Movie: %s .\n", thePick );
                    qtss_printf("  %i of %i hinted tracks will not broadcast.\n", tracks- mtracks, tracks );
                }
            }
            
            if ( !preflight && (playError != 0) )
                sgLogger->LogMediaError( thePick, GetMovieFileErrString( playError ),NULL );
        }
        

        delete [] thePick;
        thePick = NULL;
		
    } //while (true)
    
    remove(broadcastParms->mCurrentFile);
    remove(broadcastParms->mUpcomingFile);  

    if ( preflight )
    {
        

        char    str[256];   
        qtss_printf( " - "  );
        if (numMovieErrors == 1)
            strcpy(str, "PlaylistBroadcaster found one problem movie file.");
        else
            qtss_sprintf( str, "PlaylistBroadcaster found %d problem movie files." , numMovieErrors );
        qtss_printf( "%s\n", str );
        if (sgLogger != NULL) 
            sgLogger->LogInfo( str );
        
        if (numMovieErrors == moviePlayCount)
        {
            qtss_printf("There are no valid movies to play\n");
            sNumErrors++;
        }
    }
    

    if (NULL != sBroadcasterSession && !sBroadcasterSession->IsDone() )
    {   sErrorEvaluted = true;
    }
        
bail:

    Cleanup();

    delete picker;
    
    sBroadcastParms = NULL;
    delete broadcastParms;

    if ( !preflight )
    {
        if (sgLogger != NULL) 
        {   if (!sQuitImmediate)
                sgLogger->LogInfo( "PlaylistBroadcaster finished." );
            else
                sgLogger->LogInfo( "PlaylistBroadcaster stopped." );            
        }
        
        if (!sQuitImmediate) // test sQuitImmediate again 
            qtss_printf( "\nPlaylistBroadcaster broadcast finished.\n" ); 
        else
            qtss_printf( "\nPlaylistBroadcaster broadcast stopped.\n" ); 
    }
    else
    {
        if (sgLogger != NULL) 
        {   if (!sQuitImmediate)
                sgLogger->LogInfo( "PlaylistBroadcaster preflight finished." );
            else
                sgLogger->LogInfo( "PlaylistBroadcaster preflight stopped." );
        }
        
        if (!sQuitImmediate)  // test sQuitImmediate again
            qtss_printf( "\nPlaylistBroadcaster preflight finished.\n" ); 
        else
            qtss_printf( "\nPlaylistBroadcaster preflight stopped.\n" ); 

    }
    sgLogger = NULL; // protect the interrupt handler and just let it die don't delete because it is a task thread

#ifndef __Win32__
    if ( sgTrackingSucceeded )
    {
        // remove ourselves from the tracker
        // this is the "normal" remove, also signal handlers
        // may remove us.
        
        BCasterTracker  tracker( sgTrackerFilePath );
        
        tracker.RemoveByProcessID( getpid() );
        tracker.Save();
    }
#endif //__Win32__
    if (sBroadcasterSession != NULL && !sBroadcasterSession->IsDone())
    {   
        //qtss_printf("QUIT now sBroadcasterSession->TearDownNow();\n");
        sBroadcasterSession->TearDownNow();
        int count=0;
        while (count++ < 30 && !sBroadcasterSession->IsDone() )
        {   sBroadcasterSession->Run();
            OSThread::Sleep(100);
        }
        sBroadcasterSession = NULL;
    }
   
}

static void Cleanup()
{
    if (sCleanupDone == true)
        return;
    
    sCleanupDone = true;
    
    if (sPreflight)
    {
            qtss_printf("Warnings: %d\n", sNumWarnings);
            qtss_printf("Errors: %d\n", sNumErrors);
    }
    else
    {
            qtss_printf("Broadcast Warnings: %d\n", sNumWarnings);
            qtss_printf("Broadcast Errors: %d\n", sNumErrors);
    }
    
    RemoveFiles(sBroadcastParms);
        
}

static void version()
{
    /*
        print PlaylistBroadcaster version and build info
        
        see revision.h
    */
    
    //RoadRunner/2.0.0-v24 Built on: Sep 17 1999 , 16:09:53
    //revision.h (project level file) -- include with -i option
    qtss_printf("PlaylistBroadcaster/%s Built on: %s, %s\n",  kVersionString, __DATE__, __TIME__ );

}

static void usage()
{
    /*
        print PlaylistBroadcaster usage string

    */
#ifndef __Win32__
    qtss_printf("usage: PlaylistBroadcaster [-v] [-h] [-p] [-c] [-a] [-t] [-i destAddress] [-e filename] [-f] [-d] [-l] [-s broadcastNum] filename\n" );
#else
    qtss_printf("usage: PlaylistBroadcaster [-v] [-h] [-p] [-c] [-a] [-t] [-i destAddress] [-e filename] [-f] filename\n" );
#endif

    qtss_printf("       -v: Display version\n" );
    qtss_printf("       -h: Display help\n" );
    qtss_printf("       -p: Verify a broadcast description file and movie list.\n" );
    qtss_printf("       -c: Show the current movie in the log file.\n" );
    qtss_printf("       -a: Announce the broadcast to the server.\n" );
    qtss_printf("       -t: Send the broadcast over TCP to the server.\n" );
    qtss_printf("       -i: Specify the destination ip address. Over-rides config file value.\n" );
    qtss_printf("       -e: Log errors to filename.\n" );
    qtss_printf("       -f: Force a new SDP file to be created even if one already exists.\n" );
#ifndef __Win32__
    qtss_printf("       -d: Run attached to the terminal.\n" );
    qtss_printf("       -l: List running currently broadcasts.\n" );
    qtss_printf("       -s: Stop a running broadcast.\n" );
#endif
    qtss_printf("        filename: Broadcast description filename.\n" );


}



static void help()
{
    /*
        print PlaylistBroadcaster help info

    */
    ::version();
    
    ::usage();
    
    
    qtss_printf("\n\nSample broadcast description file: ");
    PLBroadcastDef(NULL,NULL,false);

}



static PlaylistPicker* MakePickerFromConfig( PLBroadcastDef* broadcastParms )
{
    // construct a PlaylistPicker object using options set from a PLBroadcastDef
    
    PlaylistPicker *picker = NULL;
    
    if ( broadcastParms && broadcastParms->ParamsAreValid() )
    {
        if ( broadcastParms->mPlayMode )
        {
            if ( !::strcmp( broadcastParms->mPlayMode, "weighted_random" ) )
            {
                int     noPlayQueueLen = 0;

                noPlayQueueLen = broadcastParms->mLimitPlayQueueLength;
                
                picker = new PlaylistPicker( 10, noPlayQueueLen );
                
            }
            else if ( !::strcmp( broadcastParms->mPlayMode, "sequential_looped" ) )
            {           
                picker = new PlaylistPicker(true);
                picker->mRemoveFlag = true;
            }
            else if ( !::strcmp( broadcastParms->mPlayMode, "sequential" ) )
            {           
                picker = new PlaylistPicker(false);
                picker->mRemoveFlag = true;
            }
        
        }
    }
    
    return picker;
}

static const char* GetMovieFileErrString( int err )
{
    static  char buff[80];
    
    switch ( err )
    {
        case 0:
            break;
            
        case QTFileBroadcaster::eMovieFileNotFound:
            return "Movie file not found."; 
        
        case QTFileBroadcaster::eMovieFileNoHintedTracks:
            return "Movie file has no hinted tracks."; 

        case QTFileBroadcaster::eMovieFileNoSDPMatches : 
            return "Movie file does not match SDP.";
            
        case QTFileBroadcaster::eMovieFileInvalid:
            return "Movie file is invalid.";
            
        case QTFileBroadcaster::eMovieFileInvalidName:
            return "Movie file name is missing or too long."; 

        default:
            qtss_sprintf( buff, "Movie set up error %d occured.",err );
            return buff;
    }
    
    return NULL;

}

static int sErr = 0;
static int EvalBroadcasterErr(int err)
{
    int returnErr = err;
    if (sErr != 0) 
        return sErr;
    
    switch (err)
    {   case 412:
        {   qtss_printf("- Server Session Failed: The request was denied.");
            qtss_printf("\n");
            break;
        }
        case QTFileBroadcaster::eNoAvailableSockets :   
        case QTFileBroadcaster::eSDPFileNotFound    :
        case QTFileBroadcaster::eSDPDestAddrInvalid :
        case QTFileBroadcaster::eSDPFileInvalid     : 
        case QTFileBroadcaster::eSDPFileNoMedia     : 
        case QTFileBroadcaster::eSDPFileNoPorts     :
        case QTFileBroadcaster::eSDPFileInvalidPort :   
        {   qtss_printf("- SDP set up failed:  ");
            switch( err )
            {
                case QTFileBroadcaster::eNoAvailableSockets : qtss_printf("System error. No sockets are available to broadcast from."); 
                break;
                case QTFileBroadcaster::eSDPFileNotFound : qtss_printf("The SDP file is missing."); 
                break;
                case QTFileBroadcaster::eSDPDestAddrInvalid : qtss_printf("The SDP file server address is invalid."); 
                break;
                case QTFileBroadcaster::eSDPFileInvalid      : qtss_printf("The SDP file is invalid."); 
                break; 
                case QTFileBroadcaster::eSDPFileNoMedia      : qtss_printf("The SDP file is missing media (m=) references."); 
                break; 
                case QTFileBroadcaster::eSDPFileNoPorts      : qtss_printf("The SDP file is missing server port information."); 
                break;
                case QTFileBroadcaster::eSDPFileInvalidPort  : qtss_printf("The SDP file contains an invalid port. Valid range is 5004 - 65530."); 
                break;

                default: qtss_printf("SDP set up error %d occured.",err);
                break;  
            
            };
            qtss_printf("\n");
        }
        break;
        
        case QTFileBroadcaster::eSDPFileInvalidTTL:
        case QTFileBroadcaster::eDescriptionInvalidDestPort:
        case QTFileBroadcaster::eSDPFileInvalidName: 
        case QTFileBroadcaster::eNetworkSDPFileNameInvalidBadPath:
        case QTFileBroadcaster::eNetworkSDPFileNameInvalidMissing:
        {   qtss_printf("- Description set up failed: ");
            switch( err )
            {
            
                case QTFileBroadcaster::eSDPFileInvalidTTL   : qtss_printf("The multicast_ttl value is incorrect. Valid range is 1 to 255."); 
                break;
                case QTFileBroadcaster::eDescriptionInvalidDestPort  : qtss_printf("The destination_base_port value is incorrect. Valid range is 5004 - 65530."); 
                break;
                case QTFileBroadcaster::eSDPFileInvalidName  : qtss_printf("The sdp_file name is missing or too long."); 
                break;
                case QTFileBroadcaster::eNetworkSDPFileNameInvalidBadPath: qtss_printf("The specified destination_sdp_file must a be relative file path in the movies directory."); 
                break;
                case QTFileBroadcaster::eNetworkSDPFileNameInvalidMissing: qtss_printf("The specified destination_sdp_file name is missing."); 
                break;
                            
                default: qtss_printf("Description set up error %d occured.",err);
                break;  
            }
            qtss_printf("\n");
        }
        break;
        
        
        
        case QTFileBroadcaster::eMovieFileNotFound          :
        case QTFileBroadcaster::eMovieFileNoHintedTracks    :
        case QTFileBroadcaster::eMovieFileNoSDPMatches      :
        case QTFileBroadcaster::eMovieFileInvalid           :
        case QTFileBroadcaster::eMovieFileInvalidName       :
        {   qtss_printf("- Movie set up failed: ");
            switch( err )
            {   
                case QTFileBroadcaster::eMovieFileNotFound          : qtss_printf("Movie file not found."); 
                break;
                case QTFileBroadcaster::eMovieFileNoHintedTracks    : qtss_printf("Movie file has no hinted tracks."); 
                break;
                case QTFileBroadcaster::eMovieFileNoSDPMatches      : qtss_printf("Movie file does not match SDP."); 
                break;
                case QTFileBroadcaster::eMovieFileInvalid           : qtss_printf("Movie file is invalid."); 
                break;
                case QTFileBroadcaster::eMovieFileInvalidName       : qtss_printf("Movie file name is missing or too long."); 
                break;
        
                default: qtss_printf("Movie set up error %d occured.",err);
                break;  
        
            };
            qtss_printf("\n");
        }
        break;
        
        case QTFileBroadcaster::eMem:       
        case QTFileBroadcaster::eInternalError:
        {   qtss_printf("- Internal Error: ");
            switch( err )
            {   
                case QTFileBroadcaster::eMem            : qtss_printf("Memory error."); 
                break;
                
                case QTFileBroadcaster::eInternalError   : qtss_printf("Internal error."); 
                break;
            
                default: qtss_printf("internal error %d occured.",err);
                break;  
            }
            qtss_printf("\n");
        }
        break;
        
        case QTFileBroadcaster::eFailedBind:
        case QTFileBroadcaster::eNetworkConnectionError:
        case QTFileBroadcaster::eNetworkRequestError:
        case QTFileBroadcaster::eNetworkConnectionStopped:
        case QTFileBroadcaster::eNetworkAuthorization:
        case QTFileBroadcaster::eNetworkNotSupported:
        case QTFileBroadcaster::eNetworkConnectionFailed:
        {   sErrorEvaluted = true;
            qtss_printf("- Network Connection: ");
            switch( err )
            {   
                case QTFileBroadcaster::eFailedBind      : qtss_printf("A Socket failed trying to open and bind to a local port.\n."); 
                break;

                case QTFileBroadcaster::eNetworkConnectionError  : qtss_printf("Failed to connect."); 
                break;
                
                case QTFileBroadcaster::eNetworkRequestError     : qtss_printf("Server returned error."); 
                break;
            
                case QTFileBroadcaster::eNetworkConnectionStopped: qtss_printf("Connection stopped."); 
                break;

                case QTFileBroadcaster::eNetworkAuthorization: qtss_printf("Authorization failed."); 
                break;

                case QTFileBroadcaster::eNetworkNotSupported: qtss_printf("Connection not supported by server."); 
                break;

                case QTFileBroadcaster::eNetworkConnectionFailed     : qtss_printf("Disconnected."); 
                break;

                default: qtss_printf("internal error %d occured.",err);
                break;  
            }
            qtss_printf("\n");
            returnErr = QTFileBroadcaster::eNetworkConnectionError;
        }
        break;
            
        default:
            
        break;
    }
    
    sErr = returnErr;

    return returnErr;
}


static int MyAccess( const char* path, int mode )
{
    int     error = 0;

#ifndef __Win32__
    if ( access( path, mode ) )
        error = errno;
    else
        error = 0;
#endif 
    
    return error;
}

static int PreflightClientBufferDelay( const char * delay, Float32 *ioDelay)
{
    
    int     numPreflightErrors = 0;
    
    if ( NULL == delay|| 0 == delay[0] )
        numPreflightErrors++;
    else 
    {   Float32 delayValue = 0.0;
        ::sscanf(delay, "%f", &delayValue);
        if (delayValue < 0.0)
            numPreflightErrors++;
        if (ioDelay != NULL)
            *ioDelay = delayValue;
    }       
    
    if (numPreflightErrors > 0)
        qtss_printf("- PlaylistBroadcaster: The client_buffer_delay parameter is invalid.\n" );

    return numPreflightErrors;
}

static int PreflightDestSDPFileAccess( const char * sdpPath )
{
    int     numPreflightErrors = 0;
    
    if ( NULL == sdpPath || 0==sdpPath[0] || 0 == ::strcmp(sdpPath, "no_name") )
    {   qtss_printf("- PlaylistBroadcaster: The destination_sdp_file parameter is missing from the Broadcaster description file.\n" );
        numPreflightErrors++;
    }

    return numPreflightErrors;

}

static int PreflightSDPFileAccess( const char * sdpPath )
{
    int     numPreflightErrors = 0;

    Assert( sdpPath );
    
    if ( !sdpPath )
    {   qtss_printf("- PlaylistBroadcaster: The sdp_file parameter is missing from the Broadcaster description file.\n" );
        numPreflightErrors++;
    }
    else
    {   int     accessError;
    
        accessError = MyAccess( sdpPath, R_OK | W_OK );
        
        switch( accessError )
        {
        
            case 0:
            case ENOENT:    // if its not there, we'll create it
                break;
            
            case EACCES:
                qtss_printf("- PlaylistBroadcaster: Permission to access the SDP File was denied.\n  Read/Write access is required.\n  (path: %s, errno: %i).\n", sdpPath, accessError);
                numPreflightErrors++;
                break;
                
            default:
                qtss_printf("- PlaylistBroadcaster: Unable to access the SDP File.\n  (path: %s, errno: %i).\n", sdpPath, accessError);
                numPreflightErrors++;
                break;
                
        }
    
    }

    return numPreflightErrors;

}

static int PreflightDestinationAddress( const char *destAddress )
{
    int     numPreflightErrors = 0;


    if ( !destAddress || (SocketUtils::ConvertStringToAddr(destAddress) == INADDR_NONE) )
    {   qtss_printf("- PlaylistBroadcaster: Error, desitination_ip_address parameter missing or incorrect in the Broadcaster description file.\n" );
        numPreflightErrors++;
    }
    
    return numPreflightErrors;

}

static int PreflightBasePort( const char *basePort )
{
    int     numPreflightErrors = 0;

        
    Assert( basePort );
    if ( basePort == NULL )
    {   qtss_printf("- PlaylistBroadcaster: Error, destination_base_port parameter missing from the Broadcaster description file.\n" );
        numPreflightErrors++;
    }
    else if ( (atoi(basePort) & 1)  != 0)
    {
        qtss_printf("- PlaylistBroadcaster: Warning, the destination_base_port parameter(%s) is an odd port number.  It should be even.\n", basePort );        
    }
    
    return numPreflightErrors;

}


static int PreflightReferenceMovieAccess( const char *refMoviePath )
{
    int     numPreflightErrors = 0;
    
    Assert( refMoviePath );
    if ( !refMoviePath )
    {   qtss_printf("- PlaylistBroadcaster: Error, sdp_reference_movie parameter missing from the Broadcaster description file.\n" );
        numPreflightErrors++;
    }
    else
    {   int     accessError;
    
        accessError = MyAccess( refMoviePath, R_OK );
        
        switch( accessError )
        {           
            case 0:
                break;
                
            case ENOENT:
                qtss_printf("- PlaylistBroadcaster: Error, SDP Reference Movie is missing.\n  (path: %s, errno: %i).\n", refMoviePath, accessError);
                numPreflightErrors++;
                break;
            
            case EACCES:
                qtss_printf("- PlaylistBroadcaster: Permission to access the SDP reference movie was denied.\n  Read access required.\n  (path: %s, errno: %i).\n", refMoviePath, accessError);
                numPreflightErrors++;
                break;
                
            default:
                qtss_printf("- PlaylistBroadcaster: Unable to access the SDP reference movie.\n  (path: %s, errno: %i).\n", refMoviePath, accessError);
                numPreflightErrors++;
                break;
                
        }
    
    }

    return numPreflightErrors;
}


static void ShowPlaylistStatus()
{   
#ifndef __Win32__
    BCasterTracker  tracker( sgTrackerFilePath );

    if ( tracker.IsOpen() )
    {
        tracker.Show();
    }
    else
    {   
        qtss_printf("- PlaylistBroadcaster: Could not open %s.  Reason: access denied.\n", sgTrackerFilePath );
        qtss_printf("- PlaylistBroadcaster: Change user or change the directory's privileges to access %s.\n",sgTrackerFilePath) ;
        ::exit(-1);

    }
#else
    qtss_printf("Showing Playlist status is currently not supported on this platform\n");
#endif
}

static void StopABroadcast( const char* arg )
{
#ifndef __Win32__
    if ( arg )
    {   
        
        BCasterTracker  tracker( sgTrackerFilePath );
        int             playlistIDToKill;
        
        playlistIDToKill = ::atoi( arg );                       
        playlistIDToKill--;     // convert from UI one based to to zero based ID for BCasterTracker

        
        if ( tracker.IsOpen() )
        {
            bool    broadcastIDIsValid;
            int     error;
            
            error = tracker.Remove( playlistIDToKill );
            
            if ( !error )   // remove the xth item from the list.
            {   tracker.Save();
                broadcastIDIsValid = true;  
            }
            else
                broadcastIDIsValid = false;
                
            if ( !broadcastIDIsValid )
            {
                if ( playlistIDToKill >= 0 )
                {
                    if ( error == ESRCH || error == -1 )
                        qtss_printf( "- PlaylistBroadcaster Broadcast ID (%s) not running.\n", arg );
                    else
                        qtss_printf( "- PlaylistBroadcaster Broadcast ID (%s), permission to stop denied (%i).\n", arg, error );
                    
                }
                else
                {   qtss_printf( "- Bad argument for stop: (%s).\n", arg );
                    ::exit( -1 );
                }
            }
            else
                qtss_printf("PlaylistBroadcaster stopped Broadcast ID: %s.\n", arg);
        }
        else
        {   qtss_printf("- PlaylistBroadcaster: Could not open %s.  Reason: access denied.\n", sgTrackerFilePath );
            qtss_printf("- PlaylistBroadcaster: Change user or change the directory's privileges to access %s.\n",sgTrackerFilePath) ;
            ::exit( -1 );

        }
    }
    else
    {   // getopt will catch this problem before we see it.
        qtss_printf("- Stop requires a Broadcast ID.\n");
        ::exit( -1 );
    }
#else
    qtss_printf("Stopping a broadcast is currently not supported on this platform\n");
#endif
}


static bool DoSDPGen( PLBroadcastDef *broadcastParms, bool preflight, bool overWriteSDP, bool generateNewSDP, int* numErrorsPtr, char* refMovie)
{
    int numSDPSetupErrors = 0;
    bool sdpFileCreated = false;
    
    if (sAnnounceBroadcast)
    {   numSDPSetupErrors += PreflightDestSDPFileAccess( broadcastParms->mDestSDPFile );
        Assert(broadcastParms->mBasePort != NULL);
        Assert(broadcastParms->mBasePort[0]!=0);
        broadcastParms->mBasePort[0] = '0';//set to dynamic. ignore any defined.
        broadcastParms->mBasePort[1] = 0;
    }
    
    if (broadcastParms->mSDPReferenceMovie != NULL)
        refMovie = broadcastParms->mSDPReferenceMovie;
        
    qtss_printf("Ref Movie = %s\n", refMovie);
    qtss_printf("SDP file = %s\n", broadcastParms->mSDPFile);
 
    numSDPSetupErrors += PreflightSDPFileAccess( broadcastParms->mSDPFile );
    
    
    numSDPSetupErrors += PreflightReferenceMovieAccess( refMovie );
    
    
    numSDPSetupErrors += PreflightDestinationAddress( broadcastParms->mDestAddress );


    numSDPSetupErrors += PreflightBasePort( broadcastParms->mBasePort );

    Float32 bufferDelay = 0.0;  
    numSDPSetupErrors += PreflightClientBufferDelay(  broadcastParms->mClientBufferDelay,&bufferDelay );
    
    if ( numSDPSetupErrors == 0 )
    {
        SDPGen* sdpGen = new SDPGen;
        
        int     sdpResult = -1;
        
        Assert( sdpGen );
        
        if ( sdpGen )
        {   
            sdpGen->SetClientBufferDelay(bufferDelay); // sdp "a=x-bufferdelay: value" 
            sdpGen->KeepSDPTracks(false); // set this to keep a=control track ids if we are going to ANNOUNCE the sdp to the server.
            sdpGen->AddIndexTracks(true); // set this if KeepTracks is false and to ANNOUNCE the sdp to the server.
            sdpResult = sdpGen->Run( refMovie, broadcastParms->mSDPFile, 
                          broadcastParms->mBasePort, broadcastParms->mDestAddress, 
                          sSDPBuffer, maxSDPbuffSize, 
                          overWriteSDP,  generateNewSDP,
                          broadcastParms->mStartTime,
                          broadcastParms->mEndTime,
                          broadcastParms->mIsDynamic,
                          broadcastParms->mTTL
                          );
        
            sdpFileCreated =  sdpGen->fSDPFileCreated;
            
            if ( sdpGen->fSDPFileCreated && !preflight)
                qtss_printf( "- PlaylistBroadcaster: Created SDP file.\n  (path: %s)\n", broadcastParms->mSDPFile);
        }
        
        
        if ( sdpResult )
        {
            if (sdpResult ==  -2)
                qtss_printf("- SDP generation failed: Unable to create the SDP File.\n  (path: %s, errno: %d).\n", broadcastParms->mSDPFile, sdpResult);
            else
                qtss_printf( "- SDP generation failed (error: %li).\n", (SInt32)sdpResult );
                        
            (*numErrorsPtr)++;
            return sdpFileCreated;
        }
        
        if ( sdpGen )
            delete sdpGen;
        
        sdpGen = NULL;
    }
    else
    {   qtss_printf( "- PlaylistBroadcaster: Too many SDP set up errors, exiting.\n" );
        *numErrorsPtr += numSDPSetupErrors;
    }

    return sdpFileCreated;

}

static bool PreflightTrackerFileAccess( int mode )
{
    
#ifndef __Win32__   
    int     trackerError;

    int dirlen = strlen(sgTrackerDirPath) + 1;
    OSCharArrayDeleter trackDir(new char[dirlen]);
    char *trackDirPtr = trackDir.GetObject();
    ::memcpy(trackDirPtr, sgTrackerDirPath, dirlen);

    trackerError= OS::RecursiveMakeDir( trackDirPtr );
    
    if ( trackerError == 0 )
        trackerError = MyAccess( sgTrackerFilePath, mode );
    
    switch ( trackerError )
    {           
        case 0:
        case ENOENT:
            break;
            
        default:
            qtss_printf("- PlaylistBroadcaster: Error opening %s.  (errno: %i).\n", sgTrackerFilePath, trackerError);
            qtss_printf("- PlaylistBroadcaster: Change user or change the directory's privileges to access %s.\n",sgTrackerFilePath) ;
            return false;
            break;
            
    }
#endif
    return true;
}

static void ShowSetupParams(PLBroadcastDef* broadcastParms, const char *bcastSetupFilePath)
{       
    qtss_printf( "\n" );
    qtss_printf( "PlaylistBroadcaster broadcast description File\n" );
    qtss_printf( "----------------------------------------------\n" );
    qtss_printf( "%s\n", bcastSetupFilePath );

    broadcastParms->ShowSettings();

}

static bool PreFlightSetupFile( const char * bcastSetupFilePath )
{
        bool success = true;
        
    // now complain!
    if ( !bcastSetupFilePath )
    {   qtss_printf("- PlaylistBroadcaster: A broadcast description file is required.\n" );
        ::usage();
        success = false;
    }
    else if (::strlen(bcastSetupFilePath) > PLBroadcastDef::kMaxBufferStringLen)
    {    qtss_printf("- PlaylistBroadcaster: A broadcast description file path cannot be longer than %d in length.\n",PLBroadcastDef::kMaxBufferStringLen );
        ::usage();
        success = false;
    }
    else
    {   int     accessError;
    
        accessError = MyAccess(bcastSetupFilePath, R_OK  );
        
        switch( accessError )
        {
        
            case 0:
                break;
                
            case ENOENT:    
                qtss_printf("- PlaylistBroadcaster: The broadcast description file is missing.\n  (path: %s, errno: %i).\n", bcastSetupFilePath, accessError);
                success = false;
                break;
            
            case EACCES:
                qtss_printf("- PlaylistBroadcaster: Permission denied to access the broadcast description file.\n  Read access required.\n  (path: %s, errno: %i).\n", bcastSetupFilePath, accessError);
                success = false;
                break;
                
            default:
                qtss_printf("- PlaylistBroadcaster: Unable to access the broadcast description file.\n  (path: %s, errno: %i).\n", bcastSetupFilePath, accessError);
                success = false;
                break;
                
        }
    
    }

        return success;
}

static bool AddOurPIDToTracker( const char* bcastSetupFilePath )
{
    // add our pid and Broadcast description file File to the tracker
#ifndef __Win32__   
    BCasterTracker  tracker( sgTrackerFilePath );
    
    if ( tracker.IsOpen() )
    {   sgTrackingSucceeded = 1;
        tracker.Add( getpid(), bcastSetupFilePath );
        tracker.Save();
    }
    else
    {               
        qtss_printf("- PlaylistBroadcaster: Could not open %s.  Reason: access denied.\n", sgTrackerFilePath );
        qtss_printf("- PlaylistBroadcaster: Change user or change the directory's privileges to access %s.\n",sgTrackerFilePath) ;
        return false;
    }
#endif
    return true; // return true if pid was successfully added to the tracker
}

#if 0

static void ShowPickDistribution( PlaylistPicker *picker )
{       
    qtss_printf( "\n" );
    qtss_printf( "Pick Distribution by Bucket\n" );
    qtss_printf( "---------------------------\n" );
    
    UInt32      bucketIndex;
    
    for ( bucketIndex = 0; bucketIndex < picker->GetNumBuckets(); bucketIndex++ )
    {   
        qtss_printf( "bucket total for w: %li, (%li)\n", (bucketIndex + 1), (SInt32)picker->mPickCounts[bucketIndex] );
    
    }
}

// archaic debug code

static void PrintContents( PLDoubleLinkedListNode<SimplePlayListElement> *node, void *  );
static void ShowPicker(PlaylistPicker *picker);


static void PrintContents( PLDoubleLinkedListNode<SimplePlayListElement> *node, void *  )
{
    qtss_printf( "element name %s\n", node->mElement->mElementName );
}

static void ShowPicker(PlaylistPicker *picker)
{
    int x;
    
    for ( x= 0; x < picker->GetNumBuckets(); x++ )
    {
        picker->GetBucket(x)->ForEach( PrintContents, NULL );
    }

}

#endif
/* changed by emil@popwire.com (see relaod.txt for info) */
bool FileCreateAndCheckAccess(char *theFileName){
    FILE *theFile;


#ifndef __Win32__
    if(access(theFileName, F_OK)){
        /* file does not exist  - create and set rights */
        theFile = ::fopen(theFileName, "w+");
        if(theFile){
            /* make sure everybody has r/w access to file */
            (void)::chmod(theFileName, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
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

static void PrintPlaylistElement(PLDoubleLinkedListNode<SimplePlayListElement> *node,void *file)
{   
    sElementCount ++;
    if (sElementCount <= sMaxUpcomingListSize)  
    {   char* thePick = node->fElement->mElementName;
        if (sSetupDirPath.EqualIgnoreCase(thePick, sSetupDirPath.Len) || '\\' == thePick[0] || '/' == thePick[0])
            qtss_fprintf((FILE*)file,"%s\n", thePick);
        else
            qtss_fprintf((FILE*)file,"%s%s\n", sSetupDirPath.Ptr,thePick);
    }
}

static void ShowPlaylistElements(PlaylistPicker *picker,FILE *file)
{
    if (sElementCount > sMaxUpcomingListSize)   
        return;
        
    UInt32  x;
    for (x= 0;x<picker->GetNumBuckets();x++)
    {   
        picker->GetBucket(x)->ForEach(PrintPlaylistElement,file);
    }
}
/* ***************************************************** */

static void RegisterEventHandlers()
{
#ifdef  __Win32__
    SetConsoleCtrlHandler( (PHANDLER_ROUTINE) SignalEventHandler, true);
    return;
#endif

#ifndef __Win32__   

struct sigaction act;
    
#if defined(sun) || defined(i386) || defined(__x86_64__) || defined(__MacOSX__) || defined(__powerpc__) || defined (__sgi_cc__) || defined(__osf__) || defined(__hpux__)
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
    act.sa_handler = (void(*)(int))&SignalEventHandler;
#elif defined(__sgi__) 
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	act.sa_handler = (void(*)(...))&SignalEventHandler;
#else
    act.sa_mask = 0;
    act.sa_flags = 0;
    act.sa_handler = (void(*)(...))&SignalEventHandler;
#endif

    if ( ::signal(SIGTERM, SIG_IGN) != SIG_IGN) 
    {   // from kill...
        if ( ::sigaction(SIGTERM, &act, NULL) != 0 )
        {   qtss_printf( "- PlaylistBroadcaster: System error (%"_SPOINTERSIZEARG_").\n", (PointerSizedInt)SIG_ERR );
        }
    }

    if ( ::signal(SIGINT, SIG_IGN) != SIG_IGN) 
    {   // ^C signal
        if ( ::sigaction(SIGINT, &act, NULL)  != 0 )
        {   qtss_printf( "- PlaylistBroadcaster: System error (%"_SPOINTERSIZEARG_").\n", (PointerSizedInt)SIG_ERR );
        }
        
    }
    
    if ( ::signal(SIGPIPE, SIG_IGN) != SIG_IGN) 
    {   // broken pipe probably from a failed RTSP session (the server went down?)
        if ( ::sigaction(SIGPIPE, &act, NULL)   != 0 )
        {   qtss_printf( "- PlaylistBroadcaster: System error (%"_SPOINTERSIZEARG_").\n", (PointerSizedInt)SIG_ERR );
        }
        
    }
 
    if ( ::signal(SIGHUP, SIG_IGN) != SIG_IGN) 
    {   // broken pipe probably from a failed RTSP session (the server went down?)
        if ( ::sigaction(SIGHUP, &act, NULL)  != 0)
        {   qtss_printf( "- PlaylistBroadcaster: System error (%"_SPOINTERSIZEARG_").\n", (PointerSizedInt)SIG_ERR );
        }
        
    }
    
    
#endif


}

/* ========================================================================
 * Signal and error handler.
 */
static void RemoveFiles(PLBroadcastDef* broadcastParms)
{
    if (broadcastParms != NULL)
    {
        if (broadcastParms->mPIDFile != NULL)
                    remove(broadcastParms->mPIDFile);
        if (broadcastParms->mStopFile != NULL)
            remove(broadcastParms->mStopFile);
        if (broadcastParms->mReplaceFile != NULL)
            remove(broadcastParms->mReplaceFile);
        if (broadcastParms->mInsertFile != NULL)
            remove(broadcastParms->mInsertFile);
        if (broadcastParms->mCurrentFile != NULL)
            remove(broadcastParms->mCurrentFile);
        if (broadcastParms->mUpcomingFile != NULL)
            remove(broadcastParms->mUpcomingFile);  
    }

}
/* ========================================================================
 * Signal and error handler.
 */
static void SignalEventHandler( int signalID )
{   

#ifdef __Win32__
        if ( (signalID != SIGINT) && (signalID != SIGTERM) )
#else
        if (signalID == SIGPIPE)
#endif
            sNumErrors++;
        
        // evaluate the error
    if (sRTSPClientError != 0 && !sErrorEvaluted)
    {   
        EvalBroadcasterErr(sRTSPClientError);
        sErrorEvaluted = true;
    }
    else if (sBroadcasterSession != NULL && !sErrorEvaluted)
    {
#ifndef __Win32__
        if ( (signalID == SIGINT) || (signalID == SIGTERM) )
        {   EvalBroadcasterErr(QTFileBroadcaster::eNetworkConnectionStopped);
        }
        else
        {   EvalBroadcasterErr(QTFileBroadcaster::eNetworkConnectionFailed);
        }
#else       
        EvalBroadcasterErr(QTFileBroadcaster::eNetworkConnectionStopped);
#endif
        sErrorEvaluted = true;
    }

        // do the cleanup - write warning and error messages and remove files
        Cleanup();
        
#ifndef __Win32__
    if (sBroadcasterSession != NULL)
    {   

        if ( (signalID == SIGINT) || (signalID == SIGTERM) )
            sQuitImmediate = true; // just in case we get called again
            
        return; //we need to let the broadcaster session teardown and clean up
    }

    if ( sgTrackingSucceeded )
    {   
        // give tracker scope so that it really does it's
        // thing before "exit" is called.
        BCasterTracker  tracker( sgTrackerFilePath );
    
        tracker.RemoveByProcessID( getpid() );
        tracker.Save();
    }
    
#endif  

    if (!sQuitImmediate) // do once
    {   
        sQuitImmediate = true; // just in case we get called again

#ifdef __Win32__

        if (sBroadcasterSession && !sBroadcasterSession->IsDone())
        {   sBroadcasterSession->TearDownNow();
            OSThread::Sleep(1000);
        }
        if (NULL == sBroadcasterSession)
            qtss_printf("\n"); // make sure the message was printed before quitting.
        
#endif      
        return;
        
    }
    ::exit(-1);
}
