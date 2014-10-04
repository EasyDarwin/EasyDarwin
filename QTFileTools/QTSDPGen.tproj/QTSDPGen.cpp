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
#include <fcntl.h>

#ifndef __MacOSX__
#include "getopt.h"
#include <unistd.h>
#endif


#include "QTRTPFile.h"
    
int main(int argc, char *argv[]) {
    // Temporary vars
    int             ch;

    // General vars
    const char      *MovieFilename;
    const char      *OutputFilename = NULL;
    QTRTPFile       *RTPFile;
    bool            Debug = false, DeepDebug = false;
    extern char* optarg;
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

            case 'f':
                OutputFilename = optarg;
            break;
        }
    }

    argc -= optind;
    argv += optind;

    //
    // Validate our arguments.
    if( argc < 1 ) {
        qtss_printf("usage: QTSDPGen <filename[s]>\n");
        exit(1);
    }
    
    //MovieFilename = *argv++;

    while ((MovieFilename = *argv++) != NULL)
    {

        //
        // Open the movie.
        RTPFile = new QTRTPFile();
        if( RTPFile->Initialize(MovieFilename) != QTRTPFile::errNoError ) {
            qtss_printf("Error!  Could not open movie file \"%s\"!\n", MovieFilename);
            continue;
            //exit(1);
        }
        
        
        //
        // Get the SDP file and write it out.
        {
            // General vars
            char        *SDPFile;
            int         SDPFileLength;
            
            int         fdsdp;
            char        SDPFilename[255 + 1];
            
            
            //
            // Get the file
            SDPFile = RTPFile->GetSDPFile(&SDPFileLength);
            if( SDPFile == NULL ) {
                qtss_printf("Error!  Could not get SDP file!\n");
                continue;
                //exit(1);
            }
    
            //
            // Create our SDP file and write out the data
            if( OutputFilename == NULL ) {
                fdsdp = STDOUT_FILENO;
            } else {
                qtss_sprintf(SDPFilename, "%s.sdp", MovieFilename);
                fdsdp = open(SDPFilename, O_CREAT | O_TRUNC | O_WRONLY, 0664);
                if( fdsdp == -1 ) {
                    qtss_printf("Error!  Could not create SDP file \"%s\"!\n", SDPFilename);
                    continue;
                    //exit(1);
                }
            }
            
            qtss_printf("\n--%s--\n", MovieFilename);
            
            write(fdsdp, SDPFile, SDPFileLength);
            
            qtss_printf("\n");

            
            if( OutputFilename != NULL )
                close(fdsdp);
        }
    
        //
        // Close our RTP file.
        delete RTPFile;

    }

    return 0;
}

