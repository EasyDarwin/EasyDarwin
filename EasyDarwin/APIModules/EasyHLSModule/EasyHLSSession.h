/*
	Copyright (c) 2013-2014 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       EasyHLSSession.h
    Contains:   HLS
*/
#include "QTSS.h"
#include "OSRef.h"
#include "StrPtrLen.h"
#include "ResizeableStringFormatter.h"
#include "MyAssert.h"

#include "ReflectorStream.h"
#include "SourceInfo.h"
#include "OSArrayObjectDeleter.h"

#ifndef __EASYHLS_SESSION__
#define __EASYHLS_SESSION__

class EasyHLSSession
{
    public:
        EasyHLSSession(StrPtrLen* inSourceID);
        virtual ~EasyHLSSession();
        
        //
        // ACCESSORS
        
        OSRef*          GetRef()            { return &fRef; }
        OSQueueElem*    GetQueueElem()      { return &fQueueElem; }

        StrPtrLen*      GetSourcePath()     { return &fSourceID; }
   
    private:

        // For storage in the session map       
        OSRef       fRef;
        StrPtrLen   fSourceID;
        OSQueueElem fQueueElem; // Relay uses this.         
};

#endif

