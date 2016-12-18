/*
 *					GPAC Multimedia Framework
 *
 *			Authors: Cyril Concolato - Jean le Feuvre
 *				Copyright (c) 2005-200X ENST
 *					All rights reserved
 *
 *  This file is part of GPAC 
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


#ifndef _GF_SCENEENGINE_H_
#define _GF_SCENEENGINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/scene_manager.h>

#ifndef GPAC_DISABLE_SENG

typedef struct __tag_scene_engine GF_SceneEngine;


typedef void (*gf_seng_callback)(void *udta, u16 ESID, char *data, u32 size, u64 ts);

/**
 * @calling_object is the calling object on which call back will be called
 * @inputContext is the name of a scene file (bt, xmt or mp4) to initialize the coding context
 * @load_type is the prefered loader type for the content (e.g. SVG vs DIMS)
 * @dump_path is the path where scenes are dumped 
 * @embed_resources indicates if images and scripts should be encoded inlined with the content
 *
 * must be called only one time (by process calling the DLL) before other calls
 */
GF_SceneEngine *gf_seng_init(void *calling_object, char *inputContext, u32 load_type, char *dump_path, Bool embed_resources);

/**
 * @calling_object is the calling object on which call back will be called
 * @inputContext is an UTF-8 scene description (with or without IOD) in BT or XMT-A format
 * @load_type is the prefered loader type for the content (e.g. SVG vs DIMS)
 * @width, @height: width and height of scene if no IOD is given in the context.
 * @usePixelMetrics: metrics system used in the scene, if no IOD is given in the context.
 * @dump_path is the path where scenes are dumped 
 *
 * must be called only one time (by process calling the DLL) before other calls
 */
GF_SceneEngine *gf_seng_init_from_string(void *calling_object, char *inputContext, u32 load_type, u32 width, u32 height, Bool usePixelMetrics, char *dump_path);


/**
 * @calling_object is the calling object on which call back will be called
 * @ctx is an already loaded scene manager
 * @dump_path is the path where scenes are dumped 
 *
 * must be called only one time (by process calling the DLL) before other calls
 */
GF_SceneEngine *gf_seng_init_from_context(void *calling_object, GF_SceneManager *ctx, char *dump_path);

/**
 * @seng, pointer to the GF_SceneEngine returned by gf_seng_init()
 *
 * must be called after gf_seng_init()
 */
u32 gf_seng_get_stream_count(GF_SceneEngine *seng);

/**
 * @seng, pointer to the GF_SceneEngine returned by gf_seng_init()
 * @ESID, ID of the stream
 * @carousel_period: pointer to store the carousel_period 
 * @aggregate_on_es_id: pointer to store the target carousel stream ID
 *
 * must be called after gf_seng_init()
 */
GF_Err gf_seng_get_stream_carousel_info(GF_SceneEngine *seng, u16 ESID, u32 *carousel_period, u16 *aggregate_on_es_id);

/**
 * @seng, pointer to the GF_SceneEngine returned by gf_seng_init()
 * @idx: stream index
 * @ESID: pointer to the stream ID
 * @config: pointer to the encoded BIFS config (memory is not allocated)
 * @config_len: length of the buffer
 * @streamType: pointer to get stream type
 * @objectType: pointer to get object type
 * @timeScale: pointer to get time scale
 *
 * must be called after gf_seng_init()
 */
GF_Err gf_seng_get_stream_config(GF_SceneEngine *seng, u32 idx, u16 *ESID, char * * const config, u32 *config_len, u32 *streamType, u32 *objectType, u32 *timeScale);

/**
 * Encodes the AU context which is not encoded when calling BENC_EncodeAUFromString/File
 * Should be called after Aggregate.
 *
 * @seng, pointer to the GF_SceneEngine returned by gf_seng_init()
 * @AUCallback, pointer on a callback function to get the result of the coding the AU using the current context
 *
 */
GF_Err gf_seng_encode_context(GF_SceneEngine *seng, gf_seng_callback callback);

/**
 * @seng, pointer to the GF_SceneEngine returned by gf_seng_init()
 * @ESID, target streams when no indication is present in the file (eg, no atES_ID )
 * @auFile, name of a file containing a description for an access unit (BT or XMT)
 * @AUCallback, pointer on a callback function to get the result of the coding the AU using the current context
 *
 */
GF_Err gf_seng_encode_from_file(GF_SceneEngine *seng, u16 ESID, Bool disable_aggregation, char *auFile, gf_seng_callback callback);

/**
 * @seng, pointer to the GF_SceneEngine returned by gf_seng_init()
 * @ESID, target streams when no indication is present in the file (eg, no atES_ID )
 * @auString, a char string to encode (must one or several complete nodes in BT
 * @AUCallback, pointer on a callback function to get the result of the coding the AU using the current context
 *
 */
GF_Err gf_seng_encode_from_string(GF_SceneEngine *seng, u16 ESID, Bool disable_aggregation, char *auString, gf_seng_callback callback);

/**
 * @seng, pointer to the GF_SceneEngine returned by gf_seng_init()
 * @ESID, indicates the stream to which these commands apply (0 if first scene stream)
 * @commans, the list of commands to encode
 * @AUCallback, pointer on a callback function to get the result of the coding the AU using the current context
 *
 */
GF_Err gf_seng_encode_from_commands(GF_SceneEngine *codec, u16 ESID, Bool disable_aggregation, u32 time, GF_List *commands, gf_seng_callback callback);

/**
 * @seng, pointer to the GF_SceneEngine returned by gf_seng_init()
 * @ctxFileName, name of the file to save the current state of the BIFS scene to
 *
 * save the current context of the seng.
 * if you want to save an aggregate context, use BENC_AggregateCurrentContext before
 *
 */
GF_Err gf_seng_save_context(GF_SceneEngine *seng, char *ctxFileName);

/**
 * @seng, pointer to the GF_SceneEngine returned by gf_seng_init()
 * @ESID, stream ID
 * @onESID: set stream aggragation on to the specified stream, or off if onESID is 0
 *
 * marks the stream as carrying its own "rap" in the first AU of the stream 
 */
GF_Err gf_seng_enable_aggregation(GF_SceneEngine *codec, u16 ESID, u16 onESID);

/**
 * @seng, pointer to the GF_SceneEngine returned by gf_seng_init()
 *
 * aggregates the current context of the seng, creates a scene replace
 * if @ESID is specified, only aggregate commands for this stream
 */
GF_Err gf_seng_aggregate_context(GF_SceneEngine *seng, u16 ESID);

/**
 * @seng, pointer to the GF_SceneEngine returned by gf_seng_init()
 *
 * release the memory used by this seng, no more call on the seng should happen after this
 *
 */
void gf_seng_terminate(GF_SceneEngine *seng);

/**
 * @seng, pointer to the GF_SceneEngine returned by gf_seng_init()
 * @buf64, pointer to a buffer that will be allocated and will contain the base64 encoded version of the binary IOD
 * @buf64, pointer to the size of the buffer
 *
 * encodes the IOD for this BIFS Engine into Base64
 *
 */
char *gf_seng_get_base64_iod(GF_SceneEngine *seng);

GF_Descriptor *gf_seng_get_iod(GF_SceneEngine *seng);

GF_Err gf_seng_dump_rap_on(GF_SceneEngine *seng, Bool dump_rap);

#endif /*GPAC_DISABLE_SENG*/


#ifdef __cplusplus
}
#endif // __cplusplus


#endif	/*_GF_SCENEENGINE_H_*/

