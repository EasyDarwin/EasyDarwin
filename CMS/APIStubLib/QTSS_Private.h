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
	Copyleft (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
/*
    File:       QTSS_Private.h

    Contains:   Implementation-specific structures and typedefs used by the
                implementation of QTSS API in the Darwin Streaming Server
                    
    
    
*/


#ifndef QTSS_PRIVATE_H
#define QTSS_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "OSHeaders.h"
#include "QTSS.h"

class QTSSModule;
class Task;

typedef QTSS_Error  (*QTSS_CallbackProcPtr)(...);
typedef void*       (*QTSS_CallbackPtrProcPtr)(...);

enum
{
    // Indexes for each callback routine. Addresses of the callback routines get
    // placed in an array. 
    // IMPORTANT: When adding new callbacks, add only to the end of the list and increment the 
    //            kLastCallback value. Inserting or changing the index order will break dynamic modules
    //            built with another release.
    
    kNewCallback                    = 0,
    kDeleteCallback                 = 1,
    kMillisecondsCallback           = 2,
    kConvertToUnixTimeCallback      = 3,
    kAddRoleCallback                = 4,
    kAddAttributeCallback           = 5,
    kIDForTagCallback               = 6,
    kGetAttributePtrByIDCallback    = 7,
    kGetAttributeByIDCallback       = 8,
    kSetAttributeByIDCallback       = 9,
    kWriteCallback                  = 10,
    kWriteVCallback                 = 11,
    kFlushCallback                  = 12,
    kAddServiceCallback             = 13,
    kIDForServiceCallback           = 14,
    kDoServiceCallback              = 15,
    kSendRTSPHeadersCallback        = 16,
    kAppendRTSPHeadersCallback      = 17,
    kSendStandardRTSPCallback       = 18,
    kAddRTPStreamCallback           = 19,
    kPlayCallback                   = 20,
    kPauseCallback                  = 21,
    kTeardownCallback               = 22,
    kRequestEventCallback           = 23,
    kSetIdleTimerCallback           = 24,
    kOpenFileObjectCallback         = 25,
    kCloseFileObjectCallback        = 26,
    kReadCallback                   = 27,
    kSeekCallback                   = 28,
    kAdviseCallback                 = 29,
    kGetNumValuesCallback           = 30,
    kGetNumAttributesCallback       = 31,
    kSignalStreamCallback           = 32,
    kCreateSocketStreamCallback     = 33,
    kDestroySocketStreamCallback    = 34,
    kAddStaticAttributeCallback     = 35,
    kAddInstanceAttributeCallback   = 36,
    kRemoveInstanceAttributeCallback= 37,
    kGetAttrInfoByIndexCallback     = 38,
    kGetAttrInfoByNameCallback      = 39,
    kGetAttrInfoByIDCallback        = 40,
    kGetValueAsStringCallback       = 41,
    kTypeToTypeStringCallback       = 42,
    kTypeStringToTypeCallback       = 43,
    kStringToValueCallback          = 44,       
    kValueToStringCallback          = 45,       
    kRemoveValueCallback            = 46,
    kRequestGlobalLockCallback      = 47, 
    kIsGlobalLockedCallback         = 48, 
    kUnlockGlobalLock               = 49, 
    kAuthenticateCallback           = 50,
    kAuthorizeCallback              = 51,   
    kRefreshTimeOutCallback         = 52,
    kCreateObjectValueCallback      = 53,
    kCreateObjectTypeCallback       = 54,
    kLockObjectCallback             = 55,
    kUnlockObjectCallback           = 56,
    kSetAttributePtrCallback        = 57,
    kSetIntervalRoleTimerCallback   = 58,
    kLockStdLibCallback             = 59,
    kUnlockStdLibCallback           = 60,
	kSendHTTPPacketCallback			= 61,
	kRegDevSessionCallback			= 62,
	kUpdateDevRedisCallback			= 63,
	kUpdateDevSnapCallback			= 64,
    kLastCallback                   = 65
};

typedef struct {
    // Callback function pointer array
    QTSS_CallbackProcPtr addr [kLastCallback];
} QTSS_Callbacks, *QTSS_CallbacksPtr;

typedef struct
{
    UInt32                  inServerAPIVersion;
    QTSS_CallbacksPtr       inCallbacks;
    QTSS_StreamRef          inErrorLogStream;
    UInt32                  outStubLibraryVersion;
    QTSS_DispatchFuncPtr    outDispatchFunction;
    
} QTSS_PrivateArgs, *QTSS_PrivateArgsPtr;

typedef struct
{
    QTSSModule* curModule;  // this structure is setup in each thread
    QTSS_Role   curRole;    // before invoking a module in a role. Sometimes
    Task*       curTask;    // this info. helps callback implementation
    Bool16      eventRequested;
    Bool16      globalLockRequested;    // request event with global lock.
    Bool16      isGlobalLocked;
    SInt64      idleTime;   // If a module has requested idle time.
    
} QTSS_ModuleState, *QTSS_ModuleStatePtr;

QTSS_StreamRef  GetErrorLogStream();


#ifdef __cplusplus
}
#endif

#endif
