/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Authors: Jean le Feuvre
 *				Copyright (c) 2005-200X ENST
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

#ifndef _GF_LASER_H_
#define _GF_LASER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/nodes_svg.h>

#ifndef GPAC_DISABLE_LASER

/*for LASeRConfig*/
#include <gpac/mpeg4_odf.h>

typedef struct __tag_laser_codec GF_LASeRCodec;


/*LASeR decoder constructor*/
GF_LASeRCodec *gf_laser_decoder_new(GF_SceneGraph *scenegraph);
void gf_laser_decoder_del(GF_LASeRCodec *codec);

/*sets the scene time. Scene time is the real clock of the bifs stream in secs*/
void gf_laser_decoder_set_clock(GF_LASeRCodec *codec, Double (*GetSceneTime)(void *st_cbk), void *st_cbk );

/*setup a stream*/
GF_Err gf_laser_decoder_configure_stream(GF_LASeRCodec *codec, u16 ESID, char *DecoderSpecificInfo, u32 DecoderSpecificInfoLength);
/*removes a stream*/
GF_Err gf_laser_decoder_remove_stream(GF_LASeRCodec *codec, u16 ESID);

/*decode a laser AU and applies it to the graph (non-memory mode only)*/
GF_Err gf_laser_decode_au(GF_LASeRCodec *codec, u16 ESID, const char *data, u32 data_length);

/*Memory laser decoding - fills the command list with the content of the AU - cf scenegraph_vrml.h for commands usage
	@ESID: ID of input stream
	@data, @data_length: BIFS AU
	@com_list: target list for decoded commands
*/
GF_Err gf_laser_decode_command_list(GF_LASeRCodec *codec, u16 ESID, char *data, u32 data_length, GF_List *com_list);


/*constructor - @graph: scene graph being encoded*/
GF_LASeRCodec *gf_laser_encoder_new(GF_SceneGraph *graph);
/*destructor*/
void gf_laser_encoder_del(GF_LASeRCodec *codec);
/*setup a destination stream*/
GF_Err gf_laser_encoder_new_stream(GF_LASeRCodec *codec, u16 ESID, GF_LASERConfig *cfg);
/*encodes a list of commands for the given stream in the output buffer - data is dynamically allocated for output*/
GF_Err gf_laser_encode_au(GF_LASeRCodec *codec, u16 ESID, GF_List *command_list, Bool reset_encoding_context, char **out_data, u32 *out_data_length);
/*returns encoded config desc*/
GF_Err gf_laser_encoder_get_config(GF_LASeRCodec *codec, u16 ESID, char **out_data, u32 *out_data_length);

/*Encodes current graph as a scene replace*/
GF_Err gf_laser_encoder_get_rap(GF_LASeRCodec *codec, char **out_data, u32 *out_data_length);


#endif /*GPAC_DISABLE_LASER*/

#ifdef __cplusplus
}
#endif

#endif

