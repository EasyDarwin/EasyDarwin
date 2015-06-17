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
#ifndef __HTTPPROTOCOL_H__
#define __HTTPPROTOCOL_H__

#include "OSHeaders.h"
#include "StrPtrLen.h"

// Versions
enum
{
    http09version = 0,
    http10Version = 1,
    http11Version = 2,
    
    httpNumVersions = 3,
    httpIllegalVersion = 3
};
typedef UInt32 HTTPVersion;

// Methods
enum
{
    httpGetMethod           = 0,
    httpHeadMethod          = 1,
    httpPostMethod          = 2,
    httpOptionsMethod       = 3,
    httpPutMethod           = 4,
    httpDeleteMethod        = 5,
    httpTraceMethod         = 6,
    httpConnectMethod       = 7,

    httpNumMethods          = 8,
    httpIllegalMethod       = 8
};
typedef UInt32 HTTPMethod;

// HTTP Types
// Methods
enum
{
    httpRequestType		= 0,
    httpResponseType	= 1,

    httpNumTypes		= 3,
    httpIllegalType		= 3
};
typedef UInt32 HTTPType;

// Headers
enum
{
    // VIP headers
    httpConnectionHeader        = 0, // general header
    httpDateHeader          = 1, // general header
    httpAuthorizationHeader         = 2, // request header
    httpIfModifiedSinceHeader   = 3, // request header
    httpServerHeader        = 4, // response header
    httpWWWAuthenticateHeader   = 5, // response header
    httpExpiresHeader       = 6, // entity header
    httpLastModifiedHeader      = 7, // entity header
    httpNumVIPHeaders       = 8,

    //Other general http headers
    httpCacheControlHeader      = 8,
    httpPragmaHeader        = 9,
    httpTrailerHeader       = 10,
    httpTransferEncodingHeader  = 11,
    httpUpgradeHeader       = 12,
    httpViaHeader           = 13,
    httpWarningHeader       = 14,

    // Other request headers
    httpAcceptHeader        = 15,
    httpAcceptCharsetHeader     = 16,
    httpAcceptEncodingHeader    = 17,
    httpAcceptLanguageHeader    = 18,
    httpExpectHeader        = 19,
    httpFromHeader          = 20,
    httpHostHeader          = 21,
    httpIfMatchHeader       = 22,
    httpIfNoneMatchHeader       = 23,
    httpIfRangeHeader       = 24,
    httpIfUnmodifiedSinceHeader = 25,
    httpMaxForwardsHeader       = 26,
    httpProxyAuthorizationHeader    = 27,
    httpRangeHeader         = 28,
    httpRefererHeader       = 29,
    httpTEHeader            = 30,
    httpUserAgentHeader     = 31,

    // Other response headers
    httpAcceptRangesHeader      = 32,
    httpAgeHeader           = 33,
    httpETagHeader          = 34,
    httpLocationHeader      = 35,
    httpProxyAuthenticateHeader = 36,
    httpRetryAfterHeader        = 37,
    httpVaryHeader          = 38,

    // Other entity headers
    httpAllowHeader         = 39,
    httpContentEncodingHeader   = 40,
    httpContentLanguageHeader   = 41,
    httpContentLengthHeader     = 42,
    httpContentLocationHeader   = 43,
    httpContentMD5Header        = 44,
    httpContentRangeHeader      = 45,
    httpContentTypeHeader       = 46,

    // QTSS Specific headers
    // Add headers that are not part of the HTTP spec here 
    // Make sure and up the number of headers and httpIllegalHeader number
    httpSessionCookieHeader     = 47,           // Used for HTTP tunnelling
    httpServerIPAddressHeader   = 48,
        
    httpNumHeaders              = 49,
    httpIllegalHeader           = 49
};
typedef UInt32 HTTPHeader;

// Status codes
enum
{
    httpContinue            = 0,            //100
    httpSwitchingProtocols      = 1,            //101
    httpOK              = 2,            //200
    httpCreated         = 3,            //201
    httpAccepted            = 4,            //202
    httpNonAuthoritativeInformation = 5,            //203
    httpNoContent           = 6,            //204
    httpResetContent        = 7,            //205
    httpPartialContent      = 8,            //206
    httpMultipleChoices     = 9,            //300
    httpMovedPermanently        = 10,           //301
    httpFound           = 11,           //302
    httpSeeOther            = 12,           //303
    httpNotModified         = 13,           //304
    httpUseProxy            = 14,           //305
    httpTemporaryRedirect       = 15,           //307
    httpBadRequest          = 16,           //400
    httpUnAuthorized        = 17,           //401
    httpPaymentRequired     = 18,           //402
    httpForbidden           = 19,           //403
    httpNotFound            = 20,           //404
    httpMethodNotAllowed        = 21,           //405
    httpNotAcceptable       = 22,           //406
    httpProxyAuthenticationRequired = 23,           //407
    httpRequestTimeout      = 24,           //408
    httpConflict            = 25,           //409
    httpGone            = 26,           //410
    httpLengthRequired      = 27,           //411
    httpPreconditionFailed      = 28,           //412
    httpRequestEntityTooLarge   = 29,           //413
    httpRequestURITooLarge      = 30,           //414
    httpUnsupportedMediaType    = 31,           //415
    httpRequestRangeNotSatisfiable  = 32,           //416
    httpExpectationFailed       = 33,           //417
    httpInternalServerError     = 34,           //500
    httpNotImplemented      = 35,           //501
    httpBadGateway          = 36,           //502
    httpServiceUnavailable      = 37,           //503
    httpGatewayTimeout      = 38,           //504
    httpHTTPVersionNotSupported = 39,           //505

    httpNumStatusCodes      = 40
};
typedef UInt32 HTTPStatusCode;

class HTTPProtocol
{
public:
    // Methods
    static HTTPMethod                   GetMethod(const StrPtrLen* inMethodStr);
    static StrPtrLen*                   GetMethodString(HTTPMethod inMethod) { return &sMethods[inMethod]; }
    // Headers
    static HTTPHeader                   GetHeader(const StrPtrLen* inHeaderStr);
    static StrPtrLen*                   GetHeaderString(HTTPHeader inHeader) { return &sHeaders[inHeader]; }
    // Status codes
    static StrPtrLen*                   GetStatusCodeString(HTTPStatusCode inStat) { return &sStatusCodeStrings[inStat]; }
    static SInt32                       GetStatusCode(HTTPStatusCode inStat) { return sStatusCodes[inStat]; }
    static StrPtrLen*                   GetStatusCodeAsString(HTTPStatusCode inStat) { return &sStatusCodeAsStrings[inStat]; }  
	static HTTPStatusCode				GetStatusCodeEnum(SInt32 inCode);
    // Versions
    static HTTPVersion                  GetVersion(StrPtrLen* versionStr);
    static StrPtrLen*                   GetVersionString(HTTPVersion version) { return &sVersionStrings[version]; }

private:
    static StrPtrLen                        sMethods[];
    static StrPtrLen                        sHeaders[];
    static StrPtrLen                        sStatusCodeStrings[];
    static StrPtrLen                        sStatusCodeAsStrings[];
    static SInt32                           sStatusCodes[];
    static StrPtrLen                        sVersionStrings[];
};
#endif // __HTTPPROTOCOL_H__
