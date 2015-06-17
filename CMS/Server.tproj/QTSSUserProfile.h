/*
	Copyright (c) 2013-2014 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       QTSSUserProfile.h

    Contains:   An object to store User Profile, for authentication
                and authorization
                
                Implements the RTSP Request dictionary for QTSS API.
    
    
*/


#ifndef __QTSSUSERPROFILE_H__
#define __QTSSUSERPROFILE_H__

//INCLUDES:
#include "QTSS.h"
#include "QTSSDictionary.h"
#include "StrPtrLen.h"

class QTSSUserProfile : public QTSSDictionary
{
    public:

        //Initialize
        //Call initialize before instantiating this class. For maximum performance, this class builds
        //any response header it can at startup time.
        static void         Initialize();
        
        //CONSTRUCTOR & DESTRUCTOR
        QTSSUserProfile();
        virtual ~QTSSUserProfile() {}
        
    protected:
        
        enum
        {
            kMaxUserProfileNameLen      = 32,
            kMaxUserProfilePasswordLen  = 32
        };
        
        char    fUserNameBuf[kMaxUserProfileNameLen];       // Set by RTSPRequest object
        char    fUserPasswordBuf[kMaxUserProfilePasswordLen];// Set by authentication module through API

        UInt32  fUserRights;  //Set by authorization module.
        //Dictionary support
        static QTSSAttrInfoDict::AttrInfo   sAttributes[];
};
#endif // __QTSSUSERPROFILE_H__

