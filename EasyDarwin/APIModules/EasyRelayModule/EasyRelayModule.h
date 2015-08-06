/*
	Copyright (c) 2013-2014 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       QTSSOnDemandRelayModule.h
    Contains:   QTSS API Module On Demand Relay Module
*/

#ifndef _EASYRELAYMODULE_H_
#define _EASYRELAYMODULE_H_

#include "QTSS.h"

extern "C"
{
    EXPORT QTSS_Error EasyRelayModule_Main(void* inPrivateArgs);
}
#endif //_EASYRELAYMODULE_H_
