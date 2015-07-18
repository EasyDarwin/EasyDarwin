/*
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 1999-2008 Apple Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 *
 */
/*
    File:       win32ev.cpp

    Contains:   WSA implementation of socket event queue functions.
    
    Written By: Denis Serenyi

    
*/

#include "ev.h"
#include "OSHeaders.h"
#include "OSThread.h"
#include "MyAssert.h"
#include <string>
//
// You have to create a window to get socket events? What's up with that?
static HWND sMsgWindow = NULL;

//
LRESULT CALLBACK select_wndproc(HWND inWIndow, UINT inMsg, WPARAM inParam, LPARAM inOtherParam);

void select_startevents()
{
    //
    // This call occurs from the main thread. In Win32, apparently, you
    // have to create your WSA window from the same thread that calls GetMessage.
    // So, we have to create the window from select_waitevent
}

int select_removeevent(int /*which*/)
{
    //
    // Not needed for WSA.
    return 0;
}

int select_watchevent(struct eventreq *req, int which)
{
    return select_modwatch(req, which);
}

int select_modwatch(struct eventreq *req, int which)
{
    //
    // If our WSAAsyncSelect window is not constructed yet, wait
    // until it is construected. The window gets constructed when the server
    // is done starting up, so this should only happen when select_modwatch
    // is being called as the server is starting up.
    while (sMsgWindow == NULL)
        OSThread::Sleep(10);
        
    // Convert EV_RE and EV_WR to the proper WSA event codes.
    // WSA event codes are more specific than what POSIX provides, so
    // just wait on any kind of read related event for EV_RE, same for EV_WR
    SInt32 theEvent = 0;
    
    if (which & EV_RE)
        theEvent |= FD_READ | FD_ACCEPT | FD_CLOSE;
    if (which & EV_WR)
        theEvent |= FD_WRITE | FD_CONNECT;
    
    // This is a little bit of a hack, because we are assuming that the caller
    // is actually putting a UInt32 in the void*, not a void*, and we are also
    // assuming caller is not using the 0 - WM_USER range of values, but
    // both of these things are true in the EventContext.cpp code, and this
    // mechanism of passing around cookies is just too convienent to ignore.
    unsigned int theMsg = (unsigned int)(req->er_data);
    
    return ::WSAAsyncSelect(req->er_handle, sMsgWindow, theMsg, theEvent);
}

int select_waitevent(struct eventreq *req, void* /*onlyForMacOSX*/)
{
    if (sMsgWindow == NULL)
    {
        //
        // This is the first time we've called this function. Do our
        // window initialization now.
        
        // We basically just want the simplest window possible.
        WNDCLASSEX theWndClass;
        theWndClass.cbSize = sizeof(theWndClass);
        theWndClass.style = 0;
        theWndClass.lpfnWndProc = &select_wndproc;
        theWndClass.cbClsExtra = 0;
        theWndClass.cbWndExtra = 0;
        theWndClass.hInstance = NULL;
        theWndClass.hIcon = NULL;
        theWndClass.hCursor = NULL;
        theWndClass.hbrBackground = NULL;
        theWndClass.lpszMenuName = NULL;
#ifdef LIB_EASY_CMS
		const char* sName =  "libEasyCMSServerWindow";
#else
       const char* sName =  "EasyDarwinServerWindow";
#endif
        theWndClass.lpszClassName = sName;
		theWndClass.hIconSm = NULL;
        
        ATOM theWndAtom = ::RegisterClassEx(&theWndClass);
        Assert(theWndAtom != NULL);
        if (theWndAtom == NULL)
            ::exit(-1); // Poor error recovery, but this should never happen.
                
        sMsgWindow = ::CreateWindow(    sName,  // Window class name
                                        sName,  // Window title bar
                                        WS_POPUP,   // Window style ( a popup doesn't need a parent )
                                        0,          // x pos
                                        0,          // y pos
                                        CW_USEDEFAULT,  // default width
                                        CW_USEDEFAULT,  // default height
                                        NULL,           // No parent
                                        NULL,           // No menu handle
                                        NULL,           // Ignored on WinNT
                                        NULL);          // data for message proc. Who cares?
        Assert(sMsgWindow != NULL);
        if (sMsgWindow == NULL)
            ::exit(-1);
    }
    
    MSG theMessage;
    
    //
    // Get a message for my goofy window. 0, 0 indicates that we
    // want any message for that window.
    //
    // Convienently, this function blocks until there is a message, so it works
    // much like waitevent would on Mac OS X.
    UInt32 theErr = ::GetMessage(&theMessage, sMsgWindow, 0, 0);
    
    if (theErr > 0)
    {
        UInt32 theSelectErr = WSAGETSELECTERROR(theMessage.lParam);
        UInt32 theEvent = WSAGETSELECTEVENT(theMessage.lParam);
        
        req->er_handle = theMessage.wParam; // the wParam is the FD
        req->er_eventbits = EV_RE;          // WSA events & socket events don't map...
                                            // but the server state machines never care
                                            // what the event is anyway.

        // we use the message # as our way of passing around the user data.
        req->er_data = (void*)(theMessage.message);
        
        //
        // We should prevent this socket from getting events until modwatch is called.
        (void)::WSAAsyncSelect(req->er_handle, sMsgWindow, 0, 0); 
        
        return 0;
    }
    else
    {
        //
        // Do we ever get WM_QUIT messages? Can there ever be an error?
        Assert(0);
        return EINTR;
    }
}


LRESULT CALLBACK select_wndproc(HWND /*inWIndow*/, UINT inMsg, WPARAM /*inParam*/, LPARAM /*inOtherParam*/)
{
    // If we don't return true for this message, window creation will not proceed
    if (inMsg == WM_NCCREATE)
        return TRUE;
    
    // All other messages we can ignore and return 0
    return 0;
}
