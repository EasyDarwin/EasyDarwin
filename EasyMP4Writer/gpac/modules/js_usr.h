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


#ifndef _GF_MODULE_JS_USR_H_
#define _GF_MODULE_JS_USR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/module.h>
#include <gpac/scenegraph.h>

/*interface name and version for JavaScript User Extensions*/
#define GF_JS_USER_EXT_INTERFACE		GF_4CC('G','J','S', '1')

typedef struct _js_usr_ext GF_JSUserExtension;

typedef struct JSContext GF_JSContext;
typedef struct JSObject GF_JSObject;

struct _js_usr_ext
{
	/* interface declaration*/
	GF_DECL_MODULE_INTERFACE

	/*load JS extension
	 doc: scene graph in which the extension is loaded
	 jsctx: JavaScript context in which the extension is loaded. 
		For BIFS/VRML/X3D, one context is created per script node
		For other graphs, one context is created per scene/document
	 global: JavaScript global object for the context
	 unload: if true, the extension should be unloaded from the JavaScript context (called upon destroy). Otherwise it should be loaded
	*/
	void (*load)(GF_JSUserExtension *jsext, GF_SceneGraph *doc, GF_JSContext *jsctx, GF_JSObject *global, Bool unload);
	/*module private*/
	void *udta;
};


#ifdef __cplusplus
}
#endif


#endif	/*#define _GF_MODULE_JS_USR_H_*/

