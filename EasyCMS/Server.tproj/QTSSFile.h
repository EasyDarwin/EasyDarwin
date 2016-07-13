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
	 File:       QTSSFile.h

	 Contains:




 */

#include "QTSSDictionary.h"
#include "QTSSModule.h"

#include "OSFileSource.h"
#include "EventContext.h"

class QTSSFile : public QTSSDictionary
{
public:

	QTSSFile();
	virtual ~QTSSFile() {}

	static void     Initialize();

	//
	// Opening & Closing
	QTSS_Error          Open(char* inPath, QTSS_OpenFileFlags inFlags);
	void                Close();

	//
	// Implementation of stream functions.
	virtual QTSS_Error  Read(void* ioBuffer, UInt32 inLen, UInt32* outLen);

	virtual QTSS_Error  Seek(UInt64 inNewPosition);

	virtual QTSS_Error  Advise(UInt64 inPosition, UInt32 inAdviseSize);

	virtual QTSS_Error  RequestEvent(QTSS_EventType inEventMask);

private:

	QTSSModule* fModule;
	UInt64      fPosition;
	QTSSFile*   fThisPtr;

	//
	// File attributes
	UInt64      fLength;
	time_t      fModDate;

	static QTSSAttrInfoDict::AttrInfo   sAttributes[];
};

