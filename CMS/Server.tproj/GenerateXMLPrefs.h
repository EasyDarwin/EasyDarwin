/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       GenerateXMLPrefs.h

    Contains:   Routine that updates a QTSS 1.x 2.x PrefsSource to the new XMLPrefsSource.

*/

#include "PrefsSource.h"
#include "FilePrefsSource.h"
#include "XMLPrefsParser.h"

int GenerateStandardXMLPrefs(PrefsSource* inPrefsSource, XMLPrefsParser* inXMLPrefs);
int GenerateAllXMLPrefs(FilePrefsSource* inPrefsSource, XMLPrefsParser* inXMLPrefs);
