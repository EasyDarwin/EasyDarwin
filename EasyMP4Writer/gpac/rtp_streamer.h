/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2008-
 *					All rights reserved
 *
 *  This file is part of GPAC / media tools sub-project
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

#ifndef _GF_RTPSTREAMER_H_
#define _GF_RTPSTREAMER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *	\file <gpac/rtp_streamer.h>
 *	\brief RTP streamer functions (packetizer and RTP socket).
 */

/*!
 *	\addtogroup media_grp RTPStreamer
 *	\ingroup media_grp
 *	\brief RTPStreamer object
 *
 *	This section documents the RTP streamer object of the GPAC framework.
 *	@{
 */

#include <gpac/ietf.h>
#include <gpac/isomedia.h>

#ifndef GPAC_DISABLE_STREAMING
    
typedef struct __rtp_streamer GF_RTPStreamer;

/*!
 *	\brief RTP Streamer constructor
 *
 *	Constructs a new RTP file streamer
 *\param ifce_addr IP of the local interface to use (may be NULL)
 *\return new object
 */

GF_RTPStreamer *gf_rtp_streamer_new(u32 streamType, u32 oti, u32 timeScale, 
								const char *ip_dest, u16 port, u32 MTU, u8 TTL, const char *ifce_addr, 
								u32 flags, char *dsi, u32 dsi_len);


GF_RTPStreamer *gf_rtp_streamer_new_extended(u32 streamType, u32 oti, u32 timeScale, 
								const char *ip_dest, u16 port, u32 MTU, u8 TTL, const char *ifce_addr, 
								 u32 flags, char *dsi, u32 dsi_len, 								 
								 u32 PayloadType, u32 sample_rate, u32 nb_ch,
								 Bool is_crypted, u32 IV_length, u32 KI_length,
								 u32 MinSize, u32 MaxSize, u32 avgTS, u32 maxDTSDelta, u32 const_dur, u32 bandwidth, u32 max_ptime, 
								 u32 au_sn_len);

/*!
 *	\brief RTP file streamer destructor
 *
 *	Destructs an RTP file streamer
 *	\param ptr object to destruct
 */
void gf_rtp_streamer_del(GF_RTPStreamer *streamer);

/*!
 *	\brief gets the SDP file
 *
 *	Gets the SDP asscoiated with all media in the streaming session (only media parts are returned)
 *	\param streamer RTP streamer object
 *	\param out_sdp_buffer location to the SDP buffer to allocate and fill
 */
GF_Err gf_rtp_streamer_append_sdp(GF_RTPStreamer *rtp, u16 ESID, char *dsi, u32 dsi_len, char *KMS_URI, char **out_sdp_buffer);

GF_Err gf_rtp_streamer_append_sdp_extended(GF_RTPStreamer *rtp, u16 ESID, char *dsi, u32 dsi_len, GF_ISOFile *isofile, u32 isotrack, char *KMS_URI, u32 width, u32 height, char **out_sdp_buffer) ;

GF_Err gf_rtp_streamer_send_au(GF_RTPStreamer *rtp, char *data, u32 size, u64 cts, u64 dts, Bool is_rap);

GF_Err gf_rtp_streamer_send_au_with_sn(GF_RTPStreamer *rtp, char *data, u32 size, u64 cts, u64 dts, Bool is_rap, Bool inc_au_sn);

GF_Err gf_rtp_streamer_send_data(GF_RTPStreamer *rtp, char *data, u32 size, u32 fullsize, u64 cts, u64 dts, Bool is_rap, Bool au_start, Bool au_end, u32 au_sn, u32 sampleDuration, u32 sampleDescIndex);

char *gf_rtp_streamer_format_sdp_header(char *app_name, char *ip_dest, char *session_name, char *iod64);

void gf_rtp_streamer_disable_auto_rtcp(GF_RTPStreamer *streamer);

GF_Err gf_rtp_streamer_send_rtcp(GF_RTPStreamer *streamer, Bool force_ts, u32 rtp_ts);

/*! @} */

#ifdef __cplusplus
}
#endif

#endif //GPAC_DISABLE_STREAMING

#endif		/*_GF_RTPSTREAMER_H_*/

