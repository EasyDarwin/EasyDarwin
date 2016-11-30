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
	 File:       RTSPSession3GPP.h

	 Contains:   This parses 3gpp headers from a request



 */

#ifndef __RTSPSESSION3GPP_H__
#define __RTSPSESSION3GPP_H__

#include "QTSS.h"
#include "QTSSDictionary.h"


 //RTSPSession 3GPP utility class definition
class RTSPSession3GPP : public QTSSDictionary
{
public:
	//Initialize
	//Call initialize before instantiating this class: see QTSServer.cpp.
	static void         Initialize();

	//CONSTRUCTOR / DESTRUCTOR
	//these do very little. Just initialize / delete some member data.
	//
	//Arguments:        enable the object
	RTSPSession3GPP(Bool16 enabled);
	~RTSPSession3GPP() {}



private:
	Bool16 fEnabled;

	//Dictionary support
	static QTSSAttrInfoDict::AttrInfo   sAttributes[];

};
#endif // __RTSPSESSION3GPP_H__

