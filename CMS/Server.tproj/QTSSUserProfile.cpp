/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
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

