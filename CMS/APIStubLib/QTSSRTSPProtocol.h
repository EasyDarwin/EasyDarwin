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
	Copyleft (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       QTSSRTSPProtocol.h

    Contains:   Constant & Enum definitions for RTSP protocol type parts
                of QTSS API.

    
*/

#ifndef QTSS_RTSPPROTOCOL_H
#define QTSS_RTSPPROTOCOL_H
#include "OSHeaders.h"

#ifdef __cplusplus
extern "C" {
#endif


enum
{
    qtssDescribeMethod      = 0, 
    qtssSetupMethod         = 1,
    qtssTeardownMethod      = 2,
    qtssNumVIPMethods       = 3,

    qtssPlayMethod          = 3,
    qtssPauseMethod         = 4,
    qtssOptionsMethod       = 5,
    qtssAnnounceMethod      = 6,
    qtssGetParameterMethod  = 7,
    qtssSetParameterMethod  = 8,
    qtssRedirectMethod      = 9,
    qtssRecordMethod        = 10,
    
    qtssNumMethods          = 11,
    qtssIllegalMethod       = 11
    
};
typedef UInt32 QTSS_RTSPMethod;


enum
{
    //These are the common request headers (optimized)
    qtssAcceptHeader            = 0,
    qtssCSeqHeader              = 1,
    qtssUserAgentHeader         = 2,
    qtssTransportHeader         = 3,
    qtssSessionHeader           = 4,
    qtssRangeHeader             = 5,
    qtssNumVIPHeaders           = 6,
    
    //Other request headers
    qtssAcceptEncodingHeader    = 6,
    qtssAcceptLanguageHeader    = 7,
    qtssAuthorizationHeader     = 8,        
    qtssBandwidthHeader         = 9,
    qtssBlockSizeHeader         = 10,
    qtssCacheControlHeader      = 11,
    qtssConferenceHeader        = 12,       
    qtssConnectionHeader        = 13,
    qtssContentBaseHeader       = 14,
    qtssContentEncodingHeader   = 15,
    qtssContentLanguageHeader   = 16,
    qtssContentLengthHeader     = 17,
    qtssContentLocationHeader   = 18,
    qtssContentTypeHeader       = 19,
    qtssDateHeader              = 20,
    qtssExpiresHeader           = 21,
    qtssFromHeader              = 22,
    qtssHostHeader              = 23,
    qtssIfMatchHeader           = 24,
    qtssIfModifiedSinceHeader   = 25,
    qtssLastModifiedHeader      = 26,
    qtssLocationHeader          = 27,
    qtssProxyAuthenticateHeader = 28,
    qtssProxyRequireHeader      = 29,
    qtssRefererHeader           = 30,
    qtssRetryAfterHeader        = 31,
    qtssRequireHeader           = 32,
    qtssRTPInfoHeader           = 33,
    qtssScaleHeader             = 34,
    qtssSpeedHeader             = 35,
    qtssTimestampHeader         = 36,
    qtssVaryHeader              = 37,
    qtssViaHeader               = 38,
    qtssNumRequestHeaders       = 39,
    
    //Additional response headers
    qtssAllowHeader             = 39,
    qtssPublicHeader            = 40,
    qtssServerHeader            = 41,
    qtssUnsupportedHeader       = 42,
    qtssWWWAuthenticateHeader   = 43,
    qtssSameAsLastHeader        = 44,
    
    //Newly added headers
    qtssExtensionHeaders        = 45,
    
    qtssXRetransmitHeader       = 45,
    qtssXAcceptRetransmitHeader = 46,
    qtssXRTPMetaInfoHeader      = 47,
    qtssXTransportOptionsHeader = 48,
    qtssXPacketRangeHeader      = 49,
    qtssXPreBufferHeader        = 50,
	qtssXDynamicRateHeader      = 51,
	qtssXAcceptDynamicRateHeader= 52,
	
	// QT Player random data request
	qtssXRandomDataSizeHeader   = 53,
	
	// 3gpp release 6
	qtss3GPPLinkCharHeader      = 54,
	qtss3GPPAdaptationHeader    = 55,
	qtss3GPPQOEFeedback         = 56,
	qtss3GPPQOEMetrics          = 57,

	// 3gpp annex g
	qtssXPreDecBufSizeHeader             = 58,
	qtssXInitPredecBufPeriodHeader       = 59,
	qtssXInitPostDecBufPeriodHeader      = 60,
	qtss3GPPVideoPostDecBufSizeHeader    = 61,
	

	qtssNumHeaders				= 62,
	qtssIllegalHeader 			= 62
    
};
typedef UInt32 QTSS_RTSPHeader;


enum
{
    qtssContinue                        = 0,        //100
    qtssSuccessOK                       = 1,        //200
    qtssSuccessCreated                  = 2,        //201
    qtssSuccessAccepted                 = 3,        //202
    qtssSuccessNoContent                = 4,        //203
    qtssSuccessPartialContent           = 5,        //204
    qtssSuccessLowOnStorage             = 6,        //250
    qtssMultipleChoices                 = 7,        //300
    qtssRedirectPermMoved               = 8,        //301
    qtssRedirectTempMoved               = 9,        //302
    qtssRedirectSeeOther                = 10,       //303
    qtssRedirectNotModified             = 11,       //304
    qtssUseProxy                        = 12,       //305
    qtssClientBadRequest                = 13,       //400
    qtssClientUnAuthorized              = 14,       //401
    qtssPaymentRequired                 = 15,       //402
    qtssClientForbidden                 = 16,       //403
    qtssClientNotFound                  = 17,       //404
    qtssClientMethodNotAllowed          = 18,       //405
    qtssNotAcceptable                   = 19,       //406
    qtssProxyAuthenticationRequired     = 20,       //407
    qtssRequestTimeout                  = 21,       //408
    qtssClientConflict                  = 22,       //409
    qtssGone                            = 23,       //410
    qtssLengthRequired                  = 24,       //411
    qtssPreconditionFailed              = 25,       //412
    qtssRequestEntityTooLarge           = 26,       //413
    qtssRequestURITooLarge              = 27,       //414
    qtssUnsupportedMediaType            = 28,       //415
    qtssClientParameterNotUnderstood    = 29,       //451
    qtssClientConferenceNotFound        = 30,       //452
    qtssClientNotEnoughBandwidth        = 31,       //453
    qtssClientSessionNotFound           = 32,       //454
    qtssClientMethodNotValidInState     = 33,       //455
    qtssClientHeaderFieldNotValid       = 34,       //456
    qtssClientInvalidRange              = 35,       //457
    qtssClientReadOnlyParameter         = 36,       //458
    qtssClientAggregateOptionNotAllowed = 37,       //459
    qtssClientAggregateOptionAllowed    = 38,       //460
    qtssClientUnsupportedTransport      = 39,       //461
    qtssClientDestinationUnreachable    = 40,       //462
    qtssServerInternal                  = 41,       //500
    qtssServerNotImplemented            = 42,       //501
    qtssServerBadGateway                = 43,       //502
    qtssServerUnavailable               = 44,       //503
    qtssServerGatewayTimeout            = 45,       //505
    qtssRTSPVersionNotSupported         = 46,       //504
    qtssServerOptionNotSupported        = 47,       //551
    qtssNumStatusCodes                  = 48
    
};
typedef UInt32 QTSS_RTSPStatusCode;

#ifdef __cplusplus
}
#endif

#endif
