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
 // QTHintTrack:
 //   The central point of control for a hint track in a QTFile.

#ifndef QTHintTrack_H
#define QTHintTrack_H


//
// Includes
#include "QTTrack.h"
#include "QTAtom_hinf.h"
#include "QTAtom_tref.h"
#include "RTPMetaInfoPacket.h"
#include "MyAssert.h"


//
// External classes
class QTFile;
class QTAtom_stsc_SampleTableControlBlock;
class QTAtom_stts_SampleTableControlBlock;


class QTHintTrackRTPHeaderData {

public:
	UInt16      rtpHeaderBits;
	UInt16      rtpSequenceNumber;
	SInt32      relativePacketTransmissionTime;
	UInt16      hintFlags;
	UInt16      dataEntryCount;
	UInt32      tlvSize;
	SInt32      tlvTimestampOffset; //'rtpo' TLV which is the timestamp offset for this packet
};

//
// Class state cookie
class QTHintTrack_HintTrackControlBlock {

public:
	//
	// Constructor and destructor.
	QTHintTrack_HintTrackControlBlock(QTFile_FileControlBlock * FCB = NULL);
	virtual             ~QTHintTrack_HintTrackControlBlock();

	//
	// If you are moving around randomly (seeking), you should call this to reset
	// caches
	void                Reset();

	//
	// If you want this HTCB to build RTP Meta Info packets,
	// tell it which fields to add, and also which IDs to assign, by passing
	// in an array of RTPMetaInfoPacket::kNumFields size, with all the right info
	void SetupRTPMetaInfo(RTPMetaInfoPacket::FieldID* inFieldArray, Bool16 isVideo)
	{
		Assert(fRTPMetaInfoFieldArray == NULL); fRTPMetaInfoFieldArray = inFieldArray;
		fIsVideo = isVideo;
	}

	//
	// File control block
	QTFile_FileControlBlock *fFCB;

	//
	// Sample Table control blocks
	QTAtom_stsc_SampleTableControlBlock  fstscSTCB;
	QTAtom_stts_SampleTableControlBlock  fsttsSTCB;

	//
	// Sample cache
	UInt32              fCachedSampleNumber;
	char *              fCachedSample;
	UInt32              fCachedSampleSize, fCachedSampleLength;

	//
	// Sample (description) cache
	UInt32              fCachedHintTrackSampleNumber, fCachedHintTrackSampleOffset;
	char *              fCachedHintTrackSample;
	UInt32              fCachedHintTrackSampleLength;
	UInt32              fCachedHintTrackBufferLength;

	UInt16              fLastPacketNumberFetched;   // for optimizing Getting a packet from a cached sample
	char*               fPointerToNextPacket;       // after we get one, we point the next at this...

	//
	// To support RTP-Meta-Info payload
	RTPMetaInfoPacket::FieldID*         fRTPMetaInfoFieldArray;
	UInt32                              fSyncSampleCursor; // Where are we in the sync sample table?
	Bool16                              fIsVideo; // so that we know what to do with the frame type field
	UInt64              fCurrentPacketNumber;
	UInt64              fCurrentPacketPosition;

	SInt32              fMediaTrackRefIndex;
	QTAtom_stsc_SampleTableControlBlock * fMediaTrackSTSC_STCB;

};


//
// QTHintTrack class
class QTHintTrack : public QTTrack {

public:
	//
	// Constructors and destructor.
	QTHintTrack(QTFile * File, QTFile::AtomTOCEntry * trakAtom,
		Bool16 Debug = false, Bool16 DeepDebug = false);
	virtual             ~QTHintTrack();


	//
	// Initialization functions.
	virtual ErrorCode   Initialize();

	Bool16              IsHintTrackInitialized() { return fHintTrackInitialized; }

	//
	// Accessors.
	ErrorCode   GetSDPFileLength(int * Length);
	char *      GetSDPFile(int * Length);

	inline  UInt64      GetTotalRTPBytes() { return fHintInfoAtom ? fHintInfoAtom->GetTotalRTPBytes() : 0; }
	inline  UInt64      GetTotalRTPPackets() { return fHintInfoAtom ? fHintInfoAtom->GetTotalRTPPackets() : 0; }

	inline  UInt32      GetFirstRTPTimestamp() { return fFirstRTPTimestamp; }
	inline  void        SetAllowInvalidHintRefs(Bool16 inAllowInvalidHintRefs) { fAllowInvalidHintRefs = inAllowInvalidHintRefs; }

	//
	// Sample functions
	Bool16      GetSamplePtr(UInt32 SampleNumber, char ** Buffer, UInt32 * Length,
		QTHintTrack_HintTrackControlBlock * HTCB);

	//
	// Packet functions
	inline  UInt32      GetRTPTimescale() { return fRTPTimescale; }

	inline  UInt32      GetRTPTimestampRandomOffset() { return fTimestampRandomOffset; }

	inline  UInt16      GetRTPSequenceNumberRandomOffset() { return fSequenceNumberRandomOffset; }

	ErrorCode   GetNumPackets(UInt32 SampleNumber, UInt16 * NumPackets,
		QTHintTrack_HintTrackControlBlock * HTCB = NULL);

	//
	// This function will build an RTP-Meta-Info packet if the last argument
	// is non-NULL. Some caveats apply to maximize performance of this operation:
	//
	// 1.   If the "md" (media data) field is desired, please put it at the end.
	//
	// 2.   If you want to use compressed fields, pass in the field ID in the first
	//      byte of the TwoCharConst. Also set the high bit to indicate that this
	//      is a compressed field ID.
	//
	// Supported fields: tt, md, ft, pp, pn, sq
	ErrorCode   GetPacket(UInt32 SampleNumber, UInt16 PacketNumber,
		char * Buffer, UInt32 * Length,
		Float64 * TransmitTime,
		Bool16 dropBFrames,
		Bool16 dropRepeatPackets = false,
		UInt32 SSRC = 0,
		QTHintTrack_HintTrackControlBlock * HTCB = NULL);

	inline ErrorCode    GetSampleData(QTHintTrack_HintTrackControlBlock * htcb, char **buffPtr, char **ppPacketBufOut, UInt32 sampleNumber, UInt16 packetNumber, UInt32 buffOutLen);

	//
	// Debugging functions.
	virtual void        DumpTrack();

	// only reliable after all of the packets have been played
	// any hint packet may reference another media track and we don't know until all have been played.
	inline SInt16 GetHintTrackType() { return fHintType; }

protected:

	enum
	{
		kRepeatPacketMask = 0x0001,
		kBFrameBitMask = 0x0002
	};

	enum
	{
		kUnknown = 0,
		kOptimized = -1,
		kUnoptimized = 1
	};

	enum
	{
		kMaxHintTrackRefs = 1024
	};

	//
	// Protected member variables.
	QTAtom_hinf         *fHintInfoAtom;
	QTAtom_tref         *fHintTrackReferenceAtom;

	QTTrack             **fTrackRefs;

	UInt32              fMaxPacketSize;
	UInt32              fRTPTimescale, fFirstRTPTimestamp;
	UInt32              fTimestampRandomOffset;
	UInt16              fSequenceNumberRandomOffset;
	Bool16              fHintTrackInitialized;
	SInt16              fHintType;
	Float64  			fFirstTransmitTime;
	Bool16              fAllowInvalidHintRefs;
	//
	// Used by GetPacket for RTP-Meta-Info payload stuff
	void                WriteMetaInfoField(RTPMetaInfoPacket::FieldIndex inFieldIndex,
		RTPMetaInfoPacket::FieldID inFieldID,
		void* inFieldData, UInt32 inFieldLen, char** ioBuffer);

	inline QTTrack::ErrorCode   GetSamplePacketPtr(char ** samplePacketPtr, UInt32 sampleNumber, UInt16 packetNumber, QTHintTrackRTPHeaderData &hdrData, QTHintTrack_HintTrackControlBlock & htcb);
	inline void         GetSamplePacketHeaderVars(char *samplePacketPtr, char *maxBuffPtr, QTHintTrackRTPHeaderData &hdrData);
};

#endif // QTHintTrack_H
