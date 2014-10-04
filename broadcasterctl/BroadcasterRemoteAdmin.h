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
/*
    File:			BroadcasterRemoteAdmin.h
    Description:	This file contains the BroadcastController protocol. This protocol is
                    used to communicate with QuickTime Broadcaster.

*/

    // Mach nameserver name
#define kBroadcasterRemoteAdmin					@"QuickTimeBroadcasterRemoteAdmin"

    // stream types
#define kStreamTypeAudio						@"audio"
#define kStreamTypeVideo						@"video"

    // distributed notifications
#define kBroadcastQuitNotification				@"BroadcastQuitNotification"
#define kBroadcastLaunchedNotification			@"BraodcastLaunchedNotification"

    // settings
enum
{
    kSettingsAudio = 0,
    kSettingsVideo,
    kSettingsNetwork
};

    // preset types
enum
{
    kPresetAudio = 0,
    kPresetVideo,
    kPresetNetwork
};

    // broadcast state
enum
{
    kBroadcasterStateSetup = 0,
    kBroadcasterStateStartingBroadcast,
    kBroadcasterStatePrerolling,
    kBroadcasterStateBroadcasting,
    kBroadcasterStateStoppingBroadcast
};

    // stats
#define kBroadcastStatisticCPULoad					'cpul'
#define kBroadcastStatisticNumberOfUsersConnected	'nviw'
#define kBroadcastStatisticDataRate					'drto'
#define kBroadcastStatisticFrameRate				'fps '

    // the protocol used for remote admin (version 1)
@protocol BroadcastControllerProtocolVersion1

    // getters
- (NSString *)currentPresetName:(short)thePresetType;
- (BOOL)recording;
- (NSString *)recordingPath;
- (NSString *)recordingFileName;
- (NSArray *)presetNameList:(short)thePresetType;
- (NSString *)broadcastSettingsFile;
- (NSString *)stat:(OSType)theType forStreamType:(NSString *)theStreamType;
- (UInt32)state;
- (NSString *)stateString;
- (BOOL)streamEnabled:(NSString *)theStreamType;
- (BOOL)streamActive:(NSString *)theStreamType;
- (ComponentResult)broadcastError;
- (NSString *)broadcastErrorString;

    // setters
- (void)setCurrentPresetName:(NSString *)thePresetName ofType:(short)thePresetType;
- (void)setRecording:(BOOL)recordOn;
- (void)setRecordingPath:(NSString *)thePath;
- (void)setRecordingFileName:(NSString *)theFileName;
- (void)setBroadcastSettingsFile:(NSString *)thePath;
- (void)setStreamEnabled:(BOOL)enable ofType:(NSString *)theStreamType;

    // methods
- (void)startBroadcast;
- (void)stopBroadcast;
- (void)quit;

@end
