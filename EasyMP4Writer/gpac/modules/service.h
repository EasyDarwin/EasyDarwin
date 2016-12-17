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

#ifndef _GF_SERVICE_H_
#define _GF_SERVICE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*for SL, ESD and OD*/
#include <gpac/mpeg4_odf.h>
#include <gpac/events.h>
#include <gpac/download.h>

/*handle to service*/
typedef struct _net_service GF_ClientService;

/*handle to channel*/
typedef void *LPNETCHANNEL;

enum
{
	/*channel control, app->module. Note that most modules don't need to handle pause/resume/set_speed*/
	GF_NET_CHAN_PLAY,
	GF_NET_CHAN_STOP,
	GF_NET_CHAN_PAUSE,
	GF_NET_CHAN_RESUME,
	GF_NET_CHAN_SET_SPEED,
	/*channel configuration, app->module*/
	GF_NET_CHAN_CONFIG,
	/*channel duration, app<->module (in case duration is not known at setup)*/
	GF_NET_CHAN_DURATION,
	/*channel buffer, app->module*/
	GF_NET_CHAN_BUFFER,
	/*channel buffer query, app<-module*/
	GF_NET_CHAN_BUFFER_QUERY,
	/*retrieves DSI from channel (DSI may be caried by net with a != value than OD), app->module*/
	GF_NET_CHAN_GET_DSI,
	/*set media padding for all AUs fetched (pull mode only). 
	If not supported the channel will have to run in push mode. app->module*/
	GF_NET_CHAN_SET_PADDING,
	/*sets input channel to pull mode if possible, app->module*/
	GF_NET_CHAN_SET_PULL,
	/*query channel capability to pause/resume and seek(play from an arbitrary range)
	a non-interactive channel doesn't have to handle SET_SPEED, PAUSE and RESUME commands but can 
	still work in pull mode*/
	GF_NET_CHAN_INTERACTIVE,
	/*map net time (OTB) to media time (up only) - this is needed by some signaling protocols when the 
	real play range is not the requested one */
	GF_NET_CHAN_MAP_TIME,
	/*reconfiguration of channel comming from network (up only) - this is used to override the SL config
	if it differs from the one specified at config*/
	GF_NET_CHAN_RECONFIG,
	/*signal channel is ISMACryp'ted (net->term only)*/
	GF_NET_CHAN_DRM_CFG,
	
	/*retrieves ESD for channel - net->term only, for cache configuration*/
	GF_NET_CHAN_GET_ESD,
	/*retrieves visual PAR as indicated in container if any*/
	GF_NET_CHAN_GET_PIXEL_AR,
	
	/*service buffer query (for all channels running in service), app<-module*/
	GF_NET_BUFFER_QUERY,
	/*retrieves network stats for service/channel; app->module*/
	GF_NET_GET_STATS,
	/*retrieves whether service can be cached (rtp, http streaming radios, etc) or not. No associated struct*/
	GF_NET_IS_CACHABLE,

	/*sets info for service - net->term only*/
	GF_NET_SERVICE_INFO,
	/*checks if there is an audio stream in the service - term->net only*/
	GF_NET_SERVICE_HAS_AUDIO,
	/*instructs the service to get the migration info - term->net only*/
	GF_NET_SERVICE_MIGRATION_INFO,

	/*When using DASH or playlists, query the next file to concatenate to thecurrent one net->proxy only*/
	GF_NET_SERVICE_QUALITY_SWITCH,

	/*When using DASH or playlists, query the next file to concatenate to thecurrent one net->proxy only*/
	GF_NET_SERVICE_QUERY_NEXT,
};

/*channel command for all commands that don't need params:
GF_NET_CHAN_SET_PULL: module shall return GF_OK or GF_NOT_SUPPORTED
GF_NET_CHAN_INTERACTIVE: module shall return GF_OK or GF_NOT_SUPPORTED
*/
typedef struct
{
	/*command type*/
	u32 command_type;
	/*channel*/
	LPNETCHANNEL on_channel;
} GF_NetComBase;

/*GF_NET_CHAN_PLAY, GF_NET_CHAN_SET_SPEED*/
typedef struct
{
	u32 command_type;
	LPNETCHANNEL on_channel;
	/*params for GF_NET_CHAN_PLAY, ranges in sec - if range is <0, then it is ignored (eg [2, -1] with speed>0 means 2 +oo) */
	Double start_range, end_range;
	/*params for GF_NET_CHAN_PLAY and GF_NET_CHAN_SPEED*/
	Double speed;
} GF_NetComPlay;


/*GF_NET_CHAN_CONFIG, GF_NET_CHAN_RECONFIG
channel config may happen as soon as the channel is open, even if the module hasn't acknowledge creation
channel config can also be used from network to app, with GF_NET_CHAN_RECONFIG type - only the SL config is then used
*/
typedef struct
{
	u32 command_type;
	LPNETCHANNEL on_channel;

	/*SL config of the stream as delivered in OD (app->channel) or by network (channel->app)*/
	GF_SLConfig sl_config;		
	/*stream priority packet drops are more tolerable if low priority - app->channel only*/
	u32 priority;
	/*sync ID: all channels with the same sync ID run on the same timeline, thus the module should 
	try to match this - note this may not be possible (typically RTP/RTSP)*/
	u32 sync_id;
	/*audio frame duration and sample rate if any - this is needed by some RTP payload*/
	u32 frame_duration, sample_rate;
	/*do we use MPEG-2 section for SL packets ? this field is not coded*/
	Bool use_m2ts_sections;
} GF_NetComConfig;

/*GF_NET_CHAN_BUFFER, GF_NET_CHAN_BUFFER_QUERY*/
typedef struct
{
	u32 command_type;
	LPNETCHANNEL on_channel;
	/*the recommended buffering limits in ms - this depends on the modules preferences and on the service 
	type (multicast, vod, ...) - below buffer_min the stream will pause if possible until buffer_max is reached
	note the app will fill in default values before querying*/
	u32 min, max;
	/*only used with GF_NET_CHAN_BUFFER_QUERY - amount of media in decoding buffer, in ms*/
	u32 occupancy;
} GF_NetComBuffer;

/*GF_NET_CHAN_DURATION*/
typedef struct
{
	u32 command_type;
	LPNETCHANNEL on_channel;
	/*duration in sec*/
	Double duration;
} GF_NetComDuration;

/*GF_NET_CHAN_GET_DSI*/
typedef struct
{
	u32 command_type;
	LPNETCHANNEL on_channel;
	/*carries specific info for codec - data shall be allocated by service and is freed by user*/
	char *dsi;
	u32 dsi_len;
} GF_NetComGetDSI;

/*GF_NET_CHAN_SET_PADDING*/
typedef struct
{
	u32 command_type;
	LPNETCHANNEL on_channel;
	u32 padding_bytes;
} GF_NetComPadding;

/*GF_NET_CHAN_MAP_TIME*/
typedef struct
{
	u32 command_type;
	LPNETCHANNEL on_channel;
	/*MediaTime at this timestamp*/
	Double media_time;
	/*TS where mapping is done (in SL TS resolution)*/
	u64 timestamp;
	/*specifies whether decoder input data shall be discarded or only have its timing updated*/
	Bool reset_buffers;
} GF_NetComMapTime;

/*GF_NET_CHAN_ISMACRYP_CFG*/
typedef struct
{
	u32 command_type;
	LPNETCHANNEL on_channel;


	/*per channel, regardless of DRM schemes (ISMA, OMA, )*/
	u32 scheme_version;
	u32 scheme_type;
	const char *scheme_uri;
	const char *kms_uri;
	/*OMA DRM info*/
	const char *contentID;
	u32 oma_drm_crypt_type;
	Bool oma_drm_use_pad, oma_drm_use_hdr;
	const char *oma_drm_textual_headers;
	u32 oma_drm_textual_headers_len;

	/*SHA-1 file hash*/
	u8 hash[20];
} GF_NetComDRMConfig;

/*GF_NET_CHAN_GET_ESD*/
typedef struct
{
	u32 command_type;
	LPNETCHANNEL on_channel;
	const GF_ESD *esd;
	Bool is_iod_stream;
} GF_NetComGetESD;

/*GF_NET_GET_STATS
Notes
1: only channels using network must reply. All channels fetching data through a 
file downloader (cf below) shall NOT answer, the app manages downloader bandwidth internally.
2: BANDWIDTH USED BY SIGNALING PROTOCOL IS IGNORED IN GPAC
*/
typedef struct __netstatcom
{
	u32 command_type;
	/*MAY BE NULL, in which case the module must fill in ONLY the control channel part. This
	is not used yet, but could be with a protocol using a single control socket for N media channels.*/
	LPNETCHANNEL on_channel;
	/*percentage of packet loss from network. This cannot be figured out by the app since there is no
	one-to-one mapping between the protocol packets and the final SL packet (cf RTP payloads)*/
	Float pck_loss_percentage;
	/*channel port, control channel port if any (eg RTCP)*/
	u16 port, ctrl_port;
	/*bandwidth used by channel & its control channel if any (both up and down) - expressed in bits per second*/
	u32 bw_up, bw_down, ctrl_bw_down, ctrl_bw_up;
	/*set to 0 if channel is not part of a multiplex. Otherwise set to the multiplex port, and 
	above port info shall be identifiers in the multiplex - note that multiplexing overhead is ignored 
	in GPAC for the current time*/
	u16 multiplex_port;
} GF_NetComStats;

/*GF_NET_CHAN_GET_PIXEL_AR*/
typedef struct
{
	u32 command_type;
	LPNETCHANNEL on_channel;
	u32 hSpacing, vSpacing;
	u32 width, height, pixel_format;
} GF_NetComPixelAR;

/*GF_NET_SERVICE_INFO*/
typedef struct __netinfocom
{
	u32 command_type;
	/*currently NULL only*/
	LPNETCHANNEL on_channel;
	/*packed trackNumber(16 bits)/totaltrack(16 bits)*/
	u32 track_info;
	u32 genre;
	const char *album;
	const char *artist;
	const char *comment;
	const char *composer;
	const char *name;
	const char *writer;
} GF_NetComInfo;

/*GF_NET_CHAN_GET_PIXEL_AR*/
typedef struct
{
	u32 command_type;
	char *base_url;
} GF_NetComHasAudio;

/*GF_NET_SERVICE_MIGRATION_INFO*/
typedef struct
{
	u32 command_type;

	/*out: migration data, allocated and freed by the plugin*/
	char *data;
	u32 data_len;
} GF_NetComMigration;

/*GF_NET_SERVICE_QUERY_NEXT*/
typedef struct
{
	u32 command_type;
	/*out: next url to play after current one*/
	const char *next_url;
} GF_NetURLQuery;

/*GF_NET_SERVICE_QUALITY_SWITCH*/
typedef struct
{
	u32 command_type;
	/*currently NULL only*/
	LPNETCHANNEL on_channel;
	/*out: next url to play after current one*/
	Bool up;
} GF_NetQualitySwitch;

typedef union __netcommand
{
	u32 command_type;
	GF_NetComBase base;
	GF_NetComPlay play;
	GF_NetComConfig cfg;
	GF_NetComBuffer buffer;
	GF_NetComDuration duration;
	GF_NetComGetDSI get_dsi;
	GF_NetComPadding pad;
	GF_NetComMapTime map_time;
	GF_NetComStats net_stats;
	GF_NetComDRMConfig drm_cfg;
	GF_NetComGetESD cache_esd;
	GF_NetComInfo info;
	GF_NetComPixelAR par;
	GF_NetComHasAudio audio;
	GF_NetComMigration migrate;
	GF_NetURLQuery url_query;
	GF_NetQualitySwitch switch_quality;
} GF_NetworkCommand;

/*
	network modules
*/

/*interface name and version for input service*/
#define GF_NET_CLIENT_INTERFACE			GF_4CC('G', 'I', 'S', '1')

typedef struct _netinterface
{
	/* interface declaration*/
	GF_DECL_MODULE_INTERFACE

	/*retuns 1 if module can process this URL, 0 otherwise. This is only called when the file extension/mimeType cannot be
	retrieved in the cfg file, otherwise the mime type/file ext is used to load service. Typically a module would 
	register its mime types in this function (cf gf_term_register_mime_type below)
	*/
	Bool (*CanHandleURL)(struct _netinterface *, const char *url);

	/*connects the service to the desired URL - the service handle is used for callbacks. 
	Only one service can be connected to a loaded interface.
	*/
	GF_Err (*ConnectService) (struct _netinterface *, GF_ClientService *serv, const char *url);

	/*disconnects service - the module is no longer used after this call - if immediate_shutdown is set the module
	shall not attempt to get confirmation from remote side, it will be deleted right away
	
	NOTE: depending on how the client/server exchange is happening, it may happen that the CloseService is called
	in the same context as a reply from your module. This can result into deadlocks if you're using threads. 
	You should therefore only try to destroy threads used in the interface shutdown process, which is guarantee
	to be in a different context call.
	*/
	GF_Err (*CloseService) (struct _netinterface *);

	/*retrieves service decsriptor (expressed as an MPEG4 OD/IOD) for accessing this service 
	descriptor is allocated by plugin and destroyed by user
	the IOD shall refer to the service attached to the module
	@expect_type is a hint in case the service regenerates an IOD. It indicates whether the entry point expected is 
	INLINE, BIFS animation stream, video, audio or input sensor.
	@sub_url: indicates fetching of an IOD for a given object in the service.
	Only used for services handling the optional CanHandleURLInService below
		NULL for main service
		service extension for sub-service (cf CanHandleURLInService below). For ex,
		"rtsp://myserver/file.mp4/ES_ID=3" and "rtsp://myserver/file.mp4/ES_ID=4" 
		or "file.avi#audio" and "file.avi#video".In this case a partial IOD for the desired object is expected
	Note: once a service is acknowledged as connected, this function must be executed synchronously
	The service can return NULL for a descriptor:
		* if the expected media type is a single media, this means the media couldn't be found
		* if the expected media type is a scene, this means the terminalk shall create and manage the scene
	*/
	GF_Descriptor *(*GetServiceDescriptor) (struct _netinterface *, u32 expect_type, const char *sub_url);
	

	/*sends command to the service / channel - cf command structure*/
	GF_Err (*ServiceCommand) (struct _netinterface *, GF_NetworkCommand *com);

	/*data channel setup - url is either
	"ES_ID=ID" where ID is the stream ID in this service
	or a control string depending on the service/stream. The URL is first used to load a module able to handle it, 
	so the module has no redirection to handle
	*/
	GF_Err (*ConnectChannel) (struct _netinterface *, LPNETCHANNEL channel, const char *url, Bool upstream);
	/*teardown of data channel*/
	GF_Err (*DisconnectChannel) (struct _netinterface *, LPNETCHANNEL channel);

	/*optional - fetch MPEG4 data from channel - data shall not be duplicated and must be released at ReleaseData
	SL info shall be written to provided header - if the data is a real SL packet the flag sl_compressed shall be 
	set to signal the app this is a full SL pdu (@out_sl_hdr is then ignored)
	set to NULL if not supported
	*/
	GF_Err (*ChannelGetSLP) (struct _netinterface *, LPNETCHANNEL channel, char **out_data_ptr, u32 *out_data_size, GF_SLHeader *out_sl_hdr, Bool *sl_compressed, GF_Err *out_reception_status, Bool *is_new_data);

	/*optional - release SLP data allocated on channel by the previous call, if any
	set to NULL if not supported*/
	GF_Err (*ChannelReleaseSLP) (struct _netinterface *, LPNETCHANNEL channel);

	/*this is needed for modules to query other modules, the typical case being 2 ESD URLs pointing to the 
	same media (audio and video streams in an RTSP session). This is always used on loaded modules but 
	doesn't have to be declared*/
	Bool (*CanHandleURLInService)(struct _netinterface *, const char *url);

/*private*/
	void *priv;

/*proxy stuff*/
	GF_Err (*query_proxy)(struct _netinterface *, GF_NetworkCommand *param);
	void *proxy_udta;
	/*!
	 * This is needed for modules supporting mime types, when this method is called,
	 * the module has to call gf_term_register_mime_type() for all the mime-types
	 * its supports.
	 * \return The number of declared mime types
	 * \see gf_term_register_mime_type(GF_InputService *, const char *, const char *, const char *)
	 */
	u32 (*RegisterMimeTypes) (const struct _netinterface *);
} GF_InputService;

/*callback functions - these can be linked with non-LGPL modules*/
/*message from service - error is set if error*/
void gf_term_on_message(GF_ClientService *service, GF_Err error, const char *message);
/*to call on service (if channel is NULL) or channel connect completed*/
void gf_term_on_connect(GF_ClientService *service, LPNETCHANNEL ns, GF_Err response);
/*to call on service (if channel is NULL) or channel disconnect completed*/
void gf_term_on_disconnect(GF_ClientService *service, LPNETCHANNEL ns, GF_Err response);
/* acknowledgement of service command - service commands handle both services and channels
Most of the time commands are NOT acknowledged, typical acknowledgement are needed for setup and control
with remote servers. 
command can also be triggered from the service (QoS, broadcast announcements)
cf above for command usage
*/
void gf_term_on_command(GF_ClientService *service, GF_NetworkCommand *com, GF_Err response);
/*to call when data packet is received. 
@data, data_size: data received
@hdr: uncompressed SL header passed with data for stream sync - if not present then data shall be a valid SL packet 
	(header + PDU). Note that using an SLConfig resulting in an empty GF_SLHeader allows sending raw data directly
@reception_status: data reception status. To signal end of stream, set this to GF_EOS
*/
void gf_term_on_sl_packet(GF_ClientService *service, LPNETCHANNEL ns, char *data, u32 data_size, GF_SLHeader *hdr, GF_Err reception_status);
/*returns URL associated with service (so that you don't need to store it)*/
const char *gf_term_get_service_url(GF_ClientService *service);

/*adds a new media from network. !! The media descriptor is then owned/destroyed by the term!!
media_desc: object descriptor for the new media. May be NULL to force scene rebuilt.
no_scene_check: specifies if the scene description shall be rebuilt or not.
*/
void gf_term_add_media(GF_ClientService *service, GF_Descriptor *media_desc, Bool no_scene_update);

Bool gf_term_on_service_event(GF_ClientService *service, GF_Event *service_event);


/*check if @fileExt extension is supported for given mimeType, and if associated with module. If mimeType not registered, register it for given module*/
Bool gf_term_check_extension(GF_InputService *ifce, const char *mimeType, const char *extList, const char *description, const char *fileExt);
/*register mime types & file extensions - most modules should only need the check version above*/
void gf_term_register_mime_type(const GF_InputService *ifce, const char *mimeType, const char *extList, const char *description);

GF_InputService *gf_term_get_service_interface(GF_ClientService *service);

/*file downloading - can and MUST be used by any module (regardless of license) in order not to interfere 
with net management*/
/*creates a new downloading session in the given service - if url is relative, it will be interpreted through
the service URL*/
GF_DownloadSession * gf_term_download_new(GF_ClientService *service, const char *url, u32 flags, gf_dm_user_io user_io, void *cbk);
/*closes the downloading session*/
void gf_term_download_del(GF_DownloadSession * dnload);
/*send progress and connection messages to user...*/
void gf_term_download_update_stats(GF_DownloadSession * sess);


/*MPEG-4 media cache interface name*/
#define GF_STREAMING_MEDIA_CACHE		GF_4CC('G', 'M', 'C', 0x01)

typedef struct _cacheinterface
{
	/* interface declaration*/
	GF_DECL_MODULE_INTERFACE

	/*opens media cache at given place - extension is handled by cache module
	@serv: service owning cache (eg, where to send data when requested)
	@keep_existing_files: don't overwrite previously recorded sessions*/
	GF_Err (*Open)(struct _cacheinterface *, GF_ClientService *serv, const char *location_and_name, Bool keep_existing_files);
	/*closes media cache, delete file(s) if desired*/
	GF_Err (*Close)(struct _cacheinterface *, Bool delete_cache);
	/*writes data to cache. data is always a complete AU as reconstructed by gpac core
	If first time data is written, user should query channel desc through service commands*/
	GF_Err (*Write)(struct _cacheinterface *, LPNETCHANNEL ch, char *data, u32 data_size, GF_SLHeader *sl_hdr);

	/*same as reader, except they MUST be provided - in other words, only PULL mode is supported for cache
	at the current time*/
	GF_Err (*ServiceCommand) (struct _cacheinterface *, GF_NetworkCommand *com);
	GF_Err (*ChannelGetSLP) (struct _cacheinterface *, LPNETCHANNEL channel, char **out_data_ptr, u32 *out_data_size, GF_SLHeader *out_sl_hdr, Bool *sl_compressed, GF_Err *out_reception_status, Bool *is_new_data);
	GF_Err (*ChannelReleaseSLP) (struct _cacheinterface *, LPNETCHANNEL channel);

	/*module private*/
	void *priv;
} GF_StreamingCache;


#ifdef __cplusplus
}
#endif

#endif	/*_GF_SERVICE_H_*/
