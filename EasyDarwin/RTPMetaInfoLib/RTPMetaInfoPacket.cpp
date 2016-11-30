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
 //
 // RTPMetaInfo.h:
 //   Some defs for RTP-Meta-Info payloads.


#include "RTPMetaInfoPacket.h"
#include "MyAssert.h"
#include "StringParser.h"
#include "OS.h"
#include <string.h>

#ifndef __Win32__
#include <netinet/in.h>
#endif

const RTPMetaInfoPacket::FieldName RTPMetaInfoPacket::kFieldNameMap[] =
{
	TW0_CHARS_TO_INT('p', 'p'),
	TW0_CHARS_TO_INT('t', 't'),
	TW0_CHARS_TO_INT('f', 't'),
	TW0_CHARS_TO_INT('p', 'n'),
	TW0_CHARS_TO_INT('s', 'q'),
	TW0_CHARS_TO_INT('m', 'd')
};

const UInt32 RTPMetaInfoPacket::kFieldLengthValidator[] =
{
	8,  //pp
	8,  //tt
	2,  //ft
	8,  //pn
	2,  //sq
	0,  //md
	0   //illegal / unknown
};


RTPMetaInfoPacket::FieldIndex RTPMetaInfoPacket::GetFieldIndexForName(FieldName inName)
{
	for (int x = 0; x < kNumFields; x++)
	{
		if (inName == kFieldNameMap[x])
			return x;
	}
	return kIllegalField;
}

void RTPMetaInfoPacket::ConstructFieldIDArrayFromHeader(StrPtrLen* inHeader, FieldID* ioFieldIDArray)
{
	for (UInt32 x = 0; x < kNumFields; x++)
		ioFieldIDArray[x] = kFieldNotUsed;

	//
	// Walk through the fields in this header
	StringParser theParser(inHeader);

	UInt16 fieldNameValue = 0;

	while (theParser.GetDataRemaining() > 0)
	{
		StrPtrLen theFieldP;
		(void)theParser.GetThru(&theFieldP, ';');

		//
		// Corrupt or something... just bail
		if (theFieldP.Len < 2)
			break;

		//
		// Extract the Field Name and convert it to a Field Index
		::memcpy(&fieldNameValue, theFieldP.Ptr, sizeof(UInt16));
		FieldIndex theIndex = RTPMetaInfoPacket::GetFieldIndexForName(ntohs(fieldNameValue));
		//      FieldIndex theIndex = RTPMetaInfoPacket::GetFieldIndexForName(ntohs(*(UInt16*)theFieldP.Ptr));

				//
				// Get the Field ID if there is one.
		FieldID theID = kUncompressed;
		if (theFieldP.Len > 3)
		{
			StringParser theIDExtractor(&theFieldP);
			theIDExtractor.ConsumeLength(NULL, 3);
			theID = theIDExtractor.ConsumeInteger(NULL);
		}

		if (theIndex != kIllegalField)
			ioFieldIDArray[theIndex] = theID;
	}
}


Bool16 RTPMetaInfoPacket::ParsePacket(UInt8* inPacketBuffer, UInt32 inPacketLen, FieldID* inFieldIDArray)
{
	UInt8* theFieldP = inPacketBuffer + 12; // skip RTP header
	UInt8* theEndP = inPacketBuffer + inPacketLen;

	SInt64 sInt64Val = 0;
	UInt16 uInt16Val = 0;

	while (theFieldP < (theEndP - 2))
	{
		FieldIndex theFieldIndex = kIllegalField;
		UInt32 theFieldLen = 0;
		FieldName* theFieldName = (FieldName*)theFieldP;

		if (*theFieldName & 0x8000)
		{
			Assert(inFieldIDArray != NULL);

			// If this is a compressed field, find to which field the ID maps
			UInt8 theFieldID = *theFieldP & 0x7F;

			for (int x = 0; x < kNumFields; x++)
			{
				if (theFieldID == inFieldIDArray[x])
				{
					theFieldIndex = x;
					break;
				}
			}

			theFieldLen = *(theFieldP + 1);
			theFieldP += 2;
		}
		else
		{
			// This is not a compressed field. Make sure there is enough room
			// in the packet for this to make sense
			if (theFieldP >= (theEndP - 4))
				break;

			::memcpy(&uInt16Val, theFieldP, sizeof(uInt16Val));
			theFieldIndex = this->GetFieldIndexForName(ntohs(uInt16Val));

			::memcpy(&uInt16Val, theFieldP + 2, sizeof(uInt16Val));
			theFieldLen = ntohs(uInt16Val);
			theFieldP += 4;
		}

		//
		// Validate the length of this field if possible.
		// If the field is of the wrong length, return an error.
		if ((kFieldLengthValidator[theFieldIndex] > 0) &&
			(kFieldLengthValidator[theFieldIndex] != theFieldLen))
			return false;
		if ((theFieldP + theFieldLen) > theEndP)
			return false;

		//
		// We now know what field we are dealing with, so store off
		// the proper value depending on the field
		switch (theFieldIndex)
		{
		case kPacketPosField:
			{
				::memcpy(&sInt64Val, theFieldP, sizeof(sInt64Val));
				fPacketPosition = (UInt64)OS::NetworkToHostSInt64(sInt64Val);
				break;
			}
		case kTransTimeField:
			{
				::memcpy(&sInt64Val, theFieldP, sizeof(sInt64Val));
				fTransmitTime = (UInt64)OS::NetworkToHostSInt64(sInt64Val);
				break;
			}
		case kFrameTypeField:
			{
				fFrameType = ntohs(*((FrameTypeField*)theFieldP));
				break;
			}
		case kPacketNumField:
			{
				::memcpy(&sInt64Val, theFieldP, sizeof(sInt64Val));
				fPacketNumber = (UInt64)OS::NetworkToHostSInt64(sInt64Val);
				break;
			}
		case kSeqNumField:
			{

				::memcpy(&uInt16Val, theFieldP, sizeof(uInt16Val));
				fSeqNum = ntohs(uInt16Val);
				break;
			}
		case kMediaDataField:
			{
				fMediaDataP = theFieldP;
				fMediaDataLen = theFieldLen;
				break;
			}
		default:
			break;
		}

		//
		// Skip onto the next field
		theFieldP += theFieldLen;
	}
	return true;
}

UInt8* RTPMetaInfoPacket::MakeRTPPacket(UInt32* outPacketLen)
{
	if (fMediaDataP == NULL)
		return NULL;

	//
	// Just move the RTP header to right before the media data.
	::memmove(fMediaDataP - 12, fPacketBuffer, 12);

	//
	// Report the length of the resulting RTP packet 
	if (outPacketLen != NULL)
		*outPacketLen = fMediaDataLen + 12;

	return fMediaDataP - 12;
}


