/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Authors: Jean le Feuvre
 *				Copyright (c) 2005-200X ENST
 *					All rights reserved
 *
 *  This file is part of GPAC / Elementary Stream Interface sub-project
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

#ifndef _GF_ESI_H_
#define _GF_ESI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/tools.h>

/* ESI input control commands*/
enum
{
	/*forces a data flush from interface to dest (caller) - used for non-threaded interfaces
		corresponding parameter: unused
	*/
	GF_ESI_INPUT_DATA_FLUSH,
	/*pulls a COMPLETE AU from the stream
		corresponding parameter: pointer to a GF_ESIPacket to fill. The indut data_len in the packet is used to indicate any padding in bytes
	*/
	GF_ESI_INPUT_DATA_PULL,
	/*releases the currently pulled AU from the stream - AU cannot be pulled after that, unless seek happens
		corresponding parameter: unused
	*/
	GF_ESI_INPUT_DATA_RELEASE,

	/*destroys any allocated resource by the stream interface*/
	GF_ESI_INPUT_DESTROY,
};

/* ESI output control commands*/
enum
{
	/*forces a data flush from interface to dest (caller) - used for non-threaded interfaces
		corresponding parameter: unused
	*/
	GF_ESI_OUTPUT_DATA_DISPATCH
};
	
/*
	data packet flags
*/
enum
{
	GF_ESI_DATA_AU_START	=	1,
	GF_ESI_DATA_AU_END		=	1<<1,
	GF_ESI_DATA_AU_RAP		=	1<<2,
	GF_ESI_DATA_HAS_CTS		=	1<<3,
	GF_ESI_DATA_HAS_DTS		=	1<<4,
	GF_ESI_DATA_REPEAT		=	1<<5,
	GF_ESI_DATA_CRITICAL	=	1<<6,
	GF_ESI_DATA_ENCRYPTED	=	1<<7,
};

typedef struct __data_packet_ifce
{
	u32 flags;
	char *data;
	u32 data_len;
	/*DTS, CTS/PTS and duration expressed in media timescale*/
	u64 dts, cts;
	u32 duration;
	u32 pck_sn;
	/*MPEG-4 stuff*/
	u32 au_sn;
	/*for packets using ISMACrypt/OMA/3GPP based crypto*/
	u32 isma_bso;
} GF_ESIPacket;

struct __esi_video_info
{
	u32 width, height, par;
	Double FPS;
};
struct __esi_audio_info
{
	u32 sample_rate, nb_channels;
};

enum
{
	/*data can be pulled from this stream*/
	GF_ESI_AU_PULL_CAP	=	1,
	/*DTS is signaled for this stream*/
	GF_ESI_SIGNAL_DTS =	1<<1,
	/*no more data to expect from this stream*/
	GF_ESI_STREAM_IS_OVER	=	1<<2,
};

typedef struct __elementary_stream_ifce 
{
	/*misc caps of the stream*/
	u32 caps;
	/*matches PID for MPEG2, ES_ID for MPEG-4*/
	u32 stream_id;
	/*MPEG-TS program number if any*/
	u16 program_number;
	/*MPEG-4 ST/OTIs*/
	u8 stream_type;
	u8 object_type_indication;
	/*stream 4CC for non-mpeg codecs, 0 otherwise (stream is identified through StreamType/ObjectType)*/
	u32 fourcc;
	/*packed 3-char language code (4CC with last byte ' ')*/
	u32 lang;
	/*media timescale*/
	u32 timescale;
	/*duration in ms - 0 if unknown*/
	Double duration;
	/*average bit rate in bit/sec - 0 if unknown*/
	u32 bit_rate;
	/*repeat rate in ms for carrouseling - 0 if no repeat*/
	u32 repeat_rate;
	
	char *decoder_config;
	u32 decoder_config_size;

	struct __esi_video_info info_video;
	struct __esi_audio_info info_audio;

	/*input ES control from caller*/
	GF_Err (*input_ctrl)(struct __elementary_stream_ifce *_self, u32 ctrl_type, void *param);
	/*input user data of interface - usually set by interface owner*/
	void *input_udta;

	/*output ES control of destination*/
	GF_Err (*output_ctrl)(struct __elementary_stream_ifce *_self, u32 ctrl_type, void *param);
	/*output user data of interface - usually set during interface setup*/
	void *output_udta;

} GF_ESInterface;

typedef struct __service_ifce
{
	u32 type;

	/*input service control from caller*/
	GF_Err (*input_ctrl)(struct __service_ifce *_self, u32 ctrl_type, void *param);
	/*input user data of interface - usually set by interface owner*/
	void *input_udta;

	/*output service control of destination*/
	GF_Err (*output_ctrl)(struct __service_ifce *_self, u32 ctrl_type, void *param);
	/*output user data of interface - usually set during interface setup*/
	void *output_udta;

	GF_ESInterface **streams;
	u32 nb_streams;
} GF_ServiceInterface;


typedef struct __data_io
{
	u32 (*read)(struct __data_io *_self, char *buffer, u32 nb_bytes);
	u32 (*write)(struct __data_io *_self, char *buffer, u32 nb_bytes);
	void *udta;
} GF_DataIO;


#ifdef __cplusplus
}
#endif

#endif	//_GF_ESI_H_

