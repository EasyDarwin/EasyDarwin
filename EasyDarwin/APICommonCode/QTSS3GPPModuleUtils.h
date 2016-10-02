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
	 File:       QTSS3GPPModuleUtils.h

	 Contains:   Utility routines for 3GPP modules to use.

 */


#ifndef _QTSS_3GPP_MODULE_UTILS_H_
#define _QTSS_3GPP_MODULE_UTILS_H_

#include "SafeStdLib.h"
#include "QTSS.h"
#include "StrPtrLen.h"
#include "SDPUtils.h"

class QTSS3GPPModuleUtils
{
public:


	static void     Initialize(QTSS_Initialize_Params* initParams);
	static void     ReadPrefs();


	static SDPContainer*    Get3GPPSDPFeatureListCopy(ResizeableStringFormatter &buffer);

private:

	static void ValidatePrefs();

	//
	// Used in the implementation of the above functions

	static QTSS_TextMessagesObject  sMessages;
	static QTSS_ServerObject        sServer;
	static QTSS_PrefsObject         sServerPrefs;
	static QTSS_StreamRef           sErrorLog;
	static StrPtrLen				s3gppBitRateAdaptationSDPStr;
	static bool                   s3gppEnabled;
	static bool                   s3gppRateAdaptationEnabled;
	static UInt16                   s3gppRateAdaptationReportFrequency;



};


#endif //_QTSS_3GPP_MODULE_UTILS_H_
