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
	 File:       RCFSourceInfo.cpp

	 Contains:   Implementation of object defined in .h file

	 Copyright:  ?1998 by Apple Computer, Inc., all rights reserved.



 */

#include "RCFSourceInfo.h"
#include "OSMemory.h"
#include "SocketUtils.h"

void RCFSourceInfo::SetName(const char* inName)
{
	if (inName != NULL)
	{
		fName = NEW char[::strlen(inName) + 1];
		::strcpy(fName, inName);
	}
}

RCFSourceInfo::~RCFSourceInfo()
{
	if (fName != NULL)
		delete fName;

	// Not necessary anymore as the destructor of the base class will take care
// of deleting all allocated memory for fOutputArray and fStreamArray
/*
if (fOutputArray != NULL)
{
	for (UInt32 x = 0; x < fNumOutputs; x++)
		delete [] fOutputArray[x].fPortArray;

	char* theOutputArray = (char*)fOutputArray;
	delete [] theOutputArray;
}
if (fStreamArray != NULL)
{
	char* theStreamArray = (char*)fStreamArray;
	delete [] theStreamArray;
}
*/
}

void RCFSourceInfo::Parse(XMLTag* relayTag)
{
	XMLTag* sourceTag = relayTag->GetEmbeddedTagByNameAndAttr("OBJECT", "CLASS", "source");
	if (sourceTag == NULL)
		return;

	fNumStreams = 0;
	UInt32 destIPAddr = 0;
	UInt32 srcIPAddr = 0;
	UInt16 ttl = 0;

	XMLTag* prefTag;

	prefTag = sourceTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "in_addr");
	if (prefTag != NULL)
	{
		char* destAddrStr = prefTag->GetValue();
		if (destAddrStr != NULL)
			destIPAddr = SocketUtils::ConvertStringToAddr(destAddrStr);
	}
	prefTag = sourceTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "source_addr");
	if (prefTag != NULL)
	{
		char* srcAddrStr = prefTag->GetValue();
		if (srcAddrStr != NULL)
			srcIPAddr = SocketUtils::ConvertStringToAddr(srcAddrStr);
	}
	prefTag = sourceTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "ttl");
	if (prefTag != NULL)
	{
		char* ttlStr = prefTag->GetValue();
		if (ttlStr != NULL)
			ttl = atoi(ttlStr);
	}
	prefTag = sourceTag->GetEmbeddedTagByNameAndAttr("LIST-PREF", "NAME", "udp_ports");
	if (prefTag != NULL)
	{
		fNumStreams = prefTag->GetNumEmbeddedTags();

		// Allocate a proper sized stream array
		fStreamArray = NEW StreamInfo[fNumStreams];

		for (UInt32 x = 0; x < fNumStreams; x++)
		{
			XMLTag* portTag = prefTag->GetEmbeddedTagByName("VALUE", x);
			int port = 0;
			if (portTag != NULL)
			{
				char* portStr = portTag->GetValue();
				if (portStr != NULL)
					port = atoi(portStr);
			}

			// Setup all the StreamInfo structures
			fStreamArray[x].fSrcIPAddr = srcIPAddr;
			fStreamArray[x].fDestIPAddr = destIPAddr;
			fStreamArray[x].fPort = port;
			fStreamArray[x].fTimeToLive = ttl;
			fStreamArray[x].fPayloadType = qtssUnknownPayloadType;
			fStreamArray[x].fTrackID = x + 1;
		}
	}

	// Now go through all the relay_destination lines (starting from the next line after the
	// relay_source line.
	this->ParseRelayDestinations(relayTag);
}

void RCFSourceInfo::ParseRelayDestinations(XMLTag* relayTag)
{
	// parse the NAME attribute of the relay tag and store it in the relayname attribute
	char* name = relayTag->GetAttributeValue("NAME");
	if (name != NULL)
	{
		fName = NEW char[::strlen(name) + 1];
		::strcpy(fName, name);
	}

	UInt32 numTags = relayTag->GetNumEmbeddedTags();
	AllocateOutputArray(numTags);   // not all these are relay tags, but most are

	// Now actually go through and figure out what to put into these OutputInfo structures,
	// based on what's on the relay_destination line
	fNumOutputs = 0;
	for (UInt32 y = 0; y < numTags; y++)
	{
		XMLTag* destTag = relayTag->GetEmbeddedTagByNameAndAttr("OBJECT", "CLASS", "destination", y);
		if (destTag == NULL)
			return;

		char* destType = destTag->GetAttributeValue("TYPE");
		if (destType == NULL)
			return;

		if (!strcmp(destType, "udp_destination"))
			ParseDestination(destTag, y);
		else if (!strcmp(destType, "announced_destination"))
			ParseAnnouncedDestination(destTag, y);

		fNumOutputs++;
	}
}

void RCFSourceInfo::ParseDestination(XMLTag* destTag, UInt32 index)
{
	XMLTag* prefTag;

	prefTag = destTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "out_addr");
	if (prefTag != NULL)
	{
		char* outAddrStr = prefTag->GetValue();
		if (outAddrStr != NULL)
			fOutputArray[index].fLocalAddr = SocketUtils::ConvertStringToAddr(outAddrStr);
	}
	prefTag = destTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "dest_addr");
	if (prefTag != NULL)
	{
		char* destAddrStr = prefTag->GetValue();
		if (destAddrStr != NULL)
			fOutputArray[index].fDestAddr = SocketUtils::ConvertStringToAddr(destAddrStr);
	}
	prefTag = destTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "ttl");
	if (prefTag != NULL)
	{
		char* ttlStr = prefTag->GetValue();
		if (ttlStr != NULL)
			fOutputArray[index].fTimeToLive = atoi(ttlStr);
	}
	prefTag = destTag->GetEmbeddedTagByNameAndAttr("LIST-PREF", "NAME", "udp_ports");
	if (prefTag != NULL)
	{
		fOutputArray[index].fNumPorts = prefTag->GetNumEmbeddedTags();

		fOutputArray[index].fPortArray = NEW UInt16[fOutputArray[index].fNumPorts];
		::memset(fOutputArray[index].fPortArray, 0, fOutputArray[index].fNumPorts * sizeof(UInt16));

		for (UInt32 x = 0; x < fOutputArray[index].fNumPorts; x++)
		{
			XMLTag* portTag = prefTag->GetEmbeddedTagByName("VALUE", x);
			if (portTag != NULL)
			{
				char* portStr = portTag->GetValue();
				if (portStr != NULL)
				{
					fOutputArray[index].fPortArray[x] = atoi(portStr);
				}
			}
		}
	}
	else
	{
		prefTag = destTag->GetEmbeddedTagByNameAndAttr("PREF", "NAME", "udp_base_port");
		if (prefTag == NULL)
			qtss_printf("Missing both 'udp_base_port' and 'udp_ports' tags.\n Cannot set up the destination!\n");
		else
		{
			char* basePortStr = prefTag->GetValue();
			if (basePortStr != NULL)
				fOutputArray[index].fBasePort = atoi(basePortStr);
		}
	}
}

void RCFSourceInfo::ParseAnnouncedDestination(XMLTag* destTag, UInt32 index)
{
	// should log some sort of error
	// can't announce without an sdp
}

void RCFSourceInfo::AllocateOutputArray(UInt32 numOutputs)
{
	// Allocate the proper number of relay outputs
	fOutputArray = NEW OutputInfo[numOutputs];
}
