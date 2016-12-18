/*
 *			GPAC - Multimedia Framework C SDK
 *
 *				Authors: Jean Le Feuvre 
 *			Copyright (c) Telecom ParisTech 2010
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

#ifndef GPAC_JSAPI
#define GPAC_JSAPI

#include <gpac/setup.h>

#ifdef GPAC_ANDROID
#ifndef XP_UNIX
#define XP_UNIX
#endif
#endif

#include <jsapi.h>

#ifndef JS_VERSION
#define JS_VERSION 170
#endif

/*new APIs*/
#if (JS_VERSION>=185)

#define JS_NewDouble(c, v)	v
#define JS_PropertyStub_forSetter JS_StrictPropertyStub
#define SMJS_PROP_SETTER jsid id, JSBool strict
#define SMJS_PROP_GETTER jsid id
#define SMJS_FUNCTION_SPEC(__name, __fun, __argc) {__name, __fun, __argc, 0}
#define SMJS_FUNCTION(__name) __name(JSContext *c, uintN argc, jsval *argsvp)
#define SMJS_FUNCTION_EXT(__name, __ext) __name(JSContext *c, uintN argc, jsval *argsvp, __ext)
#define SMJS_ARGS	jsval *argv = JS_ARGV(c, argsvp);
#define SMJS_OBJ	JSObject *obj = JS_THIS_OBJECT(c, argsvp);
#define SMJS_SET_RVAL(__rval) JS_SET_RVAL(c, argsvp, __rval)
#define SMJS_GET_RVAL & JS_RVAL(c, argsvp)
#define SMJS_CALL_ARGS	c, argc, argsvp
#define SMJS_DECL_RVAL jsval *rval = & JS_RVAL(c, argsvp);

#define SMJS_CHARS_FROM_STRING(__c, __jsstr)	(char *) JS_EncodeString(__c, __jsstr)
#define SMJS_CHARS(__c, __val)	SMJS_CHARS_FROM_STRING(__c, JSVAL_TO_STRING(__val))
#define SMJS_FREE(__c, __str)	if (__str) JS_free(__c, __str)


#define SMJS_OBJ_CONSTRUCTOR	JSObject *obj = NULL;\
	if (!JS_IsConstructing_PossiblyWithGivenThisObject(c, argsvp, &obj)) {	\
		return JS_FALSE;\
	}\
	if (obj == NULL) obj = JS_NewObjectForConstructor(c, argsvp);	\
	SMJS_SET_RVAL(OBJECT_TO_JSVAL(obj));\

#define JS_GetFunctionName(_v) (JS_GetFunctionId(_v)!=NULL) ? SMJS_CHARS_FROM_STRING(c, JS_GetFunctionId(_v)) : NULL

#define SMJS_ID_IS_STRING	JSID_IS_STRING
#define SMJS_ID_TO_STRING		JSID_TO_STRING
#define SMJS_ID_IS_INT	JSID_IS_INT
#define SMJS_ID_TO_INT		JSID_TO_INT

#ifndef JS_THREADSAFE
#define JS_THREADSAFE
#endif

#else
#define SMJS_PROP_SETTER jsval id
#define SMJS_PROP_GETTER jsval id
#define JS_PropertyStub_forSetter JS_PropertyStub
#define SMJS_FUNCTION_SPEC(__name, __fun, __argc) {__name, __fun, __argc, 0, 0}
#define SMJS_FUNCTION(__name) __name(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
#define SMJS_FUNCTION_EXT(__name, __ext) __name(JSContext *c, JSObject *obj, uintN argc, jsval *argv, jsval *rval, __ext)
#define SMJS_ARGS
#define SMJS_OBJ	
#define SMJS_OBJ_CONSTRUCTOR
#define SMJS_GET_RVAL rval
#define SMJS_SET_RVAL(__rval) *rval = __rval
#define SMJS_CALL_ARGS	c, obj, argc, argv, rval
#define SMJS_DECL_RVAL

#define SMJS_CHARS_FROM_STRING(__c, __str)	JS_GetStringBytes(__str)
#define SMJS_CHARS(__c, __val)	JS_GetStringBytes(JSVAL_TO_STRING(__val))
#define SMJS_FREE(__c, __str)

#define SMJS_ID_IS_STRING	JSVAL_IS_STRING
#define SMJS_ID_TO_STRING		JSVAL_TO_STRING
#define SMJS_ID_IS_INT	JSVAL_IS_INT
#define SMJS_ID_TO_INT		JSVAL_TO_INT

#endif


#ifdef __cplusplus
extern "C" {
#endif

#if (JS_VERSION>=185)
JSBool gf_sg_js_has_instance(JSContext *c, JSObject *obj,const jsval *val, JSBool *vp);
#else
JSBool gf_sg_js_has_instance(JSContext *c, JSObject *obj, jsval val, JSBool *vp);
#endif

#define JS_SETUP_CLASS(the_class, cname, flag, getp, setp, fin)	\
	memset(&the_class, 0, sizeof(the_class));	\
	the_class.name = cname;	\
	the_class.flags = flag;	\
	the_class.addProperty = JS_PropertyStub;	\
	the_class.delProperty = JS_PropertyStub;	\
	the_class.getProperty = getp;	\
	the_class.setProperty = setp;	\
	the_class.enumerate = JS_EnumerateStub;	\
	the_class.resolve = JS_ResolveStub;		\
	the_class.convert = JS_ConvertStub;		\
	the_class.finalize = fin;	\
	the_class.hasInstance = gf_sg_js_has_instance;


#define JS_MAKE_DOUBLE(__c, __double)	DOUBLE_TO_JSVAL(JS_NewDouble(__c, __double) ) 


JSObject *gf_sg_js_global_object(JSContext *cx, JSClass *__class);

#ifdef __cplusplus
}
#endif

#endif //GPAC_JSAPI
