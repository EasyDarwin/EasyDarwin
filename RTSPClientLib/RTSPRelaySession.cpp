/*
	Copyright (c) 2013-2014 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       RTSPRelaySession.cpp
    Contains:   RTSP Relay Client
*/
#include "RTSPRelaySession.h"
#include <stdlib.h>

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const RTSPClient& rtspClient) {
  return env << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env, const MediaSubsession& subsession) {
  return env << subsession.mediumName() << "/" << subsession.codecName();
}

void shutdownStream(RTSPClient* rtspClient, int exitCode = 1);

class StreamClientState {
	public:
	  StreamClientState();
	  virtual ~StreamClientState();

	public:
	  MediaSubsessionIterator* iter;
	  MediaSession* session;
	  MediaSubsession* subsession;
	  TaskToken streamTimerTask;
	  double duration;
};

StreamClientState::StreamClientState()
  : iter(NULL), session(NULL), subsession(NULL), streamTimerTask(NULL), duration(0.0) {
}

StreamClientState::~StreamClientState() {
  delete iter;
  if (session != NULL) {
    // We also need to delete "session", and unschedule "streamTimerTask" (if set)
    UsageEnvironment& env = session->envir(); // alias

    env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
    Medium::close(session);
  }
}


class easyRTSPClient: public RTSPClient {
public:
  static easyRTSPClient* createNew(UsageEnvironment& env, 
									char const* rtspURL,
									RTSPRelaySession* relaySession,
									bool rtpOverTCP = True,
									int verbosityLevel = 0,
									char const* applicationName = NULL,
									portNumBits tunnelOverHTTPPortNum = 0);

protected:
  easyRTSPClient(UsageEnvironment& env, char const* rtspURL, RTSPRelaySession* relaySession,
		bool rtpOverTCP, int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum);
    // called only by createNew();
  virtual ~easyRTSPClient();

public:
  StreamClientState scs;
  char fLive555WatchVariable;
  StrPtrLen fSdpDescription;
  Boolean fRTPOverTCP;
  RTSPRelaySession* fRelaySession;
  Boolean fLiveClient;

public:
	RTSPRelaySession* GetRelaySession() {return fRelaySession; }
	void SetRelaySession(RTSPRelaySession* inValue) { fRelaySession = inValue;}
};


// Implementation of "easyRTSPClient":

easyRTSPClient* easyRTSPClient::createNew(UsageEnvironment& env, char const* rtspURL, RTSPRelaySession* relaySession, 
	bool rtpOverTCP, int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum) 
{
	return new easyRTSPClient(env, rtspURL, relaySession, rtpOverTCP, verbosityLevel,  applicationName, tunnelOverHTTPPortNum);
}

easyRTSPClient::easyRTSPClient(UsageEnvironment& env, char const* rtspURL, RTSPRelaySession* relaySession, 
	bool rtpOverTCP, int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum)
  : RTSPClient(env,rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1),
  fSdpDescription(NULL),
  fRTPOverTCP(rtpOverTCP),
  fRelaySession(relaySession),
  fLiveClient(True)
{}

easyRTSPClient::~easyRTSPClient() {
	fLive555WatchVariable = ~0;
	delete [] fSdpDescription.Ptr;
}

///////////////////////live555Thread Implement////////////////////////////

live555Thread::live555Thread(QTSS_Object inScheduler, QTSS_Object inEnv) 
	: OSThread(),
	fLive555TaskScheaduler(inScheduler),
	fLive555Env(inEnv),
	fLive555EventLoopWatchVariable(0),
	fLive555EventLoopWatchVariablePtr(NULL)
{
	this->Start();
}

live555Thread::~live555Thread() 
{
	printf("~live555Thread loop \n");
	fLive555EventLoopWatchVariable = ~0;
	if(fLive555EventLoopWatchVariablePtr)
		*fLive555EventLoopWatchVariablePtr = ~0;
	this->StopAndWaitForThread();

	if(fLive555Env) 
	{
		((UsageEnvironment*)fLive555Env)->reclaim();
		fLive555Env = NULL;
	}
	printf("~live555Thread loop fLive555Env delete \n");
	if(fLive555TaskScheaduler) 
	{
		delete fLive555TaskScheaduler;
		fLive555TaskScheaduler = NULL;
	}
	printf("~live555Thread loop fLive555TaskScheaduler delete\n");
}
	
// This task runs once an hour to check and see if the log needs to roll.
void live555Thread::Entry()
{
	OSMutexLocker locker(&fMutex);
	if(fLive555Env == NULL)
		return;

	qtss_printf("live555Thread loop start\n");

	UsageEnvironment* env = (UsageEnvironment*)fLive555Env;
	fLive555EventLoopWatchVariablePtr = &fLive555EventLoopWatchVariable;
	env->taskScheduler().doEventLoop(fLive555EventLoopWatchVariablePtr);
	qtss_printf("live555Thread loop over\n");	
}

void live555Thread::live555EventLoop(char* watchVariable)
{
	if(fLive555Env == NULL)
		return;

	//停止主线程
	this->fLive555EventLoopWatchVariable = ~0;
	OSMutexLocker locker(&fMutex);
	//等待线程
	qtss_printf("live555Thread event loop start\n");
	UsageEnvironment* env = (UsageEnvironment*)fLive555Env;
	fLive555EventLoopWatchVariablePtr = watchVariable;
	env->taskScheduler().doEventLoop(watchVariable);
	fLive555EventLoopWatchVariablePtr = NULL;
	qtss_printf("live555Thread event loop over\n");

	//重新开始主线程
	fLive555EventLoopWatchVariable = 0;
	fLive555EventLoopWatchVariablePtr = &fLive555EventLoopWatchVariable;
	
	//locker.Unlock();
	this->Start();
}

///////////////////////live555Thread Implement////////////////////////////

// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video 'substream').
// In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming audio or video.
// Or it might be a "FileSink", for outputting the received data into a file (as is done by the "openRTSP" application).
// In this example code, however, we define a simple 'dummy' sink that receives incoming data, but does nothing with it.

class EasyRelaySink: public MediaSink {
public:
  static EasyRelaySink* createNew(UsageEnvironment& env,
			      MediaSubsession& subsession, // identifies the kind of data that's being received
			      RTSPRelaySession* relaySession = NULL); // identifies the stream itself (optional)

private:
  EasyRelaySink(UsageEnvironment& env, MediaSubsession& subsession, RTSPRelaySession* relaySession);
    // called only by "createNew()"
  virtual ~EasyRelaySink();

  static void afterGettingFrame(void* clientData, unsigned frameSize,
                                unsigned numTruncatedBytes,
				struct timeval presentationTime,
                                unsigned durationInMicroseconds);
  void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
			 struct timeval presentationTime, unsigned durationInMicroseconds);

private:
  // redefined virtual functions:
  virtual Boolean continuePlaying();

private:
  u_int8_t* fReceiveBuffer;
  MediaSubsession& fSubsession;
  RTSPRelaySession* fRelaySession;
};


// RTSP 'response handlers':
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);

// Other event handler functions:
void subsessionAfterPlaying(void* clientData); // called when a stream's subsession (e.g., audio or video substream) ends
void subsessionByeHandler(void* clientData); // called when a RTCP "BYE" is received for a subsession
void streamTimerHandler(void* clientData);

// Used to iterate through each stream's 'subsessions', setting up each one:
void setupNextSubsession(RTSPClient* rtspClient);
// Used to shut down and close a stream (including its "RTSPClient" object):
////void shutdownStream(RTSPClient* rtspClient, int exitCode = 1);


RTSPRelaySession::RTSPRelaySession(char* inURL, ClientType inClientType, UInt32 inOptionsIntervalInSec, 
								   Bool16 sendOptions, const char* streamName)
:   fLive555Client(NULL),
    //fTimeoutTask(this, 20000),
	fLive555LoopThread(NULL),
    fOptionsIntervalInSec(inOptionsIntervalInSec),
    
    fOptions(sendOptions),

	fReflectorSession(NULL),
	fStreamName(NULL),
	fURL(NULL)
{
    this->SetTaskName("RTSPRelaySession");

	//建立live555线程
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

	fLive555LoopThread = NEW live555Thread(scheduler, env);
	fLive555Client = (QTSS_Object)easyRTSPClient::createNew(*env, inURL, this, inClientType);


    StrPtrLen theURL(inURL);

	fURL = NEW char[::strlen(inURL) + 2];
	::strcpy(fURL, inURL);

	if (streamName != NULL)
    {
		fStreamName.Ptr = NEW char[strlen(streamName) + 1];
		::memset(fStreamName.Ptr,0,strlen(streamName) + 1);
		::memcpy(fStreamName.Ptr, streamName, strlen(streamName));
		fStreamName.Len = strlen(streamName);
		fRef.Set(fStreamName, this);
    }

	qtss_printf("\nNew Connection %s:%s\n",fStreamName.Ptr,fURL);
}

RTSPRelaySession::~RTSPRelaySession()
{
	qtss_printf("\nDisconnect %s:%s\n",fStreamName.Ptr,fURL);

	delete [] fStreamName.Ptr;

	if(fLive555Client)
	{
		shutdownStream((RTSPClient*)fLive555Client);
		fLive555Client = NULL;
	}

	if(fLive555LoopThread)
		delete fLive555LoopThread;


	qtss_printf("Disconnect complete\n");
	//if(fClient)
	//	delete fClient;
}

SInt64 RTSPRelaySession::Run()
{
    EventFlags theEvents = this->GetEvents();

	if (theEvents & Task::kKillEvent)
    {
        return -1;
    }

    return 0;
}


QTSS_Error RTSPRelaySession::SendDescribe()
{
	QTSS_Error theErr = QTSS_RequestFailed;
	easyRTSPClient* rtspClient = (easyRTSPClient*)fLive555Client;
	UsageEnvironment& env = rtspClient->envir(); 

	rtspClient->fLive555WatchVariable = 0;
	rtspClient->sendDescribeCommand(continueAfterDESCRIBE);
	env<< "RTSPRelaySession Send Describe" << "\n";
	fLive555LoopThread->live555EventLoop(&rtspClient->fLive555WatchVariable);

	if(rtspClient->fLiveClient && rtspClient->fSdpDescription.Len)
	{
		fSDPParser.Parse(rtspClient->fSdpDescription.Ptr, rtspClient->fSdpDescription.Len);
		theErr = QTSS_NoErr;
	}
	else
	{
		theErr = QTSS_RequestFailed;
	}

	return theErr;
}

// Implementation of the RTSP 'response handlers':

void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) {
  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((easyRTSPClient*)rtspClient)->scs; // alias
	
    if (resultCode != 0) {
      env << *rtspClient << "Failed to get a SDP description: " << resultString << "\n";
      delete[] resultString;
      break;
    }

    char* const sdpDescription = resultString;
    env << *rtspClient << "Got a SDP description:\n" << sdpDescription << "\n";
	//此过程可以做到easyRTSPClient类里面
	if (sdpDescription != NULL)
    {
		((easyRTSPClient*)rtspClient)->fSdpDescription.Ptr = NEW char[strlen(sdpDescription) + 1];
		::memset(((easyRTSPClient*)rtspClient)->fSdpDescription.Ptr,0,strlen(sdpDescription) + 1);
        ::memcpy(((easyRTSPClient*)rtspClient)->fSdpDescription.Ptr, sdpDescription, strlen(sdpDescription));
        ((easyRTSPClient*)rtspClient)->fSdpDescription.Len = strlen(sdpDescription);
    }

    // Create a media session object from this SDP description:
    scs.session = MediaSession::createNew(env, sdpDescription);
    delete[] sdpDescription; // because we don't need it anymore
    if (scs.session == NULL) {
      env << *rtspClient << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
      break;
    } else if (!scs.session->hasSubsessions()) {
      env << *rtspClient << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
      break;
    }



    // Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
    // calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
    // (Each 'subsession' will have its own data source.)
    scs.iter = new MediaSubsessionIterator(*scs.session);
    setupNextSubsession(rtspClient);

    return;
  } while (0);

  // An unrecoverable error occurred with this stream.
  //shutdownStream(rtspClient);
  ((easyRTSPClient*)rtspClient)->fLiveClient = false;
  ((easyRTSPClient*)rtspClient)->fLive555WatchVariable = ~0;
}

void setupNextSubsession(RTSPClient* rtspClient) {
  UsageEnvironment& env = rtspClient->envir(); // alias
  StreamClientState& scs = ((easyRTSPClient*)rtspClient)->scs; // alias
  
  scs.subsession = scs.iter->next();
  if (scs.subsession != NULL) {
    if (!scs.subsession->initiate(LIVEMEDIA_EASYDARWIN_RELAY_MARK)) {
      env << *rtspClient << "Failed to initiate the \"" << *scs.subsession << "\" subsession: " << env.getResultMsg() << "\n";
      setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
    } else {
      env << *rtspClient << "Initiated the \"" << *scs.subsession << "\" subsession (";
      if (scs.subsession->rtcpIsMuxed()) {
	env << "client port " << scs.subsession->clientPortNum();
      } else {
	env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1;
      }
      env << ")\n";

      // Continue setting up this subsession, by sending a RTSP "SETUP" command:
	  rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, False, ((easyRTSPClient*)rtspClient)->fRTPOverTCP);
    }
    return;
  }

  // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
  if (scs.session->absStartTime() != NULL) {
    // Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
    rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY, scs.session->absStartTime(), scs.session->absEndTime());
  } else {
    scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
    rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
  }
}

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {
  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((easyRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to set up the \"" << *scs.subsession << "\" subsession: " << resultString << "\n";
      break;
    }

    env << *rtspClient << "Set up the \"" << *scs.subsession << "\" subsession (";
    if (scs.subsession->rtcpIsMuxed()) {
      env << "client port " << scs.subsession->clientPortNum();
    } else {
      env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1;
    }
    env << ")\n";

    // Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
    // (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
    // after we've sent a RTSP "PLAY" command.)

	scs.subsession->sink = EasyRelaySink::createNew(env, *scs.subsession, ((easyRTSPClient*)rtspClient)->GetRelaySession());
      // perhaps use your own custom "MediaSink" subclass instead
    if (scs.subsession->sink == NULL) {
      env << *rtspClient << "Failed to create a data sink for the \"" << *scs.subsession
	  << "\" subsession: " << env.getResultMsg() << "\n";
      break;
    }

    env << *rtspClient << "Created a data sink for the \"" << *scs.subsession << "\" subsession\n";
    scs.subsession->miscPtr = rtspClient; // a hack to let subsession handle functions get the "RTSPClient" from the subsession 
    scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
				       subsessionAfterPlaying, scs.subsession);
    // Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
    if (scs.subsession->rtcpInstance() != NULL) {
      scs.subsession->rtcpInstance()->setByeHandler(subsessionByeHandler, scs.subsession);
    }
  } while (0);
  delete[] resultString;

  // Set up the next subsession, if any:
  setupNextSubsession(rtspClient);
}

void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString) {
  Boolean success = False;

  do {
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((easyRTSPClient*)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to start playing session: " << resultString << "\n";
      break;
    }

    // Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
    // using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
    // 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
    // (Alternatively, if you don't want to receive the entire stream, you could set this timer for some shorter value.)
    if (scs.duration > 0) {
      unsigned const delaySlop = 2; // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
      scs.duration += delaySlop;
      unsigned uSecsToDelay = (unsigned)(scs.duration*1000000);
      scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)streamTimerHandler, rtspClient);
    }

    env << *rtspClient << "Started playing session";
    if (scs.duration > 0) {
      env << " (for up to " << scs.duration << " seconds)";
    }
    env << "...\n";

    success = True;
  } while (0);
  delete[] resultString;

  if (!success) {
    // An unrecoverable error occurred with this stream.
    //shutdownStream(rtspClient);
	((easyRTSPClient*)rtspClient)->fLiveClient = false;
  }

  ((easyRTSPClient*)rtspClient)->fLive555WatchVariable = ~0;
}


// Implementation of the other event handlers:

void subsessionAfterPlaying(void* clientData) {
  MediaSubsession* subsession = (MediaSubsession*)clientData;
  RTSPClient* rtspClient = (RTSPClient*)(subsession->miscPtr);

  // Begin by closing this subsession's stream:
  Medium::close(subsession->sink);
  subsession->sink = NULL;

  // Next, check whether *all* subsessions' streams have now been closed:
  MediaSession& session = subsession->parentSession();
  MediaSubsessionIterator iter(session);
  while ((subsession = iter.next()) != NULL) {
    if (subsession->sink != NULL) return; // this subsession is still active
  }

  // All subsessions' streams have now been closed, so shutdown the client:
  shutdownStream(rtspClient);
}

void subsessionByeHandler(void* clientData) {
  MediaSubsession* subsession = (MediaSubsession*)clientData;
  RTSPClient* rtspClient = (RTSPClient*)subsession->miscPtr;
  UsageEnvironment& env = rtspClient->envir(); // alias

  env << *rtspClient << "Received RTCP \"BYE\" on \"" << *subsession << "\" subsession\n";

  // Now act as if the subsession had closed:
  subsessionAfterPlaying(subsession);
}

void streamTimerHandler(void* clientData) {
  easyRTSPClient* rtspClient = (easyRTSPClient*)clientData;
  StreamClientState& scs = rtspClient->scs; // alias

  scs.streamTimerTask = NULL;

  // Shut down the stream:
  shutdownStream(rtspClient);
}

void shutdownStream(RTSPClient* rtspClient, int exitCode) 
{
  UsageEnvironment& env = rtspClient->envir(); // alias
  StreamClientState& scs = ((easyRTSPClient*)rtspClient)->scs; // alias

  // First, check whether any subsessions have still to be closed:
  if (scs.session != NULL) { 
    Boolean someSubsessionsWereActive = False;
    MediaSubsessionIterator iter(*scs.session);
    MediaSubsession* subsession;

    while ((subsession = iter.next()) != NULL) {
      if (subsession->sink != NULL) {
	Medium::close(subsession->sink);
	subsession->sink = NULL;

	if (subsession->rtcpInstance() != NULL) {
	  subsession->rtcpInstance()->setByeHandler(NULL, NULL); // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
	}

	someSubsessionsWereActive = True;
      }
    }

    if (someSubsessionsWereActive) {
      // Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
      // Don't bother handling the response to the "TEARDOWN".
      rtspClient->sendTeardownCommand(*scs.session, NULL);
    }
  }
  //((easyRTSPClient*)rtspClient)->fLive555WatchVariable = ~0;
  env << *rtspClient << "Closing the stream.\n";
  Medium::close(rtspClient);
    // Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.
}

// Implementation of "EasyRelaySink":

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 100000

EasyRelaySink* EasyRelaySink::createNew(UsageEnvironment& env, MediaSubsession& subsession, RTSPRelaySession* relaySession) {
  return new EasyRelaySink(env, subsession, relaySession);
}

EasyRelaySink::EasyRelaySink(UsageEnvironment& env, MediaSubsession& subsession, RTSPRelaySession* relaySession)
  : MediaSink(env),
    fSubsession(subsession) {
  fRelaySession = relaySession;
  fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
}

EasyRelaySink::~EasyRelaySink() {
  delete[] fReceiveBuffer;
  fRelaySession = NULL;
}

void EasyRelaySink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned durationInMicroseconds) {
  EasyRelaySink* sink = (EasyRelaySink*)clientData;
  sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

// If you don't want to see debugging output for each received frame, then comment out the following line:
#define DEBUG_PRINT_EACH_RECEIVED_FRAME 1

void EasyRelaySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
				  struct timeval presentationTime, unsigned /*durationInMicroseconds*/) {
  // We've just received a frame of data.  (Optionally) print out information about it:
//#ifdef DEBUG_PRINT_EACH_RECEIVED_FRAME
//  envir() << fSubsession.mediumName() << "/" << fSubsession.codecName() << ":\tReceived " << frameSize << " bytes";
//  if (numTruncatedBytes > 0) envir() << " (with " << numTruncatedBytes << " bytes truncated)";
//  char uSecsStr[6+1]; // used to output the 'microseconds' part of the presentation time
//  sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);
//  envir() << ".\tPresentation time: " << (int)presentationTime.tv_sec << "." << uSecsStr;
//  if (fSubsession.rtpSource() != NULL && !fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP()) {
//    envir() << "!"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
//  }
//#ifdef DEBUG_PRINT_NPT
//  envir() << "\tNPT: " << fSubsession.getNormalPlayTime(presentationTime);
//#endif
//  envir() << "\n";
//#endif


  QTSS_ReflectRTPData(fRelaySession, (char*)fReceiveBuffer, frameSize, fSubsession.trackIndex());
  
  // Then continue, to request the next frame of data:
  continuePlaying();
}

Boolean EasyRelaySink::continuePlaying() {
  if (fSource == NULL) return False; // sanity check (should not happen)

  // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
  fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE,
                        afterGettingFrame, this,
                        onSourceClosure, this);
  return True;
}
