/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
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
#include "NVSourceAPI.h"
#include "EasyHLSAPI.h"

#ifndef __EASYHLS_SESSION__
#define __EASYHLS_SESSION__

class EasyHLSSession
{
    public:
        EasyHLSSession(StrPtrLen* inSourceID);
        virtual ~EasyHLSSession();
        
        //º”‘ÿƒ£øÈ≈‰÷√
        static void Initialize(QTSS_ModulePrefsObject inPrefs);

        // ACCESSORS
        
        OSRef*          GetRef()            { return &fRef; }
        OSQueueElem*    GetQueueElem()      { return &fQueueElem; }
	
        StrPtrLen*      GetSessionID()     { return &fHLSSessionID; }
		QTSS_Error		ProcessData(int _chid, int mediatype, char *pbuf, NVS_FRAME_INFO *frameinfo);
		QTSS_Error		HLSSessionCreate(char* rtspUrl);
		QTSS_Error		HLSSessionRelease();
   
    private:

        // For storage in the session map       
        OSRef       fRef;
        StrPtrLen   fHLSSessionID;
        OSQueueElem fQueueElem; // Relay uses this.  
		NVS_HANDLE	fNVSHandle;
		Easy_HLS_Handle fHLSHandle;

		static UInt32	sM3U8Version;
		static Bool16	sAllowCache;
		static UInt32	sTargetDuration;
		static UInt32	sPlaylistCapacity;
		static char*	sLocalRootDir;
		static char*	sHTTPRootDir;
};

#endif

