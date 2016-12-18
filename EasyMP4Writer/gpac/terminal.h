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

#ifndef _GF_TERMINAL_H_
#define _GF_TERMINAL_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/user.h>

/*creates a new terminal for a userApp callback*/
GF_Terminal *gf_term_new(GF_User *user);

/*delete the app - stop is done automatically, you don't have to do it before deleting the app
returns GF_IO_ERR if client couldn't be shutdown normally*/
GF_Err gf_term_del(GF_Terminal *term);

/* Browses all registered relocators (ZIP-based, ISOFF-based or file-system-based to relocate a URL based on the user locale */
Bool gf_term_relocate_url(GF_Terminal *term, const char *service_url, const char *parent_url, char *relocated_url, char *localized_url);
/*connects to a URL*/
void gf_term_connect(GF_Terminal *term, const char *URL);
/*disconnects the url*/
void gf_term_disconnect(GF_Terminal *term);
/*navigates to a given destination or shutdown/restart the current one if any.
This is the only safe way of restarting/jumping a presentation from inside the EventProc 
where doing a disconnect/connect could deadlock if toURL is NULL, uses the current URL*/
void gf_term_navigate_to(GF_Terminal *term, const char *toURL);
/*restarts url from given time (in ms). 
@pause_at_first_frame: if 1, pauses at the first frame. If 2, pauses at the first frame only if the terminal is in paused state.
Return value: 
	0: service is not connected yet
	1: service has no seeking capabilities
	2: service has been seeked
*/
u32 gf_term_play_from_time(GF_Terminal *term, u64 from_time, u32 pause_at_first_frame);
/*connect URL and seek right away - only needed when reloading the complete player (avoids waiting
for connection and post a seek..)*/
void gf_term_connect_from_time(GF_Terminal *term, const char *URL, u64 time_in_ms, Bool pause_at_first_frame);

/*same as gf_term_connect but specifies a parent path*/
void gf_term_connect_with_path(GF_Terminal *term, const char *URL, const char *parent_URL);

/*returns current framerate
	if @absoluteFPS is set, the return value is the absolute framerate, eg NbFrameCount/NbTimeSpent regardless of
whether a frame has been drawn or not, which means the FPS returned can be much greater than the target rendering 
framerate
	if @absoluteFPS is not set, the return value is the FPS taking into account not drawn frames (eg, less than 
	or equal to compositor FPS)
*/
Double gf_term_get_framerate(GF_Terminal *term, Bool absoluteFPS);
/*get main scene current time in milliseconds*/
u32 gf_term_get_time_in_ms(GF_Terminal *term);

/*returns current URL address*/
const char *gf_term_get_url(GF_Terminal *term);

/*get viewpoints/viewports for main scene - idx is 1-based, and if greater than number of viewpoints return GF_EOS*/
GF_Err gf_term_get_viewpoint(GF_Terminal *term, u32 viewpoint_idx, const char **outName, Bool *is_bound);
/*set active viewpoints/viewports for main scene given its name - idx is 1-based, or 0 to set by viewpoint name
if only one viewpoint is present in the scene, this will bind/unbind it*/
GF_Err gf_term_set_viewpoint(GF_Terminal *term, u32 viewpoint_idx, const char *viewpoint_name);

/*adds an object to the scene - only possible if scene has selectable streams (cf GF_OPT_CAN_SELECT_STREAMS option)*/
GF_Err gf_term_add_object(GF_Terminal *term, const char *url, Bool auto_play);


/*set/set option - most of the terminal cfg is done through options, please refer to options.h for details*/
GF_Err gf_term_set_option(GF_Terminal *term, u32 opt_type, u32 opt_value);
u32 gf_term_get_option(GF_Terminal *term, u32 opt_type);

/*checks if given URL is understood by client.
if use_parent_url is set, relative URLs are solved against the current presentation URL*/
Bool gf_term_is_supported_url(GF_Terminal *term, const char *fileName, Bool use_parent_url, Bool no_mime_check);

/*returns the current service ID for MPEG-2 TS mux - returns 0 if no service ID is associated (or not loaded yet)*/
u32 gf_term_get_current_service_id(GF_Terminal *term);

/*sets simulation frame rate*/
GF_Err gf_term_set_simulation_frame_rate(GF_Terminal * term, Double frame_rate);
/*gets simulation frame rate*/
Double gf_term_get_simulation_frame_rate(GF_Terminal *term);

/*process shortcuts*/
void gf_term_process_shortcut(GF_Terminal *term, GF_Event *ev);

void gf_term_set_speed(GF_Terminal *term, Fixed speed);

/*sends a set of scene commands (BT, XMT, X3D, LASeR+XML) to the scene
type indicates the language used - accepted values are 
	"model/x3d+xml" or "x3d": commands is an X3D+XML scene
	"model/x3d+vrml" or  "xrdv": commands is an X3D+VRML scene
	"model/vrml" or "vrml": commands is an VRML scene
	"application/x-xmt" or "xmt": commands is an XMT-A scene or a list of XMT-A updates
	"application/x-bt" or "bt": commands is a BT scene or a list of BT updates
	"image/svg+xml" or "svg": commands is an SVG scene
	"application/x-laser+xml" or "laser": commands is an SVG/LASeR+XML  scene or a list of LASeR+XML updates
	if not specified, the type will be guessed from the current root node if any
*/
GF_Err gf_term_scene_update(GF_Terminal *term, char *type, char *com);


/*request visual output size change:
	* NOT NEEDED WHEN THE TERMINAL IS HANDLING THE DISPLAY WINDOW (cf user.h)
	* if the user app manages the output window it shall resize it before calling this
*/
GF_Err gf_term_set_size(GF_Terminal *term, u32 NewWidth, u32 NewHeight);

/*returns current text selection if any, or NULL otherwise. If probe mode is set, returns a non-NULL string ("") 
if some text is selected*/
const char *gf_term_get_text_selection(GF_Terminal *term, Bool probe_only);
/*pastes text into current selection if any. If probe mode is set, only check if text is currently edited
if some text is selected*/
GF_Err gf_term_paste_text(GF_Terminal *term, const char *txt, Bool probe_only);


/*decodes pending media and render frame. 
NOTE: This can only be used when the terminal runs without visual thread (GF_TERM_NO_VISUAL_THREAD flag set)
*/
GF_Err gf_term_process_step(GF_Terminal *term);

/*decodes all pending media and render frame until no scene changes are detected.
NOTE: This can only be used when the terminal runs without visual thread (GF_TERM_NO_VISUAL_THREAD flag set)
*/
GF_Err gf_term_process_flush(GF_Terminal *term);

/*post user interaction to terminal*/
/*NOT NEEDED WHEN THE TERMINAL IS HANDLING THE DISPLAY WINDOW (cf user.h)*/
Bool gf_term_user_event(GF_Terminal *term, GF_Event *event);

/*post extended user mouse interaction to terminal 
	X and Y are point coordinates in the display expressed in 2D coord system top-left (0,0), Y increasing towards bottom
	@xxx_but_down: specifiy whether the mouse button is down(2) or up (1), 0 if unchanged
	@wheel: specifiy current wheel inc (0: unchanged , +1 for one wheel delta forward, -1 for one wheel delta backward)
*/
/*NOT NEEDED WHEN THE TERMINAL IS HANDLING THE DISPLAY WINDOW (cf user.h)*/
void gf_term_mouse_input(GF_Terminal *term, GF_EventMouse *event);

/*post extended user key interaction to terminal 
	@key_code: GPAC DOM code of input key
	@hw_code: hardware code of input key
	@isKeyUp: set if key is released
*/
/*NOT NEEDED WHEN THE TERMINAL IS HANDLING THE DISPLAY WINDOW (cf user.h)*/
void gf_term_keyboard_input(GF_Terminal *term, u32 key_code, u32 hw_code, Bool isKeyUp);

/*post extended user character interaction to terminal 
	@character: unicode character input
*/
/*NOT NEEDED WHEN THE TERMINAL IS HANDLING THE DISPLAY WINDOW (cf user.h)*/
void gf_term_string_input(GF_Terminal *term, u32 character);



/*framebuffer access*/
#include <gpac/color.h>

/*gets screen buffer - this locks the scene graph too until released is called*/
GF_Err gf_term_get_screen_buffer(GF_Terminal *term, GF_VideoSurface *framebuffer);
/*releases screen buffer and unlocks graph*/
GF_Err gf_term_release_screen_buffer(GF_Terminal *term, GF_VideoSurface *framebuffer);

/*gets view buffer - this locks the scene graph too until released is called
view_idx ranges from 0 to GF_OPT_NUM_STEREO_VIEWS value
the buffer is released by calling gf_term_release_screen_buffer*/
GF_Err gf_term_get_offscreen_buffer(GF_Terminal *term, GF_VideoSurface *framebuffer, u32 view_idx, u32 depth_buffer_type);


/*ObjectManager used by both terminal and object browser (term_info.h)*/
typedef struct _od_manager GF_ObjectManager;

/*switches quality up or down - can be called several time in the same direction
this will call all decoders to adjust their quality levels
VERY BASIC INTERFACE*/
void gf_term_switch_quality(GF_Terminal *term, Bool up);

#ifdef __cplusplus
}
#endif


#endif	/*_GF_TERMINAL_H_*/
