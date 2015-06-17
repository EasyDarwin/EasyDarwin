/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       QTSSStream.h

    Contains:   Abstract base class containing the prototypes for generalized
                stream functions.
                
                Any server object that wants to act as a QTSS_StreamRef should
                derive off of this and implement one or more of the stream APIs.

*/


#ifndef __QTSS_STREAM_H__
#define __QTSS_STREAM_H__

#include "QTSS.h"
#include "Task.h"

class QTSSStream
{
    public:

        QTSSStream() : fTask(NULL) {}
        virtual ~QTSSStream() {}
        
        //
        // A stream can have a task associated with it. If this stream supports
        // async I/O, the task is needed to know what to wakeup when there is an event
        void    SetTask(Task* inTask)   { fTask = inTask; }
        Task*   GetTask()               { return fTask; }

        virtual QTSS_Error  Read(void* /*ioBuffer*/, UInt32 /*inLen*/, UInt32* /*outLen*/)
                                                            { return QTSS_Unimplemented; }
                                                            
        virtual QTSS_Error  Write(void* /*inBuffer*/, UInt32 /*inLen*/, UInt32* /*outLenWritten*/, UInt32 /*inFlags*/)
                                                            { return QTSS_Unimplemented; }
        
        virtual QTSS_Error  WriteV(iovec* /*inVec*/, UInt32 /*inNumVectors*/, UInt32 /*inTotalLength*/, UInt32* /*outLenWritten*/)
                                                            { return QTSS_Unimplemented; }
                                                            
        virtual QTSS_Error  Flush()                         { return QTSS_Unimplemented; }
        
        virtual QTSS_Error  Seek(UInt64 /*inNewPosition*/)  { return QTSS_Unimplemented; }
        
        virtual QTSS_Error  Advise(UInt64 /*inPosition*/, UInt32 /*inAdviseSize*/)
                                                            { return QTSS_Unimplemented; }
                                                            
        virtual QTSS_Error  RequestEvent(QTSS_EventType /*inEventMask*/)
                                                            { return QTSS_Unimplemented; }
    
    private:
    
        Task* fTask;
};

#endif //__QTSS_STREAM_H__
