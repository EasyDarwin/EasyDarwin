/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005
 *					All rights reserved
 *
 *  This file is part of GPAC / BIFS codec sub-project
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

#ifndef _GF_BIFS_H_
#define _GF_BIFS_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <gpac/nodes_mpeg4.h>
/*for BIFSConfig*/
#include <gpac/mpeg4_odf.h>

#ifndef GPAC_DISABLE_BIFS


typedef struct __tag_bifs_dec GF_BifsDecoder;

/*BIFS decoder constructor - 
 @command_dec: if set, the decoder will only work in memory mode (creating commands for the graph)
 otherwise the decoder will always apply commands while decoding them*/
GF_BifsDecoder *gf_bifs_decoder_new(GF_SceneGraph *scenegraph, Bool command_dec);
void gf_bifs_decoder_del(GF_BifsDecoder *codec);

/*assigns extraction path for BIFS decoder - default is current directory*/
void gf_bifs_decoder_set_extraction_path(GF_BifsDecoder *codec, const char *path, const char *serviceURL);

/*sets the scene time. Scene time is the real clock of the bifs stream in secs*/
void gf_bifs_decoder_set_time_offset(GF_BifsDecoder *codec, Double ts);

/*signals the sizeInfo of the config should be ignored - used for BIFS in AnimationStream nodes*/
void gf_bifs_decoder_ignore_size_info(GF_BifsDecoder *codec);

/*setup a stream*/
GF_Err gf_bifs_decoder_configure_stream(GF_BifsDecoder *codec, u16 ESID, char *DecoderSpecificInfo, u32 DecoderSpecificInfoLength, u32 objectTypeIndication);
/*removes a stream*/
GF_Err gf_bifs_decoder_remove_stream(GF_BifsDecoder *codec, u16 ESID);

/*decode a BIFS AU and applies it to the graph (non-memory mode only)*/
GF_Err gf_bifs_decode_au(GF_BifsDecoder *codec, u16 ESID, const char *data, u32 data_length, Double ts_offset);

/*Memory BIFS decoding - fills the command list with the content of the AU - cf scenegraph_vrml.h for commands usage
	@ESID: ID of input stream
	@data, @data_length: BIFS AU
	@com_list: target list for decoded commands
*/
GF_Err gf_bifs_decode_command_list(GF_BifsDecoder *codec, u16 ESID, char *data, u32 data_length, GF_List *com_list);


#ifndef GPAC_DISABLE_BIFS_ENC
/*BIFS encoding*/
typedef struct __tag_bifs_enc GF_BifsEncoder;

/*constructor - @graph: scene graph being encoded*/
GF_BifsEncoder *gf_bifs_encoder_new(GF_SceneGraph *graph);
/*destructor*/
void gf_bifs_encoder_del(GF_BifsEncoder *codec);
/*setup a destination stream*/
GF_Err gf_bifs_encoder_new_stream(GF_BifsEncoder *codec, u16 ESID, GF_BIFSConfig *cfg, Bool encodeNames, Bool has_predictive);
/*encodes a list of commands for the given stream in the output buffer - data is dynamically allocated for output
the scenegraph used is the one described in SceneReplace command, hence scalable streams shall be encoded in time order
*/
GF_Err gf_bifs_encode_au(GF_BifsEncoder *codec, u16 ESID, GF_List *command_list, char **out_data, u32 *out_data_length);
/*returns encoded config desc*/
GF_Err gf_bifs_encoder_get_config(GF_BifsEncoder *codec, u16 ESID, char **out_data, u32 *out_data_length);
/*returns BIFS version used by codec for given stream*/
u8 gf_bifs_encoder_get_version(GF_BifsEncoder *codec, u16 ESID);

/*Encodes current graph as a scene replace*/
GF_Err gf_bifs_encoder_get_rap(GF_BifsEncoder *codec, char **out_data, u32 *out_data_length);

#endif /*GPAC_DISABLE_BIFS_ENC*/

#endif /*GPAC_DISABLE_BIFS*/

#ifdef __cplusplus
}
#endif

#endif	/*_GF_BIFS_H_*/

