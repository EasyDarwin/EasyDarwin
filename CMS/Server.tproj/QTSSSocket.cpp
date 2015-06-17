/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.EasyDarwin.org
*/
/*
    File:       QTSSSocket.cpp
    Contains:   A QTSS Stream object for a generic socket
*/

#include "QTSSSocket.h"

QTSS_Error  QTSSSocket::RequestEvent(QTSS_EventType inEventMask)
{
    int theMask = 0;
    
    if (inEventMask & QTSS_ReadableEvent)
        theMask |= EV_RE;
    if (inEventMask & QTSS_WriteableEvent)
        theMask |= EV_WR;
        
    fEventContext.SetTask(this->GetTask());
    fEventContext.RequestEvent(theMask);
    return QTSS_NoErr;
}
