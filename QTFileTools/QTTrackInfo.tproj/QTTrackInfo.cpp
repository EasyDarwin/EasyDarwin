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
    

*/

#include <stdio.h>
#include <stdlib.h>
#include "SafeStdLib.h"
#include <string.h>
#include <time.h>

#ifndef __MacOSX__
#include "getopt.h"
#include <unistd.h>
#endif

#include "QTFile.h"

#include "QTTrack.h"
#include "QTHintTrack.h"

int main(int argc, char *argv[]) {
    // Temporary vars
    int         ch;

    // General vars
    const char      *MovieFilename;
    int             TrackNumber;

    QTTrack         *Track;
    QTHintTrack     *HintTrack;
    bool            Debug = false, DeepDebug = false;
    char            *Table = NULL;
    extern char* optarg;
    extern int optind;

    //
    // Read our command line options
    while( (ch = getopt(argc, argv, "dDT:")) != -1 ) {
        switch( ch ) {
            case 'd':
                Debug = true;
            break;

            case 'D':
                Debug = true;
                DeepDebug = true;
            break;
            
            case 'T':
                Table = strdup(optarg);
            break;
        }
    }

    argc -= optind;
    argv += optind;

    //
    // Validate our arguments.
    if( argc != 2 ) {
        qtss_printf("usage: QTTrackInfo [-d] [-D] [-T <table name>]<filename> <track number>\n");
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
    // Initialize the track.
    if( Track->Initialize() != QTTrack::errNoError ) {
        qtss_printf("Error!  Failed to initialize track %d in file \"%s\"!\n",
               TrackNumber, MovieFilename);
        exit(1);
    }
    
    //
    // Dump some information about this track.
    {
        time_t      unixCreationTime = (time_t)Track->GetCreationTime() + (time_t)QT_TIME_TO_LOCAL_TIME;
        time_t      unixModificationTime = (time_t)Track->GetModificationTime() + (time_t)QT_TIME_TO_LOCAL_TIME;
        char        buffer[kTimeStrSize];
        struct tm  timeResult;
        
        qtss_printf("-- Track #%02"_S32BITARG_" ---------------------------\n", Track->GetTrackID());
        qtss_printf("   Name               : %s\n", Track->GetTrackName());
        qtss_printf("   Created on         : %s", qtss_asctime(qtss_gmtime(&unixCreationTime, &timeResult),buffer, sizeof(buffer)));
        qtss_printf("   Modified on        : %s", qtss_asctime(qtss_gmtime(&unixModificationTime, &timeResult),buffer, sizeof(buffer)));

        //
        // Dump hint information is possible.
        if( file.IsHintTrack(Track) ) {
            HintTrack = (QTHintTrack *)Track;

            qtss_printf("   Total RTP bytes    : %"_64BITARG_"u\n", HintTrack->GetTotalRTPBytes());
            qtss_printf("   Total RTP packets  : %"_64BITARG_"u\n", HintTrack->GetTotalRTPPackets());
            qtss_printf("   Average bitrate    : %.2f Kbps\n", ((HintTrack->GetTotalRTPBytes() << 3) / file.GetDurationInSeconds()) / 1024);
            qtss_printf("   Average packet size: %"_64BITARG_"u\n", HintTrack->GetTotalRTPBytes() / HintTrack->GetTotalRTPPackets());

            UInt32 UDPIPHeaderSize = (UInt32) (56 * HintTrack->GetTotalRTPPackets());
            UInt32 RTPUDPIPHeaderSize = (UInt32) ((56+12) * HintTrack->GetTotalRTPPackets());
            qtss_printf("   Percentage of stream wasted on UDP/IP headers    : %.2f\n", (float)UDPIPHeaderSize / (float)(HintTrack->GetTotalRTPBytes() + UDPIPHeaderSize) * 100);
            qtss_printf("   Percentage of stream wasted on RTP/UDP/IP headers: %.2f\n", (float)RTPUDPIPHeaderSize / (float)(HintTrack->GetTotalRTPBytes() + RTPUDPIPHeaderSize) * 100);
        }

        qtss_printf("\n");
        qtss_printf("\n");
    }

    //
    // Dump all of the entries in the specified table (if we were given one).
    // Go through all of the samples in this track, printing out their offsets
    // and sizes.
    if( Table != NULL ) {
        if( (strcmp(Table, "stco") == 0) || (strcmp(Table, "co64") == 0) ) {
            Track->DumpChunkOffsetTable();
        } else if( strcmp(Table, "stsc") == 0 ) {
            Track->DumpSampleToChunkTable();
        } else if( strcmp(Table, "stsz") == 0 ) {
            Track->DumpSampleSizeTable();
        } else if( strcmp(Table, "stts") == 0 ) {
            Track->DumpTimeToSampleTable();
        } else if( strcmp(Table, "ctts") == 0 ) {
            Track->DumpCompTimeToSampleTable();
        }
    }

    return 0;
}
