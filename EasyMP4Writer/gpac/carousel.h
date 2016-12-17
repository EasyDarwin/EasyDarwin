/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Authors: Telecom Paristech
 *    Copyright (c)2006-200X ENST - All rights reserved
 *
 *  This file is part of GPAC / MPEG2-TS sub-project
 *
 *  GPAC is gf_free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the gf_free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  GPAC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *   
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; see the file COPYING.  If not, write to
 *  the gf_free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */


#ifndef _GF_CAROUSSEL_H_
#define _GF_CAROUSSEL_H_

#include <gpac/mpegts.h>
#include <string.h>
#include <gpac/bitstream.h>

#define AIT_SECTION_LENGTH_MAX 1021
#define APPLICATION_TYPE_HTTP_APPLICATION 16

typedef enum {
	APPLICATION_DESCRIPTOR = 0x00,
	APPLICATION_NAME_DESCRIPTOR = 0x01,
	TRANSPORT_PROTOCOL_DESCRIPTOR = 0x02,
	SIMPLE_APPLICATION_LOCATION_DESCRIPTOR = 0x15,
	APPLICATION_USAGE_DESCRIPTOR = 0x16,
} DESCRIPTOR_TAG;


typedef struct
{
	
	ABSTRACT_ES
	GF_M2TS_SectionFilter *sec;

	u32 service_id;
	u8 table_id;
	Bool section_syntax_indicator;
	u16 section_length;
	Bool test_application_flag;
	u16 application_type;
	u8 version_number;
	Bool current_next_indicator;
	u8 section_number;
	u8 last_section_number;
	u16 common_descriptors_length;
	GF_List * common_descriptors;
	u16 application_loop_length;
	GF_List * application;
	u32 CRC_32;

} GF_M2TS_AIT;


typedef struct
{
	u32 organisation_id;
	u16 application_id;
	u8 application_control_code;
	u16 application_descriptors_loop_length;
	GF_List * application_descriptors;
	u8 application_descriptors_id[50];
	u8 index_app_desc_id;

}
GF_M2TS_AIT_APPLICATION;


typedef enum {
	FUTURE_USE = 0x00,
	CAROUSEL = 0x01,
	RESERVED = 0x02,
	TRANSPORT_HTTP = 0x03,
	DVB_USE = 0x04,
	TO_REGISTER = 0x100,
} PROTOCOL_ID;

typedef struct
{
	u8 descriptor_tag;
	u8 descriptor_length;
	u8 application_profiles_length;
	u16 application_profile;
	u8 version_major;
	u8 version_minor;
	u8 version_micro;
	Bool service_bound_flag;
	u8 visibility;
	u8 application_priority;
	u8 transport_protocol_label;

} GF_M2TS_APPLICATION_DESCRIPTOR;

typedef struct
{
	u8 descriptor_tag;
	u8 descriptor_length;
	u8 usage_type;

} GF_M2TS_APPLICATION_USAGE;

typedef struct
{
	u8 descriptor_tag;
	u8 descriptor_length;
	char* initial_path_bytes;

}GF_M2TS_SIMPLE_APPLICATION_LOCATION;

typedef struct
{
	Bool remote_connection;
	u16 original_network_id;
	u16 transport_stream_id;
	u16 service_id;
	u8 component_tag;

} GF_M2TS_OBJECT_CAROUSEL_SELECTOR_BYTE;

typedef struct{

	u8 URL_extension_length;
	char* URL_extension_byte;

}GF_M2TS_TRANSPORT_HTTP_URL_EXTENTION;

typedef struct
{
	u8 URL_base_length;
	char* URL_base_byte;
	u8 URL_extension_count;
	GF_M2TS_TRANSPORT_HTTP_URL_EXTENTION* URL_extentions;	

} GF_M2TS_TRANSPORT_HTTP_SELECTOR_BYTE;

typedef struct
{
	u8 descriptor_tag;
	u8 descriptor_length;
	u16 protocol_id;
	u8 transport_protocol_label;
	void* selector_byte;

} GF_M2TS_TRANSPORT_PROTOCOL_DESCRIPTOR;

typedef struct
{
	u8 descriptor_tag;
	u8 descriptor_length;
	u32 ISO_639_language_code;
	u8 application_name_length;
	char* application_name_char;

} GF_M2TS_APPLICATION_NAME_DESCRIPTOR;

GF_Err gf_m2ts_process_ait(GF_M2TS_AIT *es, char  *data, u32 data_size, u32 table_id);
void on_ait_section(GF_M2TS_Demuxer *ts, u32 evt_type, void *par);
GF_M2TS_ES *gf_ait_section_new(u32 service_id);
void gf_ait_destroy(GF_M2TS_AIT* ait);


#endif	//_GF_CAROUSSEL_H_

