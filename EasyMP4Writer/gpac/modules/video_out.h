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


/*

		Note on video driver: this is not a graphics driver, the only thing requested from this driver
	is accessing video memory and performing stretch of YUV and RGB on the backbuffer (bitmap node)
	the graphics driver is a different entity that performs 2D rasterization

*/

#ifndef _GF_MODULE_VIDEO_OUT_H_
#define _GF_MODULE_VIDEO_OUT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*include module system*/
#include <gpac/module.h>
/*include event system*/
#include <gpac/events.h>
/*include framebuffer definition*/
#include <gpac/color.h>

/*
		Video hardware output module
*/

enum
{
	/*HW supports RGB->backbuffer blitting*/
	GF_VIDEO_HW_HAS_RGB = (1<<1),
	/*HW supports RGBA->backbuffer blitting*/
	GF_VIDEO_HW_HAS_RGBA = (1<<2),
	/*HW supports YUV->backbuffer blitting*/
	GF_VIDEO_HW_HAS_YUV = (1<<3),
	/*HW supports YUV overlays*/
	GF_VIDEO_HW_HAS_YUV_OVERLAY = (1<<4),
	/*HW supports stretching for RGB and YUV buffers*/
	GF_VIDEO_HW_HAS_STRETCH	= (1<<5),
	/*HW supports OpenGL rendering. Whether this is OpenGL or OpenGL-ES depends on compilation settings
	and cannot be changed at runtime*/
	GF_VIDEO_HW_OPENGL = (1<<6),
	/*HW supports OpenGL offscreen rendering. Whether this is OpenGL or OpenGL-ES depends on compilation settings
	and cannot be changed at runtime*/
	GF_VIDEO_HW_OPENGL_OFFSCREEN = (1<<7),
	/*HW supports OpenGL offscreen rendering with alpha. Whether this is OpenGL or OpenGL-ES depends on compilation settings
	and cannot be changed at runtime*/
	GF_VIDEO_HW_OPENGL_OFFSCREEN_ALPHA = (1<<8),

	/*HW supports RGB+Depth or YUV+Depth blitting*/
	GF_VIDEO_HW_HAS_DEPTH = (1<<9),

	/*HW supports line blitting*/
	GF_VIDEO_HW_HAS_LINE_BLIT = (1<<15),
	/*HW supports locking a surface by device context (Win32 only)*/
	GF_VIDEO_HW_HAS_HWND_HDC	= (1<<16),
	/*HW only supports direct rendering mode*/
	GF_VIDEO_HW_DIRECT_ONLY	= (1<<17),
};

typedef struct
{	
	GF_IRect *list;
	u32 count;
} GF_DirtyRectangles;

typedef struct _gf_sc_texture_handler GF_TextureH;

/*interface name and version for video output*/
#define GF_VIDEO_OUTPUT_INTERFACE	GF_4CC('G','V','O','1') 

/*
			video output interface

	the video output may run in 2 modes: 2D and 3D.

	** the 2D video output works by accessing a backbuffer surface on the video mem board - 
	the app accesses to the surface through the GF_VideoSurface handler. 
	The module may support HW blitting of RGB or YUV data to backbuffer.

	** the 3D video output only handles window management and openGL contexts setup.
	The context shall be setup in Resize and SetFullScreen calls which are always happening in the main 
	rendering thread. This will take care of openGL context issues with multithreading

	By default all modules are required to be setup in 2D. If 3D is needed, a GF_EVENT_VIDEO_SETUP will
	be sent with the desired configuration.

	Except Setup and Shutdown functions, all interface functions are called through the main compositor thread
	or its user to avoid multithreading issues. Care must still be taken when handling events
*/
typedef struct _video_out
{
	/* interface declaration*/
	GF_DECL_MODULE_INTERFACE

	/*setup system - if os_handle is NULL the driver shall create the output display (common case)
	the other case is currently only used by child windows on win32 and winCE
	@init_flags: a list of initialization flags as specified in user.h*/
	GF_Err (*Setup)(struct _video_out *vout, void *os_handle, void *os_display, u32 init_flags);
	/*shutdown system */
	void (*Shutdown) (struct _video_out *vout);

	/*flush video: the video shall be presented to screen 
	the destination area to update is in client display coordinates (0,0) being top-left, (w,h) bottom-right
	Note: dest is always NULL in 3D mode (buffer flip only)*/
	GF_Err (*Flush) (struct _video_out *vout, GF_Window *dest);

	GF_Err (*SetFullScreen) (struct _video_out *vout, Bool fs_on, u32 *new_disp_width, u32 *new_disp_height);

	/*window events sent to output:
	GF_EVENT_SET_CURSOR: sets cursor
	GF_EVENT_SET_CAPTION: sets caption
	GF_EVENT_SHOWHIDE: show/hide output window for self-managed output
	GF_EVENT_SIZE:  inital window resize upon scene load
	GF_EVENT_VIDEO_SETUP: all HW related setup:
		* for 2D output, this means resizing the backbuffer if needed (depending on HW constraints)
		* for 3D output, this means re-setup of OpenGL context (depending on HW constraints).
			* This can be a request for an offscreen rendering surface. If supported, this surface SHALL
			be readable through glReadPixels. If not supported, just return an error.
			Note that GPAC never uses more than one GL context (offscreen or main video)
			* Depending on windowing systems and implementations, it could be possible to resize a window 
		without destroying the GL context. If the GL context is destroyed, the module should send an event
		of the same type to the player.
	
	This function is also called with a NULL event at the begining of each rendering cycle, in order to allow event 
	handling for modules uncapable of safe multithreading (eg X11)
	*/
	GF_Err (*ProcessEvent)(struct _video_out *vout, GF_Event *event);

	/*pass events to user (assigned before setup) - return 1 if the event has been processed by GPAC 
	(eiher scene or navigation), 0 otherwise*/
	void *evt_cbk_hdl;
	Bool (*on_event)(void *hdl, GF_Event *event);

	/*
			All the following are 2D specific and are NEVER called in 3D mode
	*/
	/*locks backbuffer video memory
	do_lock: specifies whether backbuffer shall be locked or released
	*/
	GF_Err (*LockBackBuffer)(struct _video_out *vout, GF_VideoSurface *video_info, Bool do_lock);

	/*lock video mem through OS context (only HDC for Win32 at the moment)
	do_lock: specifies whether OS context shall be locked or released*/
	void *(*LockOSContext)(struct _video_out *vout, Bool do_lock);

	/*blit surface src to backbuffer - if a window is not specified, the full surface is used
	the blitter MUST support stretching and RGB24 sources. Support for YUV is indicated in the hw caps
	of the driver. If none is supported, just set this function to NULL and let gpac performs software blitting.
	Whenever this function fails, the blit will be performed in software mode
	if is_overlay is set, this is an overlay on the video memory (Flush would have been called before)
		overlay_type 1: this is regular overlay without color keying
		overlay_type 2: this is overlay with color keying
	*/
	GF_Err (*Blit)(struct _video_out *vout, GF_VideoSurface *video_src, GF_Window *src_wnd, GF_Window *dst_wnd, u32 overlay_type);

	/*optional
	blits the texture as a bitmap with the specified transform cliped with the given cliper, with alpha and 
	color keying (NULL if no keying)
	*/
	Bool (*BlitTexture)(struct _video_out *vout, GF_TextureH *texture, GF_Matrix2D *transform, GF_IRect *clip, u8 alpha, GF_ColorKey *col_key, Fixed depth_offset, Fixed depth_gain);
	/*optional
		releases any HW resource used by the texture object due to a call to BlitTexture. This is called when
		the object is about to be destroyed or is no longer visible on screen
	*/
	void (*ReleaseTexture)(struct _video_out *vout, GF_TextureH *texture);

	/*optional
		flushes only the listed rectangles
	*/
	void (*FlushRectangles)(struct _video_out *vout, GF_DirtyRectangles *rectangles);
	
	/*ignored if GF_VIDEO_HW_HAS_LINE_BLIT is not set*/
	void (*DrawHLine)(struct _video_out *vout, u32 x, u32 y, u32 length, GF_Color color);
	void (*DrawHLineAlpha)(struct _video_out *vout, u32 x, u32 y, u32 length, GF_Color color, u8 alpha);
	void (*DrawRectangle)(struct _video_out *vout, u32 x, u32 y, u32 width, u32 height, GF_Color color);



	/*set of above HW flags*/
	u32 hw_caps;
	/*main pixel format of video board (informative only)*/
	u32 pixel_format;
	/*yuv pixel format if HW YUV blitting is supported (informative only) */
	u32 yuv_pixel_format;
	/*maximum resolution of the screen*/
	u32 max_screen_width, max_screen_height;
	/* dpi of the screen*/
	u32 dpi_x, dpi_y;

	/*overlay color key used by the hardware bliter - if not set, only top-level overlay can be used*/
	u32 overlay_color_key;

	/*for auto-stereoscopic output*/
    /*maximum pixel disparity*/
    u32 disparity;
    /*nominal display viewing distance in cm*/
    Fixed view_distance;

	/*driver private*/
	void *opaque;
} GF_VideoOutput;



#ifdef __cplusplus
}
#endif


#endif	/*_GF_MODULE_VIDEO_OUT_H_*/

