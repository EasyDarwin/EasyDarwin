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



#ifndef _GF_USER_H_
#define _GF_USER_H_

#ifdef __cplusplus
extern "C" {
#endif

//#include <gpac/math.h>
#include <gpac/events.h>
#include <gpac/module.h>

/*GPAC client terminal*/
typedef struct _tag_terminal GF_Terminal;
typedef struct _tag_user GF_User;
	

enum
{
	/*display should be hidden upon initialization*/
	GF_TERM_INIT_HIDE = 1,
	/*no audio renderer will be created*/
	GF_TERM_NO_AUDIO = 1<<1,
	/*terminal is used without visual threading: 
		* media codecs are not threaded
		* all composition memories are filled before rendering
		* rendering is done after media decoding
		* the user is responsible for updating the terminal
	*/
	GF_TERM_NO_DECODER_THREAD = 1<<2,
	
	/*works with no visual thread for the composition - compositor is driven by the media manager
		if GF_TERM_NO_DECODER_THREAD, equivalent to GF_TERM_NO_COMPOSITOR_THREAD
	*/
	GF_TERM_NO_VISUAL_THREAD = 1<<3,

	/*works with no visual thread for the composition - compositor is driven by gf_term_process*/
	GF_TERM_NO_COMPOSITOR_THREAD = 1<<4,

	/*disables frame-rate regulation (used when dumping content)*/
	GF_TERM_NO_REGULATION = 1<<5,
	/*works without window thread*/
	GF_TERM_WINDOW_NO_THREAD = 1<<6,
	/*lets the main user handle window events (needed for browser plugins)*/
	GF_TERM_NO_WINDOWPROC_OVERRIDE = 1<<7,
	/*works without title bar*/
	GF_TERM_WINDOW_NO_DECORATION = 1<<8,
	/*framebuffer works in 32 bit alpha mode - experimental, only supported on Win32*/
	GF_TERM_WINDOW_TRANSPARENT = 1<<9,
	/*works in windowless mode - experimental, only supported on Win32*/
	GF_TERM_WINDOWLESS = 1<<10,
};

/*user object for all callbacks*/
struct _tag_user
{
	/*user defined callback for all functions - cannot be NULL*/
	void *opaque;
	/*the event proc. Return value depend on the event type, usually 0
	cannot be NULL if os_window_handler is specified and dont_override_window_proc is set
	may be NULL otherwise*/
	Bool (*EventProc)(void *opaque, GF_Event *event);

	/*config file of client - cannot be NULL*/
	GF_Config *config;
	/*modules manager - cannot be NULL - owned by the user (to allow selection of module directory)*/
	GF_ModuleManager *modules;
	/*optional os window handler (HWND on win32/winCE, XWindow for X11) 
	if not set the video outut will create and manage the display window.*/
	void *os_window_handler;
	/*for now, only used by X11 (indicates display the window is on)*/
	void *os_display;

	/*init flags bypassing GPAC config file	*/
	u32 init_flags;
};


#ifdef __cplusplus
}
#endif

#endif	/*_GF_USER_H_*/

