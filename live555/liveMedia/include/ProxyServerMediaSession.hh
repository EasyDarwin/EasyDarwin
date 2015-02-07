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
// C++ header

#ifndef _PROXY_SERVER_MEDIA_SESSION_HH
#define _PROXY_SERVER_MEDIA_SESSION_HH

#ifndef _SERVER_MEDIA_SESSION_HH
#include "ServerMediaSession.hh"
#endif
#ifndef _MEDIA_SESSION_HH
#include "MediaSession.hh"
#endif
#ifndef _RTSP_CLIENT_HH
#include "RTSPClient.hh"
#endif

// A subclass of "RTSPClient", used to refer to the particular "ProxyServerMediaSession" object being used.
// It is used only within the implementation of "ProxyServerMediaSession", but is defined here, in case developers wish to
// subclass it.

class ProxyRTSPClient: public RTSPClient {
public:
  ProxyRTSPClient(class ProxyServerMediaSession& ourServerMediaSession, char const* rtspURL,
                  char const* username, char const* password,
                  portNumBits tunnelOverHTTPPortNum, int verbosityLevel, int socketNumToServer);
  virtual ~ProxyRTSPClient();

  void continueAfterDESCRIBE(char const* sdpDescription);
  void continueAfterLivenessCommand(int resultCode, Boolean serverSupportsGetParameter);
  void continueAfterSETUP();

private:
  void reset();

  Authenticator* auth() { return fOurAuthenticator; }

  void scheduleLivenessCommand();
  static void sendLivenessCommand(void* clientData);

  void scheduleDESCRIBECommand();
  static void sendDESCRIBE(void* clientData);

  static void subsessionTimeout(void* clientData);
  void handleSubsessionTimeout();

private:
  friend class ProxyServerMediaSession;
  friend class ProxyServerMediaSubsession;
  ProxyServerMediaSession& fOurServerMediaSession;
  char* fOurURL;
  Authenticator* fOurAuthenticator;
  Boolean fStreamRTPOverTCP;
  class ProxyServerMediaSubsession *fSetupQueueHead, *fSetupQueueTail;
  unsigned fNumSetupsDone;
  unsigned fNextDESCRIBEDelay; // in seconds
  Boolean fServerSupportsGetParameter, fLastCommandWasPLAY;
  TaskToken fLivenessCommandTask, fDESCRIBECommandTask, fSubsessionTimerTask;
};


typedef ProxyRTSPClient*
createNewProxyRTSPClientFunc(ProxyServerMediaSession& ourServerMediaSession,
			     char const* rtspURL,
			     char const* username, char const* password,
			     portNumBits tunnelOverHTTPPortNum, int verbosityLevel,
			     int socketNumToServer);
ProxyRTSPClient*
defaultCreateNewProxyRTSPClientFunc(ProxyServerMediaSession& ourServerMediaSession,
				    char const* rtspURL,
				    char const* username, char const* password,
				    portNumBits tunnelOverHTTPPortNum, int verbosityLevel,
				    int socketNumToServer);

class ProxyServerMediaSession: public ServerMediaSession {
public:
  static ProxyServerMediaSession* createNew(UsageEnvironment& env,
					    RTSPServer* ourRTSPServer, // Note: We can be used by just one "RTSPServer"
					    char const* inputStreamURL, // the "rtsp://" URL of the stream we'll be proxying
					    char const* streamName = NULL,
					    char const* username = NULL, char const* password = NULL,
					    portNumBits tunnelOverHTTPPortNum = 0,
					        // for streaming the *proxied* (i.e., back-end) stream
					    int verbosityLevel = 0,
					    int socketNumToServer = -1);
      // Hack: "tunnelOverHTTPPortNum" == 0xFFFF (i.e., all-ones) means: Stream RTP/RTCP-over-TCP, but *not* using HTTP
      // "verbosityLevel" == 1 means display basic proxy setup info; "verbosityLevel" == 2 means display RTSP client protocol also.
      // If "socketNumToServer" is >= 0, then it is the socket number of an already-existing TCP connection to the server.
      //      (In this case, "inputStreamURL" must point to the socket's endpoint, so that it can be accessed via the socket.)

  virtual ~ProxyServerMediaSession();

  char const* url() const;

  char describeCompletedFlag;
    // initialized to 0; set to 1 when the back-end "DESCRIBE" completes.
    // (This can be used as a 'watch variable' in "doEventLoop()".)
  Boolean describeCompletedSuccessfully() const { return fClientMediaSession != NULL; }
    // This can be used - along with "describeCompletdFlag" - to check whether the back-end "DESCRIBE" completed *successfully*.

protected:
  ProxyServerMediaSession(UsageEnvironment& env, RTSPServer* ourRTSPServer,
			  char const* inputStreamURL, char const* streamName,
			  char const* username, char const* password,
			  portNumBits tunnelOverHTTPPortNum, int verbosityLevel,
			  int socketNumToServer,
			  createNewProxyRTSPClientFunc* ourCreateNewProxyRTSPClientFunc
			  = defaultCreateNewProxyRTSPClientFunc);

  // If you subclass "ProxyRTSPClient", then you will also need to define your own function
  // - with signature "createNewProxyRTSPClientFunc" (see above) - that creates a new object
  // of this subclass.  You should also subclass "ProxyServerMediaSession" and, in your
  // subclass's constructor, initialize the parent class (i.e., "ProxyServerMediaSession")
  // constructor by passing your new function as the "ourCreateNewProxyRTSPClientFunc"
  // parameter.

protected:
  RTSPServer* fOurRTSPServer;
  ProxyRTSPClient* fProxyRTSPClient;
  MediaSession* fClientMediaSession;

private:
  friend class ProxyRTSPClient;
  friend class ProxyServerMediaSubsession;
  void continueAfterDESCRIBE(char const* sdpDescription);
  void resetDESCRIBEState(); // undoes what was done by "contineAfterDESCRIBE()"

private:
  int fVerbosityLevel;
  class PresentationTimeSessionNormalizer* fPresentationTimeSessionNormalizer;
  createNewProxyRTSPClientFunc* fCreateNewProxyRTSPClientFunc;
};


////////// PresentationTimeSessionNormalizer and PresentationTimeSubsessionNormalizer definitions //////////

// The following two classes are used by proxies to convert incoming streams' presentation times into wall-clock-aligned
// presentation times that are suitable for our "RTPSink"s (for the corresponding outgoing streams).
// (For multi-subsession (i.e., audio+video) sessions, the outgoing streams' presentation times retain the same relative
//  separation as those of the incoming streams.)

class PresentationTimeSubsessionNormalizer: public FramedFilter {
public:
  void setRTPSink(RTPSink* rtpSink) { fRTPSink = rtpSink; }

private:
  friend class PresentationTimeSessionNormalizer;
  PresentationTimeSubsessionNormalizer(PresentationTimeSessionNormalizer& parent, FramedSource* inputSource, RTPSource* rtpSource,
				       char const* codecName, PresentationTimeSubsessionNormalizer* next);
      // called only from within "PresentationTimeSessionNormalizer"
  virtual ~PresentationTimeSubsessionNormalizer();

  static void afterGettingFrame(void* clientData, unsigned frameSize,
                                unsigned numTruncatedBytes,
                                struct timeval presentationTime,
                                unsigned durationInMicroseconds);
  void afterGettingFrame(unsigned frameSize,
			 unsigned numTruncatedBytes,
			 struct timeval presentationTime,
			 unsigned durationInMicroseconds);

private: // redefined virtual functions:
  virtual void doGetNextFrame();

private:
  PresentationTimeSessionNormalizer& fParent;
  RTPSource* fRTPSource;
  RTPSink* fRTPSink;
  char const* fCodecName;
  PresentationTimeSubsessionNormalizer* fNext;
};

class PresentationTimeSessionNormalizer: public Medium {
public:
  PresentationTimeSessionNormalizer(UsageEnvironment& env);
  virtual ~PresentationTimeSessionNormalizer();

  PresentationTimeSubsessionNormalizer*
  createNewPresentationTimeSubsessionNormalizer(FramedSource* inputSource, RTPSource* rtpSource, char const* codecName);

private: // called only from within "~PresentationTimeSubsessionNormalizer":
  friend class PresentationTimeSubsessionNormalizer;
  void normalizePresentationTime(PresentationTimeSubsessionNormalizer* ssNormalizer,
				 struct timeval& toPT, struct timeval const& fromPT);
  void removePresentationTimeSubsessionNormalizer(PresentationTimeSubsessionNormalizer* ssNormalizer);

private:
  PresentationTimeSubsessionNormalizer* fSubsessionNormalizers;
  PresentationTimeSubsessionNormalizer* fMasterSSNormalizer; // used for subsessions that have been RTCP-synced

  struct timeval fPTAdjustment; // Added to (RTCP-synced) subsession presentation times to 'normalize' them with wall-clock time.
};

#endif
