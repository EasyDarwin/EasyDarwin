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
    File:       PlayerSimulator.h

    Simulates a client; track duplicate/late/missing packets
*/

#ifndef _PLAYERSIMULATOR_H_
#define _PLAYERSIMULATOR_H_

#include "SafeStdLib.h""
#include "OSHeaders.h"
#include "OS.h"
#include "SVector.h"

/* Macros for min/max. */
#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif

class PlayerSimulator
{
	public:
		PlayerSimulator(UInt32 verboseLevel = 0)
			: fTargetBufferingDelay(0), fStartPlayDelay(0), fLocalStartPlayTime(0), fCurrentMediaTime(0), fVerboseLevel(verboseLevel), fIsPlaying(false)
		{}
	
		//call Setup and then SetupTrack per stream to initialize the class.  Use a targetBufferingDelay of 0 to start playing immediately
		//The PlayerSimulator can be reused by calling Setup and SetupTrack again
		void Setup(UInt32 numTracks, UInt32 targetBufferingDelay = 0, UInt32 startPlayDelay = 0)
		{
			fTargetBufferingDelay = targetBufferingDelay;
			fStartPlayDelay = startPlayDelay;
			fLocalStartPlayTime = 0;
			fIsPlaying = false;
			fTrackInfo.clear();
			fTrackInfo.resize(numTracks);
		}
		//Use a bufferSize of 0 to have an unlimited buffer size;
		void SetupTrack(UInt32 trackIndex, UInt32 samplingRate, UInt32 bufferSize = 0)
		{
			fTrackInfo[trackIndex].fBufferSize = bufferSize == 0 ? kUInt32_Max : bufferSize;
			fTrackInfo[trackIndex].fSamplingRate = samplingRate;
		}
	
		//Please note that PlayerSimulator use track index, not track ID; payloadSize should exclude the RTP header.
		//Returns true if the packet is a duplicate; the sequence number of the RTP packet MUST be in range.
		Bool16	ProcessRTPPacket(UInt32 trackIndex, UInt32 packetLen, UInt32 payloadSize, UInt32 seqNum, UInt32 timeStamp)
		{
			Assert(trackIndex <= fTrackInfo.size());
			TrackInfo &trackInfo = fTrackInfo[trackIndex];
			SVector<PacketInfo> &playBuffer = trackInfo.fPlayBuffer;

            Bool16 addPacketToBuffer = false;
			Bool16 packetIsDuplicate = false;

            if (fIsPlaying)
            {
				AdvanceTime();
                if(seqNum < trackInfo.fNextSeqNum)
                {
                    //definately a late packet; try to see if the packet is one of the lost packets
			        UInt32 index = trackInfo.fLostPackets.find(seqNum);
					if (index != trackInfo.fLostPackets.size())
					{
						//a late, non-duplicate packet
						trackInfo.fNumLatePackets++;
						trackInfo.fLostPackets.erase(index);
					}
					else //a late-duplicate
					{
						trackInfo.fNumDuplicates++;
						packetIsDuplicate = true;
					}
                }
				else
				{
					//look in the play buffer
					if (playBuffer.empty() || seqNum < playBuffer.front().fSeqNum)
					{
						//the packet has an earlier sequence number than any other packets in the buffer; it may still be late
						if (RTPTime2MediaTime(trackIndex, timeStamp) <= fCurrentMediaTime)
							trackInfo.fNumLatePackets++;
						if (GetFreeBufferSpace(trackIndex) >= packetLen)
							playBuffer.insert(0, PacketInfo(seqNum, timeStamp, packetLen));
						else
							trackInfo.fNumBufferOverflowedPackets++;
					}
					else
						addPacketToBuffer = true;
				}
            }
            else
            {
			    if (playBuffer.empty())
				{
					//This is the first packet in the stream
					if (GetFreeBufferSpace(trackIndex) >= packetLen)
						playBuffer.push_back(PacketInfo(seqNum, timeStamp, packetLen));
					else
						trackInfo.fNumBufferOverflowedPackets++;
				}
                else
                    addPacketToBuffer = true;
            }

			if (addPacketToBuffer)
			{
				//check the buffer to see if the packet is a duplicate
				UInt32 index = 0;
				for(index = 0; index < playBuffer.size(); ++index)
				{
					if (seqNum == playBuffer[index].fSeqNum)
					{
						//a non-late, duplicate packet
						trackInfo.fNumDuplicates++;
						packetIsDuplicate = true;
						break;
					}
					else if (seqNum < playBuffer[index].fSeqNum) //found where to insert
						break;
				}

				if(!packetIsDuplicate)
				{
					//a non-late non-duplicate -- may be out of order
					if (GetFreeBufferSpace(trackIndex) >= packetLen)
						playBuffer.insert(index, PacketInfo(seqNum, timeStamp, packetLen));
					else
						trackInfo.fNumBufferOverflowedPackets++;
				}
			}

			if(!fIsPlaying)
			{
				//is it time to start playing?
				Bool16 haveEnoughBuffer = true;
				for(UInt32 index = 0; index < fTrackInfo.size(); ++index)
				{
					if (fTrackInfo[index].fPlayBuffer.empty() || GetBufferingDelay(index) <= fStartPlayDelay)
					{
						haveEnoughBuffer = false;
						break;
					}
				}
				
				if(haveEnoughBuffer) //lets start playing!
				{
					fIsPlaying = true;
					fLocalStartPlayTime = OS::Milliseconds();

					for(UInt32 index = 0; index < fTrackInfo.size(); ++index)
					{
						//for each track, set the next sequence number and the earliest RTP timestamp
						TrackInfo &curTrack = fTrackInfo[index];
						curTrack.fRTPTimeStampBase = curTrack.fPlayBuffer.front().fTimeStamp;
						curTrack.fNextSeqNum = curTrack.fPlayBuffer.front().fSeqNum;
						
						if(fVerboseLevel >= 1)
							qtss_printf("Track %"_U32BITARG_" is now playing; initial seq=%"_U32BITARG_", initial time stamp=%"_U32BITARG_"\n",
								index, curTrack.fNextSeqNum, curTrack.fRTPTimeStampBase);
					}
					AdvanceTime();
				}
			}
			
			if(fVerboseLevel >= 2)
			{
				if (fIsPlaying)
					qtss_printf("Processed packet: track=%"_U32BITARG_", len=%"_U32BITARG_", seq=%"_U32BITARG_", time=%"_U32BITARG_"(%"_U32BITARG_"); "
						"bufferingDelay=%"_U32BITARG_", FBS=%"_U32BITARG_", media time=%"_U32BITARG_"\n",
						trackIndex, packetLen, seqNum, timeStamp, RTPTime2MediaTime(trackIndex, timeStamp), 
						GetBufferingDelay(trackIndex), GetFreeBufferSpace(trackIndex), fCurrentMediaTime);
				else
					qtss_printf("Processed packet: track=%"_U32BITARG_", len=%"_U32BITARG_", seq=%"_U32BITARG_", time=%"_U32BITARG_"(%"_U32BITARG_"); "
						"bufferingDelay=%"_U32BITARG_", FBS=%"_U32BITARG_"\n",
						trackIndex, packetLen, seqNum, timeStamp, RTPTime2MediaTime(trackIndex, timeStamp),
						GetBufferingDelay(trackIndex), GetFreeBufferSpace(trackIndex));
			}

			return packetIsDuplicate;
		}
		
		//returns kUInt32_Max if the buffer is empty; otherwise returns the first sequence number in the buffer
		UInt32 GetNextSeqNumToDecode(UInt32 trackIndex)
		{
			SVector<PacketInfo> &playBuffer = fTrackInfo[trackIndex].fPlayBuffer;
			return playBuffer.empty() ? kUInt32_Max : playBuffer.front().fSeqNum;
		}
		
		//in milliseconds; returns kUInt32_Max if the stream is not playing or if the buffer is empty
		UInt32	GetPlayoutDelay(UInt32 trackIndex)
		{
			SVector<PacketInfo> &playBuffer = fTrackInfo[trackIndex].fPlayBuffer;
			if(!fIsPlaying || playBuffer.empty())
				return kUInt32_Max;
			else
			{
				UInt32 nextPacketTime = playBuffer.front().fTimeStamp;
				return RTPTime2MediaTime(trackIndex, nextPacketTime) - fCurrentMediaTime;
			}
		}
		 
		//in milliseconds; returns kUInt32_Max if the buffer is empty
		UInt32	GetBufferingDelay(UInt32 trackIndex)
		{
			SVector<PacketInfo> &playBuffer = fTrackInfo[trackIndex].fPlayBuffer;
			if(playBuffer.empty())
				return kUInt32_Max;
				
			UInt64 delta = playBuffer.back().fTimeStamp - playBuffer.front().fTimeStamp;
			UInt32 bufferDelay = static_cast<UInt32>((delta * 1000) / fTrackInfo[trackIndex].fSamplingRate);
			if (fIsPlaying)
				bufferDelay += GetPlayoutDelay(trackIndex);
			return bufferDelay;
		}
		
		//in bytes
		UInt32	GetFreeBufferSpace(UInt32 trackIndex)
		{
			SVector<PacketInfo> &playBuffer = fTrackInfo[trackIndex].fPlayBuffer;
			UInt32 bytesInBuffer = 0;
			for(UInt32 i = 0; i < playBuffer.size(); ++i)
				bytesInBuffer += playBuffer[i].fPayloadSize;
			Assert(fTrackInfo[trackIndex].fBufferSize >= bytesInBuffer);
			return fTrackInfo[trackIndex].fBufferSize - bytesInBuffer;
		}
		
		//The lost packets returned by this function is NOT the same as the lost packets defined by the RTCP RR.
		UInt32	GetNumPacketsLost(UInt32 trackIndex)						{ return fTrackInfo[trackIndex].fLostPackets.size(); }
		UInt32	GetNumLatePackets(UInt32 trackIndex)						{ return fTrackInfo[trackIndex].fNumLatePackets; }
		UInt32	GetNumBufferOverflowedPackets(UInt32 trackIndex)			{ return fTrackInfo[trackIndex].fNumBufferOverflowedPackets; }
		UInt32	GetNumDuplicates(UInt32 trackIndex)							{ return fTrackInfo[trackIndex].fNumDuplicates; }
		
	private:
		struct PacketInfo
		{
			UInt32			fSeqNum;					//kUInt32_Max is invalid sequence number
			UInt32			fTimeStamp;
			UInt32			fPayloadSize;
			
			PacketInfo() : fSeqNum(0), fTimeStamp(0), fPayloadSize(0)		{}
			PacketInfo(UInt32 seqNum, UInt32 timeStamp, UInt32 payloadSize)
			: fSeqNum(seqNum), fTimeStamp(timeStamp), fPayloadSize(payloadSize)
			{}
		};
		struct TrackInfo
		{
			UInt32				fNumDuplicates;
			UInt32				fNumBufferOverflowedPackets;	//number of packets lost due to buffer overflow
			UInt32				fNumLatePackets;			//packet that did not arrive on time, but did arrive

			UInt32				fBufferSize;				//in bytes
			UInt32				fSamplingRate;				//in samples per second

			//These two values have meaning only while the stream is playing.
			//All packets with sequence number less than fNextSeqNum have already expired, and all other packets PROBABLY goes into the buffer
			UInt32				fRTPTimeStampBase;
            UInt32              fNextSeqNum;

			SVector<UInt32>		fLostPackets;				//All the packets with seq number less than fNextSeqNum that has not yet arrived.
			SVector<PacketInfo>	fPlayBuffer;                //Can contain only packets with seq number >= than fNextSeqNum

			TrackInfo() : fNumDuplicates(0), fNumBufferOverflowedPackets(0), fNumLatePackets(0), fBufferSize(0), fSamplingRate(0),
				fRTPTimeStampBase(0), fNextSeqNum(0)
			{}
		};
		
		//Advances the curent media play time according to the wallclock time; remove packets in the buffer whose timestamp has expired,
		//increases fNextSeqNum, and find out which packet is missing and put them on the lost packts list.
		//Also updates the current media play time -- all packets with an earlier timestamp has expired, and all packets with a later timestamp
		//must be in the buffer or is not received yet.
		void AdvanceTime()
		{
			fCurrentMediaTime = LocalTime2MediaTime(OS::Milliseconds());
			for(UInt32 trackIndex = 0; trackIndex < fTrackInfo.size(); ++trackIndex)
			{
				TrackInfo &trackInfo = fTrackInfo[trackIndex];
				SVector<PacketInfo> &playBuffer = trackInfo.fPlayBuffer;
				UInt32 prevTimeStamp = 0;

				//go through the play buffer until a packet with a later timestamp than the current time is found
				UInt32 i = 0;
				for(; i < playBuffer.size(); ++i)
				{
					if(prevTimeStamp > playBuffer[i].fTimeStamp)
					{   if(fVerboseLevel >= 2) //this can happen from out of order packets from udp network routing or retransmits from rudp. It should never happen over tcp or http.
						    qtss_printf("WARNING: RTP timestamp is not monotonic! seq=%"_U32BITARG_" timestamp=%"_U32BITARG_"\n", playBuffer[i].fSeqNum, playBuffer[i].fTimeStamp);
				    }
					else
						prevTimeStamp = playBuffer[i].fTimeStamp;

					if (fCurrentMediaTime >= RTPTime2MediaTime(trackIndex, playBuffer[i].fTimeStamp))
					{
						//the packet has expired; add packets that has not yet arrived to the lost packet list
						while(trackInfo.fNextSeqNum !=  playBuffer[i].fSeqNum)
						{
							trackInfo.fLostPackets.push_back(trackInfo.fNextSeqNum);
							trackInfo.fNextSeqNum++;
						}
						trackInfo.fNextSeqNum++;
					}
					else
						break;
				}
				//remove packets that have expired
				playBuffer.erase(0, i);
			}
		}
		
		//can be called only when the media is playing; media time is measured in milliseconds
		UInt32 LocalTime2MediaTime(SInt64 localTime)
		{
			SInt64 delta = localTime - fLocalStartPlayTime;
			Assert(fIsPlaying &&  (SInt64) 0 <= (SInt64) delta && delta <= (SInt64)kUInt32_Max);
			return delta;
		}
		
		UInt32	RTPTime2MediaTime(UInt32 trackIndex, UInt32 RTPTimestamp)
		{
			TrackInfo &trackInfo = fTrackInfo[trackIndex];
			UInt32 RTPTimestampBase = RTPTimestamp;
			if(fIsPlaying)
				RTPTimestampBase = trackInfo.fRTPTimeStampBase;
			else if (!trackInfo.fPlayBuffer.empty())
				RTPTimestampBase = trackInfo.fPlayBuffer.front().fTimeStamp;
			UInt64 delta = RTPTimestamp - RTPTimestampBase;
			return static_cast<UInt32>((delta*1000) / trackInfo.fSamplingRate);
		}
		
		SVector<TrackInfo>	fTrackInfo;
		UInt32				fTargetBufferingDelay;			//in milliseconds
		UInt32				fStartPlayDelay;				//in milliseconds
		SInt64				fLocalStartPlayTime;			//in UNIX time(milliseconds);
		UInt32				fCurrentMediaTime;				//current media time(in millisecond); valid only while playing
		UInt32				fVerboseLevel;
		Bool16				fIsPlaying;
};

#endif //_PLAYERSIMULATOR_H_
