/*
	Copyright (c) 2013-2014 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       EasyHLSSession.cpp
    Contains:   Implementation of object defined in EasyHLSSession.h. 
*/
#include "EasyHLSSession.h"
#include "SocketUtils.h"
#include "EventContext.h"
#include "OSMemory.h"
#include "OS.h"
#include "atomic.h"
#include "QTSSModuleUtils.h"
#include <errno.h>

#ifndef __Win32__
    #include <unistd.h>
#endif

EasyHLSSession::EasyHLSSession(StrPtrLen* inSourceID)
:   fQueueElem()
{

    fQueueElem.SetEnclosingObject(this);
    if (inSourceID != NULL)
    {
        fSourceID.Ptr = NEW char[inSourceID->Len + 1];
        ::memcpy(fSourceID.Ptr, inSourceID->Ptr, inSourceID->Len);
        fSourceID.Len = inSourceID->Len;
        fRef.Set(fSourceID, this);
    }
}


EasyHLSSession::~EasyHLSSession()
{
    fSourceID.Delete();
}