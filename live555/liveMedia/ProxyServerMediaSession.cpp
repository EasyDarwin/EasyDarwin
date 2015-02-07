/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2015 Live Networks, Inc.  All rights reserved.
// A subclass of "ServerMediaSession" that can be used to create a (unicast) RTSP servers that acts as a 'proxy' for
// another (unicast or multicast) RTSP/RTP stream.
// Implementation

#include "liveMedia.hh"
#include "RTSPCommon.hh"
#include "GroupsockHelper.hh" // for "our_random()"

#ifndef MILLION
#define MILLION 1000000
#endif

// A "OnDemandServerMediaSubsession" subclass, used to implement a unicast RTSP server that's proxying another RTSP stream:

class ProxyServerMediaSubsession: public OnDemandServerMediaSubsession {
public:
  ProxyServerMediaSubsession(MediaSubsession& mediaSubsession);
  virtual ~ProxyServerMediaSubsession();

  char const* codecName() const { return fClientMediaSubsession.codecName(); }

private: // redefined virtual functions
  virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
                                              unsigned& estBitrate);
  virtual void closeStreamSource(FramedSource *inputSource);
  virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                    unsigned char rtpPayloadTypeIfDynamic,
                                    FramedSource* inputSource);

private:
  static void subsessionByeHandler(void* clientData);
  void subsessionByeHandler();

  int verbosityLevel() const { return ((ProxyServerMediaSession*)fParentSession)->fVerbosityLevel; }

private:
  friend class ProxyRTSPClient;
  MediaSubsession& fClientMediaSubsession; // the 'client' media subsession object that corresponds to this 'server' media subsession
  ProxyServerMediaSubsession* fNext; // used when we're part of a queue
  Boolean fHaveSetupStream;
};


////////// ProxyServerMediaSession implementation //////////

UsageEnvironment& operator<<(UsageEnvironment& env, const ProxyServerMediaSession& psms) { // used for debugging
  return env << "ProxyServerMediaSession[\"" << psms.url() << "\"]";
}

ProxyRTSPClient*
defaultCreateNewProxyRTSPClientFunc(ProxyServerMediaSession& ourServerMediaSession,
				    char const* rtspURL,
				    char const* username, char const* password,
				    portNumBits tunnelOverHTTPPortNum, int verbosityLevel,
				    int socketNumToServer) {
  return new ProxyRTSPClient(ourServerMediaSession, rtspURL, username, password,
			     tunnelOverHTTPPortNum, verbosityLevel, socketNumToServer);
}

ProxyServerMediaSession* ProxyServerMediaSession
::createNew(UsageEnvironment& env, RTSPServer* ourRTSPServer,
	    char const* inputStreamURL, char const* streamName,
	    char const* username, char const* password,
	    portNumBits tunnelOverHTTPPortNum, int verbosityLevel, int socketNumToServer) {
  return new ProxyServerMediaSession(env, ourRTSPServer, inputStreamURL, streamName, username, password,
				     tunnelOverHTTPPortNum, verbosityLevel, socketNumToServer);
}


ProxyServerMediaSession
::ProxyServerMediaSession(UsageEnvironment& env, RTSPServer* ourRTSPServer,
			  char const* inputStreamURL, char const* streamName,
			  char const* username, char const* password,
			  portNumBits tunnelOverHTTPPortNum, int verbosityLevel,
			  int socketNumToServer,
			  createNewProxyRTSPClientFunc* ourCreateNewProxyRTSPClientFunc)
  : ServerMediaSession(env, streamName, NULL, NULL, False, NULL),
    describeCompletedFlag(0), fOurRTSPServer(ourRTSPServer), fClientMediaSession(NULL),
    fVerbosityLevel(verbosityLevel),
    fPresentationTimeSessionNormalizer(new PresentationTimeSessionNormalizer(envir())),
    fCreateNewProxyRTSPClientFunc(ourCreateNewProxyRTSPClientFunc) {
  // Open a RTSP connection to the input stream, and send a "DESCRIBE" command.
  // We'll use the SDP description in the response to set ourselves up.
  fProxyRTSPClient
    = (*fCreateNewProxyRTSPClientFunc)(*this, inputStreamURL, username, password,
				       tunnelOverHTTPPortNum,
				       verbosityLevel > 0 ? verbosityLevel-1 : verbosityLevel,
				       socketNumToServer);
  ProxyRTSPClient::sendDESCRIBE(fProxyRTSPClient);
}

ProxyServerMediaSession::~ProxyServerMediaSession() {
  if (fVerbosityLevel > 0) {
    envir() << *this << "::~ProxyServerMediaSession()\n";
  }

  // Begin by sending a "TEARDOWN" command (without checking for a response):
  if (fProxyRTSPClient != NULL) fProxyRTSPClient->sendTeardownCommand(*fClientMediaSession, NULL, fProxyRTSPClient->auth());

  // Then delete our state:
  Medium::close(fClientMediaSession);
  Medium::close(fProxyRTSPClient);
  delete fPresentationTimeSessionNormalizer;
}

char const* ProxyServerMediaSession::url() const {
  return fProxyRTSPClient == NULL ? NULL : fProxyRTSPClient->url();
}

void ProxyServerMediaSession::continueAfterDESCRIBE(char const* sdpDescription) {
  describeCompletedFlag = 1;

  // Create a (client) "MediaSession" object from the stream's SDP description ("resultString"), then iterate through its
  // "MediaSubsession" objects, to set up corresponding "ServerMediaSubsession" objects that we'll use to serve the stream's tracks.
  do {
    fClientMediaSession = MediaSession::createNew(envir(), sdpDescription);
    if (fClientMediaSession == NULL) break;

    MediaSubsessionIterator iter(*fClientMediaSession);
    for (MediaSubsession* mss = iter.next(); mss != NULL; mss = iter.next()) {
      ServerMediaSubsession* smss = new ProxyServerMediaSubsession(*mss);
      addSubsession(smss);
      if (fVerbosityLevel > 0) {
	envir() << *this << " added new \"ProxyServerMediaSubsession\" for "
		<< mss->protocolName() << "/" << mss->mediumName() << "/" << mss->codecName() << " track\n";
      }
    }
  } while (0);
}

void ProxyServerMediaSession::resetDESCRIBEState() {
  // Delete all of our "ProxyServerMediaSubsession"s; they'll get set up again once we get a response to the new "DESCRIBE".
  if (fOurRTSPServer != NULL) {
    // First, close any RTSP client connections that may have already been set up:
    fOurRTSPServer->closeAllClientSessionsForServerMediaSession(this);
  }
  deleteAllSubsessions();

  // Finally, delete the client "MediaSession" object that we had set up after receiving the response to the previous "DESCRIBE":
  Medium::close(fClientMediaSession); fClientMediaSession = NULL;
}

///////// RTSP 'response handlers' //////////

static void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) {
  char const* res;

  if (resultCode == 0) {
    // The "DESCRIBE" command succeeded, so "resultString" should be the stream's SDP description.
    res = resultString;
  } else {
    // The "DESCRIBE" command failed.
    res = NULL;
  }
  ((ProxyRTSPClient*)rtspClient)->continueAfterDESCRIBE(res);
  delete[] resultString;
}

static void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) {
  if (resultCode == 0) {
    ((ProxyRTSPClient*)rtspClient)->continueAfterSETUP();
  }
  delete[] resultString;
}

static void continueAfterOPTIONS(RTSPClient* rtspClient, int resultCode, char* resultString) {
  Boolean serverSupportsGetParameter = False;
  if (resultCode == 0) {
    // Note whether the server told us that it supports the "GET_PARAMETER" command:
    serverSupportsGetParameter = RTSPOptionIsSupported("GET_PARAMETER", resultString);
  }
  ((ProxyRTSPClient*)rtspClient)->continueAfterLivenessCommand(resultCode, serverSupportsGetParameter);
  delete[] resultString;
}

#ifdef SEND_GET_PARAMETER_IF_SUPPORTED
static void continueAfterGET_PARAMETER(RTSPClient* rtspClient, int resultCode, char* resultString) {
  ((ProxyRTSPClient*)rtspClient)->continueAfterLivenessCommand(resultCode, True);
  delete[] resultString;
}
#endif


////////// "ProxyRTSPClient" implementation /////////

UsageEnvironment& operator<<(UsageEnvironment& env, const ProxyRTSPClient& proxyRTSPClient) { // used for debugging
  return env << "ProxyRTSPClient[\"" << proxyRTSPClient.url() << "\"]";
}

ProxyRTSPClient::ProxyRTSPClient(ProxyServerMediaSession& ourServerMediaSession, char const* rtspURL,
				 char const* username, char const* password,
				 portNumBits tunnelOverHTTPPortNum, int verbosityLevel, int socketNumToServer)
  : RTSPClient(ourServerMediaSession.envir(), rtspURL, verbosityLevel, "ProxyRTSPClient",
	       tunnelOverHTTPPortNum == (portNumBits)(~0) ? 0 : tunnelOverHTTPPortNum, socketNumToServer),
    fOurServerMediaSession(ourServerMediaSession), fOurURL(strDup(rtspURL)), fStreamRTPOverTCP(tunnelOverHTTPPortNum != 0),
    fSetupQueueHead(NULL), fSetupQueueTail(NULL), fNumSetupsDone(0), fNextDESCRIBEDelay(1),
    fServerSupportsGetParameter(False), fLastCommandWasPLAY(False),
    fLivenessCommandTask(NULL), fDESCRIBECommandTask(NULL), fSubsessionTimerTask(NULL) { 
  if (username != NULL && password != NULL) {
    fOurAuthenticator = new Authenticator(username, password);
  } else {
    fOurAuthenticator = NULL;
  }
}

void ProxyRTSPClient::reset() {
  envir().taskScheduler().unscheduleDelayedTask(fLivenessCommandTask); fLivenessCommandTask = NULL;
  envir().taskScheduler().unscheduleDelayedTask(fDESCRIBECommandTask); fDESCRIBECommandTask = NULL;
  envir().taskScheduler().unscheduleDelayedTask(fSubsessionTimerTask); fSubsessionTimerTask = NULL;

  fSetupQueueHead = fSetupQueueTail = NULL;
  fNumSetupsDone = 0;
  fNextDESCRIBEDelay = 1;
  fLastCommandWasPLAY = False;

  RTSPClient::reset();
}

ProxyRTSPClient::~ProxyRTSPClient() {
  reset();

  delete fOurAuthenticator;
  delete[] fOurURL;
}

void ProxyRTSPClient::continueAfterDESCRIBE(char const* sdpDescription) {
  if (sdpDescription != NULL) {
    fOurServerMediaSession.continueAfterDESCRIBE(sdpDescription);

    // Unlike most RTSP streams, there might be a long delay between this "DESCRIBE" command (to the downstream server) and the
    // subsequent "SETUP"/"PLAY" - which doesn't occur until the first time that a client requests the stream.
    // To prevent the proxied connection (between us and the downstream server) from timing out, we send periodic 'liveness'
    // ("OPTIONS" or "GET_PARAMETER") commands.  (The usual RTCP liveness mechanism wouldn't work here, because RTCP packets
    // don't get sent until after the "PLAY" command.)
    scheduleLivenessCommand();
  } else {
    // The "DESCRIBE" command failed, most likely because the server or the stream is not yet running.
    // Reschedule another "DESCRIBE" command to take place later:
    scheduleDESCRIBECommand();
  }
}

void ProxyRTSPClient::continueAfterLivenessCommand(int resultCode, Boolean serverSupportsGetParameter) {
  if (resultCode != 0) {
    // The periodic 'liveness' command failed, suggesting that the back-end stream is no longer alive.
    // We handle this by resetting our connection state with this server.  Any current clients will be closed, but
    // subsequent clients will cause new RTSP "SETUP"s and "PLAY"s to get done, restarting the stream.
    // Then continue by sending more "DESCRIBE" commands, to try to restore the stream.

    fServerSupportsGetParameter = False; // until we learn otherwise, in response to a future "OPTIONS" command

    if (resultCode < 0) {
      // The 'liveness' command failed without getting a response from the server (otherwise "resultCode" would have been > 0).
      // This suggests that the RTSP connection itself has failed.  Print this error code, in case it's useful for debugging:
      if (fVerbosityLevel > 0) {
	envir() << *this << ": lost connection to server ('errno': " << -resultCode << ").  Resetting...\n";
      }
    }

    reset();
    fOurServerMediaSession.resetDESCRIBEState();

    setBaseURL(fOurURL); // because we'll be sending an initial "DESCRIBE" all over again
    sendDESCRIBE(this);
    return;
  }

  fServerSupportsGetParameter = serverSupportsGetParameter;

  // Schedule the next 'liveness' command (i.e., to tell the back-end server that we're still alive):
  scheduleLivenessCommand();
}

#define SUBSESSION_TIMEOUT_SECONDS 10 // how many seconds to wait for the last track's "SETUP" to be done (note below)

void ProxyRTSPClient::continueAfterSETUP() {
  if (fVerbosityLevel > 0) {
    envir() << *this << "::continueAfterSETUP(): head codec: " << fSetupQueueHead->fClientMediaSubsession.codecName()
	    << "; numSubsessions " << fSetupQueueHead->fParentSession->numSubsessions() << "\n\tqueue:";
    for (ProxyServerMediaSubsession* p = fSetupQueueHead; p != NULL; p = p->fNext) {
      envir() << "\t" << p->fClientMediaSubsession.codecName();
    }
    envir() << "\n";
  }
  envir().taskScheduler().unscheduleDelayedTask(fSubsessionTimerTask); // in case it had been set

  // Dequeue the first "ProxyServerMediaSubsession" from our 'SETUP queue'.  It will be the one for which this "SETUP" was done:
  ProxyServerMediaSubsession* smss = fSetupQueueHead; // Assert: != NULL
  fSetupQueueHead = fSetupQueueHead->fNext;
  if (fSetupQueueHead == NULL) fSetupQueueTail = NULL;

  if (fSetupQueueHead != NULL) {
    // There are still entries in the queue, for tracks for which we have still to do a "SETUP".
    // "SETUP" the first of these now:
    sendSetupCommand(fSetupQueueHead->fClientMediaSubsession, ::continueAfterSETUP,
		     False, fStreamRTPOverTCP, False, fOurAuthenticator);
    ++fNumSetupsDone;
    fSetupQueueHead->fHaveSetupStream = True;
  } else {
    if (fNumSetupsDone >= smss->fParentSession->numSubsessions()) {
      // We've now finished setting up each of our subsessions (i.e., 'tracks').
      // Continue by sending a "PLAY" command (an 'aggregate' "PLAY" command, on the whole session):
      sendPlayCommand(smss->fClientMediaSubsession.parentSession(), NULL, -1.0f, -1.0f, 1.0f, fOurAuthenticator);
          // the "-1.0f" "start" parameter causes the "PLAY" to be sent without a "Range:" header, in case we'd already done
          // a "PLAY" before (as a result of a 'subsession timeout' (note below))
      fLastCommandWasPLAY = True;
    } else {
      // Some of this session's subsessions (i.e., 'tracks') remain to be "SETUP".  They might get "SETUP" very soon, but it's
      // also possible - if the remote client chose to play only some of the session's tracks - that they might not.
      // To allow for this possibility, we set a timer.  If the timer expires without the remaining subsessions getting "SETUP",
      // then we send a "PLAY" command anyway:
      fSubsessionTimerTask
	= envir().taskScheduler().scheduleDelayedTask(SUBSESSION_TIMEOUT_SECONDS*MILLION, (TaskFunc*)subsessionTimeout, this);
    }
  }
}

void ProxyRTSPClient::scheduleLivenessCommand() {
  // Delay a random time before sending another 'liveness' command.
  unsigned delayMax = sessionTimeoutParameter(); // if the server specified a maximum time between 'liveness' probes, then use that
  if (delayMax == 0) {
    delayMax = 60;
  }

  // Choose a random time from [delayMax/2,delayMax-1) seconds:
  unsigned const us_1stPart = delayMax*500000;
  unsigned uSecondsToDelay;
  if (us_1stPart <= 1000000) {
    uSecondsToDelay = us_1stPart;
  } else {
    unsigned const us_2ndPart = us_1stPart-1000000;
    uSecondsToDelay = us_1stPart + (us_2ndPart*our_random())%us_2ndPart;
  }
  fLivenessCommandTask = envir().taskScheduler().scheduleDelayedTask(uSecondsToDelay, sendLivenessCommand, this);
}

void ProxyRTSPClient::sendLivenessCommand(void* clientData) {
  ProxyRTSPClient* rtspClient = (ProxyRTSPClient*)clientData;

  // Note.  By default, we do not send "GET_PARAMETER" as our 'liveness notification' command, even if the server previously
  // indicated (in its response to our earlier "OPTIONS" command) that it supported "GET_PARAMETER".  This is because
  // "GET_PARAMETER" crashes some camera servers (even though they claimed to support "GET_PARAMETER").
#ifdef SEND_GET_PARAMETER_IF_SUPPORTED
  MediaSession* sess = rtspClient->fOurServerMediaSession.fClientMediaSession;

  if (rtspClient->fServerSupportsGetParameter && rtspClient->fNumSetupsDone > 0 && sess != NULL) {
    rtspClient->sendGetParameterCommand(*sess, ::continueAfterGET_PARAMETER, "", rtspClient->auth());
  } else {
#endif
    rtspClient->sendOptionsCommand(::continueAfterOPTIONS, rtspClient->auth());
#ifdef SEND_GET_PARAMETER_IF_SUPPORTED
  }
#endif
}

void ProxyRTSPClient::scheduleDESCRIBECommand() {
  // Delay 1s, 2s, 4s, 8s ... 256s until sending the next "DESCRIBE".  Then, keep delaying a random time from [256..511] seconds:
  unsigned secondsToDelay;
  if (fNextDESCRIBEDelay <= 256) {
    secondsToDelay = fNextDESCRIBEDelay;
    fNextDESCRIBEDelay *= 2;
  } else {
    secondsToDelay = 256 + (our_random()&0xFF); // [256..511] seconds
  }

  if (fVerbosityLevel > 0) {
    envir() << *this << ": RTSP \"DESCRIBE\" command failed; trying again in " << secondsToDelay << " seconds\n";
  }
  fDESCRIBECommandTask = envir().taskScheduler().scheduleDelayedTask(secondsToDelay*MILLION, sendDESCRIBE, this);
}

void ProxyRTSPClient::sendDESCRIBE(void* clientData) {
  ProxyRTSPClient* rtspClient = (ProxyRTSPClient*)clientData;
  if (rtspClient != NULL) rtspClient->sendDescribeCommand(::continueAfterDESCRIBE, rtspClient->auth());
}

void ProxyRTSPClient::subsessionTimeout(void* clientData) {
  ((ProxyRTSPClient*)clientData)->handleSubsessionTimeout();
}

void ProxyRTSPClient::handleSubsessionTimeout() {
  // We still have one or more subsessions ('tracks') left to "SETUP".  But we can't wait any longer for them.  Send a "PLAY" now:
  MediaSession* sess = fOurServerMediaSession.fClientMediaSession;
  if (sess != NULL) sendPlayCommand(*sess, NULL, -1.0f, -1.0f, 1.0f, fOurAuthenticator);
  fLastCommandWasPLAY = True;
}


//////// "ProxyServerMediaSubsession" implementation //////////

ProxyServerMediaSubsession::ProxyServerMediaSubsession(MediaSubsession& mediaSubsession)
  : OnDemandServerMediaSubsession(mediaSubsession.parentSession().envir(), True/*reuseFirstSource*/),
    fClientMediaSubsession(mediaSubsession), fNext(NULL), fHaveSetupStream(False) {
}

UsageEnvironment& operator<<(UsageEnvironment& env, const ProxyServerMediaSubsession& psmss) { // used for debugging
  return env << "ProxyServerMediaSubsession[\"" << psmss.codecName() << "\"]";
}

ProxyServerMediaSubsession::~ProxyServerMediaSubsession() {
  if (verbosityLevel() > 0) {
    envir() << *this << "::~ProxyServerMediaSubsession()\n";
  }
}

FramedSource* ProxyServerMediaSubsession::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate) {
  ProxyServerMediaSession* const sms = (ProxyServerMediaSession*)fParentSession;

  if (verbosityLevel() > 0) {
    envir() << *this << "::createNewStreamSource(session id " << clientSessionId << ")\n";
  }

  // If we haven't yet created a data source from our 'media subsession' object, initiate() it to do so:
  if (fClientMediaSubsession.readSource() == NULL) {
    fClientMediaSubsession.receiveRawMP3ADUs(); // hack for MPA-ROBUST streams
    fClientMediaSubsession.receiveRawJPEGFrames(); // hack for proxying JPEG/RTP streams. (Don't do this if we're transcoding.)
    fClientMediaSubsession.initiate();
    if (verbosityLevel() > 0) {
      envir() << "\tInitiated: " << *this << "\n";
    }

    if (fClientMediaSubsession.readSource() != NULL) {
      // Add to the front of all data sources a filter that will 'normalize' their frames' presentation times,
      // before the frames get re-transmitted by our server:
      char const* const codecName = fClientMediaSubsession.codecName();
      FramedFilter* normalizerFilter = sms->fPresentationTimeSessionNormalizer
	->createNewPresentationTimeSubsessionNormalizer(fClientMediaSubsession.readSource(), fClientMediaSubsession.rtpSource(),
							codecName);
      fClientMediaSubsession.addFilter(normalizerFilter);

      // Some data sources require a 'framer' object to be added, before they can be fed into
      // a "RTPSink".  Adjust for this now:
      if (strcmp(codecName, "H264") == 0) {
	fClientMediaSubsession.addFilter(H264VideoStreamDiscreteFramer
					 ::createNew(envir(), fClientMediaSubsession.readSource()));
      } else if (strcmp(codecName, "H265") == 0) {
	fClientMediaSubsession.addFilter(H265VideoStreamDiscreteFramer
					 ::createNew(envir(), fClientMediaSubsession.readSource()));
      } else if (strcmp(codecName, "MP4V-ES") == 0) {
	fClientMediaSubsession.addFilter(MPEG4VideoStreamDiscreteFramer
					 ::createNew(envir(), fClientMediaSubsession.readSource(),
						     True/* leave PTs unmodified*/));
      } else if (strcmp(codecName, "MPV") == 0) {
	fClientMediaSubsession.addFilter(MPEG1or2VideoStreamDiscreteFramer
					 ::createNew(envir(), fClientMediaSubsession.readSource(),
						     False, 5.0, True/* leave PTs unmodified*/));
      } else if (strcmp(codecName, "DV") == 0) {
	fClientMediaSubsession.addFilter(DVVideoStreamFramer
					 ::createNew(envir(), fClientMediaSubsession.readSource(),
						     False, True/* leave PTs unmodified*/));
      }
    }

    if (fClientMediaSubsession.rtcpInstance() != NULL) {
      fClientMediaSubsession.rtcpInstance()->setByeHandler(subsessionByeHandler, this);
    }
  }

  ProxyRTSPClient* const proxyRTSPClient = sms->fProxyRTSPClient;
  if (clientSessionId != 0) {
    // We're being called as a result of implementing a RTSP "SETUP".
    if (!fHaveSetupStream) {
      // This is our first "SETUP".  Send RTSP "SETUP" and later "PLAY" commands to the proxied server, to start streaming:
      // (Before sending "SETUP", enqueue ourselves on the "RTSPClient"s 'SETUP queue', so we'll be able to get the correct
      //  "ProxyServerMediaSubsession" to handle the response.  (Note that responses come back in the same order as requests.))
      Boolean queueWasEmpty = proxyRTSPClient->fSetupQueueHead == NULL;
      if (queueWasEmpty) {
	proxyRTSPClient->fSetupQueueHead = this;
      } else {
	proxyRTSPClient->fSetupQueueTail->fNext = this;
      }
      proxyRTSPClient->fSetupQueueTail = this;

      // Hack: If there's already a pending "SETUP" request (for another track), don't send this track's "SETUP" right away, because
      // the server might not properly handle 'pipelined' requests.  Instead, wait until after previous "SETUP" responses come back.
      if (queueWasEmpty) {
	proxyRTSPClient->sendSetupCommand(fClientMediaSubsession, ::continueAfterSETUP,
					  False, proxyRTSPClient->fStreamRTPOverTCP, False, proxyRTSPClient->auth());
	++proxyRTSPClient->fNumSetupsDone;
	fHaveSetupStream = True;
      }
    } else {
      // This is a "SETUP" from a new client.  We know that there are no other currently active clients (otherwise we wouldn't
      // have been called here), so we know that the substream was previously "PAUSE"d.  Send "PLAY" downstream once again,
      // to resume the stream:
      if (!proxyRTSPClient->fLastCommandWasPLAY) { // so that we send only one "PLAY"; not one for each subsession
	proxyRTSPClient->sendPlayCommand(fClientMediaSubsession.parentSession(), NULL, -1.0f/*resume from previous point*/,
					 -1.0f, 1.0f, proxyRTSPClient->auth());
	proxyRTSPClient->fLastCommandWasPLAY = True;
      }
    }
  }

  estBitrate = fClientMediaSubsession.bandwidth();
  if (estBitrate == 0) estBitrate = 50; // kbps, estimate
  return fClientMediaSubsession.readSource();
}

void ProxyServerMediaSubsession::closeStreamSource(FramedSource* inputSource) {
  if (verbosityLevel() > 0) {
    envir() << *this << "::closeStreamSource()\n";
  }
  // Because there's only one input source for this 'subsession' (regardless of how many downstream clients are proxying it),
  // we don't close the input source here.  (Instead, we wait until *this* object gets deleted.)
  // However, because (as evidenced by this function having been called) we no longer have any clients accessing the stream,
  // then we "PAUSE" the downstream proxied stream, until a new client arrives:
  if (fHaveSetupStream) {
    ProxyServerMediaSession* const sms = (ProxyServerMediaSession*)fParentSession;
    ProxyRTSPClient* const proxyRTSPClient = sms->fProxyRTSPClient;
    if (proxyRTSPClient->fLastCommandWasPLAY) { // so that we send only one "PAUSE"; not one for each subsession
      proxyRTSPClient->sendPauseCommand(fClientMediaSubsession.parentSession(), NULL, proxyRTSPClient->auth());
      proxyRTSPClient->fLastCommandWasPLAY = False;
    }
  }
}

RTPSink* ProxyServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource) {
  if (verbosityLevel() > 0) {
    envir() << *this << "::createNewRTPSink()\n";
  }

  // Create (and return) the appropriate "RTPSink" object for our codec:
  RTPSink* newSink;
  char const* const codecName = fClientMediaSubsession.codecName();
  if (strcmp(codecName, "AC3") == 0 || strcmp(codecName, "EAC3") == 0) {
    newSink = AC3AudioRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
					 fClientMediaSubsession.rtpTimestampFrequency()); 
#if 0 // This code does not work; do *not* enable it:
  } else if (strcmp(codecName, "AMR") == 0 || strcmp(codecName, "AMR-WB") == 0) {
    Boolean isWideband = strcmp(codecName, "AMR-WB") == 0;
    newSink = AMRAudioRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
					 isWideband, fClientMediaSubsession.numChannels());
#endif
  } else if (strcmp(codecName, "DV") == 0) {
    newSink = DVVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
  } else if (strcmp(codecName, "GSM") == 0) {
    newSink = GSMAudioRTPSink::createNew(envir(), rtpGroupsock);
  } else if (strcmp(codecName, "H263-1998") == 0 || strcmp(codecName, "H263-2000") == 0) {
    newSink = H263plusVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
					      fClientMediaSubsession.rtpTimestampFrequency()); 
  } else if (strcmp(codecName, "H264") == 0) {
    newSink = H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
					  fClientMediaSubsession.fmtp_spropparametersets());
  } else if (strcmp(codecName, "H265") == 0) {
    newSink = H265VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
					  fClientMediaSubsession.fmtp_spropvps(),
					  fClientMediaSubsession.fmtp_spropsps(),
					  fClientMediaSubsession.fmtp_sproppps());
  } else if (strcmp(codecName, "JPEG") == 0) {
    newSink = SimpleRTPSink::createNew(envir(), rtpGroupsock, 26, 90000, "video", "JPEG",
				       1/*numChannels*/, False/*allowMultipleFramesPerPacket*/, False/*doNormalMBitRule*/);
  } else if (strcmp(codecName, "MP4A-LATM") == 0) {
    newSink = MPEG4LATMAudioRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
					       fClientMediaSubsession.rtpTimestampFrequency(),
					       fClientMediaSubsession.fmtp_config(),
					       fClientMediaSubsession.numChannels());
  } else if (strcmp(codecName, "MP4V-ES") == 0) {
    newSink = MPEG4ESVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
					     fClientMediaSubsession.rtpTimestampFrequency(),
					     fClientMediaSubsession.attrVal_unsigned("profile-level-id"),
					     fClientMediaSubsession.fmtp_config()); 
  } else if (strcmp(codecName, "MPA") == 0) {
    newSink = MPEG1or2AudioRTPSink::createNew(envir(), rtpGroupsock);
  } else if (strcmp(codecName, "MPA-ROBUST") == 0) {
    newSink = MP3ADURTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
  } else if (strcmp(codecName, "MPEG4-GENERIC") == 0) {
    newSink = MPEG4GenericRTPSink::createNew(envir(), rtpGroupsock,
					     rtpPayloadTypeIfDynamic, fClientMediaSubsession.rtpTimestampFrequency(),
					     fClientMediaSubsession.mediumName(),
					     fClientMediaSubsession.attrVal_strToLower("mode"),
					     fClientMediaSubsession.fmtp_config(), fClientMediaSubsession.numChannels());
  } else if (strcmp(codecName, "MPV") == 0) {
    newSink = MPEG1or2VideoRTPSink::createNew(envir(), rtpGroupsock);
  } else if (strcmp(codecName, "OPUS") == 0) {
    newSink = SimpleRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
				       48000, "audio", "OPUS", 2, False/*only 1 Opus 'packet' in each RTP packet*/);
  } else if (strcmp(codecName, "T140") == 0) {
    newSink = T140TextRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
  } else if (strcmp(codecName, "THEORA") == 0) {
    newSink = TheoraVideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
					    fClientMediaSubsession.fmtp_config()); 
  } else if (strcmp(codecName, "VORBIS") == 0) {
    newSink = VorbisAudioRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
					    fClientMediaSubsession.rtpTimestampFrequency(), fClientMediaSubsession.numChannels(),
					    fClientMediaSubsession.fmtp_config()); 
  } else if (strcmp(codecName, "VP8") == 0) {
    newSink = VP8VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
  } else if (strcmp(codecName, "VP9") == 0) {
    newSink = VP9VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
  } else if (strcmp(codecName, "AMR") == 0 || strcmp(codecName, "AMR-WB") == 0) {
    // Proxying of these codecs is currently *not* supported, because the data received by the "RTPSource" object is not in a
    // form that can be fed directly into a corresponding "RTPSink" object.
    if (verbosityLevel() > 0) {
      envir() << "\treturns NULL (because we currently don't support the proxying of \""
	      << fClientMediaSubsession.mediumName() << "/" << codecName << "\" streams)\n";
    }
    return NULL;
  } else if (strcmp(codecName, "QCELP") == 0 ||
	     strcmp(codecName, "H261") == 0 ||
	     strcmp(codecName, "H263-1998") == 0 || strcmp(codecName, "H263-2000") == 0 ||
	     strcmp(codecName, "X-QT") == 0 || strcmp(codecName, "X-QUICKTIME") == 0) {
    // This codec requires a specialized RTP payload format; however, we don't yet have an appropriate "RTPSink" subclass for it:
    if (verbosityLevel() > 0) {
      envir() << "\treturns NULL (because we don't have a \"RTPSink\" subclass for this RTP payload format)\n";
    }
    return NULL;
  } else {
    // This codec is assumed to have a simple RTP payload format that can be implemented just with a "SimpleRTPSink":
    Boolean allowMultipleFramesPerPacket = True; // by default
    Boolean doNormalMBitRule = True; // by default
    // Some codecs change the above default parameters:
    if (strcmp(codecName, "MP2T") == 0) {
      doNormalMBitRule = False; // no RTP 'M' bit
    }
    newSink = SimpleRTPSink::createNew(envir(), rtpGroupsock,
				       rtpPayloadTypeIfDynamic, fClientMediaSubsession.rtpTimestampFrequency(),
				       fClientMediaSubsession.mediumName(), fClientMediaSubsession.codecName(),
				       fClientMediaSubsession.numChannels(), allowMultipleFramesPerPacket, doNormalMBitRule);
  }

  // Because our relayed frames' presentation times are inaccurate until the input frames have been RTCP-synchronized,
  // we temporarily disable RTCP "SR" reports for this "RTPSink" object:
  newSink->enableRTCPReports() = False;

  // Also tell our "PresentationTimeSubsessionNormalizer" object about the "RTPSink", so it can enable RTCP "SR" reports later:
  PresentationTimeSubsessionNormalizer* ssNormalizer;
  if (strcmp(codecName, "H264") == 0 ||
      strcmp(codecName, "H265") == 0 ||
      strcmp(codecName, "MP4V-ES") == 0 ||
      strcmp(codecName, "MPV") == 0 ||
      strcmp(codecName, "DV") == 0) {
    // There was a separate 'framer' object in front of the "PresentationTimeSubsessionNormalizer", so go back one object to get it:
    ssNormalizer = (PresentationTimeSubsessionNormalizer*)(((FramedFilter*)inputSource)->inputSource());
  } else {
    ssNormalizer = (PresentationTimeSubsessionNormalizer*)inputSource;
  }
  ssNormalizer->setRTPSink(newSink);

  return newSink;
}

void ProxyServerMediaSubsession::subsessionByeHandler(void* clientData) {
  ((ProxyServerMediaSubsession*)clientData)->subsessionByeHandler();
}

void ProxyServerMediaSubsession::subsessionByeHandler() {
  if (verbosityLevel() > 0) {
    envir() << *this << ": received RTCP \"BYE\".  (The back-end stream has ended.)\n";
  }

  // This "BYE" signals that our input source has (effectively) closed, so pass this onto the front-end clients:
  fHaveSetupStream = False; // hack to stop "PAUSE" getting sent by:
  fClientMediaSubsession.readSource()->handleClosure();

  // And then treat this as if we had lost connection to the back-end server,
  // and can reestablish streaming from it only by sending another "DESCRIBE":
  ProxyServerMediaSession* const sms = (ProxyServerMediaSession*)fParentSession;
  ProxyRTSPClient* const proxyRTSPClient = sms->fProxyRTSPClient;
  proxyRTSPClient->continueAfterLivenessCommand(1/*hack*/, proxyRTSPClient->fServerSupportsGetParameter);
}


////////// PresentationTimeSessionNormalizer and PresentationTimeSubsessionNormalizer implementations //////////

// PresentationTimeSessionNormalizer:

PresentationTimeSessionNormalizer::PresentationTimeSessionNormalizer(UsageEnvironment& env)
  : Medium(env),
    fSubsessionNormalizers(NULL), fMasterSSNormalizer(NULL) {
}

PresentationTimeSessionNormalizer::~PresentationTimeSessionNormalizer() {
  while (fSubsessionNormalizers != NULL) {
    delete fSubsessionNormalizers;
  }
}

PresentationTimeSubsessionNormalizer*
PresentationTimeSessionNormalizer::createNewPresentationTimeSubsessionNormalizer(FramedSource* inputSource, RTPSource* rtpSource,
										 char const* codecName) {
  fSubsessionNormalizers
    = new PresentationTimeSubsessionNormalizer(*this, inputSource, rtpSource, codecName, fSubsessionNormalizers);
  return fSubsessionNormalizers;
}

void PresentationTimeSessionNormalizer::normalizePresentationTime(PresentationTimeSubsessionNormalizer* ssNormalizer,
								  struct timeval& toPT, struct timeval const& fromPT) {
  Boolean const hasBeenSynced = ssNormalizer->fRTPSource->hasBeenSynchronizedUsingRTCP();

  if (!hasBeenSynced) {
    // If "fromPT" has not yet been RTCP-synchronized, then it was generated by our own receiving code, and thus
    // is already aligned with 'wall-clock' time.  Just copy it 'as is' to "toPT":
    toPT = fromPT;
  } else {
    if (fMasterSSNormalizer == NULL) {
      // Make "ssNormalizer" the 'master' subsession - meaning that its presentation time is adjusted to align with 'wall clock'
      // time, and the presentation times of other subsessions (if any) are adjusted to retain their relative separation with
      // those of the master:
      fMasterSSNormalizer = ssNormalizer;

      struct timeval timeNow;
      gettimeofday(&timeNow, NULL);

      // Compute: fPTAdjustment = timeNow - fromPT
      fPTAdjustment.tv_sec = timeNow.tv_sec - fromPT.tv_sec;
      fPTAdjustment.tv_usec = timeNow.tv_usec - fromPT.tv_usec;
      // Note: It's OK if one or both of these fields underflows; the result still works out OK later.
    }

    // Compute a normalized presentation time: toPT = fromPT + fPTAdjustment
    toPT.tv_sec = fromPT.tv_sec + fPTAdjustment.tv_sec - 1;
    toPT.tv_usec = fromPT.tv_usec + fPTAdjustment.tv_usec + MILLION;
    while (toPT.tv_usec > MILLION) { ++toPT.tv_sec; toPT.tv_usec -= MILLION; }

    // Because "ssNormalizer"s relayed presentation times are accurate from now on, enable RTCP "SR" reports for its "RTPSink":
    RTPSink* const rtpSink = ssNormalizer->fRTPSink;
    if (rtpSink != NULL) { // sanity check; should always be true
      rtpSink->enableRTCPReports() = True;
    }
  }
}

void PresentationTimeSessionNormalizer
::removePresentationTimeSubsessionNormalizer(PresentationTimeSubsessionNormalizer* ssNormalizer) {
  // Unlink "ssNormalizer" from the linked list (starting with "fSubsessionNormalizers"):
  if (fSubsessionNormalizers == ssNormalizer) {
    fSubsessionNormalizers = fSubsessionNormalizers->fNext;
  } else {
    PresentationTimeSubsessionNormalizer** ssPtrPtr = &(fSubsessionNormalizers->fNext);
    while (*ssPtrPtr != ssNormalizer) ssPtrPtr = &((*ssPtrPtr)->fNext);
    *ssPtrPtr = (*ssPtrPtr)->fNext;
  }
}

// PresentationTimeSubsessionNormalizer:

PresentationTimeSubsessionNormalizer
::PresentationTimeSubsessionNormalizer(PresentationTimeSessionNormalizer& parent, FramedSource* inputSource, RTPSource* rtpSource,
				       char const* codecName, PresentationTimeSubsessionNormalizer* next)
  : FramedFilter(parent.envir(), inputSource),
    fParent(parent), fRTPSource(rtpSource), fRTPSink(NULL), fCodecName(codecName), fNext(next) {
}

PresentationTimeSubsessionNormalizer::~PresentationTimeSubsessionNormalizer() {
  fParent.removePresentationTimeSubsessionNormalizer(this);
}

void PresentationTimeSubsessionNormalizer::afterGettingFrame(void* clientData, unsigned frameSize,
							     unsigned numTruncatedBytes,
							     struct timeval presentationTime,
							     unsigned durationInMicroseconds) {
  ((PresentationTimeSubsessionNormalizer*)clientData)
    ->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

void PresentationTimeSubsessionNormalizer::afterGettingFrame(unsigned frameSize,
							     unsigned numTruncatedBytes,
							     struct timeval presentationTime,
							     unsigned durationInMicroseconds) {
  // This filter is implemented by passing all frames through unchanged, except that "fPresentationTime" is changed:
  fFrameSize = frameSize;
  fNumTruncatedBytes = numTruncatedBytes;
  fDurationInMicroseconds = durationInMicroseconds;

  fParent.normalizePresentationTime(this, fPresentationTime, presentationTime);

  // Hack for JPEG/RTP proxying.  Because we're proxying JPEG by just copying the raw JPEG/RTP payloads, without interpreting them,
  // we need to also 'copy' the RTP 'M' (marker) bit from the "RTPSource" to the "RTPSink":
  if (fRTPSource->curPacketMarkerBit() && strcmp(fCodecName, "JPEG") == 0) ((SimpleRTPSink*)fRTPSink)->setMBitOnNextPacket();

  // Complete delivery:
  FramedSource::afterGetting(this);
}

void PresentationTimeSubsessionNormalizer::doGetNextFrame() {
  fInputSource->getNextFrame(fTo, fMaxSize, afterGettingFrame, this, FramedSource::handleClosure, this);
}
