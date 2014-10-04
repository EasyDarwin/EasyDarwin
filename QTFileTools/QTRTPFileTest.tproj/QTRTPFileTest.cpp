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
#include <fcntl.h>

#ifndef __MacOSX__
#include "getopt.h"
#include <unistd.h>
#endif

#include "OS.h"
#include "QTRTPFile.h"
    
int main(int argc, char *argv[]) {
    // Temporary vars
    int         ch = '\0';

    // General vars
    int             fd = -1;
    
    const char      *MovieFilename;
    QTRTPFile       *RTPFile = NULL;
    bool            Debug = false, DeepDebug = false;
    bool            silent = false;
    bool            trackCache= false;
    bool            everytrack= false;
    bool            hintOnly = false;
	bool			keyFramesOnly = false;
    extern int optind;
    QTRTPFile::RTPTrackListEntry *trackListEntry = NULL;

    //
    // Read our command line options
	while( (ch = getopt(argc, argv, "dDhsetk")) != -1 ) {
        switch( ch ) {
            case 'e':
                everytrack = true;
            break;

            case 's':
                silent = true;
            break;

            case 't':
                trackCache = true;
            break;
            
            case 'h':
                hintOnly = true;
            break;

            case 'd':
                Debug = true;
            break;

            case 'D':
                Debug = true;
                DeepDebug = true;
            break;
			case 'k':
				keyFramesOnly = true;
			break;
        }
    }

    argc -= optind;
    argv += optind;

    //
    // Validate our arguments.
    if( argc < 1 ) {
        qtss_printf("usage: QTRTPFileTest <filename> <track#n> <track#n+1> ..\n");
        qtss_printf("usage: -s no packet printfs\n");
        qtss_printf("usage: -e test every hint track\n");
        qtss_printf("usage: -k list only packets belonging to key frames. Specify a single video track with this option\n");
        qtss_printf("usage: -t write packets to track.cache file\n");
        qtss_printf("usage: -h show hinted (.unopt, .opt)\n");
        exit(1);
    }
    
    MovieFilename = *argv++;
    argc--;

    if (!hintOnly)
        qtss_printf("****************** QTRTPFileTest ******************\n");

    //
    // Open the movie.
    RTPFile = new QTRTPFile(Debug, DeepDebug);
    switch( RTPFile->Initialize(MovieFilename) ) {
        case QTRTPFile::errNoError:
        case QTRTPFile::errNoHintTracks:
        break;

        case QTRTPFile::errFileNotFound:
            qtss_printf("Error!  File not found \"%s\"!\n", MovieFilename);
            exit(1);
        case QTRTPFile::errInvalidQuickTimeFile:
            qtss_printf("Error!  Invalid movie file \"%s\"!\n", MovieFilename);
            exit(1);
        case QTRTPFile::errInternalError:
            qtss_printf("Error!  Internal error opening movie file \"%s\"!\n", MovieFilename);
            exit(1);
        case QTRTPFile::errTrackIDNotFound:
        case QTRTPFile::errCallAgain:
            //noops
            break;
    }
    
    
    //
    // Get the SDP file and print it out.
    char        *SDPFile;
    int         SDPFileLength;

    {
        //
        // Get the file
        SDPFile = RTPFile->GetSDPFile(&SDPFileLength);
        if( SDPFile == NULL ) {
            qtss_printf("Error!  Could not get SDP file!\n");
            exit(1);
        }
        if (!hintOnly)
        {
            write(1, SDPFile, SDPFileLength);
            write(1, "\n", 1);
        }
    }

    //
    // Open our file to write the packets out to.
    if (trackCache)
    {
        fd = open("track.cache", O_CREAT | O_TRUNC | O_WRONLY, 0664);
        if( fd == -1 ) {
            qtss_printf("Error!  Could not create output file!\n");
            exit(1);
        }
    }
    if (everytrack || hintOnly)
    {
        int trackcount = 0;
        int hinttracks[20];
        memset(&hinttracks,0,sizeof(hinttracks));
        bool found = false;
        char *trackPtr = SDPFile;
        while (true) 
        {   
            trackPtr = ::strstr(trackPtr,"trackID=");
            if (trackPtr != NULL)
            {   trackPtr+= ::strlen("trackID=");
                sscanf(trackPtr, "%d",&hinttracks[trackcount]);
                trackcount++;
                found = true;
            }
            else 
                break;
            
        }
        while (trackcount)
        {
            switch( RTPFile->AddTrack(hinttracks[trackcount -1]) ) {
                case QTRTPFile::errNoError:
                case QTRTPFile::errNoHintTracks:
                break;

                case QTRTPFile::errFileNotFound:
                case QTRTPFile::errInvalidQuickTimeFile:
                case QTRTPFile::errInternalError:
                    qtss_printf("Error!  Invalid movie file \"%s\"!\n", MovieFilename);
                    exit(1);
                    
                case QTRTPFile::errTrackIDNotFound:
                case QTRTPFile::errCallAgain:
                    //noops
                break;
            }

            RTPFile->SetTrackCookies(hinttracks[trackcount], (char *)hinttracks[trackcount], 0);
            (void)RTPFile->GetSeekTimestamp(hinttracks[trackcount]);
            trackcount --;
        }
    }
    else
    {
        //
        // Add the tracks that we're interested in.
        while(argc--) {
            switch( RTPFile->AddTrack(atoi(*argv)) ) {
                case QTRTPFile::errNoError:
                case QTRTPFile::errNoHintTracks:
                case QTRTPFile::errTrackIDNotFound:
                case QTRTPFile::errCallAgain:
                break;

                case QTRTPFile::errFileNotFound:
                case QTRTPFile::errInvalidQuickTimeFile:
                case QTRTPFile::errInternalError:
                    qtss_printf("Error!  Invalid movie file \"%s\"!\n", MovieFilename);
                    exit(1);
            }

            RTPFile->FindTrackEntry(atoi(*argv), &trackListEntry);
            RTPFile->SetTrackCookies(atoi(*argv), (char *)atoi(*argv), 0);
            (void)RTPFile->GetSeekTimestamp(atoi(*argv));
            argv++;
        }
    }
    
    
    //
    // Display some stats about the movie.
    
    if (!hintOnly)
        qtss_printf("Total RTP bytes of all added tracks: %"_64BITARG_"u\n", RTPFile->GetAddedTracksRTPBytes());

    //
    // Seek to the beginning of the movie.
    if( RTPFile->Seek(0.0) != QTRTPFile::errNoError ) {
        qtss_printf("Error!  Couldn't seek to time 0.0!\n");
        exit(1);
    }
    
    //
    // Suck down packets..
    UInt32      NumberOfPackets = 0;
    Float64     TotalInterpacketDelay = 0.0,
                LastPacketTime = 0.0;
                
    SInt64 startTime = 0;
    SInt64 durationTime = 0;
    SInt64 packetCount = 0;

    while(1) 
    {
        // Temporary vars
        UInt16  tempInt16;

        // General vars
        char    *Packet;
        int     PacketLength;
        //SInt32  Cookie;
        UInt32  RTPTimestamp;
        UInt16  RTPSequenceNumber;
        int     maxHintPackets = 100; // cheat assume this many packets is good enough to assume entire file is the same at these packets
    

        //
        // Get the next packet.
        startTime = OS::Milliseconds();
		if (keyFramesOnly)
			RTPFile->SetTrackQualityLevel(trackListEntry, QTRTPFile::kKeyFramesOnly);

        Float64 TransmitTime = RTPFile->GetNextPacket(&Packet, &PacketLength);
        SInt64 thisDuration = OS::Milliseconds() - startTime;
        durationTime += thisDuration;
        packetCount++;

        if( Packet == NULL )
            break;
        
        if (hintOnly)
        {   if (--maxHintPackets == 0 )   
                break;
            continue;
        }
            
        memcpy(&RTPSequenceNumber, Packet + 2, 2);
        RTPSequenceNumber = ntohs(RTPSequenceNumber);
        memcpy(&RTPTimestamp, Packet + 4, 4);
        RTPTimestamp = ntohl(RTPTimestamp);
        
        if (!hintOnly)
            if (!silent)
                qtss_printf("TransmitTime = %.2f; SEQ = %u; TS = %"_U32BITARG_"\n", TransmitTime, RTPSequenceNumber, RTPTimestamp);
        
        if (trackCache)
        {
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
        // Compute the Inter-packet delay and keep track of it.
        if( TransmitTime >= LastPacketTime ) {
            TotalInterpacketDelay += TransmitTime - LastPacketTime;
            LastPacketTime = TransmitTime;
            NumberOfPackets++;
        }
        
    }


    //
    // Compute and display the Inter-packet delay.
    if( (!hintOnly) && NumberOfPackets > 0 )
    {
        qtss_printf("QTRTPFileTest: Total GetNextPacket durationTime = %"_U32BITARG_"ms packetCount= %"_U32BITARG_"\n",(UInt32)durationTime,(UInt32)packetCount);      
        qtss_printf("QTRTPFileTest: Average Inter-packet delay: %"_64BITARG_"uus\n", (UInt64)((TotalInterpacketDelay / NumberOfPackets) * 1000 * 1000));
    }   
    
    SInt32 hintType = RTPFile->GetMovieHintType(); // this can only be reliably called after playing all the packets. 
    if (!hintOnly)
    {
        if (hintType < 0) 
            qtss_printf("QTRTPFileTest: HintType=Optimized\n");
        else if (hintType > 0) 
            qtss_printf("QTRTPFileTest: HintType=Unoptimized\n");
        else 
            qtss_printf("QTRTPFileTest: HintType=Unknown\n");
    }
    else
    {
        if (hintType < 0) 
            qtss_printf("%s.opt\n",MovieFilename);
        else if (hintType > 0) 
            qtss_printf("%s.unopt\n",MovieFilename);
        else 
            qtss_printf("%s\n",MovieFilename);
    }       
    
    if (trackCache)
    {   //
        // Close the output file.
        close(fd);
    }
    
    //
    // Close our RTP file.
    delete RTPFile;

    return 0;
}
