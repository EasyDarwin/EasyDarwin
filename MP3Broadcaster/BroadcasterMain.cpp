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

#ifndef kVersionString
#include "revision.h"
#endif
#include "MP3Broadcaster.h"
#include "OSHeaders.h"
#include "SocketUtils.h"
#include "OSThread.h"
#include "OS.h"
#include "OSMemory.h"
#include <signal.h>
#include "getopt.h"

MP3Broadcaster* gBroadcaster = NULL;

static void RegisterEventHandlers();
static void SignalEventHandler( int signalID );

void version()
{
    /*
        print PlaylistBroadcaster version and build info
        
        see revision.h
    */
    
    //RoadRunner/2.0.0-v24 Built on: Sep 17 1999 , 16:09:53
    //revision.h (project level file) -- include with -i option
    qtss_printf("MP3Broadcaster/%s Built on: %s, %s\n",  kVersionString, __DATE__, __TIME__ );

}

void usage()
{
    /*
        print MP3Broadcaster usage string

    */ 
    
    qtss_printf("usage: MP3Broadcaster [-v] [-d] [-i] [-x] [-X] [-a ipAddress] [-p portNum] [-l filename] [-w filename] [-e filename] -c filename\n" );
    qtss_printf("       -v: display version\n" );
    qtss_printf("       -d: run in foreground\n" );
    qtss_printf("       -i: use 'icy-' protocol header prefix (default is 'x-audio-')\n" );
    qtss_printf("       -x: preflight configuration\n" );
    qtss_printf("       -X: check MP3 files\n" );
    qtss_printf("       -a <ipaddr>: broadcast to this address (default = local loopback)\n" );
    qtss_printf("       -p <port>: broadcast to this port (default = 8000)\n" );
    qtss_printf("       -c <path>: path to config file\n" );
    qtss_printf("       -l <path>: path to playlist (overrides config file)\n" );
    qtss_printf("       -w <path>: path to dir to create temp lists (overrides config file)\n" );
    qtss_printf("       -e <path>: print output to error file\n" );
}


int main(int argc, char* argv[])
{
    bool daemonize = true;
    bool useICY = false;
    char* ipaddr = NULL;
    int port = 0;
    char* config = NULL;
    char* playList = NULL;
    char* workingDir = NULL;
    char ch;
    bool preflight = false;
    bool checkMP3s = false;
    char*   errorlog = NULL;
//    extern int optind;
    extern char* optarg;
    
#ifdef __Win32__        
    //
    // Start Win32 DLLs
    WORD wsVersion = MAKEWORD(1, 1);
    WSADATA wsData;
    (void)::WSAStartup(wsVersion, &wsData);
#endif

    OS::Initialize();
    OSThread::Initialize();
    OSMemory::SetMemoryError(ENOMEM);
    Socket::Initialize();
    SocketUtils::Initialize(false);
    
    if (SocketUtils::GetNumIPAddrs() == 0)
    {
        qtss_printf("Network initialization failed. IP must be enabled to run MP3Broadcaster\n");
        ::exit(0);
    }

    while ( (ch = getopt(argc,argv, "vdixXa:p:c:l:e:w:")) != EOF ) // opt: means requires option
    {
        switch(ch)
        {
            case 'v':
                ::version();
                ::usage();
                return 0;
                
            case 'd':
                daemonize = false;
                break;
                
            case 'i':
                useICY = true;
                break;
                
            case 'a':
                ipaddr = (char*)malloc(strlen(optarg)+1);
                strcpy(ipaddr, optarg);
                break;
                
            case 'p':
                port = atoi(optarg);
                break;
                
            case 'c':
                config = (char*)malloc(strlen(optarg)+1);
                strcpy(config, optarg);
                break;
                
            case 'l':
                playList = (char*)malloc(strlen(optarg)+1);
                strcpy(playList, optarg);
                break;
                
            case 'w':
                workingDir = (char*)malloc(strlen(optarg)+1);
                break;
                
            case 'x':
                preflight = true;
                daemonize = false;
                break;
                
            case 'X':
                preflight = true;
                daemonize = false;
                checkMP3s = true;
                break;
                
            case 'e':
                errorlog = (char*)malloc(strlen(optarg)+1);
                strcpy(errorlog, optarg);
                break;
                
            default:
                ::usage();
                return 0;
        }
    }

    if (config == NULL)
    {
        qtss_printf("missing -c option\n");
        ::usage();
        return 0;
    }
    
    if (errorlog != NULL)
    {   
                if (preflight)
                    freopen(errorlog, "w", stdout);
                else
                    freopen(errorlog, "a", stdout);
        ::setvbuf(stdout, (char *)NULL, _IONBF, 0);
    }
        
    gBroadcaster = new MP3Broadcaster(ipaddr, port, config, playList, workingDir, useICY);

    if (!gBroadcaster->IsValid())
    {
        qtss_printf("Bad config--exiting\n");
        qtss_printf("Warnings: 0\nErrors: 1\n");
        return 0;
    }
    
    RegisterEventHandlers();
    gBroadcaster->PreFlightOrBroadcast(preflight, daemonize, false, false, checkMP3s, errorlog);

	return 0;
}

static void RegisterEventHandlers()
{
#ifdef  __Win32__
    SetConsoleCtrlHandler( (PHANDLER_ROUTINE) SignalEventHandler, true);
    return;
#endif

#ifndef __Win32__   

struct sigaction act;
    
#if defined(sun) || defined(i386) || defined(__x86_64__) || defined(__MacOSX__) || defined(__sgi__) || defined(__osf__) || defined(__hpux__) || defined(__linuxppc__)
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = (void(*)(int))&SignalEventHandler;
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
    {   // catch any SIGHUP
        if ( ::sigaction(SIGHUP, &act, NULL)  != 0)
        {   qtss_printf( "- PlaylistBroadcaster: System error (%"_SPOINTERSIZEARG_").\n", (PointerSizedInt)SIG_ERR );
        }
        
    }
    
    if ( ::signal(SIGALRM, SIG_IGN) != SIG_IGN) 
    {   // catch any SIGALRM
        if ( ::sigaction(SIGALRM, &act, NULL)  != 0)
        {   qtss_printf( "- PlaylistBroadcaster: System error (%"_SPOINTERSIZEARG_").\n", (PointerSizedInt)SIG_ERR );
        }
        
    }
    
#endif
}

/* ========================================================================
 * Signal and error handler.
 */
static void SignalEventHandler( int signalID )
{   
   if (gBroadcaster)
   {
#ifdef __Win32__
      if ( (signalID != SIGINT) && (signalID != SIGTERM) ) 
#else
      if ( (signalID == SIGALRM) || (signalID == SIGHUP) )    // unexpected SIGALRM || SIGHUP

	  {

         qtss_printf( "- PlaylistBroadcaster: Unexpected signal type (%"_SPOINTERSIZEARG_").\n", signalID );

         // just ignore it...

         return;

	  }

      if (signalID == SIGPIPE)    // broken pipe from server
#endif

          gBroadcaster->Cleanup(true);
      else                            // kill or ^C
          gBroadcaster->Cleanup(false);
    }
    ::exit(-1);
}
