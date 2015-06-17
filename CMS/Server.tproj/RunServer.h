/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       RunServer.h
    Contains:   Routines to run the Streaming Server
*/

#include "OSHeaders.h"
#include "XMLPrefsParser.h"
#include "PrefsSource.h"
#include "QTSS.h"
#include "QTSServer.h"

enum { 
        kRunServerDebug_Off = 0,
        kRunServerDebugDisplay_On = 1 << 0, 
        kRunServerDebugLogging_On = 1 << 1 // not implemented
     };
     
inline Bool16 DebugOn(QTSServer* server) { return ( server->GetDebugOptions() != kRunServerDebug_Off )  ? true : false ; }
inline Bool16 DebugDisplayOn(QTSServer* server) { return (server->GetDebugOptions() & kRunServerDebugDisplay_On)  ? true : false ; }
inline Bool16 DebugLogOn(QTSServer* server) { return (server->GetDebugOptions() & kRunServerDebugLogging_On)  ? true : false ; }

//
// This function starts the Streaming Server. Pass in a source
// for preferences, a source for text messages, and an optional
// port to override the default.
//
// Returns the server state upon completion of startup. If this
// is qtssFatalErrorState, something went horribly wrong and caller
// should just die.
QTSS_ServerState StartServer(   XMLPrefsParser* inPrefsSource,
                                PrefsSource* inMessagesSource,
                                UInt16 inPortOverride,
                                int statsUpdateInterval,
                                QTSS_ServerState inInitialState,
								Bool16 inDontFork,
								UInt32 debugLevel, 
								UInt32 debugOptions );

//
// Call this after StartServer if it doesn't return qtssFatalError.
// This will not return until the server is going away
void RunServer();

// write pid to file
void WritePid(Bool16 forked);

// clean the pid file
void CleanPid(Bool16 force);

// make the status file
void LogStatus(QTSS_ServerState theServerState);
