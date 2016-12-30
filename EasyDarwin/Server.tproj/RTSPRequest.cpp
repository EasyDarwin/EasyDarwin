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
	 File:       RTSPRequest.cpp

	 Contains:   Implementation of RTSPRequest class.
 */


#include "RTSPRequest.h"
#include "RTSPProtocol.h"
#include "QTSServerInterface.h"

#include "RTSPSession.h"
#include "RTSPSessionInterface.h"
#include "StringParser.h"
#include "StringTranslator.h"
#include "OSMemory.h"
#include "QTSS.h"
#include "QTSSModuleUtils.h"
#include "base64.h"
#include "OSArrayObjectDeleter.h"
#include "DateTranslator.h"
#include "SocketUtils.h"

UInt8
RTSPRequest::sURLStopConditions[] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, //0-9      //'\t' is a stop condition
	1, 0, 0, 1, 0, 0, 0, 0, 0, 0, //10-19    //'\r' & '\n' are stop conditions
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //20-29
	0, 0, 1, 0, 0, 0, 0, 0, 0, 0, //30-39    //' '
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //40-49
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //50-59
	0, 0, 0, 1, 0, 0, 0, 0, 0, 0, //60-69   //'?' 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //70-79
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //80-89
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //90-99
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //100-109
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //110-119
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //120-129
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //130-139
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //140-149
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //150-159
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //160-169
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //170-179
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //180-189
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //190-199
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //200-209
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //210-219
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //220-229
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //230-239
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //240-249
	0, 0, 0, 0, 0, 0             //250-255
};

static StrPtrLen    sDefaultRealm("Streaming Server", 16);
static StrPtrLen    sAuthBasicStr("Basic", 5);
static StrPtrLen    sAuthDigestStr("Digest", 6);
static StrPtrLen    sUsernameStr("username", 8);
static StrPtrLen    sRealmStr("realm", 5);
static StrPtrLen    sNonceStr("nonce", 5);
static StrPtrLen    sUriStr("uri", 3);
static StrPtrLen    sQopStr("qop", 3);
static StrPtrLen    sQopAuthStr("auth", 4);
static StrPtrLen    sQopAuthIntStr("auth-int", 8);
static StrPtrLen    sNonceCountStr("nc", 2);
static StrPtrLen    sResponseStr("response", 8);
static StrPtrLen    sOpaqueStr("opaque", 6);
static StrPtrLen    sEqualQuote("=\"", 2);
static StrPtrLen    sQuoteCommaSpace("\", ", 3);
static StrPtrLen    sStaleTrue("stale=\"true\", ", 14);

//Parses the request
QTSS_Error RTSPRequest::Parse()
{
	StringParser parser(this->GetValue(qtssRTSPReqFullRequest));
	Assert(this->GetValue(qtssRTSPReqFullRequest)->Ptr != NULL);

	//parse status line.
	QTSS_Error error = ParseFirstLine(parser);

	//handle any errors that come up    
	if (error != QTSS_NoErr)
		return error;

	error = this->ParseHeaders(parser);
	if (error != QTSS_NoErr)
		return error;

	//Response headers should set themselves up to reflect what's in the request headers
	fResponseKeepAlive = fRequestKeepAlive;

	//Make sure that there was some path that was extracted from this request. If not, there is no way
	//we can process the request, so generate an error
	if (this->GetValue(qtssRTSPReqFilePath)->Len == 0)
		return QTSSModuleUtils::SendErrorResponse(this, qtssClientBadRequest, qtssMsgNoURLInRequest, this->GetValue(qtssRTSPReqFullRequest));

	return QTSS_NoErr;
}

//returns: StatusLineTooLong, SyntaxError, BadMethod
QTSS_Error RTSPRequest::ParseFirstLine(StringParser &parser)
{
	//first get the method
	StrPtrLen theParsedData;
	parser.ConsumeWord(&theParsedData);
	this->SetVal(qtssRTSPReqMethodStr, theParsedData.Ptr, theParsedData.Len);


	//THIS WORKS UNDER THE ASSUMPTION THAT:
	//valid HTTP/1.1 headers are: GET, HEAD, POST, PUT, OPTIONS, DELETE, TRACE
	fMethod = RTSPProtocol::GetMethod(theParsedData);
	if (fMethod == qtssIllegalMethod)
		return QTSSModuleUtils::SendErrorResponse(this, qtssClientBadRequest, qtssMsgBadRTSPMethod, &theParsedData);

	//no longer assume this is a space... instead, just consume whitespace
	parser.ConsumeWhitespace();

	//now parse the uri,for example rtsp://www.easydarwin.org:554/live.sdp?channel=1&token=888888
	QTSS_Error err = ParseURI(parser);
	if (err != QTSS_NoErr)
		return err;

	//no longer assume this is a space... instead, just consume whitespace
	parser.ConsumeWhitespace();

	//if there is a version, consume the version string
	StrPtrLen versionStr;
	parser.ConsumeUntil(&versionStr, StringParser::sEOLMask);

	//check the version
	if (versionStr.Len > 0)
		fVersion = RTSPProtocol::GetVersion(versionStr);

	//go past the end of line
	if (!parser.ExpectEOL())
		return QTSSModuleUtils::SendErrorResponse(this, qtssClientBadRequest, qtssMsgNoRTSPVersion, &theParsedData);
	return QTSS_NoErr;
}

//returns: SyntaxError if there was an error in the uri. Or InternalServerError
QTSS_Error RTSPRequest::ParseURI(StringParser &parser)
{
	//for example: rtsp://www.easydarwin.org:554/live.sdp?channel=1&token=888888
	//read in the complete URL, set it to be the qtssAbsoluteURLParam
	StrPtrLen theURL;
	parser.ConsumeUntilWhitespace(&theURL);
	//qtssRTSPReqAbsoluteURL = rtsp://www.easydarwin.org:554/live.sdp?channel=1&token=888888
	this->SetVal(qtssRTSPReqAbsoluteURL, &theURL);

	StringParser absParser(&theURL);
	StrPtrLen theAbsURL;
	//theAbsURL = rtsp://www.easydarwin.org:554/live.sdp
	absParser.ConsumeUntil(&theAbsURL, sURLStopConditions);

	StringParser urlParser(&theAbsURL);

	//we always should have a slash before the uri.
	//If not, that indicates this is a full URI. Also, this could be a '*' OPTIONS request
	if ((*theAbsURL.Ptr != '/') && (*theAbsURL.Ptr != '*'))
	{
		//if it is a full URL, store the host name off in a separate parameter
		StrPtrLen theRTSPString;
		urlParser.ConsumeLength(&theRTSPString, 7); //consume "rtsp://"
		//assign the host field here to the proper QTSS param
		StrPtrLen theHost;
		urlParser.ConsumeUntil(&theHost, '/');
		// qtssHostHeader = www.easydarwin.org:554
		fHeaderDictionary.SetVal(qtssHostHeader, &theHost);
	}

	// don't allow non-aggregate operations indicated by a url/media track=id
// might need this for rate adapt   if (qtssSetupMethod != fMethod && qtssOptionsMethod != fMethod && qtssSetParameterMethod != fMethod) // any method not a setup, options, or setparameter is not allowed to have a "/trackID=" in the url.
	if (qtssSetupMethod != fMethod) // any method not a setup is not allowed to have a "/trackID=" in the url.
	{
		StrPtrLenDel tempCStr(theAbsURL.GetAsCString());
		StrPtrLen nonaggregate(tempCStr.FindString("/trackID="));
		if (nonaggregate.Len > 0) // check for non-aggregate method and return error
			return QTSSModuleUtils::SendErrorResponse(this, qtssClientAggregateOptionAllowed, qtssMsgBadRTSPMethod, &theAbsURL);
	}

	// don't allow non-aggregate operations like a setup on a playing session
	if (qtssSetupMethod == fMethod) // if it is a setup but we are playing don't allow it
	{
		RTSPSession*  theSession = (RTSPSession*)this->GetSession();
		if (theSession != NULL && theSession->IsPlaying())
			return QTSSModuleUtils::SendErrorResponse(this, qtssClientAggregateOptionAllowed, qtssMsgBadRTSPMethod, &theAbsURL);
	}

	//
	// In case there is no URI at all... we have to fake it.
	static char* sSlashURI = "/";

	//whatever is in this position in the URL must be the URI. Store that
	//in the qtssURLParam. Confused?
	UInt32 uriLen = urlParser.GetDataReceivedLen() - urlParser.GetDataParsedLen();
	if (uriLen > 0)
		// qtssRTSPReqURI = /live.sdp
		this->SetVal(qtssRTSPReqURI, urlParser.GetCurrentPosition(), urlParser.GetDataReceivedLen() - urlParser.GetDataParsedLen());
	else
		//
		// This might happen if there is nothing after the host at all, not even
		// a '/'. This is legal (RFC 2326, Sec 3.2). If so, just pretend that there
		// is a '/'
		this->SetVal(qtssRTSPReqURI, sSlashURI, 1);

	// parse the query string from the url if present.
	// init qtssRTSPReqQueryString dictionary to an empty string
	StrPtrLen queryString;
	// qtssRTSPReqQueryString = channel=1&token=888888
	this->SetVal(qtssRTSPReqQueryString, queryString.Ptr, queryString.Len);

	if (absParser.GetDataRemaining() > 0)
	{
		if (absParser.PeekFast() == '?')
		{
			// we've got some CGI param
			absParser.ConsumeLength(&queryString, 1); // toss '?'

			// consume the rest of the line..
			absParser.ConsumeUntilWhitespace(&queryString);

			this->SetVal(qtssRTSPReqQueryString, queryString.Ptr, queryString.Len);
		}
	}


	//
	// If the is a '*', return right now because '*' is not a path
	// so the below functions don't make any sense.
	if ((*theAbsURL.Ptr == '*') && (theAbsURL.Len == 1))
	{
		this->SetValue(qtssRTSPReqFilePath, 0, theAbsURL.Ptr, theAbsURL.Len, QTSSDictionary::kDontObeyReadOnly);

		return QTSS_NoErr;
	}

	//path strings are statically allocated. Therefore, if they are longer than
	//this length we won't be able to handle the request.
	StrPtrLen* theURLParam = this->GetValue(qtssRTSPReqURI);
	if (theURLParam->Len > RTSPRequestInterface::kMaxFilePathSizeInBytes)
		return QTSSModuleUtils::SendErrorResponse(this, qtssClientBadRequest, qtssMsgURLTooLong, theURLParam);

	//decode the URL, put the result in the separate buffer for the file path,
	//set the file path StrPtrLen to the proper value
	SInt32 theBytesWritten = StringTranslator::DecodeURL(theURLParam->Ptr, theURLParam->Len,
		fFilePath, RTSPRequestInterface::kMaxFilePathSizeInBytes);
	//if negative, an error occurred, reported as an QTSS_Error
	//we also need to leave room for a terminator.
	if ((theBytesWritten < 0) || (theBytesWritten == RTSPRequestInterface::kMaxFilePathSizeInBytes))
	{
		return QTSSModuleUtils::SendErrorResponse(this, qtssClientBadRequest, qtssMsgURLInBadFormat, theURLParam);
	}

	// Convert from a / delimited path to a local file system path
	StringTranslator::DecodePath(fFilePath, theBytesWritten);

	//setup the proper QTSS param
	fFilePath[theBytesWritten] = '\0';
	//this->SetVal(qtssRTSPReqFilePath, fFilePath, theBytesWritten);
	this->SetValue(qtssRTSPReqFilePath, 0, fFilePath, theBytesWritten, QTSSDictionary::kDontObeyReadOnly);



	return QTSS_NoErr;
}


//throws eHTTPNoMoreData and eHTTPOutOfBuffer
QTSS_Error RTSPRequest::ParseHeaders(StringParser& parser)
{
	StrPtrLen theKeyWord;
	bool isStreamOK;

	//Repeat until we get a \r\n\r\n, which signals the end of the headers

	while ((parser.PeekFast() != '\r') && (parser.PeekFast() != '\n'))
	{
		//First get the header identifier

		isStreamOK = parser.GetThru(&theKeyWord, ':');
		if (!isStreamOK)
			return QTSSModuleUtils::SendErrorResponse(this, qtssClientBadRequest, qtssMsgNoColonAfterHeader, this->GetValue(qtssRTSPReqFullRequest));

		theKeyWord.TrimWhitespace();

		//Look up the proper header enumeration based on the header string.
		//Use the enumeration to look up the dictionary ID of this header,
		//and set that dictionary attribute to be whatever is in the body of the header

		UInt32 theHeader = RTSPProtocol::GetRequestHeader(theKeyWord);
		StrPtrLen theHeaderVal;
		parser.ConsumeUntil(&theHeaderVal, StringParser::sEOLMask);

		StrPtrLen theEOL;
		if ((parser.PeekFast() == '\r') || (parser.PeekFast() == '\n'))
		{
			isStreamOK = true;
			parser.ConsumeEOL(&theEOL);
		}
		else
			isStreamOK = false;

		while ((parser.PeekFast() == ' ') || (parser.PeekFast() == '\t'))
		{
			theHeaderVal.Len += theEOL.Len;
			StrPtrLen temp;
			parser.ConsumeUntil(&temp, StringParser::sEOLMask);
			theHeaderVal.Len += temp.Len;

			if ((parser.PeekFast() == '\r') || (parser.PeekFast() == '\n'))
			{
				isStreamOK = true;
				parser.ConsumeEOL(&theEOL);
			}
			else
				isStreamOK = false;
		}

		// If this is an unknown header, ignore it. Otherwise, set the proper
		// dictionary attribute
		if (theHeader != qtssIllegalHeader)
		{
			Assert(theHeader < qtssNumHeaders);
			theHeaderVal.TrimWhitespace();
			fHeaderDictionary.SetVal(theHeader, &theHeaderVal);
		}
		if (!isStreamOK)
			return QTSSModuleUtils::SendErrorResponse(this, qtssClientBadRequest, qtssMsgNoEOLAfterHeader);

		//some headers require some special processing. If this code begins
		//to get out of control, we made need to come up with a function pointer table
		switch (theHeader)
		{
		case qtssSessionHeader:             ParseSessionHeader(); break;
		case qtssTransportHeader:           ParseTransportHeader(); break;
		case qtssRangeHeader:               ParseRangeHeader();     break;
		case qtssIfModifiedSinceHeader:     ParseIfModSinceHeader(); break;
		case qtssXRetransmitHeader:         ParseRetransmitHeader(); break;
		case qtssContentLengthHeader:       ParseContentLengthHeader(); break;
		case qtssSpeedHeader:               ParseSpeedHeader();     break;
		case qtssXTransportOptionsHeader:   ParseTransportOptionsHeader(); break;
		case qtssXPreBufferHeader:          ParsePrebufferHeader(); break;
		case qtssXDynamicRateHeader:		ParseDynamicRateHeader(); break;
		case qtssXRandomDataSizeHeader:		ParseRandomDataSizeHeader(); break;
		case qtssBandwidthHeader:           ParseBandwidthHeader(); break;
		default:    break;
		}
	}

	// Tell the session what the request body length is for this request
	// so that it can prevent people from reading past the end of the request.
	StrPtrLen* theContentLengthBody = fHeaderDictionary.GetValue(qtssContentLengthHeader);
	if (theContentLengthBody->Len > 0)
	{
		StringParser theHeaderParser(fHeaderDictionary.GetValue(qtssContentLengthHeader));
		theHeaderParser.ConsumeWhitespace();
		this->GetSession()->SetRequestBodyLength(theHeaderParser.ConsumeInteger(NULL));
	}

	isStreamOK = parser.ExpectEOL();
	Assert(isStreamOK);
	return QTSS_NoErr;
}

void RTSPRequest::ParseSessionHeader()
{
	StringParser theSessionParser(fHeaderDictionary.GetValue(qtssSessionHeader));
	StrPtrLen theSessionID;
	(void)theSessionParser.GetThru(&theSessionID, ';');
	fHeaderDictionary.SetVal(qtssSessionHeader, &theSessionID);
}

bool RTSPRequest::ParseNetworkModeSubHeader(StrPtrLen* inSubHeader)
{
	static StrPtrLen sUnicast("unicast");
	static StrPtrLen sMulticast("multiicast");
	bool result = false; // true means header was found

	if (!result && inSubHeader->EqualIgnoreCase(sUnicast))
	{
		fNetworkMode = qtssRTPNetworkModeUnicast;
		result = true;
	}

	if (!result && inSubHeader->EqualIgnoreCase(sMulticast))
	{
		fNetworkMode = qtssRTPNetworkModeMulticast;
		result = true;
	}

	return result;
}

void RTSPRequest::ParseTransportHeader()
{
	static char* sRTPAVPTransportStr = "RTP/AVP";

	StringParser theTransParser(fHeaderDictionary.GetValue(qtssTransportHeader));

	//transport header from client: Transport: RTP/AVP;unicast;client_port=5000-5001\r\n
	//                              Transport: RTP/AVP;multicast;ttl=15;destination=229.41.244.93;client_port=5000-5002\r\n
	//                              Transport: RTP/AVP/TCP;unicast\r\n

	//
	// A client may send multiple transports to the server, comma separated.
	// In this case, the server should just pick one and use that. 

	while (theTransParser.GetDataRemaining() > 0)
	{
		(void)theTransParser.ConsumeWhitespace();
		(void)theTransParser.ConsumeUntil(&fFirstTransport, ',');

		if (fFirstTransport.NumEqualIgnoreCase(sRTPAVPTransportStr, ::strlen(sRTPAVPTransportStr)))
			break;

		if (theTransParser.PeekFast() == ',')
			theTransParser.Expect(',');
	}

	StringParser theFirstTransportParser(&fFirstTransport);

	StrPtrLen theTransportSubHeader;
	(void)theFirstTransportParser.GetThru(&theTransportSubHeader, ';');

	while (theTransportSubHeader.Len > 0)
	{

		// Extract the relevent information from the relevent subheader.
		// So far we care about 3 sub-headers

		if (!this->ParseNetworkModeSubHeader(&theTransportSubHeader))
		{
			theTransportSubHeader.TrimWhitespace();

			switch (*theTransportSubHeader.Ptr)
			{
			case 'r':	// rtp/avp/??? Is this tcp or udp?
			case 'R':   // RTP/AVP/??? Is this TCP or UDP?
				{
					if (theTransportSubHeader.EqualIgnoreCase("RTP/AVP/TCP"))
						fTransportType = qtssRTPTransportTypeTCP;
					break;
				}
			case 'c':   //client_port sub-header
			case 'C':   //client_port sub-header
				{
					this->ParseClientPortSubHeader(&theTransportSubHeader);
					break;
				}
			case 'd':   //destination sub-header
			case 'D':   //destination sub-header
				{
					static StrPtrLen sDestinationSubHeader("destination");

					//Parse the header, extract the destination address
					this->ParseAddrSubHeader(&theTransportSubHeader, &sDestinationSubHeader, &fDestinationAddr);
					break;
				}
			case 's':   //source sub-header
			case 'S':   //source sub-header
				{
					//Same as above code
					static StrPtrLen sSourceSubHeader("source");
					this->ParseAddrSubHeader(&theTransportSubHeader, &sSourceSubHeader, &fSourceAddr);
					break;
				}
			case 't':   //time-to-live sub-header
			case 'T':   //time-to-live sub-header
				{
					this->ParseTimeToLiveSubHeader(&theTransportSubHeader);
					break;
				}
			case 'm':   //mode sub-header
			case 'M':   //mode sub-header
				{
					this->ParseModeSubHeader(&theTransportSubHeader);
					break;
				}
			}
		}

		// Move onto the next parameter
		(void)theFirstTransportParser.GetThru(&theTransportSubHeader, ';');
	}
}

void  RTSPRequest::ParseRangeHeader()
{
	StringParser theRangeParser(fHeaderDictionary.GetValue(qtssRangeHeader));

	// Setup the start and stop time dictionary attributes
	this->SetVal(qtssRTSPReqStartTime, &fStartTime, sizeof(fStartTime));
	this->SetVal(qtssRTSPReqStopTime, &fStopTime, sizeof(fStopTime));

	theRangeParser.GetThru(NULL, '=');//consume "npt="
	theRangeParser.ConsumeWhitespace();
	fStartTime = (Float64)theRangeParser.ConsumeNPT();
	//see if there is a stop time as well.
	if (theRangeParser.GetDataRemaining() > 1)
	{
		theRangeParser.GetThru(NULL, '-');
		theRangeParser.ConsumeWhitespace();
		fStopTime = (Float64)theRangeParser.ConsumeNPT();
	}
}

void  RTSPRequest::ParseRetransmitHeader()
{
	StringParser theRetransmitParser(fHeaderDictionary.GetValue(qtssXRetransmitHeader));
	StrPtrLen theProtName;
	bool foundRetransmitProt = false;

	do
	{
		theRetransmitParser.ConsumeWhitespace();
		theRetransmitParser.ConsumeWord(&theProtName);
		theProtName.TrimTrailingWhitespace();
		foundRetransmitProt = theProtName.EqualIgnoreCase(RTSPProtocol::GetRetransmitProtocolName());
	} while ((!foundRetransmitProt) &&
		(theRetransmitParser.GetThru(NULL, ',')));

	if (!foundRetransmitProt)
		return;

	//
	// We are using Reliable RTP as the transport for this stream,
	// but if there was a previous transport header that indicated TCP,
	// do not set the transport to be reliable UDP
	if (fTransportType == qtssRTPTransportTypeUDP)
		fTransportType = qtssRTPTransportTypeReliableUDP;

	StrPtrLen theProtArg;
	while (theRetransmitParser.GetThru(&theProtArg, '='))
	{
		//
		// Parse out params
		static const StrPtrLen kWindow("window");

		theProtArg.TrimWhitespace();
		if (theProtArg.EqualIgnoreCase(kWindow))
		{
			theRetransmitParser.ConsumeWhitespace();
			fWindowSize = theRetransmitParser.ConsumeInteger(NULL);

			// Save out the window size argument as a string so we
			// can easily put it into the response
			// (we never muck with this header)
			fWindowSizeStr.Ptr = theProtArg.Ptr;
			fWindowSizeStr.Len = theRetransmitParser.GetCurrentPosition() - theProtArg.Ptr;
		}

		theRetransmitParser.GetThru(NULL, ';'); //Skip past ';'
	}
}

void  RTSPRequest::ParseContentLengthHeader()
{
	StringParser theContentLenParser(fHeaderDictionary.GetValue(qtssContentLengthHeader));
	theContentLenParser.ConsumeWhitespace();
	fContentLength = theContentLenParser.ConsumeInteger(NULL);
}

void  RTSPRequest::ParsePrebufferHeader()
{
	StringParser thePrebufferParser(fHeaderDictionary.GetValue(qtssXPreBufferHeader));

	StrPtrLen thePrebufferArg;
	while (thePrebufferParser.GetThru(&thePrebufferArg, '='))
	{
		thePrebufferArg.TrimWhitespace();

		static const StrPtrLen kMaxTimeSubHeader("maxtime");
		if (thePrebufferArg.EqualIgnoreCase(kMaxTimeSubHeader))
		{
			thePrebufferParser.ConsumeWhitespace();
			fPrebufferAmt = thePrebufferParser.ConsumeFloat();
		}

		thePrebufferParser.GetThru(NULL, ';'); //Skip past ';'

	}
}

void  RTSPRequest::ParseDynamicRateHeader()
{
	StringParser theParser(fHeaderDictionary.GetValue(qtssXDynamicRateHeader));
	theParser.ConsumeWhitespace();
	SInt32 value = theParser.ConsumeInteger(NULL);

	// fEnableDynamicRate: < 0 undefined, 0 disable, > 0 enable
	if (value > 0)
		fEnableDynamicRateState = 1;
	else
		fEnableDynamicRateState = 0;
}

void  RTSPRequest::ParseIfModSinceHeader()
{
	fIfModSinceDate = DateTranslator::ParseDate(fHeaderDictionary.GetValue(qtssIfModifiedSinceHeader));

	// Only set the param if this is a legal date
	if (fIfModSinceDate != 0)
		this->SetVal(qtssRTSPReqIfModSinceDate, &fIfModSinceDate, sizeof(fIfModSinceDate));
}

void RTSPRequest::ParseSpeedHeader()
{
	StringParser theSpeedParser(fHeaderDictionary.GetValue(qtssSpeedHeader));
	theSpeedParser.ConsumeWhitespace();
	fSpeed = theSpeedParser.ConsumeFloat();
}

void RTSPRequest::ParseTransportOptionsHeader()
{
	StringParser theRTPOptionsParser(fHeaderDictionary.GetValue(qtssXTransportOptionsHeader));
	StrPtrLen theRTPOptionsSubHeader;

	do
	{
		static StrPtrLen sLateTolerance("late-tolerance");

		if (theRTPOptionsSubHeader.NumEqualIgnoreCase(sLateTolerance.Ptr, sLateTolerance.Len))
		{
			StringParser theLateTolParser(&theRTPOptionsSubHeader);
			theLateTolParser.GetThru(NULL, '=');
			theLateTolParser.ConsumeWhitespace();
			fLateTolerance = theLateTolParser.ConsumeFloat();
			fLateToleranceStr = theRTPOptionsSubHeader;
		}

		(void)theRTPOptionsParser.GetThru(&theRTPOptionsSubHeader, ';');

	} while (theRTPOptionsSubHeader.Len > 0);
}


void RTSPRequest::ParseAddrSubHeader(StrPtrLen* inSubHeader, StrPtrLen* inHeaderName, UInt32* outAddr)
{
	if (!inSubHeader || !inHeaderName || !outAddr)
		return;

	StringParser theSubHeaderParser(inSubHeader);

	// Skip over to the value
	StrPtrLen theFirstBit;
	theSubHeaderParser.GetThru(&theFirstBit, '=');
	theFirstBit.TrimWhitespace();

	// First make sure this is the proper subheader
	if (!theFirstBit.EqualIgnoreCase(*inHeaderName))
		return;

	//Find the IP address
	theSubHeaderParser.ConsumeUntilDigit();

	//Set the addr string param.
	StrPtrLen theAddr(theSubHeaderParser.GetCurrentPosition(), theSubHeaderParser.GetDataRemaining());

	//Convert the string to a UInt32 IP address
	char theTerminator = theAddr.Ptr[theAddr.Len];
	theAddr.Ptr[theAddr.Len] = '\0';

	*outAddr = SocketUtils::ConvertStringToAddr(theAddr.Ptr);

	theAddr.Ptr[theAddr.Len] = theTerminator;

}

void RTSPRequest::ParseModeSubHeader(StrPtrLen* inModeSubHeader)
{
	static StrPtrLen sModeSubHeader("mode");
	static StrPtrLen sReceiveMode("receive");
	static StrPtrLen sRecordMode("record");
	StringParser theSubHeaderParser(inModeSubHeader);

	// Skip over to the first port
	StrPtrLen theFirstBit;
	theSubHeaderParser.GetThru(&theFirstBit, '=');
	theFirstBit.TrimWhitespace();

	// Make sure this is the client port subheader
	if (theFirstBit.EqualIgnoreCase(sModeSubHeader)) do
	{
		theSubHeaderParser.ConsumeWhitespace();

		StrPtrLen theMode;
		theSubHeaderParser.ConsumeWord(&theMode);

		if (theMode.EqualIgnoreCase(sReceiveMode) || theMode.EqualIgnoreCase(sRecordMode))
		{
			fTransportMode = qtssRTPTransportModeRecord;
			break;
		}

	} while (false);

}

void RTSPRequest::ParseClientPortSubHeader(StrPtrLen* inClientPortSubHeader)
{
	static StrPtrLen sClientPortSubHeader("client_port");
	static StrPtrLen sErrorMessage("Received invalid client_port field: ");
	StringParser theSubHeaderParser(inClientPortSubHeader);

	// Skip over to the first port
	StrPtrLen theFirstBit;
	theSubHeaderParser.GetThru(&theFirstBit, '=');
	theFirstBit.TrimWhitespace();

	// Make sure this is the client port subheader
	if (!theFirstBit.EqualIgnoreCase(sClientPortSubHeader))
		return;

	// Store the two client ports as integers
	theSubHeaderParser.ConsumeWhitespace();
	fClientPortA = (UInt16)theSubHeaderParser.ConsumeInteger(NULL);
	theSubHeaderParser.GetThru(NULL, '-');
	theSubHeaderParser.ConsumeWhitespace();
	fClientPortB = (UInt16)theSubHeaderParser.ConsumeInteger(NULL);
	if (fClientPortB != fClientPortA + 1) // an error in the port values
	{
		// The following to setup and log the error as a message level 2.
		StrPtrLen *userAgentPtr = fHeaderDictionary.GetValue(qtssUserAgentHeader);
		ResizeableStringFormatter errorPortMessage;
		errorPortMessage.Put(sErrorMessage);
		if (userAgentPtr != NULL)
			errorPortMessage.Put(*userAgentPtr);
		errorPortMessage.PutSpace();
		errorPortMessage.Put(*inClientPortSubHeader);
		errorPortMessage.PutTerminator();
		QTSSModuleUtils::LogError(qtssMessageVerbosity, qtssMsgNoMessage, 0, errorPortMessage.GetBufPtr(), NULL);


		//fix the rtcp port and hope it works.
		fClientPortB = fClientPortA + 1;
	}
}

void RTSPRequest::ParseTimeToLiveSubHeader(StrPtrLen* inTimeToLiveSubHeader)
{
	static StrPtrLen sTimeToLiveSubHeader("ttl");

	StringParser theSubHeaderParser(inTimeToLiveSubHeader);

	// Skip over to the first part
	StrPtrLen theFirstBit;
	theSubHeaderParser.GetThru(&theFirstBit, '=');
	theFirstBit.TrimWhitespace();
	// Make sure this is the ttl subheader
	if (!theFirstBit.EqualIgnoreCase(sTimeToLiveSubHeader))
		return;

	// Parse out the time to live...
	theSubHeaderParser.ConsumeWhitespace();
	fTtl = (UInt16)theSubHeaderParser.ConsumeInteger(NULL);
}

// DJM PROTOTYPE
void  RTSPRequest::ParseRandomDataSizeHeader()
{
	StringParser theContentLenParser(fHeaderDictionary.GetValue(qtssXRandomDataSizeHeader));
	theContentLenParser.ConsumeWhitespace();
	fRandomDataSize = theContentLenParser.ConsumeInteger(NULL);

	if (fRandomDataSize > RTSPSessionInterface::kMaxRandomDataSize) {
		fRandomDataSize = RTSPSessionInterface::kMaxRandomDataSize;
	}
}

void  RTSPRequest::ParseBandwidthHeader()
{
	StringParser theContentLenParser(fHeaderDictionary.GetValue(qtssBandwidthHeader));
	theContentLenParser.ConsumeWhitespace();
	fBandwidthBits = theContentLenParser.ConsumeInteger(NULL);

}



QTSS_Error RTSPRequest::ParseBasicHeader(StringParser *inParsedAuthLinePtr)
{
	QTSS_Error  theErr = QTSS_NoErr;
	fAuthScheme = qtssAuthBasic;

	StrPtrLen authWord;

	inParsedAuthLinePtr->ConsumeWhitespace();
	inParsedAuthLinePtr->ConsumeUntilWhitespace(&authWord);
	if (0 == authWord.Len)
		return theErr;

	char* encodedStr = authWord.GetAsCString();
	OSCharArrayDeleter encodedStrDeleter(encodedStr);

	char *decodedAuthWord = NEW char[Base64decode_len(encodedStr) + 1];
	OSCharArrayDeleter decodedAuthWordDeleter(decodedAuthWord);

	(void)Base64decode(decodedAuthWord, encodedStr);

	StrPtrLen   nameAndPassword;
	nameAndPassword.Set(decodedAuthWord, ::strlen(decodedAuthWord));

	StrPtrLen   name("");
	StrPtrLen   password("");
	StringParser parsedNameAndPassword(&nameAndPassword);

	parsedNameAndPassword.ConsumeUntil(&name, ':');
	parsedNameAndPassword.ConsumeLength(NULL, 1);
	parsedNameAndPassword.GetThruEOL(&password);


	// Set the qtssRTSPReqUserName and qtssRTSPReqUserPassword attributes in the Request object
	(void) this->SetValue(qtssRTSPReqUserName, 0, name.Ptr, name.Len, QTSSDictionary::kDontObeyReadOnly);
	(void) this->SetValue(qtssRTSPReqUserPassword, 0, password.Ptr, password.Len, QTSSDictionary::kDontObeyReadOnly);

	// Also set the qtssUserName attribute in the qtssRTSPReqUserProfile object attribute of the Request Object
	(void)fUserProfile.SetValue(qtssUserName, 0, name.Ptr, name.Len, QTSSDictionary::kDontObeyReadOnly);

	return theErr;
}

QTSS_Error RTSPRequest::ParseDigestHeader(StringParser *inParsedAuthLinePtr)
{
	QTSS_Error  theErr = QTSS_NoErr;
	fAuthScheme = qtssAuthDigest;

	inParsedAuthLinePtr->ConsumeWhitespace();
	StrPtrLen   *authLine = inParsedAuthLinePtr->GetStream();
	if (NULL != authLine)
	{
		StringParser digestAuthLine(authLine);
		digestAuthLine.GetThru(NULL, '=');
		digestAuthLine.ConsumeWhitespace();

		fAuthDigestResponse.Set(authLine->Ptr, authLine->Len);
	}

	while (inParsedAuthLinePtr->GetDataRemaining() != 0)
	{
		StrPtrLen fieldNameAndValue("");
		inParsedAuthLinePtr->GetThru(&fieldNameAndValue, ',');
		StringParser parsedNameAndValue(&fieldNameAndValue);
		StrPtrLen fieldName("");
		StrPtrLen fieldValue("");

		//Parse name="value" pair fields in the auth line
		parsedNameAndValue.ConsumeUntil(&fieldName, '=');
		parsedNameAndValue.ConsumeLength(NULL, 1);
		parsedNameAndValue.GetThruEOL(&fieldValue);
		StringParser::UnQuote(&fieldValue);

		// fieldValue.Ptr below is a pointer to a part of the qtssAuthorizationHeader 
		// as GetValue returns a pointer
		// Since the header attribute remains for the entire time the request is alive
		// we don't need to make copies of the values of each field into the request
		// object, and can just keep pointers to the values
		// Thus, no need to delete memory for the following fields when the request is deleted:
		// fAuthRealm, fAuthNonce, fAuthUri, fAuthNonceCount, fAuthResponse, fAuthOpaque
		if (fieldName.Equal(sUsernameStr)) {
			// Set the qtssRTSPReqUserName attribute in the Request object
			(void) this->SetValue(qtssRTSPReqUserName, 0, fieldValue.Ptr, fieldValue.Len, QTSSDictionary::kDontObeyReadOnly);
			// Also set the qtssUserName attribute in the qtssRTSPReqUserProfile object attribute of the Request Object
			(void)fUserProfile.SetValue(qtssUserName, 0, fieldValue.Ptr, fieldValue.Len, QTSSDictionary::kDontObeyReadOnly);
		}
		else if (fieldName.Equal(sRealmStr)) {
			fAuthRealm.Set(fieldValue.Ptr, fieldValue.Len);
		}
		else if (fieldName.Equal(sNonceStr)) {
			fAuthNonce.Set(fieldValue.Ptr, fieldValue.Len);
		}
		else if (fieldName.Equal(sUriStr)) {
			fAuthUri.Set(fieldValue.Ptr, fieldValue.Len);
		}
		else if (fieldName.Equal(sQopStr)) {
			if (fieldValue.Equal(sQopAuthStr))
				fAuthQop = RTSPSessionInterface::kAuthQop;
			else if (fieldValue.Equal(sQopAuthIntStr))
				fAuthQop = RTSPSessionInterface::kAuthIntQop;
		}
		else if (fieldName.Equal(sNonceCountStr)) {
			fAuthNonceCount.Set(fieldValue.Ptr, fieldValue.Len);
		}
		else if (fieldName.Equal(sResponseStr)) {
			fAuthResponse.Set(fieldValue.Ptr, fieldValue.Len);
		}
		else if (fieldName.Equal(sOpaqueStr)) {
			fAuthOpaque.Set(fieldValue.Ptr, fieldValue.Len);
		}

		inParsedAuthLinePtr->ConsumeWhitespace();
	}

	return theErr;
}

QTSS_Error RTSPRequest::ParseAuthHeader(void)
{
	QTSS_Error  theErr = QTSS_NoErr;
	QTSSDictionary *theRTSPHeaders = this->GetHeaderDictionary();
	StrPtrLen   *authLine = theRTSPHeaders->GetValue(qtssAuthorizationHeader);
	if ((authLine == NULL) || (0 == authLine->Len))
		return theErr;

	StrPtrLen   authWord("");
	StringParser parsedAuthLine(authLine);
	parsedAuthLine.ConsumeUntilWhitespace(&authWord);

	if (authWord.EqualIgnoreCase(sAuthBasicStr.Ptr, sAuthBasicStr.Len))
		return ParseBasicHeader(&parsedAuthLine);

	if (authWord.EqualIgnoreCase(sAuthDigestStr.Ptr, sAuthDigestStr.Len))
		return ParseDigestHeader(&parsedAuthLine);

	return theErr;
}

void RTSPRequest::SetupAuthLocalPath(void)
{
	QTSS_AttributeID theID = qtssRTSPReqFilePath;

	//
	// Get the truncated path on a setup, because setups have the trackID appended
	if (qtssSetupMethod == fMethod)
		theID = qtssRTSPReqFilePathTrunc;

	UInt32 theLen = 0;
	char* theFullPath = QTSSModuleUtils::GetFullPath(this, theID, &theLen, NULL);
	this->SetValue(qtssRTSPReqLocalPath, 0, theFullPath, theLen, QTSSDictionary::kDontObeyReadOnly);
	delete[] theFullPath;
}

QTSS_Error RTSPRequest::SendDigestChallenge(UInt32 qop, StrPtrLen *nonce, StrPtrLen* opaque)
{
	QTSS_Error theErr = QTSS_NoErr;

	char challengeBuf[kAuthChallengeHeaderBufSize];
	ResizeableStringFormatter challengeFormatter(challengeBuf, kAuthChallengeHeaderBufSize);

	StrPtrLen realm;
	char *prefRealmPtr = NULL;
	StrPtrLen *realmPtr = this->GetValue(qtssRTSPReqURLRealm);              // Get auth realm set by the module
	if (realmPtr->Len > 0) {
		realm = *realmPtr;
	}
	else {                                                                  // If module hasn't set the realm
		QTSServerInterface* theServer = QTSServerInterface::GetServer();    // get the realm from prefs
		prefRealmPtr = theServer->GetPrefs()->GetAuthorizationRealm();      // allocates memory
		Assert(prefRealmPtr != NULL);
		if (prefRealmPtr != NULL) {
			realm.Set(prefRealmPtr, strlen(prefRealmPtr));
		}
		else {
			realm = sDefaultRealm;
		}
	}

	// Creating the Challenge header
	challengeFormatter.Put(sAuthDigestStr);             // [Digest]
	challengeFormatter.PutSpace();                      // [Digest ] 
	challengeFormatter.Put(sRealmStr);                  // [Digest realm]
	challengeFormatter.Put(sEqualQuote);                // [Digest realm="]
	challengeFormatter.Put(realm);                      // [Digest realm="somerealm]
	challengeFormatter.Put(sQuoteCommaSpace);           // [Digest realm="somerealm", ]
	if (this->GetStale()) {
		challengeFormatter.Put(sStaleTrue);             // [Digest realm="somerealm", stale="true", ]
	}
	challengeFormatter.Put(sNonceStr);                  // [Digest realm="somerealm", nonce]
	challengeFormatter.Put(sEqualQuote);                // [Digest realm="somerealm", nonce="]
	challengeFormatter.Put(*nonce);                     // [Digest realm="somerealm", nonce="19723343a9fd75e019723343a9fd75e0]
	challengeFormatter.PutChar('"');                    // [Digest realm="somerealm", nonce="19723343a9fd75e019723343a9fd75e0"]
	challengeFormatter.PutTerminator();                 // [Digest realm="somerealm", nonce="19723343a9fd75e019723343a9fd75e0"\0]

	StrPtrLen challengePtr(challengeFormatter.GetBufPtr(), challengeFormatter.GetBytesWritten() - 1);

	this->SetValue(qtssRTSPReqDigestChallenge, 0, challengePtr.Ptr, challengePtr.Len, QTSSDictionary::kDontObeyReadOnly);
	RTSPSessionInterface* thisRTSPSession = this->GetSession();
	if (thisRTSPSession)
	{
		(void)thisRTSPSession->SetValue(qtssRTSPSesLastDigestChallenge, 0, challengePtr.Ptr, challengePtr.Len, QTSSDictionary::kDontObeyReadOnly);
	}

	fStatus = qtssClientUnAuthorized;
	this->SetResponseKeepAlive(true);
	this->AppendHeader(qtssWWWAuthenticateHeader, &challengePtr);
	this->SendHeader();

	// deleting the memory that was allocated in GetPrefs call above
	if (prefRealmPtr != NULL)
	{
		delete[] prefRealmPtr;
	}

	return theErr;
}

QTSS_Error RTSPRequest::SendBasicChallenge(void)
{
	QTSS_Error theErr = QTSS_NoErr;
	char *prefRealmPtr = NULL;

	do
	{
		char realmBuff[kRealmBuffSize] = "Basic realm=\"";
		StrPtrLen challenge(realmBuff);
		StrPtrLen whichRealm;

		// Get the module's realm
		StrPtrLen moduleRealm;
		theErr = this->GetValuePtr(qtssRTSPReqURLRealm, 0, (void **)&moduleRealm.Ptr, &moduleRealm.Len);
		if ((QTSS_NoErr == theErr) && (moduleRealm.Len > 0))
		{
			whichRealm = moduleRealm;
		}
		else
		{
			theErr = QTSS_NoErr;
			// Get the default realm from the config file or use the static default if config realm is not found
			QTSServerInterface* theServer = QTSServerInterface::GetServer();
			prefRealmPtr = theServer->GetPrefs()->GetAuthorizationRealm(); // allocates memory
			Assert(prefRealmPtr != NULL);
			if (prefRealmPtr != NULL)
			{
				whichRealm.Set(prefRealmPtr, strlen(prefRealmPtr));
			}
			else
			{
				whichRealm = sDefaultRealm;
			}
		}

		int realmLen = whichRealm.Len + challenge.Len + 2; // add 2 based on double quote char + end of string 0x00
		if (realmLen > kRealmBuffSize) // The realm is too big so use the default realm
		{
			Assert(0);
			whichRealm = sDefaultRealm;
		}
		memcpy(&challenge.Ptr[challenge.Len], whichRealm.Ptr, whichRealm.Len);
		int newLen = challenge.Len + whichRealm.Len;

		challenge.Ptr[newLen] = '"'; // add the terminating "" this was accounted for with the size check above
		challenge.Ptr[newLen + 1] = 0;// add the 0 terminator this was accounted for with the size check above
		challenge.Len = newLen + 1; // set the real size of the string excluding the 0.

#if (0)
		{  // test code
			char test[256];

			memcpy(test, sDefaultRealm.Ptr, sDefaultRealm.Len);
			test[sDefaultRealm.Len] = 0;
			qtss_printf("the static realm =%s \n", test);

			OSCharArrayDeleter prefDeleter(QTSServerInterface::GetServer()->GetPrefs()->GetAuthorizationRealm());
			memcpy(test, prefDeleter.GetObject(), strlen(prefDeleter.GetObject()));
			test[strlen(prefDeleter.GetObject())] = 0;
			qtss_printf("the Pref realm =%s \n", test);

			memcpy(test, moduleRealm.Ptr, moduleRealm.Len);
			test[moduleRealm.Len] = 0;
			qtss_printf("the moduleRealm  =%s \n", test);

			memcpy(test, whichRealm.Ptr, whichRealm.Len);
			test[whichRealm.Len] = 0;
			qtss_printf("the challenge realm  =%s \n", test);

			memcpy(test, challenge.Ptr, challenge.Len);
			test[challenge.Len] = 0;
			qtss_printf("the challenge string  =%s len = %" _S32BITARG_ "\n", test, challenge.Len);
		}
#endif

		fStatus = qtssClientUnAuthorized;
		this->SetResponseKeepAlive(true);
		this->AppendHeader(qtssWWWAuthenticateHeader, &challenge);
		this->SendHeader();


	} while (false);

	if (prefRealmPtr != NULL)
	{
		delete[] prefRealmPtr;
	}

	return theErr;
}

QTSS_Error RTSPRequest::SendForbiddenResponse(void)
{
	fStatus = qtssClientForbidden;
	this->SetResponseKeepAlive(false);
	this->SendHeader();

	return QTSS_NoErr;
}
