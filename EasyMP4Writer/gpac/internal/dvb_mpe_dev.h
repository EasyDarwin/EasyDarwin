/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Authors: Walid B.H - Jean Le Feuvre
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


#ifndef _GF_DVB_MPE_DEV_H_
#define _GF_DVB_MPE_DEV_H_

#include <gpac/dvb_mpe.h>
#include <gpac/internal/reedsolomon.h>


/*INT object*/
typedef struct
{
	u32 id;
	u32 processing_order;
	u32 number_of_descriptor;
	GF_List * descriptors;

} GF_M2TS_INT;

typedef struct
{
	u32 tag;
	u32 length;
	u32 network_id;
	u32 original_network_id;
	u32 ts_id;
	u32 service_id;
	u32 component_tag;

}GF_M2TS_LOC_DSCPTR_IP_STREAM;

typedef struct descriptor_TimeSliceFec
{
	Bool time_slicing;
	u8 mpe_fec;
	u8 frame_size;
  	u8 max_burst_duration;
	u8 max_average_rate;
	u8 time_slice_fec_id;
	u8 * id_selector;
}GF_M2TS_DesTimeSliceFec;

typedef struct
{
	u16	network_id;
	u16 original_network_id;
	u16 transport_stream_id;
	u16 service_id;
	u8 component_tag;
}GF_M2TS_DesLocation;

typedef struct {
	u8 type; /* 0 = target_IP_descriptor, 1 = target_IP_address_descriptor */
	u32 address_mask;
	u8 address[4];
	u8 slash_mask;
	u32 rx_port[10];   /* list of the adress port */
} GF_M2TS_IP_Target;

typedef struct
{
	GF_List *targets; /* list of IP destination for the IP streams in the platform */
	u32 PID;
	Bool stream_info_gathered;

	/* location descriptor only valid for the associated targets */
	GF_M2TS_DesLocation location;

	GF_M2TS_DesTimeSliceFec time_slice_fec;
} GF_M2TS_IP_Stream;


/*IP_Platform object*/
typedef struct __gf_dvb_mpe_ip_platform
{
/* remaining from INT, to be delete */
	u32 id;
	u32 processing_order;
	u32 number_of_descriptor;

	u8 *name; /* platform name */
	u8 *provider_name; /* platform provider name */

	/* location descriptor valid for the whole platform */
	GF_M2TS_DesLocation *location;

	GF_List * ip_streams;
	Bool all_info_gathered;
	GF_List *socket_struct;

} GF_M2TS_IP_PLATFORM;

typedef struct
{
	char *data;                        /* Data */
	u32 u32_version;                   /* IP version */
	u32 u32_hdr_length;                /* header length by piece of 4 bytes */
	u32 u32_total_length;              /* the length of the datagram (hdr+payload) in bytes */
	u32 u32_payload_size;              /* the length of the payload */
	u32 u32_id_nb;                     /* the number of the paquet, in case of frag */
	u32 u32_flag;                      /* if 010 unfrag packet, 100 fragmented packet, check the id_nb to know the packet number.
	                                      0 is the last one */
	u32 u32_frag_offset;               /* The offset position of this packet compare to the first packet. unit : 8 bytes */
	u32 u32_TTL;                       /* (Time To Live) when = 0 , the packet is ignored and error message */
	u32 u32_protocol;                  /* TCP = 6, UDP = 17, ICMP = 1 */
	u32 u32_crc;
	u8 u8_tx_adr[4];                   /* source adress */
	u8 u8_rx_adr[4];                   /* destination adress */
	u32 u32_size_option;               /* size of the option before payload */
	u32 u32_padding;                   /* = 1 if where read padding columns */
	u32 u32_sum;

	/* UDP */
	u32 u32_tx_udp_port;                /* source port */
	u32 u32_rx_udp_port;                /* destination port */
	u32 u32_udp_data_size;
	u32 u32_udp_chksm;
}GF_M2TS_IP_Packet;




#define MPE_ADT_COLS 191
#define MPE_RS_COLS NPAR

typedef struct mpe_error_holes
{
	u32 offset;
	u32 length;

}MPE_Error_Holes;

typedef struct mpe_fec_frame
{
	u32 rows;
	u32 col_adt ;
	u32 col_rs ;
	u8 *p_adt; /* pointer to the application data table*/
	u8 *p_rs;  /* pointer to the RS data table*/
    u32 *p_error_adt;
	u32 *p_error_rs ;

	u32 capacity_total;
	u32 current_offset_adt ;
	u32 current_offset_rs;
    u32 initialized ;
	u8  ADT_done;
	u32 PID;
	GF_List *mpe_holes;
	//u32 erasures [] p_erasures; /*pointer to the error indicators*/
} MPE_FEC_FRAME;


/* Get INT table */
void gf_m2ts_process_int(GF_M2TS_Demuxer *ts, GF_M2TS_SECTION_ES *ip_table, unsigned char *data, u32 data_size, u32 table_id);

void section_DSMCC_INT(GF_M2TS_IP_PLATFORM* ip_platform, u8 *data, u32 data_size);

u32  platform_descriptorDSMCC_INT_UNT  (GF_M2TS_IP_PLATFORM* ip_platform,u8 *data);
u32 dsmcc_pto_platform_descriptor_loop (GF_M2TS_IP_PLATFORM* ip_platform,u8 *data);

u32  descriptorDSMCC_INT_UNT  (GF_M2TS_IP_Stream *ip_str,u8 *data);
void descriptorDSMCC_target_IP_address ( GF_M2TS_IP_Stream *ip_str, u8 *data);
u32 dsmcc_pto_descriptor_loop (GF_M2TS_IP_Stream *ip_str, u8 *data);
void descriptorTime_slice_fec_identifier(GF_M2TS_IP_Stream *ip_str, u8 *data);
void gf_m2ts_target_ip( GF_M2TS_IP_Stream* ip_str, u8 *data);
void descriptorLocation(GF_M2TS_IP_Stream *ip_str , u8 *data);


void gf_ip_platform_descriptor(GF_M2TS_IP_PLATFORM* ip_platform, u8 *data);
void gf_ip_platform_provider_descriptor(GF_M2TS_IP_PLATFORM* ip_platform,u8 *data);
void gf_m2ts_ip_platform_init(GF_M2TS_IP_PLATFORM * ip_platform);

u32 gf_m2ts_ipdatagram_reader(u8 *datagram, GF_M2TS_IP_Packet *ip_packet, u32 offset);
void gf_m2ts_process_ipdatagram(MPE_FEC_FRAME *mff,GF_M2TS_Demuxer *ts);
Bool gf_m2ts_compare_ip(u8 rx_ip_adress[4], u8 ip_adress_bootstrap[4]);

struct _sock_entry
{
	u32 ipv4_addr;
	u16 port;
	GF_Socket *sock;
	Bool bind_failure;
};
struct tag_m2ts_section_mpe
{
	ABSTRACT_ES
	GF_M2TS_SectionFilter *sec;

	/* if this stream is an MPE section stream, we need:
	    - a direct access to the timeslice fec descriptor
		- an MPE FEC Frame Structure to process RS code */
	GF_M2TS_IP_Stream *ip_platform;
	MPE_FEC_FRAME *mff;

};


void gf_m2ts_process_mpe(GF_M2TS_Demuxer *ts, GF_M2TS_SECTION_MPE *mpe, unsigned char *data, u32 data_size, u8 table_id);
void gf_m2ts_gather_ipdatagram_information(MPE_FEC_FRAME *mff,GF_M2TS_Demuxer *ts);

void socket_simu(GF_M2TS_IP_Packet *ip_packet, GF_M2TS_Demuxer *ts, Bool yield);

void gf_m2ts_mpe_send_datagram(GF_M2TS_Demuxer *ts, u32 pid, unsigned char *data, u32 data_size);			

/* allocate the necessary memory space*/
u32 init_frame(MPE_FEC_FRAME * mff, u32 rows);

void getRowFromADT(MPE_FEC_FRAME * mff,u32 index, u8 * adt_row);
void getRowFromRS(MPE_FEC_FRAME * mff,u32 index, u8 * rs_row);
void setRowRS(MPE_FEC_FRAME * mff,u32 index, u8 * p_rs);

/*return the number of errors and the position of the error in the row*/
void getErrorPositions(MPE_FEC_FRAME * mff, u32 row, u32 * errPositions);

void setColRS( MPE_FEC_FRAME * mff, u32 offset, u8 * pds, u32 length );
void getColRS(MPE_FEC_FRAME * mff, u32 offset, u8 * pds, u32 length);
void setIpDatagram(MPE_FEC_FRAME * mff,u32 offset, u8 * dgram, u32 length );

void setErrorIndicator(u32 * data , u32 offset1, u32 length  );
void resetMFF(MPE_FEC_FRAME * mff) ;
u32  getErrasurePositions( MPE_FEC_FRAME *mff , u32 row, u32 *errasures);

void decode_fec(MPE_FEC_FRAME * mff);


// Descriptor tag space/scope...
typedef enum {
	MPEG, DVB_SI,
	DSMCC_STREAM, DSMCC_CAROUSEL, DSMCC_INT_UNT, MHP_AIT, TVA_RNT
} DTAG_SCOPE;

void descriptor_PRIVATE (u8 *b, DTAG_SCOPE tag_scope, GF_List * descriptors );

#endif	//_GF_DVB_MPE_DEV_H_
