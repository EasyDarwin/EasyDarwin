/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005 
 *					All rights reserved
 *
 *  This file is part of GPAC / Media Tools sub-project
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


#ifndef _GF_MEDIA_DEV_H_
#define _GF_MEDIA_DEV_H_

#include <gpac/media_tools.h>
#include <gpac/ietf.h>

#ifndef GPAC_DISABLE_ISOM
void gf_media_get_sample_average_infos(GF_ISOFile *file, u32 Track, u32 *avgSize, u32 *MaxSize, u32 *TimeDelta, u32 *maxCTSDelta, u32 *const_duration, u32 *bandwidth);
#endif


#ifndef GPAC_DISABLE_MEDIA_IMPORT
GF_Err gf_import_message(GF_MediaImporter *import, GF_Err e, char *format, ...);
#endif /*GPAC_DISABLE_MEDIA_IMPORT*/

#ifndef GPAC_DISABLE_AV_PARSERS


#define GF_SVC_SSPS_ID_SHIFT	16	

/*returns 0 if not a start code, or size of start code (3 or 4 bytes). If start code, bitstream
is positionned AFTER start code*/
u32 AVC_IsStartCode(GF_BitStream *bs);
/*returns size of chunk between current and next startcode (excluding startcode sizes), 0 if no more startcodes (eos)*/
u32 AVC_NextStartCode(GF_BitStream *bs);
/*returns NAL unit type - bitstream must be sync'ed!!*/
u8 AVC_NALUType(GF_BitStream *bs);
Bool SVC_NALUIsSlice(u8 type);


enum
{
	/*SPS has been parsed*/
	AVC_SPS_PARSED = 1,
	/*SPS has been declared to the upper layer*/
	AVC_SPS_DECLARED = 1<<1,
	/*SUB-SPS has been parsed*/
	AVC_SUBSPS_PARSED = 1<<2,
	/*SUB-SPS has been declared to the upper layer*/
	AVC_SUBSPS_DECLARED = 1<<3,
};

typedef struct 
{
	u8 cpb_removal_delay_length_minus1;
	u8 dpb_output_delay_length_minus1;
	u8 time_offset_length;
	/*to be eventually completed by other hrd members*/
} AVC_HRD;

typedef struct 
{
	s32 timing_info_present_flag;
	u32 num_units_in_tick;
	u32 time_scale;
	s32 fixed_frame_rate_flag;

	u32 par_num, par_den;

	Bool nal_hrd_parameters_present_flag;
	Bool vcl_hrd_parameters_present_flag;
	AVC_HRD hrd;

	Bool pic_struct_present_flag;
	/*to be eventually completed by other vui members*/
} AVC_VUI;

typedef struct
{
	s32 profile_idc;
	s32 level_idc;
	s32 prof_compat;
	s32 log2_max_frame_num;
	u32 poc_type, poc_cycle_length;
	s32 log2_max_poc_lsb;
	s32 delta_pic_order_always_zero_flag;
	s32 offset_for_non_ref_pic, offset_for_top_to_bottom_field;
	Bool frame_mbs_only_flag;

	s16 offset_for_ref_frame[256];

	u32 width, height;

	AVC_VUI vui;
	
	/*used to discard repeated SPSs - 0: not parsed, 1 parsed, 2 sent*/
	u32 state;

	/*for SVC stats during import*/
	u32 nb_ei, nb_ep, nb_eb;
} AVC_SPS;

typedef struct 
{
	s32 id; /* used to compare pps when storing SVC PSS */
	s32 sps_id;
	s32 pic_order_present;			/* pic_order_present_flag*/
	s32 redundant_pic_cnt_present;	/* redundant_pic_cnt_present_flag */
	u32 slice_group_count;			/* num_slice_groups_minus1 + 1*/
	/*used to discard repeated SPSs - 0: not parsed, 1 parsed, 2 sent*/
	u32 status;
} AVC_PPS;

typedef struct 
{
	s32 idr_pic_flag;
	u8 temporal_id, priority_id, dependency_id, quality_id;
} SVC_NALUHeader;

typedef struct
{
	u8 nal_ref_idc, nal_unit_type, field_pic_flag, bottom_field_flag;
	u32 frame_num, idr_pic_id, poc_lsb, slice_type;
	s32 delta_poc_bottom;
	s32 delta_poc[2];
	s32 redundant_pic_cnt;

	s32 poc;
	u32 poc_msb, poc_msb_prev, poc_lsb_prev, frame_num_prev;
	s32 frame_num_offset, frame_num_offset_prev;

	AVC_SPS *sps;
	AVC_PPS *pps;
	SVC_NALUHeader NalHeader;
} AVCSliceInfo;


typedef struct 
{
	u32 frame_cnt;
	u8 exact_match_flag;
	u8 broken_link_flag;
	u8 changing_slice_group_idc;
	u8 valid;
} AVCSeiRecoveryPoint;

typedef struct 
{
	u8 pic_struct;
	/*to be eventually completed by other pic_timing members*/
} AVCSeiPicTiming;

typedef struct 
{
	AVCSeiRecoveryPoint recovery_point;
	AVCSeiPicTiming pic_timing;
	/*to be eventually completed by other sei*/
} AVCSei;

typedef struct
{
	AVC_SPS sps[32]; /* range allowed in the spec is 0..31 */
	s8 sps_active_idx;	/*currently active sps; must be initalized to -1 in order to discard not yet decodable SEIs*/

	AVC_PPS pps[255];

	AVCSliceInfo s_info;
	AVCSei sei;

	Bool is_svc;
} AVCState;

/*return sps ID or -1 if error*/
s32 AVC_ReadSeqInfo(char *sps_data, u32 sps_size, AVCState *avc, u32 subseq_sps, u32 *vui_flag_pos);
/*return pps ID or -1 if error*/
s32 AVC_ReadPictParamSet(char *pps_data, u32 pps_size, AVCState *avc);
/*is slice a RAP*/
Bool AVC_SliceIsIDR(AVCState *avc);
/*parses NALU, updates avc state and returns:
	1 if NALU part of new frame
	0 if NALU part of prev frame
	-1 if bitstream error
*/
s32 AVC_ParseNALU(GF_BitStream *bs, u32 nal_hdr, AVCState *avc);
/*remove SEI messages not allowed in MP4*/
/*nota: 'buffer' remains unmodified but cannot be set const*/
u32 AVC_ReformatSEI_NALU(char *buffer, u32 nal_size, AVCState *avc);

#ifndef GPAC_DISABLE_ISOM
GF_Err AVC_ChangePAR(GF_AVCConfig *avcc, s32 ar_n, s32 ar_d);
GF_Err AVC_ChangeColorProp(GF_AVCConfig *avcc, s32 fullrange, s32 vidformat, s32 colorprim, s32 transfer, s32 colmatrix);
#endif

#endif /*GPAC_DISABLE_AV_PARSERS*/

typedef struct
{
	u8 rate_idx;
	u8 pck_size;
} QCPRateTable;


#if !defined(GPAC_DISABLE_ISOM) && !defined(GPAC_DISABLE_STREAMING)

GP_RTPPacketizer *gf_rtp_packetizer_create_and_init_from_file(GF_ISOFile *file, 
															  u32 TrackNum,
															  void *cbk_obj, 
															  void (*OnNewPacket)(void *cbk, GF_RTPHeader *header),
															  void (*OnPacketDone)(void *cbk, GF_RTPHeader *header),
															  void (*OnDataReference)(void *cbk, u32 payload_size, u32 offset_from_orig),
															  void (*OnData)(void *cbk, char *data, u32 data_size, Bool is_head),
															  u32 Path_MTU, 
															  u32 max_ptime, 
															  u32 default_rtp_rate, 
															  u32 flags, 
															  u8 PayloadID, 
															  Bool copy_media, 
															  u32 InterleaveGroupID, 
															  u8 InterleaveGroupPriority);

void gf_media_format_ttxt_sdp(GP_RTPPacketizer *builder, char *payload_name, char *sdpLine, GF_ISOFile *file, u32 track);

#endif

#endif		/*_GF_MEDIA_DEV_H_*/

