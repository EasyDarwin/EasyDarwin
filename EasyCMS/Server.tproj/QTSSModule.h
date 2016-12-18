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
	 Copyleft (c) 2012-2016 EasyDarwin.ORG.  All rights reserved.
	 Github: https://github.com/EasyDarwin
	 WEChat: EasyDarwin
	 Website: http://www.EasyDarwin.org
 */
 /*
	Contains:   This object represents a single QTSS API compliant module.
				A module may either be compiled directly into the server,
				or loaded from a code fragment residing on the disk.

				Object does the loading and initialization of a module, and
				stores all per-module data.


*/

#ifndef __QTSSMODULE_H__
#define __QTSSMODULE_H__

#include "QTSS.h"
#include "QTSS_Private.h"
#include "QTSSDictionary.h"
#include "Task.h"
#include "QTSSPrefs.h"

#include "OSCodeFragment.h"
#include "OSQueue.h"
#include "StrPtrLen.h"

#define MODULE_DEBUG 0

class QTSSModule : public QTSSDictionary, public Task
{
public:

	//
	// INITIALIZE
	static void     Initialize();

	// CONSTRUCTOR / SETUP / DESTRUCTOR

	// Specify the path to the code fragment if this module
	// is to be loaded from disk. If it is loaded from disk, the
	// name of the module will be its file name. Otherwise, the
	// inName parameter will set it.

	QTSSModule(char* inName, char* inPath = NULL);

	// This function does all the module setup. If the module is being
	// loaded from disk, you need not pass in a main entrypoint (as
	// it will be grabbed from the fragment). Otherwise, you must pass
	// in a main entrypoint.
	//
	// Note that this function does not invoke any public module roles.
	QTSS_Error  SetupModule(QTSS_CallbacksPtr inCallbacks, QTSS_MainEntryPointPtr inEntrypoint = NULL);

	// Doesn't free up internally allocated stuff
	virtual ~QTSSModule() {}

	//
	// MODIFIERS
	void            SetPrefsDict(QTSSPrefs* inPrefs) { fPrefs = inPrefs; }
	void            SetAttributesDict(QTSSDictionary* inAttributes) { fAttributes = inAttributes; }
	//
	// ACCESSORS

	OSQueueElem*    GetQueueElem() { return &fQueueElem; }
	bool          IsInitialized() { return fDispatchFunc != NULL; }
	QTSSPrefs*      GetPrefsDict() { return fPrefs; }
	QTSSDictionary* GetAttributesDict() { return fAttributes; }
	OSMutex*        GetAttributesMutex() { return &fAttributesMutex; }

	//convert QTSS.h 4 char id roles to private role index
	static SInt32 GetPrivateRoleIndex(QTSS_Role apiRole);


	// This calls into the module.
	QTSS_Error CallDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
	{
		SInt32 theRoleIndex = -1;

		if (MODULE_DEBUG)
		{
			this->GetValue(qtssModName)->PrintStr("QTSSModule::CallDispatch ENTER module=", " role=");
			theRoleIndex = GetPrivateRoleIndex(inRole);
			if (theRoleIndex != -1)
				qtss_printf(" %s ENTR\n", sRoleNames[theRoleIndex]);

		}
		QTSS_Error theError = (fDispatchFunc)(inRole, inParams);

		if (MODULE_DEBUG)
		{
			this->GetValue(qtssModName)->PrintStr("QTSSModule::CallDispatch EXIT  module=", " role=");
			if (theRoleIndex != -1)
				qtss_printf(" %s EXIT\n", sRoleNames[theRoleIndex]);
		}

		return theError;
	}


	// These enums allow roles to be stored in a more optimized way
	// add new RoleNames to sRoleNames in QTSSModule.cpp for debugging       
	enum
	{
		kInitializeRole = 0,
		kShutdownRole = 1,
		kErrorLogRole = 2,
		kRereadPrefsRole = 3,
		kOpenFileRole = 4,
		kOpenFilePreProcessRole = 5,
		kAdviseFileRole = 6,
		kReadFileRole = 7,
		kCloseFileRole = 8,
		kRequestEventFileRole = 9,
		kStateChangeRole = 10,
		kTimedIntervalRole = 11,

		kEasyNonceRole = 12,
		kEasyAuthRole = 13,

		kRedisTTLRole = 14,
		kRedisAddDeviceRole = 15,
		kRedisDelDeviceRole = 16,
		kRedisGetEasyDarwinRole = 17,
		kRedisGetBestEasyDarwinRole = 18,
		kRedisGenStreamIDRole = 19,

		kNumRoles = 20
	};
	typedef UInt32 RoleIndex;

	// Call this to activate this module in the specified role.
	QTSS_Error  AddRole(QTSS_Role inRole);

	// This returns true if this module is supposed to run in the specified role.
	bool  RunsInRole(RoleIndex inIndex) { Assert(inIndex < kNumRoles); return fRoleArray[inIndex]; }

	SInt64 Run();

	QTSS_ModuleState* GetModuleState() { return &fModuleState; }

private:
	QTSS_Error loadFromDisk(QTSS_MainEntryPointPtr* outEntrypoint);

	OSQueueElem                 fQueueElem;
	char*                       fPath;
	OSCodeFragment*             fFragment;
	QTSS_DispatchFuncPtr        fDispatchFunc;
	bool                      fRoleArray[kNumRoles];
	QTSSPrefs*                  fPrefs;
	QTSSDictionary*             fAttributes;
	OSMutex                     fAttributesMutex;

	static bool       sHasOpenFileModule;

	static QTSSAttrInfoDict::AttrInfo   sAttributes[];
	static char* sRoleNames[];

	QTSS_ModuleState    fModuleState;

};



#endif //__QTSSMODULE_H__
