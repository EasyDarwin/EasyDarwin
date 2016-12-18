/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005
 *					All rights reserved
 *
 *  This file is part of GPAC / Scene Management sub-project
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

#ifndef _GF_SWF_DEV_H_
#define _GF_SWF_DEV_H_

#include <gpac/scene_manager.h>
#include <gpac/color.h>

#define SWF_TWIP_SCALE				(1/20.0f)


typedef struct SWFReader SWFReader;
typedef struct SWFSound SWFSound;
typedef struct SWFText SWFText;
typedef struct SWFEditText SWFEditText;
typedef struct SWF_Button SWF_Button;
typedef struct SWFShape SWFShape;
typedef struct SWFFont SWFFont;
typedef struct SWFAction SWFAction;

enum
{
	SWF_PLACE ,
	SWF_REPLACE,
	SWF_MOVE,
};


struct SWFReader
{
	GF_SceneLoader *load;
	FILE *input;

	char *localPath;
	/*file header*/
	u32 length;
	char *mem;
	u32 frame_rate;
	u32 frame_count;
	Fixed width, height;
	Bool has_interact, no_as;

	/*copy of the swf import flags*/	
	u32 flags;

	/*bit reader*/
	GF_BitStream *bs;
	GF_Err ioerr;	
	
	u32 current_frame;

	/*current tag*/
	u32 tag, size;

	GF_List *display_list;
	u32 max_depth;

	/*defined fonts*/
	GF_List *fonts;

	/*define sounds*/
	GF_List *sounds;

	/*the one and only sound stream for current timeline*/
	SWFSound *sound_stream;

	/*when creating sprites: 
		1- all BIFS AUs in sprites are random access
		2- depth is ignored in Sprites
	*/
	u32 current_sprite_id;

	/*the parser can decide to remove nearly aligned pppoints in lineTo sequences*/
	/*flatten limit - 0 means no flattening*/
	Fixed flat_limit;
	/*number of points removed*/
	u32 flatten_points;

	u8 *jpeg_hdr;
	u32 jpeg_hdr_size;


	/*callback functions for translator*/
	GF_Err (*set_backcol)(SWFReader *read, u32 xrgb);
	GF_Err (*show_frame)(SWFReader *read);

	/*checks if display list is large enough - returns 1 if yes, 0 otherwise (and allocate space)*/
	Bool (*allocate_depth)(SWFReader *read, u32 depth); 
	GF_Err (*place_obj)(SWFReader *read, u32 depth, u32 ID, u32 prev_id, u32 type, GF_Matrix2D *mat, GF_ColorMatrix *cmat, GF_Matrix2D *prev_mat, GF_ColorMatrix *prev_cmat);
	GF_Err (*remove_obj)(SWFReader *read, u32 depth, u32 ID);

	GF_Err (*define_shape)(SWFReader *read, SWFShape *shape, SWFFont *parent_font, Bool last_sub_shape);
	GF_Err (*define_sprite)(SWFReader *read, u32 nb_frames);
	GF_Err (*define_text)(SWFReader *read, SWFText *text);
	GF_Err (*define_edit_text)(SWFReader *read, SWFEditText *text);
	/*@button is NULL to signal end of button declaration, non-null otherwise. "action" callback will be 
	called inbetween*/
	GF_Err (*define_button)(SWFReader *read, SWF_Button *button);

	GF_Err (*setup_image)(SWFReader *read, u32 ID, char *fileName);
	/*called whenever a sound is found. For soundstreams, called twice, once on the header (declaration), 
	and one on the first soundstream block for offset signaling*/
	GF_Err (*setup_sound)(SWFReader *read, SWFSound *snd, Bool soundstream_first_block);
	GF_Err (*start_sound)(SWFReader *read, SWFSound *snd, Bool stop);
	/*performs an action, returns 0 if action not supported*/
	Bool (*action)(SWFReader *read, SWFAction *act);

	void (*finalize)(SWFReader *read);


	/* <BIFS conversion state> */

	/*all simple appearances (no texture)*/
	GF_List *apps;

	GF_List *buttons;

	/*current BIFS stream*/
	GF_StreamContext *bifs_es;
	GF_AUContext *bifs_au;

	GF_StreamContext *bifs_dict_es;
	GF_AUContext *bifs_dict_au;

	/*for sound insert*/
	GF_Node *root;

	/*current OD AU*/
	GF_StreamContext *od_es;
	GF_AUContext *od_au;

	GF_Node *cur_shape;
	u16 prev_od_id, prev_es_id;

	u32 wait_frame;
	SWF_Button *btn;
	GF_List *btn_over, *btn_not_over, *btn_active, *btn_not_active;

	/* </BIFS conversion state> */
};


void swf_report(SWFReader *read, GF_Err e, char *format, ...);
SWFFont *swf_find_font(SWFReader *read, u32 fontID);
GF_Err swf_parse_sprite(SWFReader *read);


GF_Err swf_to_bifs_init(SWFReader *read);



typedef struct
{
	Fixed x, y;
	Fixed w, h;
} SWFRec;

typedef struct
{
	/*0: not defined, otherwise index of shape*/
	u32 nbType;
	/*0: moveTo, 1: lineTo, 2: quad curveTo*/
	u32 *types;
	SFVec2f *pts;
	u32 nbPts;
	/*used by SWF->BIFS for IndexedCurveSet*/
	u32 *idx;
} SWFPath;

typedef struct
{
	u32 type;
	u32 solid_col;
	u32 nbGrad;
	u32 *grad_col;
	u8 *grad_ratio;
	GF_Matrix2D mat;
	u32 img_id;
	Fixed width;

	SWFPath *path;
} SWFShapeRec;

struct SWFShape 
{
	GF_List *fill_left, *fill_right, *lines;
	u32 ID;
	SWFRec rc;
};

/*SWF font object*/
struct SWFFont
{
	u32 fontID;
	u32 nbGlyphs;
	GF_List *glyphs;

	/*the following may all be overridden by a DefineFontInfo*/

	/*index -> glyph code*/
	u16 *glyph_codes;
	/*index -> glyph advance*/
	s16 *glyph_adv;

	/*font flags (SWF 3.0)*/
	Bool has_layout;
	Bool has_shiftJIS;
	Bool is_unicode, is_ansi;
	Bool is_bold, is_italic;
	s16 ascent, descent, leading;

	/*font familly*/
	char *fontName;
};

/*chunk of text with the same aspect (font, col)*/
typedef struct
{
	u32 fontID;
	u32 col;
	/*font size*/
	u32 fontSize;
	/*origin point in local metrics*/
	Fixed orig_x, orig_y;

	u32 nbGlyphs;
	u32 *indexes;
	Fixed *dx;
} SWFGlyphRec;

struct SWFText
{
	u32 ID;
	GF_Matrix2D mat;
	GF_List *text;
};

struct SWFEditText 
{
	u32 ID;
	char *init_value;
	SWFRec bounds;
	Bool word_wrap, multiline, password, read_only, auto_size, no_select, html, outlines, has_layout, border;
	u32 color;
	Fixed max_length, font_height;
	u32 fontID;

	u32 align;
	Fixed left, right, indent, leading;
};


enum
{
	SWF_SND_UNCOMP = 0,
	SWF_SND_ADPCM,
	SWF_SND_MP3
};

struct SWFSound
{
	u32 ID;
	u8 format;
	/*0: 5.5k - 1: 11k - 2: 22k - 3: 44k*/
	u8 sound_rate;
	u8 bits_per_sample;
	Bool stereo;
	u16 sample_count;
	u32 frame_delay_ms;

	/*IO*/
	FILE *output;
	char *szFileName;

	/*set when sound is setup (OD inserted)*/
	Bool is_setup;
};

typedef struct 
{
	/*interaction states*/
	Bool hitTest, down, over, up;
	u32 character_id;
	u16 depth;
	GF_Matrix2D mx;
	GF_ColorMatrix cmx;
	Bool skip;
} SWF_ButtonRecord;


struct SWF_Button 
{
	u32 count;
	SWF_ButtonRecord buttons[40];
	u32 ID;
};

/*AS codes.*/
enum
{
	GF_SWF_AS3_GOTO_FRAME,
	GF_SWF_AS3_GET_URL,
	GF_SWF_AS3_NEXT_FRAME,
	GF_SWF_AS3_PREV_FRAME,
	GF_SWF_AS3_PLAY,
	GF_SWF_AS3_STOP,
	GF_SWF_AS3_TOGGLE_QUALITY,
	GF_SWF_AS3_STOP_SOUNDS,
	GF_SWF_AS3_WAIT_FOR_FRAME,
	GF_SWF_AS3_SET_TARGET,
	GF_SWF_AS3_GOTO_LABEL,
};

enum
{
	GF_SWF_COND_IDLE_TO_OVERDOWN = 1,
	GF_SWF_COND_OUTDOWN_TO_IDLE = 1<<1,
	GF_SWF_COND_OUTDOWN_TO_OVERDOWN = 1<<2,
	GF_SWF_COND_OVERDOWN_TO_OUTDOWN = 1<<3,
	GF_SWF_COND_OVERDOWN_TO_OUTUP = 1<<4,
	GF_SWF_COND_OVERUP_TO_OVERDOWN = 1<<5,
	GF_SWF_COND_OVERUP_TO_IDLE = 1<<6,
	GF_SWF_COND_IDLE_TO_OVERUP = 1<<7,
	GF_SWF_COND_OVERDOWN_TO_IDLE = 1<<8,
};

struct SWFAction
{
	u32 type;
	u32 frame_number;
	u32 button_mask, button_key;
	/*target (geturl/set_target), label (goto_frame)*/
	char *target;
	char *url;
};



#endif /*_GF_SWF_DEV_H_*/
