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
    kSendHttpPacketCallback        = 16,
    kRequestEventCallback           = 17,
    kSetIdleTimerCallback           = 18,
    kOpenFileObjectCallback         = 19,
    kCloseFileObjectCallback        = 20,
    kReadCallback                   = 21,
    kSeekCallback                   = 22,
    kAdviseCallback                 = 23,
    kGetNumValuesCallback           = 24,
    kGetNumAttributesCallback       = 25,
    kSignalStreamCallback           = 26,
    kAddStaticAttributeCallback     = 27,
    kAddInstanceAttributeCallback   = 28,
    kRemoveInstanceAttributeCallback= 29,
    kGetAttrInfoByIndexCallback     = 30,
    kGetAttrInfoByNameCallback      = 31,
    kGetAttrInfoByIDCallback        = 32,
    kGetValueAsStringCallback       = 33,
    kTypeToTypeStringCallback       = 34,
    kTypeStringToTypeCallback       = 35,
    kStringToValueCallback          = 36,       
    kValueToStringCallback          = 37,       
    kRemoveValueCallback            = 38,
    kRequestGlobalLockCallback      = 39, 
    kIsGlobalLockedCallback         = 40, 
    kUnlockGlobalLock               = 41, 
    kCreateObjectValueCallback      = 42,
    kCreateObjectTypeCallback       = 43,
    kLockObjectCallback             = 44,
    kUnlockObjectCallback           = 45,
    kSetAttributePtrCallback        = 46,
    kSetIntervalRoleTimerCallback   = 47,
    kLockStdLibCallback             = 48,
    kUnlockStdLibCallback           = 49,
    kLastCallback                   = 50
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
