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
    File:       RTPSession.cpp

    Contains:   Implementation of RTPSession class. 
                    
    Change History (most recent first):

    
    
*/

#include "RTPSession3GPP.h"

#include "RTSPProtocol.h" 
#include "QTSServerInterface.h"
#include "QTSS.h"

#include "OS.h"
#include "OSMemory.h"

#include <errno.h>

#define RTPSESSION_DEBUGGING 0

QTSSAttrInfoDict::AttrInfo  RTPSession3GPP::sAttributes[] = 
{   /*fields:   fAttrName, fFuncPtr, fAttrDataType, fAttrPermission */
   /* 0 */ { "qtss3GPPCliSesEnabled",                           NULL,   qtssAttrDataTypeBool16,     qtssAttrModeRead | qtssAttrModePreempSafe },
   /* 1 */ { "qtss3GPPCliSesLinkCharGuaranteedBitRate",         NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe },
   /* 2 */ { "qtss3GPPCliSesLinkCharMaxBitRate",                NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe },
   /* 3 */ { "qtss3GPPCliSesLinkCharMaxTransferDelayMilliSec",  NULL,   qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe },
   /* 4 */ { "qtss3GPPCliSesLinkCharURL",                       NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe }

};


void    RTPSession3GPP::Initialize()
{
    for (int x = 0; x < qtss3GPPCliSesNumParams; x++)
        QTSSDictionaryMap::GetMap(QTSSDictionaryMap::k3GPPClientSessionDictIndex)->
            SetAttribute(x, sAttributes[x].fAttrName, sAttributes[x].fFuncPtr,
                sAttributes[x].fAttrDataType, sAttributes[x].fAttrPermission);
}


RTPSession3GPP::RTPSession3GPP(Bool16 enabled)
:   QTSSDictionary(QTSSDictionaryMap::GetMap(QTSSDictionaryMap::k3GPPClientSessionDictIndex), NULL),
    fEnabled (enabled)
{
    this->SetVal(qtss3GPPCliSesEnabled, &fEnabled, sizeof(fEnabled));

    this->SetVal(qtss3GPPCliSesLinkCharGuaranteedBitRate, &fLinkCharGuarntdKBitsPerSec, sizeof(fLinkCharGuarntdKBitsPerSec));
    this->SetVal(qtss3GPPCliSesLinkCharMaxBitRate, &fLinkCharMaxKBitsPerSec, sizeof(fLinkCharMaxKBitsPerSec));
    this->SetVal(qtss3GPPCliSesLinkCharMaxTransferDelayMilliSec, &fLinkCharMaxTransferDelayMilliSec, sizeof(fLinkCharMaxTransferDelayMilliSec));
    this->SetVal(qtss3GPPCliSesLinkCharURL, fURL, sizeof(fURL));       
    fURL[0] =0;
}


void   RTPSession3GPP::SetLinkCharData(LinkCharDataFields* linkCharDataPtr)
{
    if (NULL == linkCharDataPtr)
        return;
        
    this->SetGKbits(linkCharDataPtr->GetGKbits());
    this->SetMaxKbits(linkCharDataPtr->GetMaxKBits());
    this->SetMaxDelayMilliSecs(linkCharDataPtr->GetMaxDelayMilliSecs());
    this->SetURLCopy(linkCharDataPtr->GetURL());
  
}


RTPSession3GPP::~RTPSession3GPP()
{
}
