/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005 
 *					All rights reserved
 *
 *  This file is part of GPAC / Stream Management sub-project
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



#ifndef _GF_TERM_INFO_H_
#define _GF_TERM_INFO_H_


#ifdef __cplusplus
extern "C" {
#endif

/*
	OD Browsing API - YOU MUST INCLUDE <gpac/terminal.h> before 
	(this has been separated from terminal.h to limit dependency of core to mpeg4_odf.h header)
	ALL ITEMS ARE READ-ONLY AND SHALL NOT BE MODIFIED
*/
#include <gpac/mpeg4_odf.h>

/*returns top-level OD of the presentation*/
GF_ObjectManager *gf_term_get_root_object(GF_Terminal *term);
/*returns number of sub-ODs in the current root. scene_od must be an inline OD*/
u32 gf_term_get_object_count(GF_Terminal *term, GF_ObjectManager *scene_od);
/*returns indexed (0-based) OD manager in the scene*/
GF_ObjectManager *gf_term_get_object(GF_Terminal *term, GF_ObjectManager *scene_od, u32 index);
/*return values:
	0: regular media object, not inline
	1: root scene
	2: inline scene
	3: externProto library
*/
u32 gf_term_object_subscene_type(GF_Terminal *term, GF_ObjectManager *odm);

/*select given object when stream selection is available*/
void gf_term_select_object(GF_Terminal *term, GF_ObjectManager *odm);

typedef struct
{
	GF_ObjectDescriptor *od;
	Double duration;
	Double current_time;
	/*0: stopped, 1: playing, 2: paused, 3: not setup, 4; setup failed.*/
	u32 status;
	/*if set, the PL flags are valid*/
	Bool has_profiles;
	Bool inline_pl;
	u8 OD_pl; 
	u8 scene_pl;
	u8 audio_pl;
	u8 visual_pl;
	u8 graphics_pl;

	/*name of module handling the service service */
	const char *service_handler;
	/*name of service*/
	const char *service_url;
	/*set if the service is owned by this object*/
	Bool owns_service;

	/*stream buffer:
		-2: stream is not playing
		-1: stream has no buffering
		>=0: amount of media data present in buffer, in ms
	*/
	s32 buffer;
	/*number of AUs in DB (cumulated on all input channels)*/
	u32 db_unit_count;
	/*number of CUs in composition memory (if any) and CM capacity*/
	u16 cb_unit_count, cb_max_count;
	/*clock drift in ms of object clock: this is the delay set by the audio renderer to keep AV in sync*/
	s32 clock_drift;
	/*codec name*/
	const char *codec_name;
	/*object type - match streamType (cf constants.h)*/
	u32 od_type;
	/*audio properties*/
	u32 sample_rate, bits_per_sample, num_channels;
	/*video properties (w & h also used for scene codecs)*/
	u32 width, height, pixelFormat, par;

	/*average birate over last second and max bitrate over one second at decoder input - expressed in bits per sec*/
	u32 avg_bitrate, max_bitrate;
	u32 total_dec_time, max_dec_time, nb_dec_frames, nb_droped;

	/*set if ISMACryp present on the object - will need refinement for IPMPX...
	0: not protected - 1: protected and OK - 2: protected and DRM failed*/
	u32 protection;

	u32 lang;

	/*name of media if not defined in OD framework*/
	const char *media_url;
} GF_MediaInfo;

/*fills the GF_MediaInfo structure describing the OD manager*/
GF_Err gf_term_get_object_info(GF_Terminal *term, GF_ObjectManager *odm, GF_MediaInfo *info);
/*gets current downloads info for the service - only use if ODM owns thesrevice, returns 0 otherwise.
	@d_enum: in/out current enum - shall start to 0, incremented at each call. fct returns 0 if no more 
	downloads
	@server: server name
	@path: file/data location on server
	@bytes_done, @total_bytes: file info. total_bytes may be 0 (eg http streaming)
	@bytes_per_sec: guess what
*/
Bool gf_term_get_download_info(GF_Terminal *term, GF_ObjectManager *odm, u32 *d_enum, const char **server, const char **path, u32 *bytes_done, u32 *total_bytes, u32 *bytes_per_sec);

/*same principles as above , struct __netcom is defined in service.h*/
typedef struct __netstatcom NetStatCommand;
Bool gf_term_get_channel_net_info(GF_Terminal *term, GF_ObjectManager *odm, u32 *d_enum, u32 *chid, NetStatCommand *netcom, GF_Err *ret_code);

/*same principles as above , struct __netinfo is defined in service.h*/
typedef struct __netinfocom NetInfoCommand;
GF_Err gf_term_get_service_info(GF_Terminal *term, GF_ObjectManager *odm, NetInfoCommand *netcom);

/*retrieves world info of the scene @od belongs to. 
If @odm is or points to an inlined OD the world info of the inlined content is retrieved
If @odm is NULL the world info of the main scene is retrieved
returns NULL if no WorldInfo available
returns world title if available 
@descriptions: any textual descriptions is stored here
  strings are not allocated
*/
const char *gf_term_get_world_info(GF_Terminal *term, GF_ObjectManager *scene_od, GF_List *descriptions);

/*dumps scene graph in specified file, in BT or XMT format
@rad_name: file radical (NULL for stdout) - if not NULL MUST BE GF_MAX_PATH length
@filename [out]: if not null, returns the complete filename (rad + ext); MUST BE FREED BY THE CALLER
if @skip_proto is set proto declarations are not dumped
If @odm is or points to an inlined OD the inlined scene is dumped
If @odm is NULL the main scene is dumped
*/
GF_Err gf_term_dump_scene(GF_Terminal *term, char *rad_name, char **filename, Bool xml_dump, Bool skip_proto, GF_ObjectManager *odm);


#ifdef __cplusplus
}
#endif


#endif	/*_GF_TERM_INFO_H_*/
