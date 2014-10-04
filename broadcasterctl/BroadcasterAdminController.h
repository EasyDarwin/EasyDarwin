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

#import <Foundation/Foundation.h>

#define kDefaultBroadcasterLoc @"/Applications/QuickTime Broadcaster.app"
#define kCantFindBroadcasterError "ERROR: No QuickTime Broadcaster at this location!\n"
#define kCantLaunchBroacasterError "ERROR: Couldn't launch QuickTime Broadcaster!\n"
#define kBroadcasterNotRunningError "ERROR: QuickTime Broadcaster not running!\n"
#define kBadConfigFileError "ERROR: Bad Config File! Quitting QuickTime Broadcaster.\n"
#define kCantKillError "ERROR: Can't quit QuickTime Broadcaster. Try running broadcasterctl as root.\n"

@interface BroadcasterAdminController : NSObject {
    NSString *myBroadcasterAppPath;
    id myBroadcasterProxy;
    BOOL outputXML;
}

// Class methods
+ (id)broadcasterAdminController;
+ (void)printUsage;

// Commands (from broadcasterctl args)
- (void)launchBroadcaster;
- (void)startBroadcaster;
- (void)stopBroadcaster;
- (void)quitBroadcaster;
- (void)printConfig;
- (void)printXMLConfig;
- (void)printPresets;

// Accessor methods
- (NSString *)myBroadcasterAppPath;
- (void)setMyBroadcasterAppPath:(NSString *)newPath;
- (id)myBroadcasterProxy;

@end
