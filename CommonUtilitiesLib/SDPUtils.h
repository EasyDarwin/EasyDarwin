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
  File:       SDPUtils.h

  Contains:   Some static routines for dealing with SDPs


  */

#ifndef __SDPUtilsH__
#define __SDPUtilsH__

#include "OS.h"
#include "StrPtrLen.h"
#include "ResizeableStringFormatter.h"
#include "StringParser.h"
#include "OSMemory.h"

class SDPLine : public StrPtrLen
{
public:
	SDPLine() {}
	virtual ~SDPLine() {}

	char    GetHeaderType() { if (Ptr && Len) return this->Ptr[0]; return 0; }
};

class SDPContainer
{
	enum { kBaseLines = 20, kLineTypeArraySize = 256 };

	enum {
		kVPos = 0,
		kSPos,
		kTPos,
		kOPos
	};

	enum {
		kV = 1 << kVPos,
		kS = 1 << kSPos,
		kT = 1 << kTPos,
		kO = 1 << kOPos,
		kAllReq = kV | kS | kT | kO
	};


public:

	SDPContainer(UInt32 numStrPtrs = SDPContainer::kBaseLines) :
		fNumSDPLines(numStrPtrs),
		fSDPLineArray(NULL)
	{
		Initialize();
	}

	~SDPContainer() { delete[] fSDPLineArray; }
	void		Initialize();
	SInt32      AddHeaderLine(StrPtrLen *theLinePtr);
	SInt32      FindHeaderLineType(char id, SInt32 start);
	SDPLine*    GetNextLine();
	SDPLine*    GetLine(SInt32 lineIndex);
	void        SetLine(SInt32 index);
	void        Parse();
	Bool16      SetSDPBuffer(char *sdpBuffer);
	Bool16      SetSDPBuffer(StrPtrLen *sdpBufferPtr);
	Bool16      IsSDPBufferValid() { return fValid; }
	Bool16      HasReqLines() { return (Bool16)(fReqLines == kAllReq); }
	Bool16      HasLineType(char lineType) { return (Bool16)(lineType == fFieldStr[(UInt8)lineType]); }
	char*       GetReqLinesArray;
	void        PrintLine(SInt32 lineIndex);
	void        PrintAllLines();
	SInt32      GetNumLines() { return  fNumUsedLines; }

	SInt32      fCurrentLine;
	SInt32      fNumSDPLines;
	SInt32      fNumUsedLines;
	SDPLine*    fSDPLineArray;
	Bool16      fValid;
	StrPtrLen   fSDPBuffer;
	UInt16      fReqLines;

	char        fFieldStr[kLineTypeArraySize]; // 
	char*       fLineSearchTypeArray;
};



class SDPLineSorter {

public:
	SDPLineSorter() : fSessionLineCount(0), fSDPSessionHeaders(NULL, 0), fSDPMediaHeaders(NULL, 0) {};
	SDPLineSorter(SDPContainer *rawSDPContainerPtr, Float32 adjustMediaBandwidthPercent = 1.0, SDPContainer *insertMediaLinesArray = NULL);

	StrPtrLen* GetSessionHeaders() { return &fSessionHeaders; }
	StrPtrLen* GetMediaHeaders() { return &fMediaHeaders; }
	char* GetSortedSDPCopy();
	Bool16 ValidateSessionHeader(StrPtrLen *theHeaderLinePtr);


	StrPtrLen fullSDPBuffSPL;
	SInt32 fSessionLineCount;
	SDPContainer fSessionSDPContainer;
	ResizeableStringFormatter fSDPSessionHeaders;
	ResizeableStringFormatter fSDPMediaHeaders;
	StrPtrLen fSessionHeaders;
	StrPtrLen fMediaHeaders;
	static char sSessionOrderedLines[];// = "vosiuepcbtrzka"; // chars are order dependent: declared by rfc 2327
	static char sessionSingleLines[];//  = "vosiuepcbzk";    // return only 1 of each of these session field types
	static StrPtrLen sEOL;
	static StrPtrLen sMaxBandwidthTag;
};


#endif

