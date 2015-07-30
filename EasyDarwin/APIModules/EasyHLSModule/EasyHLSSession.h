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
    
        // Public interface to generic RTP packet forwarding engine
        
        //
        // Initialize
        //
        // Call initialize before calling any other function in this class
        static void Initialize();
            
        // Create one of these ReflectorSessions per source broadcast. For mapping purposes,
        // the object can be constructred using an optional source ID.
        //
        // Caller may also provide a SourceInfo object, though it is not needed and
        // will also need to be provided to SetupReflectorSession when that is called.
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

