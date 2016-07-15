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
	 File:       QTSSWebDebugModule.cpp

	 Contains:   Implements web debug module


 */

#include "QTSSWebDebugModule.h"
#include "StrPtrLen.h"

 // STATIC DATA

static QTSS_AttributeID sStateAttr = qtssIllegalAttrID;

static StrPtrLen    sRequestHeader("GET /debug HTTP");

#if MEMORY_DEBUGGING
static char*        sResponseHeader = "HTTP/1.0 200 OK\r\nServer: TimeShare/1.0\r\nConnection: Close\r\nContent-Type: text/html\r\n\r\n";
static char*        sResponseEnd = "</BODY></HTML>";
#endif

// FUNCTION PROTOTYPES

QTSS_Error QTSSWebDebugModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams);
static QTSS_Error   Register(QTSS_Register_Params* inParams);
static QTSS_Error   Filter(QTSS_Filter_Params* inParams);

QTSS_Error QTSSWebDebugModule_Main(void* inPrivateArgs)
{
	return _stublibrary_main(inPrivateArgs, QTSSWebDebugModuleDispatch);
}

QTSS_Error QTSSWebDebugModuleDispatch(QTSS_Role inRole, QTSS_RoleParamPtr inParams)
{
	switch (inRole)
	{
	case QTSS_Register_Role:
		return Register(&inParams->regParams);
	case QTSS_RTSPFilter_Role:
		return Filter(&inParams->rtspFilterParams);
	}
	return QTSS_NoErr;
}

QTSS_Error Register(QTSS_Register_Params* inParams)
{
	// Do role & attribute setup
	(void)QTSS_AddRole(QTSS_RTSPFilter_Role);

	// Register an attribute
	static char*        sStateName = "QTSSWebDebugModuleState";
	(void)QTSS_AddStaticAttribute(qtssRTSPRequestObjectType, sStateName, NULL, qtssAttrDataTypeUInt32);
	(void)QTSS_IDForAttr(qtssRTSPRequestObjectType, sStateName, &sStateAttr);

	// Tell the server our name!
	static char* sModuleName = "QTSSWebDebugModule";
	::strcpy(inParams->outModuleName, sModuleName);

	return QTSS_NoErr;
}

QTSS_Error Filter(QTSS_Filter_Params* inParams)
{
	UInt32 theLen = 0;
	char* theFullRequest = NULL;
	(void)QTSS_GetValuePtr(inParams->inRTSPRequest, qtssRTSPReqFullRequest, 0, (void**)&theFullRequest, &theLen);

	if ((theFullRequest == NULL) || (theLen < sRequestHeader.Len))
		return QTSS_NoErr;
	if (::memcmp(theFullRequest, sRequestHeader.Ptr, sRequestHeader.Len) != 0)
		return QTSS_NoErr;

#if MEMORY_DEBUGGING
	UInt32* theStateVal = NULL;
	(void)QTSS_GetValuePtr(inParams->inRTSPRequest, sStateAttr, 0, (void**)&theStateVal, &theLen);
	//if ((theStateVal == NULL) || (theLen != sizeof(UInt32)))
	//{
	Bool16 theFalse = false;
	(void)QTSS_SetValue(inParams->inRTSPRequest, qtssRTSPReqRespKeepAlive, 0, &theFalse, sizeof(theFalse));

	// Begin writing the HTTP response. We don't need to worry about flow control
	// because we're using the QTSS_RTSPRequestObject for the response, which does buffering
	(void)QTSS_Write(inParams->inRTSPRequest, sResponseHeader, ::strlen(sResponseHeader), &theLen, 0);

	//QTSS_EventContextRef* theContext = NULL;
	//(void)QTSS_GetValuePtr(inParams->inRTSPSession, qtssRTSPSesEventCntxt, 0, (void**)&theContext, &theLen);
	//Assert(theContext != NULL);
	//Assert(theLen == sizeof(QTSS_EventContextRef));

	//(void)QTSS_RequestEvent(*theContext, EV_WR);

//  UInt32 theValue = 4;
//  (void)QTSS_SetValue(inParams->inRTSPRequest, sStateAttr, 0, &theValue, sizeof(theValue));
//  return QTSS_NoErr;
//}

//we must hold the tagQueue mutex for the duration of this exercise because
//we don't want any of the values we are reporting to change
	OSMutexLocker locker(OSMemory::GetTagQueueMutex());

	//write out header and total allocated memory
	char buffer[1024];
	qtss_sprintf(buffer, "<HTML><TITLE>TimeShare Debug Page</TITLE><BODY>Total dynamic memory allocated: %" _S32BITARG_ "<P>List of objects:<BR>", OSMemory::GetAllocatedMemory());
	(void)QTSS_Write(inParams->inRTSPRequest, buffer, ::strlen(buffer), &theLen, 0);

	//now report the list of tags:
	for (OSQueueIter iter(OSMemory::GetTagQueue()); !iter.IsDone(); iter.Next())
	{
		OSMemory::TagElem* elem = (OSMemory::TagElem*)iter.GetCurrent()->GetEnclosingObject();
		Assert(elem != NULL);
		if (elem->numObjects > 0)
		{
			qtss_sprintf(buffer, "Object allocated at: %s, %d. Number of currently allocated objects: %" _S32BITARG_ ", Total size: %" _S32BITARG_ "<BR>", elem->fileName, elem->line, elem->numObjects, elem->totMemory);
			(void)QTSS_Write(inParams->inRTSPRequest, buffer, ::strlen(buffer), &theLen, 0);
		}
	}
	(void)QTSS_Write(inParams->inRTSPRequest, sResponseEnd, ::strlen(sResponseEnd), &theLen, 0);
#endif
	return QTSS_NoErr;
}
