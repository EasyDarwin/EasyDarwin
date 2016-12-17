/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005
 *					All rights reserved
 *
 *  This file is part of GPAC / modules interfaces
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


#ifndef _GF_MODULE_FONT_H_
#define _GF_MODULE_FONT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/path2d.h>
#include <gpac/module.h>
#include <gpac/user.h>


typedef struct _gf_glyph
{
	/*glyphs are stored as linked lists*/
	struct _gf_glyph *next;
	/*glyph ID as used in *_get_glyphs - this may not match the UTF name*/
	u32 ID;
	/*UTF-name of the glyph if any*/
	u32 utf_name;
	GF_Path *path;
	/*width of the glyph - !! this can be more than the horizontal advance !! */
	u32 width;
	/*glyph horizontal advance in font EM size*/
	s32 horiz_advance;
	/*height of the glyph - !! this can be more than the vertical advance !! */
	u32 height;
	/*glyph vertical advance in font EM size*/
	s32 vert_advance;
} GF_Glyph;

enum
{
	/*font styles*/
	GF_FONT_ITALIC = 1,
	GF_FONT_OBLIQUE = 1<<1,
	/*font variant (smallcaps)*/
	GF_FONT_SMALLCAPS = 1<<2,
	/*font decoration*/
	GF_FONT_UNDERLINED = 1<<3,
	GF_FONT_STRIKEOUT = 1<<4,

	/*all font weight modification are placed AFTER 1<<9*/
	GF_FONT_WEIGHT_100 = 1<<10,
	GF_FONT_WEIGHT_LIGHTER = 1<<11,
	GF_FONT_WEIGHT_200 = 1<<12,
	GF_FONT_WEIGHT_300 = 1<<13,
	GF_FONT_WEIGHT_400 = 1<<14,
	GF_FONT_WEIGHT_NORMAL = 1<<15,
	GF_FONT_WEIGHT_500 = 1<<16,
	GF_FONT_WEIGHT_600 = 1<<17,
	GF_FONT_WEIGHT_700 = 1<<18,
	GF_FONT_WEIGHT_BOLD = 1<<19,
	GF_FONT_WEIGHT_800 = 1<<20,
	GF_FONT_WEIGHT_900 = 1<<21,
	GF_FONT_WEIGHT_BOLDER = 1<<22
};
/*mask for font styles used for CSS2 selection*/
#define GF_FONT_STYLE_MASK	0x00000007
/*mask for all font weight*/
#define GF_FONT_WEIGHT_MASK	0xFFFFFC00

/*interface name and version for font engine*/
#define GF_FONT_READER_INTERFACE		GF_4CC('G','F','T', '1')


typedef struct _font_reader
{
	/* interface declaration*/
	GF_DECL_MODULE_INTERFACE

	/*inits font engine.*/
	GF_Err (*init_font_engine)(struct _font_reader *dr);
	/*shutdown font engine*/
	GF_Err (*shutdown_font_engine)(struct _font_reader *dr);

	/*set active font . @styles indicates font styles (PLAIN, BOLD, ITALIC, 
	BOLDITALIC and UNDERLINED, STRIKEOUT)*/
	GF_Err (*set_font)(struct _font_reader *dr, const char *fontName, u32 styles);
	/*gets font info*/
	GF_Err (*get_font_info)(struct _font_reader *dr, char **font_name, s32 *em_size, s32 *ascent, s32 *descent, s32 *underline, s32 *line_spacing, s32 *max_advance_h, s32 *max_advance_v);

	/*translate string to glyph sequence*/
	GF_Err (*get_glyphs)(struct _font_reader *dr, const char *utf_string, u32 *glyph_id_buffer, u32 *io_glyph_id_buffer_size, const char *xml_lang, Bool *rev_layout);

	/*loads glyph by name - returns NULL if glyph cannot be found*/
	GF_Glyph *(*load_glyph)(struct _font_reader *dr, u32 glyph_name);

/*module private*/
	void *udta;
} GF_FontReader;



#ifdef __cplusplus
}
#endif


#endif	/*_GF_MODULE_FONT_H_*/

