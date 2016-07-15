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
	 File:       RTSPProtocol.h

	 Contains:   A grouping of static utilities that abstract keyword strings
				 in the RTSP protocol. This should be maintained as new versions
				 of the RTSP protoocl appear & as the server evolves to take
				 advantage of new RTSP features.



 */


#ifndef __RTSPPROTOCOL_H__
#define __RTSPPROTOCOL_H__

#include "QTSSRTSPProtocol.h"
#include "StrPtrLen.h"

class RTSPProtocol
{
public:

	//METHODS

	//  Method enumerated type definition in QTSS_RTSPProtocol.h

	//The lookup function. Very simple
	static UInt32   GetMethod(const StrPtrLen &inMethodStr);

	static StrPtrLen&   GetMethodString(QTSS_RTSPMethod inMethod)
	{
		return sMethods[inMethod];
	}

	//HEADERS

	//  Header enumerated type definitions in QTSS_RTSPProtocol.h

	//The lookup function. Very simple
	static UInt32 GetRequestHeader(const StrPtrLen& inHeaderStr);

	//The lookup function. Very simple.
	static StrPtrLen& GetHeaderString(UInt32 inHeader)
	{
		return sHeaders[inHeader];
	}


	//STATUS CODES

	//returns name of this error
	static StrPtrLen&       GetStatusCodeString(QTSS_RTSPStatusCode inStat)
	{
		return sStatusCodeStrings[inStat];
	}
	//returns error number for this error
	static SInt32           GetStatusCode(QTSS_RTSPStatusCode inStat)
	{
		return sStatusCodes[inStat];
	}
	//returns error number as a string
	static StrPtrLen&       GetStatusCodeAsString(QTSS_RTSPStatusCode inStat)
	{
		return sStatusCodeAsStrings[inStat];
	}

	// VERSIONS
	enum RTSPVersion
	{
		k10Version = 0,
		kIllegalVersion = 1
	};

	// NAMES OF THINGS
	static StrPtrLen&       GetRetransmitProtocolName() { return sRetrProtName; }

	//accepts strings that look like "RTSP/1.0" etc...
	static RTSPVersion      GetVersion(StrPtrLen &versionStr);
	static StrPtrLen&       GetVersionString(RTSPVersion version)
	{
		return sVersionString[version];
	}

private:

	//for other lookups
	static StrPtrLen            sMethods[];
	static StrPtrLen            sHeaders[];
	static StrPtrLen            sStatusCodeStrings[];
	static StrPtrLen            sStatusCodeAsStrings[];
	static SInt32               sStatusCodes[];
	static StrPtrLen            sVersionString[];

	static StrPtrLen            sRetrProtName;

};
#endif // __RTSPPROTOCOL_H__
