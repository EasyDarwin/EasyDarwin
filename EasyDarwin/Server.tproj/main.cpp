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
    File:       main.cpp

    Contains:   main function to drive streaming server.


*/

#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include "defaultPaths.h"
#ifndef __MacOSX__ 
#include "getopt.h"
#endif

#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/resource.h>
#include <sys/wait.h>

#ifndef __Win32__ 
#include <unistd.h>
#endif

#if defined (__solaris__) || defined (__osf__) || defined (__hpux__)
#include "daemon.h"
#endif

#if __MacOSX__ || __FreeBSD__
#include <sys/sysctl.h>
#include <sys/stat.h>
#endif

#include "FilePrefsSource.h"
#include "RunServer.h"
#include "QTSServer.h"
#include "QTSSExpirationDate.h"
#include "GenerateXMLPrefs.h"

static int sSigIntCount = 0;
static int sSigTermCount = 0;
static pid_t sChildPID = 0;

void usage();

void usage()
{
    const char *usage_name = PLATFORM_SERVER_BIN_NAME;
//long ptrsize = sizeof(char *); printf("size of ptr = %ld\n", ptrsize);
//long longsize = sizeof(long); printf("size of long = %ld\n", longsize);

   qtss_printf("%s/%s ( Build/%s; Platform/%s; %s) Built on: %s\n",QTSServerInterface::GetServerName().Ptr,
                                        QTSServerInterface::GetServerVersion().Ptr,
                                        QTSServerInterface::GetServerBuild().Ptr,
                                        QTSServerInterface::GetServerPlatform().Ptr,
                                        QTSServerInterface::GetServerComment().Ptr,
                                        QTSServerInterface::GetServerBuildDate().Ptr);
    qtss_printf("usage: %s [ -d | -p port | -v | -c /myconfigpath.xml | -o /myconfigpath.conf | -x | -S numseconds | -I | -h ]\n", usage_name);
    qtss_printf("-d: Run in the foreground\n");
    qtss_printf("-D: Display performance data\n");
    qtss_printf("-p XXX: Specify the default RTSP listening port of the server\n");
    qtss_printf("-c /myconfigpath.xml: Specify a config file\n");
    qtss_printf("-o /myconfigpath.conf: Specify a DSS 1.x / 2.x config file to build XML file from\n");
    qtss_printf("-x: Force create new .xml config file and exit.\n");
    qtss_printf("-S n: Display server stats in the console every \"n\" seconds\n");
    qtss_printf("-I: Start the server in the idle state\n");
    qtss_printf("-h: Prints usage\n");
}

Bool16 sendtochild(int sig, pid_t myPID);
Bool16 sendtochild(int sig, pid_t myPID)
{
    if (sChildPID != 0 && sChildPID != myPID) // this is the parent
    {   // Send signal to child
        ::kill(sChildPID, sig);
        return true;
    }

    return false;
}

void sigcatcher(int sig, int /*sinfo*/, struct sigcontext* /*sctxt*/);
void sigcatcher(int sig, int /*sinfo*/, struct sigcontext* /*sctxt*/)
{
#if DEBUG
    qtss_printf("Signal %d caught\n", sig);
#endif
    pid_t myPID = getpid();
    //
    // SIGHUP means we should reread our preferences
    if (sig == SIGHUP)
    {
        if (sendtochild(sig,myPID))
        {
            return;
        }
        else
        {
            // This is the child process.
            // Re-read our preferences.
            RereadPrefsTask* task = new RereadPrefsTask;
            task->Signal(Task::kStartEvent);

        }
    }
        
    //Try to shut down gracefully the first time, shutdown forcefully the next time
    if (sig == SIGINT) // kill the child only
    {
        if (sendtochild(sig,myPID))
        {
            return;// ok we're done 
        }
        else
        {
			//
			// Tell the server that there has been a SigInt, the main thread will start
			// the shutdown process because of this. The parent and child processes will quit.
			if (sSigIntCount == 0)
				QTSServerInterface::GetServer()->SetSigInt();
			sSigIntCount++;
		}
    }
	
	if (sig == SIGTERM || sig == SIGQUIT) // kill child then quit
    {
        if (sendtochild(sig,myPID))
        {
             return;// ok we're done 
        }
        else
        {
			// Tell the server that there has been a SigTerm, the main thread will start
			// the shutdown process because of this only the child will quit
    
    
			if (sSigTermCount == 0)
				QTSServerInterface::GetServer()->SetSigTerm();
			sSigTermCount++;
		}
    }
}

extern "C" {
typedef int (*EntryFunction)(int input);
}

Bool16 RunInForeground();
Bool16 RunInForeground()
{

    #if __linux__ || __MacOSX__
         (void) setvbuf(stdout, NULL, _IOLBF, 0);
         OSThread::WrapSleep(true);
    #endif
    
    return true;
}


Bool16 RestartServer(char* theXMLFilePath)
{
	Bool16 autoRestart = true;
	XMLPrefsParser theXMLParser(theXMLFilePath);
	theXMLParser.Parse();
	
	ContainerRef server = theXMLParser.GetRefForServer();
	ContainerRef pref = theXMLParser.GetPrefRefByName( server, "auto_restart" );
	char* autoStartSetting = NULL;
	
	if (pref != NULL)
		autoStartSetting = theXMLParser.GetPrefValueByRef( pref, 0, NULL, NULL );
		
	if ( (autoStartSetting != NULL) && (::strcmp(autoStartSetting, "false") == 0) )
		autoRestart = false;
		
	return autoRestart;
}

int main(int argc, char * argv[]) 
{
    extern char* optarg;
    
    

    // on write, don't send signal for SIGPIPE, just set errno to EPIPE
    // and return -1
    //signal is a deprecated and potentially dangerous function
    //(void) ::signal(SIGPIPE, SIG_IGN);
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
    (void)::sigaction(SIGHUP, &act, NULL);
    (void)::sigaction(SIGINT, &act, NULL);
    (void)::sigaction(SIGTERM, &act, NULL);
    (void)::sigaction(SIGQUIT, &act, NULL);
    (void)::sigaction(SIGALRM, &act, NULL);


#if __solaris__ || __linux__ || __hpux__
    //grow our pool of file descriptors to the max!
    struct rlimit rl;
    
    // set it to the absolute maximum that the operating system allows - have to be superuser to do this
    rl.rlim_cur = RLIM_INFINITY;
    rl.rlim_max = RLIM_INFINITY;
 
    setrlimit (RLIMIT_NOFILE, &rl);
#endif

#if __MacOSX__
    struct rlimit rl;
    getrlimit(RLIMIT_NOFILE,  &rl); //get the default values
    //printf("current open file limit =%"_U32BITARG_"\n", (UInt32) rl.rlim_cur); //leopard returns  256
    //printf("current open file max =%"_U32BITARG_"\n", (UInt32) rl.rlim_max);//leopard returns infinity (-1)
    
    rl. rlim_max = (rlim_t) RLIM_INFINITY -1; //use a big number to find out the real max but do not use RLIM_INFINITY that is not allowed. see man page
    setrlimit (RLIMIT_NOFILE, &rl); //resets the max value stored by limits to the boot config values.
    getrlimit(RLIMIT_NOFILE,  &rl); //now get the real max value
    //printf("current open file limit =%"_U32BITARG_"\n", (UInt32) rl.rlim_cur);
    //printf("current open file max =%"_U32BITARG_"\n", (UInt32) rl.rlim_max);
    
    rl.rlim_cur = (rlim_t) ( (float) rl.rlim_max * 0.9);   //use 90% of the max set in /etc/rc.server and /etc/sysctl.conf.default
    setrlimit (RLIMIT_NOFILE, &rl);  //finally set the current limit 
    
#endif
    
#if 0 // testing
    getrlimit(RLIMIT_NOFILE,  &rl);
    printf("current open file limit =%"_U32BITARG_"\n", (UInt32) rl.rlim_cur);
    printf("current open file max =%"_U32BITARG_"\n", (UInt32) rl.rlim_max);
#endif

#if __MacOSX__ || __FreeBSD__
        //
        // These 2 OSes have problems with large socket buffer sizes. Make sure they allow even
        // ridiculously large ones, because we may need them to receive a large volume of ACK packets
        // from the client
        
        //
        // We raise the limit imposed by the kernel by calling the sysctl system call.
        int mib[CTL_MAXNAME];
        mib[0] = CTL_KERN;
        mib[1] = KERN_IPC;
        mib[2] = KIPC_MAXSOCKBUF;
        mib[3] = 0;

        int maxSocketBufferSizeVal = 2000 * 1024; // Allow up to 2 MB. That is WAY more than we should need
        (void) ::sysctl(mib, 3, 0, 0, &maxSocketBufferSizeVal, sizeof(maxSocketBufferSizeVal));
        //int sysctlErr =  ::sysctl(mib, 3, 0, 0, &maxSocketBufferSizeVal, sizeof(maxSocketBufferSizeVal));
        //qtss_printf("sysctl maxSocketBufferSizeVal=%d err=%d\n",maxSocketBufferSizeVal, sysctlErr);
 #endif
    
    //First thing to do is to read command-line arguments.
    int ch;
    int thePort = 0; //port can be set on the command line
    int statsUpdateInterval = 0;
    QTSS_ServerState theInitialState = qtssRunningState;
    
    Bool16 dontFork = false;
    Bool16 theXMLPrefsExist = true;
    UInt32 debugLevel = 0;
    UInt32 debugOptions = kRunServerDebug_Off;
	static char* sDefaultConfigFilePath = DEFAULTPATHS_ETC_DIR_OLD "easydarwin.conf";
	static char* sDefaultXMLFilePath = DEFAULTPATHS_ETC_DIR "easydarwin.xml";

    char* theConfigFilePath = sDefaultConfigFilePath;
    char* theXMLFilePath = sDefaultXMLFilePath;
    while ((ch = getopt(argc,argv, "vdfxp:DZ:c:o:S:Ih")) != EOF) // opt: means requires option arg
    {
        switch(ch)
        {
            case 'v':
                usage();
                ::exit(0);  
            case 'd':
                dontFork = RunInForeground();
                
                break;                
            case 'D':
               dontFork = RunInForeground();

               debugOptions |= kRunServerDebugDisplay_On;
                
               if (debugLevel == 0)
                    debugLevel = 1;
                    
               if (statsUpdateInterval == 0)
                    statsUpdateInterval = 3;
                    
               break;            
            case 'Z':
                Assert(optarg != NULL);// this means we didn't declare getopt options correctly or there is a bug in getopt.
                debugLevel = (UInt32) ::atoi(optarg);
                                
                break;
            case 'f':
				theXMLFilePath  = DEFAULTPATHS_ETC_DIR "easydarwin.xml";
                break;
            case 'p':
                Assert(optarg != NULL);// this means we didn't declare getopt options correctly or there is a bug in getopt.
                thePort = ::atoi(optarg);
                break;
            case 'S':
                dontFork = RunInForeground();
                Assert(optarg != NULL);// this means we didn't declare getopt options correctly or there is a bug in getopt.
                statsUpdateInterval = ::atoi(optarg);
                break;
            case 'c':
                Assert(optarg != NULL);// this means we didn't declare getopt options correctly or there is a bug in getopt.
                theXMLFilePath = optarg;
                break;
            case 'o':
                Assert(optarg != NULL);// this means we didn't declare getopt options correctly or there is a bug in getopt.
                theConfigFilePath = optarg;
                break;
            case 'x':
                theXMLPrefsExist = false; // Force us to generate a new XML prefs file
                theInitialState = qtssShuttingDownState;
                dontFork = true;
                break;
            case 'I':
                theInitialState = qtssIdleState;
                break;
            case 'h':
                usage();
                ::exit(0);
            default:
                break;
        }
    }
    
  
    // Check port
    if (thePort < 0 || thePort > 65535)
    { 
        qtss_printf("Invalid port value = %d max value = 65535\n",thePort);
        exit (-1);
    }

    // Check expiration date
    QTSSExpirationDate::PrintExpirationDate();
    if (QTSSExpirationDate::IsSoftwareExpired())
    {
        qtss_printf("Streaming Server has expired\n");
        ::exit(0);
    }


    XMLPrefsParser theXMLParser(theXMLFilePath);
    
    //
    // Check to see if the XML file exists as a directory. If it does,
    // just bail because we do not want to overwrite a directory
    if (theXMLParser.DoesFileExistAsDirectory())
    {
        qtss_printf("Directory located at location where streaming server prefs file should be.\n");
        exit(-1);
    }
    
    //
    // Check to see if we can write to the file
    if (!theXMLParser.CanWriteFile())
    {
        qtss_printf("Cannot write to the streaming server prefs file.\n");
        exit(-1);
    }

    // If we aren't forced to create a new XML prefs file, whether
    // we do or not depends solely on whether the XML prefs file exists currently.
    if (theXMLPrefsExist)
        theXMLPrefsExist = theXMLParser.DoesFileExist();
    
    if (!theXMLPrefsExist)
    {
        
        //
        // The XML prefs file doesn't exist, so let's create an old-style
        // prefs source in order to generate a fresh XML prefs file.
        
        if (theConfigFilePath != NULL)
        {   
            FilePrefsSource* filePrefsSource = new FilePrefsSource(true); // Allow dups
            
            if ( filePrefsSource->InitFromConfigFile(theConfigFilePath) )
            { 
               qtss_printf("Generating a new prefs file at %s\n", theXMLFilePath);
            }

            if (GenerateAllXMLPrefs(filePrefsSource, &theXMLParser))
            {
                qtss_printf("Fatal Error: Could not create new prefs file at: %s. (%d)\n", theXMLFilePath, OSThread::GetErrno());
                ::exit(-1);
            }
        }
    }

 
    //
    // Parse the configs from the XML file
    int xmlParseErr = theXMLParser.Parse();
    if (xmlParseErr)
    {
        qtss_printf("Fatal Error: Could not load configuration file at %s. (%d)\n", theXMLFilePath, OSThread::GetErrno());
        ::exit(-1);
    }
    
    //Unless the command line option is set, fork & daemonize the process at this point
    if (!dontFork)
    {
#ifdef __sgi__
		// for some reason, this method doesn't work right on IRIX 6.4 unless the first arg
		// is _DF_NOFORK.  if the first arg is 0 (as it should be) the result is a server
		// that is essentially paralized and doesn't respond to much at all.  So for now,
		// leave the first arg as _DF_NOFORK
//		if (_daemonize(_DF_NOFORK, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO) != 0)
        if (_daemonize(0, STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO) != 0)
#else
        if (daemon(0,0) != 0)
#endif
        {
#if DEBUG
            qtss_printf("Failed to daemonize process. Error = %d\n", OSThread::GetErrno());
#endif
            exit(-1);
        }
    }
    
    //Construct a Prefs Source object to get server text messages
    FilePrefsSource theMessagesSource;
    theMessagesSource.InitFromConfigFile("qtssmessages.txt");
    

    int status = 0;
    int pid = 0;
    pid_t processID = 0;
	
    if ( !dontFork) // if (fork) 
    {
        //loop until the server exits normally. If the server doesn't exit
        //normally, then restart it.
        // normal exit means the following
        // the child quit 
        do // fork at least once but stop on the status conditions returned by wait or if autoStart pref is false
        {
            processID = fork();
            Assert(processID >= 0);
            if (processID > 0) // this is the parent and we have a child
            {
                sChildPID = processID;
                status = 0;
                while (status == 0) //loop on wait until status is != 0;
                {	
                 	pid =::wait(&status);
                 	SInt8 exitStatus = (SInt8) WEXITSTATUS(status);
                	//qtss_printf("Child Process %d wait exited with pid=%d status=%d exit status=%d\n", processID, pid, status, exitStatus);
                	
					if (WIFEXITED(status) && pid > 0 && status != 0) // child exited with status -2 restart or -1 don't restart 
					{
						//qtss_printf("child exited with status=%d\n", exitStatus);
						
						if ( exitStatus == -1) // child couldn't run don't try again
						{
							qtss_printf("child exited with -1 fatal error so parent is exiting too.\n");
							exit (EXIT_FAILURE); 
						}
						break; // restart the child
							
					}
					
					if (WIFSIGNALED(status)) // child exited on an unhandled signal (maybe a bus error or seg fault)
					{	
						//qtss_printf("child was signalled\n");
						break; // restart the child
					}

                 		
                	if (pid == -1 && status == 0) // parent woken up by a handled signal
                   	{
						//qtss_printf("handled signal continue waiting\n");
                   		continue;
                   	}
                   	
                 	if (pid > 0 && status == 0)
                 	{
                 		//qtss_printf("child exited cleanly so parent is exiting\n");
                 		exit(EXIT_SUCCESS);                		
                	}
                	
                	//qtss_printf("child died for unknown reasons parent is exiting\n");
                	exit (EXIT_FAILURE);
                }
            }
            else if (processID == 0) // must be the child
				break;
            else
            	exit(EXIT_FAILURE);
            	
            	
            //eek. If you auto-restart too fast, you might start the new one before the OS has
            //cleaned up from the old one, resulting in startup errors when you create the new
            //one. Waiting for a second seems to work
            sleep(1);
        } while (RestartServer(theXMLFilePath)); // fork again based on pref if server dies
        if (processID != 0) //the parent is quitting
        	exit(EXIT_SUCCESS);   

        
    }
    sChildPID = 0;
    //we have to do this again for the child process, because sigaction states
    //do not span multiple processes.
    (void)::sigaction(SIGPIPE, &act, NULL);
    (void)::sigaction(SIGHUP, &act, NULL);
    (void)::sigaction(SIGINT, &act, NULL);
    (void)::sigaction(SIGTERM, &act, NULL);
    (void)::sigaction(SIGQUIT, &act, NULL);

#ifdef __hpux__  
	// Set Priority Type to Real Time, timeslice = 100 milliseconds. Change the timeslice upwards as needed. This keeps the server priority above the playlist broadcaster which is a time-share scheduling type.
	char commandStr[64];
	qtss_sprintf(commandStr, "/usr/bin/rtprio -t -%d", (int) getpid()); 
#if DEBUG
	qtss_printf("setting priority to Real Time: %s\n", commandStr);
#endif
	(void) ::system(commandStr);    
#endif
    
#ifdef __solaris__  
    // Set Priority Type to Real Time, timeslice = 100 milliseconds. Change the timeslice upwards as needed. This keeps the server priority above the playlist broadcaster which is a time-share scheduling type.
    char commandStr[64];
    qtss_sprintf(commandStr, "priocntl -s -c RT -t 10 -i pid %d", (int) getpid()); 
    (void) ::system(commandStr);    
#endif

#ifdef __MacOSX__
    (void) ::umask(S_IWGRP|S_IWOTH); // make sure files are opened with default of owner -rw-r-r-
#endif

    //This function starts, runs, and shuts down the server
    if (::StartServer(&theXMLParser, &theMessagesSource, thePort, statsUpdateInterval, theInitialState, dontFork, debugLevel, debugOptions) != qtssFatalErrorState)
    {    ::RunServer();
         CleanPid(false);
         exit (EXIT_SUCCESS);
    }
    else
    	exit(-1); //Cant start server don't try again
}
