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
    File:       RTSPRequest.h

    Contains:   This class encapsulates a single RTSP request. It stores the meta data
                associated with a request, and provides an interface (through its base
                class) for modules to access request information
                
                It divides the request into a series of states.
                
                State 1: At first, when the object is constructed or right after 
                         its Reset function is called, it is in an uninitialized state.
                State 2: Parse() parses an RTSP header. Once this function returns,
                         most of the request-related query functions have been setup.
                         (unless an error occurs)
                State 3: GetHandler() uses the request information to create the
                         proper request Handler object for this request. After that,
                         it is the Handler object's responsibilty to process the
                         request and send a response to the client.
                    
    

*/

#ifndef __RTSPREQUEST_H__
#define __RTSPREQUEST_H__

#include "RTSPRequestInterface.h"
#include "RTSPRequestStream.h"
#include "RTSPResponseStream.h"
#include "RTSPSessionInterface.h"
#include "StringParser.h"
#include "QTSSRTSPProtocol.h"

//HTTPRequest class definition
class RTSPRequest : public RTSPRequestInterface
{
public:

    //CONSTRUCTOR / DESTRUCTOR
    //these do very little. Just initialize / delete some member data.
    //
    //Arguments:        session: the session this request is on (massive cyclical dependency)
    RTSPRequest(RTSPSessionInterface* session)
        : RTSPRequestInterface(session) {}
    virtual ~RTSPRequest() {}
    
    //Parses the request. Returns an error handler if there was an error encountered
    //in parsing.
    QTSS_Error Parse();
    
    QTSS_Error ParseAuthHeader(void);
    // called by ParseAuthHeader
    QTSS_Error ParseBasicHeader(StringParser *inParsedAuthLinePtr);
    
    // called by ParseAuthHeader
    QTSS_Error ParseDigestHeader(StringParser *inParsedAuthLinePtr);

    void SetupAuthLocalPath(void);
    QTSS_Error SendBasicChallenge(void);
    QTSS_Error SendDigestChallenge(UInt32 qop, StrPtrLen *nonce, StrPtrLen* opaque);
    QTSS_Error SendForbiddenResponse(void);
private:

    //PARSING
    enum { kRealmBuffSize = 512, kAuthNameAndPasswordBuffSize = 128, kAuthChallengeHeaderBufSize = 512};
    
    //Parsing the URI line (first line of request
    QTSS_Error ParseFirstLine(StringParser &parser);
    
    //Utility functions called by above
    QTSS_Error ParseURI(StringParser &parser);

    //Parsing the rest of the headers
    //This assumes that the parser is at the beginning of the headers. It will parse
    //the headers, fill out the data & HTTPParameters object.
    //
    //Returns:      A handler object signifying that a fatal syntax error has occurred
    QTSS_Error ParseHeaders(StringParser& parser);


    //Functions to parse the contents of particuarly complicated headers (as a convienence
    //for modules)
    void    ParseRangeHeader();
    void    ParseTransportHeader();
    void    ParseIfModSinceHeader();
    void    ParseAddrSubHeader(StrPtrLen* inSubHeader, StrPtrLen* inHeaderName, UInt32* outAddr);
    void    ParseRetransmitHeader();
    void    ParseContentLengthHeader();
    void    ParseSpeedHeader();
    void    ParsePrebufferHeader();
    void    ParseTransportOptionsHeader();
    void    ParseSessionHeader();
    void    ParseClientPortSubHeader(StrPtrLen* inClientPortSubHeader);
    void    ParseTimeToLiveSubHeader(StrPtrLen* inTimeToLiveSubHeader);
    void    ParseModeSubHeader(StrPtrLen* inModeSubHeader);
    Bool16  ParseNetworkModeSubHeader(StrPtrLen* inSubHeader);
	void 	ParseDynamicRateHeader();
	void	ParseRandomDataSizeHeader();
	void    ParseBandwidthHeader();

    static UInt8    sURLStopConditions[];
};
#endif // __RTSPREQUEST_H__

