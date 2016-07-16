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
	 File:       QTSSUserProfile.h

	 Contains:   An object to store User Profile, for authentication
				 and authorization

				 Implements the RTSP Request dictionary for QTSS API.


 */


#ifndef __QTSSUSERPROFILE_H__
#define __QTSSUSERPROFILE_H__

 //INCLUDES:
#include "QTSS.h"
#include "QTSSDictionary.h"

class QTSSUserProfile : public QTSSDictionary
{
public:

	//Initialize
	//Call initialize before instantiating this class. For maximum performance, this class builds
	//any response header it can at startup time.
	static void         Initialize();

	//CONSTRUCTOR & DESTRUCTOR
	QTSSUserProfile();
	virtual ~QTSSUserProfile() {}

protected:

	enum
	{
		kMaxUserProfileNameLen = 32,
		kMaxUserProfilePasswordLen = 32
	};

	char    fUserNameBuf[kMaxUserProfileNameLen];       // Set by RTSPRequest object
	char    fUserPasswordBuf[kMaxUserProfilePasswordLen];// Set by authentication module through API

	UInt32  fUserRights;  //Set by authorization module.
	//Dictionary support
	static QTSSAttrInfoDict::AttrInfo   sAttributes[];
};
#endif // __QTSSUSERPROFILE_H__

