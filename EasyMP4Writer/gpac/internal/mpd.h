/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Authors: Cyril Concolato
 *			Copyright (c) Telecom ParisTech 2010-
 *					All rights reserved
 *
 *  This file is part of GPAC / 3GPP/MPEG Media Presentation Description input module
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
#ifndef _MPD_IN_H_
#define _MPD_IN_H_

#include <gpac/constants.h>
#include <gpac/xml.h>
#include <gpac/media_tools.h>
#include <gpac/internal/terminal_dev.h>

typedef struct
{
    char *url;
    Bool use_byterange;
    u32 byterange_start;
    u32 byterange_end;
} GF_MPD_SegmentInfo;

typedef struct {
    char *id;
    u32 bandwidth;
    u32 width;
    u32 height;
    char *lang;
    char *mime;
    u32 groupID;
    Bool disabled;
    Bool startWithRap;
	/* TODO: maximumRAPPeriod */
	/* TODO: depid*/
	/* TODO: default rep*/

    u32 qualityRanking;
    char *content_protection_type;
    char *content_protection_uri;
    double alternatePlayoutRate;
    u32 default_segment_duration;
	/*TODO: multiple views */
    char *default_base_url;

    /* initialization segment */
    char *init_url;
    Bool init_use_range;
    u32 init_byterange_start;
    u32 init_byterange_end;
    
    /* other segments */
    char *url_template;
    u32 startIndex;
    u32 endIndex;

	GF_List *segments;
} GF_MPD_Representation;

typedef struct {
    u32 start; /* expressed in seconds, relative to the start of the MPD */
	u32 duration; /* TODO */
	char *id; /* TODO */
    u8 flags;
    Bool segment_alignment_flag; /* to be merged into real flags */
    Bool bitstream_switching_flag;

    u32 default_segment_duration; /* milliseconds */
    char *default_base_url;
	/* TODO: default timeline */
    char *url_template;

	/* TODO: xlink:href & xlink:actuate */

    GF_List *representations;
	/* TODO: representation groups */
	/* TODO: subset */
} GF_MPD_Period;

typedef enum {
    GF_MPD_TYPE_ON_DEMAND,
    GF_MPD_TYPE_LIVE,
} GF_MPD_Type;

typedef struct {
    GF_MPD_Type type;
    char *base_url;
	/* TODO: add alternate URL */
    u32 duration; /* expressed in milliseconds */
    u32 min_update_time; /* expressed in milliseconds */
    u32 min_buffer_time; /* expressed in milliseconds */
    /*start time*/
    /*end time*/
    u32 time_shift_buffer_depth; /* expressed in milliseconds */
    char *title;
    char *source;
    char *copyright;
    char *more_info_url;

    /* the number of periods is dynamic since we may update the MPD from time to time, cannot avoid GF_List */
    GF_List *periods;
} GF_MPD;

GF_Err gf_mpd_init_from_dom(GF_XMLNode *root, GF_MPD *mpd, const char *base_url);

GF_MPD *gf_mpd_new();
void gf_mpd_del(GF_MPD *mpd);

GF_Err gf_m3u8_to_mpd(GF_ClientService *service, const char *m3u8_file, const char *base_url, 
					  const char *mpd_file,
					  u32 reload_count, char *mimeTypeForM3U8Segments);

#endif // _MPD_IN_H_

