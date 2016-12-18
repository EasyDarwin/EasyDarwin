/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Authors: Jean le Feuvre
 *				Copyright (c) 2005-200X ENST
 *					All rights reserved
 *
 *  This file is part of GPAC / LASeR codec sub-project
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


#ifndef _GF_LASER_DEV_H_
#define _GF_LASER_DEV_H_

#include <gpac/laser.h>

#ifndef GPAC_DISABLE_LASER

/*per_stream config support*/
typedef struct 
{
	GF_LASERConfig cfg;
	u16 ESID;
} LASeRStreamInfo;

typedef struct
{
	/*colors can be encoded on up to 16 bits per comp*/
	u16 r, g, b;
} LSRCol;

struct __tag_laser_codec
{
	GF_BitStream *bs;
	GF_SceneGraph *sg;
	GF_Err last_error;

	/*all attached streams*/
	GF_List *streamInfo;

	LASeRStreamInfo *info;
	Fixed res_factor/*2^-coord_res*/;
	/*duplicated from config*/
	u8 scale_bits;
	u8 coord_bits;
	u16 time_resolution;
	u16 color_scale;

	LSRCol *col_table;
	u32 nb_cols;
	/*computed dynamically*/
	u32 colorIndexBits;
	GF_List *font_table;
	u32 fontIndexBits;

	u32 privateData_id_index, privateTag_index;

	/*decoder only*/
	Double (*GetSceneTime)(void *cbk);
	void *cbk;

	/*sameElement coding*/
	SVG_Element *prev_g;
	SVG_Element *prev_line;
	SVG_Element *prev_path;
	SVG_Element *prev_polygon;
	SVG_Element *prev_rect;
	SVG_Element *prev_text;
	SVG_Element *prev_use;
	GF_Node *current_root;

	/*0: normal playback, store script content
	  1: memory decoding of scene, decompress script into commands
	*/
	Bool memory_dec;

	GF_List *defered_hrefs;
	GF_List *defered_anims;
	GF_List *defered_listeners;

	char *cache_dir, *service_name;
	GF_List *unresolved_commands;
};

s32 gf_lsr_anim_type_from_attribute(u32 tag);
s32 gf_lsr_anim_type_to_attribute(u32 tag);
s32 gf_lsr_rare_type_from_attribute(u32 tag);
s32 gf_lsr_rare_type_to_attribute(u32 tag);
u32 gf_lsr_same_rare(SVGAllAttributes *elt_atts, SVGAllAttributes *base_atts);


/*transform*/
#define RARE_TRANSFORM				47

enum
{
	LSR_EVT_abort = 0,
	LSR_EVT_accessKey = 1,
	LSR_EVT_activate = 2,
	LSR_EVT_activatedEvent = 3,
	LSR_EVT_beginEvent = 4,
	LSR_EVT_click = 5,
	LSR_EVT_deactivatedEvent = 6,
	LSR_EVT_endEvent = 7,
	LSR_EVT_error = 8,
	LSR_EVT_executionTime = 9,
	LSR_EVT_focusin = 10,
	LSR_EVT_focusout = 11,
	LSR_EVT_keydown = 12,
	LSR_EVT_keyup = 13,
	LSR_EVT_load = 14,
	LSR_EVT_longAccessKey = 15,
	LSR_EVT_mousedown = 16,
	LSR_EVT_mousemove = 17,
	LSR_EVT_mouseout = 18,
	LSR_EVT_mouseover = 19,
	LSR_EVT_mouseup = 20,
	LSR_EVT_pause = 21,
	LSR_EVT_pausedEvent = 22,
	LSR_EVT_play = 23,
	LSR_EVT_repeatEvent = 24,
	LSR_EVT_repeatKey = 25,
	LSR_EVT_resize = 26,
	LSR_EVT_resumedEvent = 27,
	LSR_EVT_scroll = 28,
	LSR_EVT_shortAccessKey = 29,
	LSR_EVT_textinput = 30,
	LSR_EVT_unload = 31,
	LSR_EVT_zoom = 32
};	

u32 dom_to_lsr_key(u32 dom_k);


#define LSR_UPDATE_TYPE_ROTATE			76
#define LSR_UPDATE_TYPE_SCALE			79
#define LSR_UPDATE_TYPE_SVG_HEIGHT		94
#define LSR_UPDATE_TYPE_SVG_WIDTH		95
#define LSR_UPDATE_TYPE_TEXT_CONTENT	107
#define LSR_UPDATE_TYPE_TRANSFORM		108
#define LSR_UPDATE_TYPE_TRANSLATION		110


/*LASeR commands code*/
enum
{
	LSR_UPDATE_ADD = 0,
	LSR_UPDATE_CLEAN,
	LSR_UPDATE_DELETE,
	LSR_UPDATE_INSERT,
	LSR_UPDATE_NEW_SCENE,
	LSR_UPDATE_REFRESH_SCENE,
	LSR_UPDATE_REPLACE,
	LSR_UPDATE_RESTORE,
	LSR_UPDATE_SAVE,
	LSR_UPDATE_SEND_EVENT,
	LSR_UPDATE_EXTEND,
	LSR_UPDATE_TEXT_CONTENT
};

/*Code point Path code*/
enum
{
	LSR_PATH_COM_C = 0,
	LSR_PATH_COM_H,
	LSR_PATH_COM_L,
	LSR_PATH_COM_M, 
	LSR_PATH_COM_Q, 
	LSR_PATH_COM_S, 
	LSR_PATH_COM_T, 
	LSR_PATH_COM_V, 
	LSR_PATH_COM_Z, 
	LSR_PATH_COM_c, 
	LSR_PATH_COM_h, 
	LSR_PATH_COM_l, 
	LSR_PATH_COM_m, 
	LSR_PATH_COM_q, 
	LSR_PATH_COM_s, 
	LSR_PATH_COM_t,
	LSR_PATH_COM_v,
	LSR_PATH_COM_z
};




enum
{
	LSR_SCENE_CONTENT_MODEL_a = 0,
	LSR_SCENE_CONTENT_MODEL_animate,
	LSR_SCENE_CONTENT_MODEL_animateColor,
	LSR_SCENE_CONTENT_MODEL_animateMotion,
	LSR_SCENE_CONTENT_MODEL_animateTransform,
	LSR_SCENE_CONTENT_MODEL_audio,
	LSR_SCENE_CONTENT_MODEL_circle,
	LSR_SCENE_CONTENT_MODEL_defs,
	LSR_SCENE_CONTENT_MODEL_desc,
	LSR_SCENE_CONTENT_MODEL_ellipse,
	LSR_SCENE_CONTENT_MODEL_foreignObject,
	LSR_SCENE_CONTENT_MODEL_g,
	LSR_SCENE_CONTENT_MODEL_image,
	LSR_SCENE_CONTENT_MODEL_line,
	LSR_SCENE_CONTENT_MODEL_linearGradient,
	LSR_SCENE_CONTENT_MODEL_metadata,
	LSR_SCENE_CONTENT_MODEL_mpath,
	LSR_SCENE_CONTENT_MODEL_path,
	LSR_SCENE_CONTENT_MODEL_polygon,
	LSR_SCENE_CONTENT_MODEL_polyline,
	LSR_SCENE_CONTENT_MODEL_radialGradient,
	LSR_SCENE_CONTENT_MODEL_rect,
	LSR_SCENE_CONTENT_MODEL_sameg,
	LSR_SCENE_CONTENT_MODEL_sameline,
	LSR_SCENE_CONTENT_MODEL_samepath,
	LSR_SCENE_CONTENT_MODEL_samepathfill,
	LSR_SCENE_CONTENT_MODEL_samepolygon,
	LSR_SCENE_CONTENT_MODEL_samepolygonfill,
	LSR_SCENE_CONTENT_MODEL_samepolygonstroke,
	LSR_SCENE_CONTENT_MODEL_samepolyline,
	LSR_SCENE_CONTENT_MODEL_samepolylinefill,
	LSR_SCENE_CONTENT_MODEL_samepolylinestroke,
	LSR_SCENE_CONTENT_MODEL_samerect,
	LSR_SCENE_CONTENT_MODEL_samerectfill,
	LSR_SCENE_CONTENT_MODEL_sametext,
	LSR_SCENE_CONTENT_MODEL_sametextfill,
	LSR_SCENE_CONTENT_MODEL_sameuse,
	LSR_SCENE_CONTENT_MODEL_script,
	LSR_SCENE_CONTENT_MODEL_set,
	LSR_SCENE_CONTENT_MODEL_stop,
	LSR_SCENE_CONTENT_MODEL_switch,
	LSR_SCENE_CONTENT_MODEL_text,
	LSR_SCENE_CONTENT_MODEL_title,
	LSR_SCENE_CONTENT_MODEL_tspan,
	LSR_SCENE_CONTENT_MODEL_use,
	LSR_SCENE_CONTENT_MODEL_video,
	LSR_SCENE_CONTENT_MODEL_listener,
	LSR_SCENE_CONTENT_MODEL_conditional,
	LSR_SCENE_CONTENT_MODEL_cursorManager,
	LSR_SCENE_CONTENT_MODEL_element_any,
	LSR_SCENE_CONTENT_MODEL_privateContainer,
	LSR_SCENE_CONTENT_MODEL_rectClip,
	LSR_SCENE_CONTENT_MODEL_selector,
	LSR_SCENE_CONTENT_MODEL_simpleLayout,
	LSR_SCENE_CONTENT_MODEL_textContent,
	LSR_SCENE_CONTENT_MODEL_extension,
};

enum
{
	LSR_UPDATE_CONTENT_MODEL_a = 0,
	LSR_UPDATE_CONTENT_MODEL_animate,
	LSR_UPDATE_CONTENT_MODEL_animateColor,
	LSR_UPDATE_CONTENT_MODEL_animateMotion,
	LSR_UPDATE_CONTENT_MODEL_animateTransform,
	LSR_UPDATE_CONTENT_MODEL_audio,
	LSR_UPDATE_CONTENT_MODEL_circle,
	LSR_UPDATE_CONTENT_MODEL_defs,
	LSR_UPDATE_CONTENT_MODEL_desc,
	LSR_UPDATE_CONTENT_MODEL_ellipse,
	LSR_UPDATE_CONTENT_MODEL_foreignObject,
	LSR_UPDATE_CONTENT_MODEL_g,
	LSR_UPDATE_CONTENT_MODEL_image,
	LSR_UPDATE_CONTENT_MODEL_line,
	LSR_UPDATE_CONTENT_MODEL_linearGradient,
	LSR_UPDATE_CONTENT_MODEL_metadata,
	LSR_UPDATE_CONTENT_MODEL_mpath,
	LSR_UPDATE_CONTENT_MODEL_path,
	LSR_UPDATE_CONTENT_MODEL_polygon,
	LSR_UPDATE_CONTENT_MODEL_polyline,
	LSR_UPDATE_CONTENT_MODEL_radialGradient,
	LSR_UPDATE_CONTENT_MODEL_rect,
	LSR_UPDATE_CONTENT_MODEL_script,
	LSR_UPDATE_CONTENT_MODEL_set,
	LSR_UPDATE_CONTENT_MODEL_stop,
	LSR_UPDATE_CONTENT_MODEL_svg,
	LSR_UPDATE_CONTENT_MODEL_switch,
	LSR_UPDATE_CONTENT_MODEL_text,
	LSR_UPDATE_CONTENT_MODEL_title,
	LSR_UPDATE_CONTENT_MODEL_tspan,
	LSR_UPDATE_CONTENT_MODEL_use,
	LSR_UPDATE_CONTENT_MODEL_video,
	LSR_UPDATE_CONTENT_MODEL_listener,
};

enum
{
	LSR_UPDATE_CONTENT_MODEL2_conditional = 0,
	LSR_UPDATE_CONTENT_MODEL2_cursorManager,
	LSR_UPDATE_CONTENT_MODEL2_extend,
	LSR_UPDATE_CONTENT_MODEL2_private,
	LSR_UPDATE_CONTENT_MODEL2_rectClip,
	LSR_UPDATE_CONTENT_MODEL2_selector,
	LSR_UPDATE_CONTENT_MODEL2_simpleLayout,
};

/*just to remember them, not implemented yet*/
enum
{
	LSR_SVG12_EXT_animation = 0,
	LSR_SVG12_EXT_discard,
	LSR_SVG12_EXT_font,
	LSR_SVG12_EXT_font_face,
	LSR_SVG12_EXT_font_face_src,
	LSR_SVG12_EXT_font_face_uri,
	LSR_SVG12_EXT_glyph,
	LSR_SVG12_EXT_handler,
	LSR_SVG12_EXT_hkern,
	LSR_SVG12_EXT_missingGlyph,
	LSR_SVG12_EXT_prefetch,
	LSR_SVG12_EXT_solidColor,
	LSR_SVG12_EXT_tBreak,
	LSR_SVG12_EXT_textArea,
};

/*just to remember them, not implemented yet*/
enum
{
	LSR_AMD1_EXT_animateScroll = 0,
	LSR_AMD1_EXT_setScroll,
	LSR_AMD1_EXT_streamSource,
	LSR_AMD1_EXT_updateSource,
};

#endif /*GPAC_DISABLE_LASER*/

#endif

