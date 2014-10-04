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

#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include <string.h>
#include <time.h>
#include <fcntl.h>

#ifndef __MacOSX__
#include "getopt.h"
#include <unistd.h>
#endif

#include "QTFile.h"
#include "QTTrack.h"
#include "QTHintTrack.h"


int main(int argc, char *argv[]) {
    // Temporary vars
    int             ch;
    UInt16          tempInt16;
    UInt32          tempInt32;
    Float32         tempFloat32;
    Float64         tempFloat64;

    // General vars
    int             fd;
    
    const char      *MovieFilename;
    int             TrackNumber;

    QTTrack         *Track;
    QTHintTrack     *HintTrack;
    bool            Debug = false, DeepDebug = false;
    
    UInt16          NumPackets;
    extern int optind;

    //
    // Read our command line options
    while( (ch = getopt(argc, argv, "dD")) != -1 ) {
        switch( ch ) {
            case 'd':
                Debug = true;
            break;

            case 'D':
                Debug = true;
                DeepDebug = true;
            break;
        }
    }

    argc -= optind;
    argv += optind;

    //
    // Validate our arguments.
    if( argc != 2 ) {
        qtss_printf("usage: QTRTPGen [-d] [-D] <filename> <track number>\n");
        exit(1);
    }
    
    MovieFilename = *argv++;
    TrackNumber = atoi(*argv++);


    //
    // Open the movie.
    QTFile file(Debug, DeepDebug);
    file.Open(MovieFilename);

    //
    // Find the specified track and dump out information about its' samples.
    if( !file.FindTrack(TrackNumber, &Track) ) {
        qtss_printf("Error!  Could not find track number %d in file \"%s\"!",
               TrackNumber, MovieFilename);
        exit(1);
    }
    
    //
    // Make sure that this is a hint track.
    if( !file.IsHintTrack(Track) ) {
        qtss_printf("Error!  Track number %d is not a hint track!\n", TrackNumber);
        exit(1);
    }
    HintTrack = (QTHintTrack *)Track;
    
    //
    // Initialize this track.
    HintTrack->Initialize();

    //
    // Dump some information about this track.
    {
        time_t      unixCreationTime = (time_t)HintTrack->GetCreationTime() + (time_t)QT_TIME_TO_LOCAL_TIME;
        time_t      unixModificationTime = (time_t)HintTrack->GetModificationTime() + (time_t)QT_TIME_TO_LOCAL_TIME;
        char        buffer[kTimeStrSize];
        struct tm  timeResult;
        
        qtss_printf("-- Track #%02"_S32BITARG_" ---------------------------\n", HintTrack->GetTrackID());
        qtss_printf("   Name               : %s\n", HintTrack->GetTrackName());
        qtss_printf("   Created on         : %s", qtss_asctime(qtss_gmtime(&unixCreationTime, &timeResult),buffer, sizeof(buffer)));
        qtss_printf("   Modified on        : %s", qtss_asctime(qtss_gmtime(&unixModificationTime, &timeResult),buffer, sizeof(buffer)));

        qtss_printf("   Total RTP bytes    : %"_64BITARG_"u\n", HintTrack->GetTotalRTPBytes());
        qtss_printf("   Total RTP packets  : %"_64BITARG_"u\n", HintTrack->GetTotalRTPPackets());
        qtss_printf("   Average bitrate    : %.2f Kbps\n",(float) ((HintTrack->GetTotalRTPBytes() << 3) / file.GetDurationInSeconds()) / 1024);
        qtss_printf("   Average packet size: %"_64BITARG_"u\n", HintTrack->GetTotalRTPBytes() / HintTrack->GetTotalRTPPackets());

        UInt32 UDPIPHeaderSize = (56 * HintTrack->GetTotalRTPPackets());
        UInt32 RTPUDPIPHeaderSize = ((56+12) * HintTrack->GetTotalRTPPackets());
        qtss_printf("   Percentage of stream wasted on UDP/IP headers    : %.2f\n", (float)UDPIPHeaderSize / (float)(HintTrack->GetTotalRTPBytes() + UDPIPHeaderSize) * 100);
        qtss_printf("   Percentage of stream wasted on RTP/UDP/IP headers: %.2f\n", (float)RTPUDPIPHeaderSize / (float)(HintTrack->GetTotalRTPBytes() + RTPUDPIPHeaderSize) * 100);
        qtss_printf("\n");
        qtss_printf("\n");
    }

    //
    // Open our file to write the packets out to.
    fd = open("track.cache", O_CREAT | O_TRUNC | O_WRONLY, 0666);
    if( fd == -1 ) {
        qtss_printf("Error!  Could not create output file!\n");
        exit(1);
    }
    
    //
    // Write out the header.
    tempInt16 = 1;
    write(fd, (char *)&tempInt16, 2);   // isCompletelyWritten
    tempInt16 = 0;
    write(fd, (char *)&tempInt16, 2);   // padding1
    tempFloat64 = file.GetDurationInSeconds();
    write(fd, (char *)&tempFloat64, 8); // movieLength
    tempInt32 = HintTrack->GetRTPTimescale();
    write(fd, (char *)&tempInt32, 4);   // rtpTimescale
    tempInt16 = HintTrack->GetRTPSequenceNumberRandomOffset();
    write(fd, (char *)&tempInt16, 2);   // seqNumRandomOffset
    tempInt16 = 0;
    write(fd, (char *)&tempInt16, 2);   // padding2
    tempFloat32 = HintTrack->GetTotalRTPBytes() / file.GetDurationInSeconds();
    write(fd, (char *)&tempFloat32, 4); // dataRate
    
    //
    // Go through all of the samples in this track, printing out their offsets
    // and sizes.
    qtss_printf("Sample #     NPkts\n");
    qtss_printf("--------     -----\n");
    UInt32 curSample = 1;
    QTHintTrack_HintTrackControlBlock HCB;
    while( HintTrack->GetNumPackets(curSample, &NumPackets) == QTTrack::errNoError ) {
        //
        // Generate all of the packets.
        qtss_printf("Generating %u packet(s) in sample #%"_U32BITARG_"..\n", NumPackets, curSample);
        for( UInt16 curPacket = 1; curPacket <= NumPackets; curPacket++ ) {
            // General vars
            #define MAX_PACKET_LEN 2048
            char        Packet[MAX_PACKET_LEN];
            UInt32      PacketLength;
            Float64     TransmitTime;
        
        
            //
            // Generate this packet.
            PacketLength = MAX_PACKET_LEN;
            HintTrack->GetPacket(curSample, curPacket,
                                 Packet, &PacketLength, 
                                 &TransmitTime, false, false, 0,&HCB);
            
            //
            // Write out the packet header.
            write(fd, (char *)&TransmitTime, 8);    // transmitTime
            tempInt16 = PacketLength;
            write(fd, (char *)&tempInt16, 2);       // packetLength
            tempInt16 = 0;
            write(fd, (char *)&tempInt16, 2);       // padding1
            
            //
            // Write out the packet.
            write(fd, Packet, PacketLength);
        }

        //
        // Next sample.
        curSample++;
    }
    
    //
    // Close the output file.
    close(fd);

    return 0;
}
