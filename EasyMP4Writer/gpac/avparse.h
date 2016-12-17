/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005 
 *					All rights reserved
 *
 *  This file is part of GPAC / Authoring Tools sub-project
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

#ifndef _GF_PARSERS_AV_H_
#define _GF_PARSERS_AV_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/bitstream.h>

/*basic MPEG (1,2,4) visual object parser (DSI extraction and timing/framing)*/
typedef struct
{
	/*video PL*/
	u8 VideoPL;
	u8 RAP_stream, objectType, has_shape, enh_layer;
	/*video resolution*/
	u16 width, height;
	/*pixel aspect ratio*/
	u8 par_num, par_den;

	u16 clock_rate;
	u8 NumBitsTimeIncrement;
	u32 time_increment;
	/*for MPEG 1/2*/
	Double fps;
} GF_M4VDecSpecInfo;


typedef struct __tag_m4v_parser GF_M4VParser;

#ifndef GPAC_DISABLE_AV_PARSERS

GF_M4VParser *gf_m4v_parser_new(char *data, u64 data_size, Bool mpeg12video);
GF_M4VParser *gf_m4v_parser_bs_new(GF_BitStream *bs, Bool mpeg12video);
void gf_m4v_parser_del(GF_M4VParser *m4v);
GF_Err gf_m4v_parse_config(GF_M4VParser *m4v, GF_M4VDecSpecInfo *dsi);

/*get a frame (can contain GOP). The parser ALWAYS resync on the next object in the bitstream
thus you can seek the bitstream to copy the payload without re-seeking it */
GF_Err gf_m4v_parse_frame(GF_M4VParser *m4v, GF_M4VDecSpecInfo dsi, u8 *frame_type, u32 *time_inc, u64 *size, u64 *start, Bool *is_coded);
/*returns current object start in bitstream*/
u64 gf_m4v_get_object_start(GF_M4VParser *m4v);
/*returns 1 if current object is a valid MPEG-4 Visual object*/
Bool gf_m4v_is_valid_object_type(GF_M4VParser *m4v);
/*decodes DSI*/
GF_Err gf_m4v_get_config(char *rawdsi, u32 rawdsi_size, GF_M4VDecSpecInfo *dsi);
/*rewrites PL code in DSI*/
void gf_m4v_rewrite_pl(char **io_dsi, u32 *io_dsi_len, u8 PL);
/*rewrites PAR code in DSI. Negative values will remove the par*/
GF_Err gf_m4v_rewrite_par(char **o_data, u32 *o_dataLen, s32 par_n, s32 par_d);

#endif /*GPAC_DISABLE_AV_PARSERS*/

/*returns readable description of profile*/
const char *gf_m4v_get_profile_name(u8 video_pl);

#ifndef GPAC_DISABLE_AV_PARSERS
s32 gf_mv12_next_start_code(unsigned char *pbuffer, u32 buflen, u32 *optr, u32 *scode);
s32 gf_mv12_next_slice_start(unsigned char *pbuffer, u32 startoffset, u32 buflen, u32 *slice_offset);

#endif /* GPAC_DISABLE_AV_PARSERS*/

#ifndef GPAC_DISABLE_AV_PARSERS

/*MP3 tools*/
u8 gf_mp3_num_channels(u32 hdr);
u16 gf_mp3_sampling_rate(u32 hdr);
u16 gf_mp3_window_size(u32 hdr);
u16 gf_mp3_bit_rate(u32 hdr);
u8 gf_mp3_object_type_indication(u32 hdr);
u8 gf_mp3_layer(u32 hdr);
u16 gf_mp3_frame_size(u32 hdr);
u32 gf_mp3_get_next_header(FILE* in);
u32 gf_mp3_get_next_header_mem(const char *buffer, u32 size, u32 *pos);

#endif /*GPAC_DISABLE_AV_PARSERS*/

u8 gf_mp3_version(u32 hdr);
const char *gf_mp3_version_name(u32 hdr);



#if !defined(GPAC_DISABLE_AV_PARSERS) && !defined (GPAC_DISABLE_OGG)

/*vorbis tools*/
typedef struct
{
	u32 sample_rate, channels, version;
	s32 max_r, avg_r, low_r;
	u32 min_block, max_block;

	/*do not touch, parser private*/
	Bool is_init;
	u32 modebits;
	Bool mode_flag[64];
} GF_VorbisParser;

/*call with vorbis header packets - you MUST initialize the structure to 0 before!!
returns 1 if success, 0 if error.*/
Bool gf_vorbis_parse_header(GF_VorbisParser *vp, char *data, u32 data_len);
/*returns 0 if init error or not a vorbis frame, otherwise returns the number of audio samples
in this frame*/
u32 gf_vorbis_check_frame(GF_VorbisParser *vp, char *data, u32 data_length);

#endif /*!defined(GPAC_DISABLE_AV_PARSERS) && !defined (GPAC_DISABLE_OGG)*/


enum
{
    GF_M4A_AAC_MAIN = 1,
    GF_M4A_AAC_LC = 2,
    GF_M4A_AAC_SSR = 3,
    GF_M4A_AAC_LTP = 4,
    GF_M4A_AAC_SBR = 5,
    GF_M4A_AAC_SCALABLE = 6,
    GF_M4A_TWINVQ = 7,
    GF_M4A_CELP = 8, 
    GF_M4A_HVXC = 9,
    GF_M4A_TTSI = 12,
    GF_M4A_MAIN_SYNTHETIC = 13,
    GF_M4A_WAVETABLE_SYNTHESIS = 14,
    GF_M4A_GENERAL_MIDI = 15,
    GF_M4A_ALGO_SYNTH_AUDIO_FX = 16,
    GF_M4A_ER_AAC_LC = 17,
    GF_M4A_ER_AAC_LTP = 19,
    GF_M4A_ER_AAC_SCALABLE = 20,
    GF_M4A_ER_TWINVQ = 21,
    GF_M4A_ER_BSAC = 22,
    GF_M4A_ER_AAC_LD = 23,
    GF_M4A_ER_CELP = 24,
    GF_M4A_ER_HVXC = 25,
    GF_M4A_ER_HILN = 26,
    GF_M4A_ER_PARAMETRIC = 27,
    GF_M4A_SSC = 28,
    GF_M4A_AAC_PS = 29,
    GF_M4A_LAYER1 = 32,
    GF_M4A_LAYER2 = 33,
    GF_M4A_LAYER3 = 34,
    GF_M4A_DST = 35,
    GF_M4A_ALS = 36
};

#ifndef GPAC_DISABLE_AV_PARSERS

static const u32 GF_M4ASampleRates[] =
{
    96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 
	16000, 12000, 11025, 8000, 7350, 0, 0, 0
};

/*get Audio type from dsi. return audio codec type:*/
typedef struct
{
	u32 nb_chan;
	u32 base_object_type, base_sr, base_sr_index;
	/*SBR*/
	Bool has_sbr;
	u32 sbr_object_type, sbr_sr, sbr_sr_index;
	/*PS*/
	Bool has_ps;
	/*PL indication*/
	u8 audioPL;
} GF_M4ADecSpecInfo;
/*parses dsi and updates audioPL*/
GF_Err gf_m4a_get_config(char *dsi, u32 dsi_size, GF_M4ADecSpecInfo *cfg);
/*gets audioPL for given cfg*/
u32 gf_m4a_get_profile(GF_M4ADecSpecInfo *cfg);

GF_Err gf_m4a_write_config(GF_M4ADecSpecInfo *cfg, char **dsi, u32 *dsi_size);
GF_Err gf_m4a_write_config_bs(GF_BitStream *bs, GF_M4ADecSpecInfo *cfg);
GF_Err gf_m4a_parse_config(GF_BitStream *bs, GF_M4ADecSpecInfo *cfg, Bool size_known);

#endif /*GPAC_DISABLE_AV_PARSERS*/

const char *gf_m4a_object_type_name(u32 objectType);
const char *gf_m4a_get_profile_name(u8 audio_pl);

#ifndef GPAC_DISABLE_AV_PARSERS


typedef struct
{
	u32 bitrate;
	u32 sample_rate;
	u32 framesize;
	u32 channels;
	/*only set if full parse*/
	u8 fscod, bsid, bsmod, acmod, lfon, brcode;
} GF_AC3Header;

Bool gf_ac3_parser(u8 *buffer, u32 buffer_size, u32 *pos, GF_AC3Header *out_hdr, Bool full_parse);
Bool gf_ac3_parser_bs(GF_BitStream *bs, GF_AC3Header *hdr, Bool full_parse);
u32 gf_ac3_get_channels(u32 acmod);
u32 gf_ac3_get_bitrate(u32 brcode);

GF_Err gf_avc_get_sps_info(char *sps, u32 sps_size, u32 *sps_id, u32 *width, u32 *height, s32 *par_n, s32 *par_d);
const char *gf_avc_get_profile_name(u8 video_prof);

#endif /*GPAC_DISABLE_AV_PARSERS*/



/*gets image size (bs must contain the whole image) 
@OTI: image type (JPEG=0x6C, PNG=0x6D)
@width, height: image resolution - for jpeg max size if thumbnail included*/
void gf_img_parse(GF_BitStream *bs, u8 *OTI, u32 *mtype, u32 *width, u32 *height, char **dsi, u32 *dsi_len);

GF_Err gf_img_jpeg_dec(char *jpg, u32 jpg_size, u32 *width, u32 *height, u32 *pixel_format, char *dst, u32 *dst_size, u32 dst_nb_comp);

GF_Err gf_img_png_dec(char *png, u32 png_size, u32 *width, u32 *height, u32 *pixel_format, char *dst, u32 *dst_size);
GF_Err gf_img_png_file_dec(char *png_file, u32 *width, u32 *height, u32 *pixel_format, char **dst, u32 *dst_size);
GF_Err gf_img_png_enc(char *data, u32 width, u32 height, s32 stride, u32 pixel_format, char *dst, u32 *dst_size);

#ifdef __cplusplus
}
#endif


#endif	/*_GF_PARSERS_AV_H_*/

