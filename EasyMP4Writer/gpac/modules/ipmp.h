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


#ifndef _GF_MODULE_IPMP_H_
#define _GF_MODULE_IPMP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/module.h>

/*
	NOTE ON IPMP TOOLS

  The current implementation is very basic and does not follow MPEG-4 IPMPX architecture
  This is just a place holder for ISMA/OMA-like schemes
  Currently all operations are synchronous...
*/

enum
{
	/*push some configuration data to the IPMP tool*/
	GF_IPMP_TOOL_SETUP,
	/*request access to the object (eg, PLAY)*/
	GF_IPMP_TOOL_GRANT_ACCESS,
	/*release access to the object (eg, STOP)*/
	GF_IPMP_TOOL_RELEASE_ACCESS,
	/*push some configuration data to the IPMP tool*/
	GF_IPMP_TOOL_PROCESS_DATA,
};

typedef struct
{
	u32 scheme_version;
	u32 scheme_type;
	const char *scheme_uri;
	const char *kms_uri;
} GF_ISMACrypConfig;

typedef struct
{
	u32 scheme_version;
	u32 scheme_type;
	const char *scheme_uri;
	const char *kms_uri;
	/*SHA-1 hash*/
	u8 hash[20];

	const char *contentID;
	u32 oma_drm_crypt_type;
	Bool oma_drm_use_pad, oma_drm_use_hdr;
	const char *oma_drm_textual_headers;
	u32 oma_drm_textual_headers_len;
} GF_OMADRM2Config;

/*IPMP events*/
typedef struct
{
	/*event type*/
	u32 event_type;

	/*gpac's channel triggering this event, NULL if unknown/unspecified*/
	struct _es_channel *channel;

	/*identifier of the config data (GF_IPMP_TOOL_SETUP)*/
	u32 config_data_code;
	/*config data (GF_IPMP_TOOL_SETUP). Type depends on the config_data_code*/
	void *config_data;

	Bool restart_requested;

	/*data manipulation (GF_IPMP_TOOL_PROCESS_DATA) - data is always processed in-place in a
	synchronous way*/
	char *data;
	u32 data_size;
	u32 out_data_size;
	/*indicates if payload passed is encrypted or not - this is used by ISMA, OMA and 3GP*/
	Bool is_encrypted;
	/*ISMA payload resync indicator*/
	u64 isma_BSO;
} GF_IPMPEvent;

/*interface name and version for IPMP tools*/
#define GF_IPMP_TOOL_INTERFACE		GF_4CC('G','I','P', '1')

typedef struct _ipmp_tool GF_IPMPTool;

struct _ipmp_tool
{
	/* interface declaration*/
	GF_DECL_MODULE_INTERFACE

	/*process an ipmp event*/
	GF_Err (*process)(GF_IPMPTool *dr, GF_IPMPEvent *evt);
	/*tool private*/
	void *udta;

};


#ifdef __cplusplus
}
#endif


#endif	/*#define _GF_MODULE_IPMP_H_
*/

