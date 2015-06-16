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
// A RTSP server
// Implementation

#include "RTSPServer.hh"
#include "RTSPCommon.hh"
#include "RTSPRegisterSender.hh"
#include "ProxyServerMediaSession.hh"
#include "Base64.hh"
#include <GroupsockHelper.hh>

////////// RTSPServer implementation //////////

RTSPServer*
RTSPServer::createNew(UsageEnvironment& env, Port ourPort,
		      UserAuthenticationDatabase* authDatabase,
		      unsigned reclamationTestSeconds) {
  int ourSocket = setUpOurSocket(env, ourPort);
  if (ourSocket == -1) return NULL;
  
  return new RTSPServer(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds);
}

Boolean RTSPServer::lookupByName(UsageEnvironment& env,
				 char const* name,
				 RTSPServer*& resultServer) {
  resultServer = NULL; // unless we succeed
  
  Medium* medium;
  if (!Medium::lookupByName(env, name, medium)) return False;
  
  if (!medium->isRTSPServer()) {
    env.setResultMsg(name, " is not a RTSP server");
    return False;
  }
  
  resultServer = (RTSPServer*)medium;
  return True;
}

void RTSPServer::addServerMediaSession(ServerMediaSession* serverMediaSession) {
  if (serverMediaSession == NULL) return;
  
  char const* sessionName = serverMediaSession->streamName();
  if (sessionName == NULL) sessionName = "";
  removeServerMediaSession(sessionName); // in case an existing "ServerMediaSession" with this name already exists
  
  fServerMediaSessions->Add(sessionName, (void*)serverMediaSession);
}

ServerMediaSession* RTSPServer
::lookupServerMediaSession(char const* streamName, Boolean /*isFirstLookupInSession*/) {
  // Default implementation:
  return (ServerMediaSession*)(fServerMediaSessions->Lookup(streamName));
}

void RTSPServer::removeServerMediaSession(ServerMediaSession* serverMediaSession) {
  if (serverMediaSession == NULL) return;
  
  fServerMediaSessions->Remove(serverMediaSession->streamName());
  if (serverMediaSession->referenceCount() == 0) {
    Medium::close(serverMediaSession);
  } else {
    serverMediaSession->deleteWhenUnreferenced() = True;
  }
}

void RTSPServer::removeServerMediaSession(char const* streamName) {
  removeServerMediaSession((ServerMediaSession*)(fServerMediaSessions->Lookup(streamName)));
}

void RTSPServer::closeAllClientSessionsForServerMediaSession(ServerMediaSession* serverMediaSession) {
  if (serverMediaSession == NULL) return;
  
  HashTable::Iterator* iter = HashTable::Iterator::create(*fClientSessions);
  RTSPServer::RTSPClientSession* clientSession;
  char const* key; // dummy
  while ((clientSession = (RTSPServer::RTSPClientSession*)(iter->next(key))) != NULL) {
    if (clientSession->fOurServerMediaSession == serverMediaSession) {
      delete clientSession;
    }
  }
  delete iter;
}

void RTSPServer::closeAllClientSessionsForServerMediaSession(char const* streamName) {
  closeAllClientSessionsForServerMediaSession((ServerMediaSession*)(fServerMediaSessions->Lookup(streamName)));
}

void RTSPServer::deleteServerMediaSession(ServerMediaSession* serverMediaSession) {
  if (serverMediaSession == NULL) return;
  
  closeAllClientSessionsForServerMediaSession(serverMediaSession);
  removeServerMediaSession(serverMediaSession);
}

void RTSPServer::deleteServerMediaSession(char const* streamName) {
  deleteServerMediaSession((ServerMediaSession*)(fServerMediaSessions->Lookup(streamName)));
}

void rtspRegisterResponseHandler(RTSPClient* rtspClient, int resultCode, char* resultString); // forward

// A class that represents the state of a "REGISTER" request in progress:
class RegisterRequestRecord: public RTSPRegisterSender {
public:
  RegisterRequestRecord(RTSPServer& ourServer, unsigned requestId,
			char const* remoteClientNameOrAddress, portNumBits remoteClientPortNum, char const* rtspURLToRegister,
			RTSPServer::responseHandlerForREGISTER* responseHandler, Authenticator* authenticator,
			Boolean requestStreamingViaTCP, char const* proxyURLSuffix)
    : RTSPRegisterSender(ourServer.envir(), remoteClientNameOrAddress, remoteClientPortNum, rtspURLToRegister,
			 rtspRegisterResponseHandler, authenticator,
			 requestStreamingViaTCP, proxyURLSuffix, True/*reuseConnection*/,
#ifdef DEBUG
			 1/*verbosityLevel*/,
#else
			 0/*verbosityLevel*/,
#endif
			 NULL),
      fOurServer(ourServer), fRequestId(requestId), fResponseHandler(responseHandler) {
    // Add ourself to our server's 'pending REGISTER requests' table:
    ourServer.fPendingRegisterRequests->Add((char const*)this, this);
  }

  virtual ~RegisterRequestRecord(){
    // Remove ourself from the server's 'pending REGISTER requests' hash table before we go:
    fOurServer.fPendingRegisterRequests->Remove((char const*)this);
  }

  void handleResponse(int resultCode, char* resultString) {
    if (resultCode == 0) {
      // The "REGISTER" request succeeded, so use the still-open RTSP socket to await incoming commands from the remote endpoint:
      int sock;
      struct sockaddr_in remoteAddress;

      grabConnection(sock, remoteAddress);
      if (sock >= 0) (void)fOurServer.createNewClientConnection(sock, remoteAddress);
    }

    if (fResponseHandler != NULL) {
      // Call our (REGISTER-specific) response handler now:
      (*fResponseHandler)(&fOurServer, fRequestId, resultCode, resultString);
    } else {
      // We need to delete[] "resultString" before we leave:
      delete[] resultString;
    }

    // We're completely done with the REGISTER command now, so delete ourself now:
    delete this;
  }

private:
  RTSPServer& fOurServer;
  unsigned fRequestId;
  RTSPServer::responseHandlerForREGISTER* fResponseHandler;
};

void rtspRegisterResponseHandler(RTSPClient* rtspClient, int resultCode, char* resultString) {
  RegisterRequestRecord* registerRequestRecord = (RegisterRequestRecord*)rtspClient;

  registerRequestRecord->handleResponse(resultCode, resultString);
}

unsigned RTSPServer::registerStream(ServerMediaSession* serverMediaSession,
				    char const* remoteClientNameOrAddress, portNumBits remoteClientPortNum,
				    responseHandlerForREGISTER* responseHandler,
				    char const* username, char const* password,
				    Boolean receiveOurStreamViaTCP, char const* proxyURLSuffix) {
  // Create a new "RegisterRequestRecord" that will send the "REGISTER" command.
  // (This object will automatically get deleted after we get a response to the "REGISTER" command, or if we're deleted.)
  Authenticator* authenticator = NULL;
  if (username != NULL) {
    if (password == NULL) password = "";
    authenticator = new Authenticator(username, password);
  }
  unsigned requestId = ++fRegisterRequestCounter;
  new RegisterRequestRecord(*this, requestId,
			    remoteClientNameOrAddress, remoteClientPortNum, rtspURL(serverMediaSession),
			    responseHandler, authenticator,
			    receiveOurStreamViaTCP, proxyURLSuffix);
  
  delete authenticator; // we can do this here because it was copied to the "RegisterRequestRecord"
  return requestId;
}

char* RTSPServer
::rtspURL(ServerMediaSession const* serverMediaSession, int clientSocket) const {
  char* urlPrefix = rtspURLPrefix(clientSocket);
  char const* sessionName = serverMediaSession->streamName();
  
  char* resultURL = new char[strlen(urlPrefix) + strlen(sessionName) + 1];
  sprintf(resultURL, "%s%s", urlPrefix, sessionName);
  
  delete[] urlPrefix;
  return resultURL;
}

char* RTSPServer::rtspURLPrefix(int clientSocket) const {
  struct sockaddr_in ourAddress;
  if (clientSocket < 0) {
    // Use our default IP address in the URL:
    ourAddress.sin_addr.s_addr = ReceivingInterfaceAddr != 0
      ? ReceivingInterfaceAddr
      : ourIPAddress(envir()); // hack
  } else {
    SOCKLEN_T namelen = sizeof ourAddress;
    getsockname(clientSocket, (struct sockaddr*)&ourAddress, &namelen);
  }
  
  char urlBuffer[100]; // more than big enough for "rtsp://<ip-address>:<port>/"
  
  portNumBits portNumHostOrder = ntohs(fRTSPServerPort.num());
  if (portNumHostOrder == 554 /* the default port number */) {
    sprintf(urlBuffer, "rtsp://%s/", AddressString(ourAddress).val());
  } else {
    sprintf(urlBuffer, "rtsp://%s:%hu/",
	    AddressString(ourAddress).val(), portNumHostOrder);
  }
  
  return strDup(urlBuffer);
}

UserAuthenticationDatabase* RTSPServer::setAuthenticationDatabase(UserAuthenticationDatabase* newDB) {
  UserAuthenticationDatabase* oldDB = fAuthDB;
  fAuthDB = newDB;
  
  return oldDB;
}

Boolean RTSPServer::setUpTunnelingOverHTTP(Port httpPort) {
  fHTTPServerSocket = setUpOurSocket(envir(), httpPort);
  if (fHTTPServerSocket >= 0) {
    fHTTPServerPort = httpPort;
    envir().taskScheduler().turnOnBackgroundReadHandling(fHTTPServerSocket,
							 (TaskScheduler::BackgroundHandlerProc*)&incomingConnectionHandlerHTTP, this);
    return True;
  }
  
  return False;
}

portNumBits RTSPServer::httpServerPortNum() const {
  return ntohs(fHTTPServerPort.num());
}

#define LISTEN_BACKLOG_SIZE 20

int RTSPServer::setUpOurSocket(UsageEnvironment& env, Port& ourPort) {
  int ourSocket = -1;
  
  do {
    // The following statement is enabled by default.
    // Don't disable it (by defining ALLOW_RTSP_SERVER_PORT_REUSE) unless you know what you're doing.
#ifndef ALLOW_RTSP_SERVER_PORT_REUSE
    NoReuse dummy(env); // Don't use this socket if there's already a local server using it
#endif
    
    ourSocket = setupStreamSocket(env, ourPort);
    if (ourSocket < 0) break;
    
    // Make sure we have a big send buffer:
    if (!increaseSendBufferTo(env, ourSocket, 50*1024)) break;
    
    // Allow multiple simultaneous connections:
    if (listen(ourSocket, LISTEN_BACKLOG_SIZE) < 0) {
      env.setResultErrMsg("listen() failed: ");
      break;
    }
    
    if (ourPort.num() == 0) {
      // bind() will have chosen a port for us; return it also:
      if (!getSourcePort(env, ourSocket, ourPort)) break;
    }
    
    return ourSocket;
  } while (0);
  
  if (ourSocket != -1) ::closeSocket(ourSocket);
  return -1;
}

char const* RTSPServer::allowedCommandNames() {
  return "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER";
}

Boolean RTSPServer::weImplementREGISTER(char const* /*proxyURLSuffix*/, char*& responseStr) {
  // By default, servers do not implement our custom "REGISTER" command:
  responseStr = NULL;
  return False;
}

void RTSPServer::implementCmd_REGISTER(char const* /*url*/, char const* /*urlSuffix*/, int /*socketToRemoteServer*/,
				       Boolean /*deliverViaTCP*/, char const* /*proxyURLSuffix*/) {
  // By default, this function is a 'noop'
}

UserAuthenticationDatabase* RTSPServer::getAuthenticationDatabaseForCommand(char const* /*cmdName*/) {
  // default implementation
  return fAuthDB;
}

Boolean RTSPServer::specialClientAccessCheck(int /*clientSocket*/, struct sockaddr_in& /*clientAddr*/, char const* /*urlSuffix*/) {
  // default implementation
  return True;
}

Boolean RTSPServer::specialClientUserAccessCheck(int /*clientSocket*/, struct sockaddr_in& /*clientAddr*/,
						 char const* /*urlSuffix*/, char const * /*username*/) {
  // default implementation; no further access restrictions:
  return True;
}


RTSPServer::RTSPServer(UsageEnvironment& env,
		       int ourSocket, Port ourPort,
		       UserAuthenticationDatabase* authDatabase,
		       unsigned reclamationTestSeconds)
  : Medium(env),
    fRTSPServerPort(ourPort), fRTSPServerSocket(ourSocket), fHTTPServerSocket(-1), fHTTPServerPort(0),
    fServerMediaSessions(HashTable::create(STRING_HASH_KEYS)),
    fClientConnections(HashTable::create(ONE_WORD_HASH_KEYS)),
    fClientConnectionsForHTTPTunneling(NULL), // will get created if needed
    fClientSessions(HashTable::create(STRING_HASH_KEYS)),
    fPendingRegisterRequests(HashTable::create(ONE_WORD_HASH_KEYS)), fRegisterRequestCounter(0),
    fAuthDB(authDatabase), fReclamationTestSeconds(reclamationTestSeconds),
    fAllowStreamingRTPOverTCP(True) {
  ignoreSigPipeOnSocket(ourSocket); // so that clients on the same host that are killed don't also kill us
  
  // Arrange to handle connections from others:
  env.taskScheduler().turnOnBackgroundReadHandling(fRTSPServerSocket,
						   (TaskScheduler::BackgroundHandlerProc*)&incomingConnectionHandlerRTSP, this);
}

RTSPServer::~RTSPServer() {
  // Turn off background read handling:
  envir().taskScheduler().turnOffBackgroundReadHandling(fRTSPServerSocket);
  ::closeSocket(fRTSPServerSocket);
  
  envir().taskScheduler().turnOffBackgroundReadHandling(fHTTPServerSocket);
  ::closeSocket(fHTTPServerSocket);
  
  // Close all client session objects:
  RTSPServer::RTSPClientSession* clientSession;
  while ((clientSession = (RTSPServer::RTSPClientSession*)fClientSessions->getFirst()) != NULL) {
    delete clientSession;
  }
  delete fClientSessions;
  
  // Close all client connection objects:
  RTSPServer::RTSPClientConnection* connection;
  while ((connection = (RTSPServer::RTSPClientConnection*)fClientConnections->getFirst()) != NULL) {
    delete connection;
  }
  delete fClientConnections;
  delete fClientConnectionsForHTTPTunneling; // all content was already removed as a result of the loop above
  
  // Delete all server media sessions
  ServerMediaSession* serverMediaSession;
  while ((serverMediaSession = (ServerMediaSession*)fServerMediaSessions->getFirst()) != NULL) {
    removeServerMediaSession(serverMediaSession); // will delete it, because it no longer has any 'client session' objects using it
  }
  delete fServerMediaSessions;
  
  // Delete any pending REGISTER requests:
  RegisterRequestRecord* registerRequest;
  while ((registerRequest = (RegisterRequestRecord*)fPendingRegisterRequests->getFirst()) != NULL) {
    delete registerRequest;
  }
  delete fPendingRegisterRequests;
}

Boolean RTSPServer::isRTSPServer() const {
  return True;
}

void RTSPServer::incomingConnectionHandlerRTSP(void* instance, int /*mask*/) {
  RTSPServer* server = (RTSPServer*)instance;
  server->incomingConnectionHandlerRTSP1();
}
void RTSPServer::incomingConnectionHandlerRTSP1() {
  incomingConnectionHandler(fRTSPServerSocket);
}

void RTSPServer::incomingConnectionHandlerHTTP(void* instance, int /*mask*/) {
  RTSPServer* server = (RTSPServer*)instance;
  server->incomingConnectionHandlerHTTP1();
}
void RTSPServer::incomingConnectionHandlerHTTP1() {
  incomingConnectionHandler(fHTTPServerSocket);
}

void RTSPServer::incomingConnectionHandler(int serverSocket) {
  struct sockaddr_in clientAddr;
  SOCKLEN_T clientAddrLen = sizeof clientAddr;
  int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
  if (clientSocket < 0) {
    int err = envir().getErrno();
    if (err != EWOULDBLOCK) {
      envir().setResultErrMsg("accept() failed: ");
    }
    return;
  }
  makeSocketNonBlocking(clientSocket);
  increaseSendBufferTo(envir(), clientSocket, 50*1024);
  
#ifdef DEBUG
  envir() << "accept()ed connection from " << AddressString(clientAddr).val() << "\n";
#endif
  
  // Create a new object for handling this RTSP connection:
  (void)createNewClientConnection(clientSocket, clientAddr);
}


////////// RTSPServer::RTSPClientConnection implementation //////////

RTSPServer::RTSPClientConnection
::RTSPClientConnection(RTSPServer& ourServer, int clientSocket, struct sockaddr_in clientAddr)
  : fOurServer(ourServer), fIsActive(True),
    fClientInputSocket(clientSocket), fClientOutputSocket(clientSocket), fClientAddr(clientAddr),
    fRecursionCount(0), fOurSessionCookie(NULL) {
  // Add ourself to our 'client connections' table:
  fOurServer.fClientConnections->Add((char const*)this, this);
  
  // Arrange to handle incoming requests:
  resetRequestBuffer();
  envir().taskScheduler().setBackgroundHandling(fClientInputSocket, SOCKET_READABLE|SOCKET_EXCEPTION,
						(TaskScheduler::BackgroundHandlerProc*)&incomingRequestHandler, this);
}

RTSPServer::RTSPClientConnection::~RTSPClientConnection() {
  // Remove ourself from the server's 'client connections' hash table before we go:
  fOurServer.fClientConnections->Remove((char const*)this);
  
  if (fOurSessionCookie != NULL) {
    // We were being used for RTSP-over-HTTP tunneling. Also remove ourselves from the 'session cookie' hash table before we go:
    fOurServer.fClientConnectionsForHTTPTunneling->Remove(fOurSessionCookie);
    delete[] fOurSessionCookie;
  }
  
  closeSockets();
}

// Special mechanism for handling our custom "REGISTER" command:

RTSPServer::RTSPClientConnection::ParamsForREGISTER
::ParamsForREGISTER(RTSPServer::RTSPClientConnection* ourConnection, char const* url, char const* urlSuffix,
		    Boolean reuseConnection, Boolean deliverViaTCP, char const* proxyURLSuffix)
  : fOurConnection(ourConnection), fURL(strDup(url)), fURLSuffix(strDup(urlSuffix)),
    fReuseConnection(reuseConnection), fDeliverViaTCP(deliverViaTCP), fProxyURLSuffix(strDup(proxyURLSuffix)) {
}

RTSPServer::RTSPClientConnection::ParamsForREGISTER::~ParamsForREGISTER() {
  delete[] fURL; delete[] fURLSuffix; delete[] fProxyURLSuffix;
}

// Handler routines for specific RTSP commands:

void RTSPServer::RTSPClientConnection::handleCmd_OPTIONS() {
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 200 OK\r\nCSeq: %s\r\n%sPublic: %s\r\n\r\n",
	   fCurrentCSeq, dateHeader(), fOurServer.allowedCommandNames());
}

void RTSPServer::RTSPClientConnection
::handleCmd_GET_PARAMETER(char const* /*fullRequestStr*/) {
  // By default, we implement "GET_PARAMETER" (on the entire server) just as a 'no op', and send back a dummy response.
  // (If you want to handle this type of "GET_PARAMETER" differently, you can do so by defining a subclass of "RTSPServer"
  // and "RTSPServer::RTSPClientConnection", and then reimplement this virtual function in your subclass.)
  setRTSPResponse("200 OK", LIVEMEDIA_LIBRARY_VERSION_STRING);
}

void RTSPServer::RTSPClientConnection
::handleCmd_SET_PARAMETER(char const* /*fullRequestStr*/) {
  // By default, we implement "SET_PARAMETER" (on the entire server) just as a 'no op', and send back an empty response.
  // (If you want to handle this type of "SET_PARAMETER" differently, you can do so by defining a subclass of "RTSPServer"
  // and "RTSPServer::RTSPClientConnection", and then reimplement this virtual function in your subclass.)
  setRTSPResponse("200 OK");
}

void RTSPServer::RTSPClientConnection
::handleCmd_DESCRIBE(char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr) {
  ServerMediaSession* session = NULL;
  char* sdpDescription = NULL;
  char* rtspURL = NULL;
  do {
    char urlTotalSuffix[RTSP_PARAM_STRING_MAX];
    urlTotalSuffix[0] = '\0';
    if (urlPreSuffix[0] != '\0') {
      strcat(urlTotalSuffix, urlPreSuffix);
      strcat(urlTotalSuffix, "/");
    }
    strcat(urlTotalSuffix, urlSuffix);
    
    if (!authenticationOK("DESCRIBE", urlTotalSuffix, fullRequestStr)) break;
    
    // We should really check that the request contains an "Accept:" #####
    // for "application/sdp", because that's what we're sending back #####
    
    // Begin by looking up the "ServerMediaSession" object for the specified "urlTotalSuffix":
    session = fOurServer.lookupServerMediaSession(urlTotalSuffix);
    if (session == NULL) {
      handleCmd_notFound();
      break;
    }
    
    // Increment the "ServerMediaSession" object's reference count, in case someone removes it
    // while we're using it:
    session->incrementReferenceCount();

    // Then, assemble a SDP description for this session:
    sdpDescription = session->generateSDPDescription();
    if (sdpDescription == NULL) {
      // This usually means that a file name that was specified for a
      // "ServerMediaSubsession" does not exist.
      setRTSPResponse("404 File Not Found, Or In Incorrect Format");
      break;
    }
    unsigned sdpDescriptionSize = strlen(sdpDescription);
    
    // Also, generate our RTSP URL, for the "Content-Base:" header
    // (which is necessary to ensure that the correct URL gets used in subsequent "SETUP" requests).
    rtspURL = fOurServer.rtspURL(session, fClientInputSocket);
    
    snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	     "RTSP/1.0 200 OK\r\nCSeq: %s\r\n"
	     "%s"
	     "Content-Base: %s/\r\n"
	     "Content-Type: application/sdp\r\n"
	     "Content-Length: %d\r\n\r\n"
	     "%s",
	     fCurrentCSeq,
	     dateHeader(),
	     rtspURL,
	     sdpDescriptionSize,
	     sdpDescription);
  } while (0);
  
  if (session != NULL) {
    // Decrement it's reference count, now that we're done using it:
    session->decrementReferenceCount();
    if (session->referenceCount() == 0 && session->deleteWhenUnreferenced()) {
      fOurServer.removeServerMediaSession(session);
    }
  }

  delete[] sdpDescription;
  delete[] rtspURL;
}

static void lookForHeader(char const* headerName, char const* source, unsigned sourceLen, char* resultStr, unsigned resultMaxSize) {
  resultStr[0] = '\0';  // by default, return an empty string
  unsigned headerNameLen = strlen(headerName);
  for (int i = 0; i < (int)(sourceLen-headerNameLen); ++i) {
    if (strncmp(&source[i], headerName, headerNameLen) == 0 && source[i+headerNameLen] == ':') {
      // We found the header.  Skip over any whitespace, then copy the rest of the line to "resultStr":
      for (i += headerNameLen+1; i < (int)sourceLen && (source[i] == ' ' || source[i] == '\t'); ++i) {}
      for (unsigned j = i; j < sourceLen; ++j) {
	if (source[j] == '\r' || source[j] == '\n') {
	  // We've found the end of the line.  Copy it to the result (if it will fit):
	  if (j-i+1 > resultMaxSize) break;
	  char const* resultSource = &source[i];
	  char const* resultSourceEnd = &source[j];
	  while (resultSource < resultSourceEnd) *resultStr++ = *resultSource++;
	  *resultStr = '\0';
	  break;
	}
      }
    }
  }
}

void RTSPServer
::RTSPClientConnection::handleCmd_REGISTER(char const* url, char const* urlSuffix, char const* fullRequestStr,
					   Boolean reuseConnection, Boolean deliverViaTCP, char const* proxyURLSuffix) {
  char* responseStr;
  if (fOurServer.weImplementREGISTER(proxyURLSuffix, responseStr)) {
    // The "REGISTER" command - if we implement it - may require access control:
    if (!authenticationOK("REGISTER", urlSuffix, fullRequestStr)) return;
    
    // We implement the "REGISTER" command by first replying to it, then actually handling it
    // (in a separate event-loop task, that will get called after the reply has been done):
    setRTSPResponse(responseStr == NULL ? "200 OK" : responseStr);
    delete[] responseStr;
    
    ParamsForREGISTER* registerParams = new ParamsForREGISTER(this, url, urlSuffix, reuseConnection, deliverViaTCP, proxyURLSuffix);
    envir().taskScheduler().scheduleDelayedTask(0, (TaskFunc*)continueHandlingREGISTER, registerParams);
  } else if (responseStr != NULL) {
    setRTSPResponse(responseStr);
    delete[] responseStr;
  } else {
    handleCmd_notSupported();
  }
}

void RTSPServer::RTSPClientConnection::handleCmd_bad() {
  // Don't do anything with "fCurrentCSeq", because it might be nonsense
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 400 Bad Request\r\n%sAllow: %s\r\n\r\n",
	   dateHeader(), fOurServer.allowedCommandNames());
}

void RTSPServer::RTSPClientConnection::handleCmd_notSupported() {
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 405 Method Not Allowed\r\nCSeq: %s\r\n%sAllow: %s\r\n\r\n",
	   fCurrentCSeq, dateHeader(), fOurServer.allowedCommandNames());
}

void RTSPServer::RTSPClientConnection::handleCmd_notFound() {
  setRTSPResponse("404 Stream Not Found");
}

void RTSPServer::RTSPClientConnection::handleCmd_sessionNotFound() {
  setRTSPResponse("454 Session Not Found");
}

void RTSPServer::RTSPClientConnection::handleCmd_unsupportedTransport() {
  setRTSPResponse("461 Unsupported Transport");
}

Boolean RTSPServer::RTSPClientConnection::parseHTTPRequestString(char* resultCmdName, unsigned resultCmdNameMaxSize,
								 char* urlSuffix, unsigned urlSuffixMaxSize,
								 char* sessionCookie, unsigned sessionCookieMaxSize,
								 char* acceptStr, unsigned acceptStrMaxSize) {
  // Check for the limited HTTP requests that we expect for specifying RTSP-over-HTTP tunneling.
  // This parser is currently rather dumb; it should be made smarter #####
  char const* reqStr = (char const*)fRequestBuffer;
  unsigned const reqStrSize = fRequestBytesAlreadySeen;
  
  // Read everything up to the first space as the command name:
  Boolean parseSucceeded = False;
  unsigned i;
  for (i = 0; i < resultCmdNameMaxSize-1 && i < reqStrSize; ++i) {
    char c = reqStr[i];
    if (c == ' ' || c == '\t') {
      parseSucceeded = True;
      break;
    }
    
    resultCmdName[i] = c;
  }
  resultCmdName[i] = '\0';
  if (!parseSucceeded) return False;
  
  // Look for the string "HTTP/", before the first \r or \n:
  parseSucceeded = False;
  for (; i < reqStrSize-5 && reqStr[i] != '\r' && reqStr[i] != '\n'; ++i) {
    if (reqStr[i] == 'H' && reqStr[i+1] == 'T' && reqStr[i+2]== 'T' && reqStr[i+3]== 'P' && reqStr[i+4]== '/') {
      i += 5; // to advance past the "HTTP/"
      parseSucceeded = True;
      break;
    }
  }
  if (!parseSucceeded) return False;
  
  // Get the 'URL suffix' that occurred before this:
  unsigned k = i-6;
  while (k > 0 && reqStr[k] == ' ') --k; // back up over white space
  unsigned j = k;
  while (j > 0 && reqStr[j] != ' ' && reqStr[j] != '/') --j;
  // The URL suffix is in position (j,k]:
  if (k - j + 1 > urlSuffixMaxSize) return False; // there's no room> 
  unsigned n = 0;
  while (++j <= k) urlSuffix[n++] = reqStr[j];
  urlSuffix[n] = '\0';
  
  // Look for various headers that we're interested in:
  lookForHeader("x-sessioncookie", &reqStr[i], reqStrSize-i, sessionCookie, sessionCookieMaxSize);
  lookForHeader("Accept", &reqStr[i], reqStrSize-i, acceptStr, acceptStrMaxSize);
  
  return True;
}

void RTSPServer::RTSPClientConnection::handleHTTPCmd_notSupported() {
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "HTTP/1.1 405 Method Not Allowed\r\n%s\r\n\r\n",
	   dateHeader());
}

void RTSPServer::RTSPClientConnection::handleHTTPCmd_notFound() {
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "HTTP/1.1 404 Not Found\r\n%s\r\n\r\n",
	   dateHeader());
}

void RTSPServer::RTSPClientConnection::handleHTTPCmd_OPTIONS() {
#ifdef DEBUG
  fprintf(stderr, "Handled HTTP \"OPTIONS\" request\n");
#endif
  // Construct a response to the "OPTIONS" command that notes that our special headers (for RTSP-over-HTTP tunneling) are allowed:
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "HTTP/1.1 200 OK\r\n"
	   "%s"
	   "Access-Control-Allow-Origin: *\r\n"
	   "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
	   "Access-Control-Allow-Headers: x-sessioncookie, Pragma, Cache-Control\r\n"
	   "Access-Control-Max-Age: 1728000\r\n"
	   "\r\n",
	   dateHeader());
}

void RTSPServer::RTSPClientConnection::handleHTTPCmd_TunnelingGET(char const* sessionCookie) {
  // Record ourself as having this 'session cookie', so that a subsequent HTTP "POST" command (with the same 'session cookie')
  // can find us:
  if (fOurServer.fClientConnectionsForHTTPTunneling == NULL) {
    fOurServer.fClientConnectionsForHTTPTunneling = HashTable::create(STRING_HASH_KEYS);
  }
  delete[] fOurSessionCookie; fOurSessionCookie = strDup(sessionCookie);
  fOurServer.fClientConnectionsForHTTPTunneling->Add(sessionCookie, (void*)this);
#ifdef DEBUG
  fprintf(stderr, "Handled HTTP \"GET\" request (client output socket: %d)\n", fClientOutputSocket);
#endif
  
  // Construct our response:
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "HTTP/1.1 200 OK\r\n"
	   "%s"
	   "Cache-Control: no-cache\r\n"
	   "Pragma: no-cache\r\n"
	   "Content-Type: application/x-rtsp-tunnelled\r\n"
	   "\r\n",
	   dateHeader());
}

Boolean RTSPServer::RTSPClientConnection
::handleHTTPCmd_TunnelingPOST(char const* sessionCookie, unsigned char const* extraData, unsigned extraDataSize) {
  // Use the "sessionCookie" string to look up the separate "RTSPClientConnection" object that should have been used to handle
  // an earlier HTTP "GET" request:
  if (fOurServer.fClientConnectionsForHTTPTunneling == NULL) {
    fOurServer.fClientConnectionsForHTTPTunneling = HashTable::create(STRING_HASH_KEYS);
  }
  RTSPServer::RTSPClientConnection* prevClientConnection
    = (RTSPServer::RTSPClientConnection*)(fOurServer.fClientConnectionsForHTTPTunneling->Lookup(sessionCookie));
  if (prevClientConnection == NULL) {
    // There was no previous HTTP "GET" request; treat this "POST" request as bad:
    handleHTTPCmd_notSupported();
    fIsActive = False; // triggers deletion of ourself
    return False;
  }
#ifdef DEBUG
  fprintf(stderr, "Handled HTTP \"POST\" request (client input socket: %d)\n", fClientInputSocket);
#endif
  
  // Change the previous "RTSPClientSession" object's input socket to ours.  It will be used for subsequent requests:
  prevClientConnection->changeClientInputSocket(fClientInputSocket, extraData, extraDataSize);
  fClientInputSocket = fClientOutputSocket = -1; // so the socket doesn't get closed when we get deleted
  return True;
}

void RTSPServer::RTSPClientConnection::handleHTTPCmd_StreamingGET(char const* /*urlSuffix*/, char const* /*fullRequestStr*/) {
  // By default, we don't support requests to access streams via HTTP:
  handleHTTPCmd_notSupported();
}

void RTSPServer::RTSPClientConnection::resetRequestBuffer() {
  fRequestBytesAlreadySeen = 0;
  fRequestBufferBytesLeft = sizeof fRequestBuffer;
  fLastCRLF = &fRequestBuffer[-3]; // hack: Ensures that we don't think we have end-of-msg if the data starts with <CR><LF>
  fBase64RemainderCount = 0;
}

void RTSPServer::RTSPClientConnection::closeSockets() {
  // Turn off background handling on our input socket (and output socket, if different); then close it (or them):
  if (fClientOutputSocket != fClientInputSocket) {
    envir().taskScheduler().disableBackgroundHandling(fClientOutputSocket);
    ::closeSocket(fClientOutputSocket);
  }
  
  envir().taskScheduler().disableBackgroundHandling(fClientInputSocket);
  ::closeSocket(fClientInputSocket);
  
  fClientInputSocket = fClientOutputSocket = -1;
}

void RTSPServer::RTSPClientConnection::incomingRequestHandler(void* instance, int /*mask*/) {
  RTSPClientConnection* session = (RTSPClientConnection*)instance;
  session->incomingRequestHandler1();
}

void RTSPServer::RTSPClientConnection::incomingRequestHandler1() {
  struct sockaddr_in dummy; // 'from' address, meaningless in this case
  
  int bytesRead = readSocket(envir(), fClientInputSocket, &fRequestBuffer[fRequestBytesAlreadySeen], fRequestBufferBytesLeft, dummy);
  handleRequestBytes(bytesRead);
}

void RTSPServer::RTSPClientConnection::handleAlternativeRequestByte(void* instance, u_int8_t requestByte) {
  RTSPClientConnection* session = (RTSPClientConnection*)instance;
  session->handleAlternativeRequestByte1(requestByte);
}

void RTSPServer::RTSPClientConnection::handleAlternativeRequestByte1(u_int8_t requestByte) {
  if (requestByte == 0xFF) {
    // Hack: The new handler of the input TCP socket encountered an error reading it.  Indicate this:
    handleRequestBytes(-1);
  } else if (requestByte == 0xFE) {
    // Another hack: The new handler of the input TCP socket no longer needs it, so take back control of it:
    envir().taskScheduler().setBackgroundHandling(fClientInputSocket, SOCKET_READABLE|SOCKET_EXCEPTION,
						  (TaskScheduler::BackgroundHandlerProc*)&incomingRequestHandler, this);
  } else {
    // Normal case: Add this character to our buffer; then try to handle the data that we have buffered so far:
    if (fRequestBufferBytesLeft == 0 || fRequestBytesAlreadySeen >= RTSP_BUFFER_SIZE) return;
    fRequestBuffer[fRequestBytesAlreadySeen] = requestByte;
    handleRequestBytes(1);
  }
}

// A special version of "parseTransportHeader()", used just for parsing the "Transport:" header in an incoming "REGISTER" command:
static void parseTransportHeaderForREGISTER(char const* buf,
					    Boolean &reuseConnection,
					    Boolean& deliverViaTCP,
					    char*& proxyURLSuffix) {
  // Initialize the result parameters to default values:
  reuseConnection = False;
  deliverViaTCP = False;
  proxyURLSuffix = NULL;
  
  // First, find "Transport:"
  while (1) {
    if (*buf == '\0') return; // not found
    if (*buf == '\r' && *(buf+1) == '\n' && *(buf+2) == '\r') return; // end of the headers => not found
    if (_strncasecmp(buf, "Transport:", 10) == 0) break;
    ++buf;
  }
  
  // Then, run through each of the fields, looking for ones we handle:
  char const* fields = buf + 10;
  while (*fields == ' ') ++fields;
  char* field = strDupSize(fields);
  while (sscanf(fields, "%[^;\r\n]", field) == 1) {
    if (strcmp(field, "reuse_connection") == 0) {
      reuseConnection = True;
    } else if (_strncasecmp(field, "preferred_delivery_protocol=udp", 31) == 0) {
      deliverViaTCP = False;
    } else if (_strncasecmp(field, "preferred_delivery_protocol=interleaved", 39) == 0) {
      deliverViaTCP = True;
    } else if (_strncasecmp(field, "proxy_url_suffix=", 17) == 0) {
      delete[] proxyURLSuffix;
      proxyURLSuffix = strDup(field+17);
    }
    
    fields += strlen(field);
    while (*fields == ';' || *fields == ' ' || *fields == '\t') ++fields; // skip over separating ';' chars or whitespace
    if (*fields == '\0' || *fields == '\r' || *fields == '\n') break;
  }
  delete[] field;
}

void RTSPServer::RTSPClientConnection::handleRequestBytes(int newBytesRead) {
  int numBytesRemaining = 0;
  ++fRecursionCount;
  
  do {
    RTSPServer::RTSPClientSession* clientSession = NULL;

    if (newBytesRead < 0 || (unsigned)newBytesRead >= fRequestBufferBytesLeft) {
      // Either the client socket has died, or the request was too big for us.
      // Terminate this connection:
#ifdef DEBUG
      fprintf(stderr, "RTSPClientConnection[%p]::handleRequestBytes() read %d new bytes (of %d); terminating connection!\n", this, newBytesRead, fRequestBufferBytesLeft);
#endif
      fIsActive = False;
      break;
    }
    
    Boolean endOfMsg = False;
    unsigned char* ptr = &fRequestBuffer[fRequestBytesAlreadySeen];
#ifdef DEBUG
    ptr[newBytesRead] = '\0';
    fprintf(stderr, "RTSPClientConnection[%p]::handleRequestBytes() %s %d new bytes:%s\n",
	    this, numBytesRemaining > 0 ? "processing" : "read", newBytesRead, ptr);
#endif
    
    if (fClientOutputSocket != fClientInputSocket && numBytesRemaining == 0) {
      // We're doing RTSP-over-HTTP tunneling, and input commands are assumed to have been Base64-encoded.
      // We therefore Base64-decode as much of this new data as we can (i.e., up to a multiple of 4 bytes).
      
      // But first, we remove any whitespace that may be in the input data:
      unsigned toIndex = 0;
      for (int fromIndex = 0; fromIndex < newBytesRead; ++fromIndex) {
	char c = ptr[fromIndex];
	if (!(c == ' ' || c == '\t' || c == '\r' || c == '\n')) { // not 'whitespace': space,tab,CR,NL
	  ptr[toIndex++] = c;
	}
      }
      newBytesRead = toIndex;
      
      unsigned numBytesToDecode = fBase64RemainderCount + newBytesRead;
      unsigned newBase64RemainderCount = numBytesToDecode%4;
      numBytesToDecode -= newBase64RemainderCount;
      if (numBytesToDecode > 0) {
	ptr[newBytesRead] = '\0';
	unsigned decodedSize;
	unsigned char* decodedBytes = base64Decode((char const*)(ptr-fBase64RemainderCount), numBytesToDecode, decodedSize);
#ifdef DEBUG
	fprintf(stderr, "Base64-decoded %d input bytes into %d new bytes:", numBytesToDecode, decodedSize);
	for (unsigned k = 0; k < decodedSize; ++k) fprintf(stderr, "%c", decodedBytes[k]);
	fprintf(stderr, "\n");
#endif
	
	// Copy the new decoded bytes in place of the old ones (we can do this because there are fewer decoded bytes than original):
	unsigned char* to = ptr-fBase64RemainderCount;
	for (unsigned i = 0; i < decodedSize; ++i) *to++ = decodedBytes[i];
	
	// Then copy any remaining (undecoded) bytes to the end:
	for (unsigned j = 0; j < newBase64RemainderCount; ++j) *to++ = (ptr-fBase64RemainderCount+numBytesToDecode)[j];
	
	newBytesRead = decodedSize - fBase64RemainderCount + newBase64RemainderCount;
	  // adjust to allow for the size of the new decoded data (+ remainder)
	delete[] decodedBytes;
      }
      fBase64RemainderCount = newBase64RemainderCount;
    }
    
    unsigned char *tmpPtr = fLastCRLF + 2;
    if (fBase64RemainderCount == 0) { // no more Base-64 bytes remain to be read/decoded
      // Look for the end of the message: <CR><LF><CR><LF>
      if (tmpPtr < fRequestBuffer) tmpPtr = fRequestBuffer;
      while (tmpPtr < &ptr[newBytesRead-1]) {
	if (*tmpPtr == '\r' && *(tmpPtr+1) == '\n') {
	  if (tmpPtr - fLastCRLF == 2) { // This is it:
	    endOfMsg = True;
	    break;
	  }
	  fLastCRLF = tmpPtr;
	}
	++tmpPtr;
      }
    }
    
    fRequestBufferBytesLeft -= newBytesRead;
    fRequestBytesAlreadySeen += newBytesRead;
    
    if (!endOfMsg) break; // subsequent reads will be needed to complete the request
    
    // Parse the request string into command name and 'CSeq', then handle the command:
    fRequestBuffer[fRequestBytesAlreadySeen] = '\0';
    char cmdName[RTSP_PARAM_STRING_MAX];
    char urlPreSuffix[RTSP_PARAM_STRING_MAX];
    char urlSuffix[RTSP_PARAM_STRING_MAX];
    char cseq[RTSP_PARAM_STRING_MAX];
    char sessionIdStr[RTSP_PARAM_STRING_MAX];
    unsigned contentLength = 0;
    fLastCRLF[2] = '\0'; // temporarily, for parsing
    Boolean parseSucceeded = parseRTSPRequestString((char*)fRequestBuffer, fLastCRLF+2 - fRequestBuffer,
						    cmdName, sizeof cmdName,
						    urlPreSuffix, sizeof urlPreSuffix,
						    urlSuffix, sizeof urlSuffix,
						    cseq, sizeof cseq,
						    sessionIdStr, sizeof sessionIdStr,
						    contentLength);
    fLastCRLF[2] = '\r'; // restore its value
    Boolean playAfterSetup = False;
    if (parseSucceeded) {
#ifdef DEBUG
      fprintf(stderr, "parseRTSPRequestString() succeeded, returning cmdName \"%s\", urlPreSuffix \"%s\", urlSuffix \"%s\", CSeq \"%s\", Content-Length %u, with %ld bytes following the message.\n", cmdName, urlPreSuffix, urlSuffix, cseq, contentLength, ptr + newBytesRead - (tmpPtr + 2));
#endif
      // If there was a "Content-Length:" header, then make sure we've received all of the data that it specified:
      if (ptr + newBytesRead < tmpPtr + 2 + contentLength) break; // we still need more data; subsequent reads will give it to us 
      
      // If the request included a "Session:" id, and it refers to a client session that's
      // current ongoing, then use this command to indicate 'liveness' on that client session:
      Boolean const requestIncludedSessionId = sessionIdStr[0] != '\0';
      if (requestIncludedSessionId) {
	clientSession = (RTSPServer::RTSPClientSession*)(fOurServer.fClientSessions->Lookup(sessionIdStr));
	if (clientSession != NULL) clientSession->noteLiveness();
      }
    
      // We now have a complete RTSP request.
      // Handle the specified command (beginning with commands that are session-independent):
      fCurrentCSeq = cseq;
      if (strcmp(cmdName, "OPTIONS") == 0) {
	// If the "OPTIONS" command included a "Session:" id for a session that doesn't exist,
	// then treat this as an error:
	if (requestIncludedSessionId && clientSession == NULL) {
	  handleCmd_sessionNotFound();
	} else {
	  // Normal case:
	  handleCmd_OPTIONS();
	}
      } else if (urlPreSuffix[0] == '\0' && urlSuffix[0] == '*' && urlSuffix[1] == '\0') {
	// The special "*" URL means: an operation on the entire server.  This works only for GET_PARAMETER and SET_PARAMETER:
	if (strcmp(cmdName, "GET_PARAMETER") == 0) {
	  handleCmd_GET_PARAMETER((char const*)fRequestBuffer);
	} else if (strcmp(cmdName, "SET_PARAMETER") == 0) {
	  handleCmd_SET_PARAMETER((char const*)fRequestBuffer);
	} else {
	  handleCmd_notSupported();
	}
      } else if (strcmp(cmdName, "DESCRIBE") == 0) {
	handleCmd_DESCRIBE(urlPreSuffix, urlSuffix, (char const*)fRequestBuffer);
      } else if (strcmp(cmdName, "SETUP") == 0) {
	Boolean areAuthenticated = True;

	if (!requestIncludedSessionId) {
	  // No session id was present in the request.  So create a new "RTSPClientSession" object
	  // for this request.  Choose a random (unused) 32-bit integer for the session id
	  // (it will be encoded as a 8-digit hex number).  (We avoid choosing session id 0,
	  // because that has a special use (by "OnDemandServerMediaSubsession").)

	  // But first, make sure that we're authenticated to perform this command:
	  char urlTotalSuffix[RTSP_PARAM_STRING_MAX];
	  urlTotalSuffix[0] = '\0';
	  if (urlPreSuffix[0] != '\0') {
	    strcat(urlTotalSuffix, urlPreSuffix);
	    strcat(urlTotalSuffix, "/");
	  }
	  strcat(urlTotalSuffix, urlSuffix);
	  if (authenticationOK("SETUP", urlTotalSuffix, (char const*)fRequestBuffer)) {
	    u_int32_t sessionId;
	    do {
	      sessionId = (u_int32_t)our_random32();
	      sprintf(sessionIdStr, "%08X", sessionId);
	    } while (sessionId == 0 || fOurServer.fClientSessions->Lookup(sessionIdStr) != NULL);
	    clientSession = fOurServer.createNewClientSession(sessionId);
	    fOurServer.fClientSessions->Add(sessionIdStr, clientSession);
	  } else {
	    areAuthenticated = False;
	  }
	}
	if (clientSession != NULL) {
	  clientSession->handleCmd_SETUP(this, urlPreSuffix, urlSuffix, (char const*)fRequestBuffer);
	  playAfterSetup = clientSession->fStreamAfterSETUP;
	} else if (areAuthenticated) {
	  handleCmd_sessionNotFound();
	}
      } else if (strcmp(cmdName, "TEARDOWN") == 0
		 || strcmp(cmdName, "PLAY") == 0
		 || strcmp(cmdName, "PAUSE") == 0
		 || strcmp(cmdName, "GET_PARAMETER") == 0
		 || strcmp(cmdName, "SET_PARAMETER") == 0) {
	if (clientSession != NULL) {
	  clientSession->handleCmd_withinSession(this, cmdName, urlPreSuffix, urlSuffix, (char const*)fRequestBuffer);
	} else {
	  handleCmd_sessionNotFound();
	}
      } else if (strcmp(cmdName, "REGISTER") == 0) {
	// Because - unlike other commands - an implementation of this command needs
	// the entire URL, we re-parse the command to get it:
	char* url = strDupSize((char*)fRequestBuffer);
	if (sscanf((char*)fRequestBuffer, "%*s %s", url) == 1) {
	  // Check for special command-specific parameters in a "Transport:" header:
	  Boolean reuseConnection, deliverViaTCP;
	  char* proxyURLSuffix;
	  parseTransportHeaderForREGISTER((const char*)fRequestBuffer, reuseConnection, deliverViaTCP, proxyURLSuffix);

	  handleCmd_REGISTER(url, urlSuffix, (char const*)fRequestBuffer, reuseConnection, deliverViaTCP, proxyURLSuffix);
	  delete[] proxyURLSuffix;
	} else {
	  handleCmd_bad();
	}
	delete[] url;
      } else {
	// The command is one that we don't handle:
	handleCmd_notSupported();
      }
    } else {
#ifdef DEBUG
      fprintf(stderr, "parseRTSPRequestString() failed; checking now for HTTP commands (for RTSP-over-HTTP tunneling)...\n");
#endif
      // The request was not (valid) RTSP, but check for a special case: HTTP commands (for setting up RTSP-over-HTTP tunneling):
      char sessionCookie[RTSP_PARAM_STRING_MAX];
      char acceptStr[RTSP_PARAM_STRING_MAX];
      *fLastCRLF = '\0'; // temporarily, for parsing
      parseSucceeded = parseHTTPRequestString(cmdName, sizeof cmdName,
					      urlSuffix, sizeof urlPreSuffix,
					      sessionCookie, sizeof sessionCookie,
					      acceptStr, sizeof acceptStr);
      *fLastCRLF = '\r';
      if (parseSucceeded) {
#ifdef DEBUG
	fprintf(stderr, "parseHTTPRequestString() succeeded, returning cmdName \"%s\", urlSuffix \"%s\", sessionCookie \"%s\", acceptStr \"%s\"\n", cmdName, urlSuffix, sessionCookie, acceptStr);
#endif
	// Check that the HTTP command is valid for RTSP-over-HTTP tunneling: There must be a 'session cookie'.
	Boolean isValidHTTPCmd = True;
	if (strcmp(cmdName, "OPTIONS") == 0) {
	  handleHTTPCmd_OPTIONS();
	} else if (sessionCookie[0] == '\0') {
	  // There was no "x-sessioncookie:" header.  If there was an "Accept: application/x-rtsp-tunnelled" header,
	  // then this is a bad tunneling request.  Otherwise, assume that it's an attempt to access the stream via HTTP.
	  if (strcmp(acceptStr, "application/x-rtsp-tunnelled") == 0) {
	    isValidHTTPCmd = False;
	  } else {
	    handleHTTPCmd_StreamingGET(urlSuffix, (char const*)fRequestBuffer);
	  }
	} else if (strcmp(cmdName, "GET") == 0) {
	  handleHTTPCmd_TunnelingGET(sessionCookie);
	} else if (strcmp(cmdName, "POST") == 0) {
	  // We might have received additional data following the HTTP "POST" command - i.e., the first Base64-encoded RTSP command.
	  // Check for this, and handle it if it exists:
	  unsigned char const* extraData = fLastCRLF+4;
	  unsigned extraDataSize = &fRequestBuffer[fRequestBytesAlreadySeen] - extraData;
	  if (handleHTTPCmd_TunnelingPOST(sessionCookie, extraData, extraDataSize)) {
	    // We don't respond to the "POST" command, and we go away:
	    fIsActive = False;
	    break;
	  }
	} else {
	  isValidHTTPCmd = False;
	}
	if (!isValidHTTPCmd) {
	  handleHTTPCmd_notSupported();
	}
      } else {
#ifdef DEBUG
	fprintf(stderr, "parseHTTPRequestString() failed!\n");
#endif
	handleCmd_bad();
      }
    }
    
#ifdef DEBUG
    fprintf(stderr, "sending response: %s", fResponseBuffer);
#endif
    send(fClientOutputSocket, (char const*)fResponseBuffer, strlen((char*)fResponseBuffer), 0);
    
    if (playAfterSetup) {
      // The client has asked for streaming to commence now, rather than after a
      // subsequent "PLAY" command.  So, simulate the effect of a "PLAY" command:
      clientSession->handleCmd_withinSession(this, "PLAY", urlPreSuffix, urlSuffix, (char const*)fRequestBuffer);
    }
    
    // Check whether there are extra bytes remaining in the buffer, after the end of the request (a rare case).
    // If so, move them to the front of our buffer, and keep processing it, because it might be a following, pipelined request.
    unsigned requestSize = (fLastCRLF+4-fRequestBuffer) + contentLength;
    numBytesRemaining = fRequestBytesAlreadySeen - requestSize;
    resetRequestBuffer(); // to prepare for any subsequent request
    
    if (numBytesRemaining > 0) {
      memmove(fRequestBuffer, &fRequestBuffer[requestSize], numBytesRemaining);
      newBytesRead = numBytesRemaining;
    }
  } while (numBytesRemaining > 0);
  
  --fRecursionCount;
  if (!fIsActive) {
    if (fRecursionCount > 0) closeSockets(); else delete this;
    // Note: The "fRecursionCount" test is for a pathological situation where we reenter the event loop and get called recursively
    // while handling a command (e.g., while handling a "DESCRIBE", to get a SDP description).
    // In such a case we don't want to actually delete ourself until we leave the outermost call.
  }
}

static Boolean parseAuthorizationHeader(char const* buf,
					char const*& username,
					char const*& realm,
					char const*& nonce, char const*& uri,
					char const*& response) {
  // Initialize the result parameters to default values:
  username = realm = nonce = uri = response = NULL;
  
  // First, find "Authorization:"
  while (1) {
    if (*buf == '\0') return False; // not found
    if (_strncasecmp(buf, "Authorization: Digest ", 22) == 0) break;
    ++buf;
  }
  
  // Then, run through each of the fields, looking for ones we handle:
  char const* fields = buf + 22;
  while (*fields == ' ') ++fields;
  char* parameter = strDupSize(fields);
  char* value = strDupSize(fields);
  while (1) {
    value[0] = '\0';
    if (sscanf(fields, "%[^=]=\"%[^\"]\"", parameter, value) != 2 &&
	sscanf(fields, "%[^=]=\"\"", parameter) != 1) {
      break;
    }
    if (strcmp(parameter, "username") == 0) {
      username = strDup(value);
    } else if (strcmp(parameter, "realm") == 0) {
      realm = strDup(value);
    } else if (strcmp(parameter, "nonce") == 0) {
      nonce = strDup(value);
    } else if (strcmp(parameter, "uri") == 0) {
      uri = strDup(value);
    } else if (strcmp(parameter, "response") == 0) {
      response = strDup(value);
    }
    
    fields += strlen(parameter) + 2 /*="*/ + strlen(value) + 1 /*"*/;
    while (*fields == ',' || *fields == ' ') ++fields;
        // skip over any separating ',' and ' ' chars
    if (*fields == '\0' || *fields == '\r' || *fields == '\n') break;
  }
  delete[] parameter; delete[] value;
  return True;
}

Boolean RTSPServer::RTSPClientConnection
::authenticationOK(char const* cmdName, char const* urlSuffix, char const* fullRequestStr) {
  if (!fOurServer.specialClientAccessCheck(fClientInputSocket, fClientAddr, urlSuffix)) {
    setRTSPResponse("401 Unauthorized");
    return False;
  }
  
  // If we weren't set up with an authentication database, we're OK:
  UserAuthenticationDatabase* authDB = fOurServer.getAuthenticationDatabaseForCommand(cmdName);
  if (authDB == NULL) return True;
  
  char const* username = NULL; char const* realm = NULL; char const* nonce = NULL;
  char const* uri = NULL; char const* response = NULL;
  Boolean success = False;
  
  do {
    // To authenticate, we first need to have a nonce set up
    // from a previous attempt:
    if (fCurrentAuthenticator.nonce() == NULL) break;
    
    // Next, the request needs to contain an "Authorization:" header,
    // containing a username, (our) realm, (our) nonce, uri,
    // and response string:
    if (!parseAuthorizationHeader(fullRequestStr,
				  username, realm, nonce, uri, response)
	|| username == NULL
	|| realm == NULL || strcmp(realm, fCurrentAuthenticator.realm()) != 0
	|| nonce == NULL || strcmp(nonce, fCurrentAuthenticator.nonce()) != 0
	|| uri == NULL || response == NULL) {
      break;
    }
    
    // Next, the username has to be known to us:
    char const* password = authDB->lookupPassword(username);
#ifdef DEBUG
    fprintf(stderr, "lookupPassword(%s) returned password %s\n", username, password);
#endif
    if (password == NULL) break;
    fCurrentAuthenticator.setUsernameAndPassword(username, password, authDB->passwordsAreMD5());
    
    // Finally, compute a digest response from the information that we have,
    // and compare it to the one that we were given:
    char const* ourResponse
      = fCurrentAuthenticator.computeDigestResponse(cmdName, uri);
    success = (strcmp(ourResponse, response) == 0);
    fCurrentAuthenticator.reclaimDigestResponse(ourResponse);
  } while (0);
  
  delete[] (char*)realm; delete[] (char*)nonce;
  delete[] (char*)uri; delete[] (char*)response;
  
  if (success) {
    // The user has been authenticated.
    // Now allow subclasses a chance to validate the user against the IP address and/or URL suffix.
    if (!fOurServer.specialClientUserAccessCheck(fClientInputSocket, fClientAddr, urlSuffix, username)) {
      // Note: We don't return a "WWW-Authenticate" header here, because the user is valid,
      // even though the server has decided that they should not have access.
      setRTSPResponse("401 Unauthorized");
      delete[] (char*)username;
      return False;
    }
  }
  delete[] (char*)username;
  if (success) return True;
  
  // If we get here, we failed to authenticate the user.
  // Send back a "401 Unauthorized" response, with a new random nonce:
  fCurrentAuthenticator.setRealmAndRandomNonce(authDB->realm());
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 401 Unauthorized\r\n"
	   "CSeq: %s\r\n"
	   "%s"
	   "WWW-Authenticate: Digest realm=\"%s\", nonce=\"%s\"\r\n\r\n",
	   fCurrentCSeq,
	   dateHeader(),
	   fCurrentAuthenticator.realm(), fCurrentAuthenticator.nonce());
  return False;
}

void RTSPServer::RTSPClientConnection
::setRTSPResponse(char const* responseStr) {
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 %s\r\n"
	   "CSeq: %s\r\n"
	   "%s\r\n",
	   responseStr,
	   fCurrentCSeq,
	   dateHeader());
}

void RTSPServer::RTSPClientConnection
::setRTSPResponse(char const* responseStr, u_int32_t sessionId) {
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 %s\r\n"
	   "CSeq: %s\r\n"
	   "%s"
	   "Session: %08X\r\n\r\n",
	   responseStr,
	   fCurrentCSeq,
	   dateHeader(),
	   sessionId);
}

void RTSPServer::RTSPClientConnection
::setRTSPResponse(char const* responseStr, char const* contentStr) {
  if (contentStr == NULL) contentStr = "";
  unsigned const contentLen = strlen(contentStr);
  
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 %s\r\n"
	   "CSeq: %s\r\n"
	   "%s"
	   "Content-Length: %d\r\n\r\n"
	   "%s",
	   responseStr,
	   fCurrentCSeq,
	   dateHeader(),
	   contentLen,
	   contentStr);
}

void RTSPServer::RTSPClientConnection
::setRTSPResponse(char const* responseStr, u_int32_t sessionId, char const* contentStr) {
  if (contentStr == NULL) contentStr = "";
  unsigned const contentLen = strlen(contentStr);
  
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 %s\r\n"
	   "CSeq: %s\r\n"
	   "%s"
	   "Session: %08X\r\n"
	   "Content-Length: %d\r\n\r\n"
	   "%s",
	   responseStr,
	   fCurrentCSeq,
	   dateHeader(),
	   sessionId,
	   contentLen,
	   contentStr);
}

void RTSPServer::RTSPClientConnection
::changeClientInputSocket(int newSocketNum, unsigned char const* extraData, unsigned extraDataSize) {
  envir().taskScheduler().disableBackgroundHandling(fClientInputSocket);
  fClientInputSocket = newSocketNum;
  envir().taskScheduler().setBackgroundHandling(fClientInputSocket, SOCKET_READABLE|SOCKET_EXCEPTION,
						(TaskScheduler::BackgroundHandlerProc*)&incomingRequestHandler, this);
  
  // Also write any extra data to our buffer, and handle it:
  if (extraDataSize > 0 && extraDataSize <= fRequestBufferBytesLeft/*sanity check; should always be true*/) {
    unsigned char* ptr = &fRequestBuffer[fRequestBytesAlreadySeen];
    for (unsigned i = 0; i < extraDataSize; ++i) {
      ptr[i] = extraData[i];
    }
    handleRequestBytes(extraDataSize);
  }
}

void RTSPServer::RTSPClientConnection::continueHandlingREGISTER(ParamsForREGISTER* params) {
  params->fOurConnection->continueHandlingREGISTER1(params);
}

void RTSPServer::RTSPClientConnection::continueHandlingREGISTER1(ParamsForREGISTER* params) {
  // Reuse our socket if requested:
  int socketNumToBackEndServer = params->fReuseConnection ? fClientOutputSocket : -1;

  RTSPServer* ourServer = &fOurServer; // copy the pointer now, in case we "delete this" below
  
  if (socketNumToBackEndServer >= 0) {
    // Because our socket will no longer be used by the server to handle incoming requests, we can now delete this
    // "RTSPClientConnection" object.  We do this now, in case the "implementCmd_REGISTER()" call below would also end up
    // deleting this.
    fClientInputSocket = fClientOutputSocket = -1; // so the socket doesn't get closed when we get deleted
    delete this;
  }
  
  ourServer->implementCmd_REGISTER(params->fURL, params->fURLSuffix, socketNumToBackEndServer,
				   params->fDeliverViaTCP, params->fProxyURLSuffix);
  delete params;
}


////////// RTSPServer::RTSPClientSession implementation //////////

RTSPServer::RTSPClientSession
::RTSPClientSession(RTSPServer& ourServer, u_int32_t sessionId)
  : fOurServer(ourServer), fOurSessionId(sessionId), fOurServerMediaSession(NULL), fIsMulticast(False), fStreamAfterSETUP(False),
    fTCPStreamIdCount(0), fLivenessCheckTask(NULL), fNumStreamStates(0), fStreamStates(NULL) {
  noteLiveness();
}

RTSPServer::RTSPClientSession::~RTSPClientSession() {
  // Turn off any liveness checking:
  envir().taskScheduler().unscheduleDelayedTask(fLivenessCheckTask);
  
  // Remove ourself from the server's 'client sessions' hash table before we go:
  char sessionIdStr[9];
  sprintf(sessionIdStr, "%08X", fOurSessionId);
  fOurServer.fClientSessions->Remove(sessionIdStr);
  
  reclaimStreamStates();
  
  if (fOurServerMediaSession != NULL) {
    fOurServerMediaSession->decrementReferenceCount();
    if (fOurServerMediaSession->referenceCount() == 0
	&& fOurServerMediaSession->deleteWhenUnreferenced()) {
      fOurServer.removeServerMediaSession(fOurServerMediaSession);
      fOurServerMediaSession = NULL;
    }
  }
}

void RTSPServer::RTSPClientSession::reclaimStreamStates() {
  for (unsigned i = 0; i < fNumStreamStates; ++i) {
    if (fStreamStates[i].subsession != NULL) {
      fStreamStates[i].subsession->deleteStream(fOurSessionId, fStreamStates[i].streamToken);
    }
  }
  delete[] fStreamStates; fStreamStates = NULL;
  fNumStreamStates = 0;
}

typedef enum StreamingMode {
  RTP_UDP,
  RTP_TCP,
  RAW_UDP
} StreamingMode;

static void parseTransportHeader(char const* buf,
				 StreamingMode& streamingMode,
				 char*& streamingModeString,
				 char*& destinationAddressStr,
				 u_int8_t& destinationTTL,
				 portNumBits& clientRTPPortNum, // if UDP
				 portNumBits& clientRTCPPortNum, // if UDP
				 unsigned char& rtpChannelId, // if TCP
				 unsigned char& rtcpChannelId // if TCP
				 ) {
  // Initialize the result parameters to default values:
  streamingMode = RTP_UDP;
  streamingModeString = NULL;
  destinationAddressStr = NULL;
  destinationTTL = 255;
  clientRTPPortNum = 0;
  clientRTCPPortNum = 1;
  rtpChannelId = rtcpChannelId = 0xFF;
  
  portNumBits p1, p2;
  unsigned ttl, rtpCid, rtcpCid;
  
  // First, find "Transport:"
  while (1) {
    if (*buf == '\0') return; // not found
    if (*buf == '\r' && *(buf+1) == '\n' && *(buf+2) == '\r') return; // end of the headers => not found
    if (_strncasecmp(buf, "Transport:", 10) == 0) break;
    ++buf;
  }
  
  // Then, run through each of the fields, looking for ones we handle:
  char const* fields = buf + 10;
  while (*fields == ' ') ++fields;
  char* field = strDupSize(fields);
  while (sscanf(fields, "%[^;\r\n]", field) == 1) {
    if (strcmp(field, "RTP/AVP/TCP") == 0) {
      streamingMode = RTP_TCP;
    } else if (strcmp(field, "RAW/RAW/UDP") == 0 ||
	       strcmp(field, "MP2T/H2221/UDP") == 0) {
      streamingMode = RAW_UDP;
      streamingModeString = strDup(field);
    } else if (_strncasecmp(field, "destination=", 12) == 0) {
      delete[] destinationAddressStr;
      destinationAddressStr = strDup(field+12);
    } else if (sscanf(field, "ttl%u", &ttl) == 1) {
      destinationTTL = (u_int8_t)ttl;
    } else if (sscanf(field, "client_port=%hu-%hu", &p1, &p2) == 2) {
      clientRTPPortNum = p1;
      clientRTCPPortNum = streamingMode == RAW_UDP ? 0 : p2; // ignore the second port number if the client asked for raw UDP
    } else if (sscanf(field, "client_port=%hu", &p1) == 1) {
      clientRTPPortNum = p1;
      clientRTCPPortNum = streamingMode == RAW_UDP ? 0 : p1 + 1;
    } else if (sscanf(field, "interleaved=%u-%u", &rtpCid, &rtcpCid) == 2) {
      rtpChannelId = (unsigned char)rtpCid;
      rtcpChannelId = (unsigned char)rtcpCid;
    }
    
    fields += strlen(field);
    while (*fields == ';' || *fields == ' ' || *fields == '\t') ++fields; // skip over separating ';' chars or whitespace
    if (*fields == '\0' || *fields == '\r' || *fields == '\n') break;
  }
  delete[] field;
}

static Boolean parsePlayNowHeader(char const* buf) {
  // Find "x-playNow:" header, if present
  while (1) {
    if (*buf == '\0') return False; // not found
    if (_strncasecmp(buf, "x-playNow:", 10) == 0) break;
    ++buf;
  }
  
  return True;
}

void RTSPServer::RTSPClientSession
::handleCmd_SETUP(RTSPServer::RTSPClientConnection* ourClientConnection,
		  char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr) {
  // Normally, "urlPreSuffix" should be the session (stream) name, and "urlSuffix" should be the subsession (track) name.
  // However (being "liberal in what we accept"), we also handle 'aggregate' SETUP requests (i.e., without a track name),
  // in the special case where we have only a single track.  I.e., in this case, we also handle:
  //    "urlPreSuffix" is empty and "urlSuffix" is the session (stream) name, or
  //    "urlPreSuffix" concatenated with "urlSuffix" (with "/" inbetween) is the session (stream) name.
  char const* streamName = urlPreSuffix; // in the normal case
  char const* trackId = urlSuffix; // in the normal case
  char* concatenatedStreamName = NULL; // in the normal case
  
  do {
    // First, make sure the specified stream name exists:
    ServerMediaSession* sms
      = fOurServer.lookupServerMediaSession(streamName, fOurServerMediaSession == NULL);
    if (sms == NULL) {
      // Check for the special case (noted above), before we give up:
      if (urlPreSuffix[0] == '\0') {
	streamName = urlSuffix;
      } else {
	concatenatedStreamName = new char[strlen(urlPreSuffix) + strlen(urlSuffix) + 2]; // allow for the "/" and the trailing '\0'
	sprintf(concatenatedStreamName, "%s/%s", urlPreSuffix, urlSuffix);
	streamName = concatenatedStreamName;
      }
      trackId = NULL;
      
      // Check again:
      sms = fOurServer.lookupServerMediaSession(streamName, fOurServerMediaSession == NULL);
    }
    if (sms == NULL) {
      if (fOurServerMediaSession == NULL) {
	// The client asked for a stream that doesn't exist (and this session descriptor has not been used before):
	ourClientConnection->handleCmd_notFound();
      } else {
	// The client asked for a stream that doesn't exist, but using a stream id for a stream that does exist. Bad request:
	ourClientConnection->handleCmd_bad();
      }
      break;
    } else {
      if (fOurServerMediaSession == NULL) {
	// We're accessing the "ServerMediaSession" for the first time.
	fOurServerMediaSession = sms;
	fOurServerMediaSession->incrementReferenceCount();
      } else if (sms != fOurServerMediaSession) {
	// The client asked for a stream that's different from the one originally requested for this stream id.  Bad request:
	ourClientConnection->handleCmd_bad();
	break;
      }
    }
    
    if (fStreamStates == NULL) {
      // This is the first "SETUP" for this session.  Set up our array of states for all of this session's subsessions (tracks):
      ServerMediaSubsessionIterator iter(*fOurServerMediaSession);
      for (fNumStreamStates = 0; iter.next() != NULL; ++fNumStreamStates) {} // begin by counting the number of subsessions (tracks)
      
      fStreamStates = new struct streamState[fNumStreamStates];
      
      iter.reset();
      ServerMediaSubsession* subsession;
      for (unsigned i = 0; i < fNumStreamStates; ++i) {
	subsession = iter.next();
	fStreamStates[i].subsession = subsession;
	fStreamStates[i].streamToken = NULL; // for now; it may be changed by the "getStreamParameters()" call that comes later
      }
    }
    
    // Look up information for the specified subsession (track):
    ServerMediaSubsession* subsession = NULL;
    unsigned streamNum;
    if (trackId != NULL && trackId[0] != '\0') { // normal case
      for (streamNum = 0; streamNum < fNumStreamStates; ++streamNum) {
	subsession = fStreamStates[streamNum].subsession;
	if (subsession != NULL && strcmp(trackId, subsession->trackId()) == 0) break;
      }
      if (streamNum >= fNumStreamStates) {
	// The specified track id doesn't exist, so this request fails:
	ourClientConnection->handleCmd_notFound();
	break;
      }
    } else {
      // Weird case: there was no track id in the URL.
      // This works only if we have only one subsession:
      if (fNumStreamStates != 1 || fStreamStates[0].subsession == NULL) {
	ourClientConnection->handleCmd_bad();
	break;
      }
      streamNum = 0;
      subsession = fStreamStates[streamNum].subsession;
    }
    // ASSERT: subsession != NULL
    
    void*& token = fStreamStates[streamNum].streamToken; // alias
    if (token != NULL) {
      // We already handled a "SETUP" for this track (to the same client),
      // so stop any existing streaming of it, before we set it up again:
      subsession->pauseStream(fOurSessionId, token);
      subsession->deleteStream(fOurSessionId, token);
    }

    // Look for a "Transport:" header in the request string, to extract client parameters:
    StreamingMode streamingMode;
    char* streamingModeString = NULL; // set when RAW_UDP streaming is specified
    char* clientsDestinationAddressStr;
    u_int8_t clientsDestinationTTL;
    portNumBits clientRTPPortNum, clientRTCPPortNum;
    unsigned char rtpChannelId, rtcpChannelId;
    parseTransportHeader(fullRequestStr, streamingMode, streamingModeString,
			 clientsDestinationAddressStr, clientsDestinationTTL,
			 clientRTPPortNum, clientRTCPPortNum,
			 rtpChannelId, rtcpChannelId);
    if ((streamingMode == RTP_TCP && rtpChannelId == 0xFF) ||
	(streamingMode != RTP_TCP && ourClientConnection->fClientOutputSocket != ourClientConnection->fClientInputSocket)) {
      // An anomolous situation, caused by a buggy client.  Either:
      //     1/ TCP streaming was requested, but with no "interleaving=" fields.  (QuickTime Player sometimes does this.), or
      //     2/ TCP streaming was not requested, but we're doing RTSP-over-HTTP tunneling (which implies TCP streaming).
      // In either case, we assume TCP streaming, and set the RTP and RTCP channel ids to proper values:
      streamingMode = RTP_TCP;
      rtpChannelId = fTCPStreamIdCount; rtcpChannelId = fTCPStreamIdCount+1;
    }
    if (streamingMode == RTP_TCP) fTCPStreamIdCount += 2;
    
    Port clientRTPPort(clientRTPPortNum);
    Port clientRTCPPort(clientRTCPPortNum);
    
    // Next, check whether a "Range:" or "x-playNow:" header is present in the request.
    // This isn't legal, but some clients do this to combine "SETUP" and "PLAY":
    double rangeStart = 0.0, rangeEnd = 0.0;
    char* absStart = NULL; char* absEnd = NULL;
    Boolean startTimeIsNow;
    if (parseRangeHeader(fullRequestStr, rangeStart, rangeEnd, absStart, absEnd, startTimeIsNow)) {
      delete[] absStart; delete[] absEnd;
      fStreamAfterSETUP = True;
    } else if (parsePlayNowHeader(fullRequestStr)) {
      fStreamAfterSETUP = True;
    } else {
      fStreamAfterSETUP = False;
    }
    
    // Then, get server parameters from the 'subsession':
    int tcpSocketNum = streamingMode == RTP_TCP ? ourClientConnection->fClientOutputSocket : -1;
    netAddressBits destinationAddress = 0;
    u_int8_t destinationTTL = 255;
#ifdef RTSP_ALLOW_CLIENT_DESTINATION_SETTING
    if (clientsDestinationAddressStr != NULL) {
      // Use the client-provided "destination" address.
      // Note: This potentially allows the server to be used in denial-of-service
      // attacks, so don't enable this code unless you're sure that clients are
      // trusted.
      destinationAddress = our_inet_addr(clientsDestinationAddressStr);
    }
    // Also use the client-provided TTL.
    destinationTTL = clientsDestinationTTL;
#endif
    delete[] clientsDestinationAddressStr;
    Port serverRTPPort(0);
    Port serverRTCPPort(0);
    
    // Make sure that we transmit on the same interface that's used by the client (in case we're a multi-homed server):
    struct sockaddr_in sourceAddr; SOCKLEN_T namelen = sizeof sourceAddr;
    getsockname(ourClientConnection->fClientInputSocket, (struct sockaddr*)&sourceAddr, &namelen);
    netAddressBits origSendingInterfaceAddr = SendingInterfaceAddr;
    netAddressBits origReceivingInterfaceAddr = ReceivingInterfaceAddr;
    // NOTE: The following might not work properly, so we ifdef it out for now:
#ifdef HACK_FOR_MULTIHOMED_SERVERS
    ReceivingInterfaceAddr = SendingInterfaceAddr = sourceAddr.sin_addr.s_addr;
#endif
    
    subsession->getStreamParameters(fOurSessionId, ourClientConnection->fClientAddr.sin_addr.s_addr,
				    clientRTPPort, clientRTCPPort,
				    tcpSocketNum, rtpChannelId, rtcpChannelId,
				    destinationAddress, destinationTTL, fIsMulticast,
				    serverRTPPort, serverRTCPPort,
				    fStreamStates[streamNum].streamToken);
    SendingInterfaceAddr = origSendingInterfaceAddr;
    ReceivingInterfaceAddr = origReceivingInterfaceAddr;
    
    AddressString destAddrStr(destinationAddress);
    AddressString sourceAddrStr(sourceAddr);
    char timeoutParameterString[100];
    if (fOurServer.fReclamationTestSeconds > 0) {
      sprintf(timeoutParameterString, ";timeout=%u", fOurServer.fReclamationTestSeconds);
    } else {
      timeoutParameterString[0] = '\0';
    }
    if (fIsMulticast) {
      switch (streamingMode) {
          case RTP_UDP: {
	    snprintf((char*)ourClientConnection->fResponseBuffer, sizeof ourClientConnection->fResponseBuffer,
		     "RTSP/1.0 200 OK\r\n"
		     "CSeq: %s\r\n"
		     "%s"
		     "Transport: RTP/AVP;multicast;destination=%s;source=%s;port=%d-%d;ttl=%d\r\n"
		     "Session: %08X%s\r\n\r\n",
		     ourClientConnection->fCurrentCSeq,
		     dateHeader(),
		     destAddrStr.val(), sourceAddrStr.val(), ntohs(serverRTPPort.num()), ntohs(serverRTCPPort.num()), destinationTTL,
		     fOurSessionId, timeoutParameterString);
	    break;
	  }
          case RTP_TCP: {
	    // multicast streams can't be sent via TCP
	    ourClientConnection->handleCmd_unsupportedTransport();
	    break;
	  }
          case RAW_UDP: {
	    snprintf((char*)ourClientConnection->fResponseBuffer, sizeof ourClientConnection->fResponseBuffer,
		     "RTSP/1.0 200 OK\r\n"
		     "CSeq: %s\r\n"
		     "%s"
		     "Transport: %s;multicast;destination=%s;source=%s;port=%d;ttl=%d\r\n"
		     "Session: %08X%s\r\n\r\n",
		     ourClientConnection->fCurrentCSeq,
		     dateHeader(),
		     streamingModeString, destAddrStr.val(), sourceAddrStr.val(), ntohs(serverRTPPort.num()), destinationTTL,
		     fOurSessionId, timeoutParameterString);
	    break;
	  }
      }
    } else {
      switch (streamingMode) {
          case RTP_UDP: {
	    snprintf((char*)ourClientConnection->fResponseBuffer, sizeof ourClientConnection->fResponseBuffer,
		     "RTSP/1.0 200 OK\r\n"
		     "CSeq: %s\r\n"
		     "%s"
		     "Transport: RTP/AVP;unicast;destination=%s;source=%s;client_port=%d-%d;server_port=%d-%d\r\n"
		     "Session: %08X%s\r\n\r\n",
		     ourClientConnection->fCurrentCSeq,
		     dateHeader(),
		     destAddrStr.val(), sourceAddrStr.val(), ntohs(clientRTPPort.num()), ntohs(clientRTCPPort.num()), ntohs(serverRTPPort.num()), ntohs(serverRTCPPort.num()),
		     fOurSessionId, timeoutParameterString);
	    break;
	  }
          case RTP_TCP: {
	    if (!fOurServer.fAllowStreamingRTPOverTCP) {
	      ourClientConnection->handleCmd_unsupportedTransport();
	    } else {
	      snprintf((char*)ourClientConnection->fResponseBuffer, sizeof ourClientConnection->fResponseBuffer,
		       "RTSP/1.0 200 OK\r\n"
		       "CSeq: %s\r\n"
		       "%s"
		       "Transport: RTP/AVP/TCP;unicast;destination=%s;source=%s;interleaved=%d-%d\r\n"
		       "Session: %08X%s\r\n\r\n",
		       ourClientConnection->fCurrentCSeq,
		       dateHeader(),
		       destAddrStr.val(), sourceAddrStr.val(), rtpChannelId, rtcpChannelId,
		       fOurSessionId, timeoutParameterString);
	    }
	    break;
	  }
          case RAW_UDP: {
	    snprintf((char*)ourClientConnection->fResponseBuffer, sizeof ourClientConnection->fResponseBuffer,
		     "RTSP/1.0 200 OK\r\n"
		     "CSeq: %s\r\n"
		     "%s"
		     "Transport: %s;unicast;destination=%s;source=%s;client_port=%d;server_port=%d\r\n"
		     "Session: %08X%s\r\n\r\n",
		     ourClientConnection->fCurrentCSeq,
		     dateHeader(),
		     streamingModeString, destAddrStr.val(), sourceAddrStr.val(), ntohs(clientRTPPort.num()), ntohs(serverRTPPort.num()),
		     fOurSessionId, timeoutParameterString);
	    break;
	  }
      }
    }
    delete[] streamingModeString;
  } while (0);
  
  delete[] concatenatedStreamName;
}

void RTSPServer::RTSPClientSession
::handleCmd_withinSession(RTSPServer::RTSPClientConnection* ourClientConnection,
			  char const* cmdName,
			  char const* urlPreSuffix, char const* urlSuffix,
			  char const* fullRequestStr) {
  // This will either be:
  // - a non-aggregated operation, if "urlPreSuffix" is the session (stream)
  //   name and "urlSuffix" is the subsession (track) name, or
  // - an aggregated operation, if "urlSuffix" is the session (stream) name,
  //   or "urlPreSuffix" is the session (stream) name, and "urlSuffix" is empty,
  //   or "urlPreSuffix" and "urlSuffix" are both nonempty, but when concatenated, (with "/") form the session (stream) name.
  // Begin by figuring out which of these it is:
  ServerMediaSubsession* subsession;
  
  if (fOurServerMediaSession == NULL) { // There wasn't a previous SETUP!
    ourClientConnection->handleCmd_notSupported();
    return;
  } else if (urlSuffix[0] != '\0' && strcmp(fOurServerMediaSession->streamName(), urlPreSuffix) == 0) {
    // Non-aggregated operation.
    // Look up the media subsession whose track id is "urlSuffix":
    ServerMediaSubsessionIterator iter(*fOurServerMediaSession);
    while ((subsession = iter.next()) != NULL) {
      if (strcmp(subsession->trackId(), urlSuffix) == 0) break; // success
    }
    if (subsession == NULL) { // no such track!
      ourClientConnection->handleCmd_notFound();
      return;
    }
  } else if (strcmp(fOurServerMediaSession->streamName(), urlSuffix) == 0 ||
	     (urlSuffix[0] == '\0' && strcmp(fOurServerMediaSession->streamName(), urlPreSuffix) == 0)) {
    // Aggregated operation
    subsession = NULL;
  } else if (urlPreSuffix[0] != '\0' && urlSuffix[0] != '\0') {
    // Aggregated operation, if <urlPreSuffix>/<urlSuffix> is the session (stream) name:
    unsigned const urlPreSuffixLen = strlen(urlPreSuffix);
    if (strncmp(fOurServerMediaSession->streamName(), urlPreSuffix, urlPreSuffixLen) == 0 &&
	fOurServerMediaSession->streamName()[urlPreSuffixLen] == '/' &&
	strcmp(&(fOurServerMediaSession->streamName())[urlPreSuffixLen+1], urlSuffix) == 0) {
      subsession = NULL;
    } else {
      ourClientConnection->handleCmd_notFound();
      return;
    }
  } else { // the request doesn't match a known stream and/or track at all!
    ourClientConnection->handleCmd_notFound();
    return;
  }
  
  if (strcmp(cmdName, "TEARDOWN") == 0) {
    handleCmd_TEARDOWN(ourClientConnection, subsession);
  } else if (strcmp(cmdName, "PLAY") == 0) {
    handleCmd_PLAY(ourClientConnection, subsession, fullRequestStr);
  } else if (strcmp(cmdName, "PAUSE") == 0) {
    handleCmd_PAUSE(ourClientConnection, subsession);
  } else if (strcmp(cmdName, "GET_PARAMETER") == 0) {
    handleCmd_GET_PARAMETER(ourClientConnection, subsession, fullRequestStr);
  } else if (strcmp(cmdName, "SET_PARAMETER") == 0) {
    handleCmd_SET_PARAMETER(ourClientConnection, subsession, fullRequestStr);
  }
}

void RTSPServer::RTSPClientSession
::handleCmd_TEARDOWN(RTSPServer::RTSPClientConnection* ourClientConnection,
		     ServerMediaSubsession* subsession) {
  unsigned i;
  for (i = 0; i < fNumStreamStates; ++i) {
    if (subsession == NULL /* means: aggregated operation */
	|| subsession == fStreamStates[i].subsession) {
      if (fStreamStates[i].subsession != NULL) {
	fStreamStates[i].subsession->deleteStream(fOurSessionId, fStreamStates[i].streamToken);
	fStreamStates[i].subsession = NULL;
      }
    }
  }
  
  setRTSPResponse(ourClientConnection, "200 OK");
  
  // Optimization: If all subsessions have now been torn down, then we know that we can reclaim our object now.
  // (Without this optimization, however, this object would still get reclaimed later, as a result of a 'liveness' timeout.)
  Boolean noSubsessionsRemain = True;
  for (i = 0; i < fNumStreamStates; ++i) {
    if (fStreamStates[i].subsession != NULL) {
      noSubsessionsRemain = False;
      break;
    }
  }
  if (noSubsessionsRemain) delete this;
}

void RTSPServer::RTSPClientSession
::handleCmd_PLAY(RTSPServer::RTSPClientConnection* ourClientConnection,
		 ServerMediaSubsession* subsession, char const* fullRequestStr) {
  char* rtspURL = fOurServer.rtspURL(fOurServerMediaSession, ourClientConnection->fClientInputSocket);
  unsigned rtspURLSize = strlen(rtspURL);
  
  // Parse the client's "Scale:" header, if any:
  float scale;
  Boolean sawScaleHeader = parseScaleHeader(fullRequestStr, scale);
  
  // Try to set the stream's scale factor to this value:
  if (subsession == NULL /*aggregate op*/) {
    fOurServerMediaSession->testScaleFactor(scale);
  } else {
    subsession->testScaleFactor(scale);
  }
  
  char buf[100];
  char* scaleHeader;
  if (!sawScaleHeader) {
    buf[0] = '\0'; // Because we didn't see a Scale: header, don't send one back
  } else {
    sprintf(buf, "Scale: %f\r\n", scale);
  }
  scaleHeader = strDup(buf);
  
  // Parse the client's "Range:" header, if any:
  float duration = 0.0;
  double rangeStart = 0.0, rangeEnd = 0.0;
  char* absStart = NULL; char* absEnd = NULL;
  Boolean startTimeIsNow;
  Boolean sawRangeHeader
    = parseRangeHeader(fullRequestStr, rangeStart, rangeEnd, absStart, absEnd, startTimeIsNow);
  
  if (sawRangeHeader && absStart == NULL/*not seeking by 'absolute' time*/) {
    // Use this information, plus the stream's duration (if known), to create our own "Range:" header, for the response:
    duration = subsession == NULL /*aggregate op*/
      ? fOurServerMediaSession->duration() : subsession->duration();
    if (duration < 0.0) {
      // We're an aggregate PLAY, but the subsessions have different durations.
      // Use the largest of these durations in our header
      duration = -duration;
    }
    
    // Make sure that "rangeStart" and "rangeEnd" (from the client's "Range:" header)
    // have sane values, before we send back our own "Range:" header in our response:
    if (rangeStart < 0.0) rangeStart = 0.0;
    else if (rangeStart > duration) rangeStart = duration;
    if (rangeEnd < 0.0) rangeEnd = 0.0;
    else if (rangeEnd > duration) rangeEnd = duration;
    if ((scale > 0.0 && rangeStart > rangeEnd && rangeEnd > 0.0) ||
	(scale < 0.0 && rangeStart < rangeEnd)) {
      // "rangeStart" and "rangeEnd" were the wrong way around; swap them:
      double tmp = rangeStart;
      rangeStart = rangeEnd;
      rangeEnd = tmp;
    }
  }
  
  // Create a "RTP-Info:" line.  It will get filled in from each subsession's state:
  char const* rtpInfoFmt =
    "%s" // "RTP-Info:", plus any preceding rtpInfo items
    "%s" // comma separator, if needed
    "url=%s/%s"
    ";seq=%d"
    ";rtptime=%u"
    ;
  unsigned rtpInfoFmtSize = strlen(rtpInfoFmt);
  char* rtpInfo = strDup("RTP-Info: ");
  unsigned i, numRTPInfoItems = 0;
  
  // Do any required seeking/scaling on each subsession, before starting streaming.
  // (However, we don't do this if the "PLAY" request was for just a single subsession
  // of a multiple-subsession stream; for such streams, seeking/scaling can be done
  // only with an aggregate "PLAY".)
  for (i = 0; i < fNumStreamStates; ++i) {
    if (subsession == NULL /* means: aggregated operation */ || fNumStreamStates == 1) {
      if (fStreamStates[i].subsession != NULL) {
	if (sawScaleHeader) {
	  fStreamStates[i].subsession->setStreamScale(fOurSessionId, fStreamStates[i].streamToken, scale);
	}
	if (absStart != NULL) {
	  // Special case handling for seeking by 'absolute' time:
	
	  fStreamStates[i].subsession->seekStream(fOurSessionId, fStreamStates[i].streamToken, absStart, absEnd);
	} else {
	  // Seeking by relative (NPT) time:
	  
	  u_int64_t numBytes;
	  if (!sawRangeHeader || startTimeIsNow) {
	    // We're resuming streaming without seeking, so we just do a 'null' seek
	    // (to get our NPT, and to specify when to end streaming):
	    fStreamStates[i].subsession->nullSeekStream(fOurSessionId, fStreamStates[i].streamToken,
							rangeEnd, numBytes);
	  } else {
	    // We do a real 'seek':
	    double streamDuration = 0.0; // by default; means: stream until the end of the media
	    if (rangeEnd > 0.0 && (rangeEnd+0.001) < duration) {
	      // the 0.001 is because we limited the values to 3 decimal places
	      // We want the stream to end early.  Set the duration we want:
	      streamDuration = rangeEnd - rangeStart;
	      if (streamDuration < 0.0) streamDuration = -streamDuration;
	          // should happen only if scale < 0.0
	    }
	    fStreamStates[i].subsession->seekStream(fOurSessionId, fStreamStates[i].streamToken,
						    rangeStart, streamDuration, numBytes);
	  }
	}
      }
    }
  }
  
  // Create the "Range:" header that we'll send back in our response.
  // (Note that we do this after seeking, in case the seeking operation changed the range start time.)
  if (absStart != NULL) {
    // We're seeking by 'absolute' time:
    if (absEnd == NULL) {
      sprintf(buf, "Range: clock=%s-\r\n", absStart);
    } else {
      sprintf(buf, "Range: clock=%s-%s\r\n", absStart, absEnd);
    }
    delete[] absStart; delete[] absEnd;
  } else {
    // We're seeking by relative (NPT) time:
    if (!sawRangeHeader || startTimeIsNow) {
      // We didn't seek, so in our response, begin the range with the current NPT (normal play time):
      float curNPT = 0.0;
      for (i = 0; i < fNumStreamStates; ++i) {
	if (subsession == NULL /* means: aggregated operation */
	    || subsession == fStreamStates[i].subsession) {
	  if (fStreamStates[i].subsession == NULL) continue;
	  float npt = fStreamStates[i].subsession->getCurrentNPT(fStreamStates[i].streamToken);
	  if (npt > curNPT) curNPT = npt;
	  // Note: If this is an aggregate "PLAY" on a multi-subsession stream,
	  // then it's conceivable that the NPTs of each subsession may differ
	  // (if there has been a previous seek on just one subsession).
	  // In this (unusual) case, we just return the largest NPT; I hope that turns out OK...
	}
      }
      rangeStart = curNPT;
    }

    if (rangeEnd == 0.0 && scale >= 0.0) {
      sprintf(buf, "Range: npt=%.3f-\r\n", rangeStart);
    } else {
      sprintf(buf, "Range: npt=%.3f-%.3f\r\n", rangeStart, rangeEnd);
    }
  }
  char* rangeHeader = strDup(buf);
  
  // Now, start streaming:
  for (i = 0; i < fNumStreamStates; ++i) {
    if (subsession == NULL /* means: aggregated operation */
	|| subsession == fStreamStates[i].subsession) {
      unsigned short rtpSeqNum = 0;
      unsigned rtpTimestamp = 0;
      if (fStreamStates[i].subsession == NULL) continue;
      fStreamStates[i].subsession->startStream(fOurSessionId,
					       fStreamStates[i].streamToken,
					       (TaskFunc*)noteClientLiveness, this,
					       rtpSeqNum, rtpTimestamp,
					       RTSPServer::RTSPClientConnection::handleAlternativeRequestByte, ourClientConnection);
      const char *urlSuffix = fStreamStates[i].subsession->trackId();
      char* prevRTPInfo = rtpInfo;
      unsigned rtpInfoSize = rtpInfoFmtSize
	+ strlen(prevRTPInfo)
	+ 1
	+ rtspURLSize + strlen(urlSuffix)
	+ 5 /*max unsigned short len*/
	+ 10 /*max unsigned (32-bit) len*/
	+ 2 /*allows for trailing \r\n at final end of string*/;
      rtpInfo = new char[rtpInfoSize];
      sprintf(rtpInfo, rtpInfoFmt,
	      prevRTPInfo,
	      numRTPInfoItems++ == 0 ? "" : ",",
	      rtspURL, urlSuffix,
	      rtpSeqNum,
	      rtpTimestamp
	      );
      delete[] prevRTPInfo;
    }
  }
  if (numRTPInfoItems == 0) {
    rtpInfo[0] = '\0';
  } else {
    unsigned rtpInfoLen = strlen(rtpInfo);
    rtpInfo[rtpInfoLen] = '\r';
    rtpInfo[rtpInfoLen+1] = '\n';
    rtpInfo[rtpInfoLen+2] = '\0';
  }
  
  // Fill in the response:
  snprintf((char*)ourClientConnection->fResponseBuffer, sizeof ourClientConnection->fResponseBuffer,
	   "RTSP/1.0 200 OK\r\n"
	   "CSeq: %s\r\n"
	   "%s"
	   "%s"
	   "%s"
	   "Session: %08X\r\n"
	   "%s\r\n",
	   ourClientConnection->fCurrentCSeq,
	   dateHeader(),
	   scaleHeader,
	   rangeHeader,
	   fOurSessionId,
	   rtpInfo);
  delete[] rtpInfo; delete[] rangeHeader;
  delete[] scaleHeader; delete[] rtspURL;
}

void RTSPServer::RTSPClientSession
::handleCmd_PAUSE(RTSPServer::RTSPClientConnection* ourClientConnection,
		  ServerMediaSubsession* subsession) {
  for (unsigned i = 0; i < fNumStreamStates; ++i) {
    if (subsession == NULL /* means: aggregated operation */
	|| subsession == fStreamStates[i].subsession) {
      if (fStreamStates[i].subsession != NULL) {
	fStreamStates[i].subsession->pauseStream(fOurSessionId, fStreamStates[i].streamToken);
      }
    }
  }
  
  setRTSPResponse(ourClientConnection, "200 OK", fOurSessionId);
}

void RTSPServer::RTSPClientSession
::handleCmd_GET_PARAMETER(RTSPServer::RTSPClientConnection* ourClientConnection,
			  ServerMediaSubsession* /*subsession*/, char const* /*fullRequestStr*/) {
  // By default, we implement "GET_PARAMETER" just as a 'keep alive', and send back a dummy response.
  // (If you want to handle "GET_PARAMETER" properly, you can do so by defining a subclass of "RTSPServer"
  // and "RTSPServer::RTSPClientSession", and then reimplement this virtual function in your subclass.)
  setRTSPResponse(ourClientConnection, "200 OK", fOurSessionId, LIVEMEDIA_LIBRARY_VERSION_STRING);
}

void RTSPServer::RTSPClientSession
::handleCmd_SET_PARAMETER(RTSPServer::RTSPClientConnection* ourClientConnection,
			  ServerMediaSubsession* /*subsession*/, char const* /*fullRequestStr*/) {
  // By default, we implement "SET_PARAMETER" just as a 'keep alive', and send back an empty response.
  // (If you want to handle "SET_PARAMETER" properly, you can do so by defining a subclass of "RTSPServer"
  // and "RTSPServer::RTSPClientSession", and then reimplement this virtual function in your subclass.)
  setRTSPResponse(ourClientConnection, "200 OK", fOurSessionId);
}

RTSPServer::RTSPClientConnection*
RTSPServer::createNewClientConnection(int clientSocket, struct sockaddr_in clientAddr) {
  return new RTSPClientConnection(*this, clientSocket, clientAddr);
}

RTSPServer::RTSPClientSession*
RTSPServer::createNewClientSession(u_int32_t sessionId) {
  return new RTSPClientSession(*this, sessionId);
}

void RTSPServer::RTSPClientSession::noteLiveness() {
  if (fOurServer.fReclamationTestSeconds > 0) {
    envir().taskScheduler()
      .rescheduleDelayedTask(fLivenessCheckTask,
			     fOurServer.fReclamationTestSeconds*1000000,
			     (TaskFunc*)livenessTimeoutTask, this);
  }
}

void RTSPServer::RTSPClientSession
::noteClientLiveness(RTSPClientSession* clientSession) {
#ifdef DEBUG
  char const* streamName
    = (clientSession->fOurServerMediaSession == NULL) ? "???" : clientSession->fOurServerMediaSession->streamName();
  fprintf(stderr, "RTSP client session (id \"%08X\", stream name \"%s\"): Liveness indication\n",
	  clientSession->fOurSessionId, streamName);
#endif
  clientSession->noteLiveness();
}

void RTSPServer::RTSPClientSession
::livenessTimeoutTask(RTSPClientSession* clientSession) {
  // If this gets called, the client session is assumed to have timed out,
  // so delete it:
#ifdef DEBUG
  char const* streamName
    = (clientSession->fOurServerMediaSession == NULL) ? "???" : clientSession->fOurServerMediaSession->streamName();
  fprintf(stderr, "RTSP client session (id \"%08X\", stream name \"%s\") has timed out (due to inactivity)\n",
	  clientSession->fOurSessionId, streamName);
#endif
  delete clientSession;
}


////////// ServerMediaSessionIterator implementation //////////

RTSPServer::ServerMediaSessionIterator
::ServerMediaSessionIterator(RTSPServer& server)
  : fOurIterator((server.fServerMediaSessions == NULL)
		 ? NULL : HashTable::Iterator::create(*server.fServerMediaSessions)) {
}

RTSPServer::ServerMediaSessionIterator::~ServerMediaSessionIterator() {
  delete fOurIterator;
}

ServerMediaSession* RTSPServer::ServerMediaSessionIterator::next() {
  if (fOurIterator == NULL) return NULL;
  
  char const* key; // dummy
  return (ServerMediaSession*)(fOurIterator->next(key));
}


////////// UserAuthenticationDatabase implementation //////////

UserAuthenticationDatabase::UserAuthenticationDatabase(char const* realm,
						       Boolean passwordsAreMD5)
  : fTable(HashTable::create(STRING_HASH_KEYS)),
    fRealm(strDup(realm == NULL ? "LIVE555 Streaming Media" : realm)),
    fPasswordsAreMD5(passwordsAreMD5) {
}

UserAuthenticationDatabase::~UserAuthenticationDatabase() {
  delete[] fRealm;
  
  // Delete the allocated 'password' strings that we stored in the table, and then the table itself:
  char* password;
  while ((password = (char*)fTable->RemoveNext()) != NULL) {
    delete[] password;
  }
  delete fTable;
}

void UserAuthenticationDatabase::addUserRecord(char const* username,
					       char const* password) {
  fTable->Add(username, (void*)(strDup(password)));
}

void UserAuthenticationDatabase::removeUserRecord(char const* username) {
  char* password = (char*)(fTable->Lookup(username));
  fTable->Remove(username);
  delete[] password;
}

char const* UserAuthenticationDatabase::lookupPassword(char const* username) {
  return (char const*)(fTable->Lookup(username));
}


///////// RTSPServerWithREGISTERProxying implementation /////////

RTSPServerWithREGISTERProxying* RTSPServerWithREGISTERProxying
::createNew(UsageEnvironment& env, Port ourPort,
	    UserAuthenticationDatabase* authDatabase, UserAuthenticationDatabase* authDatabaseForREGISTER,
	    unsigned reclamationTestSeconds,
	    Boolean streamRTPOverTCP, int verbosityLevelForProxying) {
  int ourSocket = setUpOurSocket(env, ourPort);
  if (ourSocket == -1) return NULL;
  
  return new RTSPServerWithREGISTERProxying(env, ourSocket, ourPort, authDatabase, authDatabaseForREGISTER, reclamationTestSeconds,
					    streamRTPOverTCP, verbosityLevelForProxying);
}

RTSPServerWithREGISTERProxying
::RTSPServerWithREGISTERProxying(UsageEnvironment& env, int ourSocket, Port ourPort,
				 UserAuthenticationDatabase* authDatabase, UserAuthenticationDatabase* authDatabaseForREGISTER,
				 unsigned reclamationTestSeconds,
				 Boolean streamRTPOverTCP, int verbosityLevelForProxying)
  : RTSPServer(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds),
    fStreamRTPOverTCP(streamRTPOverTCP), fVerbosityLevelForProxying(verbosityLevelForProxying),
    fRegisteredProxyCounter(0), fAllowedCommandNames(NULL), fAuthDBForREGISTER(authDatabaseForREGISTER) {
}

RTSPServerWithREGISTERProxying::~RTSPServerWithREGISTERProxying() {
  delete[] fAllowedCommandNames;
}

char const* RTSPServerWithREGISTERProxying::allowedCommandNames() {
  if (fAllowedCommandNames == NULL) {
    char const* baseAllowedCommandNames = RTSPServer::allowedCommandNames();
    char const* newAllowedCommandName = ", REGISTER";
    fAllowedCommandNames = new char[strlen(baseAllowedCommandNames) + strlen(newAllowedCommandName) + 1/* for '\0' */];
    sprintf(fAllowedCommandNames, "%s%s", baseAllowedCommandNames, newAllowedCommandName);
  }
  return fAllowedCommandNames;
}

Boolean RTSPServerWithREGISTERProxying::weImplementREGISTER(char const* proxyURLSuffix, char*& responseStr) {
  // First, check whether we have already proxied a stream as "proxyURLSuffix":
  if (proxyURLSuffix != NULL && lookupServerMediaSession(proxyURLSuffix) != NULL) {
    responseStr = strDup("451 Invalid parameter");
    return False;
  }

  // Otherwise, we will implement it:
  responseStr = NULL;
  return True;
}

void RTSPServerWithREGISTERProxying::implementCmd_REGISTER(char const* url, char const* /*urlSuffix*/, int socketToRemoteServer,
							   Boolean deliverViaTCP, char const* proxyURLSuffix) {
  // Continue setting up proxying for the specified URL.
  // By default:
  //    - We use "registeredProxyStream-N" as the (front-end) stream name (ignoring the back-end stream's 'urlSuffix'),
  //      unless "proxyURLSuffix" is non-NULL (in which case we use that)
  //    - There is no 'username' and 'password' for the back-end stream.  (Thus, access-controlled back-end streams will fail.)
  //    - If "fStreamRTPOverTCP" is True, then we request delivery over TCP, regardless of the value of "deliverViaTCP".
  //      (Otherwise, if "fStreamRTPOverTCP" is False, we use the value of "deliverViaTCP" to decide this.)
  // To change this default behavior, you will need to subclass "RTSPServerWithREGISTERProxying", and reimplement this function.
  
  char const* proxyStreamName;
  char proxyStreamNameBuf[100];
  if (proxyURLSuffix == NULL) {
    sprintf(proxyStreamNameBuf, "registeredProxyStream-%u", ++fRegisteredProxyCounter);
    proxyStreamName = proxyStreamNameBuf;
  } else {
    proxyStreamName = proxyURLSuffix;
  }

  if (fStreamRTPOverTCP) deliverViaTCP = True;
  portNumBits tunnelOverHTTPPortNum = deliverViaTCP ? (portNumBits)(~0) : 0;
      // We don't support streaming from the back-end via RTSP/RTP/RTCP-over-HTTP; only via RTP/RTCP-over-TCP or RTP/RTCP-over-UDP

  ServerMediaSession* sms
    = ProxyServerMediaSession::createNew(envir(), this, url, proxyStreamName, NULL, NULL,
					 tunnelOverHTTPPortNum, fVerbosityLevelForProxying, socketToRemoteServer);
  addServerMediaSession(sms);
  
  // (Regardless of the verbosity level) announce the fact that we're proxying this new stream, and the URL to use to access it:
  char* proxyStreamURL = rtspURL(sms);
  envir() << "Proxying the registered back-end stream \"" << url << "\".\n";
  envir() << "\tPlay this stream using the URL: " << proxyStreamURL << "\n";
  delete[] proxyStreamURL;
}

UserAuthenticationDatabase* RTSPServerWithREGISTERProxying::getAuthenticationDatabaseForCommand(char const* cmdName) {
  if (strcmp(cmdName, "REGISTER") == 0) return fAuthDBForREGISTER;

  return RTSPServer::getAuthenticationDatabaseForCommand(cmdName);
}
