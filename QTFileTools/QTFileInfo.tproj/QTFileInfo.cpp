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
    QTTrack         *Track;
    bool            Debug = false, DeepDebug = false;
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
    if( argc != 1 ) {
        qtss_printf("usage: QTFileInfo [-d] [-D] <filename>\n");
        exit(1);
    }


    //
    // Open the movie.
    QTFile file(Debug, DeepDebug);
    file.Open(*argv);
    if(Debug) file.DumpAtomTOC();

    //
    // Print out some information.
    qtss_printf("-- Movie %s \n", *argv);
    qtss_printf("   Duration        : %f\n", file.GetDurationInSeconds());
    
    Track = NULL;
    while( file.NextTrack(&Track, Track) ) {
        // Temporary vars
        QTHintTrack *HintTrack;
        time_t      unixCreationTime = (time_t)Track->GetCreationTime() + (time_t)QT_TIME_TO_LOCAL_TIME;
        time_t      unixModificationTime = (time_t)Track->GetModificationTime() + (time_t)QT_TIME_TO_LOCAL_TIME;
        char        buffer[kTimeStrSize];
        struct tm  timeResult;

        //
        // Initialize the track and dump it.
        if( Track->Initialize() != QTTrack::errNoError ) {
            qtss_printf("!!! Failed to initialize track !!!\n");
            continue;
        }
        if(DeepDebug) Track->DumpTrack();
        
        //
        // Dump some info.
        qtss_printf("-- Track #%02"_S32BITARG_" ---------------------------\n", Track->GetTrackID());
        qtss_printf("   Name               : %s\n", Track->GetTrackName());
        qtss_printf("   Created on         : %s", qtss_asctime(qtss_gmtime(&unixCreationTime, &timeResult), buffer, sizeof(buffer) ));
        qtss_printf("   Modified on        : %s", qtss_asctime(qtss_gmtime(&unixModificationTime, &timeResult), buffer, sizeof(buffer) ));

        //
        // Dump hint information is possible.
        if( file.IsHintTrack(Track) ) {
            HintTrack = (QTHintTrack *)Track;

            qtss_printf("   Total RTP bytes    : %"_64BITARG_"u\n", HintTrack->GetTotalRTPBytes());
            qtss_printf("   Total RTP packets  : %"_64BITARG_"u\n", HintTrack->GetTotalRTPPackets());
            qtss_printf("   Average bitrate    : %.2f Kbps\n", file.GetDurationInSeconds() == 0 ? 0.0 : ((HintTrack->GetTotalRTPBytes() << 3) / file.GetDurationInSeconds()) / 1024);
            qtss_printf("   Average packet size: %"_64BITARG_"u\n", HintTrack->GetTotalRTPPackets() == 0 ? 0 : HintTrack->GetTotalRTPBytes() / HintTrack->GetTotalRTPPackets());

            UInt32 UDPIPHeaderSize = (56 * HintTrack->GetTotalRTPPackets());
            UInt32 RTPUDPIPHeaderSize = ((56+12) * HintTrack->GetTotalRTPPackets());
            qtss_printf("   Percentage of stream wasted on UDP/IP headers    : %.2f\n", HintTrack->GetTotalRTPBytes() == 0 ? 0 : (float)UDPIPHeaderSize / (float)(HintTrack->GetTotalRTPBytes() + UDPIPHeaderSize) * 100);
            qtss_printf("   Percentage of stream wasted on RTP/UDP/IP headers: %.2f\n", HintTrack->GetTotalRTPBytes()  == 0 ? 0 : (float)RTPUDPIPHeaderSize / (float)(HintTrack->GetTotalRTPBytes() + RTPUDPIPHeaderSize) * 100);
        }
    }
    
    return 0;
}
