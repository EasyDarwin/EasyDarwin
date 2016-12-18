/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005
 *					All rights reserved
 *
 *  This file is part of GPAC / IETF RTP/RTSP/SDP sub-project
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

#ifndef	_GF_IETF_DEV_H_
#define _GF_IETF_DEV_H_

#include <gpac/ietf.h>

#ifndef GPAC_DISABLE_STREAMING

#include <gpac/thread.h>

/*
			RTP intern
*/

typedef struct
{
	/*version of the packet. Must be 2*/
	u8 Version;
	/*padding bits at the end of the payload*/
	u8 Padding;
	/*number of reports*/
	u8 Count;
	/*payload type of RTCP pck*/
	u8 PayloadType;
	/*The length of this RTCP packet in 32-bit words minus one including the header and any padding*/
	u16 Length;
	/*sync source identifier*/
	u32 SSRC;
} GF_RTCPHeader;	


typedef struct __PRO_item
{
	struct __PRO_item *next;
	u32 pck_seq_num;
	void *pck;
	u32 size;
} GF_POItem;

typedef struct __PO
{
	struct __PRO_item *in;
	u32 head_seqnum;
	u32 Count;
	u32 MaxCount;
	u32 IsInit;
	u32 MaxDelay, LastTime;
} GF_RTPReorder;

/* creates new RTP reorderer
	@MaxCount: forces automatic packet flush. 0 means no flush
	@MaxDelay: is the max time in ms the queue will wait for a missing packet
*/
GF_RTPReorder *gf_rtp_reorderer_new(u32 MaxCount, u32 MaxDelay);
void gf_rtp_reorderer_del(GF_RTPReorder *po);
/*reset the Queue*/
void gf_rtp_reorderer_reset(GF_RTPReorder *po);

/*Adds a packet to the queue. Packet Data is memcopied*/
GF_Err gf_rtp_reorderer_add(GF_RTPReorder *po, void *pck, u32 pck_size, u32 pck_seqnum);
/*gets the output of the queue. Packet Data IS YOURS to delete*/
void *gf_rtp_reorderer_get(GF_RTPReorder *po, u32 *pck_size);


/*the RTP channel with both RTP and RTCP sockets and buffers
each channel is identified by a control string given in RTSP Describe
this control string is used with Darwin
*/
struct __tag_rtp_channel
{
	/*global transport info for the session*/
	GF_RTSPTransport net_info;
	
	/*RTP CHANNEL*/
	GF_Socket *rtp;
	/*RTCP CHANNEL*/
	GF_Socket *rtcp;
	
	/*RTP Packet reordering. Turned on/off during initialization. The library forces a 200 ms
	max latency at the reordering queue*/
	GF_RTPReorder *po;

	/*RTCP report times*/
	u32 last_report_time;
	u32 next_report_time;

	/*NAT keep-alive*/
	u32 last_nat_keepalive_time, nat_keepalive_time_period;

	
	/*the seq number of the first packet as signaled by the server if any, or first
	RTP SN received (RTP multicast)*/
	u32 rtp_first_SN;
	/*the TS of the associated first packet as signaled by the server if any, or first
	RTP TS received (RTP multicast)*/
	u32 rtp_time;
	/*NPT from the rtp_time*/
	u32 CurrentTime;
	/*num loops of pck sn*/
	u32 num_sn_loops;
	/*some mapping info - we should support # payloads*/
	u8 PayloadType;
	u32 TimeScale;

	/*static buffer for RTP sending*/
	char *send_buffer;
	u32 send_buffer_size;
	u32 pck_sent_since_last_sr;
	u32 last_pck_ts;
	u32 last_pck_ntp_sec, last_pck_ntp_frac;
	u32 num_pck_sent, num_payload_bytes;

	Bool no_auto_rtcp;
	/*RTCP info*/
	char *s_name, *s_email, *s_location, *s_phone, *s_tool, *s_note, *s_priv;
//	s8 first_rtp_pck;
	s8 first_SR;
	u32 SSRC;
	u32 SenderSSRC;

	u32 last_pck_sn;
	/*indicates if a packet loss is detected between current and previous packet*/
	Bool packet_loss;

	char *CName;

	u32 rtcp_bytes_sent;
	/*total pck rcv*/
	u32 tot_num_pck_rcv, tot_num_pck_expected;
	/*stats since last SR*/
	u32 last_num_pck_rcv, last_num_pck_expected, last_num_pck_loss;
	/*jitter compute*/
	u32 Jitter, ntp_init;
	s32 last_deviance;	
	/*NTP of last SR*/
	u32 last_SR_NTP_sec, last_SR_NTP_frac;
	/*RTP time at last SR as indicated in SR*/
	u32 last_SR_rtp_time;
	/*payload info*/
	u32 total_pck, total_bytes;
};

/*gets UTC in the channel RTP timescale*/
u32 gf_rtp_channel_time(GF_RTPChannel *ch);
/*gets time in 1/65536 seconds (for reports)*/
u32 gf_rtp_get_report_time();
/*updates the time for the next report (SR, RR)*/
void gf_rtp_get_next_report_time(GF_RTPChannel *ch);


/*
			RTSP intern
*/

#define GF_RTSP_DEFAULT_BUFFER		2048
#define GF_RTSP_VERSION		"RTSP/1.0"

/*macros for RTSP command and response formmating*/
#define RTSP_WRITE_STEPALLOC	250

#define RTSP_WRITE_ALLOC_STR_WITHOUT_CHECK(buf, buf_size, pos, str)		\
	if (strlen(str)+pos >= buf_size) {	\
		buf_size += RTSP_WRITE_STEPALLOC;	\
		buf = (char *) gf_realloc(buf, buf_size);		\
	}	\
	strcpy(buf+pos, (const char *)(str));		\
	pos += strlen((const char *)(str)); \

#define RTSP_WRITE_ALLOC_STR(buf, buf_size, pos, str)		\
	if (str){	\
		RTSP_WRITE_ALLOC_STR_WITHOUT_CHECK(buf, buf_size, pos, str);	\
	}	\
		
#define RTSP_WRITE_HEADER(buf, buf_size, pos, type, str)		\
	if(str) {	\
	RTSP_WRITE_ALLOC_STR(buf, buf_size, pos, type);		\
	RTSP_WRITE_ALLOC_STR(buf, buf_size, pos, ": ");		\
	RTSP_WRITE_ALLOC_STR(buf, buf_size, pos, (str));	\
	RTSP_WRITE_ALLOC_STR(buf, buf_size, pos, "\r\n");	\
	}	\

#define RTSP_WRITE_INT(buf, buf_size, pos, d, sig)		\
	if (sig < 0) { \
		sprintf(temp, "%d", (s32)d);		\
	} else { \
		sprintf(temp, "%u", (u32)d);		\
	}	\
	RTSP_WRITE_ALLOC_STR_WITHOUT_CHECK(buf, buf_size, pos, temp);

#define RTSP_WRITE_FLOAT_WITHOUT_CHECK(buf, buf_size, pos, d)		\
	sprintf(temp, "%.4f", d);		\
	RTSP_WRITE_ALLOC_STR_WITHOUT_CHECK(buf, buf_size, pos, temp);

#define RTSP_WRITE_FLOAT(buf, buf_size, pos, d)		\
	sprintf(temp, "%.4f", d);		\
	RTSP_WRITE_ALLOC_STR(buf, buf_size, pos, temp);

/*default packet size, but resize on the fly if needed*/
#define RTSP_PCK_SIZE			6000
#define RTSP_TCP_BUF_SIZE		0x10000ul


typedef struct
{
	u8 rtpID;
	u8 rtcpID;
	void *ch_ptr;
} GF_TCPChan;

/**************************************
		RTSP Session
***************************************/
struct _tag_rtsp_session
{
	/*service name (extracted from URL) ex: news/latenight.mp4, vod.mp4 ...*/
	char *Service;	
	/*server name (extracted from URL)*/
	char *Server;
	/*server port (extracted from URL)*/
	u16 Port;

	/*if RTSP is on UDP*/
	u8 ConnectionType;
	/*TCP interleaving ID*/
	u8 InterID;
	/*http tunnel*/
	Bool HasTunnel;
	GF_Socket *http;
	char HTTP_Cookie[30];
	u32 CookieRadLen;

	/*RTSP CHANNEL*/
	GF_Socket *connection;
	u32 SockBufferSize;
	/*needs connection*/
	u32 NeedConnection;

	/*the RTSP sequence number*/
	u32 CSeq;
	/*this is for aggregated request in order to check SeqNum*/
	u32 NbPending;

	/*RTSP sessionID, arbitrary length, alpha-numeric*/
	const char *last_session_id;

	/*RTSP STATE machine*/
	u32 RTSP_State;
	char RTSPLastRequest[40];

	/*current buffer from TCP if any*/
	char TCPBuffer[RTSP_TCP_BUF_SIZE];
	u32 CurrentSize, CurrentPos;

	/*RTSP interleaving*/
	GF_Err (*RTSP_SignalData)(GF_RTSPSession *sess, void *chan, char *buffer, u32 bufferSize, Bool IsRTCP);
	
	/*buffer for pck reconstruction*/
	char *rtsp_pck_buf;
	u32 rtsp_pck_size;
	u32 pck_start, payloadSize;

	/*all RTP channels in an interleaved RTP on RTSP session*/
	GF_List *TCPChannels;
	/*thread-safe, full duplex library for PLAY and RECORD*/
	GF_Mutex *mx;

	char *MobileIP;	
};

GF_RTSPSession *gf_rtsp_session_new(char *sURL, u16 DefaultPort);

/*check connection status*/
GF_Err gf_rtsp_check_connection(GF_RTSPSession *sess);
/*send data on RTSP*/
GF_Err gf_rtsp_send_data(GF_RTSPSession *sess, char *buffer, u32 Size);

/* 
			Common RTSP tools
*/

/*locate body-start and body size in response/commands*/
void gf_rtsp_get_body_info(GF_RTSPSession *sess, u32 *body_start, u32 *body_size);
/*read TCP until a full command/response is received*/
GF_Err gf_rtsp_read_reply(GF_RTSPSession *sess);
/*fill the TCP buffer*/
GF_Err gf_rtsp_fill_buffer(GF_RTSPSession *sess);
/*force a fill on TCP buffer - used for de-interleaving and TCP-fragmented RTSP messages*/
GF_Err gf_rtsp_refill_buffer(GF_RTSPSession *sess);
/*parses a transport string and returns a transport structure*/
GF_RTSPTransport *gf_rtsp_transport_parse(char *buffer);
/*parsing of header for com and rsp*/
GF_Err gf_rtsp_parse_header(char *buffer, u32 BufferSize, u32 BodyStart, GF_RTSPCommand *com, GF_RTSPResponse *rsp);
void gf_rtsp_set_command_value(GF_RTSPCommand *com, char *Header, char *Value);
void gf_rtsp_set_response_value(GF_RTSPResponse *rsp, char *Header, char *Value);
/*deinterleave a data packet*/
GF_Err gf_rtsp_set_deinterleave(GF_RTSPSession *sess);
/*start session through HTTP tunnel (QTSS)*/
GF_Err gf_rtsp_http_tunnel_start(GF_RTSPSession *sess, char *UserAgent);


/*packetization routines*/
GF_Err gp_rtp_builder_do_mpeg4(GP_RTPPacketizer *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize);
GF_Err gp_rtp_builder_do_h263(GP_RTPPacketizer *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize);
GF_Err gp_rtp_builder_do_amr(GP_RTPPacketizer *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize);
#ifndef GPAC_DISABLE_AV_PARSERS
GF_Err gp_rtp_builder_do_mpeg12_video(GP_RTPPacketizer *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize);
#endif
GF_Err gp_rtp_builder_do_mpeg12_audio(GP_RTPPacketizer *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize);
GF_Err gp_rtp_builder_do_tx3g(GP_RTPPacketizer *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize, u32 duration, u8 descIndex);
GF_Err gp_rtp_builder_do_avc(GP_RTPPacketizer *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize);
GF_Err gp_rtp_builder_do_qcelp(GP_RTPPacketizer *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize);
GF_Err gp_rtp_builder_do_smv(GP_RTPPacketizer *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize);
GF_Err gp_rtp_builder_do_latm(GP_RTPPacketizer *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize, u32 duration); 
GF_Err gp_rtp_builder_do_dims(GP_RTPPacketizer *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize, u32 duration);
GF_Err gp_rtp_builder_do_ac3(GP_RTPPacketizer *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize);


#endif /*GPAC_DISABLE_STREAMING*/

#endif	/*_GF_IETF_DEV_H_*/

