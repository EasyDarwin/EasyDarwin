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
#ifndef playlist_elements_H
#define playlist_elements_H
 
#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include <string.h>
#include <errno.h>
#ifndef __Win32__
    #include <sys/types.h>
    #include <sys/time.h>
    #include <sys/stat.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <fcntl.h>
#endif

#include "playlist_array.h"
#include "OSHeaders.h"
#include "playlist_SimpleParse.h"
#include "QTRTPFile.h"
#include "BroadcasterSession.h"

class MediaStream;

struct SoundDescription {
    SInt32                           descSize;                   /* total size of SoundDescription including extra data */
    SInt32                           dataFormat;                 /* sound format */
    SInt32                           resvd1;                     /* reserved for apple use. set to zero */
    SInt16                           resvd2;                     /* reserved for apple use. set to zero */
    SInt16                           dataRefIndex;
    SInt16                           version;                    /* which version is this data */
    SInt16                           revlevel;                   /* what version of that codec did this */
    SInt32                           vendor;                     /* whose  codec compressed this data */
    SInt16                           numChannels;                /* number of channels of sound */
    SInt16                           sampleSize;                 /* number of bits per sample */
    SInt16                           compressionID;              /* unused. set to zero. */
    SInt16                           packetSize;                 /* unused. set to zero. */
    UInt32                   		 sampleRate;                 /* sample rate sound is captured at */
};


class PayLoad {
    public:
    PayLoad(void) : payloadID(0), timeScale(1) {};
    int             payloadID;
    SimpleString    payLoadString;
    UInt32          timeScale;
};


class TypeMap {
    public:
        TypeMap(void) : fMediaStreamPtr(0), fTrackID(0), fPort(0), fTimeScale(1) {};
        ~TypeMap() { } ;
        
        MediaStream                 *fMediaStreamPtr;
        int                         fTrackID;
        int                         fPort;
        UInt32                      fTimeScale;
        SimpleString                fTheTypeStr;
        SimpleString                fProtocolStr;
        ArrayList<PayLoad>          fPayLoadTypes;
        ArrayList<SInt16>            fPayLoads;
    
};


class RTpPacket 
{
        

struct SoundHeader {
    char bytes[4];
    SInt32 skip1;
    char sndtype[4];
    SInt32 skip2;
    char test[4];
};


    public:
        enum    { kRTpHeaderSize = 12 };
        char    *fThePacket;
        int     fLength;
        bool    fHasSoundDescription;
        SInt32    fSoundDescriptionLen;

        RTpPacket()  : fThePacket(NULL), fLength(0), fHasSoundDescription(false), fSoundDescriptionLen(0) {};
         
        void    SetPacketData(char *thePacket, int length)  {fThePacket = thePacket; fLength = length; };
        SInt16  SetHeaderInfo( UInt32 timeStamp, UInt16 sequence,UInt32 SSRC,unsigned char payloadType);
        SInt16  GetHeaderInfo( UInt32 *timeStampPtr, UInt16 *sequencePtr,UInt32 *SSRCPtr,unsigned char*payloadType);
        
        bool    HasSoundDescription();
        SInt16  GetSoundDescriptionRef(SoundDescription **soundDescriptionPtr);
        
    protected: 
    
        enum { cSequenceNumber = 1, cTimeStamp = 1, cSSRC = 2, cPayloadType = 1};
        
};



class UDPSocketPair
{
    public:
    enum
    {
        kBound      = 1L << 0,
        kConnected  = 1L << 1
    };

    enum    {   eBindMaxTries = 100,
                eSourcePortStart = 49152, // rtp + rtcp
                eSourcePortRange =  65535 // 49152,49153 - 65534,65535
            };

    enum
    {
        kInvalidSocket = -1,    //int
        kPortRTCpufSizeInBytes = 8, //UInt32
        kMaxIPAddrSizeInBytes = 20  //UInt32
    };

    int                 fMaxBindAttempts;
    int                 fState;
    int                 fSocketRTp;
    struct sockaddr_in  fLocalAddrRTp;
    struct sockaddr_in  fDestAddrRTp;
    
    int                 fSocketRTCp;
    struct sockaddr_in  fLocalAddrRTCp;
    struct sockaddr_in  fDestAddrRTCp;
    BroadcasterSession *fBroadcasterSession;
    UInt8               fChannel;
    Bool16              fIsMultiCast;
    Bool16              fMultiCastJoined;
    
    UDPSocketPair() :   fMaxBindAttempts(eBindMaxTries), 
                        fState(0),  
                        fSocketRTp(0), 
                        fSocketRTCp(0),
                        fBroadcasterSession(NULL),
                        fChannel(0),
                        fIsMultiCast(false),
                        fMultiCastJoined(false)
                        {};  
    ~UDPSocketPair() { Close(); };
    
    SInt16  Open();
    void    Close();
    void    InitPorts(UInt32 addr);
    SInt16  Bind(UInt32 addr);
    SInt16  OpenAndBind( UInt16 rtpPort,UInt16 rtcpPort,char *destAddress);

    SInt16  SetDestination (char *destAddress,UInt16 destPortRTp, UInt16 destPortRTCp);
    SInt16  SetTTL(SInt16 timeToLive);
    SInt16  JoinMulticast();
    SInt16  LeaveMulticast();
    SInt16  SetMulticastInterface();
    SInt16  SetMultiCastOptions(SInt16 ttl);

    SInt16  SendTo(int socket, sockaddr *destAddrPtr, char* inBuffer, UInt32 inLength );
    SInt16  SendRTp(char* inBuffer, UInt32 inLength);
    SInt16  SendRTCp(char* inBuffer, UInt32 inLength);
    
    SInt16  RecvFrom(sockaddr *recvAddrPtr, int socket, char* ioBuffer, UInt32 inBufLen, UInt32* outRecvLen);
    SInt16  RecvRTp(char* ioBuffer, UInt32 inBufLen, UInt32* outRecvLen);
    SInt16  RecvRTCp(char* ioBuffer, UInt32 inBufLen, UInt32* outRecvLen);

    void    SetRTSPSession(BroadcasterSession *theSession,UInt8 channel) {fBroadcasterSession = theSession, fChannel=channel;}
};

class ReceiveBuffer
{
    public:
            enum { kReadBufferSize = 256 }; //UInt32
            char            fReadBuffer[kReadBufferSize];
            UInt32          fReceiveLen;    

};

class MediaStream
{         
    protected:

        int             SendRTp(RTpPacket *packet);
        int             CalcRTCps();
        int             SendRTCp_SenderReport();
        static UInt32   GetACName(char* ioCNameBuffer);
        
        void MapToStream(UInt32 curRTpTimeStamp, UInt16 curRTpSequenceNumber, unsigned char curPayload, UInt32 *outRTpTimeStampPtr, UInt16 *outRTpSequenceNumberPtr, unsigned char *outPayloadPtr);
        void            UpdatePacketInStream(RTpPacket *packetPtr);
        SInt16          Accounting(RTpPacket *packetPtr);
        void            BuildStaticRTCpReport();
        void            InitIfAudio();
        void            TestAndIncSoundDescriptor(RTpPacket *packetPtr);
    public:
        enum { kMaxCNameLen = 20 }; //UInt32
        enum { eMaxSoundDescriptionSize = 1024};
        enum {
                kSenderReportSizeInBytes = 36,  //UInt32
                kMaxRTCpPacketSizeInBytes = 1024,   //All are UInt32s
                kMaxSsrcSizeInBytes = 25,
                kSenderReportIntervalInSecs = 5
             };
             

        enum {  
                eSocketNotOpen,
                eSocketFailed 
              };
    
        struct MemberData {
            ReceiveBuffer   fPortRTpReadBuff;
            ReceiveBuffer   fPortRTCpReadBuff;
            
            UInt64          fRTCpTimer;
            SInt16          fState;
            
            
            UInt64          fBytesSent;
            UInt64          fPacketsSent;
            
            SInt64          fStreamStartTime;
            SInt64          fNTPPlayTime;
            
            char            fSenderReportBuffer[kSenderReportSizeInBytes + kMaxCNameLen];
            UInt32          fSenderReportSize;
            
            //who am i sending to?
            UInt16          fRemoteRTpPort;
            UInt16          fRemoteRTCpPort;
            
            //RTCp stuff 
            SInt64          fLastSenderReportTime;
            UInt32          fPacketCount;
            UInt32          fByteCount;
            
            Bool16          fSenderReportReady;
            UInt32          fLastTimeStamp;

            // current RTP packet info
            UInt32          fLastTimeStampOffset;

            UInt32          fTimeStamp;
            UInt32          fSsrc;
            
            UInt16          fLastSequenceNumber;
                    
            UInt32          fInitSSRC; // initial SSRC
            UInt64          fCurStreamRTpSequenceNumber; // now
            
            TypeMap         *fStreamMediaTypePtr;
            TypeMap         *fMovieMediaTypePtr;
            SInt64          fMovieStartTime;
            SInt64          fMovieEndTime;
            
            Float64         fLastMovieDurationSecs;
            UInt64          fMediaStartOffsetMediaScale;
            UInt64          fMovieStartOffset;
            
            UInt32          fSeqRandomOffset;
            UInt32          fRTpRandomOffset;
            
            bool            fNewMovieStarted;   
            bool            fNewStreamStarted;
            
            bool            fIsSoundStream;
            bool            fIsVideoStream;
            
            char            *fSoundDescriptionBuffer;
            SInt32            fSavedSoundDescSize;
            SInt16           fSavedDataRefIndex;
            UDPSocketPair   *fSocketPair;
            QTRTPFile       *fRTPFilePtr;
        };
          
        MemberData      fData;
        bool            fSend;
                ~MediaStream();
                MediaStream();
        char*   GetRTCpSR()             { return fData.fSenderReportBuffer; }
        UInt32  GetRTCpSRLen()          { return fData.fSenderReportSize;   }
        SInt64  GetPlayTime()           { return fData.fStreamStartTime; }
        SInt64  GetNTPPlayTime()        { return fData.fNTPPlayTime; }
        SInt16  Send(RTpPacket *packetPtr);
        void    ReceiveOnPorts();
        int     UpdateSenderReport(SInt64 theTime);
        void    StreamStart(SInt64 startTime);
        void    MovieStart(SInt64 startTime);
        void    MovieEnd(SInt64 endTime);
};






#endif // playlist_elements_H
