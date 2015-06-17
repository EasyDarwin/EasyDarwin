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
#include "HTTPProtocol.h"

StrPtrLen HTTPProtocol::sMethods[] =
{
    StrPtrLen("GET"),
    StrPtrLen("HEAD"),
    StrPtrLen("POST"),
    StrPtrLen("OPTIONS"),
    StrPtrLen("PUT"),
    StrPtrLen("DELETE"),
    StrPtrLen("TRACE"),
    StrPtrLen("CONNECT"),
};

HTTPMethod HTTPProtocol::GetMethod(const StrPtrLen* inMethodStr)
{
    HTTPMethod theMethod = httpIllegalMethod;

    if (inMethodStr->Len == 0)
            return httpIllegalMethod;

    switch((inMethodStr->Ptr)[0])
        {
        case 'G':       theMethod = httpGetMethod; break;
        case 'H':       theMethod = httpHeadMethod; break;
        case 'P':       theMethod = httpPostMethod; break;  // Most likely POST and not PUT
        case 'O':       theMethod = httpOptionsMethod; break;
        case 'D':       theMethod = httpDeleteMethod; break;
        case 'T':       theMethod = httpTraceMethod; break;
        case 'C':       theMethod = httpConnectMethod; break;
        }

    if ( (theMethod != httpIllegalMethod) && (inMethodStr->Equal(sMethods[theMethod])) )
            return theMethod;

    // Check for remaining methods (Only PUT method is left)
    if ( inMethodStr->Equal(sMethods[httpPutMethod]) )
            return httpPutMethod;

    return httpIllegalMethod;
}

StrPtrLen HTTPProtocol::sHeaders[] =
{
    StrPtrLen("Connection"),
    StrPtrLen("Date"),
    StrPtrLen("Authorization"),
    StrPtrLen("If-Modified-Since"),
    StrPtrLen("Server"),
    StrPtrLen("WWW-Authenticate"),
    StrPtrLen("Expires"),
    StrPtrLen("Last-Modified"),

    StrPtrLen("Cache-Control"),
    StrPtrLen("Pragma"),
    StrPtrLen("Trailer"),
    StrPtrLen("Transfer-Encoding"),
    StrPtrLen("Upgrade"),
    StrPtrLen("Via"),
    StrPtrLen("Warning"),

    StrPtrLen("Accept"),
    StrPtrLen("Accept-Charset"),
    StrPtrLen("Accept-Encoding"),
    StrPtrLen("Accept-Language"),
    StrPtrLen("Expect"),
    StrPtrLen("From"),
    StrPtrLen("Host"),
    StrPtrLen("If-Match"),
    StrPtrLen("If-None-Match"),
    StrPtrLen("If-Range"),
    StrPtrLen("If-Unmodified-Since"),
    StrPtrLen("Max-Forwards"),
    StrPtrLen("Proxy-Authorization"),
    StrPtrLen("Range"),
    StrPtrLen("Referer"),
    StrPtrLen("TE"),
    StrPtrLen("User-Agent"),

    StrPtrLen("Accept-Ranges"),
    StrPtrLen("Age"),
    StrPtrLen("ETag"),
    StrPtrLen("Location"),
    StrPtrLen("Proxy-Authenticate"),
    StrPtrLen("Retry-After"),
    StrPtrLen("Vary"),

    StrPtrLen("Allow"),
    StrPtrLen("Content-Encoding"),
    StrPtrLen("Content-Language"),
    StrPtrLen("Content-Length"),
    StrPtrLen("Content-Location"),
    StrPtrLen("Content-MD5"),
    StrPtrLen("Content-Range"),
    StrPtrLen("Content-Type"),
    
    StrPtrLen("X-SessionCookie"),
    StrPtrLen("X-Server-IP-Address"),

    StrPtrLen(" ,")
};

HTTPHeader HTTPProtocol::GetHeader(const StrPtrLen* inHeaderStr)
{
    if (inHeaderStr->Len == 0)
            return httpIllegalHeader;

    HTTPHeader theHeader = httpIllegalHeader;

    //chances are this is one of our selected "VIP" headers. so check for this.
    switch((inHeaderStr->Ptr)[0])
        {
        case 'C':       case 'c':       theHeader = httpConnectionHeader; break;
        case 'S':       case 's':       theHeader = httpServerHeader; break;
        case 'D':       case 'd':       theHeader = httpDateHeader; break;
        case 'A':       case 'a':       theHeader = httpAuthorizationHeader; break;
        case 'W':       case 'w':       theHeader = httpWWWAuthenticateHeader; break;
        case 'I':       case 'i':       theHeader = httpIfModifiedSinceHeader; break;
        case 'E':       case 'e':       theHeader = httpExpiresHeader; break;
        case 'L':       case 'l':       theHeader = httpLastModifiedHeader; break;
        // Added this to optimize for HTTP tunnelling in the server (Not really a VIP header)
        case 'X':       case 'x':       theHeader = httpSessionCookieHeader; break;
        }

    if ((theHeader != httpIllegalHeader) && 
            (inHeaderStr->EqualIgnoreCase(sHeaders[theHeader].Ptr, sHeaders[theHeader].Len)))
            return theHeader;

    //If this isn't one of our VIP headers, go through the remaining request headers, trying
    //to find the right one.
    for (SInt32 x = httpNumVIPHeaders; x < httpNumHeaders; x++)
            if (inHeaderStr->EqualIgnoreCase(sHeaders[x].Ptr, sHeaders[x].Len))
                return x;
    return httpIllegalHeader;
}

StrPtrLen HTTPProtocol::sStatusCodeStrings[] =
{
    StrPtrLen("Continue"),              //kContinue
    StrPtrLen("Switching Protocols"),       //kSwitchingProtocols
    StrPtrLen("OK"),                //kOK
    StrPtrLen("Created"),               //kCreated
    StrPtrLen("Accepted"),              //kAccepted
    StrPtrLen("Non Authoritative Information"), //kNonAuthoritativeInformation
    StrPtrLen("No Content"),            //kNoContent
    StrPtrLen("Reset Content"),         //kResetContent
    StrPtrLen("Partial Content"),           //kPartialContent
    StrPtrLen("Multiple Choices"),          //kMultipleChoices
    StrPtrLen("Moved Permanently"),         //kMovedPermanently
    StrPtrLen("Found"),             //kFound
    StrPtrLen("See Other"),             //kSeeOther
    StrPtrLen("Not Modified"),          //kNotModified
    StrPtrLen("Use Proxy"),             //kUseProxy
    StrPtrLen("Temporary Redirect"),        //kTemporaryRedirect
    StrPtrLen("Bad Request"),           //kBadRequest
    StrPtrLen("Unauthorized"),          //kUnAuthorized
    StrPtrLen("Payment Required"),          //kPaymentRequired
    StrPtrLen("Forbidden"),             //kForbidden
    StrPtrLen("Not Found"),             //kNotFound
    StrPtrLen("Method Not Allowed"),        //kMethodNotAllowed
    StrPtrLen("Not Acceptable"),            //kNotAcceptable
    StrPtrLen("Proxy Authentication Required"), //kProxyAuthenticationRequired
    StrPtrLen("Request Time-out"),          //kRequestTimeout
    StrPtrLen("Conflict"),              //kConflict
    StrPtrLen("Gone"),              //kGone
    StrPtrLen("Length Required"),           //kLengthRequired
    StrPtrLen("Precondition Failed"),       //kPreconditionFailed
    StrPtrLen("Request Entity Too Large"),      //kRequestEntityTooLarge
    StrPtrLen("Request-URI Too Large"),     //kRequestURITooLarge
    StrPtrLen("Unsupported Media Type"),        //kUnsupportedMediaType
    StrPtrLen("Request Range Not Satisfiable"), //kRequestRangeNotSatisfiable
    StrPtrLen("Expectation Failed"),        //kExpectationFailed
    StrPtrLen("Internal Server Error"),     //kInternalServerError
    StrPtrLen("Not Implemented"),           //kNotImplemented
    StrPtrLen("Bad Gateway"),           //kBadGateway
    StrPtrLen("Service Unavailable"),       //kServiceUnavailable
    StrPtrLen("Gateway Timeout"),           //kGatewayTimeout
    StrPtrLen("HTTP Version not supported")     //kHTTPVersionNotSupported
};

SInt32 HTTPProtocol::sStatusCodes[] =
{
    100,            //kContinue
    101,            //kSwitchingProtocols
    200,            //kOK
    201,            //kCreated
    202,            //kAccepted
    203,            //kNonAuthoritativeInformation
    204,            //kNoContent
    205,            //kResetContent
    206,            //kPartialContent
    300,            //kMultipleChoices
    301,            //kMovedPermanently
    302,            //kFound
    303,            //kSeeOther
    304,            //kNotModified
    305,            //kUseProxy
    307,            //kTemporaryRedirect
    400,            //kBadRequest
    401,            //kUnAuthorized
    402,            //kPaymentRequired
    403,            //kForbidden
    404,            //kNotFound
    405,            //kMethodNotAllowed
    406,            //kNotAcceptable
    407,            //kProxyAuthenticationRequired
    408,            //kRequestTimeout
    409,            //kConflict
    410,            //kGone
    411,            //kLengthRequired
    412,            //kPreconditionFailed
    413,            //kRequestEntityTooLarge
    414,            //kRequestURITooLarge
    415,            //kUnsupportedMediaType
    416,            //kRequestRangeNotSatisfiable
    417,            //kExpectationFailed
    500,            //kInternalServerError
    501,            //kNotImplemented
    502,            //kBadGateway
    503,            //kServiceUnavailable
    504,            //kGatewayTimeout
    505				//kHTTPVersionNotSupported
};

StrPtrLen HTTPProtocol::sStatusCodeAsStrings[] =
{
  StrPtrLen("100"),               //kContinue
  StrPtrLen("101"),               //kSwitchingProtocols
  StrPtrLen("200"),               //kOK
  StrPtrLen("201"),               //kCreated
  StrPtrLen("202"),               //kAccepted
  StrPtrLen("203"),               //kNonAuthoritativeInformation
  StrPtrLen("204"),               //kNoContent
  StrPtrLen("205"),               //kResetContent
  StrPtrLen("206"),               //kPartialContent
  StrPtrLen("300"),               //kMultipleChoices
  StrPtrLen("301"),               //kMovedPermanently
  StrPtrLen("302"),               //kFound
  StrPtrLen("303"),               //kSeeOther
  StrPtrLen("304"),               //kNotModified
  StrPtrLen("305"),               //kUseProxy
  StrPtrLen("307"),               //kTemporaryRedirect
  StrPtrLen("400"),               //kBadRequest
  StrPtrLen("401"),               //kUnAuthorized
  StrPtrLen("402"),               //kPaymentRequired
  StrPtrLen("403"),               //kForbidden
  StrPtrLen("404"),               //kNotFound
  StrPtrLen("405"),               //kMethodNotAllowed
  StrPtrLen("406"),               //kNotAcceptable
  StrPtrLen("407"),               //kProxyAuthenticationRequired
  StrPtrLen("408"),               //kRequestTimeout
  StrPtrLen("409"),               //kConflict
  StrPtrLen("410"),               //kGone
  StrPtrLen("411"),               //kLengthRequired
  StrPtrLen("412"),               //kPreconditionFailed
  StrPtrLen("413"),               //kRequestEntityTooLarge
  StrPtrLen("414"),               //kRequestURITooLarge
  StrPtrLen("415"),               //kUnsupportedMediaType
  StrPtrLen("416"),               //kRequestRangeNotSatisfiable
  StrPtrLen("417"),               //kExpectationFailed
  StrPtrLen("500"),               //kInternalServerError
  StrPtrLen("501"),               //kNotImplemented
  StrPtrLen("502"),               //kBadGateway
  StrPtrLen("503"),               //kServiceUnavailable
  StrPtrLen("504"),               //kGatewayTimeout
  StrPtrLen("505")                //kHTTPVersionNotSupported
};

StrPtrLen HTTPProtocol::sVersionStrings[] =
{
    StrPtrLen("HTTP/0.9"),
    StrPtrLen("HTTP/1.0"),
    StrPtrLen("HTTP/1.1")
};

HTTPVersion HTTPProtocol::GetVersion(StrPtrLen* versionStr)
{
    if (versionStr->Len != 8)
            return httpIllegalVersion;
    SInt32 limit = httpNumVersions;
    for (SInt32 x = 0; x < limit; x++)
    {
        if (versionStr->EqualIgnoreCase(sVersionStrings[x].Ptr, sVersionStrings[x].Len))
            return x;
    }
  
    return httpIllegalVersion;
}

HTTPStatusCode	HTTPProtocol::GetStatusCodeEnum(SInt32 inCode)
{
	HTTPStatusCode statusCode = httpInternalServerError;
	for (SInt32 x = 0; x < httpNumStatusCodes; x++)
		if (GetStatusCode(x) == inCode)
			return x;
    return statusCode;
}