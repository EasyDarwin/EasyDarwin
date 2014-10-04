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
    File:       QTSSUserProfile.cp

    Contains:   Implementation of class defined in QTSSUserProfile.h
    

    

*/


//INCLUDES:
#include "QTSSUserProfile.h"

QTSSAttrInfoDict::AttrInfo  QTSSUserProfile::sAttributes[] =
{   /*fields:   fAttrName, fFuncPtr, fAttrDataType, fAttrPermission */
    /* 0 */ { "qtssUserName",       NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe },
    /* 1 */ { "qtssUserPassword",   NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeWrite},
    /* 2 */ { "qtssUserGroups",     NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeWrite}, 
    /* 3 */ { "qtssUserRealm",      NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeWrite},
    /* 4 */ { "qtssUserRights",      NULL,  qtssAttrDataTypeUInt32,     qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeWrite},
    /* 5 */ { "qtssUserExtendedRights",      NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeWrite},
    /* 6 */ { "qtssUserQTSSExtendedRights",  NULL,   qtssAttrDataTypeCharArray,  qtssAttrModeRead | qtssAttrModePreempSafe | qtssAttrModeWrite}
};

void  QTSSUserProfile::Initialize()
{
    //Setup all the dictionary stuff
    for (UInt32 x = 0; x < qtssUserNumParams; x++)
        QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kQTSSUserProfileDictIndex)->
            SetAttribute(x, sAttributes[x].fAttrName, sAttributes[x].fFuncPtr,
                            sAttributes[x].fAttrDataType, sAttributes[x].fAttrPermission);  


}

//CONSTRUCTOR / DESTRUCTOR: very simple stuff
QTSSUserProfile::QTSSUserProfile()
:   QTSSDictionary(QTSSDictionaryMap::GetMap(QTSSDictionaryMap::kQTSSUserProfileDictIndex))
{
    this->SetEmptyVal(qtssUserName, &fUserNameBuf[0], kMaxUserProfileNameLen);
    this->SetEmptyVal(qtssUserPassword, &fUserPasswordBuf[0], kMaxUserProfilePasswordLen);
    this->SetVal(qtssUserRights, &fUserRights, sizeof(fUserRights));
    this->fUserRights = qtssAttrRightNone;
}

