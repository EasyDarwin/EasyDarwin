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
	File:		StreamingLoadTool.cpp

	Contains:       Tool that simulates streaming movie load
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include<string.h>

#ifndef kVersionString
#include "revision.h"
#endif
#ifndef __Win32__
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#endif

#include "ClientSession.h"
#include "FilePrefsSource.h"
#include "OSMemory.h"
#include "OSArrayObjectDeleter.h"
#include "SocketUtils.h"
#include "StringFormatter.h"
#include "Socket.h"
#include "OS.h"
#include "Task.h"
#include "TimeoutTask.h"
#include "SVector.h"
#ifndef __MacOSX__
#include "getopt.h"
#include "revision.h"
#endif

#define STREAMINGLOADTOOL_DEBUG 0
#define PACKETADDSIZE  28 // IP headers = 20 + UDP headers = 8

//
// Static data
static UInt32	sConnectionsThatErrored = 0;
static UInt32	sFailedConnections = 0;
static UInt32	sSuccessfulConnections = 0;
static FILE*	sLog = NULL;

static ClientSession** sClientSessionArray = NULL;
static UInt32 sNumClients = 1;
static Bool16 sNumClientsIsSpecified = false;
static Bool16 sGotSigInt = false;
static Bool16 sQuitNow = false;
static SInt64 sSigIntTime = 0;

static UInt64 sTotalBytesReceived = 0;
static UInt64 sTotalPacketsReceived = 0;
static UInt64 sTotalPacketsLost = 0;
static UInt64 sTotalOutOfOrder = 0;
static UInt64 sTotalOutOfBound = 0;
static UInt64 sTotalDuplicates = 0;
static UInt64 sTotalNumAcks = 0;
static UInt64 sTotalMalformed = 0;
static UInt64 sTotalLatePackets;
static UInt64 sTotalBufferOverflowedPackets;
static Bool16 sEnable3GPP = false;
	

int main(int argc, char *argv[]);

//
// Helper functions
char* 	GetClientTypeDescription(ClientSession::ClientType inClientType);
void	DoDNSLookup(SVector<char *> &theURLlist, SVector<UInt32> &ioIPAddrs);
void 	RecordClientInfoBeforeDeath(ClientSession* inSession);
char*	GetDeathReasonDescription(UInt32 inDeathReason);
char*	GetPayloadDescription(QTSS_RTPPayloadType inPayload);
void	CheckForStreamingLoadToolDotMov(SVector<UInt32> &ioIPAddrArray, SVector<char *> &theURLlist, UInt16 inPort, SVector<char *> &userList, SVector<char *> &passwordList, UInt32 verboseLevel);
UInt32 CalcStartTime(Bool16 inRandomThumb, UInt32 inMovieLength);
extern char* optarg;

#ifndef __Win32__
void sigcatcher(int sig, int /*sinfo*/, struct sigcontext* /*sctxt*/);

void sigcatcher(int sig, int /*sinfo*/, struct sigcontext* /*sctxt*/)
{
	//printf("sigcatcher =%d\n", sig);

	if ((sig == SIGINT) || (sig == SIGTERM))
	{       // TheTime check and sQuitNow flag tests are needed test with Linux and OSX. 
		if (sGotSigInt && (OS::Milliseconds() > sSigIntTime + 500) )
		{	if (!sQuitNow)// print only once
				printf("Force quitting\n");
			sQuitNow = true;
		}
		else
		{
			sSigIntTime = OS::Milliseconds();
			sGotSigInt = true;
		}
	}
}
#endif

int main(int argc, char *argv[])
{
#ifndef __Win32__
	struct sigaction act;
	
#if defined(sun) || defined(i386) || defined(__x86_64__) || defined (__MacOSX__) || defined(__powerpc__) || defined (__osf__) || defined (__sgi_cc__) || defined (__hpux__)
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = (void(*)(int))&sigcatcher;
#elif defined(__sgi__) 
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	act.sa_handler = (void(*)(...))&sigcatcher;
#else
    act.sa_mask = 0;
    act.sa_flags = 0;
    act.sa_handler = (void(*)(...))&sigcatcher;
#endif
	(void)::sigaction(SIGPIPE, &act, NULL);
	(void)::sigaction(SIGINT, &act, NULL);
	(void)::sigaction(SIGTERM, &act, NULL);

#if __MacOSX__ || __solaris__
	//grow our pool of file descriptors to the max!
	struct rlimit rl;
               
	rl.rlim_cur = Socket::kMaxNumSockets;
	rl.rlim_max = Socket::kMaxNumSockets;

	setrlimit (RLIMIT_NOFILE, &rl);
#endif
#else
	//
	// Start Win32 DLLs
	WORD wsVersion = MAKEWORD(1, 1);
	WSADATA wsData;
	(void)::WSAStartup(wsVersion, &wsData);
#endif

#ifdef __Win32__
	char* configFilePath = "streamingloadtool.cfg";
#else
	char* configFilePath = "streamingloadtool.conf";
#endif
    Bool16 configFilePathIsSpecified = false;
	Bool16 dropPost = false;
	ClientSession::ClientType theClientType = ClientSession::kRTSPUDPClientType;
    Bool16 theClientTypeIsSpecified = false;
	UInt16 thePort = 554;
	Bool16 thePortIsSpecified = false;
	UInt32 theMovieLength = 60;
    Bool16 theMovieLengthIsSpecified = false;
	Bool16 runForever = false;
	UInt32 theHTTPCookie = 1;
	Bool16 shouldLog = false;
	char* logPath = "streamingloadtool.log";
	UInt32 proxyIP = 0;
	Bool16 appendJunk = false;
	UInt32 theReadInterval = 50;
	UInt32 sockRcvBuf = 32768;
	Float32 lateTolerance = 0;
	char* rtpMetaInfo = NULL;
	Float32 speed = 1;
	UInt32 verboseLevel = 0;
	char* packetPlayHeader = NULL;
	UInt32 overbufferwindowInK = 0;
	Bool16 randomThumb = false;
	Bool16 sendOptions = false; 
	Bool16 requestRandomData = false; 
	SInt32 randomDataSize = 0;
	UInt32 rtcpInterval = 5000;
	UInt32 bandwidth = 0;
	UInt32 guarenteedBitRate = 0;
	UInt32 maxBitRate = 0;
	UInt32 maxTransferDelay = 0;
	Bool16 enableForcePlayoutDelay = false;
	UInt32 playoutDelay = 0;
    UInt32 bufferSpace = 100000;
    UInt32 delayTime = 10000;
	Float32 startDelayFrac = 0.5;
	//
	// Set up our User Agent string for the RTSP client
	char theUserAgentStr[128];
	::sprintf(theUserAgentStr, "StreamingLoadTool-%s",kVersionString);
	RTSPClient::SetUserAgentStr(theUserAgentStr);

	SVector<char *>userList;
	SVector<char *>passwordList;
	SVector<char *> theURLlist;

    char* controlID = NULL;
	char			ch = 0;
	//
	// Read our command line options
	while( (ch = getopt(argc, argv, "vf:c:i:V:u:t:p:n:l:s:")) != -1 )
	{
		switch( ch )
		{
			case '?':
			case 'v':
				printf("StreamingLoadTool %s, built on %s, %s.\n", kVersionString, __DATE__  , __TIME__);
				printf("Usage: StreamingLoadTool [-u theURL] [-t transport] [-n port] [-l length] [-i urlid] [-f path] [-c #] [-V #]\n");
				printf("-v: Print this message\n");
				printf("-f: Config file to use. Defaults to streamingloadtool.conf if the URL (-u) is not specified.\n");
				printf("-c: HTTP cookie to use. Overrides what is in config file\n");
				printf("-V: debug verbose level\n");
				printf("-i: RTSP stream URL id (i.e. trackID or streamID etc.). Default is -i trackID\n");
                printf("-u: the URL (format: rtsp://etc/movie.mov).\n");
                printf("-t: the transport type: udp, reliableudp, tcp, or http.\n");
                printf("-p: the port.\n");
                printf("-n: the number of clients.\n");
                printf("-l: the movie duration.\n");
                printf("-s: the user and password format: (user:password).\n");
                
				exit(0);
				break;
			case 'f':
                configFilePathIsSpecified = true;
				configFilePath = optarg;
				break;
		    case 'i':
		        controlID = optarg;
		        break;
			case 'c':
				theHTTPCookie = atoi(optarg);
				break;
			case 'V':
				verboseLevel = atoi(optarg);
				break;
            case 'u':
                theURLlist.push_back(optarg);
                break;
            case 't':
                theClientTypeIsSpecified = true;
                if (::strcmp("http", optarg) == 0)
                    theClientType = ClientSession::kRTSPHTTPClientType;
                else if (::strcmp("reliableudp", optarg) == 0)
                    theClientType = ClientSession::kRTSPReliableUDPClientType;
                else if (::strcmp("tcp", optarg) == 0)
                    theClientType = ClientSession::kRTSPTCPClientType;
                else if (::strcmp("udp", optarg) == 0)
                    theClientType = ClientSession::kRTSPUDPClientType;
                else if (::strcmp("3gudp", optarg) == 0)
                {    theClientType = ClientSession::kRTSPUDPClientType;
                     sEnable3GPP = true;
                }
                else
                {
                    printf("Invalid transport type: %s\n", optarg);
                    exit(0);
                }
                break;
            case 'p':
                thePortIsSpecified = true;
                thePort = atoi(optarg);
                break;
            case 'n':
                sNumClientsIsSpecified = true;
                sNumClients = atoi(optarg);
                break;
            case 'l':
                theMovieLengthIsSpecified = true;
                theMovieLength = atoi(optarg);
                break;
            case 's' :
                {
                    char *str = ::strdup(optarg);
                    char *user = str;
                    char *password = NULL;
                    char *colonChar = ::strchr(str, ':');
                    printf("optarg=%s\n",str);
                    if (colonChar != NULL)
                    {
                        *colonChar = '\0';
                        password = colonChar + 1;
                    }
                    if ((user != NULL) && (password != NULL) )
                    {
                      	userList.push_back(user);
                    	passwordList.push_back(password);
                   	}
                    break;
                }
                
		}
	}
	
	FilePrefsSource theFileParser(true);
	// Get settings from the file
	Bool16 configFileError = theFileParser.InitFromConfigFile(configFilePath);
    if (configFileError && theURLlist.empty())
    {
        printf("Couldn't find StreamingLoadTool config file at: %s\n", configFilePath);
        ::exit(-1);
    }
	else if (!configFileError)
    {
        for (UInt32 x = 0; true; x++)
        {
            char* theValue = theFileParser.GetValueAtIndex(x);
            char* theKey = theFileParser.GetKeyAtIndex(x);

            if (theKey == NULL)
                break;
            
            if (theValue != NULL)
            {
                if (::strcmp("clienttype", theKey) == 0 && !theClientTypeIsSpecified)
                {
                    if (::strcmp("http", theValue) == 0)
                        theClientType = ClientSession::kRTSPHTTPClientType;
                    else if (::strcmp("reliableudp", theValue) == 0)
                        theClientType = ClientSession::kRTSPReliableUDPClientType;
                    else if (::strcmp("tcp", theValue) == 0)
                        theClientType = ClientSession::kRTSPTCPClientType;
                    else if (::strcmp("udp", theValue) == 0)
                        theClientType = ClientSession::kRTSPUDPClientType;
                    else if (::strcmp("3gudp", theValue) == 0)
                    {    theClientType = ClientSession::kRTSPUDPClientType;
                          sEnable3GPP = true;
                    }
                    else
                    {
                        printf("Invalid transport type: %s\n", theValue);
                        exit(0);
                    }
                }
                else if (::strcmp("player", theKey) == 0)
                {
                    if (theValue[0] != 0)
                    {	
                        ::strncpy(theUserAgentStr, theValue, sizeof(theUserAgentStr));
                        theUserAgentStr[sizeof(theUserAgentStr) -1] = 0;
                        RTSPClient::SetUserAgentStr(theUserAgentStr);
                    }
                }
                else if (::strcmp("droppost", theKey) == 0)
                {
                    if (::strcmp("yes", theValue) == 0)
                        dropPost = true;
                }
                else if (::strcmp("concurrentclients", theKey) == 0 && !sNumClientsIsSpecified)
                {
                    ::sscanf(theValue, "%"_U32BITARG_"", &sNumClients);
                }
                else if (::strcmp("port", theKey) == 0 && !thePortIsSpecified)
                {
                    UInt32 tempPort = 0;
                    ::sscanf(theValue, "%"_U32BITARG_"", &tempPort);
                    thePort = (UInt16)tempPort;
                }
                else if (::strcmp("movielength", theKey) == 0 && !theMovieLengthIsSpecified)
                {
                    ::sscanf(theValue, "%"_U32BITARG_"", &theMovieLength);
                }
                else if (::strcmp("runforever", theKey) == 0)
                {
                    if (::strcmp("yes", theValue) == 0)
                        runForever = true;
                }
                else if (::strcmp("shouldlog", theKey) == 0)
                {
                    if (::strcmp("yes", theValue) == 0)
                        shouldLog = true;
                }
                else if (::strcmp("appendjunk", theKey) == 0)
                {
                    if (::strcmp("yes", theValue) == 0)
                        appendJunk = true;
                }
                else if (::strcmp("logpath", theKey) == 0)
                {
                    logPath = theValue;
                }
                else if (::strcmp("proxyip", theKey) == 0)
                {
                    proxyIP = SocketUtils::ConvertStringToAddr(theValue);
                }
                else if (::strcmp("clientwindow", theKey) == 0)
                {
                    ::sscanf(theValue, "%"_U32BITARG_"", &sockRcvBuf);
                }
                else if (::strcmp("httpcookie", theKey) == 0)
                {
                    if (theHTTPCookie == 1)
                    {
                        // Ignore if set by command line
                        ::sscanf(theValue, "%"_U32BITARG_"", &theHTTPCookie);
                        theHTTPCookie *= 1000000;
                    }
                }
                else if (::strcmp("readinterval", theKey) == 0)
                {
                    ::sscanf(theValue, "%"_U32BITARG_"", &theReadInterval);
                }
                else if (::strcmp("latetolerance", theKey) == 0)
                {
                    ::sscanf(theValue, "%f", &lateTolerance);
                }
                else if (::strcmp("rtpmetainfo", theKey) == 0)
                {
                    if (::strlen(theValue) > 0)
                        rtpMetaInfo = theValue;
                }
                else if (::strcmp("speed", theKey) == 0)
                {
                    ::sscanf(theValue, "%f", &speed);
                }
                else if (::strcmp("packetplayheader", theKey) == 0)
                {
                    if (::strlen(theValue) > 0)
                        packetPlayHeader = theValue;
                }
                else if (::strcmp("overbufferwindowsize", theKey) == 0)
                {
                    ::sscanf(theValue, "%"_U32BITARG_"", &overbufferwindowInK);
                }
                else if (::strcmp("randomthumb", theKey) == 0)
                {
                    if (::strcmp("yes", theValue) == 0)
                        randomThumb = true;
                }
                
                else if (::strcmp("sendoptions", theKey) == 0)
                {
                    if (::strcmp("yes", theValue) == 0)
                        sendOptions = true;
                }
                else if (::strcmp("requestrandomdata", theKey) == 0)
                {
                    if (::strcmp("yes", theValue) == 0)
                        requestRandomData = true;
                }
                else if (::strcmp("randomdatasize", theKey) == 0)
                {
                    ::sscanf(theValue, "%"_S32BITARG_"", &randomDataSize);
                }
				else if (::strcmp("rtcpinterval", theKey) == 0)
                {
                    ::sscanf(theValue, "%"_S32BITARG_"", &rtcpInterval);
                }
                else if (::strcmp("url", theKey) == 0)
                {
                    theURLlist.push_back(theValue);
                }
                else if (::strcmp("user", theKey) == 0)
                {
                    char *str = ::strdup(theValue);
                    char *user = str;
                    char *password = NULL;
                    char *colonChar = ::strchr(str, ':');
                    if (colonChar != NULL)
                    {
                        *colonChar = '\0';
                        password = colonChar + 1;
                    }
                    userList.push_back(user);
                    passwordList.push_back(password);
                }
                else if (::strcmp("bandwidth", theKey) == 0)
                {
                    ::sscanf(theValue, "%"_U32BITARG_, &bandwidth);
                }
                else if (::strcmp("enable3GPP", theKey) == 0)
                {
                    if (::strcmp("yes", theValue) == 0)
                        sEnable3GPP = true;
                }
                else if (::strcmp("GBW", theKey) == 0)
                {
                    ::sscanf(theValue, "%"_U32BITARG_, &guarenteedBitRate);
                }
                else if (::strcmp("MBW", theKey) == 0)
                {
                    ::sscanf(theValue, "%"_U32BITARG_, &maxBitRate);
                }
                else if (::strcmp("MTD", theKey) == 0)
                {
                    ::sscanf(theValue, "%"_U32BITARG_, &maxTransferDelay);
                }
                else if (::strcmp("enableForcePlayoutDelay", theKey) == 0)
				{
					if (::strcmp("yes", theValue) == 0)
						enableForcePlayoutDelay = true;
				}
				else if (::strcmp("playoutDelay", theKey) == 0)
				{
					::sscanf(theValue, "%"_U32BITARG_, &playoutDelay);
				}
                else if (::strcmp("buffer", theKey) == 0)
                {
                    ::sscanf(theValue, "%"_U32BITARG_"", &bufferSpace);
                }
                else if (::strcmp("delay", theKey) == 0)
                {
                    ::sscanf(theValue, "%"_U32BITARG_"", &delayTime);
                }
                else if (::strcmp("startdelay", theKey) == 0)
                {
                    ::sscanf(theValue, "%f", &startDelayFrac);
					if(startDelayFrac < 0)
						startDelayFrac = -1.0;
                }
                else
                    printf("Found bad directive in StreamingLoadTool config file: %s\n", theKey);
            }
        }
    }
	
    if(theURLlist.empty())
	{
		printf("Didn't find any URLs in StreamingLoadTool config file.\n");
		exit(-1);
	}
	
	//
	// Figure out what type of clients we are going to run
	if ((theClientType == ClientSession::kRTSPHTTPClientType) && dropPost)
		theClientType = ClientSession::kRTSPHTTPDropPostClientType;
		
	// Do IP lookups on all the URLs
    SVector<UInt32> theIPAddrArray;
    theIPAddrArray.resize(theURLlist.size(), 0);
	::DoDNSLookup(theURLlist, theIPAddrArray);


	// Final setup before running the show
	OS::Initialize();
	OSThread::Initialize();
	OSMemory::SetMemoryError(ENOMEM);
	Socket::Initialize();
	SocketUtils::Initialize();

#if !MACOSXEVENTQUEUE
	::select_startevents();//initialize the select() implementation of the event queue
#endif
	TaskThreadPool::AddThreads(1);
	TimeoutTask::Initialize();
	Socket::StartThread();

	//Check for the existance of a file streamingloadtool.mov before proceeding
	::CheckForStreamingLoadToolDotMov(theIPAddrArray, theURLlist, thePort, userList, passwordList, verboseLevel);

	// If user specified a proxy to connect through, use that IP address, not the IP address in the URL.
	if (proxyIP != 0)
	{
		for (UInt32 ipAddrCounter = 0; ipAddrCounter < theURLlist.size(); ipAddrCounter++)
			theIPAddrArray[ipAddrCounter] = proxyIP;
	}
	
	// Open the log if we are logging
	if (shouldLog)
		sLog = ::fopen(logPath, "w");
	
	sClientSessionArray = NEW ClientSession*[sNumClients];
	
	UInt32 theCurURLIndex = 0;
	UInt32 theCurUserIndex = 0;
	for (UInt32 clientCount = 0; clientCount < sNumClients; clientCount++, theCurURLIndex = (theCurURLIndex + 1) % theURLlist.size(), theCurUserIndex++)
	{
		while(theIPAddrArray[theCurURLIndex] == 0)
			theCurURLIndex = (theCurURLIndex + 1) % theURLlist.size();
		if (theCurUserIndex == userList.size())
			theCurUserIndex = 0;
			
		sClientSessionArray[clientCount] = NEW ClientSession( theIPAddrArray[theCurURLIndex],
																thePort,					
                                                                theURLlist[theCurURLIndex],
																theClientType,		// Client type
																theMovieLength,		// Movie length
																CalcStartTime(randomThumb, theMovieLength),		// Movie start time
																rtcpInterval,
																0,					// Options interval
																theHTTPCookie++,	// HTTP cookie
																appendJunk,
																theReadInterval,	// Interval between data reads	
																sockRcvBuf,			// socket recv buffer size
																lateTolerance,		// late tolerance
																rtpMetaInfo,
																speed,
																verboseLevel,
																packetPlayHeader,
																overbufferwindowInK,
																sendOptions,        // send options request before the Describe
																requestRandomData,  // request random data in the options request
																randomDataSize, 	// size of the random data to request
																sEnable3GPP,
																guarenteedBitRate,
																maxBitRate,
																maxTransferDelay,
																enableForcePlayoutDelay,
																playoutDelay,
																bandwidth,
																bufferSpace,
																delayTime,
																static_cast<UInt32>(startDelayFrac * delayTime),
																controlID,
																userList.empty() ? NULL : userList[theCurUserIndex],
																passwordList.empty() ? NULL : passwordList[theCurUserIndex]);

#if STREAMINGLOADTOOL_DEBUG
		printf("Creating a ClientSession for URL: %s\n", theURLlist[theCurURLIndex]);
#endif
	}

	//
	// Now, all we have to do is loop, destroying and re-creating ClientSession objects when they die.
	// Occassionally, lets print out status to show what is currently going on
	Bool16 isStillActive = false;
	UInt32 loopCount = 0;
	do
	{
		if (sGotSigInt)
			break;

		OSThread::Sleep(1000);
		
		isStillActive = false;
		for (UInt32 y = 0; y < sNumClients; y++)
		{
			if (sClientSessionArray == NULL)
				continue; //skip over NULL client sessions
		
            if ((loopCount & 7) == 0) // Fancy way of mod'ing by 8, so the following will execute every 8 seconds
			{
				// if the server has not sent me any packets in the last 8 seconds, and the at least 10 seconds has elapsed since the RTSP play, then timeout
			    if ( (sClientSessionArray[y] != NULL && sClientSessionArray[y]->GetSessionPacketsReceived() == 0 && sClientSessionArray[y]->GetTotalPlayTimeInMsec() > 10000) )
				{
					sClientSessionArray[y]->Signal(Task::kTimeoutEvent); // Tell it to destroy itself if the Client is idle for a while
				}
			}

			if ( (sClientSessionArray[y] != NULL && sClientSessionArray[y]->IsDone()) )
			{
				while(theIPAddrArray[theCurURLIndex] == 0)
					theCurURLIndex = (theCurURLIndex + 1) % theURLlist.size();
				if (theCurUserIndex == userList.size())
					theCurUserIndex = 0;
			
				::RecordClientInfoBeforeDeath(sClientSessionArray[y]);
				sClientSessionArray[y]->Signal(Task::kKillEvent); // Tell it to destroy itself
				sClientSessionArray[y] = NULL;
				if (runForever)
				{
					isStillActive = true;
					// If there is a new URL to fetch, kill this client and restart
					sClientSessionArray[y] = NEW ClientSession(	theIPAddrArray[theCurURLIndex],	// IP addr
                                                                                        thePort,			// IP port
                                                                                        theURLlist[theCurURLIndex],
                                                                                        theClientType,		// Client type
                                                                                        theMovieLength,		// Movie length
                                                                                        CalcStartTime(randomThumb, theMovieLength),		// Movie start time
                                                                                        rtcpInterval,
                                                                                        0,					// Options interval
                                                                                        theHTTPCookie++,	// HTTP cookie
                                                                                        appendJunk,
                                                                                        theReadInterval,	// Interval between data reads	
                                                                                        sockRcvBuf,			// socket recv buffer size	
                                                                                        lateTolerance,		// late tolerance
                                                                                        rtpMetaInfo,
                                                                                        speed,
                                                                                        verboseLevel,
                                                                                        packetPlayHeader,
                                                                                        overbufferwindowInK,	
                                                                                        sendOptions,        // send options request before the Describe
                                                                                        requestRandomData,  // request random data in the options request
                                                                                        randomDataSize,		// size of the random data to request
																						sEnable3GPP,
																						guarenteedBitRate,
																						maxBitRate,
																						maxTransferDelay,
																						enableForcePlayoutDelay,
																						playoutDelay,
																						bandwidth,
																						bufferSpace,
																						delayTime,
																						(startDelayFrac < 0) ?  static_cast<UInt32>(kUInt32_Max) : static_cast<UInt32>(startDelayFrac * delayTime),
																						controlID,
																						userList.empty() ? NULL : userList[theCurUserIndex],
																						passwordList.empty() ? NULL : passwordList[theCurUserIndex]);

#if STREAMINGLOADTOOL_DEBUG
					printf("Creating a ClientSession for URL: %s\n", theURLlist[theCurURLIndex]);
#endif
					theCurURLIndex = (theCurURLIndex + 1) % theURLlist.size();
					theCurUserIndex++;
				}
			}
            else if (sClientSessionArray[y] != NULL) //still running
				isStillActive = true;
		}
		
		if ((loopCount & 7) == 0) // Fancy way of mod'ing by 8, so the following will execute every 8 seconds
		{

			if ((loopCount & 127) == 0) // Fancy way of dividing by 127,
			{
				// Occassionally give the user lots of info
				printf("StreamingLoadTool test in progress.\n");
				printf("Config file: %s. Client type: %s. Num clients: %"_U32BITARG_".\n",
								configFilePath, GetClientTypeDescription(theClientType), sNumClients);
				printf("Movie length: %"_U32BITARG_". Run forever: %d. HTTP cookie: %"_U32BITARG_". Port: %d\n",
								theMovieLength, runForever, theHTTPCookie, thePort);
				if (shouldLog)
					printf("Writing StreamingLoadTool log at: %s\n", logPath);
				printf("Active Playing Attempts Success Errors Failed Bitrate\n");
			}
			
			UInt32 addBytes = ClientSession::GetConnectionPacketsReceived() * PACKETADDSIZE;
            Float32 bitsReceived = (Float32) (ClientSession::GetConnectionBytesReceived() + addBytes) / 1000;                 
            
            bitsReceived += .5;
            
 			printf("%5"_U32BITARG_" %6"_U32BITARG_" %8"_U32BITARG_" %6"_U32BITARG_" %6"_U32BITARG_" %6"_U32BITARG_" %9.0fk\n",
				ClientSession:: GetActiveConnections (),
				ClientSession:: GetPlayingConnections (),
				ClientSession:: GetConnectionAttempts (),
				sSuccessfulConnections,
				sConnectionsThatErrored,
				sFailedConnections,
				bitsReceived// depends on 8 second update for bits per second
				);
		        
		}
		loopCount++;
		
	} while (isStillActive == true);

	if (sGotSigInt)
	{
		//
		// If someone hits Ctrl-C, force all the clients to wrap it up
		printf("Sending TEARDOWNs.\n");
		
		//
		// Tell all the clients to wrap it up.
		for (UInt32 clientCount = 0; clientCount < sNumClients; clientCount++)
		{
			if (sClientSessionArray == NULL)
				continue; //skip over NULL client sessions
			if (sClientSessionArray[clientCount] != NULL)	
				sClientSessionArray[clientCount]->Signal(ClientSession::kTeardownEvent);
		}
		//
		// Wait for the clients to complete
		Bool16 isDone = false;
		while (!isDone && !sQuitNow)
		{
			//wait until all the clients are done doing teardown
			OSThread::Sleep(1000);
			isDone = true;
			for (UInt32 cc2 = 0; cc2 < sNumClients; cc2++)
			{
				if (sClientSessionArray == NULL)
					continue; //skip over NULL client sessions
			
				if (sClientSessionArray[cc2] == NULL)	
					continue;

				if (!sClientSessionArray[cc2]->IsDone())
					isDone = false;
			}
		}	
	}
	
	//
	// We're done... now go through and delete the last sessions(not really)
	for (UInt32 z = 0; z < sNumClients; z++)
	{
		if (sClientSessionArray == NULL)
			continue; //skip over NULL client sessions
				
		if (sClientSessionArray[z] == NULL)	
			continue;
	
		::RecordClientInfoBeforeDeath(sClientSessionArray[z]);
	}
	
	if (sLog != NULL)
		::fclose(sLog);
		
	printf("%5"_U32BITARG_" %6"_U32BITARG_" %8"_U32BITARG_" %6"_U32BITARG_" %6"_U32BITARG_" %6"_U32BITARG_" %9.0fk\n",
		ClientSession:: GetActiveConnections (),
		ClientSession:: GetPlayingConnections (),
		ClientSession:: GetConnectionAttempts (),
		sSuccessfulConnections,
		sConnectionsThatErrored,
		sFailedConnections,
		0.0// depends on 8 second update for bits per second
	);
		
	printf("StreamingLoadTool test complete. Total number of connections: %"_U32BITARG_".\n", ClientSession:: GetConnectionAttempts ());
    printf(
            "Total bytes received: %"_U64BITARG_
            ". Total packets received: %"_U64BITARG_
            ". Total out of order packets: %"_U64BITARG_
            ". Total out of bound packets: %"_U64BITARG_
            ". Total ACKs sent: %"_U64BITARG_
            ". Total malformed packets: %"_U64BITARG_,
            sTotalBytesReceived,
            sTotalPacketsReceived,
            sTotalOutOfOrder,
            sTotalOutOfBound,
            sTotalNumAcks,
            sTotalMalformed
    );
    if (sEnable3GPP)
    {
        printf(
            ". Total 3g packets lost: %"_U64BITARG_
            ". Total 3g duplicate packets: %"_U64BITARG_
            ". Total 3g late packets: %"_U64BITARG_
            ". Total 3g buffer-overflowed packets: %"_U64BITARG_,
            sTotalPacketsLost,
            sTotalDuplicates,
            sTotalLatePackets,
            sTotalBufferOverflowedPackets
    );
	
	
	
	}
	
	printf(".\n");
	
}

UInt32 CalcStartTime(Bool16 inRandomThumb, UInt32 inMovieLength)
{
	UInt32 theMovieLength = inMovieLength;
	if (theMovieLength > 1)
		theMovieLength--;
		
	if (inRandomThumb)
		return ::rand() % theMovieLength;
	else
		return 0;
}

void CheckForStreamingLoadToolPermission(UInt32* inIPAddrArray, UInt32 inNumURLs, UInt16 inPort)
{
	//Eventually check for the existance of a specially formatted sdp file (assuming the server blindly returns sdps)
}

//Currently will only authenticate with the FIRST username/password if provided
void	CheckForStreamingLoadToolDotMov(SVector<UInt32> &ioIPAddrArray, SVector<char *> &theURLlist, UInt16 inPort, SVector<char *> &userList, SVector<char *> &passwordList, UInt32 verboseLevel)
{
    Assert(ioIPAddrArray.size() == theURLlist.size());
	printf("Checking for 'streamingloadtool.mov' on the target servers\n");

	OSArrayObjectDeleter<UInt32> holder = NEW UInt32[theURLlist.size() + 1];
	UInt32* uniqueIPAddrs = holder.GetObject();
	::memset(uniqueIPAddrs, 0, sizeof(UInt32) * (theURLlist.size() + 1));
	

	for (UInt32 count = 0; count < theURLlist.size(); count++)
	{
		if (ioIPAddrArray[count] == 0) //skip over one's that failed DNS
			continue;
		
		//check for duplicates
		/*
		Bool16 dup = false;
		for (UInt32 x = 0; uniqueIPAddrs[x] != 0; x++)
		{
			if (uniqueIPAddrs[x] == ioIPAddrArray[count])
			{
				dup = true;
				break;
			}
		}
		if (dup)
			continue;

		// For tracking dups.
		uniqueIPAddrs[count] = ioIPAddrArray[count];
		*/

 		
		// Format the URL: rtsp://xx.xx.xx.xx/streamingloadtool.mov
		char theAddrBuf[50];
		StrPtrLen theAddrBufPtr(theAddrBuf, 50);
		struct in_addr theAddr;
		theAddr.s_addr = htonl(ioIPAddrArray[count]);
		
		SocketUtils::ConvertAddrToString(theAddr, &theAddrBufPtr);

		char theURLBuf[100];
		StringFormatter theFormatter(theURLBuf, 100);
		
		theFormatter.Put("rtsp://");
		theFormatter.Put(theAddrBufPtr);
		theFormatter.Put("/streamingloadtool.mov");
		theFormatter.PutTerminator();

		StrPtrLenDel theURL(theFormatter.GetAsCString());
		
		// Make an RTSP client. We'll send a DESCRIBE to the server to check for this sucker
		TCPClientSocket theSocket = TCPClientSocket(0); //blocking

		// tell the client this is the URL to use
		theSocket.Set(ioIPAddrArray[count], inPort);

		RTSPClient theClient = RTSPClient(&theSocket);
		theClient.SetVerboseLevel(verboseLevel);

		if(userList.size() > 0)
		{
			theClient.SetName(userList.back());
			theClient.SetPassword(passwordList.back());
		}
		theClient.Set(theURL);

		//
		// Send the DESCRIBE! Whoo hoo!
		OS_Error theErr = theClient.SendDescribe();
		
		while (theErr == EINPROGRESS || theErr == EAGAIN)
				theErr = theClient.SendDescribe();
		if (theErr != OS_NoErr)
		{
			printf("##WARNING: Error connecting to %s.\n\n", theURLlist[count]);
			ioIPAddrArray[count] = 0;
			continue;
		}
		
		if (theClient.GetStatus() != 200)
		{
			printf("##WARNING: Cannot access %s\n\n", theURL.Ptr);
			ioIPAddrArray[count] = 0;
		}
		theClient.SendTeardown();
	}

	int addrCount = 0;
	for (UInt32 x = 0; x < theURLlist.size(); x++)
	{
		if ( 0 != ioIPAddrArray[x])
			addrCount++ ;
	}
	if (addrCount == 0)
	{	printf("No valid destinations\n");
		exit (-1);
	}
	printf("Done checking for 'streamingloadtool.mov' on all servers -- %i valid URL's\n", addrCount);	
}


void	DoDNSLookup(SVector<char *> &theURLlist, SVector<UInt32> &ioIPAddrs)
{
    Assert(theURLlist.size() == ioIPAddrs.size());
    enum { eDNSNameSize = 128 };
    char theDNSName[eDNSNameSize + 1];
	
	for (UInt32 x = 0; x < theURLlist.size(); x++)
	{
		// First extract the DNS name from this URL as a c-string
        StrPtrLen theURL = StrPtrLen(theURLlist[x]);
		StringParser theURLParser(&theURL);
		StrPtrLen theDNSNamePtr;
		
		theURLParser.ConsumeLength(NULL, 7); // skip over rtsp://
		theURLParser.ConsumeUntil(&theDNSNamePtr, '/'); // grab the DNS name
		StringParser theDNSParser(&theDNSNamePtr);
		theDNSParser.ConsumeUntil(&theDNSNamePtr, ':'); // strip off the port number if any
		
			
        if (theDNSNamePtr.Len > eDNSNameSize)
        {
            theDNSNamePtr.PrintStr("DSN Name Failed Lookup.\n", "\n");
            printf("The DNS name is %"_U32BITARG_" in length and is longer than the allowed %d.\n",theDNSNamePtr.Len, eDNSNameSize);
            return;
        }

		theDNSName[0] = 0;
		::memcpy(theDNSName, theDNSNamePtr.Ptr, theDNSNamePtr.Len);
		theDNSName[theDNSNamePtr.Len] = 0;
		
		
		ioIPAddrs[x] = 0;
		
		// Now pass that DNS name into gethostbyname.
		struct hostent* theHostent = ::gethostbyname(theDNSName);
		
		if (theHostent != NULL)
			ioIPAddrs[x] = ntohl(*(UInt32*)(theHostent->h_addr_list[0]));
		else
			ioIPAddrs[x] = SocketUtils::ConvertStringToAddr(theDNSName);
		
		if (ioIPAddrs[x] == 0)
		{
			printf("Couldn't look up host name: %s.\n", theDNSName);
			//exit(-1);
		}
	}
}

char* 	GetClientTypeDescription(ClientSession::ClientType inClientType)
{
	static char* kUDPString = "RTSP/UDP client";
	static char* kTCPString = "RTSP/TCP client";
	static char* kHTTPString = "RTSP/HTTP client";
	static char* kHTTPDropPostString = "RTSP/HTTP drop post client";
	static char* kReliableUDPString = "RTSP/ReliableUDP client";
	
	switch (inClientType)
	{
		case ClientSession::kRTSPUDPClientType:
			return kUDPString;
		case ClientSession::kRTSPTCPClientType:
			return kTCPString;
		case ClientSession::kRTSPHTTPClientType:
			return kHTTPString;
		case ClientSession::kRTSPHTTPDropPostClientType:
			return kHTTPDropPostString;
		case ClientSession::kRTSPReliableUDPClientType:
			return kReliableUDPString;
	}
	Assert(0);
	return NULL;
}

char*	GetDeathReasonDescription(UInt32 inDeathReason)
{
	static char* kDiedNormallyString = "Completed normally";
	static char* kTeardownFailedString = "Failure: Couldn't complete TEARDOWN";
	static char* kRequestFailedString = "Failure: Failed RTSP request";
	static char* kBadSDPString = "Failure: misformatted SDP";
	static char* kSessionTimedoutString = "Failure: Couldn't connect to server(timeout)";
	static char* kConnectionFailedString = "Failure: Server refused connection";
	static char* kDiedWhilePlayingString = "Failure: Disconnected while playing";

	switch (inDeathReason)
	{
		case ClientSession::kDiedNormally:
			return kDiedNormallyString;
		case ClientSession::kTeardownFailed:
			return kTeardownFailedString;
		case ClientSession::kRequestFailed:
			return kRequestFailedString;
		case ClientSession::kBadSDP:
			return kBadSDPString;
		case ClientSession::kSessionTimedout:
			return kSessionTimedoutString;
		case ClientSession::kConnectionFailed:
			return kConnectionFailedString;
		case ClientSession::kDiedWhilePlaying:
			return kDiedWhilePlayingString;
	}
	Assert(0);
	return NULL;
}

char*	GetPayloadDescription(QTSS_RTPPayloadType inPayload)
{
	static char*	kSound = "Sound";
	static char*	kVideo = "Video";
	static char*	kUnknown = "Unknown";

	switch (inPayload)
	{
		case qtssVideoPayloadType:
			return kVideo;
		case qtssAudioPayloadType:
			return kSound;
		default:
			return kUnknown;
	}
	return NULL;
}

void RecordClientInfoBeforeDeath(ClientSession* inSession)
{
	if (inSession->GetReasonForDying() == ClientSession::kRequestFailed)
		sConnectionsThatErrored++;
	else if (inSession->GetReasonForDying() != ClientSession::kDiedNormally)
		sFailedConnections++;
	else
		sSuccessfulConnections++;
				
				
				
				
	
	{
		UInt32 theReason = inSession->GetReasonForDying();
		in_addr theAddr;
		theAddr.s_addr = htonl(inSession->GetSocket()->GetHostAddr());
		char* theAddrStr = ::inet_ntoa(theAddr);
		
		//
		// Write a log entry for this client
		if (sLog != NULL)
		    ::fprintf(sLog, "Client complete. IP addr = %s, URL = %s. Connection status: %s. ",
						theAddrStr,
						inSession->GetClient()->GetURL()->Ptr,
						::GetDeathReasonDescription(theReason));
						
		if (theReason == ClientSession::kRequestFailed)
			if (sLog != NULL) ::fprintf(sLog, "Failed request status: %"_U32BITARG_"", inSession->GetRequestStatus());
		
		if (sLog != NULL) ::fprintf(sLog, "\n");
		
		//
		// If this was a successful connection, log statistics for this connection		
		if ((theReason == ClientSession::kDiedNormally) || (theReason == ClientSession::kTeardownFailed) || (theReason == ClientSession::kSessionTimedout))
		{
		
			UInt32 bytesReceived = 0;
			for (UInt32 trackCount = 0; trackCount < inSession->GetSDPInfo()->GetNumStreams(); trackCount++)
			{
				if (sLog != NULL) ::fprintf(sLog,
					"Track type: %s. Total packets received: %"_U32BITARG_
					". Total out of order packets: %"_U32BITARG_
					". Total out of bound packets: %"_U32BITARG_
					". Total ACKs sent: %"_U32BITARG_
					". Total malformed packets: %"_U32BITARG_,
					::GetPayloadDescription(inSession->GetTrackType(trackCount)),
					inSession->GetNumPacketsReceived(trackCount),
					inSession->GetNumPacketsOutOfOrder(trackCount),
					inSession->GetNumOutOfBoundPackets(trackCount),
					inSession->GetNumAcks(trackCount),
					inSession->GetNumMalformedPackets(trackCount)
					);
					
					
                if (sEnable3GPP)
                {
                    if (sLog != NULL) ::fprintf(sLog,
                        ". Total 3g packets lost: %"_U32BITARG_
                        ". Total 3g duplicate packets: %"_U32BITARG_
                        ". Total 3g late packets: %"_U32BITARG_
                        ". Total 3g rate adapt buffer-overflowed packets: %"_U32BITARG_,
                        inSession->Get3gNumPacketsLost(trackCount),
                        inSession->Get3gNumDuplicates(trackCount),
                        inSession->Get3gNumLatePackets(trackCount),
                        inSession->Get3gNumBufferOverflowedPackets(trackCount)
                        );
                }

                if (sLog != NULL) ::fprintf(sLog,".\n");


                bytesReceived += inSession->GetNumBytesReceived(trackCount);
                bytesReceived += inSession->GetNumPacketsReceived(trackCount) * PACKETADDSIZE;

                sTotalBytesReceived += inSession->GetNumBytesReceived(trackCount);
                sTotalPacketsReceived += inSession->GetNumPacketsReceived(trackCount);
                sTotalOutOfOrder += inSession->GetNumPacketsOutOfOrder(trackCount);
                sTotalOutOfBound += inSession->GetNumOutOfBoundPackets(trackCount);
                sTotalNumAcks += inSession->GetNumAcks(trackCount);
                sTotalMalformed += inSession->GetNumMalformedPackets(trackCount);
                sTotalPacketsLost += inSession->Get3gNumPacketsLost(trackCount);
                sTotalDuplicates += inSession->Get3gNumDuplicates(trackCount);
                sTotalLatePackets += inSession->Get3gNumLatePackets(trackCount);
                sTotalBufferOverflowedPackets += inSession->Get3gNumBufferOverflowedPackets(trackCount);
            }
			UInt32 duration = (UInt32)(inSession->GetTotalPlayTimeInMsec() / 1000);
			Float32 bitRate = (((Float32)bytesReceived) / ((Float32)duration) * 8) / 1024;
						

			if (sLog != NULL) ::fprintf(sLog, "Play duration in sec: %"_U32BITARG_". Total stream bit rate in Kbits / sec: %f.\n", duration, bitRate);
		}
		
		if (sLog != NULL) ::fprintf(sLog, "\n");		
	}
}

