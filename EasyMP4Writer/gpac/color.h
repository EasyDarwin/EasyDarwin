/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005
 *					All rights reserved
 *
 *  This file is part of GPAC / common tools interfaces
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


#ifndef _GF_COLOR_H_
#define _GF_COLOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/constants.h>
#include <gpac/math.h>


/*!
 *\addtogroup color_grp color
 *\ingroup utils_grp
 *\brief Color tools
 *
 *This section documents color tools for image processing and color conversion
 *	@{
 */


/*!\brief Video framebuffer object
 *
 *The video framebuffer object represents uncompressed color data like images in a variety of formats. Data in
 *the video framebuffer MUST be continuous.
*/
typedef struct
{
	/*!Width of the video framebuffer */
	u32 width;
	/*!Height of the video framebuffer */
	u32 height;
	/*!Horizontal pitch of the video framebuffer (number of bytes to skip to go to next (right) pixel in the buffer). May be 
	negative for some framebuffers (embedded devices). 0 means linear frame buffer (pitch_x==bytes per pixel)*/
	s32 pitch_x;
	/*!Vertical pitch of the video framebuffer (number of bytes to skip to go down one line in the buffer). May be 
	negative for some framebuffers (embedded devices)*/
	s32 pitch_y;
	/*!Pixel format of the video framebuffer*/
	u32 pixel_format;
	/*!pointer to the begining of the video memory (top-left corner)*/
	char *video_buffer;
	/*!indicates that the video data reside on systems memory or video card one*/
	Bool is_hardware_memory;
} GF_VideoSurface;

/*!\brief Video Window object
 *
 *The video window object represents a rectangle in framebuffer coordinate system
*/
typedef struct
{
	/*!left-most coordinate of the rectangle*/
	u32 x;
	/*!top-most coordinate of the rectangle*/
	u32 y;
	/*!width of the rectangle*/
	u32 w;
	/*!height of the rectangle*/
	u32 h;
} GF_Window;


/*!\brief color matrix object
 *
 *The Color transformation matrix object allows complete color space transformation (shift, rotate, skew, add).\n
 *The matrix coefs are in rgba order, hence the color RGBA is transformed to:
 \code
	R'		m0	m1	m2	m3	m4			R
	G'		m5	m6	m7	m8	m9			G
	B'	=	m10	m11	m12	m13	m14		x	B
	A'		m15	m16	m17	m18	m19			A
	0		0	0	0	0	1			0
 \endcode
 *Coeficients are in intensity scale, ranging from 0 to \ref FIX_ONE.
*/
typedef struct
{
	/*!color matrix coefficient*/
	Fixed m[20];
	/*!internal flag to speed up things when matrix is identity. This is a read only flag, do not modify it*/
	u32 identity;
} GF_ColorMatrix;

/*!\brief ARGB color object
 *
 *The color type used in the GPAC framework represents colors in the form 0xAARRGGBB, with each component ranging from 0 to 255
*/
typedef u32 GF_Color;
/*!\hideinitializer color formating macro from alpha, red, green and blue components expressed as integers ranging from 0 to 255*/
#define GF_COL_ARGB(a, r, g, b) ((a)<<24 | (r)<<16 | (g)<<8 | (b))
/*!\hideinitializer color formating macro from alpha, red, green and blue components expressed as fixed numbers ranging from 0 to \ref FIX_ONE*/
#define GF_COL_ARGB_FIXED(_a, _r, _g, _b) GF_COL_ARGB(FIX2INT(255*(_a)), FIX2INT(255*(_r)), FIX2INT(255*(_g)), FIX2INT(255*(_b)))
/*!\hideinitializer gets alpha component of a color*/
#define GF_COL_A(c) (u8) ((c)>>24)
/*!\hideinitializer gets red component of a color*/
#define GF_COL_R(c) (u8) ( ((c)>>16) & 0xFF)
/*!\hideinitializer gets green component of a color*/
#define GF_COL_G(c) (u8) ( ((c)>>8) & 0xFF)
/*!\hideinitializer gets blue component of a color*/
#define GF_COL_B(c) (u8) ( (c) & 0xFF)
/*!\hideinitializer 16-bits color formating macro from red, green and blue components*/
#define GF_COL_565(r, g, b) (u16) (((r & 248)<<8) + ((g & 252)<<3)  + (b>>3))
/*!\hideinitializer 15-bits color formating macro from red, green and blue components*/
#define GF_COL_555(r, g, b) (u16) (((r & 248)<<7) + ((g & 248)<<2)  + (b>>3))
/*!\hideinitializer 15-bits color formating macro from red, green and blue components*/
#define GF_COL_444(r, g, b) (u16) (((r & 240)<<4) + ((g & 240))  + ((b & 240)>>4))
/*!\hideinitializer 16-bits alphagrey color formating macro alpha and grey components*/
#define GF_COL_AG(a, g) (u16) ( (a << 8) | g)
/*!\hideinitializer transfoms a 32-bits color into a 16-bits one.\note alpha component is lost*/
#define GF_COL_TO_565(c) (((GF_COL_R(c) & 248)<<8) + ((GF_COL_G(c) & 252)<<3)  + (GF_COL_B(c)>>3))
/*!\hideinitializer transfoms a 32-bits color into a 15-bits one.\note alpha component is lost*/
#define GF_COL_TO_555(c) (((GF_COL_R(c) & 248)<<7) + ((GF_COL_G(c) & 248)<<2)  + (GF_COL_B(c)>>3))
/*!\hideinitializer transfoms a 32-bits color into a 16-bits alphagrey one.\note red component is used for grey, green and blue components are lost.*/
#define GF_COL_TO_AG(c) ( (GF_COL_A(c) << 8) | GF_COL_R(c))
/*!\hideinitializer transfoms a 32-bits color into a 15-bits one.\note alpha component is lost*/
#define GF_COL_TO_444(c) (((GF_COL_R(c) & 240)<<4) + ((GF_COL_G(c) & 240))  + ((GF_COL_B(c)>>4) & 240) )

/*!Inits a color matrix to identity*/
void gf_cmx_init(GF_ColorMatrix *_this);
/*!Inits all coefficients of a color matrix 
 *\param _this color matrix to initialize
 *\param coefs list of the 20 fixed numbers to copy
*/
void gf_cmx_set_all(GF_ColorMatrix *_this, Fixed *coefs);
/*!Inits all coefficients of a color matrix 
 *\param _this color matrix to initialize
 *\param mrr red-to-red multiplication factor
 *\param mrg red-to-green multiplication factor
 *\param mrb red-to-blue multiplication factor
 *\param mra red-to-alpha multiplication factor
 *\param tr red translation factor
 *\param mgr green-to-red multiplication factor
 *\param mgg green-to-green multiplication factor
 *\param mgb green-to-blue multiplication factor
 *\param mga green-to-alpha multiplication factor
 *\param tg green translation factor
 *\param mbr blue-to-red multiplication factor
 *\param mbg blue-to-green multiplication factor
 *\param mbb blue-to-blue multiplication factor
 *\param mba blue-to-alpha multiplication factor
 *\param tb blue translation factor
 *\param mar alpha-to-red multiplication factor
 *\param mag alpha-to-green multiplication factor
 *\param mab alpha-to-blue multiplication factor
 *\param maa alpha-to-alpha multiplication factor
 *\param ta alpha translation factor
*/
void gf_cmx_set(GF_ColorMatrix *_this, 
				 Fixed mrr, Fixed mrg, Fixed mrb, Fixed mra, Fixed tr,
				 Fixed mgr, Fixed mgg, Fixed mgb, Fixed mga, Fixed tg,
				 Fixed mbr, Fixed mbg, Fixed mbb, Fixed mba, Fixed tb,
				 Fixed mar, Fixed mag, Fixed mab, Fixed maa, Fixed ta);
/*!Inits a matrix from another matrix
 *\param _this color matrix to initialize
 *\param from color matrix to copy from
*/
void gf_cmx_copy(GF_ColorMatrix *_this, GF_ColorMatrix *from);
/*!\brief color matrix multiplication
 *
 *Multiplies a color matrix by another one. Result is _this*with
 *\param _this color matrix to transform. Once the function called, _this will contain the resulting color matrix
 *\param with color matrix to add
*/
void gf_cmx_multiply(GF_ColorMatrix *_this, GF_ColorMatrix *with);
/*!\brief color matrix transform
 *
 *Transforms a color with a given color matrix
 *\param _this color matrix to use.
 *\param col color to transform
 *\return transformed color
*/
GF_Color gf_cmx_apply(GF_ColorMatrix *_this, GF_Color col);
/*!\brief color components matrix transform
 *
 *Transforms color components with a given color matrix
 *\param _this color matrix to use.
 *\param a pointer to alpha component. Once the function is called, a contains the transformed alpha component
 *\param r pointer to red component. Once the function is called, r contains the transformed red component
 *\param g pointer to green component. Once the function is called, g contains the transformed green component
 *\param b pointer to blue component. Once the function is called, b contains the transformed blue component
*/
void gf_cmx_apply_fixed(GF_ColorMatrix *_this, Fixed *a, Fixed *r, Fixed *g, Fixed *b);


/*!\brief Color Key descriptor
 *
 *The ColorKey object represents a ColorKey with low and high threshold keying
*/
typedef struct
{
	/*!color key R, G, and B components*/
	u8 r, g, b;
	/*!Alpha value for opaque (non-keyed) pixels*/
	u8 alpha;
	/*!low variance threshold*/
	u8 low;
	/*!high variance threshold*/
	u8 high;
} GF_ColorKey;

/*!\brief not done yet
 *
 */
GF_Err gf_stretch_bits(GF_VideoSurface *dst, GF_VideoSurface *src, GF_Window *dst_wnd, GF_Window *src_wnd, u8 alpha, Bool flip, GF_ColorKey *colorKey, GF_ColorMatrix * cmat);



/*! @} */


#ifdef __cplusplus
}
#endif


#endif	/*_GF_COLOR_H_*/

