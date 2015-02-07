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
// C++ header

#ifndef _RTSP_REGISTER_SENDER_HH
#define _RTSP_REGISTER_SENDER_HH

#ifndef _RTSP_CLIENT_HH
#include "RTSPClient.hh"
#endif

class RTSPRegisterSender: public RTSPClient {
public:
  static RTSPRegisterSender*
  createNew(UsageEnvironment& env,
	    char const* remoteClientNameOrAddress, portNumBits remoteClientPortNum, char const* rtspURLToRegister,
	    RTSPClient::responseHandler* rtspResponseHandler, Authenticator* authenticator = NULL,
	    Boolean requestStreamingViaTCP = False, char const* proxyURLSuffix = NULL, Boolean reuseConnection = False,
	    int verbosityLevel = 0, char const* applicationName = NULL);

  void grabConnection(int& sock, struct sockaddr_in& remoteAddress); // so that the socket doesn't get closed when we're deleted

protected:
  RTSPRegisterSender(UsageEnvironment& env,
		     char const* remoteClientNameOrAddress, portNumBits remoteClientPortNum, char const* rtspURLToRegister,
		     RTSPClient::responseHandler* rtspResponseHandler, Authenticator* authenticator,
		     Boolean requestStreamingViaTCP, char const* proxyURLSuffix, Boolean reuseConnection,
		     int verbosityLevel, char const* applicationName);
    // called only by "createNew()"
  virtual ~RTSPRegisterSender();

  // Redefined virtual functions:
  virtual Boolean setRequestFields(RequestRecord* request,
                                   char*& cmdURL, Boolean& cmdURLWasAllocated,
                                   char const*& protocolStr,
                                   char*& extraHeaders, Boolean& extraHeadersWereAllocated);

public: // Some compilers complain if this is "protected:"
  // A subclass of "RTSPClient::RequestRecord", specific to our "REGISTER" command:
  class RequestRecord_REGISTER: public RTSPClient::RequestRecord {
  public:
    RequestRecord_REGISTER(unsigned cseq, RTSPClient::responseHandler* rtspResponseHandler, char const* rtspURLToRegister,
			   Boolean reuseConnection, Boolean requestStreamingViaTCP, char const* proxyURLSuffix);
    virtual ~RequestRecord_REGISTER();

    char const* rtspURLToRegister() const { return fRTSPURLToRegister; }
    Boolean reuseConnection() const { return fReuseConnection; }
    Boolean requestStreamingViaTCP() const { return fRequestStreamingViaTCP; }
    char const* proxyURLSuffix() const { return fProxyURLSuffix; }

  private:
    char* fRTSPURLToRegister;
    Boolean fReuseConnection, fRequestStreamingViaTCP;
    char* fProxyURLSuffix;
  };

private:
  portNumBits fRemoteClientPortNum;
};

#endif
