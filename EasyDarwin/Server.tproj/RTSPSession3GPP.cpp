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
	 File:       RTSPSession3GPP.cpp

	 Contains:   Implementation of RTSPSession3GPP class.



 */


#include "RTSPSession3GPP.h"

QTSSAttrInfoDict::AttrInfo  RTSPSession3GPP::sAttributes[] =
{   /*fields:   fAttrName, fFuncPtr, fAttrDataType, fAttrPermission */
	/* 0 */ { "qtss3GPPRTSPSesEnabled",         NULL,                   qtssAttrDataTypeBool16,  qtssAttrModeRead | qtssAttrModePreempSafe }

};

void  RTSPSession3GPP::Initialize()
{
	for (UInt32 x = 0; x < qtss3GPPRTSPSessNumParams; x++)
		QTSSDictionaryMap::GetMap(QTSSDictionaryMap::k3GPPRTSPSessionDictIndex)->
		SetAttribute(x, sAttributes[x].fAttrName, sAttributes[x].fFuncPtr,
			sAttributes[x].fAttrDataType, sAttributes[x].fAttrPermission);

}


RTSPSession3GPP::RTSPSession3GPP(Bool16 enabled)
	: QTSSDictionary(QTSSDictionaryMap::GetMap(QTSSDictionaryMap::k3GPPRTSPSessionDictIndex)),
	fEnabled(enabled)
{
	this->SetVal(qtss3GPPRTSPSesEnabled, &fEnabled, sizeof(fEnabled));


}

