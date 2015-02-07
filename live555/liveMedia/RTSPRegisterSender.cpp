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
// A special object which, when created, sends a custom RTSP "REGISTER" command to a specified client.
// Implementation

#include "RTSPRegisterSender.hh"
#include <GroupsockHelper.hh> // for MAKE_SOCKADDR_IN

RTSPRegisterSender* RTSPRegisterSender
::createNew(UsageEnvironment& env,
            char const* remoteClientNameOrAddress, portNumBits remoteClientPortNum, char const* rtspURLToRegister,
            RTSPClient::responseHandler* rtspResponseHandler, Authenticator* authenticator,
	    Boolean requestStreamingViaTCP, char const* proxyURLSuffix, Boolean reuseConnection,
            int verbosityLevel, char const* applicationName) {
  return new RTSPRegisterSender(env, remoteClientNameOrAddress, remoteClientPortNum, rtspURLToRegister,
				rtspResponseHandler, authenticator,
				requestStreamingViaTCP, proxyURLSuffix, reuseConnection,
				verbosityLevel, applicationName);
}

void RTSPRegisterSender::grabConnection(int& sock, struct sockaddr_in& remoteAddress) {
  sock = grabSocket();

  MAKE_SOCKADDR_IN(remoteAddr, fServerAddress, htons(fRemoteClientPortNum));
  remoteAddress = remoteAddr;
}

RTSPRegisterSender
::RTSPRegisterSender(UsageEnvironment& env,
		     char const* remoteClientNameOrAddress, portNumBits remoteClientPortNum, char const* rtspURLToRegister,
		     RTSPClient::responseHandler* rtspResponseHandler, Authenticator* authenticator,
		     Boolean requestStreamingViaTCP, char const* proxyURLSuffix, Boolean reuseConnection,
		     int verbosityLevel, char const* applicationName)
  : RTSPClient(env, NULL, verbosityLevel, applicationName, 0, -1),
    fRemoteClientPortNum(remoteClientPortNum) {
  // Set up a connection to the remote client.  To do this, we create a fake "rtsp://" URL for it:
  char const* fakeRTSPURLFmt = "rtsp://%s:%u/";
  unsigned fakeRTSPURLSize = strlen(fakeRTSPURLFmt) + strlen(remoteClientNameOrAddress) + 5/* max port num len */;
  char* fakeRTSPURL = new char[fakeRTSPURLSize];
  sprintf(fakeRTSPURL, fakeRTSPURLFmt, remoteClientNameOrAddress, remoteClientPortNum);
  setBaseURL(fakeRTSPURL);
  delete[] fakeRTSPURL;

  // Send the "REGISTER" request:
  if (authenticator != NULL) fCurrentAuthenticator = *authenticator;
  (void)sendRequest(new RequestRecord_REGISTER(++fCSeq, rtspResponseHandler,
					       rtspURLToRegister, reuseConnection, requestStreamingViaTCP, proxyURLSuffix));
}

RTSPRegisterSender::~RTSPRegisterSender() {
}

Boolean RTSPRegisterSender::setRequestFields(RequestRecord* request,
					     char*& cmdURL, Boolean& cmdURLWasAllocated,
					     char const*& protocolStr,
					     char*& extraHeaders, Boolean& extraHeadersWereAllocated) {
  if (strcmp(request->commandName(), "REGISTER") == 0) {
    RequestRecord_REGISTER* request_REGISTER = (RequestRecord_REGISTER*) request;

    setBaseURL(request_REGISTER->rtspURLToRegister());
    cmdURL = (char*)url();
    cmdURLWasAllocated = False;

    // Generate the "Transport:" header that will contain our REGISTER-specific parameters.  This will be "extraHeaders".
    // First, generate the "proxy_url_suffix" parameter string, if any:
    char* proxyURLSuffixParameterStr;
    if (request_REGISTER->proxyURLSuffix() == NULL) {
      proxyURLSuffixParameterStr = strDup("");
    } else {
      char const* proxyURLSuffixParameterFmt = "; proxy_url_suffix=%s";
      unsigned proxyURLSuffixParameterSize = strlen(proxyURLSuffixParameterFmt)
	+ strlen(request_REGISTER->proxyURLSuffix());
      proxyURLSuffixParameterStr = new char[proxyURLSuffixParameterSize];
      sprintf(proxyURLSuffixParameterStr, proxyURLSuffixParameterFmt, request_REGISTER->proxyURLSuffix());
    }

    char const* transportHeaderFmt = "Transport: %spreferred_delivery_protocol=%s%s\r\n";
    unsigned transportHeaderSize = strlen(transportHeaderFmt) + 100/*conservative*/ + strlen(proxyURLSuffixParameterStr);
    char* transportHeaderStr = new char[transportHeaderSize];
    sprintf(transportHeaderStr, transportHeaderFmt,
	    request_REGISTER->reuseConnection() ? "reuse_connection; " : "",
	    request_REGISTER->requestStreamingViaTCP() ? "interleaved" : "udp",
	    proxyURLSuffixParameterStr);
    delete[] proxyURLSuffixParameterStr;

    extraHeaders = transportHeaderStr;
    extraHeadersWereAllocated = True;

    return True;
  } else {
    return RTSPClient::setRequestFields(request, cmdURL, cmdURLWasAllocated, protocolStr, extraHeaders, extraHeadersWereAllocated);
  }
}

RTSPRegisterSender::RequestRecord_REGISTER
::RequestRecord_REGISTER(unsigned cseq, RTSPClient::responseHandler* rtspResponseHandler, char const* rtspURLToRegister,
			 Boolean reuseConnection, Boolean requestStreamingViaTCP, char const* proxyURLSuffix)
  : RTSPClient::RequestRecord(cseq, "REGISTER", rtspResponseHandler),
    fRTSPURLToRegister(strDup(rtspURLToRegister)),
    fReuseConnection(reuseConnection), fRequestStreamingViaTCP(requestStreamingViaTCP), fProxyURLSuffix(strDup(proxyURLSuffix)) {
}

RTSPRegisterSender::RequestRecord_REGISTER::~RequestRecord_REGISTER() {
  delete[] fRTSPURLToRegister;
  delete[] fProxyURLSuffix;
}
