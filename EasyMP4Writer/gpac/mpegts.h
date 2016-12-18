/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Authors: Walid B.H - Jean Le Feuvre
 *    Copyright (c)2006-200X ENST - All rights reserved
 *
 *  This file is part of GPAC / MPEG2-TS sub-project
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


#ifndef _GF_MPEG_TS_H_
#define _GF_MPEG_TS_H_

#include <gpac/list.h>

#ifndef GPAC_DISABLE_MPEG2TS

#include <gpac/internal/odf_dev.h>
#include <gpac/network.h>
#include <gpac/esi.h>
#include <gpac/thread.h>
#include <time.h>

typedef struct tag_m2ts_demux GF_M2TS_Demuxer;
typedef struct tag_m2ts_es GF_M2TS_ES;
typedef struct tag_m2ts_section_es GF_M2TS_SECTION_ES;

#ifdef GPAC_HAS_LINUX_DVB
typedef struct __gf_dvb_tuner GF_Tuner;
#endif

/*Maximum number of streams in a TS*/
#define GF_M2TS_MAX_STREAMS	8192

/*Maximum number of service in a TS*/
#define GF_M2TS_MAX_SERVICES	65535

/*When ODProfileLevelIndication has this value, only scene and od streams are sl-packetized*/
#define GPAC_MAGIC_OD_PROFILE_FOR_MPEG4_SIGNALING	10

/*Maximum size of the buffer in UDP */
#define UDP_BUFFER_SIZE	0x40000

/*MPEG-2 TS Media types*/
enum
{
	GF_M2TS_VIDEO_MPEG1				= 0x01,
	GF_M2TS_VIDEO_MPEG2				= 0x02,
	GF_M2TS_AUDIO_MPEG1				= 0x03,
	GF_M2TS_AUDIO_MPEG2				= 0x04, 
	GF_M2TS_PRIVATE_SECTION			= 0x05,
	GF_M2TS_PRIVATE_DATA			= 0x06,
	GF_M2TS_MHEG					= 0x07,
	GF_M2TS_13818_1_DSMCC			= 0x08,
	GF_M2TS_H222_1					= 0x09,
	GF_M2TS_13818_6_ANNEX_A			= 0x0A,
	GF_M2TS_13818_6_ANNEX_B			= 0x0B,
	GF_M2TS_13818_6_ANNEX_C			= 0x0C,
	GF_M2TS_13818_6_ANNEX_D			= 0x0D,
	GF_M2TS_13818_1_AUXILIARY		= 0x0E,
	GF_M2TS_AUDIO_AAC				= 0x0F,
	GF_M2TS_VIDEO_MPEG4				= 0x10,
	GF_M2TS_AUDIO_LATM_AAC			= 0x11,

	GF_M2TS_SYSTEMS_MPEG4_PES		= 0x12,
	GF_M2TS_SYSTEMS_MPEG4_SECTIONS	= 0x13,

	GF_M2TS_VIDEO_H264				= 0x1B,
	GF_M2TS_VIDEO_VC1				= 0xEA,

	GF_M2TS_AUDIO_PCM				= 0x80,
	GF_M2TS_AUDIO_AC3				= 0x81,
	GF_M2TS_AUDIO_DTS				= 0x82,
	GF_M2TS_AUDIO_TRUEHD			= 0x83,
	GF_M2TS_AUDIO_EAC3				= 0x84,
	GF_M2TS_MPE_SECTIONS            = 0x90,
	GF_M2TS_SUBTITLE_DVB			= 0x100,
	
	/*internal use*/	
	GF_M2TS_AUDIO_EC3				= 0x150,
	//GF_M2TS_VIDEO_VC1				= 0x151,
	GF_M2TS_DVB_TELETEXT			= 0x152,
	GF_M2TS_DVB_VBI					= 0x153,
	GF_M2TS_DVB_SUBTITLE			= 0x154,
};
/*returns readable name for given stream type*/
const char *gf_m2ts_get_stream_name(u32 streamType);

/*PES data framing modes*/
enum
{
	/*use data framing: recompute start of AUs (data frames)*/
	GF_M2TS_PES_FRAMING_DEFAULT,
	/*don't use data framing: all packets are raw PES packets*/
	GF_M2TS_PES_FRAMING_RAW,
	/*skip pes processing: all transport packets related to this stream are discarded*/
	GF_M2TS_PES_FRAMING_SKIP,
	/*same as GF_M2TS_PES_FRAMING_SKIP but keeps internal PES buffer alive*/
	GF_M2TS_PES_FRAMING_SKIP_NO_RESET
};

/*PES packet flags*/
enum
{
	GF_M2TS_PES_PCK_RAP = 1,
	GF_M2TS_PES_PCK_AU_START = 1<<1,
	/*visual frame starting in this packet is an I frame or IDR (AVC/H264)*/
	GF_M2TS_PES_PCK_I_FRAME = 1<<2,
	/*visual frame starting in this packet is a P frame*/
	GF_M2TS_PES_PCK_P_FRAME = 1<<3,
	/*visual frame starting in this packet is a B frame*/
	GF_M2TS_PES_PCK_B_FRAME = 1<<4,
	/*Possible PCR discontinuity from this packet on*/
	GF_M2TS_PES_PCK_DISCONTINUITY = 1<<5
};

/*Events used by the MPEGTS demuxer*/
enum
{
	/*PAT has been found (service connection) - no assoctiated parameter*/
	GF_M2TS_EVT_PAT_FOUND = 0,
	/*PAT has been updated - no assoctiated parameter*/
	GF_M2TS_EVT_PAT_UPDATE,
	/*repeated PAT has been found (carousel) - no assoctiated parameter*/
	GF_M2TS_EVT_PAT_REPEAT,
	/*PMT has been found (service tune-in) - assoctiated parameter: new PMT*/
	GF_M2TS_EVT_PMT_FOUND,
	/*repeated PMT has been found (carousel) - assoctiated parameter: updated PMT*/
	GF_M2TS_EVT_PMT_REPEAT,
	/*PMT has been changed - assoctiated parameter: updated PMT*/
	GF_M2TS_EVT_PMT_UPDATE,
	/*SDT has been received - assoctiated parameter: none*/
	GF_M2TS_EVT_SDT_FOUND,
	/*repeated SDT has been found (carousel) - assoctiated parameter: none*/
	GF_M2TS_EVT_SDT_REPEAT,
	/*SDT has been received - assoctiated parameter: none*/
	GF_M2TS_EVT_SDT_UPDATE,
	/*INT has been received - assoctiated parameter: none*/
	GF_M2TS_EVT_INT_FOUND,
	/*repeated INT has been found (carousel) - assoctiated parameter: none*/
	GF_M2TS_EVT_INT_REPEAT,
	/*INT has been received - assoctiated parameter: none*/
	GF_M2TS_EVT_INT_UPDATE,
	/*PES packet has been received - assoctiated parameter: PES packet*/
	GF_M2TS_EVT_PES_PCK,
	/*PCR has been received - assoctiated parameter: PES packet with no data*/
	GF_M2TS_EVT_PES_PCR,
	/*PTS/DTS/PCR info - assoctiated parameter: PES packet with no data*/
	GF_M2TS_EVT_PES_TIMING,
	/*An MPEG-4 SL Packet has been received in a section - assoctiated parameter: SL packet */
	GF_M2TS_EVT_SL_PCK,
	/*An IP datagram has been received in a section - assoctiated parameter: IP datagram */
	GF_M2TS_EVT_IP_DATAGRAM,

	/*AAC config has been extracted - associated parameter: PES Packet with encoded M4ADecSpecInfo in its data
		THIS MUST BE CLEANED UP
	*/
	GF_M2TS_EVT_AAC_CFG,
#if 0
	/* An EIT message for the present or following event on this TS has been received */
	GF_M2TS_EVT_EIT_ACTUAL_PF,
	/* An EIT message for the schedule of this TS has been received */
	GF_M2TS_EVT_EIT_ACTUAL_SCHEDULE,
	/* An EIT message for the present or following event of an other TS has been received */
	GF_M2TS_EVT_EIT_OTHER_PF,
	/* An EIT message for the schedule of an other TS has been received */
	GF_M2TS_EVT_EIT_OTHER_SCHEDULE,
	/* A message to inform about the current date and time in the TS */
	GF_M2TS_EVT_TDT,
	/* A message to inform about the current time offset in the TS */
	GF_M2TS_EVT_TOT,
#endif
	/* A generic event message for EIT, TDT, TOT etc */
	GF_M2TS_EVT_DVB_GENERAL,
	/* MPE / MPE-FEC frame extraction and IP datagrams decryptation */
	GF_M2TS_EVT_DVB_MPE,
	/*CAT has been found (service tune-in) - assoctiated parameter: new CAT*/
	GF_M2TS_EVT_CAT_FOUND,
	/*repeated CAT has been found (carousel) - assoctiated parameter: updated CAT*/
	GF_M2TS_EVT_CAT_REPEAT,
	/*PMT has been changed - assoctiated parameter: updated PMT*/
	GF_M2TS_EVT_CAT_UPDATE,
	/*AIT has been found (carousel) */
	GF_M2TS_EVT_AIT_FOUND,

};

enum
{
	GF_M2TS_TABLE_START		= 1,
	GF_M2TS_TABLE_END		= 1<<1,
	GF_M2TS_TABLE_FOUND		= 1<<2,
	GF_M2TS_TABLE_UPDATE	= 1<<3,
	GF_M2TS_TABLE_REPEAT	= 1<<4,
};

typedef void (*gf_m2ts_section_callback)(GF_M2TS_Demuxer *ts, GF_M2TS_SECTION_ES *es, GF_List *sections, u8 table_id, u16 ex_table_id, u8 version_number, u8 last_section_number, u32 status); 

typedef struct __m2ts_demux_section
{
	unsigned char *data;
	u32 data_size;
} GF_M2TS_Section;

typedef struct __m2ts_demux_table
{
	struct __m2ts_demux_table *next;

	u8 is_init;
	u8 is_repeat;

	/*table id*/
	u8 table_id;
	u16 ex_table_id;

	/*reassembler state*/
	u8 version_number;
	u8 last_version_number;

	u8 current_next_indicator;

	u8 section_number;
	u8 last_section_number;

	GF_List *sections;

} GF_M2TS_Table;


typedef struct GF_M2TS_SectionFilter
{
	/*section reassembler*/
	s16 cc;
	/*section buffer (max 4096)*/
	char *section;
	/*current section length as indicated in section header*/
	u16 length;
	/*number of bytes received from current section*/
	u16 received;

	/*section->table aggregator*/
	GF_M2TS_Table *table;
	
	/* indicates that the section and last_section_number do not need to be checked */
	Bool process_individual;

	/* indicates that the section header with table id and extended table id ... is
	   not parsed by the TS demuxer and left for the application  */
	Bool direct_dispatch;

	gf_m2ts_section_callback process_section; 
} GF_M2TS_SectionFilter;



/*MPEG-2 TS program object*/
typedef struct 
{
	GF_List *streams;
	u32 pmt_pid;  
	u32 pcr_pid;
	u32 number;

	GF_InitialObjectDescriptor *pmt_iod;

	/*list of additional ODs found per program !! used by media importer only , refine this !! 
		this list is only created when MPEG-4 over MPEG-2 is detected
		the list AND the ODs contained in it are destroyed when destroying the demuxer
	*/
	GF_List *additional_ods;
	/*first dts found on this program - this is used by parsers, but not setup by the lib*/
	u64 first_dts;

	/* Last PCR value received for this program and associated packet number */
	u64 last_pcr_value;
	u32 last_pcr_value_pck_number;
	/* PCR value before the last received one for this program and associated packet number 
	used to compute PCR interpolation value*/
	u64 before_last_pcr_value;
	u32 before_last_pcr_value_pck_number;
} GF_M2TS_Program;

/*ES flags*/
enum
{
	/*ES is a section stream*/
	GF_M2TS_ES_IS_SECTION = 1,
	/*ES is an mpeg-4 flexmux stream*/
	GF_M2TS_ES_IS_FMC = 1<<1,
	/*ES is an mpeg-4 SL-packetized stream*/
	GF_M2TS_ES_IS_SL = 1<<2,
	/*ES is an mpeg-4 Object Descriptor SL-packetized stream*/
	GF_M2TS_ES_IS_MPEG4_OD = 1<<3,
	/*ES is a DVB MPE stream*/
	GF_M2TS_ES_IS_MPE = 1<<4,
	
	/*all flags above this mask are used by importers & co*/
	GF_M2TS_ES_STATIC_FLAGS_MASK = 0x0000FFFF,

	/*always send sections regardless of their version_number*/
	GF_M2TS_ES_SEND_REPEATED_SECTIONS = 1<<16,
	/*Flag used by importers*/
	GF_M2TS_ES_FIRST_DTS = 1<<17,
};

/*Abstract Section/PES stream object, only used for type casting*/
#define ABSTRACT_ES		\
			GF_M2TS_Program *program; \
			u32 flags; \
			u32 pid; \
			u32 stream_type; \
			u32 mpeg4_es_id; \
			GF_SLConfig *slcfg; \
			s16 component_tag; \
			void *user; \
			u64 first_dts;

struct tag_m2ts_es
{
	ABSTRACT_ES
};


typedef struct 
{
	u8 id;
	u16 pck_len;
	u8 data_alignment;
	u64 PTS, DTS;
	u8 hdr_data_len;
} GF_M2TS_PESHeader;

struct tag_m2ts_section_es
{
	ABSTRACT_ES
	GF_M2TS_SectionFilter *sec;
};			


/*******************************************************************************/
typedef struct tag_m2ts_dvb_sub
{
	char language[3];
	u8 type;
	u16 composition_page_id;
	u16 ancillary_page_id;
} GF_M2TS_DVB_Subtitling_Descriptor;

typedef struct tag_m2ts_dvb_teletext
{
	char language[3];
	u8 type;
	u8 magazine_number;
	u8 page_number;
} GF_M2TS_DVB_Teletext_Descriptor;

/*MPEG-2 TS ES object*/
typedef struct tag_m2ts_pes
{
	ABSTRACT_ES
	/*continuity counter check*/
	s16 cc;
	u32 lang;

	/*object info*/
	u32 vid_w, vid_h, vid_par, aud_sr, aud_nb_ch;
	/*user private*/


	/*mpegts lib private - do not touch :)*/
	/*PES re-assembler*/
	unsigned char *data;
	/*amount of bytes received in the current PES packet (NOT INCLUDING ANY PENDING BYTES)*/
	u32 data_len;
	/*size of the PES packet being recevied*/
	u32 pes_len;
	Bool rap;
	u64 PTS, DTS;
	u32 pes_end_packet_number;

	u32 pes_start_packet_number;
	/* PCR info related to the PES start */
	/* Last PCR value received for this program and associated packet number */
	u64 last_pcr_value;
	u32 last_pcr_value_pck_number;
	/* PCR value before the last received one for this program and associated packet number 
	used to compute PCR interpolation value*/
	u64 before_last_pcr_value;
	u32 before_last_pcr_value_pck_number;


	/*PES reframer - if NULL, pes processing is skiped*/
	u32 frame_state;
	/*returns the number of bytes consummed from the input data buffer*/
	void (*reframe)(struct tag_m2ts_demux *ts, struct tag_m2ts_pes *pes, u64 DTS, u64 CTS, unsigned char *data, u32 data_len);
	/*LATM stuff - should be moved out of mpegts*/
	unsigned char *buf;
	u32 buf_len;

	GF_M2TS_DVB_Subtitling_Descriptor sub;
} GF_M2TS_PES;

/*SDT information object*/
typedef struct
{
	u16 original_network_id;
	u16 transport_stream_id;
	u32 service_id;
	u32 EIT_schedule;
	u32 EIT_present_following;
	u32 running_status;
	u32 free_CA_mode;
	u8 service_type;
	unsigned char *provider, *service;
} GF_M2TS_SDT;

typedef struct
{
	u16 network_id;
	unsigned char *network_name;
	u16 original_network_id;
	u16 transport_stream_id;
	u16 service_id;
	u32 service_type;
	u32 logical_channel_number;
} GF_M2TS_NIT;

#define GF_M2TS_BASE_DESCRIPTOR u32 tag;

typedef struct {
	u8 content_nibble_level_1, content_nibble_level_2, user_nibble;
} GF_M2TS_DVB_Content_Descriptor;

typedef struct {
	char country_code[3];
	u8 value;
} GF_M2TS_DVB_Rating_Descriptor;

typedef struct {
	unsigned char lang[3];
	unsigned char *event_name, *event_text;
} GF_M2TS_DVB_Short_Event_Descriptor;

typedef struct {
	unsigned char *item;
	unsigned char *description;
} GF_M2TS_DVB_Extended_Event_Item;

typedef struct {
	unsigned char lang[3];
	u32 last;
	GF_List *items;
	unsigned char *text;
} GF_M2TS_DVB_Extended_Event_Descriptor;

/*EIT information objects*/
typedef struct
{
	time_t unix_time;

	/* local time offset descriptor data (unused ...) */
	char country_code[3];
	u8 country_region_id;
	s32 local_time_offset_seconds;
	time_t unix_next_toc;
	s32 next_time_offset_seconds;

} GF_M2TS_DateTime_Event;

typedef struct {
	u8 stream_content;
	u8 component_type;
	u8 component_tag;
	char language_code[3];
	unsigned char *text;
} GF_M2TS_Component;

typedef struct
{
	u16 event_id;
	time_t unix_start;
	time_t unix_duration;


	u8 running_status;
	u8 free_CA_mode;
	GF_List *short_events;
	GF_List *extended_events;
	GF_List *components;
	GF_List *contents;
	GF_List *ratings;	
} GF_M2TS_EIT_Event;

typedef struct
{
	u32 original_network_id;
	u32 transport_stream_id;
	u16 service_id;
	GF_List *events;
	u8 table_id;
} GF_M2TS_EIT;

/*MPEG-2 TS packet*/
typedef struct
{
	char *data;
	u32 data_len;
	u32 flags;
	u64 PTS, DTS;
	/*parent stream*/
	GF_M2TS_PES *stream;
} GF_M2TS_PES_PCK;

/*MPEG-4 SL packet from MPEG-2 TS*/
typedef struct
{
	char *data;
	u32 data_len;
	u8 version_number;
	/*parent stream */
	GF_M2TS_ES *stream;
} GF_M2TS_SL_PCK;

/*MPEG-2 TS demuxer*/
struct tag_m2ts_demux
{
	/* From M2TSIn */	
	GF_List *requested_progs;
	GF_List *requested_pids;

	/*demuxer thread*/
	GF_Thread *th;
	u32 run_state;

    /*net playing*/
	GF_Socket *sock;

#ifdef GPAC_HAS_LINUX_DVB
	/*dvb playing*/
	GF_Tuner *tuner;
#endif
	/*local file playing*/
	FILE *file;
	char filename[GF_MAX_PATH];
	u32 start_range, end_range;
	u64 file_size;
	Double duration;
	u32 nb_playing;
	Bool file_regulate;
	u64 pcr_last;
	u32 stb_at_last_pcr;
	u32 nb_pck;
	Bool loop_demux;

	/* "Network" =  "MobileIP", "DefaultMCastInterface" */
	Bool MobileIPEnabled;
	const char *network_type;
	/* Set it to 1 if the TS is meant to be played during the demux */
	Bool demux_and_play;
	/* End of M2TSIn */

	GF_M2TS_ES *ess[GF_M2TS_MAX_STREAMS];
	GF_List *programs;
	u32 nb_prog_pmt_received;
	Bool all_prog_pmt_received;
	/*keep it seperate for now - TODO check if we're sure of the order*/
	GF_List *SDTs;

	/*user callback - MUST NOT BE NULL*/
	void (*on_event)(struct tag_m2ts_demux *ts, u32 evt_type, void *par);
	/*private user data*/
	void *user;

	/*private resync buffer*/
	char *buffer;
	u32 buffer_size, alloc_size;
	/*default transport PID filters*/
	GF_M2TS_SectionFilter *pat, *cat, *nit, *sdt, *eit, *tdt_tot_st;

	Bool has_4on2;
	/* analyser */
	FILE *pes_out;

	Bool direct_mpe;

	Bool dvb_h_demux;
	
	/*user callback - MUST NOT BE NULL*/
	void (*on_mpe_event)(struct tag_m2ts_demux *ts, u32 evt_type, void *par);
	/* Structure to hold all the INT tables if the TS contains IP streams */
	struct __gf_dvb_mpe_ip_platform *ip_platform;
	
	u32 pck_number;

	/*remote file handling - created and destroyed by user*/
	struct __gf_download_session *dnload;

	const char *dvb_channels_conf_path;

	const char *(*query_next)(void *udta);
	void *udta_query;
};

GF_M2TS_Demuxer *gf_m2ts_demux_new();
void gf_m2ts_demux_del(GF_M2TS_Demuxer *ts);
void gf_m2ts_reset_parsers(GF_M2TS_Demuxer *ts);
GF_ESD *gf_m2ts_get_esd(GF_M2TS_ES *es);
GF_Err gf_m2ts_set_pes_framing(GF_M2TS_PES *pes, u32 mode);
GF_Err gf_m2ts_process_data(GF_M2TS_Demuxer *ts, char *data, u32 data_size);
u32 gf_dvb_get_freq_from_url(const char *channels_config_path, const char *url);



u32 gf_m2ts_crc32_check(char *data, u32 len);

/*MPEG-2 Descriptor tags*/
enum
{
	/* ... */
	GF_M2TS_VIDEO_STREAM_DESCRIPTOR							= 0x02,
	GF_M2TS_AUDIO_STREAM_DESCRIPTOR							= 0x03,
	GF_M2TS_HIERARCHY_DESCRIPTOR							= 0x04,
	GF_M2TS_REGISTRATION_DESCRIPTOR							= 0x05,
	GF_M2TS_DATA_STREAM_ALIGNEMENT_DESCRIPTOR				= 0x06,
	GF_M2TS_TARGET_BACKGROUND_GRID_DESCRIPTOR				= 0x07,
	GF_M2TS_VIEW_WINDOW_DESCRIPTOR							= 0x08,
	GF_M2TS_CA_DESCRIPTOR									= 0x09,
	GF_M2TS_ISO_639_LANGUAGE_DESCRIPTOR						= 0x0A,
	GF_M2TS_DVB_IP_MAC_PLATFORM_NAME_DESCRIPTOR				= 0x0C,
	GF_M2TS_DVB_IP_MAC_PLATFORM_PROVIDER_NAME_DESCRIPTOR	= 0x0D,
	GF_M2TS_DVB_TARGET_IP_SLASH_DESCRIPTOR			= 0x0F,
	/* ... */
	GF_M2TS_DVB_STREAM_LOCATION_DESCRIPTOR        =0x13,
	/* ... */
	GF_M2TS_STD_DESCRIPTOR					= 0x17,
	/* ... */
	GF_M2TS_MPEG4_VIDEO_DESCRIPTOR				= 0x1B,
	GF_M2TS_MPEG4_AUDIO_DESCRIPTOR				= 0x1C,
	GF_M2TS_MPEG4_IOD_DESCRIPTOR				= 0x1D,
	GF_M2TS_MPEG4_SL_DESCRIPTOR				= 0x1E,
	GF_M2TS_MPEG4_FMC_DESCRIPTOR				= 0x1F,
	/* ... */
	GF_M2TS_AVC_VIDEO_DESCRIPTOR				= 0x28,
	/* ... */	
	GF_M2TS_AVC_TIMING_HRD_DESCRIPTOR			= 0x2A,
	/* ... */

	/* 0x2D - 0x3F - ISO/IEC 13818-6 values */
	/* 0x40 - 0xFF - User Private values */
	GF_M2TS_DVB_NETWORK_NAME_DESCRIPTOR			= 0x40,
	GF_M2TS_DVB_SERVICE_LIST_DESCRIPTOR			= 0x41,
	GF_M2TS_DVB_STUFFING_DESCRIPTOR				= 0x42,
	GF_M2TS_DVB_SAT_DELIVERY_SYSTEM_DESCRIPTOR		= 0x43,
	GF_M2TS_DVB_CABLE_DELIVERY_SYSTEM_DESCRIPTOR		= 0x44,
	GF_M2TS_DVB_VBI_DATA_DESCRIPTOR				= 0x45,
	GF_M2TS_DVB_VBI_TELETEXT_DESCRIPTOR			= 0x46,
	GF_M2TS_DVB_BOUQUET_NAME_DESCRIPTOR			= 0x47,
	GF_M2TS_DVB_SERVICE_DESCRIPTOR				= 0x48,
	GF_M2TS_DVB_COUNTRY_AVAILABILITY_DESCRIPTOR		= 0x49,
	GF_M2TS_DVB_LINKAGE_DESCRIPTOR				= 0x4A,
	GF_M2TS_DVB_NVOD_REFERENCE_DESCRIPTOR			= 0x4B,
	GF_M2TS_DVB_TIME_SHIFTED_SERVICE_DESCRIPTOR		= 0x4C,
	GF_M2TS_DVB_SHORT_EVENT_DESCRIPTOR			= 0x4D,
	GF_M2TS_DVB_EXTENDED_EVENT_DESCRIPTOR			= 0x4E,
	GF_M2TS_DVB_TIME_SHIFTED_EVENT_DESCRIPTOR		= 0x4F,
	GF_M2TS_DVB_COMPONENT_DESCRIPTOR			= 0x50,
	GF_M2TS_DVB_MOSAIC_DESCRIPTOR				= 0x51,
	GF_M2TS_DVB_STREAM_IDENTIFIER_DESCRIPTOR		= 0x52,
	GF_M2TS_DVB_CA_IDENTIFIER_DESCRIPTOR			= 0x53,
	GF_M2TS_DVB_CONTENT_DESCRIPTOR				= 0x54,
	GF_M2TS_DVB_PARENTAL_RATING_DESCRIPTOR			= 0x55,
	GF_M2TS_DVB_TELETEXT_DESCRIPTOR				= 0x56,
	/* ... */
	GF_M2TS_DVB_LOCAL_TIME_OFFSET_DESCRIPTOR		= 0x58,
	GF_M2TS_DVB_SUBTITLING_DESCRIPTOR			= 0x59,
	GF_M2TS_DVB_PRIVATE_DATA_SPECIFIER_DESCRIPTOR = 0x5F,
	/* ... */
	GF_M2TS_DVB_DATA_BROADCAST_DESCRIPTOR			= 0x64,
	/* ... */
	GF_M2TS_DVB_DATA_BROADCAST_ID_DESCRIPTOR		= 0x66,
	/* ... */
	GF_M2TS_DVB_AC3_DESCRIPTOR				= 0x6A,
	/* ... */
	GF_M2TS_DVB_TIME_SLICE_FEC_DESCRIPTOR 		   = 0x77,
	/* ... */
	GF_M2TS_DVB_EAC3_DESCRIPTOR				= 0x7A,
	GF_M2TS_DVB_LOGICAL_CHANNEL_DESCRIPTOR = 0x83,	
	
};

/* Reserved PID values */
enum {
	GF_M2TS_PID_PAT			= 0x0000,
	GF_M2TS_PID_CAT			= 0x0001,
	GF_M2TS_PID_TSDT		= 0x0002,
	/* reserved 0x0003 to 0x000F */ 
	GF_M2TS_PID_NIT_ST		= 0x0010,
	GF_M2TS_PID_SDT_BAT_ST		= 0x0011,
	GF_M2TS_PID_EIT_ST_CIT		= 0x0012,
	GF_M2TS_PID_RST_ST		= 0x0013,
	GF_M2TS_PID_TDT_TOT_ST		= 0x0014,
	GF_M2TS_PID_NET_SYNC		= 0x0015,
	GF_M2TS_PID_RNT			= 0x0016,
	/* reserved 0x0017 to 0x001B */ 
	GF_M2TS_PID_IN_SIG		= 0x001C,
	GF_M2TS_PID_MEAS		= 0x001D,
	GF_M2TS_PID_DIT			= 0x001E,
	GF_M2TS_PID_SIT			= 0x001F
};

/* max size includes first header, second header, payload and CRC */
enum {
	GF_M2TS_TABLE_ID_PAT			= 0x00,
	GF_M2TS_TABLE_ID_CAT			= 0x01, 
	GF_M2TS_TABLE_ID_PMT			= 0x02, 
	GF_M2TS_TABLE_ID_TSDT			= 0x03, /* max size for section 1024 */
	GF_M2TS_TABLE_ID_MPEG4_BIFS		= 0x04, /* max size for section 4096 */
	GF_M2TS_TABLE_ID_MPEG4_OD		= 0x05, /* max size for section 4096 */
	GF_M2TS_TABLE_ID_METADATA		= 0x06, 
	GF_M2TS_TABLE_ID_IPMP_CONTROL		= 0x07, 
	/* 0x08 - 0x37 reserved */
	/* 0x38 - 0x3D DSM-CC defined */
	GF_M2TS_TABLE_ID_DSM_CC_PRIVATE		= 0x3E, /* used for MPE (only, not MPE-FEC) */
	/* 0x3F DSM-CC defined */
	GF_M2TS_TABLE_ID_NIT_ACTUAL		= 0x40, /* max size for section 1024 */
	GF_M2TS_TABLE_ID_NIT_OTHER		= 0x41,
	GF_M2TS_TABLE_ID_SDT_ACTUAL		= 0x42, /* max size for section 1024 */
	/* 0x43 - 0x45 reserved */
	GF_M2TS_TABLE_ID_SDT_OTHER		= 0x46, /* max size for section 1024 */
	/* 0x47 - 0x49 reserved */
	GF_M2TS_TABLE_ID_BAT			= 0x4a, /* max size for section 1024 */
	/* 0x4b	reserved */
	GF_M2TS_TABLE_ID_INT			= 0x4c, /* max size for section 4096 */
	/* 0x4d reserved */
	
	GF_M2TS_TABLE_ID_EIT_ACTUAL_PF		= 0x4E, /* max size for section 4096 */
	GF_M2TS_TABLE_ID_EIT_OTHER_PF		= 0x4F,
	/* 0x50 - 0x6f EIT SCHEDULE */
	GF_M2TS_TABLE_ID_EIT_SCHEDULE_MIN	= 0x50,
	GF_M2TS_TABLE_ID_EIT_SCHEDULE_ACTUAL_MAX= 0x5F,
	GF_M2TS_TABLE_ID_EIT_SCHEDULE_MAX	= 0x6F,

	GF_M2TS_TABLE_ID_TDT			= 0x70,
	GF_M2TS_TABLE_ID_RST			= 0x71, /* max size for section 1024 */
	GF_M2TS_TABLE_ID_ST 			= 0x72, /* max size for section 4096 */
	GF_M2TS_TABLE_ID_TOT			= 0x73,
	GF_M2TS_TABLE_ID_AIT			= 0x74,
	GF_M2TS_TABLE_ID_CONT			= 0x75,
	GF_M2TS_TABLE_ID_RC			= 0x76,
	GF_M2TS_TABLE_ID_CID			= 0x77,
	GF_M2TS_TABLE_ID_MPE_FEC		= 0x78,
	GF_M2TS_TABLE_ID_RES_NOT		= 0x79,
	/* 0x7A - 0x7D reserved */
	GF_M2TS_TABLE_ID_DIT			= 0x7E,
	GF_M2TS_TABLE_ID_SIT			= 0x7F, /* max size for section 4096 */
	/* 0x80 - 0xfe reserved */
	/* 0xff reserved */
};



#define SECTION_HEADER_LENGTH 3 /* header till the last bit of the section_length field */
#define SECTION_ADDITIONAL_HEADER_LENGTH 5 /* header from the last bit of the section_length field to the payload */
#define	CRC_LENGTH 4

typedef struct
{
	u8 sync;
	u8 error;
	u8 payload_start;
	u8 priority;
	u16 pid;
	u8 scrambling_ctrl;
	u8 adaptation_field;
	u8 continuity_counter;
} GF_M2TS_Header;

typedef struct
{
	u32 discontinuity_indicator;
	u32 random_access_indicator;
	u32 priority_indicator;

	u32 PCR_flag;
	u64 PCR_base, PCR_ext;

	u32 OPCR_flag;
	u64 OPCR_base, OPCR_ext;

	u32 splicing_point_flag;
	u32 transport_private_data_flag;
	u32 adaptation_field_extension_flag;
/*	
	u32 splice_countdown;
	u32 transport_private_data_length;
	u32 adaptation_field_extension_length;
	u32 ltw_flag;
	u32 piecewise_rate_flag;
	u32 seamless_splice_flag;
	u32 ltw_valid_flag;
	u32 ltw_offset;
	u32 piecewise_rate;
	u32 splice_type;
	u32 DTS_next_AU;
*/
} GF_M2TS_AdaptationField;



void gf_m2ts_print_info(GF_M2TS_Demuxer *ts);

#ifndef GPAC_DISABLE_MPEG2TS_MUX

/*
	MPEG-2 TS Multiplexer
*/

enum {
	GF_M2TS_ADAPTATION_RESERVED	= 0,
	GF_M2TS_ADAPTATION_NONE		= 1,
	GF_M2TS_ADAPTATION_ONLY		= 2,
	GF_M2TS_ADAPTATION_AND_PAYLOAD 	= 3,
};



typedef struct __m2ts_mux_program GF_M2TS_Mux_Program;
typedef struct __m2ts_mux GF_M2TS_Mux;

typedef struct __m2ts_section {
	struct __m2ts_section *next;
	u8 *data;
	u32 length;
} GF_M2TS_Mux_Section;

typedef struct __m2ts_table {
	struct __m2ts_table *next;
	u8 table_id;
	u8 version_number;
	struct __m2ts_section *section;
} GF_M2TS_Mux_Table;

typedef struct
{
	u32 sec;
	u32 nanosec;
} GF_M2TS_Time;


typedef struct __m2ts_mux_pck
{
	struct __m2ts_mux_pck *next;
	char *data;
	u32 data_len;
	u32 flags;
	u64 cts, dts;
} GF_M2TS_Packet;


typedef struct __m2ts_mux_stream {
	struct __m2ts_mux_stream *next;

	u32 pid;
	u8 continuity_counter;
	struct __m2ts_mux_program *program;

	/*average stream bit-rate in bit/sec*/
	u32 bit_rate;
	
	/*multiplexer time - NOT THE PCR*/
	GF_M2TS_Time time;

	/*table tools*/
	GF_M2TS_Mux_Table *tables;
	/*total table sizes for bitrate estimation (PMT/PAT/...)*/
	u32 total_table_size;
	/* used for on-the-fly packetization of sections */
	GF_M2TS_Mux_Table *current_table;
	GF_M2TS_Mux_Section *current_section;
	u32 current_section_offset;
	u32 refresh_rate_ms;
	Bool table_needs_update;

	Bool (*process)(struct __m2ts_mux *muxer, struct __m2ts_mux_stream *stream);

	/*PES tools*/
	void *pes_packetizer;
	u32 mpeg2_stream_type;
	u32 mpeg2_stream_id;

	GF_ESIPacket curr_pck; /*current packet being processed - does not belong to the packet fifo*/
	u32 pck_offset;
	Bool force_new;
	Bool discard_data;

	struct __elementary_stream_ifce *ifce;
	Double ts_scale;
	u64 initial_ts;

	/*packet fifo*/
	GF_M2TS_Packet *pck_first, *pck_last;
	/*packet reassembler (PES packets are most of the time full frames)*/
	GF_M2TS_Packet *pck_reassembler;
	GF_Mutex *mx;
	/*avg bitrate compute*/
	u64 last_br_time;
	u32 bytes_since_last_time;

	/*MPEG-4 over MPEG-2*/
	u8 table_id;
	GF_SLHeader sl_header;
	/* MPEG-4 SL Config */
	GF_SLConfig sl_config;

	u32 last_aac_time;
} GF_M2TS_Mux_Stream;

enum {
	GF_M2TS_MPEG4_SIGNALING_NONE = 0,
	GF_M2TS_MPEG4_SIGNALING_FULL,
	/*experimental profile where all PES media streams (audio, video, images) are sent directly on PES without SL-packetization*/
	GF_M2TS_MPEG4_SIGNALING_SCENE
};


struct __m2ts_mux_program {
	struct __m2ts_mux_program *next;

	struct __m2ts_mux *mux;
	u16 number;
	/*all streams but PMT*/
	GF_M2TS_Mux_Stream *streams;
	/*PMT*/
	GF_M2TS_Mux_Stream *pmt;
	/*pointer to PCR stream*/
	GF_M2TS_Mux_Stream *pcr;

	/*TS time at pcr init*/
	GF_M2TS_Time ts_time_at_pcr_init;
	u64 pcr_init_time, num_pck_at_pcr_init;
	u64 last_pcr;
	u32 last_sys_clock;

	GF_Descriptor *iod;
	Bool mpeg4_signaling;
	Bool mpeg4_signaling_for_scene_only;
};

struct __m2ts_mux {
	GF_M2TS_Mux_Program *programs;
	GF_M2TS_Mux_Stream *pat;

	u16 ts_id;

	Bool needs_reconfig;

    /* used to indicate that the input data is pushed to the muxer (i.e. not read from a file)
    or that the output data is sent on sockets (not written to a file) */
	Bool real_time;

	/* indicates if the multiplexer shall target a fix bit rate (monitoring timing and produce padding packets)
       or if the output stream will contain only input data*/
	Bool fixed_rate;

	/*output bit-rate in bit/sec*/
	u32 bit_rate;

	char dst_pck[188], null_pck[188];

	/*multiplexer time, incremented each time a packet is sent
      used to monitor the sending of muxer related data (PAT, ...) */
	GF_M2TS_Time time; 
    
    /* Time of the muxer when the first call to process is made (first packet sent?) */
    GF_M2TS_Time init_ts_time;
	
    /* System time when the muxer is started */
    u32 init_sys_time;

	Bool eos_found;
	u32 pck_sent_over_br_window, last_br_time, avg_br;
	u64 tot_pck_sent, tot_pad_sent;
};




enum
{
	GF_M2TS_STATE_IDLE,
	GF_M2TS_STATE_DATA,
	GF_M2TS_STATE_PADDING,
	GF_M2TS_STATE_EOS,
};

/*!
 * mux_rate en kbps
 */

GF_M2TS_Mux *gf_m2ts_mux_new(u32 mux_rate, u32 pat_refresh_rate, Bool real_time);
void gf_m2ts_mux_del(GF_M2TS_Mux *mux);
GF_M2TS_Mux_Program *gf_m2ts_mux_program_add(GF_M2TS_Mux *muxer, u32 program_number, u32 pmt_pid, u32 pmt_refresh_rate, Bool mpeg4_signaling);
GF_M2TS_Mux_Stream *gf_m2ts_program_stream_add(GF_M2TS_Mux_Program *program, GF_ESInterface *ifce, u32 pid, Bool is_pcr);
void gf_m2ts_mux_update_config(GF_M2TS_Mux *mux, Bool reset_time);
const char *gf_m2ts_mux_process(GF_M2TS_Mux *muxer, u32 *status);
u32 gf_m2ts_get_sys_clock(GF_M2TS_Mux *muxer);
u32 gf_m2ts_get_ts_clock(GF_M2TS_Mux *muxer);

/*user inteface functions*/
GF_Err gf_m2ts_program_stream_update_ts_scale(GF_ESInterface *_self, u32 time_scale);
void gf_m2ts_program_stream_update_sl_config(GF_ESInterface *_self, GF_SLConfig *slc);


#endif /*GPAC_DISABLE_MPEG2TS_MUX*/

/******************* Demux DVB ****************************/
#include <gpac/carousel.h>


#define UDP_BUFFER_SIZE	0x40000
#define M2TS_BUFFER_MAX 400


#ifdef GPAC_HAS_LINUX_DVB
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/dvb/dmx.h>
#include <linux/dvb/frontend.h>

struct __gf_dvb_tuner {
	u32 freq;
	u16 vpid;
	u16 apid;
	fe_spectral_inversion_t specInv;
	fe_modulation_t modulation;
	fe_bandwidth_t bandwidth;
	fe_transmit_mode_t TransmissionMode;
	fe_guard_interval_t guardInterval;
	fe_code_rate_t HP_CodeRate;
	fe_code_rate_t LP_CodeRate;
	fe_hierarchy_t hierarchy;

	int ts_fd;
};


#define DVB_BUFFER_SIZE 3760							// DVB buffer size 188x20

#endif


GF_Err TSDemux_Demux_Setup(GF_M2TS_Demuxer *ts, const char *url, Bool loop);
GF_Err TSDemux_DemuxPlay(GF_M2TS_Demuxer *ts);
GF_Err TSDemux_CloseDemux(GF_M2TS_Demuxer *ts);


#endif /*GPAC_DISABLE_MPEG2TS*/

#endif	//_GF_MPEG_TS_H_
