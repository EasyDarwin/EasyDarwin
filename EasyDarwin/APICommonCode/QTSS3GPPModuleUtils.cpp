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
	 File:       QTSS3GPPModuleUtils.cpp

	 Contains:   Implements utility routines defined in QTSS3GPPModuleUtils.h.

 */

#include "QTSS3GPPModuleUtils.h"
#include "QTSSModuleUtils.h"

#include "StrPtrLen.h"
#include "OSMemory.h"
#include "MyAssert.h"
#include "StringFormatter.h"
#include "ResizeableStringFormatter.h"
#include "SafeStdLib.h"


QTSS_TextMessagesObject     QTSS3GPPModuleUtils::sMessages = NULL;
QTSS_ServerObject           QTSS3GPPModuleUtils::sServer = NULL;
QTSS_PrefsObject			QTSS3GPPModuleUtils::sServerPrefs = NULL;
QTSS_StreamRef              QTSS3GPPModuleUtils::sErrorLog = NULL;
StrPtrLen					QTSS3GPPModuleUtils::s3gppBitRateAdaptationSDPStr("a=3GPP-Adaptation-Support:");
const char*                 k3gppRateAdaptationReportFreqPrefName = "3gpp_protocol_rate_adaptation_report_frequency";

bool                      QTSS3GPPModuleUtils::s3gppEnabled = false;
bool                      QTSS3GPPModuleUtils::s3gppRateAdaptationEnabled = false;
UInt16                      QTSS3GPPModuleUtils::s3gppRateAdaptationReportFrequency = 1;

void    QTSS3GPPModuleUtils::Initialize(QTSS_Initialize_Params* initParams)
{
	if (NULL == initParams)
		return;

	sServer = initParams->inServer;
	sServerPrefs = initParams->inPrefs;
	sMessages = initParams->inMessages;
	sErrorLog = initParams->inErrorLogStream;

}

void    QTSS3GPPModuleUtils::ValidatePrefs()
{

	// min and max values per 3gpp rel-6 A26234  5.3.3.5 
	if (s3gppRateAdaptationReportFrequency < 1 || s3gppRateAdaptationReportFrequency > 9)
		QTSSModuleUtils::LogPrefErrorStr(qtssWarningVerbosity, (char*)k3gppRateAdaptationReportFreqPrefName, "has an invalid value: valid range is [1..9]");

	if (s3gppRateAdaptationReportFrequency < 1)
		s3gppRateAdaptationReportFrequency = 1;

	if (s3gppRateAdaptationReportFrequency > 9)
		s3gppRateAdaptationReportFrequency = 9;


}



void    QTSS3GPPModuleUtils::ReadPrefs()
{

	const bool kDefaultEnable = true;
	const UInt16 kDefaultReportFreq = 1;
	QTSSModuleUtils::GetAttribute(sServerPrefs, "enable_3gpp_protocol", qtssAttrDataTypeBool16,
		&s3gppEnabled, (void *)&kDefaultEnable, sizeof(s3gppEnabled));

	QTSSModuleUtils::GetAttribute(sServerPrefs, "enable_3gpp_protocol_rate_adaptation", qtssAttrDataTypeBool16,
		&s3gppRateAdaptationEnabled, (void *)&kDefaultEnable, sizeof(s3gppRateAdaptationEnabled));

	QTSSModuleUtils::GetAttribute(sServerPrefs, (char *)k3gppRateAdaptationReportFreqPrefName, qtssAttrDataTypeUInt16,
		&s3gppRateAdaptationReportFrequency, (void *)&kDefaultReportFreq, sizeof(s3gppRateAdaptationReportFrequency));


	QTSS3GPPModuleUtils::ValidatePrefs();
}

SDPContainer* QTSS3GPPModuleUtils::Get3GPPSDPFeatureListCopy(ResizeableStringFormatter &buffer)
{
	SDPContainer* resultList = NEW SDPContainer;
	StrPtrLen theLinePtr;


	if (s3gppEnabled)
	{
		if (s3gppRateAdaptationEnabled)
		{
			buffer.Put(s3gppBitRateAdaptationSDPStr);
			buffer.Put((SInt32)s3gppRateAdaptationReportFrequency);
			buffer.PutEOL();
			theLinePtr.Set(buffer.GetBufPtr(), buffer.GetBytesWritten());
			resultList->AddHeaderLine(&theLinePtr);
			buffer.Reset();
		}

	}


	return resultList;
}

