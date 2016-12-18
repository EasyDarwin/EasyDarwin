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


#ifndef	_GF_IETF_H_
#define _GF_IETF_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/list.h>

#ifndef GPAC_DISABLE_STREAMING

#include <gpac/bitstream.h>
#include <gpac/sync_layer.h>
#include <gpac/network.h>


/****************************************************************************

				RTSP VERSION 1.0 LIBRARY EXPORTS

****************************************************************************/

#define GF_RTSP_VERSION		"RTSP/1.0"


/*
 *		RTSP NOTIF CODES
 */
enum
{
	NC_RTSP_Continue		=		100,
	NC_RTSP_OK				=		200,
	NC_RTSP_Created			=		201,
	NC_RTSP_Low_on_Storage_Space	=	250,

	NC_RTSP_Multiple_Choice	=	300,
	NC_RTSP_Moved_Permanently	=	301,
	NC_RTSP_Moved_Temporarily	=	302,
	NC_RTSP_See_Other	=	303,
	NC_RTSP_Use_Proxy	=	305,

	NC_RTSP_Bad_Request	=	400,
	NC_RTSP_Unauthorized	=	401,
	NC_RTSP_Payment_Required	=	402,
	NC_RTSP_Forbidden	=	403,
	NC_RTSP_Not_Found	=	404,
	NC_RTSP_Method_Not_Allowed	=	405,
	NC_RTSP_Not_Acceptable	=	406,
	NC_RTSP_Proxy_Authentication_Required	=	407,
	NC_RTSP_Request_Timeout	=	408,
	NC_RTSP_Gone	=	410,
	NC_RTSP_Length_Required	=	411,
	NC_RTSP_Precondition_Failed	=	412,
	NC_RTSP_Request_Entity_Too_Large	=	413,
	NC_RTSP_Request_URI_Too_Long	=	414,
	NC_RTSP_Unsupported_Media_Type	=	415,

	NC_RTSP_Invalid_parameter	=	451,
	NC_RTSP_Illegal_Conference_Identifier	=	452,
	NC_RTSP_Not_Enough_Bandwidth	=	453,
	NC_RTSP_Session_Not_Found	=	454,
	NC_RTSP_Method_Not_Valid_In_This_State	=	455,
	NC_RTSP_Header_Field_Not_Valid	=	456,
	NC_RTSP_Invalid_Range	=	457,
	NC_RTSP_Parameter_Is_ReadOnly	=	458,
	NC_RTSP_Aggregate_Operation_Not_Allowed	=	459,
	NC_RTSP_Only_Aggregate_Operation_Allowed	=	460,
	NC_RTSP_Unsupported_Transport	=	461,
	NC_RTSP_Destination_Unreachable	=	462,
	
	NC_RTSP_Internal_Server_Error	=	500,
	NC_RTSP_Not_Implemented	=	501,
	NC_RTSP_Bad_Gateway	=	502,
	NC_RTSP_Service_Unavailable	=	503,
	NC_RTSP_Gateway_Timeout	=	504,
	NC_RTSP_RTSP_Version_Not_Supported	=	505,

	NC_RTSP_Option_not_support	=	551,
};

const char *gf_rtsp_nc_to_string(u32 ErrCode);

/*
		Common structures between commands and responses
*/

/*
	RTSP Range information - RTSP Session level only (though this is almost the same
	format as an SDP range, this is not used in the SDP lib as "a=range" is not part of SDP
	but part of RTSP
*/
typedef struct {
	/* start and end range. If end is -1, the range is open (from start to unknown) */
	Double start, end;
	/* use SMPTE range (Start and End specify the number of frames) (currently not supported) */
	u32 UseSMPTE;
	/* framerate for SMPTE range */
	Double FPS;
} GF_RTSPRange;

/*
parses a Range line and returns range header structure. can be used for RTSP extension of SDP
NB: Only support for npt for now
*/
GF_RTSPRange *gf_rtsp_range_parse(char *range_buf);

GF_RTSPRange *gf_rtsp_range_new();
void gf_rtsp_range_del(GF_RTSPRange *range);

/*
			Transport structure 
		contains all network info for RTSP sessions (ports, uni/multi-cast, ...)
*/

/*
	Transport Profiles as defined in RFC 2326
*/
#define GF_RTSP_PROFILE_RTP_AVP			"RTP/AVP"
#define GF_RTSP_PROFILE_RTP_AVP_TCP		"RTP/AVP/TCP"
#define GF_RTSP_PROFILE_UDP				"udp"


typedef struct
{
	/* set to 1 if unicast */
	Bool IsUnicast;
	/* for multicast */
	char *destination;
	/* for redirections internal to servers */
	char *source;
	/*IsRecord is usually 0 (PLAY) . If set, Append specify that the stream should
	be concatenated to existing resources */
	Bool IsRecord, Append;
	/* in case transport is on TCP/RTSP, If only 1 ID is specified, it is stored in rtpID (this
	is not RTP interleaving) */
	Bool IsInterleaved, rtpID, rtcpID;
	/* Multicast specific */
	u32 MulticastLayers;
	u8 TTL;
	/*RTP specific*/

	/*port for multicast*/
	/*server port in unicast - RTP implies low is even , and last is low+1*/
	u16 port_first, port_last;
	/*client port in unicast - RTP implies low is even , and last is low+1*/
	u16 client_port_first, client_port_last;
	u32 SSRC;

	/*Transport protocol. In this version we only support RTP/AVP, the following flag tells 
	us if this is RTP/AVP/TCP or RTP/AVP (default)*/
	char *Profile;
} GF_RTSPTransport;


GF_RTSPTransport *gf_rtsp_transport_clone(GF_RTSPTransport *original);
void gf_rtsp_transport_del(GF_RTSPTransport *transp);


/*
				RTSP Command
		the RTSP Response is sent by a client / received by a server
	text Allocation is done by the lib when parsing a command, and
	is automatically freed when calling reset / delete. Therefore you must
	set/allocate the fields yourself when writing a command (client)

*/

/*ALL RTSP METHODS - all other methods will be ignored*/
#define GF_RTSP_DESCRIBE		"DESCRIBE"
#define GF_RTSP_SETUP			"SETUP"
#define GF_RTSP_PLAY			"PLAY"
#define GF_RTSP_PAUSE			"PAUSE"
#define GF_RTSP_RECORD			"RECORD"
#define GF_RTSP_TEARDOWN		"TEARDOWN"
#define GF_RTSP_GET_PARAMETER	"GET_PARAMETER"
#define GF_RTSP_SET_PARAMETER	"SET_PARAMETER"
#define GF_RTSP_OPTIONS		"OPTIONS"
#define GF_RTSP_ANNOUNCE		"ANNOUNCE"
#define GF_RTSP_REDIRECTE		"REDIRECT"


typedef struct
{
	char *Accept;
	char *Accept_Encoding;
	char *Accept_Language;
	char *Authorization;
	u32 Bandwidth;
	u32 Blocksize;
	char *Cache_Control;
	char *Conference;
	char *Connection;
	u32 Content_Length;
	u32 CSeq;
	char *From;
	char *Proxy_Authorization;
	char *Proxy_Require;
	GF_RTSPRange *Range;
	char *Referer;
	Double Scale;
	char *Session;
	Double Speed;
	/*nota : RTSP allows several configurations for a single channel (multicast and 
	unicast , ...). Usually only 1*/
	GF_List *Transports;
	char *User_Agent;

	/*type of the command, one of the described above*/
	char *method;
	
	/*Header extensions*/
	GF_List *Xtensions;

	/*body of the command, size is Content-Length (auto computed when sent). It is not 
	terminated by a NULL char*/
	char *body;

	/*
			Specify ControlString if your request targets
		a specific media stream in the service. If null, the service name only will be used
		for control (for ex, both A and V streams in a single file)
		If the request is GF_RTSP_OPTIONS, you must provide a control string containing the options 
		you want to query
	*/
	char *ControlString;

	/*user data: this is never touched by the lib, its intend is to help stacking
	RTSP commands in your app*/
	void *user_data;


	/*
		Server side Extensions
	*/

	/*full URL of the command. Not used at client side, as the URL is ALWAYS relative
	to the server / service of the RTSP session 
	On the server side however redirections are up to the server, so we cannot decide for it	*/
	char *service_name;
	/*RTSP status code of the command as parsed. One of the above RTSP StatusCode*/
	u32 StatusCode;
} GF_RTSPCommand;


GF_RTSPCommand *gf_rtsp_command_new();
void gf_rtsp_command_del(GF_RTSPCommand *com);
void gf_rtsp_command_reset(GF_RTSPCommand *com);



/*
				RTSP Response
		the RTSP Response is received by a client / sent by a server
	text Allocation is done by the lib when parsing a response, and
	is automatically freed when calling reset / delete. Therefore you must
	allocate the fields yourself when writing a response (server)

*/

/*
	RTP-Info for RTP channels. There may be several RTP-Infos in one response
	based on the server implementation (DSS/QTSS begaves this way)
*/
typedef struct
{
	/*control string of the channel*/
	char *url;
	/*seq num for asociated rtp_time*/
	u32 seq;
	/*rtp TimeStamp corresponding to the Range start specified in the PLAY request*/
	u32 rtp_time;
	/*ssrc of sender if known, 0 otherwise*/
	u32 ssrc;
} GF_RTPInfo;



/*
	RTSP Response
*/
typedef struct
{
	/* response code*/
	u32 ResponseCode;
	/* comment from the server */
	char *ResponseInfo;
	
	/*	Header Fields	*/
	char *Accept;
	char *Accept_Encoding;
	char *Accept_Language;
	char *Allow;
	char *Authorization;
	u32 Bandwidth;
	u32 Blocksize;
	char *Cache_Control;
	char *Conference;
	char *Connection;
	char *Content_Base;
	char *Content_Encoding;
	char *Content_Language;
	u32 Content_Length;
	char *Content_Location;
	char *Content_Type;
	u32 CSeq;
	char *Date;
	char *Expires;
	char *From;
	char *Host;
	char *If_Match;
	char *If_Modified_Since;
	char *Last_Modified;
	char *Location;
	char *Proxy_Authenticate;
	char *Proxy_Require;
	char *Public;
	GF_RTSPRange *Range;
	char *Referer;
	char *Require;
	char *Retry_After;
	GF_List *RTP_Infos;
	Double Scale;
	char *Server;
	char *Session;
	u32 SessionTimeOut;
	Double Speed;
	char *Timestamp;
	/*nota : RTSP allows several configurations for a single channel (multicast and 
	unicast , ...). Usually only 1*/
	GF_List *Transports;
	char *Unsupported;
	char *User_Agent;
	char *Vary;
	char *Via;
	char *WWW_Authenticate;

	/*Header extensions*/
	GF_List *Xtensions;

	/*body of the response, size is Content-Length (auto computed when sent). It is not 
	terminated by a NULL char when response is parsed but must be null-terminated when 
	response is being sent*/
	char *body;
} GF_RTSPResponse;


GF_RTSPResponse *gf_rtsp_response_new();
void gf_rtsp_response_del(GF_RTSPResponse *rsp);
void gf_rtsp_response_reset(GF_RTSPResponse *rsp);



typedef struct _tag_rtsp_session GF_RTSPSession;

GF_RTSPSession *gf_rtsp_session_new(char *sURL, u16 DefaultPort);
void gf_rtsp_session_del(GF_RTSPSession *sess);

GF_Err gf_rtsp_set_buffer_size(GF_RTSPSession *sess, u32 BufferSize);

/*force the IP address the client is using*/
void gf_rtsp_set_mobile_ip(GF_RTSPSession *sess, char *MobileIP);


/*Reset state machine, invalidate SessionID
NOTE: RFC2326 requires that the session is reseted when all RTP streams
are closed. As this lib doesn't maintain the number of valid streams
you MUST call reset when all your streams are shutdown (either requested through
TEARDOWN or signaled through RTCP BYE packets for RTP, or any other signaling means
for other protocols)
reset connection will destroy the socket - this is isefull in case of timeouts, because
some servers do not restart with the right CSeq...*/
void gf_rtsp_session_reset(GF_RTSPSession *sess, Bool ResetConnection);

u32 gf_rtsp_is_my_session(GF_RTSPSession *sess, char *url);
const char *gf_rtsp_get_last_session_id(GF_RTSPSession *sess);
char *gf_rtsp_get_server_name(GF_RTSPSession *sess);
char *gf_rtsp_get_service_name(GF_RTSPSession *sess);
u16 gf_rtsp_get_session_port(GF_RTSPSession *sess);

/*Fetch an RTSP response from the server the GF_RTSPResponse will be reseted before fetch*/
GF_Err gf_rtsp_get_response(GF_RTSPSession *sess, GF_RTSPResponse *rsp);


/*RTSP State Machine. The only non blocking mode is GF_RTSP_STATE_WAIT_FOR_CONTROL*/
enum
{
	/*Initialized (connection might be off, but all structures are in place)
	This is the default state between # requests (aka, DESCRIBE and SETUP
	or SETUP and PLAY ...)*/
	GF_RTSP_STATE_INIT	=	0,
	/*Waiting*/
	GF_RTSP_STATE_WAITING,
	/*PLAY, PAUSE, RECORD. Aggregation is allowed for the same type, you can send several command
	in a row. However the session will return GF_SERVICE_ERROR if you do not have 
	a valid SessionID in the command
	You cannot issue a SETUP / DESCRIBE while in this state*/
	GF_RTSP_STATE_WAIT_FOR_CONTROL,

	/*FATAL ERROR: session is invalidated by server. Call reset and restart from SETUP if needed*/
	GF_RTSP_STATE_INVALIDATED
};

u32 gf_rtsp_get_session_state(GF_RTSPSession *sess);
/*aggregate command state-machine: the PLAY/PAUSE can be aggregated 
(sent before the reply is received). This function gets the last command sent*/
char *gf_rtsp_get_last_request(GF_RTSPSession *sess);
/*foce a reset in case of pbs*/
void gf_rtsp_reset_aggregation(GF_RTSPSession *sess);

/*
	Send an RTSP request to the server.
*/
GF_Err gf_rtsp_send_command(GF_RTSPSession *sess, GF_RTSPCommand *com);


GF_Err gf_rtsp_set_interleave_callback(GF_RTSPSession *sess,
						GF_Err (*SignalData)(GF_RTSPSession *sess, void *cbk_ptr, char *buffer, u32 bufferSize, Bool IsRTCP)
				);


GF_Err gf_rtsp_session_read(GF_RTSPSession *sess);

GF_Err gf_rtsp_register_interleave(GF_RTSPSession *sess, void *the_ch, u8 LowInterID, u8 HighInterID);
u32 gf_rtsp_unregister_interleave(GF_RTSPSession *sess, u8 LowInterID);



/*
			Server side session constructor
	create a new RTSP session from an existing socket in listen state. If no pending connection
	is detected, return NULL
*/
GF_RTSPSession *gf_rtsp_session_new_server(GF_Socket *rtsp_listener);

/*fetch an RTSP request. The GF_RTSPCommand will be reseted before fetch*/
GF_Err gf_rtsp_get_command(GF_RTSPSession *sess, GF_RTSPCommand *com);

/*unpack the URL, check the service name / server. Typically used when a client sends a 
 DESCRIBE || SETUP url RTSP/1.0. Server / service name check must be performed by your app as redirection
or services available are unknown here.*/
GF_Err gf_rtsp_load_service_name(GF_RTSPSession *sess, char *URL);

/*geenrates a session ID fpor the given session*/
char *gf_rtsp_generate_session_id(GF_RTSPSession *sess);

/*send the RTSP response*/
GF_Err gf_rtsp_send_response(GF_RTSPSession *sess, GF_RTSPResponse *rsp);

/*gets the IP address of the local host running the session
buffer shall be GF_MAX_IP_NAME_LEN long*/
GF_Err gf_rtsp_get_session_ip(GF_RTSPSession *sess, char *buffer);

/*returns the next available ID for interleaving. It is recommended that you use 2 
consecutive IDs for RTP/RTCP interleaving*/
u8 gf_rtsp_get_next_interleave_id(GF_RTSPSession *sess);

/*gets the IP address of the connected peer - buffer shall be GF_MAX_IP_NAME_LEN long*/
GF_Err gf_rtsp_get_remote_address(GF_RTSPSession *sess, char *buffer);


/*
		RTP LIB EXPORTS
*/


typedef struct tagRTP_HEADER {
	/*version, must be 2*/
	u8 Version;
	/*padding bits in the payload*/
	u8 Padding;
	/*header extension is defined*/
	u8 Extension;
	/*number of CSRC (<=15)*/
	u8 CSRCCount;
	/*Marker Bit*/
	u8 Marker;
	/*payload type on 7 bits*/
	u8 PayloadType;
	/*packet seq number*/
	u16 SequenceNumber;
	/*packet time stamp*/
	u32 TimeStamp;
	/*sync source identifier*/
	u32 SSRC;
	/*in our basic client, CSRC should always be NULL*/
	u32 CSRC[16];
} GF_RTPHeader;




/*
	structure containing the rtpmap information
*/
typedef struct
{
	/*dynamic payload type of this map*/
	u32 PayloadType;
	/*registered payload name of this map*/
	char *payload_name;
	/*RTP clock rate (TS resolution) of this map*/
	u32 ClockRate;
	/*optional parameters for audio, specifying number of channels. Unused for other media types.*/
	u32 AudioChannels;
} GF_RTPMap;



typedef struct __tag_rtp_channel GF_RTPChannel;

GF_RTPChannel *gf_rtp_new();
void gf_rtp_del(GF_RTPChannel *ch);

/*you configure a server channel through the transport structure, with the same info as a 
client channel, the client_port_* info designing the REMOTE client and port_* designing
your server channel*/
GF_Err gf_rtp_setup_transport(GF_RTPChannel *ch, GF_RTSPTransport *trans_info, const char *remote_address);

/*auto-setup of rtp/rtcp transport ports - only effective in unicast, non interleaved cases. 
for multicast port setup MUST be done through the above gf_rtp_setup_transport function
this will take care of port reuse*/
GF_Err gf_rtp_set_ports(GF_RTPChannel *ch, u16 first_port);

/*init of payload information. only ONE payload per sync source is supported in this
version of the library (a sender cannot switch payload types on a single media)*/
GF_Err gf_rtp_setup_payload(GF_RTPChannel *ch, GF_RTPMap *map);

/*enables sending of NAT keep-alive packets for NAT traversal
	@nat_timeout: specifies the inactivity period in ms after which NAT keepalive packets are sent. 
	If 0, disables NAT keep-alive packets 
*/
void gf_rtp_enable_nat_keepalive(GF_RTPChannel *ch, u32 nat_timeout);


/*initialize the RTP channel.

UDPBufferSize: UDP stack buffer size if configurable by OS/ISP - ignored otherwise
NOTE: on WinCE devices, this is not configurable on an app bases but for the whole OS
you must update the device registry with:
	[HKEY_LOCAL_MACHINE\Comm\Afd]
	DgramBuffer=dword:N

	where N is the number of UDP datagrams a socket should be able to buffer. For multimedia
app you should set N as large as possible. The device MUST be reseted for the param to take effect

ReorederingSize: max number of packets to queue for reordering. 0 means no reordering
MaxReorderDelay: max time to wait in ms before releasing first packet in reoderer when only one packet is present.
If 0 and reordering size is specified, defaults to 200 ms (usually enough).
IsSource: if true, the channel is a sender (media data, sender report, Receiver report processing)
if source, you must specify the Path MTU size. The RTP lib won't send any packet bigger than this size
your application shall perform payload size splitting if needed
local_interface_ip: local interface address to use for multicast. If NULL, default address is used
*/
GF_Err gf_rtp_initialize(GF_RTPChannel *ch, u32 UDPBufferSize, Bool IsSource, u32 PathMTU, u32 ReorederingSize, u32 MaxReorderDelay, char *local_interface_ip);

/*init the RTP info after a PLAY or PAUSE, rtp_time is the rtp TimeStamp of the RTP packet
with seq_num sequence number. This info is needed to compute the CurrentTime of the RTP channel 
ssrc may not be known if sender hasn't indicated it (use 0 then)*/
GF_Err gf_rtp_set_info_rtp(GF_RTPChannel *ch, u32 seq_num, u32 rtp_time, u32 ssrc);

/*retrieve current RTP time in sec. If rtp_time was unknown (not on demand media) the time is absolute.
Otherwise this is the time in ms elapsed since the last PLAY range start value
Not supported yet if played without RTSP (aka RTCP time not supported)*/
Double gf_rtp_get_current_time(GF_RTPChannel *ch);


void gf_rtp_reset_buffers(GF_RTPChannel *ch);

/*read any data on UDP only (not valid for TCP). Performs re-ordering if configured for it
returns amount of data read (raw UDP packet size)*/
u32 gf_rtp_read_rtp(GF_RTPChannel *ch, char *buffer, u32 buffer_size);
u32 gf_rtp_read_rtcp(GF_RTPChannel *ch, char *buffer, u32 buffer_size);

/*decodes an RTP packet and gets the begining of the RTP payload*/
GF_Err gf_rtp_decode_rtp(GF_RTPChannel *ch, char *pck, u32 pck_size, GF_RTPHeader *rtp_hdr, u32 *PayloadStart);

/*decodes an RTCP packet and update timing info, send RR too*/
GF_Err gf_rtp_decode_rtcp(GF_RTPChannel *ch, char *pck, u32 pck_size, Bool *has_sr);

/*computes and send Receiver report. If the channel is a TCP channel, you must specify
the callback function. NOTE: many RTP implementation do NOT process RTCP info received on TCP...
the lib will decide whether the report shall be sent or not, therefore you should call
this function at regular times*/
GF_Err gf_rtp_send_rtcp_report(GF_RTPChannel *ch, 
						GF_Err (*RTP_TCPCallback)(void *cbk, char *pck, u32 pck_size),
						void *rtsp_cbk);

/*send a BYE info (leaving the session)*/
GF_Err gf_rtp_send_bye(GF_RTPChannel *ch,
						GF_Err (*RTP_TCPCallback)(void *cbk, char *pck, u32 pck_size),
						void *rtsp_cbk);


/*send RTP packet. In fast_send mode, user passes a pck pointer with 12 bytes available BEFORE pck to 
write the header in place*/
GF_Err gf_rtp_send_packet(GF_RTPChannel *ch, GF_RTPHeader *rtp_hdr, char *pck, u32 pck_size, Bool fast_send);

enum
{
	GF_RTCP_INFO_NAME = 0,
	GF_RTCP_INFO_EMAIL,
	GF_RTCP_INFO_PHONE,
	GF_RTCP_INFO_LOCATION,
	GF_RTCP_INFO_TOOL,
	GF_RTCP_INFO_NOTE,
	GF_RTCP_INFO_PRIV
};

/*sets RTCP info sent in RTCP reports. info_string shall NOT exceed 255 chars*/
GF_Err gf_rtp_set_info_rtcp(GF_RTPChannel *ch, u32 InfoCode, char *info_string);

u32 gf_rtp_is_unicast(GF_RTPChannel *ch);
u32 gf_rtp_is_interleaved(GF_RTPChannel *ch);
u32 gf_rtp_get_clockrate(GF_RTPChannel *ch);
u32 gf_rtp_is_active(GF_RTPChannel *ch);
u8 gf_rtp_get_low_interleave_id(GF_RTPChannel *ch);
u8 gf_rtp_get_hight_interleave_id(GF_RTPChannel *ch);
GF_RTSPTransport *gf_rtp_get_transport(GF_RTPChannel *ch);
u32 gf_rtp_get_local_ssrc(GF_RTPChannel *ch);

Float gf_rtp_get_loss(GF_RTPChannel *ch);
u32 gf_rtp_get_tcp_bytes_sent(GF_RTPChannel *ch);
void gf_rtp_get_ports(GF_RTPChannel *ch, u16 *rtp_port, u16 *rtcp_port);




	
/****************************************************************************

					SDP LIBRARY EXPORTS
		
		  Note: SDP is mainly a text protocol with 
	well defined containers. The following structures are used to write / read
	SDP informations, and the library also provides consistency checking

  When reading SDP, all text items/structures are allocated by the lib, and you
  must call gf_sdp_info_reset(GF_SDPInfo *sdp) or gf_sdp_info_del(GF_SDPInfo *sdp) to release the memory

  When writing the SDP from a GF_SDPInfo, the output buffer is allocated by the library, 
  and you must release it yourself

  Some quick constructors are available for GF_SDPConnection and GF_SDPMedia in order to set up
  some specific parameters to their default value

  An extra function gf_sdp_info_check(GF_SDPInfo *sdp) is provided for compliency check 
  with RFC2327: all requested fields are checked as well as conflicting information forbidden
  in RFC 2327
****************************************************************************/

/*
	All attributes x-ZZZZ are considered as extensions attributes. If no "x-" is found 
	the attributes in the RTSP response is SKIPPED. The "x-" radical is removed in the structure
	when parsing commands / responses
*/
typedef struct
{
	char *Name;
	char *Value;
} GF_X_Attribute;


/*
	Structure for bandwidth info
*/
typedef struct
{
	/*"CT", "AS" are defined. Private extensions must be "X-*" ( * "are recommended to be short")*/
	char *name;
	/*in kBitsPerSec*/
	u32 value;
} GF_SDPBandwidth;

/*
	Structure for Time info
*/
/*we do not support more than ... time offsets / zone adjustment
if more are needed, RFC recommends to use several entries rather than a big*/
#define GF_SDP_MAX_TIMEOFFSET	10

typedef struct
{
	/*NPT time in sec*/
	u32 StartTime;
	/*if 0, session is unbound. NPT time in sec*/
	u32 StopTime;
	/*if 0 session is not repeated. Expressed in sec.
	Session is signaled repeated every repeatInterval*/
	u32 RepeatInterval;
	/*active duration of the session in sec*/
	u32 ActiveDuration;

	/*time offsets to use with repeat. Specify a non-regular repeat time from the Start time*/
	u32 OffsetFromStart[GF_SDP_MAX_TIMEOFFSET];
	/*Number of offsets*/
	u32 NbRepeatOffsets;

	/*EX of repeat:
	a session happens 3 times a week, on mon 1PM, thu 3PM and fri 10AM
	1- StartTime should be NPT for the session on the very first monday, StopTime
	the end of this session
	2- the repeatInterval should be 1 week, ActiveDuration the length of the session
	3- 3 offsets: 0 (for monday) (3*24+2)*3600 for thu and (4*24-3) for fri
	*/


	/*timezone adjustments, to cope with #timezones, daylight saving countries and co ...
	Ex: adjTime = [2882844526 2898848070] adjOffset=[-1h 0]
	[0]: at 2882844526 the time base by which the session's repeat times are calculated 
	is shifted back by 1 hour
	[1]: at time 2898848070 the session's original time base is restored
	*/

	/*Adjustment time at which the corresponding time offset is to be applied to the 
	session time line (time used to compute the "repeat session"). 
	All Expressed in NPT*/
	u32 AdjustmentTime[GF_SDP_MAX_TIMEOFFSET];
	/* Offset with the session time line, ALWAYS ABSOLUTE OFFSET TO the specified StartTime*/
	s32 AdjustmentOffset[GF_SDP_MAX_TIMEOFFSET];
	/*Number of offsets.*/
	u32 NbZoneOffsets;
} GF_SDPTiming;


typedef struct
{
	/*only "IN" currently defined*/
	char *net_type;
	/*"IP4","IP6"*/
	char *add_type;
	/*hex IPv6 address or doted IPv4 address*/
	char *host;
	/*TTL - MUST BE PRESENT if IP is multicast - -1 otherwise*/
	s32 TTL;
	/*multiple address counts - ONLY in media descriptions if needed. This
	is used for content scaling, when # quality of the same media are multicasted on
	# IP addresses*/
	u32 add_count;
} GF_SDPConnection;

/*
	FMTP: description of dynamic payload types. This is opaque at the SDP level.
	Each attributes is assumed to be formatted as <param_name=param_val; ...>
	If not the case the attribute will have an empty value string and only the
	parameter name.
*/
typedef struct
{
	/*payload type of the format described*/
	u32 PayloadType;
	/*list of GF_X_Attribute elements. The Value field may be NULL*/
	GF_List *Attributes;
} GF_SDP_FMTP;

typedef struct
{
	/*m=
	0: application - 1:video - 2: audio - 3: text - 4:data - 5: control*/
	u32 Type;
	/*Port Number - For transports based on UDP, the value should be in the range 1024 
	to 65535 inclusive. For RTP compliance it should be an even number*/
	u32 PortNumber;
	/*number of ports described. If >= 2, the next media(s) in the SDP will be configured
	to use the next tuple (for RTP). If 0 or 1, ignored
	Note: this is used for scalable media: PortNumber indicates the port of the base 
	media and NumPorts the ports||total number of the upper layers*/
	u32 NumPorts;
	/*currently ony "RTP/AVP" and "udp" defined*/
	char *Profile;

	/*list of GF_SDPConnection's. A media can have several connection in case of scalable content*/
	GF_List *Connections;

	/*RTPMaps contains a list SDPRTPMaps*/
	GF_List *RTPMaps;

	/*FMTP contains a list of FMTP structures*/
	GF_List *FMTP;
	
	/*for RTP this is PayloadType, but can be opaque (string) depending on the app.
	Formated as XX WW QQ FF
	When reading the SDP, the payloads defined in RTPMap are removed from this list
	When writing the SDP for RTP, you should only specify static payload types here,
	as dynamic ones are stored in RTPMaps and automatically written*/
	char *fmt_list;

	/*all attributes not defined in RFC 2327 for the media*/
	GF_List *Attributes;

	/*Other SDP attributes for media desc*/

	/*k=
	method is 'clear' (key follows), 'base64' (key in base64), 'uri' (key is the URI) 
	or 'prompt' (key not included)*/
	char *k_method, *k_key;

	GF_List *Bandwidths;

	/*0 if not present*/
	u32 PacketTime;
	/*0: none - 1: recv, 2: send, 3 both*/
	u32 SendReceive;
	char *orientation, *sdplang, *lang;
	/*for video only, 0.0 if not present*/
	Double FrameRate;
	/*between 0 and 10, -1 if not present*/
	s32 Quality;
} GF_SDPMedia;

typedef struct
{
	/*v=*/
	u32 Version;
	/*o=*/
	char *o_username, *o_session_id, *o_version, *o_address;
	/*"IN" for Net, "IP4" or "IP6" for address are currently valid*/
	char *o_net_type, *o_add_type;

	/*s=*/
	char *s_session_name;
	/*i=*/
	char *i_description;
	/*u=*/
	char *u_uri;
	/*e=*/
	char *e_email;
	/*p=*/
	char *p_phone;
	/*c= either 1 or 0 GF_SDPConnection */
	GF_SDPConnection *c_connection;	
	/*b=*/
	GF_List *b_bandwidth;
	/*All time info (t, r, z)*/
	GF_List *Timing;
	/*k=
	method is 'clear' (key follows), 'base64' (key in base64), 'uri' (key is the URI) 
	or 'prompt' (key not included)*/
	char *k_method, *k_key;
	/*all possible attributes (a=), session level*/
	char *a_cat, *a_keywds, *a_tool;
	/*0: none, 1: recv, 2: send, 3 both*/
	u32 a_SendReceive;
	/*should be `broadcast', `meeting', `moderated', `test' or `H332'*/
	char *a_type;
	char *a_charset;
	char *a_sdplang, *a_lang;

	/*all attributes not defined in RFC 2327 for the presentation*/
	GF_List *Attributes;
	
	/*list of media in the SDP*/
	GF_List *media_desc;
} GF_SDPInfo;


/*
  Memory Consideration: the destructors free all non-NULL string. You should therefore 
  be carefull while (de-)assigning the strings. The function gf_sdp_info_parse() performs a complete 
  reset of the GF_SDPInfo
*/
/*constructor*/
GF_SDPInfo *gf_sdp_info_new();
/*destructor*/
void gf_sdp_info_del(GF_SDPInfo *sdp);
/*reset all structures (destroys substructure too)*/
void gf_sdp_info_reset(GF_SDPInfo *sdp);
/*Parses a memory SDP buffer*/
GF_Err gf_sdp_info_parse(GF_SDPInfo *sdp, char *sdp_text, u32 text_size);
/*check the consistency of the GF_SDPInfo*/
GF_Err gf_sdp_info_check(GF_SDPInfo *sdp);
/*write the SDP to a new buffer and returns it. Automatically checks the SDP before calling*/
GF_Err gf_sdp_info_write(GF_SDPInfo *sdp, char **out_str_buf);


/*
	Const/dest for GF_SDPMedia
*/
GF_SDPMedia *gf_sdp_media_new();
void gf_sdp_media_del(GF_SDPMedia *media);

/*
	Const/dest for GF_SDPConnection
*/
GF_SDPConnection *gf_sdp_conn_new();
void gf_sdp_conn_del(GF_SDPConnection *conn);

/*
	Const/dest for SDP FMTP
*/
GF_SDP_FMTP *gf_sdp_fmtp_new();
void gf_sdp_fmtp_del(GF_SDP_FMTP *fmtp);



/*
	RTP packetizer
*/


/*RTP<->SL mapping*/
typedef struct 
{
	/*1 - required options*/

	/*mode, or "" if no mode ("generic" should be used instead)*/
	char mode[30];
	
	/*config of the stream if carried in SDP*/
	char *config;
	u32 configSize;
	/* Stream Type*/
	u8 StreamType;
	/* stream profile and level indication - for AVC/H264, 0xPPCCLL, with PP:profile, CC:compatibility, LL:level*/
	u32 PL_ID;


	/*2 - optional options*/
	
	/*size of AUs if constant*/
	u32 ConstantSize;
	/*duration of AUs if constant, in RTP timescale*/
	u32 ConstantDuration;

	/* Object Type Indication */
	u8 ObjectTypeIndication;
	/*audio max displacement when interleaving (eg, de-interleaving window buffer max length) in RTP timescale*/
	u32 maxDisplacement;
	/*de-interleaveBufferSize if not recomputable from maxDisplacement*/
	u32 deinterleaveBufferSize;
	
	/*The number of bits on which the AU-size field is encoded in the AU-header*/
	u32 SizeLength;
	/*The number of bits on which the AU-Index is encoded in the first AU-header*/
	u32 IndexLength;
	/*The number of bits on which the AU-Index-delta field is encoded in any non-first AU-header*/
	u32 IndexDeltaLength;

	/*The number of bits on which the DTS-delta field is encoded in the AU-header*/
	u32 DTSDeltaLength;
	/*The number of bits on which the CTS-delta field is encoded in the AU-header*/
	u32 CTSDeltaLength;
	/*random access point flag present*/
	Bool RandomAccessIndication;
	
	/*The number of bits on which the Stream-state field is encoded in the AU-header (systems only)*/
	u32 StreamStateIndication;
	/*The number of bits that is used to encode the auxiliary-data-size field 
	(no normative usage of this section)*/
	u32 AuxiliaryDataSizeLength;

	/*ISMACryp stuff*/
	u8 IV_length, IV_delta_length;
	u8 KI_length;

	/*internal stuff*/
	/*len of first AU header in an RTP payload*/
	u32 auh_first_min_len;
	u32 auh_min_len;
} GP_RTPSLMap;
	

/*packetizer config flags - some flags are dynamically re-assigned when detecting multiSL / B-Frames / ...*/
enum
{
	/*forces MPEG-4 generic transport if MPEG-4 systems mapping is available*/
	GP_RTP_PCK_FORCE_MPEG4 =	(1),
	/*Enables AUs concatenation in an RTP packet (if payload supports it) - this forces GP_RTP_PCK_SIGNAL_SIZE for MPEG-4*/
	GP_RTP_PCK_USE_MULTI	=	(1<<1),
	/*if set, audio interleaving is used if payload supports it (forces GP_RTP_PCK_USE_MULTI flag)
		THIS IS CURRENTLY NOT IMPLEMENTED*/
	GP_RTP_PCK_USE_INTERLEAVING =	(1<<2),
	/*uses static RTP payloadID if any defined*/
	GP_RTP_PCK_USE_STATIC_ID =	(1<<3),

	/*MPEG-4 generic transport option*/
	/*if flag set, RAP flag is signaled in RTP payload*/
	GP_RTP_PCK_SIGNAL_RAP	=	(1<<4),
	/*if flag set, AU indexes are signaled in RTP payload - only usable for AU interleaving (eg audio)*/
	GP_RTP_PCK_SIGNAL_AU_IDX	=	(1<<5),
	/*if flag set, AU size is signaled in RTP payload*/
	GP_RTP_PCK_SIGNAL_SIZE	=	(1<<6),
	/*if flag set, CTS is signaled in RTP payload - DTS is automatically set if needed*/
	GP_RTP_PCK_SIGNAL_TS	=	(1<<7),

	/*setup payload for carouseling of systems streams*/
	GP_RTP_PCK_SYSTEMS_CAROUSEL = (1<<8),

	/*use LATM payload for AAC-LC*/
	GP_RTP_PCK_USE_LATM_AAC	=	(1<<9),

	/*ISMACryp options*/
	/*signals that input data is selectively encrypted (eg not all input frames are encrypted) 
	- this is usually automatically set by hinter*/
	GP_RTP_PCK_SELECTIVE_ENCRYPTION =	(1<<10),
	/*signals that each sample will have its own key indicator - ignored in non-multi modes
	if not set and key indicator changes, a new RTP packet will be forced*/
	GP_RTP_PCK_KEY_IDX_PER_AU =	(1<<11),

	/*is zip compression used in DIMS unit ?*/
	GP_RTP_DIMS_COMPRESSED =	(1<<12),
};



/*
		Generic packetization tools - used by track hinters and future live tools
*/

/*currently supported payload types*/
enum 
{
	/*not defined*/
	GF_RTP_PAYT_UNKNOWN,
	/*use generic MPEG-4 transport - RFC 3016 and RFC 3640*/
	GF_RTP_PAYT_MPEG4,
	/*use generic MPEG-1/2 video transport - RFC 2250*/
	GF_RTP_PAYT_MPEG12_VIDEO,
	/*use generic MPEG-1/2 audio transport - RFC 2250*/
	GF_RTP_PAYT_MPEG12_AUDIO,
	/*use H263 transport - RFC 2429*/
	GF_RTP_PAYT_H263,
	/*use AMR transport - RFC 3267*/
	GF_RTP_PAYT_AMR,
	/*use AMR-WB transport - RFC 3267*/
	GF_RTP_PAYT_AMR_WB,
	/*use QCELP transport - RFC 2658*/
	GF_RTP_PAYT_QCELP,
	/*use EVRC/SMV transport - RFC 3558*/
	GF_RTP_PAYT_EVRC_SMV,
	/*use 3GPP Text transport - no RFC yet, only draft*/
	GF_RTP_PAYT_3GPP_TEXT,
	/*use H264 transport - no RFC yet, only draft*/
	GF_RTP_PAYT_H264_AVC,
	/*use LATM for AAC-LC*/
	GF_RTP_PAYT_LATM,
	/*use 3GPP DIMS format*/
	GF_RTP_PAYT_3GPP_DIMS,
	/*use AC3 audio format*/
	GF_RTP_PAYT_AC3,
};



/*
	RTP packetizer
*/



/*
		RTP -> SL packetization tool
	You should ONLY modify the GF_SLHeader while packetizing, all the rest is private
	to the tool.
	Also note that AU start/end is automatically updated, therefore you should only
	set CTS-DTS-OCR-sequenceNumber (which is automatically incremented when spliting a payload)
	-padding-idle infos
	SL flags are computed on the fly, but you may wish to modify them in case of 
	packet drop/... at the encoder side

*/
struct __tag_rtp_packetizer
{
	/*input packet sl header cfg. modify only if needed*/
	GF_SLHeader sl_header;

	/*
	
		PRIVATE _ DO NOT TOUCH
	*/
	
	/*RTP payload type (RFC type, NOT the RTP hdr payT)*/
	u32 rtp_payt;
	/*packetization flags*/
	u32 flags;
	/*Path MTU size without 12-bytes RTP header*/
	u32 Path_MTU;
	/*max packet duration in RTP TS*/
	u32 max_ptime;

	/*payload type of RTP packets - only one payload type can be used in GPAC*/
	u8 PayloadType;

	/*RTP header of current packet*/
	GF_RTPHeader rtp_header;

	/*RTP packet handling callbacks*/
	void (*OnNewPacket)(void *cbk_obj, GF_RTPHeader *header);
	void (*OnPacketDone)(void *cbk_obj, GF_RTPHeader *header);
	void (*OnDataReference)(void *cbk_obj, u32 payload_size, u32 offset_from_orig);
	void (*OnData)(void *cbk_obj, char *data, u32 data_size, Bool is_header);
	void *cbk_obj;
	
		/*********************************
			MPEG-4 Generic hinting 
		*********************************/

	/*SL to RTP map*/
	GP_RTPSLMap slMap;
	/*SL conf and state*/
	GF_SLConfig sl_config;

	/*set to 1 if firstSL in RTP packet*/
	Bool first_sl_in_rtp;
	Bool has_AU_header;
	/*current info writers*/
	GF_BitStream *pck_hdr, *payload;

	/*AU SN of last au*/
	u32 last_au_sn;

	/*info for the current packet*/
	u32 auh_size, bytesInPacket;

		/*********************************
				ISMACryp info
		*********************************/
	Bool force_flush, is_encrypted;
	u64 IV, first_AU_IV;
	char *key_indicator;

		/*********************************
				AVC-H264 info
		*********************************/
	/*AVC non-IDR flag: set if all NAL in current packet are non-IDR (disposable)*/
	Bool avc_non_idr;

		/*********************************
				AC3 info
		*********************************/
	/*ac3 ft flags*/
	u8 ac3_ft;

};

/*generic rtp builder (packetizer)*/
typedef struct __tag_rtp_packetizer GP_RTPPacketizer;

/*creates a new builder
	@rtp_payt: rtp payload format, one of the above GF_RTP_PAYT_*
	@flags: packetizer flags, one of the above GP_RTP_PCK_*
	@slc: user-given SL config to use. If none specified, default RFC config is used
	@cbk_obj: callback object passed back in functions
	@OnNewPacket: callback function starting new RTP packet
		@header: rtp header for new packet - note that RTP header flags are not used until PacketDone is called
	@OnPacketDone: callback function closing current RTP packet
		@header: final rtp header for packet
	@OnDataReference: optional, to call each time data from input buffer is added to current RTP packet. 
		If not set, data must be added through OnData
		@payload_size: size of reference data
		@offset_from_orig: start offset in input buffer
	@OnData: to call each time data is added to current RTP packet (either extra data from payload or
		data from input when not using referencing)
		@is_head: signal the data added MUST be inserted at the begining of the payload. Otherwise data
		is concatenated as received
*/
GP_RTPPacketizer *gf_rtp_builder_new(u32 rtp_payt, 
						GF_SLConfig *slc, 
						u32 flags,
						void *cbk_obj, 
						void (*OnNewPacket)(void *cbk, GF_RTPHeader *header),
						void (*OnPacketDone)(void *cbk, GF_RTPHeader *header),
						void (*OnDataReference)(void *cbk, u32 payload_size, u32 offset_from_orig),
						void (*OnData)(void *cbk, char *data, u32 data_size, Bool is_head)
					);

/*destroy builder*/
void gf_rtp_builder_del(GP_RTPPacketizer *builder);

/*
		init the builder
	@MaxPayloadSize: maximum payload size of RTP packets (eg MTU minus IP/UDP/RTP headers)
	@max_ptime: maximum packet duration IN RTP TIMESCALE
	@StreamType: MPEG-4 system stream type - MUST always be provided for payloads format specifying 
		audio or video streams
	@OTI : MPEG-4 system objectTypeIndication - may be 0 if stream is not mpeg4 systems

			*** all other params are for MultiSL draft ***
	
	  @avgSize: average size of an AU. This is not always known (real-time encoding). 
In this case you should specify a rough compute indicating how many packets could be 
stored per RTP packet. for ex AAC stereo at 44100 k / 64kbps , one AU ~= 380 bytes
so 3 AUs for 1500 MTU is ok - BE CAREFULL: MultiSL adds some SL info on top of the 12
byte RTP header so you should specify a smaller size
The packetizer will ALWAYS make sure there's no pb storing the packets so specifying 
more will result in a slight overhead in the SL mapping but the gain to singleSL 
will still be worth it.
	-Nota: at init, the packetizer can decide to switch to SingleSL if the average size 
specified is too close to the PathMTU

	@maxSize: max size of an AU. If unknown (real-time) set to 0
	@avgTS: average CTS progression (1000/FPS for video)
	@maxDTS: maximum DTS offset in case of bidirectional coding. 
	@IV_length: size (in bytes) of IV when ISMACrypted
	@KI_length: size (in bytes) of key indicator when ISMACrypted
	@pref_mode: MPEG-4 generic only, specifies the payload mode - can be NULL (mode generic)
*/

void gf_rtp_builder_init(GP_RTPPacketizer *builder, u8 PayloadType, u32 MaxPayloadSize, u32 max_ptime,
					   u32 StreamType, u32 OTI, u32 PL_ID,
					   u32 avgSize, u32 maxSize, 
					   u32 avgTS, u32 maxDTS,
					   u32 IV_length, u32 KI_length,
					   char *pref_mode);

/*set frame crypto info*/
void gp_rtp_builder_set_cryp_info(GP_RTPPacketizer *builder, u64 IV, char *key_indicator, Bool is_encrypted);
/*packetize input buffer
@data, @data_size: input buffer
@IsAUEnd: set to one if this buffer is the last of the AU
@FullAUSize: complete access unit size if known, 0 otherwise
@duration: sample duration in rtp timescale (only needed for 3GPP text streams)
@descIndex: sample description index (only needed for 3GPP text streams)
*/
GF_Err gf_rtp_builder_process(GP_RTPPacketizer *builder, char *data, u32 data_size, u8 IsAUEnd, u32 FullAUSize, u32 duration, u8 descIndex);

/*format the "fmtp: " attribute for the MPEG-4 generic packetizer. sdpline shall be at least 2000 char*/
GF_Err gf_rtp_builder_format_sdp(GP_RTPPacketizer *builder, char *payload_name, char *sdpLine, char *dsi, u32 dsi_size);
/*formats SDP payload name and media name - both MUST be at least 20 bytes*/
Bool gf_rtp_builder_get_payload_name(GP_RTPPacketizer *builder, char *szPayloadName, char *szMediaName);





/*rtp payload flags*/
enum
{
	/*AU end was detected (eg next packet is AU start)*/
	GF_RTP_NEW_AU = (1),
	/*AMR config*/
	GF_RTP_AMR_ALIGN = (1<<1),
	/*for RFC3016, signals bitstream inspection for RAP discovery*/
	GF_RTP_M4V_CHECK_RAP = (1<<2),
	/*flag set when unreliable usage of the M bit is detected*/
	GF_RTP_UNRELIABLE_M = (1<<3),

	/*AWFULL hack at rtp level to cope with ffmpeg h264 crashes when jumping in stream without IDR*/
	GF_RTP_AVC_WAIT_RAP = (1<<4),
	/*ISMACryp stuff*/
	GF_RTP_HAS_ISMACRYP = (1<<5),
	GF_RTP_ISMA_SEL_ENC = (1<<6),
	GF_RTP_ISMA_HAS_KEY_IDX = (1<<7)
};

/*
		SL -> RTP packetization tool

*/
struct __tag_rtp_depacketizer
{
	/*depacketize routine*/
	void (*depacketize)(struct __tag_rtp_depacketizer *rtp, GF_RTPHeader *hdr, char *payload, u32 size);

	/*output packet sl header cfg*/
	GF_SLHeader sl_hdr;

	/*RTP payload type (RFC type, NOT the RTP hdr payT)*/
	u32 payt;
	/*depacketization flags*/
	u32 flags;

	/*callback routine*/
	void (*on_sl_packet)(void *udta, char *payload, u32 size, GF_SLHeader *hdr, GF_Err e);
	void *udta;

	/*SL <-> RTP map*/
	GP_RTPSLMap sl_map;
	u32 clock_rate;

	/*inter-packet reconstruction bitstream (for 3GP text and H264)*/
	GF_BitStream *inter_bs;

	/*H264/AVC config*/
	u32 h264_pck_mode;
	
	/*3GP text reassembler state*/
	u8 nb_txt_frag, cur_txt_frag, sidx, txt_len, nb_mod_frag;

	/*ISMACryp*/
	u32 isma_scheme;
	char *key;
};

/*generic rtp builder (packetizer)*/
typedef struct __tag_rtp_depacketizer GF_RTPDepacketizer;

GF_RTPDepacketizer *gf_rtp_depacketizer_new(GF_SDPMedia *media, void (*sl_packet_cbk)(void *udta, char *payload, u32 size, GF_SLHeader *hdr, GF_Err e), void *udta);
void gf_rtp_depacketizer_del(GF_RTPDepacketizer *rtp);
void gf_rtp_depacketizer_reset(GF_RTPDepacketizer *rtp, Bool full_reset);
void gf_rtp_depacketizer_process(GF_RTPDepacketizer *rtp, GF_RTPHeader *hdr, char *payload, u32 size);

void gf_rtp_depacketizer_get_slconfig(GF_RTPDepacketizer *rtp, GF_SLConfig *sl);


#endif /*GPAC_DISABLE_STREAMING*/

#ifdef __cplusplus
}
#endif

#endif		/*_GF_IETF_H_*/

