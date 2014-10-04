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

#import "BroadcasterAdminController.h"
#import "BroadcasterRemoteAdmin.h"
#include <signal.h>
#include <unistd.h>
#include "revision.h"


@implementation BroadcasterAdminController

// Class methods
+ (id)broadcasterAdminController
{
    return [[[BroadcasterAdminController alloc] init] autorelease];
}

+ (void)printUsage
{
	NSString *versionString = [NSString stringWithCString:kVersionString];
	NSString *buildString = [NSString stringWithCString:kBuildString];
    NSString *usage = [NSString stringWithFormat:@"\nbroadcasterctl %@ (%@)\n\nUsage: broadcasterctl [-b broadcaster-path] [-a audiopreset] [-v videopreset] [-n networkpreset] [-t (audio|video|av)] [-r (record|norecord)] [-f settingsfile] ((config|status|presets|launch|start|stop|restart|quit)\n\n", versionString, buildString];

    printf([usage cString]);
}

- (NSArray *)otherBroadcasters
{
    NSTask *psTask;
    NSString *psResult;
    NSPipe *pipe;
    NSFileHandle *pipeFileHandle;
    NSArray *splitResult;
    NSEnumerator *processEnumerator;
    id processLine;
    NSMutableArray *processesToKill = [NSMutableArray array];
    NSString *processID;
    NSScanner *processIDScanner;

    psTask = [[[NSTask alloc] init] autorelease];
    [psTask setLaunchPath:@"/bin/ps"];
    [psTask setArguments:[NSArray arrayWithObject:@"awx"]];
    pipe = [NSPipe pipe];
    [psTask setStandardOutput:pipe];
    [psTask launch];
    [psTask waitUntilExit];
    pipeFileHandle = [pipe fileHandleForReading];
    psResult = [[[NSString alloc] initWithData:[pipeFileHandle readDataToEndOfFile] encoding:NSASCIIStringEncoding] autorelease];
    splitResult = [psResult componentsSeparatedByString:@"\n"];
    processEnumerator = [splitResult objectEnumerator];
    while (processLine = [processEnumerator nextObject]) {
		if ([processLine rangeOfString:@"/QuickTime Broadcaster.app"].location != NSNotFound) {
			processID = [NSString string];
			processIDScanner = [NSScanner scannerWithString:processLine];
			[processIDScanner scanUpToString:@" " intoString:&processID];
			[processesToKill addObject:processID];
		}
    }
    return [NSArray arrayWithArray:processesToKill];
}

- (void)killOtherBroadcastersWithSignal:(int)signal
{
    NSArray *otherBroadcasters = [self otherBroadcasters];
    NSEnumerator *processEnumerator = [otherBroadcasters objectEnumerator];
    id process;
	BOOL success = YES;

    while (process = [processEnumerator nextObject]) {
		if (kill([process intValue], 1) != 0)
			success = NO;
    }

	if (!success)
		printf(kCantKillError);
}

- (NSString *)resultForArgument:(NSString *)argName foundError:(BOOL *)err
{
    NSArray *args = [[NSProcessInfo processInfo] arguments];
    unsigned long argLoc;
    NSString *result;

    argLoc = [args indexOfObject:argName];
    if (argLoc == NSNotFound) {
		*err = NO;
		return nil;
    }

    argLoc++;

    // if there are no more arguments, something is wrong
    if ([args count] <= argLoc) {
		*err = YES;
		return nil;
    }

    result = [args objectAtIndex:argLoc];

    // if it starts with a dash, something is wrong
    if ([result characterAtIndex:0] == '-') {
		*err = YES;
		return nil;
    }

    // at this point, all should be fine
    *err = NO;
    return result;
}

- (NSString *)fixPath:(NSString *)path
{
    NSString *pwd = [[[NSProcessInfo processInfo] environment] objectForKey:@"PWD"];
    NSMutableArray *pathComponents = [NSMutableArray arrayWithArray:[[path stringByExpandingTildeInPath] pathComponents]];
    if (!pwd)
		return path;
    if ([path isAbsolutePath])
		return path;

    while ([[pathComponents objectAtIndex:0] isEqualToString:@".."]) {
		[pathComponents removeObjectAtIndex:0];
		pwd = [pwd stringByDeletingLastPathComponent];
    }

    return [pwd stringByAppendingPathComponent:[NSString pathWithComponents:pathComponents]];
}

- (id)init
{
    NSArray *args = [[NSProcessInfo processInfo] arguments];
    NSString *actionArg = [args lastObject];
    BOOL err;
    NSString *argValue;

    self = [super init];

    if ([args count] <= 1) {
		return nil; // bail
    }

    // look for an app path argument
    argValue = [self resultForArgument:@"-b" foundError:&err];
    if (err)
		return nil;
    if (argValue)
		[self setMyBroadcasterAppPath:[self fixPath:argValue]];
    else
		[self setMyBroadcasterAppPath:kDefaultBroadcasterLoc];

    // if we're not telling it to quit, launch the broadcaster if it's not running
    if ((![actionArg isEqualToString:@"quit"]) && (![self myBroadcasterProxy])) {
		if ([[self otherBroadcasters] count] > 0) {
			printf("\nCannot remotely administer the current Broadcaster.");
			printf("\nUse the 'quit' command to quit the currently running broadcaster.\n\n");
			return self;
		}
		if ([actionArg isEqualToString:@"launch"])
			[self launchBroadcaster];
		else {
			printf("\nThe broadcaster isn't running.");
			printf("\nLaunch it with the 'launch' command.\n\n");
			return self;
		}
    }

    // config file argument
    argValue = [self resultForArgument:@"-f" foundError:&err];
    if (err)
		return nil;
    if (argValue && [self myBroadcasterProxy]) {
		[[self myBroadcasterProxy] setCurrentPresetName:nil ofType:kPresetAudio];
		[[self myBroadcasterProxy] setCurrentPresetName:nil ofType:kPresetVideo];
		[[self myBroadcasterProxy] setCurrentPresetName:nil ofType:kPresetNetwork];
		NS_DURING
			[[self myBroadcasterProxy] setBroadcastSettingsFile:[self fixPath:argValue]];
		NS_HANDLER
			printf("ERROR: Invalid settings file.\n");
			return self;
		NS_ENDHANDLER
    }

    // audio preset argument
    argValue = [self resultForArgument:@"-a" foundError:&err];
    if (err)
		return nil;
    if (argValue && [self myBroadcasterProxy]) {
		[[self myBroadcasterProxy] setCurrentPresetName:argValue ofType:kPresetAudio];
    }

    // video preset argument
    argValue = [self resultForArgument:@"-v" foundError:&err];
    if (err)
		return nil;
    if (argValue && [self myBroadcasterProxy]) {
		[[self myBroadcasterProxy] setCurrentPresetName:argValue ofType:kPresetVideo];
    }

    // network preset argument
    argValue = [self resultForArgument:@"-n" foundError:&err];
    if (err)
		return nil;
    if (argValue && [self myBroadcasterProxy]) {
		[[self myBroadcasterProxy] setCurrentPresetName:argValue ofType:kPresetNetwork];
    }

    // stream type
    argValue = [self resultForArgument:@"-t" foundError:&err];
    if (err)
		return nil;
    if (argValue && [self myBroadcasterProxy]) {
		//audio/video/av
		if ([self myBroadcasterProxy]) {
			if ([argValue isEqualToString:@"audio"]) {
				[[self myBroadcasterProxy] setStreamEnabled:YES ofType:kStreamTypeAudio];
				[[self myBroadcasterProxy] setStreamEnabled:NO ofType:kStreamTypeVideo];
			}
			else if ([argValue isEqualToString:@"video"]) {
				[[self myBroadcasterProxy] setStreamEnabled:NO ofType:kStreamTypeAudio];
				[[self myBroadcasterProxy] setStreamEnabled:YES ofType:kStreamTypeVideo];
			}
			else if ([argValue isEqualToString:@"av"]) {
				[[self myBroadcasterProxy] setStreamEnabled:YES ofType:kStreamTypeAudio];
				[[self myBroadcasterProxy] setStreamEnabled:YES ofType:kStreamTypeVideo];
			}
			else
				return nil;
		}
    }

    /*// recording path argument
		argValue = [self resultForArgument:@"-p" foundError:&err];
    if (err)
		return nil;
    if (argValue && [self myBroadcasterProxy]) {
		[[self myBroadcasterProxy] setRecordingPath:argValue];
    }*/

    // recording argument
    argValue = [self resultForArgument:@"-r" foundError:&err];
    if (err)
		return nil;
    if (argValue && [self myBroadcasterProxy]) {
		if ([argValue isEqualToString:@"record"])
			[[self myBroadcasterProxy] setRecording:YES];
		else if ([argValue isEqualToString:@"norecord"])
			[[self myBroadcasterProxy] setRecording:NO];
    }

    if ([actionArg isEqualToString:@"launch"]) {
		// no need to do anything... it'll launch on its own
    }
    else if ([actionArg isEqualToString:@"start"])
		[self startBroadcaster];
    else if ([actionArg isEqualToString:@"stop"])
		[self stopBroadcaster];
    else if ([actionArg isEqualToString:@"restart"]) {
		[self stopBroadcaster];
		[self startBroadcaster];
    }
    else if ([actionArg isEqualToString:@"quit"])
		[self quitBroadcaster];
    else if (([actionArg isEqualToString:@"config"]) || ([actionArg isEqualToString:@"status"]))
		[self printConfig];
    else if ([actionArg isEqualToString:@"xmlstatus"])
		[self printXMLConfig];
    else if ([actionArg isEqualToString:@"presets"])
		[self printPresets];
    else
		return nil;

    return self;
}

// Commands (from broadcasterctl args)

- (void)launchBroadcaster
{
    NSString *launchPath;
    NSTask *broadcasterTask;

    launchPath = [[self myBroadcasterAppPath] stringByAppendingPathComponent:@"Contents"];
    launchPath = [launchPath stringByAppendingPathComponent:@"MacOS"];
    launchPath = [launchPath stringByAppendingPathComponent:@"QuickTime Broadcaster"];

    if (![[NSFileManager defaultManager] fileExistsAtPath:launchPath]) {
		printf(kCantFindBroadcasterError);
		return;
    }

    if (!(broadcasterTask = [NSTask launchedTaskWithLaunchPath:launchPath arguments:[NSArray arrayWithObject:@"-noui"]])) {
		printf(kCantLaunchBroacasterError);
		return;
    }

    if (![broadcasterTask isRunning]) {
		printf(kCantLaunchBroacasterError);
		return;
    }

    while (![self myBroadcasterProxy])
		sleep(1);
}

- (void)startBroadcaster
{
	int i;
	int state;

    [myBroadcasterProxy startBroadcast];

	for (i = 0; i < 7; i++) {
		sleep(1);
		state = [[self myBroadcasterProxy] state];
		if (state == kBroadcasterStateBroadcasting)
			return;
	}
	printf(kBadConfigFileError);
	[self killOtherBroadcastersWithSignal:9];
}

- (void)stopBroadcaster
{
    if (![self myBroadcasterProxy]) {
		printf(kBroadcasterNotRunningError);
		return;
    }

    [[self myBroadcasterProxy] stopBroadcast];
}

- (void)quitBroadcaster
{
    if ([self myBroadcasterProxy]) {
		[[self myBroadcasterProxy] quit];
    }
    [self killOtherBroadcastersWithSignal:1];
}

- (void)printConfig
{
    NSString *settingStringValue;

    if (![self myBroadcasterProxy]) {
		printf(kBroadcasterNotRunningError);
		return;
    }

    printf("\nCurrent State: ");
    printf([[[self myBroadcasterProxy] stateString] cString]);

    if (settingStringValue = [[self myBroadcasterProxy] broadcastSettingsFile]) {
		printf("\nBroadcast Settings File: ");
		printf([settingStringValue cString]);
    }

    printf("\nAudio Stream: ");
    if ([[self myBroadcasterProxy] streamEnabled:kStreamTypeAudio])
		printf("Enabled");
    else
		printf("Disabled");

    printf("\nVideo Stream: ");
    if ([[self myBroadcasterProxy] streamEnabled:kStreamTypeVideo])
		printf("Enabled");
    else
		printf("Disabled");

    printf("\n\nCurrently Selected Presets:");
    printf("\n    Audio: ");
    if (settingStringValue = [[self myBroadcasterProxy] currentPresetName:kPresetAudio])
		printf([settingStringValue cString]);
    else
		printf("(none)");
    printf("\n    Video: ");
    if (settingStringValue = [[self myBroadcasterProxy] currentPresetName:kPresetVideo])
		printf([settingStringValue cString]);
    else
		printf("(none)");
    printf("\n    Network: ");
    if (settingStringValue = [[self myBroadcasterProxy] currentPresetName:kPresetNetwork])
		printf([settingStringValue cString]);
    else
		printf("(none)");

    if ([[self myBroadcasterProxy] recording]) {
		printf("\n\nRecording to ");
		settingStringValue = [[self myBroadcasterProxy] recordingPath];
		if (settingStringValue) {
			printf([settingStringValue cString]);
		}
		else
			printf("(default loc)");
    }
    else {
		printf("\n\nNot Recording");
    }

    printf("\n\n");
}

- (void)printXMLConfig
{
    NSMutableDictionary *printDict = [NSMutableDictionary dictionary];
    NSString *settingStringValue;
    NSMutableDictionary *currentPresets = [NSMutableDictionary dictionary];
    NSMutableDictionary *presetDict = [NSMutableDictionary dictionary];
    NSData *plistData;

    if (![self myBroadcasterProxy]) {
		[printDict setObject:[NSString stringWithCString:kBroadcasterNotRunningError] forKey:@"error"];
		printf([[printDict description] cString]);
		return;
    }

    // state and settings file
    [printDict setObject:[[self myBroadcasterProxy] stateString] forKey:@"stateString"];
    if (!(settingStringValue = [[self myBroadcasterProxy] broadcastSettingsFile]))
		settingStringValue = @"";
    [printDict setObject:settingStringValue forKey:@"broadcastSettingsFile"];

    // currently selected presets
    if (!(settingStringValue = [[self myBroadcasterProxy] currentPresetName:kPresetAudio]))
		settingStringValue = @"";
    [currentPresets setObject:settingStringValue forKey:@"audio"];
    if (!(settingStringValue = [[self myBroadcasterProxy] currentPresetName:kPresetVideo]))
		settingStringValue = @"";
    [currentPresets setObject:settingStringValue forKey:@"video"];
    if (!(settingStringValue = [[self myBroadcasterProxy] currentPresetName:kPresetNetwork]))
		settingStringValue = @"";
    [currentPresets setObject:settingStringValue forKey:@"network"];
    [printDict setObject:currentPresets forKey:@"selectedPresets"];

    // recording options
    [printDict setObject:[NSNumber numberWithBool:[[self myBroadcasterProxy] recording]] forKey:@"recording"];
    [printDict setObject:[[self myBroadcasterProxy] recordingPath] forKey:@"recordingPath"];

    // all presets
    [presetDict setObject:[[self myBroadcasterProxy] presetNameList:kPresetAudio] forKey:@"audio"];
    [presetDict setObject:[[self myBroadcasterProxy] presetNameList:kPresetVideo] forKey:@"video"];
    [presetDict setObject:[[self myBroadcasterProxy] presetNameList:kPresetNetwork] forKey:@"network"];
    [printDict setObject:presetDict forKey:@"allPresets"];

    // print the dictionary
    plistData = (NSData *)CFPropertyListCreateXMLData(NULL, printDict);

    printf([[[NSString alloc] initWithData:plistData encoding:NSUTF8StringEncoding] cString]);
}

- (void)printPresets
{
    NSArray *allPresets;
    NSEnumerator *presetEnumerator;
    id currentPreset;

    if (![self myBroadcasterProxy]) {
		printf(kBroadcasterNotRunningError);
		return;
    }

    printf("Audio Presets:\n");
    allPresets = [[self myBroadcasterProxy] presetNameList:kPresetAudio];
    if ([allPresets count] == 0)
		printf("    (none)\n");
    else {
		presetEnumerator = [allPresets objectEnumerator];
		while (currentPreset = [presetEnumerator nextObject]) {
			printf("    ");
			printf([currentPreset cString]);
			printf("\n");
		}
    }
    printf("Video Presets:\n");
    allPresets = [[self myBroadcasterProxy] presetNameList:kPresetVideo];
    if ([allPresets count] == 0)
		printf("    (none)\n");
    else {
		presetEnumerator = [allPresets objectEnumerator];
		while (currentPreset = [presetEnumerator nextObject]) {
			printf("    ");
			printf([currentPreset cString]);
			printf("\n");
		}
    }
    printf("Network Presets:\n");
    allPresets = [[self myBroadcasterProxy] presetNameList:kPresetNetwork];
    if ([allPresets count] == 0)
		printf("    (none)\n");
    else {
		presetEnumerator = [allPresets objectEnumerator];
		while (currentPreset = [presetEnumerator nextObject]) {
			printf("    ");
			printf([currentPreset cString]);
			printf("\n");
		}
    }
}

// Accessor methods

- (NSString *)myBroadcasterAppPath
{
    return myBroadcasterAppPath;
}

- (void)setMyBroadcasterAppPath:(NSString *)newPath
{
    [newPath retain];
    if (myBroadcasterAppPath)
		[myBroadcasterAppPath release];
    myBroadcasterAppPath = newPath;
}

- (id)myBroadcasterProxy
{
    if (!myBroadcasterProxy) {
		myBroadcasterProxy = [[NSConnection rootProxyForConnectionWithRegisteredName:kBroadcasterRemoteAdmin host:nil] retain];
    }
    return myBroadcasterProxy;
}

@end
