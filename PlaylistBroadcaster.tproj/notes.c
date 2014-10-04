/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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
 */
#if 0


todo
    

v12 --
    - changed PlayListBroadcaster to PlaylistBroadcaster
    - don't strip ext from Descr filename before adding .log in a default file name.    
    - play_list_file becomes playlist_file
    - repeats_queue_size becomes recent_movies_list_size
    - limit_play is not used anymore
    

v11--
    - changed a bunch of error messages
    -   fixed: In my broadcast description file, I left off any keyword after "logging" 
        and both the preflight and the regular run of the file caused a Bus error.
    - added descriptive messages about setup file errors.
    - changed name from /tmp/trackerfile to /tmp/broadcastlist
    - chdir to location of Setup file ( means that paths in Setup file are
    relative to locaation of setup file, not PlayListBroadcaster
    
    

v10
    -- writes message that SDP file was generated in debug or preflight? 2365986
    --  fixed pathname display bug for log file name errors. ... "the.log", or else "./the.log", or else
     the full path should have been displayed... 
    -- added "PlayListBroadcaster preflight finished.\n"  at end of prefilght ( already logged ).

    -- err desc is logged, not err #

        Error:# 

        as in these samples

                # 1999-08-04 13:36:25 PlayListBroadcaster started
                1999-08-04 13:44:53 /Beamer/projects/Servers/TimeShare/PlayListBroadcaster/pl_files/lists/3movie3.mov Error:12
                1999-08-04 13:44:55 /Beamer/projects/Servers/TimeShare/PlaylistBroadcaster/pl_files/lists/test1.mov OK
                # 1999-08-04 14:57:49 PlaylistBroadcaster finished


        to 

        Error:"errstring"

        where "errstring" is one of the following:

            Movie file not found
            Movie file has no hinted tracks
            Movie file does not match SDP
            Movie file is invalid
            Movie file name is missing or too long
    
8.6.99 build 9
    - PLB logs preflights, # movie problems
    - cleaned up code in PlayListBroadcaster.cpp
    - only complains about lack of tracker and logfile access when necessary
    - "Setup File Name".los is the default logfile name
    - changed display occurances of PlaylistBroadcaster to PlayListBroadcaster
    - report permission to stop denied error, not just "not running"
    
    
8.5.99 build 8
    - chanegd track match error to warning and reworded
    - added check for tracker access.
    - added Lock to log file
    - fixed problems when running in background ( loss of tracking and logging )
    
    

8.4.99 build 7
    - added Error message "- PlayListBroadcaster: Error, unsupported Play Mode (%s)\n"
    - uses Setup supplied logfile name, doed not append .log 2364223
    - added error messages about not being able to open the log file
    - added error messages about not being able to open the Broadcast setup file.
    - added playError messages to log file:
        # 1999-08-04 13:36:25 PlayListBroadcaster started
        1999-08-04 13:44:53 /Beamer/projects/Servers/TimeShare/PlayListBroadcaster/pl_files/lists/3movie3.mov Error:12
        1999-08-04 13:44:55 /Beamer/projects/Servers/TimeShare/PlaylistBroadcaster/pl_files/lists/test1.mov OK
        # 1999-08-04 14:57:49 PlaylistBroadcaster finished

    - will not ignore ^C's when there is an initial problem with the tracker file.
    
8.2.99 -- build 6
    - gen error messages from broadcaster
    - changed to all verbose options
    - removed -c option, just list file the name at end of command.
    - destination ip now defaults to machine's primary address
    - changed BCasterTracker::BCasterTracker to 5 second open timer.
    - changed reference to "channel setup" to "PlaylistBroadcaster Description"
    - changed &d's in qtss_printf's to %d in Broadcaster error messages
    - only print errors when walking the play list
    - only show Setup options in Preflight mode
    - changed display of setup options to show user names, not C++ names.
    - the -list option no longer lists broadcasts that are not running. file is
    cleaned up on next "stop"
    - multiple options per command line are now supported, exits on first error.
    - command line options uses "getopt"'s long form that allows -fullword options
    and partial ( down to one char ) options as long as they are unambiguous.

8.3.99
    - fixed include loops
    - better error reporting from building the picklist
    - chmod's the "trackerfile" to 666 so that all users can run broadcasts 
    and track them.
    - Shows permission error if that is the reason a "stop" fails.
    - fixes problems with fully qualified paths in both Playlist and "play_list_file"
    Setup file  option
    

7.30.99 - build 5
     - Movie file and SDP parsing now includes "user" error reporting ( not using Assert ). 
     So the program should continue past recoverable errors and stop only on non recoverable 
     errors.
     
     - Playlists no longer require weights, non - weighted files in a random playlist 
     default to a weight of 10.
     
     - + symbol introduces an "included" play list
     
     - play list files MUST have "*PLAY-LIST*" as the first line ( exactly ). 
     Else they will be treated as an invalid play list file.
     
     - new "-prefilght" option displays Setup, SDP, and walks the movie list to check
     files for correctness.
     
     - removed debug options from command line options.



7.27.99 - build 4
    - removed license from about display
    - updated credit names
    - fixed mapping of --stop to 's' from 'l'
    - added -a (about) to help
    
    - builds from libQTRTPFile.a
    - added ppc only instructions to make.preamble
    
    - uses a well known place to keep the tracker file, was kept is same dir
    as PlaylistBroadcaster ( now uses /tmp/ )
    
    - better error message on "stop" with bad parameters.
    - removed Assert on playlist "includes"
    - added default destination adddress ( 127.0.0.1 "localhost" )
    - changed "weighted" mode parameter to "weighted_random" as per docs
    - changes picking algorithm to provide better weighting
    - fixes the "count5.mov" crash
    - fixes the "can't play more than 8 movies without losing audio" bug
    
    
    

7.26.99 -- build 3  Notes and Errata

    not documented: uses "sdp_reference_movie" option (mSDPReferenceMovie) to bulid SDP file.
    Add this option to the config file.  It's a required field.
    
    Use the -g file.config option to cause PlaylistBroadcaster to parse and display the 
    settings in a config file.
    
    SDP generation displays the raw SDP of the reference file, not the generated
    file.
    
    If you set the config "limit_seq_length" to a value greater than the number of 
    movies available for play in random mode, the movies will play once and the
    broadcast will stop.
    
    
    The executable is incorrectly named BroadcastPlaylist, instead of PlaylistBroacaster.
    
    The installer read me references v1.0.1, not v1.0.2.
    
    The broadcast will lose audio after 7 or 8 movies play.  You can restore audio 
    by pausing the client and then restarting.
    

    
    
    
    
    
#endif