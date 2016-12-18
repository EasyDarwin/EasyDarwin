/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005 
 *					All rights reserved
 *
 *  This file is part of GPAC / SL header file
 *
 *  GPAC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  GPAC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *   
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */

#ifndef _GF_SYNC_LAYER_H_
#define _GF_SYNC_LAYER_H_

#ifdef __cplusplus
extern "C" {
#endif

	
/*the Sync Layer config descriptor*/
typedef struct
{
	/*base descriptor*/
	u8 tag;

	u8 predefined;
	u8 useAccessUnitStartFlag;
	u8 useAccessUnitEndFlag;
	u8 useRandomAccessPointFlag;
	u8 hasRandomAccessUnitsOnlyFlag;
	u8 usePaddingFlag;
	u8 useTimestampsFlag;
	u8 useIdleFlag;
	u8 durationFlag;
	u32 timestampResolution;
	u32 OCRResolution;
	u8 timestampLength;
	u8 OCRLength;
	u8 AULength;
	u8 instantBitrateLength;
	u8 degradationPriorityLength;
	u8 AUSeqNumLength;
	u8 packetSeqNumLength;
	u32 timeScale;
	u16 AUDuration;
	u16 CUDuration;
	u64 startDTS;
	u64 startCTS;
} GF_SLConfig;

/***************************************
			SLConfig Tag
***************************************/
enum
{
	SLPredef_Null = 0x01,
	SLPredef_MP4 = 0x02,
	/*intern to GPAC, means NO SL at all (for streams unable to handle AU reconstruction a timing)*/
	SLPredef_SkipSL = 0xF0
};

/*set SL predefined (assign all fields according to sl->predefined value)*/
GF_Err gf_odf_slc_set_pref(GF_SLConfig *sl);


typedef struct
{
	u8 accessUnitStartFlag;
	u8 accessUnitEndFlag;
	u8 paddingFlag;
	u8 randomAccessPointFlag;
	u8 OCRflag;
	u8 idleFlag;
	u8 decodingTimeStampFlag;
	u8 compositionTimeStampFlag;
	u8 instantBitrateFlag;
	u8 degradationPriorityFlag;

	u8 paddingBits;
	u16 packetSequenceNumber;
	u64 objectClockReference;
	u16 AU_sequenceNumber;
	u64 decodingTimeStamp;
	u64 compositionTimeStamp;
	u16 accessUnitLength;
	u32 instantBitrate;
	u16 degradationPriority;

	/*this is NOT part of standard SL, only used internally: signals duration of access unit if known
	this is usefull for streams with very random updates, to prevent buffering for instance a subtitle stream
	which is likely to have no updates during the first minutes... expressed in media timescale*/
	u32 au_duration;
	/*ISMACryp extensions*/
	u8 isma_encrypted;
	u64 isma_BSO;
	/*version_number are pushed from m2ts sections to the mpeg4sl layer so as to handle mpeg4 stream dependencies*/
	u8 m2ts_version_number_plus_one;
	u8 m2ts_pcr;
} GF_SLHeader;


/*packetize SL-PDU. If PDU is NULL or size 0, only writes the SL header*/
void gf_sl_packetize(GF_SLConfig* slConfig, GF_SLHeader *Header, char *PDU, u32 size, char **outPacket, u32 *OutSize);
/*gets SL header size in bytes*/
u32 gf_sl_get_header_size(GF_SLConfig* slConfig, GF_SLHeader *Header);

/*depacketize SL-PDU*/
void gf_sl_depacketize(GF_SLConfig *slConfig, GF_SLHeader *Header, const char *PDU, u32 PDULength, u32 *HeaderLen);


#ifdef __cplusplus
}
#endif

#endif	/*_GF_SYNC_LAYER_H_*/
