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
	 File:       RTSPProtocol.cpp

	 Contains:   Implementation of class defined in RTSPProtocol.h

 */

#include "RTSPProtocol.h"

StrPtrLen RTSPProtocol::sRetrProtName("our-retransmit");

StrPtrLen RTSPProtocol::sMethods[] =
{
	StrPtrLen("DESCRIBE"),
	StrPtrLen("SETUP"),
	StrPtrLen("TEARDOWN"),
	StrPtrLen("PLAY"),
	StrPtrLen("PAUSE"),
	StrPtrLen("OPTIONS"),
	StrPtrLen("ANNOUNCE"),
	StrPtrLen("GET_PARAMETER"),
	StrPtrLen("SET_PARAMETER"),
	StrPtrLen("REDIRECT"),
	StrPtrLen("RECORD")
};

QTSS_RTSPMethod
RTSPProtocol::GetMethod(const StrPtrLen &inMethodStr)
{
	//chances are this is one of our selected "VIP" methods. so check for this.
	QTSS_RTSPMethod theMethod = qtssIllegalMethod;

	switch (*inMethodStr.Ptr)
	{
	case 'S':   case 's':   theMethod = qtssSetupMethod;    break;
	case 'D':   case 'd':   theMethod = qtssDescribeMethod; break;
	case 'T':   case 't':   theMethod = qtssTeardownMethod; break;
	case 'O':   case 'o':   theMethod = qtssOptionsMethod;  break;
	case 'A':   case 'a':   theMethod = qtssAnnounceMethod; break;
	}

	if ((theMethod != qtssIllegalMethod) &&
		(inMethodStr.EqualIgnoreCase(sMethods[theMethod].Ptr, sMethods[theMethod].Len)))
		return theMethod;

	for (SInt32 x = qtssNumVIPMethods; x < qtssIllegalMethod; x++)
		if (inMethodStr.EqualIgnoreCase(sMethods[x].Ptr, sMethods[x].Len))
			return x;
	return qtssIllegalMethod;
}


StrPtrLen RTSPProtocol::sHeaders[] =
{
	StrPtrLen("Accept"),
	StrPtrLen("Cseq"),
	StrPtrLen("User-Agent"),
	StrPtrLen("Transport"),
	StrPtrLen("Session"),
	StrPtrLen("Range"),

	StrPtrLen("Accept-Encoding"),
	StrPtrLen("Accept-Language"),
	StrPtrLen("Authorization"),
	StrPtrLen("Bandwidth"),
	StrPtrLen("Blocksize"),
	StrPtrLen("Cache-Control"),
	StrPtrLen("Conference"),
	StrPtrLen("Connection"),
	StrPtrLen("Content-Base"),
	StrPtrLen("Content-Encoding"),
	StrPtrLen("Content-Language"),
	StrPtrLen("Content-length"),
	StrPtrLen("Content-Location"),
	StrPtrLen("Content-Type"),
	StrPtrLen("Date"),
	StrPtrLen("Expires"),
	StrPtrLen("From"),
	StrPtrLen("Host"),
	StrPtrLen("If-Match"),
	StrPtrLen("If-Modified-Since"),
	StrPtrLen("Last-Modified"),
	StrPtrLen("Location"),
	StrPtrLen("Proxy-Authenticate"),
	StrPtrLen("Proxy-Require"),
	StrPtrLen("Referer"),
	StrPtrLen("Retry-After"),
	StrPtrLen("Require"),
	StrPtrLen("RTP-Info"),
	StrPtrLen("Scale"),
	StrPtrLen("Speed"),
	StrPtrLen("Timestamp"),
	StrPtrLen("Vary"),
	StrPtrLen("Via"),
	StrPtrLen("Allow"),
	StrPtrLen("Public"),
	StrPtrLen("Server"),
	StrPtrLen("Unsupported"),
	StrPtrLen("WWW-Authenticate"),
	StrPtrLen(","),
	StrPtrLen("x-Retransmit"),
	StrPtrLen("x-Accept-Retransmit"),
	StrPtrLen("x-RTP-Meta-Info"),
	StrPtrLen("x-Transport-Options"),
	StrPtrLen("x-Packet-Range"),
	StrPtrLen("x-Prebuffer"),
	StrPtrLen("x-Dynamic-Rate"),
	StrPtrLen("x-Accept-Dynamic-Rate"),
	// DJM PROTOTYPE
	StrPtrLen("x-Random-Data-Size"),

	//3gpp release 6 headers
	StrPtrLen("3GPP-Link-Char"),
	StrPtrLen("3GPP-Adaptation"),
	StrPtrLen("3GPP-QoE-Feedback"),
	StrPtrLen("3GPP-QoE-Metrics"),

	//Annex G
	StrPtrLen("x-predecbufsize"),
	StrPtrLen("x-initpredecbufperiod"),
	StrPtrLen("x-initpostdecbufperiod"),
	StrPtrLen("3gpp-videopostdecbufsize")



};

QTSS_RTSPHeader RTSPProtocol::GetRequestHeader(const StrPtrLen &inHeaderStr)
{
	if (inHeaderStr.Len == 0)
		return qtssIllegalHeader;

	QTSS_RTSPHeader theHeader = qtssIllegalHeader;

	//chances are this is one of our selected "VIP" headers. so check for this.
	switch (*inHeaderStr.Ptr)
	{
	case 'C':   case 'c':   theHeader = qtssCSeqHeader;         break;
	case 'S':   case 's':   theHeader = qtssSessionHeader;      break;
	case 'U':   case 'u':   theHeader = qtssUserAgentHeader;    break;
	case 'A':   case 'a':   theHeader = qtssAcceptHeader;       break;
	case 'T':   case 't':   theHeader = qtssTransportHeader;    break;
	case 'R':   case 'r':   theHeader = qtssRangeHeader;        break;
	case 'X':   case 'x':   theHeader = qtssExtensionHeaders;   break;
	}

	//
	// Check to see whether this is one of our extension headers. These
	// are very likely to appear in requests.
	if (theHeader == qtssExtensionHeaders)
	{
		for (SInt32 y = qtssExtensionHeaders; y < qtssNumHeaders; y++)
		{
			if (inHeaderStr.EqualIgnoreCase(sHeaders[y].Ptr, sHeaders[y].Len))
				return y;
		}
	}

	//
	// It's not one of our extension headers, check to see if this is one of
	// our normal VIP headers
	if ((theHeader != qtssIllegalHeader) &&
		(inHeaderStr.EqualIgnoreCase(sHeaders[theHeader].Ptr, sHeaders[theHeader].Len)))
		return theHeader;

	//
	//If this isn't one of our VIP headers, go through the remaining request headers, trying
	//to find the right one.
	for (SInt32 x = qtssNumVIPHeaders; x < qtssNumHeaders; x++)
	{
		if (inHeaderStr.EqualIgnoreCase(sHeaders[x].Ptr, sHeaders[x].Len))
			return x;
	}
	return qtssIllegalHeader;
}



StrPtrLen RTSPProtocol::sStatusCodeStrings[] =
{
	StrPtrLen("Continue"),                              //kContinue
	StrPtrLen("OK"),                                    //kSuccessOK
	StrPtrLen("Created"),                               //kSuccessCreated
	StrPtrLen("Accepted"),                              //kSuccessAccepted
	StrPtrLen("No Content"),                            //kSuccessNoContent
	StrPtrLen("Partial Content"),                       //kSuccessPartialContent
	StrPtrLen("Low on Storage Space"),                  //kSuccessLowOnStorage
	StrPtrLen("Multiple Choices"),                      //kMultipleChoices
	StrPtrLen("Moved Permanently"),                     //kRedirectPermMoved
	StrPtrLen("Found"),                                 //kRedirectTempMoved
	StrPtrLen("See Other"),                             //kRedirectSeeOther
	StrPtrLen("Not Modified"),                          //kRedirectNotModified
	StrPtrLen("Use Proxy"),                             //kUseProxy
	StrPtrLen("Bad Request"),                           //kClientBadRequest
	StrPtrLen("Unauthorized"),                          //kClientUnAuthorized
	StrPtrLen("Payment Required"),                      //kPaymentRequired
	StrPtrLen("Forbidden"),                             //kClientForbidden
	StrPtrLen("Not Found"),                             //kClientNotFound
	StrPtrLen("Method Not Allowed"),                    //kClientMethodNotAllowed
	StrPtrLen("Not Acceptable"),                        //kNotAcceptable
	StrPtrLen("Proxy Authentication Required"),         //kProxyAuthenticationRequired
	StrPtrLen("Request Time-out"),                      //kRequestTimeout
	StrPtrLen("Conflict"),                              //kClientConflict
	StrPtrLen("Gone"),                                  //kGone
	StrPtrLen("Length Required"),                       //kLengthRequired
	StrPtrLen("Precondition Failed"),                   //kPreconditionFailed
	StrPtrLen("Request Entity Too Large"),              //kRequestEntityTooLarge
	StrPtrLen("Request-URI Too Large"),                 //kRequestURITooLarge
	StrPtrLen("Unsupported Media Type"),                //kUnsupportedMediaType
	StrPtrLen("Parameter Not Understood"),              //kClientParameterNotUnderstood
	StrPtrLen("Conference Not Found"),                  //kClientConferenceNotFound
	StrPtrLen("Not Enough Bandwidth"),                  //kClientNotEnoughBandwidth
	StrPtrLen("Session Not Found"),                     //kClientSessionNotFound
	StrPtrLen("Method Not Valid in this State"),        //kClientMethodNotValidInState
	StrPtrLen("Header Field Not Valid For Resource"),   //kClientHeaderFieldNotValid
	StrPtrLen("Invalid Range"),                         //kClientInvalidRange
	StrPtrLen("Parameter Is Read-Only"),                //kClientReadOnlyParameter
	StrPtrLen("Aggregate Option Not Allowed"),          //kClientAggregateOptionNotAllowed
	StrPtrLen("Only Aggregate Option Allowed"),         //kClientAggregateOptionAllowed
	StrPtrLen("Unsupported Transport"),                 //kClientUnsupportedTransport
	StrPtrLen("Destination Unreachable"),               //kClientDestinationUnreachable
	StrPtrLen("Internal Server Error"),                 //kServerInternal
	StrPtrLen("Not Implemented"),                       //kServerNotImplemented
	StrPtrLen("Bad Gateway"),                           //kServerBadGateway
	StrPtrLen("Service Unavailable"),                   //kServerUnavailable
	StrPtrLen("Gateway Timeout"),                       //kServerGatewayTimeout
	StrPtrLen("RTSP Version not supported"),            //kRTSPVersionNotSupported
	StrPtrLen("Option Not Supported")                   //kServerOptionNotSupported
};

SInt32 RTSPProtocol::sStatusCodes[] =
{
	100,        //kContinue
	200,        //kSuccessOK
	201,        //kSuccessCreated
	202,        //kSuccessAccepted
	204,        //kSuccessNoContent
	206,        //kSuccessPartialContent
	250,        //kSuccessLowOnStorage
	300,        //kMultipleChoices
	301,        //kRedirectPermMoved
	302,        //kRedirectTempMoved
	303,        //kRedirectSeeOther
	304,        //kRedirectNotModified
	305,        //kUseProxy
	400,        //kClientBadRequest
	401,        //kClientUnAuthorized
	402,        //kPaymentRequired
	403,        //kClientForbidden
	404,        //kClientNotFound
	405,        //kClientMethodNotAllowed
	406,        //kNotAcceptable
	407,        //kProxyAuthenticationRequired
	408,        //kRequestTimeout
	409,        //kClientConflict
	410,        //kGone
	411,        //kLengthRequired
	412,        //kPreconditionFailed
	413,        //kRequestEntityTooLarge
	414,        //kRequestURITooLarge
	415,        //kUnsupportedMediaType
	451,        //kClientParameterNotUnderstood
	452,        //kClientConferenceNotFound
	453,        //kClientNotEnoughBandwidth
	454,        //kClientSessionNotFound
	455,        //kClientMethodNotValidInState
	456,        //kClientHeaderFieldNotValid
	457,        //kClientInvalidRange
	458,        //kClientReadOnlyParameter
	459,        //kClientAggregateOptionNotAllowed
	460,        //kClientAggregateOptionAllowed
	461,        //kClientUnsupportedTransport
	462,        //kClientDestinationUnreachable
	500,        //kServerInternal
	501,        //kServerNotImplemented
	502,        //kServerBadGateway
	503,        //kServerUnavailable
	504,        //kServerGatewayTimeout
	505,        //kRTSPVersionNotSupported
	551         //kServerOptionNotSupported
};

StrPtrLen RTSPProtocol::sStatusCodeAsStrings[] =
{
	StrPtrLen("100"),       //kContinue
	StrPtrLen("200"),       //kSuccessOK
	StrPtrLen("201"),       //kSuccessCreated
	StrPtrLen("202"),       //kSuccessAccepted
	StrPtrLen("204"),       //kSuccessNoContent
	StrPtrLen("206"),       //kSuccessPartialContent
	StrPtrLen("250"),       //kSuccessLowOnStorage
	StrPtrLen("300"),       //kMultipleChoices
	StrPtrLen("301"),       //kRedirectPermMoved
	StrPtrLen("302"),       //kRedirectTempMoved
	StrPtrLen("303"),       //kRedirectSeeOther
	StrPtrLen("304"),       //kRedirectNotModified
	StrPtrLen("305"),       //kUseProxy
	StrPtrLen("400"),       //kClientBadRequest
	StrPtrLen("401"),       //kClientUnAuthorized
	StrPtrLen("402"),       //kPaymentRequired
	StrPtrLen("403"),       //kClientForbidden
	StrPtrLen("404"),       //kClientNotFound
	StrPtrLen("405"),       //kClientMethodNotAllowed
	StrPtrLen("406"),       //kNotAcceptable
	StrPtrLen("407"),       //kProxyAuthenticationRequired
	StrPtrLen("408"),       //kRequestTimeout
	StrPtrLen("409"),       //kClientConflict
	StrPtrLen("410"),       //kGone
	StrPtrLen("411"),       //kLengthRequired
	StrPtrLen("412"),       //kPreconditionFailed
	StrPtrLen("413"),       //kRequestEntityTooLarge
	StrPtrLen("414"),       //kRequestURITooLarge
	StrPtrLen("415"),       //kUnsupportedMediaType
	StrPtrLen("451"),       //kClientParameterNotUnderstood
	StrPtrLen("452"),       //kClientConferenceNotFound
	StrPtrLen("453"),       //kClientNotEnoughBandwidth
	StrPtrLen("454"),       //kClientSessionNotFound
	StrPtrLen("455"),       //kClientMethodNotValidInState
	StrPtrLen("456"),       //kClientHeaderFieldNotValid
	StrPtrLen("457"),       //kClientInvalidRange
	StrPtrLen("458"),       //kClientReadOnlyParameter
	StrPtrLen("459"),       //kClientAggregateOptionNotAllowed
	StrPtrLen("460"),       //kClientAggregateOptionAllowed
	StrPtrLen("461"),       //kClientUnsupportedTransport
	StrPtrLen("462"),       //kClientDestinationUnreachable
	StrPtrLen("500"),       //kServerInternal
	StrPtrLen("501"),       //kServerNotImplemented
	StrPtrLen("502"),       //kServerBadGateway
	StrPtrLen("503"),       //kServerUnavailable
	StrPtrLen("504"),       //kServerGatewayTimeout
	StrPtrLen("505"),       //kRTSPVersionNotSupported
	StrPtrLen("551")        //kServerOptionNotSupported
};

StrPtrLen RTSPProtocol::sVersionString[] =
{
	StrPtrLen("RTSP/1.0")
};

RTSPProtocol::RTSPVersion
RTSPProtocol::GetVersion(StrPtrLen &versionStr)
{
	if (versionStr.Len != 8)
		return kIllegalVersion;
	else
		return k10Version;
}
