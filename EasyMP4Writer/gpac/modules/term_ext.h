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


#ifndef _GF_MODULE_TERM_EXT_H_
#define _GF_MODULE_TERM_EXT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/terminal.h>

/*interface name and version for Terminal Extensions services*/
#define GF_TERM_EXT_INTERFACE		GF_4CC('G','T','E', '1')

typedef struct _term_ext GF_TermExt;


typedef struct {
	void *scenegraph;
	void *ctx;
	void *global;
	Bool unload;
} GF_TermExtJS;

enum
{
	/*start terminal extension. If 0 is returned, the module will be unloaded
		associated param: GF_Terminal *term
		@return: 1 if OK, 0 otherwise (in which case the extensions will be removed without calling stop)
	*/
	GF_TERM_EXT_START = 1,
	/*stop terminal extension
		associated param: NULL
		@return: ignored
	*/
	GF_TERM_EXT_STOP,

	/*process extension - only called GF_TERM_EXTENSION_NOT_THREADED capability is set
		associated param: NULL
		@return: ignored
	*/
	GF_TERM_EXT_PROCESS,
	
	/*load/unload js bindings of this extension
		associated param: GF_TermExtJS *jsext
		@return: ignored
	*/
	GF_TERM_EXT_JSBIND,
};

enum
{
	/*signal the extension is to be called on regular basis (once per simulation tick). This MUST be set during
	the GF_TERM_EXT_START command and cannot be changed at run-time*/
	GF_TERM_EXTENSION_NOT_THREADED = 1<<1,

	GF_TERM_EXTENSION_JS = 1<<2,
};


struct _term_ext
{
	/* interface declaration*/
	GF_DECL_MODULE_INTERFACE

	/*caps of the module*/
	u32 caps;

	/*terminal extension proc
	 termext: pointer to the module
	 action: action type of this call
	 param: associated param of the call
	*/
	Bool (*process)(GF_TermExt *termext, u32 action, void *param);

	/*module private*/
	void *udta;
};



#ifdef __cplusplus
}
#endif


#endif	/*#define _GF_MODULE_TERM_EXT_H_*/

