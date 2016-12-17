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

#ifndef _GF_ISOMRTPStreamer_H_
#define _GF_ISOMRTPStreamer_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *	\file <gpac/filestreamer.h>
 *	\brief RTP file streamer functions.
 */

/*!
 *	\addtogroup media_grp FileStreamer
 *	\ingroup media_grp
 *	\brief FileStreamer object
 *
 *	This section documents the list object of the GPAC framework.
 *	@{
 */

#include <gpac/tools.h>


typedef struct __isom_rtp_streamer GF_ISOMRTPStreamer;

/*!
 *	\brief ISO File RTP Streamer list constructor
 *
 *	Constructs a new ISO file RTP streamer
 *\param file_name source file name to stream. Hint tracks will be ignored, all media tracks will be streamed
 *\param ip_dest destination IP address (V4 or V6, unicast or multicast)
 *\param port destination port
 *\param loop whether streaming stops at the end of all tracks or not. If not, RTP TS will continuously be incremented
 *\param force_mpeg4 forces usage of MPEG-4 generic (RFC3640) for all streams
 *\param path_mtu maximum RTP packet payload size allowed
 *\param ttl multicast time to live
 *\param ifce_addr IP of the local interface to use (may be NULL)
 *\return new list object
 */
GF_ISOMRTPStreamer *gf_isom_streamer_new(const char *file_name, const char *ip_dest, u16 port, Bool loop, Bool force_mpeg4, u32 path_mtu, u32 ttl, char *ifce_addr);

/*!
 *	\brief RTP file streamer destructor
 *
 *	Destructs an RTP file streamer
 *	\param ptr object to destruct
 */
void gf_isom_streamer_del(GF_ISOMRTPStreamer *streamer);

/*!
 *	\brief writes the SDP file
 *
 *	Writes the SDP file asscoiated with the streaming session
 *	\param streamer RTP streamer object
 *	\param sdpfilename SDP file name to create
 */
GF_Err gf_isom_streamer_write_sdp(GF_ISOMRTPStreamer *streamer, char*sdpfilename);

/*!
 *	\brief gets the SDP file
 *
 *	Gets the SDP asscoiated with all media in the streaming session (only media parts are returned)
 *	\param streamer RTP streamer object
 *	\param out_sdp_buffer location to the SDP buffer to allocate and fill
 */
GF_Err gf_isom_streamer_get_sdp(GF_ISOMRTPStreamer *streamer, char **out_sdp_buffer);


/*!
 *	\brief sends RTP packet
 *
 *	Sends the next RTP packet in the current file, potentially waiting for the TS to be mature. If the last packet is sent and looping is disabled, this will return GF_EOS.
 *	\param streamer RTP streamer object
 *	\param send_ahead_delay delay in milliseconds for packet sending. A packet is sent if (packet.timestamp + send_ahead_delay) is greate than the current time.
 *	\param max_sleep_time indicates that if the streamer has to wait more than max_sleep_time before sending the packet, it should return and send it later. 
 */
GF_Err gf_isom_streamer_send_next_packet(GF_ISOMRTPStreamer *streamer, s32 send_ahead_delay, s32 max_sleep_time);

/*!
 *	\brief resets RTP sender
 *
 *	Reset the RTP streams to the beginning of the media file
 *	\param streamer RTP streamer object
 *	\param is_loop indicates whether the RTP timelines shall continue from the end of the file or shall restart from 0
 */
void gf_isom_streamer_reset(GF_ISOMRTPStreamer *streamer, Bool is_loop);

/*! @} */

#ifdef __cplusplus
}
#endif


#endif		/*_GF_ISOMRTPStreamer_H_*/

