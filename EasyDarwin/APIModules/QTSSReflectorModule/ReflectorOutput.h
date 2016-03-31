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
    File:       ReflectorOutput.h

    Contains:   VERY simple abstract base class that defines one virtual method, WritePacket.
                This is extremely useful to the reflector, which, using one of these objects,
                can transparently reflect a packet, not being aware of how it will actually be
                written to the network
                    


*/

#ifndef __REFLECTOR_OUTPUT_H__
#define __REFLECTOR_OUTPUT_H__

#include "QTSS.h"
#include "StrPtrLen.h"
#include "OSHeaders.h"
#include "MyAssert.h"
#include "OS.h"
#include "OSQueue.h"


class ReflectorOutput
{
	public:
    
		ReflectorOutput() : fBookmarkedPacketsElemsArray(NULL), fNumBookmarks(0), fAvailPosition(0), fLastIntervalMilliSec(5), fLastPacketTransmitTime(0) {}   

        virtual ~ReflectorOutput() 
        {
            if ( fBookmarkedPacketsElemsArray )
            {   ::memset( fBookmarkedPacketsElemsArray, 0, sizeof ( OSQueueElem* ) * fNumBookmarks );

                delete [] fBookmarkedPacketsElemsArray;
            }
        }
        
        // an array of packet elements ( from fPacketQueue in ReflectorSender )
        // possibly one for each ReflectorSender that sends data to this ReflectorOutput        
        OSQueueElem         **fBookmarkedPacketsElemsArray;
        UInt32              fNumBookmarks;
        SInt32              fAvailPosition;
        QTSS_TimeVal        fLastIntervalMilliSec;
        QTSS_TimeVal        fLastPacketTransmitTime;
		OSMutex             fMutex;

	//add by fantasy		
	private:
		UInt64			fU64Seq;
		Bool16			fNewOutput;
	public:
		UInt64 outPutSeq()
		{
			return fU64Seq;
		}

		bool addSeq()
		{
			fU64Seq ++;
			if(fU64Seq == 0xffffffffffffffff)
			{
				fU64Seq = 0;
			}
			return true;
		}
		
		Bool16 getNewFlag()
		{
			return fNewOutput;
		}

		void setNewFlag(Bool16 flag)
		{
			fNewOutput = flag;
		}
//end add


inline  OSQueueElem*    GetBookMarkedPacket(OSQueue *thePacketQueue);
inline  Bool16          SetBookMarkPacket(OSQueueElem* thePacketElemPtr);
        
        // WritePacket
        //
        // Pass in the packet contents, the cookie of the stream to which it will be written,
        // and the QTSS API write flags (this should either be qtssWriteFlagsIsRTP or IsRTCP
        // packetLateness is how many MSec's late this packet is in being delivered ( will be < 0 if its early )
        // If this function returns QTSS_WouldBlock, timeToSendThisPacketAgain will
        // be set to # of msec in which the packet can be sent, or -1 if unknown
        virtual QTSS_Error  WritePacket(StrPtrLen* inPacket, void* inStreamCookie, UInt32 inFlags, SInt64 packetLatenessInMSec, SInt64* timeToSendThisPacketAgain, UInt64* packetIDPtr, SInt64* arrivalTimeMSec, Bool16 firstPacket ) = 0;
    
        virtual void        TearDown() = 0;
        virtual Bool16      IsUDP() = 0;
        virtual Bool16      IsPlaying() = 0;
        
        enum { kWaitMilliSec = 5, kMaxWaitMilliSec = 1000 };
        
   protected:
        void    InititializeBookmarks( UInt32 numStreams ) 
        {   
            // need 2 bookmarks for each stream ( include RTCPs )
            UInt32  numBookmarks = numStreams * 2;

            fBookmarkedPacketsElemsArray = new OSQueueElem*[numBookmarks]; 
            ::memset( fBookmarkedPacketsElemsArray, 0, sizeof ( OSQueueElem* ) * (numBookmarks) );
            
			//DT("fBookmarkedPacketsElemsArray[-1] %p= %p", &fBookmarkedPacketsElemsArray[-1], fBookmarkedPacketsElemsArray[-1]);
            fNumBookmarks = numBookmarks;
        }

};

Bool16  ReflectorOutput::SetBookMarkPacket(OSQueueElem* thePacketElemPtr)
{
	//OSMutexLocker locker(&fMutex);
    if (fAvailPosition != -1 && thePacketElemPtr)
    {    
        fBookmarkedPacketsElemsArray[fAvailPosition] = thePacketElemPtr; 
        
        for (UInt32 i = 0; i < fNumBookmarks; i++)
        {                   
            if (fBookmarkedPacketsElemsArray[i] == NULL)
            {   
                fAvailPosition = i;
                return true;
            }
        }
    }
    
    return false;

}

OSQueueElem*    ReflectorOutput::GetBookMarkedPacket(OSQueue *thePacketQueue)
{
	//OSMutexLocker locker(&fMutex);
    Assert(thePacketQueue != NULL);    
        
    OSQueueElem*        packetElem = NULL;              
    UInt32              curBookmark = 0;

    fAvailPosition = -1;       
    
    // see if we've bookmarked a held packet for this Sender in this Output
    while ( curBookmark < fNumBookmarks )
    {                   
        OSQueueElem*    bookmarkedElem = fBookmarkedPacketsElemsArray[curBookmark]; 
        
        if ( bookmarkedElem )   // there may be holes in this array
        {                           
            if ( bookmarkedElem->IsMember( *thePacketQueue ) ) 
            {   
                // this packet was previously bookmarked for this specific queue
                // remove if from the bookmark list and use it
                // to jump ahead into the Sender's over all packet queue                        
                fBookmarkedPacketsElemsArray[curBookmark] = NULL;  
                fAvailPosition = curBookmark;
                packetElem = bookmarkedElem;
                break;
            }                        
        }
        else
        {
            fAvailPosition = curBookmark;
        }
        
        curBookmark++;
            
    }
    
    return packetElem;
}                




#endif //__REFLECTOR_OUTPUT_H__
