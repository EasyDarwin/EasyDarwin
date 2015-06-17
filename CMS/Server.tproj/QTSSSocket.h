/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       QTSSSocket.h

    Contains:   A QTSS Stream object for a generic socket
    
    Written By: Denis Serenyi
*/

#ifndef __QTSS_SOCKET_H__
#define __QTSS_SOCKET_H__

#include "QTSS.h"
#include "EventContext.h"
#include "QTSSStream.h"
#include "Socket.h"

class QTSSSocket : public QTSSStream
{
    public:

        QTSSSocket(int inFileDesc) : fEventContext(inFileDesc, Socket::GetEventThread()) {}
        virtual ~QTSSSocket() {}
        
        //
        // The only operation this stream supports is the requesting of events.
        virtual QTSS_Error  RequestEvent(QTSS_EventType inEventMask);
        
    private:
    
        EventContext fEventContext;
};

#endif //__QTSS_SOCKET_H__

