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

    kRequestEventCallback           = 19,
    kSetIdleTimerCallback           = 20,
    kOpenFileObjectCallback         = 21,
    kCloseFileObjectCallback        = 22,
    kReadCallback                   = 23,
    kSeekCallback                   = 24,
    kAdviseCallback                 = 25,
    kGetNumValuesCallback           = 26,
    kGetNumAttributesCallback       = 27,
    kSignalStreamCallback           = 28,
    kCreateSocketStreamCallback     = 29,
    kDestroySocketStreamCallback    = 30,
    kAddStaticAttributeCallback     = 31,
    kAddInstanceAttributeCallback   = 32,
    kRemoveInstanceAttributeCallback= 33,
    kGetAttrInfoByIndexCallback     = 34,
    kGetAttrInfoByNameCallback      = 35,
    kGetAttrInfoByIDCallback        = 36,
    kGetValueAsStringCallback       = 37,
    kTypeToTypeStringCallback       = 38,
    kTypeStringToTypeCallback       = 39,
    kStringToValueCallback          = 40,       
    kValueToStringCallback          = 41,       
    kRemoveValueCallback            = 42,
    kRequestGlobalLockCallback      = 43, 
    kIsGlobalLockedCallback         = 44, 
    kUnlockGlobalLock               = 45, 
    kAuthenticateCallback           = 46,
    kAuthorizeCallback              = 47,   

    kCreateObjectValueCallback      = 48,
    kCreateObjectTypeCallback       = 49,
    kLockObjectCallback             = 50,
    kUnlockObjectCallback           = 51,
    kSetAttributePtrCallback        = 52,
    kSetIntervalRoleTimerCallback   = 53,
    kLockStdLibCallback             = 54,
    kUnlockStdLibCallback           = 55,
	kSendHTTPPacketCallback			= 56,
	kRegDevSessionCallback			= 57,
	kUpdateDevRedisCallback			= 58,
	kUpdateDevSnapCallback			= 59,
    kLastCallback                   = 60
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
