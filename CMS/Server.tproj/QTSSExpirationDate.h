/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       QTSSExpirationDate.h

    Contains:   Routine that checks to see if software is expired.

    Written by: Denis Serenyi

    Copyright:  ?1998 by Apple Computer, Inc., all rights reserved.


*/

#ifndef __QTSS_EXPIRATION_DATE_H__
#define __QTSS_EXPIRATION_DATE_H__

#include "OSHeaders.h"

class QTSSExpirationDate
{
    public:
    
        //checks current time vs. hard coded time constant.
        static Bool16   WillSoftwareExpire(){return sIsExpirationEnabled;}
        static Bool16   IsSoftwareExpired();
        static void PrintExpirationDate();
        static void sPrintExpirationDate(char* ioExpireMessage);
        
    private:
    
        static Bool16 sIsExpirationEnabled;
        static char*    sExpirationDate;
        
};

#endif //__QTSS_EXPIRATION_DATE_H__
