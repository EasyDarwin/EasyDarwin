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


#ifndef _GF_MODULE_RASTER2D_H_
#define _GF_MODULE_RASTER2D_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/path2d.h>
#include <gpac/module.h>
#include <gpac/color.h>


/*stencil types*/
typedef enum
{
	/*solid color stencil*/
	GF_STENCIL_SOLID = 0,
	/*linear color gradient stencil*/
	GF_STENCIL_LINEAR_GRADIENT,
	/*radial color gradient stencil*/
	GF_STENCIL_RADIAL_GRADIENT,
	/*texture stencil*/
	GF_STENCIL_VERTEX_GRADIENT,
	/*texture stencil*/
	GF_STENCIL_TEXTURE,
} GF_StencilType;


/*gradient filling modes*/
typedef enum
{
	/*edge colors are repeated until path is filled*/
	GF_GRADIENT_MODE_PAD,
	/*pattern is inversed each time it's repeated*/
	GF_GRADIENT_MODE_SPREAD,
	/*pattern is repeated to fill path*/
	GF_GRADIENT_MODE_REPEAT
} GF_GradientMode;


/*texture tiling flags*/
typedef enum
{
	/*texture is repeated in its horizontal direction*/
	GF_TEXTURE_REPEAT_S = (1<<1),
	/*texture is repeated in its horizontal direction*/
	GF_TEXTURE_REPEAT_T = (1<<2),
	/*texture is fliped vertically*/
	GF_TEXTURE_FLIP = (1<<3),
} GF_TextureTiling;

/*filter levels for texturing - up to the graphics engine but the following levels are used by
the client*/
typedef enum
{
	/*high speed mapping (ex, no filtering applied)*/
	GF_TEXTURE_FILTER_HIGH_SPEED,
	/*compromise between speed and quality (ex, filter to nearest pixel)*/
	GF_TEXTURE_FILTER_MID,
	/*high quality mapping (ex, bi-linear/bi-cubic interpolation)*/
	GF_TEXTURE_FILTER_HIGH_QUALITY
} GF_TextureFilter;

/* rasterizer antialiasing depending on the graphics engine*/
typedef enum
{
	/*raster shall use no antialiasing */
	GF_RASTER_HIGH_SPEED,
	/*raster should use fast mode and good quality if possible*/
	GF_RASTER_MID,
	/*raster should use full antialiasing*/
	GF_RASTER_HIGH_QUALITY
} GF_RasterLevel;


/*user routines for raserizer. common syntaxes:
	@cbk: user defined callback
	@x, y: first pixel position of the run, in device memory (top-left) coordinates
	@run_h_len: number of pixels to fill on line
	@color: color to fill pixel with. USER MUST IGNORE THE ALPHA COMPONENT OF THIS COLOR, the final 
		alpha is computed by the lib
	@alpha: blending amount (0->0xFF) for the pixels
*/

typedef void (*raster_cbk_fill_run_alpha) (void *, u32, u32, u32, GF_Color, u8);
typedef void (*raster_cbk_fill_run_no_alpha) (void *, u32, u32, u32, GF_Color);
typedef void (*raster_cbk_fill_rect)(void *cbk, u32 x, u32 y, u32 width, u32 height, GF_Color color);

typedef struct
{
	void *cbk;
	/*fills line pixels without any blending operation*/
	raster_cbk_fill_run_no_alpha fill_run_no_alpha;
	/* fills line pixels with blending operation - alpha combines both fill color and anti-aliasing blending */
	raster_cbk_fill_run_alpha fill_run_alpha;
	/*fills rectangle*/
	raster_cbk_fill_rect fill_rect;
} GF_RasterCallback;



/*opaque handler for all stencils*/
typedef void *GF_STENCIL;

/*visual surface handler*/
typedef void *GF_SURFACE;

/*interface name and version for raster2D*/
#define GF_RASTER_2D_INTERFACE		GF_4CC('G','R','2', '2')

/*graphics driver*/
typedef struct _raster2d_interface
{
	/* interface declaration*/
	GF_DECL_MODULE_INTERFACE

	GF_STENCIL (*stencil_new) (struct _raster2d_interface *, GF_StencilType type);
	/*common destructor for all stencils*/
	void (*stencil_delete) (GF_STENCIL _this);
	/*set stencil transformation matrix*/
	GF_Err (*stencil_set_matrix) (GF_STENCIL _this, GF_Matrix2D *mat);
	/*solid brush - set brush color*/
	GF_Err (*stencil_set_brush_color) (GF_STENCIL _this, GF_Color c);
	/*gradient brushes*/
	/*sets gradient repeat mode - return GF_NOT_SUPPORTED if driver doesn't support this to let the app compute repeat patterns
	this may be called before the gradient is setup*/
	GF_Err (*stencil_set_gradient_mode) (GF_STENCIL _this, GF_GradientMode mode);
	/*set linear gradient.  line is defined by start and end, and you can give interpolation colors at specified positions*/
	GF_Err (*stencil_set_linear_gradient) (GF_STENCIL _this, Fixed start_x, Fixed start_y, Fixed end_x, Fixed end_y);
	/*radial gradient brush center point, focal point and radius - colors can only be set through set_interpolation */
	GF_Err (*stencil_set_radial_gradient) (GF_STENCIL _this, Fixed cx, Fixed cy, Fixed fx, Fixed fy, Fixed x_radius, Fixed y_radius);
	/*radial and linear gradient (not used with vertex) - set color interpolation at given points, 
		@pos[i]: distance from (center for radial, start for linear) expressed between 0 and 1 (1 being the gradient bounds)
		@col[i]: associated color
	NOTE 1: the colors at 0 and 1.0 MUST be provided
	NOTE 2: colors shall be fed in order from 0 to 1
	NOTE 3: this overrides the colors provided for linear gradient
	*/
	GF_Err (*stencil_set_gradient_interpolation) (GF_STENCIL _this, Fixed *pos, GF_Color *col, u32 count);

	/*vertex gradient : set limit path */
	GF_Err (*stencil_set_vertex_path) (GF_STENCIL _this, GF_Path *path);
	/*set the center of the gradient*/
	GF_Err (*stencil_set_vertex_center) (GF_STENCIL _this, Fixed cx, Fixed cy, u32 color);
	/*set the center of the gradient*/
	GF_Err (*stencil_set_vertex_colors) (GF_STENCIL _this, u32 *colors, u32 nbCol);
	
	/*sets global alpha blending level for stencil (texture and gradients)
	the alpha channel shall be combined with the color matrix if any*/
	GF_Err (*stencil_set_alpha) (GF_STENCIL _this, u8 alpha);
	
	/*set stencil texture
		@pixels: texture data, from top to bottom
		@width, @height: texture size
		@stride: texture horizontal pitch (bytes to skip to get to next row)
		@pixelFormat: texture pixel format as defined in file constants.h
		@destination_format_hint: this is the current pixel format of the destination surface, and is given
		as a hint in case the texture needs to be converted by the stencil
		@no_copy: if set, specifies the texture data shall not be cached by the module (eg it must be able
		to directly modify the given memory
	NOTE: this stencil acts as a data wrapper, the pixel data is not required to be locally copied
	data is not required to be available for texturing until the stencil is used in a draw operation
	*/
	GF_Err (*stencil_set_texture) (GF_STENCIL _this, char *pixels, u32 width, u32 height, u32 stride, GF_PixelFormat pixelFormat, GF_PixelFormat destination_format_hint, Bool no_copy);
	/*creates internal texture - pixel data is owned by texture brush - set to NULL if not supported - this is used to 
	cope with engines that don't support random strides (ex: Gdiplus needs stride to be a multiple of 4) 
	if not set the compositor will create its own mem texture and pass it through set_texture - pixel format shall 
	be respected as far as Alpha is concerned (eg alpha info shall be kept and used in blit) */
	GF_Err (*stencil_create_texture) (GF_STENCIL _this, u32 width, u32 height, GF_PixelFormat pixelFormat);
	/*signals the texture has been modified (internal texture only)*/
	void (*stencil_texture_modified) (GF_STENCIL _this);

	/*sets texture tile mode*/
	GF_Err (*stencil_set_tiling) (GF_STENCIL _this, GF_TextureTiling mode);
	/*sets texture filtering mode*/
	GF_Err (*stencil_set_filter) (GF_STENCIL _this, GF_TextureFilter filter_mode);
	/*set stencil color matrix - texture stencils only. If matrix is NULL, resets current color matrix*/
	GF_Err (*stencil_set_color_matrix) (GF_STENCIL _this, GF_ColorMatrix *cmat);

	/*creates surface object*/
	/* @center_coords: true indicates mathematical-like coord system, 
					   false indicates computer-like coord system */
	GF_SURFACE (*surface_new) (struct _raster2d_interface *, Bool center_coords);
	/* delete surface object */
	void (*surface_delete) (GF_SURFACE _this);

	/* attach surface object to device object (Win32: HDC) width and height are target surface size*/
	GF_Err (*surface_attach_to_device) (GF_SURFACE _this, void *os_handle, u32 width, u32 height);
	/* attach surface object to stencil object*/
	GF_Err (*surface_attach_to_texture) (GF_SURFACE _this, GF_STENCIL sten);
	/* attach surface object to memory buffer if supported
		@pixels: texture data
		@width, @height: texture size
		@pitch_x: texture horizontal pitch (bytes to skip to get to next pixel). O means linear frame buffer (eg pitch_x==bytes per pixel)
		@pitch_y: texture vertical pitch (bytes to skip to get to next line)
		@pixelFormat: texture pixel format
	*/
	GF_Err (*surface_attach_to_buffer) (GF_SURFACE _this, char *pixels, u32 width, u32 height, s32 pitch_x, s32 pitch_y, GF_PixelFormat pixelFormat);

	GF_Err (*surface_attach_to_callbacks) (GF_SURFACE _this, GF_RasterCallback *callbacks, u32 width, u32 height);

	/* detach surface object */
	void (*surface_detach) (GF_SURFACE _this);

	/*sets rasterizer precision */
	GF_Err (*surface_set_raster_level) (GF_SURFACE _this, GF_RasterLevel RasterSetting);
	/* set the given matrix as the current transformations for all drawn paths
	if NULL reset the current transformation */
	GF_Err (*surface_set_matrix) (GF_SURFACE _this, GF_Matrix2D *mat);
	/* set the given rectangle as a clipper - nothing will be drawn outside this clipper
	if the clipper is NULL then no clipper is set
	NB: the clipper is not affected by the surface matrix and is given in pixels
	CF ABOVE NOTE ON CLIPPERS*/
	GF_Err (*surface_set_clipper) (GF_SURFACE _this, GF_IRect *rc);

	/*sets the given path as the current one for drawing - the surface transform is NEVER changed between
	setting the path and filling, only the clipper may change*/
	GF_Err (*surface_set_path) (GF_SURFACE _this, GF_Path *path);
	/*fills the current path using the given stencil - can be called several times with the same current path*/
	GF_Err (*surface_fill) (GF_SURFACE _this, GF_STENCIL stencil);

	/*flushes to surface*/
	GF_Err (*surface_flush) (GF_SURFACE _this);

	/*clears given pixel rect on the surface with the given color - REQUIRED
	the given rect is formatted as a clipper - CF ABOVE NOTE ON CLIPPERS*/
	GF_Err (*surface_clear)(GF_SURFACE _this, GF_IRect *rc, GF_Color col);

/*private:*/
	void *internal;
} GF_Raster2D;



#ifdef __cplusplus
}
#endif


#endif	/*_GF_MODULE_RASTER2D_H_*/

