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

#ifndef __MacOSX__
#include "getopt.h"
#include <unistd.h>
#endif

#include "QTFile.h"
#include "QTTrack.h"

int main(int argc, char *argv[]) {
    // Temporary vars
    int             ch;

    // General vars
    const char      *MovieFilename;
    int             TrackNumber;

    QTTrack         *Track;
    bool            Debug = false, DeepDebug = false,
                    DumpHTML = false;
    
    UInt64          Offset;
    UInt32          Size, MediaTime;
    extern int optind;

    //
    // Read our command line options
    while( (ch = getopt(argc, argv, "dDH")) != -1 ) {
        switch( ch ) {
            case 'd':
                Debug = true;
            break;

            case 'D':
                Debug = true;
                DeepDebug = true;
            break;
            
            case 'H':
                DumpHTML = true;
            break;
        }
    }

    argc -= optind;
    argv += optind;

    //
    // Validate our arguments.
    if( argc != 2 ) {
        qtss_printf("usage: QTSampleLister [-d] [-D] [-H] <filename> <track number>\n");
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
    if( Track->Initialize() != QTTrack::errNoError){
        qtss_printf("Error!  Failed to initialize track %d in file \"%s\"!\n",
               TrackNumber, MovieFilename);
        exit(1);
    }
    
    //
    // Dump some information about this track.
    if( !DumpHTML ) {
        time_t      unixCreationTime = (time_t)Track->GetCreationTime() + (time_t)QT_TIME_TO_LOCAL_TIME;
        time_t      unixModificationTime = (time_t)Track->GetModificationTime() + (time_t)QT_TIME_TO_LOCAL_TIME;
        char		buffer[kTimeStrSize];
        struct tm  timeResult;
        
        qtss_printf("-- Track #%02"_S32BITARG_" ---------------------------\n", Track->GetTrackID());
        qtss_printf("   Name               : %s\n", Track->GetTrackName());
        qtss_printf("   Created on         : %s", qtss_asctime(qtss_gmtime(&unixCreationTime, &timeResult),buffer, sizeof(buffer)));
        qtss_printf("   Modified on        : %s", qtss_asctime(qtss_gmtime(&unixModificationTime, &timeResult),buffer, sizeof(buffer)));
        qtss_printf("\n");
        qtss_printf("\n");
    }

    //
    // Go through all of the samples in this track, printing out their offsets
    // and sizes.
    if( !DumpHTML ) {
        qtss_printf("Sample #     Media Time  DataOffset  SampleSize\n");
        qtss_printf("--------     ----------  ----------  ----------\n");
    }
    
    UInt32 curSample = 1;

        QTAtom_stsc_SampleTableControlBlock stsc;
        QTAtom_stts_SampleTableControlBlock stts;
        while( Track->GetSampleInfo(curSample, &Size, &Offset, NULL, &stsc) ) {
        //
        // Get some additional information about this sample.
            Track->GetSampleMediaTime(curSample, &MediaTime, &stts);

        //
        // Dump out the sample.
        if( DumpHTML ) 
            qtss_printf("<FONT COLOR=black>%010"_64BITARG_"u: track=%02"_U32BITARG_"; size=%"_U32BITARG_"</FONT><BR>\n",Offset, Track->GetTrackID(), Size);
        else 
            qtss_printf("%8"_U32BITARG_"  -  %10"_U32BITARG_"  %10"_64BITARG_"u  %10"_U32BITARG_"\n", curSample, MediaTime, Offset, Size);

    
        // Next sample.
        curSample++;
    }

    return 0;
}
