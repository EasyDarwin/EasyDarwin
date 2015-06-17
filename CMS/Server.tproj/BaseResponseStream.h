/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       RTSPResponseStream.h

    Contains:   Object that provides a "buffered WriteV" service. Clients
                can call this function to write to a socket, and buffer flow
                controlled data in different ways.
                
                It is derived from StringFormatter, which it uses as an output
                stream buffer. The buffer may grow infinitely.
*/

#ifndef __BASE_RESPONSE_STREAM_H__
#define __BASE_RESPONSE_STREAM_H__

#include "ResizeableStringFormatter.h"
#include "TCPSocket.h"
#include "TimeoutTask.h"
#include "QTSS.h"

class BaseResponseStream : public ResizeableStringFormatter
{
    public:
    
        // This object provides some flow control buffering services.
        // It also refreshes the timeout whenever there is a successful write
        // on the socket.
        BaseResponseStream(TCPSocket* inSocket, TimeoutTask* inTimeoutTask)
            :   ResizeableStringFormatter(fOutputBuf, kOutputBufferSizeInBytes),
                fSocket(inSocket), fBytesSentInBuffer(0), fTimeoutTask(inTimeoutTask),fPrintMSG(false) {}
        
        virtual ~BaseResponseStream() {}

        // WriteV
        //
        // This function takes an input ioVec and writes it to the socket. If any
        // data has been written to this stream via Put, that data gets written first.
        //
        // In the event of flow control on the socket, less data than what was
        // requested, or no data at all, may be sent. Specify what you want this
        // function to do with the unsent data via inSendType.
        //
        // kAlwaysBuffer:   Buffer any unsent data internally.
        // kAllOrNothing:   If no data could be sent, return EWOULDBLOCK. Otherwise,
        //                  buffer any unsent data.
        // kDontBuffer:     Never buffer any data.
        //
        // If some data ends up being buffered, outLengthSent will = inTotalLength,
        // and the return value will be QTSS_NoErr 
        
        enum
        {
            kDontBuffer     = 0,
            kAllOrNothing   = 1,
            kAlwaysBuffer   = 2
        };
        QTSS_Error WriteV(iovec* inVec, UInt32 inNumVectors, UInt32 inTotalLength,
                                UInt32* outLengthSent, UInt32 inSendType);

        // Flushes any buffered data to the socket. If all data could be sent,
        // this returns QTSS_NoErr, otherwise, it returns EWOULDBLOCK
        QTSS_Error Flush();
        
        void        ShowMSG(Bool16 enable) {fPrintMSG = enable; }     

        
    private:
    
        enum
        {
            kOutputBufferSizeInBytes = 512  //UInt32
        };
        
        //The default buffer size is allocated inline as part of the object. Because this size
        //is good enough for 99.9% of all requests, we avoid the dynamic memory allocation in most
        //cases. But if the response is too big for this buffer, the BufferIsFull function will
        //allocate a larger buffer.
        char                    fOutputBuf[kOutputBufferSizeInBytes];
        TCPSocket*              fSocket;
        UInt32                  fBytesSentInBuffer;
        TimeoutTask*            fTimeoutTask;
        Bool16                  fPrintMSG;     // debugging printfs
        
        friend class RTSPRequestInterface;
};


#endif // __BASE_RESPONSE_STREAM_H__
