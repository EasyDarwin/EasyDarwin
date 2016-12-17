/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005 
 *					All rights reserved
 *
 *  This file is part of GPAC / ISO Media File Format sub-project
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



#ifndef _GF_ISOMEDIA_H_
#define _GF_ISOMEDIA_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/tools.h>

#ifndef GPAC_DISABLE_ISOM

#include <gpac/mpeg4_odf.h>

/*the isomedia file*/
typedef struct __tag_isom GF_ISOFile;

/*media sample object*/
typedef struct
{
	/*data size*/
	u32 dataLength;
	/*data with padding if requested*/
	char *data;
	/*decoding time*/
	u64 DTS;
	/*relative offset for composition if needed*/
	u32 CTS_Offset;
	/*Random Access Point flag:
	 0: not random access
	 1: regular RAP, 
	 2: sample is a redundant RAP. If set when adding the sample, this will create a sample dependency entry
	*/
	u8 IsRAP;
} GF_ISOSample;


/*creates a new empty sample*/
GF_ISOSample *gf_isom_sample_new();

/*delete a sample. NOTE:the buffer content will be destroyed by default.
if you wish to keep the buffer, set dataLength to 0 in the sample 
before deleting it
the pointer is set to NULL after deletion*/
void gf_isom_sample_del(GF_ISOSample **samp);



/********************************************************************
				FILE FORMAT CONSTANTS
********************************************************************/

/*Modes for file opening
		NOTE 1: All the READ function in this API can be used in EDIT/WRITE mode. 
However, some unexpected errors or values may happen in that case, depending
on how much modifications you made (timing, track with 0 samples, ...)
		On the other hand, none of the EDIT/WRITE functions will work in 
READ mode.
		NOTE 2: The output structure of a edited file will sometimes be different 
from the original file, but the media-data and meta-data will be identical.
The only change happens in the file media-data container(s) during edition
		NOTE 3: when editing the file, you MUST set the final name of the modified file
to something different. This API doesn't allow file overwriting.
*/
enum 
{
	/*Opens file for dumping: same as read-only but keeps all movie fragments info untouched*/
	GF_ISOM_OPEN_READ_DUMP = 0,
	/*Opens a file in READ ONLY mode*/
	GF_ISOM_OPEN_READ,
	/*Opens a file in WRITE ONLY mode. Media Data is captured on the fly. In this mode, 
	the editing functions are disabled.*/
	GF_ISOM_OPEN_WRITE,
	/*Opens an existing file in EDIT mode*/
	GF_ISOM_OPEN_EDIT,
	/*Creates a new file in EDIT mode*/
	GF_ISOM_WRITE_EDIT,
	/*Opens an existing file for fragment concatenation*/
	GF_ISOM_OPEN_CAT_FRAGMENTS,
};

/*Movie Options for file writing*/
enum
{
	/*FLAT: the MediaData (MPEG4 ESs) is stored at the begining of the file*/
	GF_ISOM_STORE_FLAT = 1,
	/*STREAMABLE: the MetaData (File Info) is stored at the begining of the file 
	for fast access during download*/
	GF_ISOM_STORE_STREAMABLE,
	/*INTERLEAVED: Same as STREAMABLE, plus the media data is mixed by chunk  of fixed duration*/
	GF_ISOM_STORE_INTERLEAVED,
	/*INTERLEAVED +DRIFT: Same as INTERLEAVED, and adds time drift control to avoid creating too long chunks*/
	GF_ISOM_STORE_DRIFT_INTERLEAVED,
	/*tightly interleaves samples based on their DTS, therefore allowing better placement of samples in the file.
	This is used for both http interleaving and Hinting optimizations*/
	GF_ISOM_STORE_TIGHT

};

/*Some track may depend on other tracks for several reasons. They reference these tracks 
through the following Reference Types*/
enum
{
	/*ref type for the OD track dependencies*/
	GF_ISOM_REF_OD			= GF_4CC( 'm', 'p', 'o', 'd' ),
	/*ref type for stream dependencies*/
	GF_ISOM_REF_DECODE = GF_4CC( 'd', 'p', 'n', 'd' ),
	/*ref type for OCR (Object Clock Reference) dependencies*/
	GF_ISOM_REF_OCR				= GF_4CC( 's', 'y', 'n', 'c' ),
	/*ref type for IPI (Intellectual Property Information) dependencies*/
	GF_ISOM_REF_IPI				= GF_4CC( 'i', 'p', 'i', 'r' ),
	/*ref type for timed Meta Data tracks*/
	GF_ISOM_REF_META		= GF_4CC( 'c', 'd', 's', 'c' ),
	/*ref type for Hint tracks*/
	GF_ISOM_REF_HINT		= GF_4CC( 'h', 'i', 'n', 't' ),
	/*ref type for QT Chapter tracks*/
	GF_ISOM_REF_CHAP		= GF_4CC( 'c', 'h', 'a', 'p' )
};

/*Track Edition flag*/
enum {
	/*empty segment in the track (no media for this segment)*/
	GF_ISOM_EDIT_EMPTY		=	0x00,
	/*dwelled segment in the track (one media sample for this segment)*/
	GF_ISOM_EDIT_DWELL		=	0x01,
	/*normal segment in the track*/
	GF_ISOM_EDIT_NORMAL		=	0x02
};

/*Generic Media Types (YOU HAVE TO USE ONE OF THESE TYPES FOR COMPLIANT ISO MEDIA FILES)*/
enum
{
	/*base media types*/
	GF_ISOM_MEDIA_VISUAL	= GF_4CC( 'v', 'i', 'd', 'e' ),
	GF_ISOM_MEDIA_AUDIO		= GF_4CC( 's', 'o', 'u', 'n' ),
	GF_ISOM_MEDIA_HINT		= GF_4CC( 'h', 'i', 'n', 't' ),
	GF_ISOM_MEDIA_META		= GF_4CC( 'm', 'e', 't', 'a' ),
	GF_ISOM_MEDIA_TEXT		= GF_4CC( 't', 'e', 'x', 't' ),
	/*subtitle code point used on ipod - same as text*/
	GF_ISOM_MEDIA_SUBT		= GF_4CC( 's', 'b', 't', 'l' ),
	GF_ISOM_MEDIA_SUBPIC	= GF_4CC( 's', 'u', 'b', 'p' ),

	/*MPEG-4 media types*/
	GF_ISOM_MEDIA_OD		= GF_4CC( 'o', 'd', 's', 'm' ),
	GF_ISOM_MEDIA_OCR		= GF_4CC( 'c', 'r', 's', 'm' ),
	GF_ISOM_MEDIA_SCENE		= GF_4CC( 's', 'd', 's', 'm' ),
	GF_ISOM_MEDIA_MPEG7		= GF_4CC( 'm', '7', 's', 'm' ),
	GF_ISOM_MEDIA_OCI		= GF_4CC( 'o', 'c', 's', 'm' ),
	GF_ISOM_MEDIA_IPMP		= GF_4CC( 'i', 'p', 's', 'm' ),
	GF_ISOM_MEDIA_MPEGJ		= GF_4CC( 'm', 'j', 's', 'm' ),
	/*GPAC-defined, for any track using MPEG-4 systems signaling but with undefined streaml types*/
	GF_ISOM_MEDIA_ESM		= GF_4CC( 'g', 'e', 's', 'm' ),

	/*DIMS media type (same as scene but with a different mediaInfo)*/
	GF_ISOM_MEDIA_DIMS		= GF_4CC( 'd', 'i', 'm', 's' ),

	GF_ISOM_MEDIA_FLASH		= GF_4CC( 'f', 'l', 's', 'h' )
};

/* Encryption Scheme Type in the SchemeTypeInfoBox */
enum 
{
	GF_ISOM_ISMACRYP_SCHEME	= GF_4CC( 'i', 'A', 'E', 'C' )
};

/*specific media sub-types - you shall make sure the media sub type is what you expect*/
enum
{
	/*reserved, internal use in the lib. Indicates the track complies to MPEG-4 system
	specification, and the usual OD framework tools may be used*/
	GF_ISOM_SUBTYPE_MPEG4		= GF_4CC( 'M', 'P', 'E', 'G' ),
	
	/*reserved, internal use in the lib. Indicates the track is of GF_ISOM_SUBTYPE_MPEG4
	but it is encrypted.*/
	GF_ISOM_SUBTYPE_MPEG4_CRYP	= GF_4CC( 'E', 'N', 'C', 'M' ),

	/*AVC/H264 media type - not listed as an MPEG-4 type, ALTHOUGH this library automatically remaps
	GF_AVCConfig to MPEG-4 ESD*/
	GF_ISOM_SUBTYPE_AVC_H264		= GF_4CC( 'a', 'v', 'c', '1' ),
	GF_ISOM_SUBTYPE_AVC2_H264		= GF_4CC( 'a', 'v', 'c', '2' ),
	GF_ISOM_SUBTYPE_SVC_H264		= GF_4CC( 's', 'v', 'c', '1' ),

	/*3GPP(2) extension subtypes*/
	GF_ISOM_SUBTYPE_3GP_H263		= GF_4CC( 's', '2', '6', '3' ),
	GF_ISOM_SUBTYPE_3GP_AMR		= GF_4CC( 's', 'a', 'm', 'r' ),
	GF_ISOM_SUBTYPE_3GP_AMR_WB	= GF_4CC( 's', 'a', 'w', 'b' ),
	GF_ISOM_SUBTYPE_3GP_EVRC		= GF_4CC( 's', 'e', 'v', 'c' ),
	GF_ISOM_SUBTYPE_3GP_QCELP	= GF_4CC( 's', 'q', 'c', 'p' ),
	GF_ISOM_SUBTYPE_3GP_SMV		= GF_4CC( 's', 's', 'm', 'v' ),

	/*3GPP DIMS*/
	GF_ISOM_SUBTYPE_3GP_DIMS	= GF_4CC( 'd', 'i', 'm', 's' ),

	GF_ISOM_SUBTYPE_AC3			= GF_4CC( 'a', 'c', '-', '3' ),
	GF_ISOM_SUBTYPE_SAC3		= GF_4CC( 's', 'a', 'c', '3' ),

	GF_ISOM_SUBTYPE_LSR1		= GF_4CC( 'l', 's', 'r', '1' ),
};




/*direction for sample search (including SyncSamples search)
Function using search allways specify the desired time in composition (presentation) time

		(Sample N-1)	DesiredTime		(Sample N)

FORWARD: will give the next sample given the desired time (eg, N)
BACKWARD: will give the previous sample given the desired time (eg, N-1)
SYNCFORWARD: will search from the desired point in time for a sync sample if any
		If no sync info, behaves as FORWARD
SYNCBACKWARD: will search till the desired point in time for a sync sample if any
		If no sync info, behaves as BACKWARD
SYNCSHADOW: use the sync shadow information to retrieve the sample.
		If no SyncShadow info, behave as SYNCBACKWARD
*/
enum
{
	GF_ISOM_SEARCH_FORWARD		=	1,
	GF_ISOM_SEARCH_BACKWARD		=	2,
	GF_ISOM_SEARCH_SYNC_FORWARD	=	3,
	GF_ISOM_SEARCH_SYNC_BACKWARD	=	4,
	GF_ISOM_SEARCH_SYNC_SHADOW		=	5
};

/*Predefined File Brand codes (MPEG-4 and JPEG2000)*/
enum
{
	/*file complying to the generic ISO Media File (base specification ISO/IEC 14496-12)
	this is the default brand when creating a new movie*/
	GF_ISOM_BRAND_ISOM = GF_4CC( 'i', 's', 'o', 'm' ),
	/*file complying to the generic ISO Media File (base specification ISO/IEC 14496-12) + Meta extensions*/
	GF_ISOM_BRAND_ISO2 =  GF_4CC( 'i', 's', 'o', '2' ),
	/*file complying to ISO/IEC 14496-1 2001 edition. A .mp4 file without a brand
	is equivalent to a file compatible with this brand*/
	GF_ISOM_BRAND_MP41 = GF_4CC( 'm', 'p', '4', '1' ),
	/*file complying to ISO/IEC 14496-14 (MP4 spec)*/
	GF_ISOM_BRAND_MP42 = GF_4CC( 'm', 'p', '4', '2' ),
	/*file complying to ISO/IEC 15444-3 (JPEG2000) without profile restriction*/
	GF_ISOM_BRAND_MJP2 = GF_4CC( 'm', 'j', 'p', '2' ),
	/*file complying to ISO/IEC 15444-3 (JPEG2000) with simple profile restriction*/
	GF_ISOM_BRAND_MJ2S = GF_4CC( 'm', 'j', '2', 's' ),
	/*old versions of 3GPP spec (without timed text)*/
	GF_ISOM_BRAND_3GP4 = GF_4CC('3', 'g', 'p', '4'),
	GF_ISOM_BRAND_3GP5 = GF_4CC('3', 'g', 'p', '5'),
	/*final version of 3GPP file spec*/
	GF_ISOM_BRAND_3GP6 = GF_4CC('3', 'g', 'p', '6'),
	/*generci 3GPP file (several audio tracks, etc..)*/
	GF_ISOM_BRAND_3GG6 = GF_4CC('3', 'g', 'g', '6'),
	/*3GPP2 file spec*/
	GF_ISOM_BRAND_3G2A = GF_4CC('3', 'g', '2', 'a'),
	/*AVC file spec*/
	GF_ISOM_BRAND_AVC1 = GF_4CC('a', 'v', 'c', '1'),
	/* file complying to ISO/IEC 21000-9:2005 (MPEG-21 spec)*/
	GF_ISOM_BRAND_MP21 = GF_4CC('m', 'p', '2', '1'),
};


/*MPEG-4 ProfileAndLevel codes*/
enum
{
	GF_ISOM_PL_AUDIO,
	GF_ISOM_PL_VISUAL,
	GF_ISOM_PL_GRAPHICS,
	GF_ISOM_PL_SCENE,
	GF_ISOM_PL_OD,
	GF_ISOM_PL_MPEGJ,
	/*not a profile, just set/unset inlineFlag*/
	GF_ISOM_PL_INLINE,
};


/********************************************************************
				GENERAL API FUNCTIONS
********************************************************************/

/*get the last fatal error that occured in the file
ANY FUNCTION OF THIS API WON'T BE PROCESSED IF THE FILE HAS AN ERROR
Note: some function may return an error while the movie has no error
the last error is a FatalError, and is not always set if a bad 
param is specified...*/
GF_Err gf_isom_last_error(GF_ISOFile *the_file);

/*returns 1 if target file is an IsoMedia file, 0 otherwise*/
Bool gf_isom_probe_file(const char *fileName);

/*Opens an isoMedia File.
tmp_dir: for the 2 edit modes only, specifies a location for temp file. If NULL, the librairy will use the default
OS temporary file management schemes.*/
GF_ISOFile *gf_isom_open(const char *fileName, u32 OpenMode, const char *tmp_dir);

/*close the file, write it if new/edited*/
GF_Err gf_isom_close(GF_ISOFile *the_file);

/*delete the movie without saving it.*/
void gf_isom_delete(GF_ISOFile *the_file);

/*Get the mode of an open file*/
u8 gf_isom_get_mode(GF_ISOFile *the_file);

Bool gf_isom_is_JPEG2000(GF_ISOFile *mov);

u64 gf_isom_get_file_size(GF_ISOFile *the_file);

/********************************************************************
				STREAMING API FUNCTIONS
********************************************************************/
/*open a movie that can be uncomplete in READ_ONLY mode
to use for http streaming & co

NOTE: you must buffer the data to a local file, this mode DOES NOT handle 
http/ftp/... streaming

BytesMissing is the predicted number of bytes missing for the file to be loaded
Note that if the file is not optimized for streaming, this number is not accurate
If the movie is successfully loaded (the_file non-NULL), BytesMissing is zero
*/
GF_Err gf_isom_open_progressive(const char *fileName, GF_ISOFile **the_file, u64 *BytesMissing);

/*If requesting a sample fails with error GF_ISOM_INCOMPLETE_FILE, use this function
to get the number of bytes missing to retrieve the sample*/
u64 gf_isom_get_missing_bytes(GF_ISOFile *the_file, u32 trackNumber);


/*Fragmented movie extensions*/

/*return 0 if movie isn't fragmented, 1 otherwise*/
u32 gf_isom_is_fragmented(GF_ISOFile *the_file);
/*return 0 if track isn't fragmented, 1 otherwise*/
u32 gf_isom_is_track_fragmented(GF_ISOFile *the_file, u32 TrackID);

/*a file being downloaded may be a fragmented file. In this case only partial info 
is available once the file is successfully open (gf_isom_open_progressive), and since there is 
no information wrt number fragments (which could actually be generated on the fly 
at the sender side), you must call this function on regular bases in order to
load newly downloaded fragments. Note this may result in Track/Movie duration changes
and SampleCount change too ...*/
GF_Err gf_isom_refresh_fragmented(GF_ISOFile *the_file, u64 *MissingBytes);

/*check if file has movie info, eg has tracks & dynamic media. Some files may just use
the base IsoMedia structure without "moov" container*/
Bool gf_isom_has_movie(GF_ISOFile *file);

/* check if the file has a top styp box and returns the brand and version of the first styp found */
Bool gf_isom_has_segment(GF_ISOFile *file, u32 *brand, u32 *version);

/********************************************************************
				READING API FUNCTIONS
********************************************************************/

/*return the number of tracks in the movie, or -1 if error*/
u32 gf_isom_get_track_count(GF_ISOFile *the_file);

/*return the timescale of the movie, 0 if error*/
u32 gf_isom_get_timescale(GF_ISOFile *the_file);

/*return the duration of the movie, 0 if error*/
u64 gf_isom_get_duration(GF_ISOFile *the_file);

/*return the creation info of the movie*/
GF_Err gf_isom_get_creation_time(GF_ISOFile *the_file, u64 *creationTime, u64 *modificationTime);

/*return the trackID of track number n, or 0 if error*/
u32 gf_isom_get_track_id(GF_ISOFile *the_file, u32 trackNumber);

/*return the track number of the track of specified ID, or 0 if error*/
u32 gf_isom_get_track_by_id(GF_ISOFile *the_file, u32 trackID);

/*gets the enable flag of a track 0: NO, 1: yes, 2: error*/
u8 gf_isom_is_track_enabled(GF_ISOFile *the_file, u32 trackNumber);

/* determines if the track is encrypted 0: NO, 1: yes, 2: error*/
u8 gf_isom_is_track_encrypted(GF_ISOFile *the_file, u32 trackNumber);

/*get the track duration return 0 if bad param*/
u64 gf_isom_get_track_duration(GF_ISOFile *the_file, u32 trackNumber);

/*return the media type FOUR CHAR code type of the media*/
u32 gf_isom_get_media_type(GF_ISOFile *the_file, u32 trackNumber);

/*return the media type FOUR CHAR code type of the media*/
u32 gf_isom_get_media_subtype(GF_ISOFile *the_file, u32 trackNumber, u32 DescriptionIndex);

/*return the media type FOUR CHAR code type of an MPEG4 media (eg, mp4a, mp4v, enca, encv, etc...)
returns 0 if not MPEG-4 subtype*/
u32 gf_isom_get_mpeg4_subtype(GF_ISOFile *the_file, u32 trackNumber, u32 DescriptionIndex);

/*Get the media (composition) time given the absolute time in the Movie
mediaTime is set to 0 if the media is not playing at that time (empty time segment)*/
GF_Err gf_isom_get_media_time(GF_ISOFile *the_file, u32 trackNumber, u32 movieTime, u64 *MediaTime);

/*Get the number of "streams" stored in the media - a media can have several stream descriptions...*/
u32 gf_isom_get_sample_description_count(GF_ISOFile *the_file, u32 trackNumber);

/*Get the stream description index (eg, the ESD) for a given time IN MEDIA TIMESCALE
return 0 if error or if empty*/
u32 gf_isom_get_sample_description_index(GF_ISOFile *the_file, u32 trackNumber, u64 for_time);

/*returns 1 if samples refering to the given stream description are present in the file
0 otherwise*/
Bool gf_isom_is_self_contained(GF_ISOFile *the_file, u32 trackNumber, u32 sampleDescriptionIndex);

/*get the media duration (without edit) return 0 if no samples (URL streams)*/
u64 gf_isom_get_media_duration(GF_ISOFile *the_file, u32 trackNumber);

/*Get the timeScale of the media. */
u32 gf_isom_get_media_timescale(GF_ISOFile *the_file, u32 trackNumber);

/*return the maximum chunk duration of the track in milliseconds*/
u32 gf_isom_get_max_chunk_duration(GF_ISOFile *the_file, u32 trackNumber);

/*Get the HandlerDescription name. The outName must be:
		 (outName != NULL && *outName == NULL)
the handler name is the string version of the MediaTypes*/
GF_Err gf_isom_get_handler_name(GF_ISOFile *the_file, u32 trackNumber, const char **outName);

/*Check a DataReference of this track (index >= 1)
A Data Reference allows to construct a file without integrating the media data*/
GF_Err gf_isom_check_data_reference(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex);

/*get the location of the data. If URL && URN are NULL, the data is in this file
both strings are const: don't free them.*/
GF_Err gf_isom_get_data_reference(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex, const char **outURL, const char **outURN);

/*Get the number of samples - return 0 if error*/
u32 gf_isom_get_sample_count(GF_ISOFile *the_file, u32 trackNumber);

/*Get constant sample size, or 0 if size not constant*/
u32 gf_isom_get_constant_sample_size(GF_ISOFile *the_file, u32 trackNumber);
/*returns total amount of media bytes in track*/
u64 gf_isom_get_media_data_size(GF_ISOFile *the_file, u32 trackNumber);

/*It may be desired to fetch samples with a bigger allocated buffer than their real size, in case the decoder
reads more data than available. This sets the amount of extra bytes to allocate when reading samples from this track
NOTE: the dataLength of the sample does NOT include padding*/
GF_Err gf_isom_set_sample_padding(GF_ISOFile *the_file, u32 trackNumber, u32 padding_bytes);

/*return a sample given its number, and set the StreamDescIndex of this sample
this index allows to retrieve the stream description if needed (2 media in 1 track)
return NULL if error*/
GF_ISOSample *gf_isom_get_sample(GF_ISOFile *the_file, u32 trackNumber, u32 sampleNumber, u32 *StreamDescriptionIndex);

/*same as gf_isom_get_sample but doesn't fetch media data
@StreamDescriptionIndex (optional): set to stream description index
@data_offset (optional): set to sample start offset in file.
	
	  NOTE: when both StreamDescriptionIndex and data_offset are NULL, only DTS, CTS_Offset and RAP indications are 
retrieved (faster)
*/
GF_ISOSample *gf_isom_get_sample_info(GF_ISOFile *the_file, u32 trackNumber, u32 sampleNumber, u32 *StreamDescriptionIndex, u64 *data_offset);

/*retrieves given sample DTS*/
u64 gf_isom_get_sample_dts(GF_ISOFile *the_file, u32 trackNumber, u32 sampleNumber);

/*returns sample duration in media timeScale*/
u32 gf_isom_get_sample_duration(GF_ISOFile *the_file, u32 trackNumber, u32 sampleNumber);

/*gets a sample given a desired decoding time IN MEDIA TIME SCALE
and set the StreamDescIndex of this sample
this index allows to retrieve the stream description if needed (2 media in 1 track)
return GF_EOS if the desired time exceeds the media duration
WARNING: the sample may not be sync even though the sync was requested (depends on the media and the editList)
the SampleNum is optional. If non-NULL, will contain the sampleNumber*/
GF_Err gf_isom_get_sample_for_media_time(GF_ISOFile *the_file, u32 trackNumber, u64 desiredTime, u32 *StreamDescriptionIndex, u8 SearchMode, GF_ISOSample **sample, u32 *SampleNum);

/*retrieves given sample DTS*/
u32 gf_isom_get_sample_from_dts(GF_ISOFile *the_file, u32 trackNumber, u64 dts);

/*Track Edition functions*/

/*return a sample given a desired time in the movie. MovieTime is IN MEDIA TIME SCALE , handles edit list.
and set the StreamDescIndex of this sample
this index allows to retrieve the stream description if needed (2 media in 1 track)
sample must be set to NULL before calling. 

result Sample is NULL if an error occured
if no sample is playing, an empty sample is returned with no data and a DTS set to MovieTime when serching in sync modes
if no sample is playing, the closest sample in the edit time-line is returned when serching in regular modes

WARNING: the sample may not be sync even though the sync was requested (depends on the media and the editList)

Note: this function will handle re-timestamping the sample according to the mapping  of the media time-line 
on the track time-line. The sample TSs (DTS / CTS offset) are expressed in MEDIA TIME SCALE 
(to match the media stream TS resolution as indicated in media header / SLConfig)

sampleNumber is optional and gives the number of the sample in the media
*/
GF_Err gf_isom_get_sample_for_movie_time(GF_ISOFile *the_file, u32 trackNumber, u64 movieTime, u32 *StreamDescriptionIndex, u8 SearchMode, GF_ISOSample **sample, u32 *sampleNumber);

/*get the number of edited segment*/
u32 gf_isom_get_edit_segment_count(GF_ISOFile *the_file, u32 trackNumber);

/*Get the desired segment information*/
GF_Err gf_isom_get_edit_segment(GF_ISOFile *the_file, u32 trackNumber, u32 SegmentIndex, u64 *EditTime, u64 *SegmentDuration, u64 *MediaTime, u8 *EditMode);

/*get the number of languages for the copyright*/
u32 gf_isom_get_copyright_count(GF_ISOFile *the_file);
/*get the copyright and its language code given the index*/
GF_Err gf_isom_get_copyright(GF_ISOFile *the_file, u32 Index, const char **threeCharCodes, const char **notice);
/*get the opaque watermark info if any - returns GF_NOT_SUPPORTED if not present*/
GF_Err gf_isom_get_watermark(GF_ISOFile *the_file, bin128 UUID, u8** data, u32* length);

/*get the number of chapter for movie or track if trackNumber !=0*/
u32 gf_isom_get_chapter_count(GF_ISOFile *the_file, u32 trackNumber);
/*get the given movie or track (trackNumber!=0) chapter time and name - index is 1-based
@chapter_time: retrives start time in milliseconds - may be NULL.
@name: retrieves chapter name - may be NULL - SHALL NOT be destroyed by user
*/
GF_Err gf_isom_get_chapter(GF_ISOFile *the_file, u32 trackNumber, u32 Index, u64 *chapter_time, const char **name);

/*
return 0 if the media has no sync point info (eg, all samples are RAPs)
return 1 if the media has sync points (eg some samples are RAPs)
return 2 if the media has empty sync point info (eg no samples are RAPs). This will likely only happen 
			in scalable context
*/
u8 gf_isom_has_sync_points(GF_ISOFile *the_file, u32 trackNumber);

/*returns number of sync points*/
u32 gf_isom_get_sync_point_count(GF_ISOFile *the_file, u32 trackNumber);

/*returns 1 if one sample of the track is found to have a composition time offset (DTS<CTS)*/
Bool gf_isom_has_time_offset(GF_ISOFile *the_file, u32 trackNumber);

/*returns 1 if the track has sync shadow samples*/
Bool gf_isom_has_sync_shadows(GF_ISOFile *the_file, u32 trackNumber);

/*returns 1 if the track has sample dep indications*/
Bool gf_isom_has_sample_dependency(GF_ISOFile *the_file, u32 trackNumber);

/*rough estimation of file size, only works for completely self-contained files and without fragmentation
for the current time*/
u64 gf_isom_estimate_size(GF_ISOFile *the_file);

u32 gf_isom_get_next_alternate_group_id(GF_ISOFile *movie);


/*
		MPEG-4 Systems extensions
*/

/*check if files has root OD/IOD or not*/
Bool gf_isom_has_root_od(GF_ISOFile *the_file);

/*return the root Object descriptor of the movie (can be NULL, OD or IOD, you have to check its tag)
YOU HAVE TO DELETE THE DESCRIPTOR
*/
GF_Descriptor *gf_isom_get_root_od(GF_ISOFile *the_file);

/*check the presence of a track in IOD. 0: NO, 1: YES, 2: ERROR*/
u8 gf_isom_is_track_in_root_od(GF_ISOFile *the_file, u32 trackNumber);

/*Get the GF_ESD given the StreamDescriptionIndex - YOU HAVE TO DELETE THE DESCRIPTOR*/
GF_ESD *gf_isom_get_esd(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex);

/*Get the decoderConfigDescriptor given the StreamDescriptionIndex - YOU HAVE TO DELETE THE DESCRIPTOR*/
GF_DecoderConfig *gf_isom_get_decoder_config(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex);

/*sets default TrackID (or ES_ID) for clock references. If trackNumber is 0, default sync track ID is reseted
and will be reassigned at next ESD fetch*/
void gf_isom_set_default_sync_track(GF_ISOFile *file, u32 trackNumber);

/*Return the number of track references of a track for a given ReferenceType - return -1 if error*/
s32 gf_isom_get_reference_count(GF_ISOFile *the_file, u32 trackNumber, u32 referenceType);

/*Return the referenced track number for a track and a given ReferenceType and Index
return -1 if error, 0 if the reference is a NULL one, or the trackNumber
*/
GF_Err gf_isom_get_reference(GF_ISOFile *the_file, u32 trackNumber, u32 referenceType, u32 referenceIndex, u32 *refTrack);

u8 gf_isom_get_pl_indication(GF_ISOFile *the_file, u8 PL_Code);

/*locates the first ObjectDescriptor using the given track by inspecting any OD tracks*/
u32 gf_isom_find_od_for_track(GF_ISOFile *file, u32 track);

/*returns file name*/
const char *gf_isom_get_filename(GF_ISOFile *the_file);

/*
		Update of the Reading API for IsoMedia Version 2
*/

/*retrieves the brand of the file. The brand is introduced in V2 to differenciate
MP4, MJPEG2000 and QT while indicating compatibilities
the brand is one of the above defined code, or any other registered brand

minorVersion is an optional parameter (can be set to NULL) , 
		"informative integer for the minor version of the major brand"
AlternateBrandsCount is an optional parameter (can be set to NULL) , 
	giving the number of compatible brands. 

	The function will set brand to 0 if no brand indication is found in the file
*/
GF_Err gf_isom_get_brand_info(GF_ISOFile *the_file, u32 *brand, u32 *minorVersion, u32 *AlternateBrandsCount);

/*gets an alternate brand indication. BrandIndex is 1-based
Note that the Major brand should always be indicated in the alternate brands*/
GF_Err gf_isom_get_alternate_brand(GF_ISOFile *the_file, u32 BrandIndex, u32 *brand);

/*get the number of padding bits at the end of a given sample if any*/
GF_Err gf_isom_get_sample_padding_bits(GF_ISOFile *the_file, u32 trackNumber, u32 sampleNumber, u8 *NbBits);
/*indicates whether the track samples use padding bits or not*/
Bool gf_isom_has_padding_bits(GF_ISOFile *the_file, u32 trackNumber);

/*returns width and height of the given visual sample desc - error if not a visual track*/
GF_Err gf_isom_get_visual_info(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex, u32 *Width, u32 *Height);

/*returns samplerate, channels and bps of the given audio track - error if not a audio track*/
GF_Err gf_isom_get_audio_info(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex, u32 *SampleRate, u32 *Channels, u8 *bitsPerSample);

/*returns track visual info - all coord values are expressed as 16.16 fixed point floats*/
GF_Err gf_isom_get_track_layout_info(GF_ISOFile *the_file, u32 trackNumber, u32 *width, u32 *height, s32 *translation_x, s32 *translation_y, s16 *layer);

/*returns track matrix info - all coord values are expressed as 16.16 fixed point floats*/
GF_Err gf_isom_get_track_matrix(GF_ISOFile *the_file, u32 trackNumber, u32 matrix[9]);

/*returns width and height of the given visual sample desc - error if not a visual track*/
GF_Err gf_isom_get_pixel_aspect_ratio(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex, u32 *hSpacing, u32 *vSpacing);

/*
	User Data Manipulation (cf write API too)
*/

/* Gets the number of UserDataItems with the same ID / UUID in the desired track or 
in the movie if trackNumber is set to 0*/
u32 gf_isom_get_user_data_count(GF_ISOFile *the_file, u32 trackNumber, u32 UserDataType, bin128 UUID);
/* Gets the UserData for the specified item from the track or the movie if trackNumber is set to 0
data is allocated by the function and is yours to free
you musty pass (userData != NULL && *userData=NULL)*/
GF_Err gf_isom_get_user_data(GF_ISOFile *the_file, u32 trackNumber, u32 UserDataType, bin128 UUID, u32 UserDataIndex, char **userData, u32 *userDataSize);


/*gets 3char media language code - @three_char_code must be at least 4 char long*/
GF_Err gf_isom_get_media_language(GF_ISOFile *the_file, u32 trackNumber, char *three_char_code);

/*Unknown sample description*/
typedef struct
{
	/*codec tag is the containing box's tag, 0 if UUID is used*/
	u32 codec_tag;
	/*entry UUID if no tag is used*/
	bin128 UUID;

	u16 version;
	u16 revision;
	u32 vendor_code;

	/*video codecs only*/
	u32 temporal_quality;
	u32 spacial_quality;
	u16 width, height;
	u32 h_res, v_res;
	u16 depth;
	u16 color_table_index;
	char compressor_name[33];

	/*audio codecs only*/
	u32 samplerate;
	u16 nb_channels;
	u16 bits_per_sample;

	/*if present*/
	char *extension_buf;
	u32 extension_buf_size;
} GF_GenericSampleDescription;

/*returns wrapper for unknown entries - you must delete it yourself*/
GF_GenericSampleDescription *gf_isom_get_generic_sample_description(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex);

/*retrieves default values for a track fragment. Each variable is optional and 
if set will contain the default value for this track samples*/
GF_Err gf_isom_get_fragment_defaults(GF_ISOFile *the_file, u32 trackNumber, 
							 u32 *defaultDuration, u32 *defaultSize, u32 *defaultDescriptionIndex,
							 u32 *defaultRandomAccess, u8 *defaultPadding, u16 *defaultDegradationPriority);


/*non standard extensions used for video packets in order to keep AU structure in the file format 
(no normative tables for that). Info is NOT written to disk.
*/
/*get number of fragments for a sample */
u32 gf_isom_get_sample_fragment_count(GF_ISOFile *the_file, u32 trackNumber, u32 sampleNumber);
/*get sample fragment size*/
u16 gf_isom_get_sample_fragment_size(GF_ISOFile *the_file, u32 trackNumber, u32 sampleNumber, u32 FragmentIndex);

/*returns 1 if file is single AV (max one audio, one video, one text and basic od/bifs)*/
Bool gf_isom_is_single_av(GF_ISOFile *file);

/*guess which std this file refers to. return value:
	GF_ISOM_BRAND_ISOM: unrecognized std
	GF_ISOM_BRAND_3GP5: 3GP file (max 1 audio, 1 video) without text track
	GF_ISOM_BRAND_3GP6: 3GP file (max 1 audio, 1 video) with text track
	GF_ISOM_BRAND_3GG6: 3GP file multitrack file
	GF_ISOM_BRAND_3G2A: 3GP2 file
	GF_ISOM_BRAND_AVC1: AVC file
	FCC("ISMA"): ISMA file (may overlap with 3GP)
	GF_ISOM_BRAND_MP42: any generic MP4 file (eg with BIFS/OD/MPEG-4 systems stuff)

  for files without movie, returns the file meta handler type
*/
u32 gf_isom_guess_specification(GF_ISOFile *file);


#ifndef GPAC_DISABLE_ISOM_WRITE


/********************************************************************
				EDITING/WRITING API FUNCTIONS
********************************************************************/

/*set the timescale of the movie*/
GF_Err gf_isom_set_timescale(GF_ISOFile *the_file, u32 timeScale);

/*creates a new Track. If trackID = 0, the trackID is chosen by the API
returns the track number or 0 if error*/
u32 gf_isom_new_track(GF_ISOFile *the_file, u32 trackID, u32 MediaType, u32 TimeScale);

/*removes the desired track - internal cross dependancies will be updated.
WARNING: any OD streams with references to this track through  ODUpdate, ESDUpdate, ESDRemove commands
will be rewritten*/
GF_Err gf_isom_remove_track(GF_ISOFile *the_file, u32 trackNumber);

/*sets the enable flag of a track*/
GF_Err gf_isom_set_track_enabled(GF_ISOFile *the_file, u32 trackNumber, u8 enableTrack);

/*sets creationTime and modificationTime of the movie to the specified date*/
GF_Err gf_isom_set_creation_time(GF_ISOFile *movie, u64 time);
/*sets creationTime and modificationTime of the track to the specified date*/
GF_Err gf_isom_set_track_creation_time(GF_ISOFile *movie,u32 trackNumber, u64 time);

/*changes the trackID - all track references present in the file are updated
returns error if trackID is already in used in the file*/
GF_Err gf_isom_set_track_id(GF_ISOFile *the_file, u32 trackNumber, u32 trackID);

/*Add samples to a track. Use streamDescriptionIndex to specify the desired stream (if several)*/
GF_Err gf_isom_add_sample(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex, GF_ISOSample *sample);

/*Add sync shadow sample to a track. 
- There must be a regular sample with the same DTS. 
- Sync Shadow samples MUST be RAP
- Currently, adding sync shadow must be done in order (no sample insertion)
*/
GF_Err gf_isom_add_sample_shadow(GF_ISOFile *the_file, u32 trackNumber, GF_ISOSample *sample);

/*add data to current sample in the track. Use this function for media with
fragmented options such as MPEG-4 video packets. This will update the data size.
CANNOT be used with OD media type*/
GF_Err gf_isom_append_sample_data(GF_ISOFile *the_file, u32 trackNumber, char *data, u32 data_size);

/*sets RAP flag of last sample added to TRUE*/
GF_Err gf_isom_set_sample_rap(GF_ISOFile *movie, u32 trackNumber);

/*Add sample references to a track. The dataOffset is the offset of the data in the referenced file
you MUST have created a StreamDescription with URL or URN specifying your referenced file
Use streamDescriptionIndex to specify the desired stream (if several)*/
GF_Err gf_isom_add_sample_reference(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex, GF_ISOSample *sample, u64 dataOffset);

/*set the duration of the last media sample. If not set, the duration of the last sample is the
duration of the previous one if any, or media TimeScale (default value).*/
GF_Err gf_isom_set_last_sample_duration(GF_ISOFile *the_file, u32 trackNumber, u32 duration);

/*sets a track reference*/
GF_Err gf_isom_set_track_reference(GF_ISOFile *the_file, u32 trackNumber, u32 referenceType, u32 ReferencedTrackID);

/*removes a track reference*/
GF_Err gf_isom_remove_track_reference(GF_ISOFile *the_file, u32 trackNumber, u32 referenceType, u32 ReferenceIndex);

/*sets track handler name. name is either NULL (reset), a UTF-8 formatted string or a UTF8 file 
resource in the form "file://path/to/file_utf8" */
GF_Err gf_isom_set_handler_name(GF_ISOFile *the_file, u32 trackNumber, const char *nameUTF8);

/*Update the sample size table - this is needed when using @gf_isom_append_sample_data in case the resulting samples
are of same sizes (typically in 3GP speech tracks)*/
GF_Err gf_isom_refresh_size_info(GF_ISOFile *file, u32 trackNumber);

/*Update Sample functions*/

/*update a given sample of the media.
@data_only: if set, only the sample data is updated, not other info*/
GF_Err gf_isom_update_sample(GF_ISOFile *the_file, u32 trackNumber, u32 sampleNumber, GF_ISOSample *sample, Bool data_only);

/*update a sample reference in the media. Note that the sample MUST exists,
that sample->data MUST be NULL and sample->dataLength must be NON NULL;*/
GF_Err gf_isom_update_sample_reference(GF_ISOFile *the_file, u32 trackNumber, u32 sampleNumber, GF_ISOSample *sample, u64 data_offset);

/*Remove a given sample*/
GF_Err gf_isom_remove_sample(GF_ISOFile *the_file, u32 trackNumber, u32 sampleNumber);

/*changes media time scale*/
GF_Err gf_isom_set_media_timescale(GF_ISOFile *the_file, u32 trackNumber, u32 new_timescale);

/*set the save file name of the (edited) movie. 
If the movie is edited, the default fileName is avp_#openName)
NOTE: you cannot save an edited file under the same name (overwrite not allowed)
If the movie is created (WRITE mode), the default filename is #openName*/
GF_Err gf_isom_set_final_name(GF_ISOFile *the_file, char *filename);


/*set the storage mode of a file (FLAT, STREAMABLE, INTERLEAVED)*/
GF_Err gf_isom_set_storage_mode(GF_ISOFile *the_file, u8 storageMode);
u8 gf_isom_get_storage_mode(GF_ISOFile *the_file);

/*set the interleaving time of media data (INTERLEAVED mode only)
InterleaveTime is in MovieTimeScale*/
GF_Err gf_isom_set_interleave_time(GF_ISOFile *the_file, u32 InterleaveTime);
u32 gf_isom_get_interleave_time(GF_ISOFile *the_file);

/*set the copyright in one language.*/
GF_Err gf_isom_set_copyright(GF_ISOFile *the_file, const char *threeCharCode, char *notice);

/*deletes copyright (1-based indexes)*/
GF_Err gf_isom_remove_copyright(GF_ISOFile *the_file, u32 index);

/*changes the handler type of the media*/
GF_Err gf_isom_set_media_type(GF_ISOFile *movie, u32 trackNumber, u32 new_type);

/*changes the type of the sampleDescriptionBox - USE AT YOUR OWN RISK, the file may not be understood afterwards*/
GF_Err gf_isom_set_media_subtype(GF_ISOFile *movie, u32 trackNumber, u32 sampleDescriptionIndex, u32 new_type);

GF_Err gf_isom_set_alternate_group_id(GF_ISOFile *movie, u32 trackNumber, u32 groupId);

/*add chapter info:
if trackNumber is 0, the chapter info is added to the movie, otherwise to the track
@timestamp: chapter start time in milliseconds. Chapters are added in order to the file. If a chapter with same timestamp
	is found, its name is updated but no entry is created.
@name: chapter name. If NULL, defaults to 'Chapter N'
*/
GF_Err gf_isom_add_chapter(GF_ISOFile *the_file, u32 trackNumber, u64 timestamp, char *name);

/*deletes copyright (1-based index, index 0 for all)*/
GF_Err gf_isom_remove_chapter(GF_ISOFile *the_file, u32 trackNumber, u32 index);

/*set watermark info for movie*/
GF_Err gf_isom_set_watermark(GF_ISOFile *the_file, bin128 UUID, u8* data, u32 length);

/*Track Edition functions - used to change the normal playback of the media if desired
NOTE: IT IS THE USER RESPONSABILITY TO CREATE A CONSISTENT TIMELINE FOR THE TRACK
This API provides the basic hooks and some basic consistency checking
but can not check the desired functionality of the track edits
*/

/*update or insert a new edit segment in the track time line. Edits are used to modify
the media normal timing. EditTime and EditDuration are expressed in Movie TimeScale
If a segment with EditTime already exists, IT IS ERASED
if there is a segment before this new one, its duration is adjust to match EditTime of
the new segment
WARNING: The first segment always have an EditTime of 0. You should insert an empty or dwelled segment first.*/
GF_Err gf_isom_set_edit_segment(GF_ISOFile *the_file, u32 trackNumber, u64 EditTime, u64 EditDuration, u64 MediaTime, u8 EditMode);

/*same as above except only modifies duartion type and mediaType*/
GF_Err gf_isom_modify_edit_segment(GF_ISOFile *the_file, u32 trackNumber, u32 seg_index, u64 EditDuration, u64 MediaTime, u8 EditMode);
GF_Err gf_isom_modify_edit_segment2(GF_ISOFile *movie, u32 trackNumber, u32 seg_index, u64 EditDuration, u64 MediaTime, u32 MediaRate, u8 EditMode);
/*same as above except only appends new segment*/
GF_Err gf_isom_append_edit_segment(GF_ISOFile *the_file, u32 trackNumber, u64 EditDuration, u64 MediaTime, u8 EditMode);
GF_Err gf_isom_append_edit_segment2(GF_ISOFile *movie, u32 trackNumber, u64 EditDuration, u64 MediaTime, u32 MediaRate, u8 EditMode);

/*remove the edit segments for the whole track*/
GF_Err gf_isom_remove_edit_segments(GF_ISOFile *the_file, u32 trackNumber);

/*remove the given edit segment (1-based index). If this is not the last segment, the next segment duration
is updated to maintain a continous timeline*/
GF_Err gf_isom_remove_edit_segment(GF_ISOFile *the_file, u32 trackNumber, u32 seg_index);

/*
				User Data Manipulation

		You can add specific typed data to either a track or the movie: the UserData
	The type must be formated as a FourCC if you have a registered 4CC type
	but the usual is to set a UUID (128 bit ID for box type) which never conflict
	with existing structures in the format
		To manipulate a UUID user data set the UserDataType to 0 and specify a valid UUID.
Otherwise the UUID parameter is ignored
		Several items with the same ID or UUID can be added (this allows you to store any
	kind/number of private information under a unique ID / UUID)
*/
/*Add a user data item in the desired track or in the movie if TrackNumber is 0*/
GF_Err gf_isom_add_user_data(GF_ISOFile *the_file, u32 trackNumber, u32 UserDataType, bin128 UUID, char *data, u32 DataLength);

/*remove all user data items from the desired track or from the movie if TrackNumber is 0*/
GF_Err gf_isom_remove_user_data(GF_ISOFile *the_file, u32 trackNumber, u32 UserDataType, bin128 UUID);

/*remove a user data item from the desired track or from the movie if TrackNumber is 0
use the UDAT read functions to get the item index*/
GF_Err gf_isom_remove_user_data_item(GF_ISOFile *the_file, u32 trackNumber, u32 UserDataType, bin128 UUID, u32 UserDataIndex);

/*remove track, moov (trackNumber=0) or file-level (trackNumber=0xFFFFFFFF) UUID box of matching type*/
GF_Err gf_isom_remove_uuid(GF_ISOFile *movie, u32 trackNumber, bin128 UUID);
/*adds track, moov (trackNumber=0) or file-level (trackNumber=0xFFFFFFFF) UUID box of given type*/
GF_Err gf_isom_add_uuid(GF_ISOFile *movie, u32 trackNumber, bin128 UUID, char *data, u32 data_size);

/*
		Update of the Writing API for IsoMedia Version 2
*/	

/*use a compact track version for sample size. This is not usually recommended 
except for speech codecs where the track has a lot of small samples
compaction is done automatically while writing based on the track's sample sizes*/
GF_Err gf_isom_use_compact_size(GF_ISOFile *the_file, u32 trackNumber, u8 CompactionOn);

/*sets the brand of the movie*/
GF_Err gf_isom_set_brand_info(GF_ISOFile *the_file, u32 MajorBrand, u32 MinorVersion);

/*adds or remove an alternate brand for the movie*/
GF_Err gf_isom_modify_alternate_brand(GF_ISOFile *the_file, u32 Brand, u8 AddIt);

/*removes all alternate brands except major brand*/
GF_Err gf_isom_reset_alt_brands(GF_ISOFile *movie);

/*set the number of padding bits at the end of a given sample if needed
if the function is never called the padding bit info is ignored
this MUST be called on an existin sample*/
GF_Err gf_isom_set_sample_padding_bits(GF_ISOFile *the_file, u32 trackNumber, u32 sampleNumber, u8 NbBits);


/*since v2 you must specify w/h of video tracks for authoring tools (no decode the video cfg / first sample)*/
GF_Err gf_isom_set_visual_info(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex, u32 Width, u32 Height);

/*mainly used for 3GPP text since most ISO-based formats ignore these (except MJ2K) 
all coord values are expressed as 16.16 fixed point floats*/
GF_Err gf_isom_set_track_layout_info(GF_ISOFile *the_file, u32 trackNumber, u32 width, u32 height, s32 translation_x, s32 translation_y, s16 layer);

/*sets track matrix - all coordinates are expressed as 16.16 floating points*/
GF_Err gf_isom_set_track_matrix(GF_ISOFile *the_file, u32 trackNumber, u32 matrix[9]);

GF_Err gf_isom_set_pixel_aspect_ratio(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex, u32 hSpacing, u32 vSpacing);

/*set SR & nbChans for audio description*/
GF_Err gf_isom_set_audio_info(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex, u32 sampleRate, u32 nbChannels, u8 bitsPerSample);

/*non standard extensions: set/remove a fragment of a sample - this is used for video packets
in order to keep AU structure in the file format (no normative tables for that). Info is NOT written to disk*/
GF_Err gf_isom_add_sample_fragment(GF_ISOFile *the_file, u32 trackNumber, u32 sampleNumber, u16 FragmentSize);
GF_Err gf_isom_remove_sample_fragment(GF_ISOFile *the_file, u32 trackNumber, u32 sampleNumber);
/*remove all sample fragment info for this track*/
GF_Err gf_isom_remove_sample_fragments(GF_ISOFile *the_file, u32 trackNumber);

/*set CTS unpack mode (used for B-frames & like): in unpack mode, each sample uses one entry in CTTS tables
unpack=0: set unpack on - !!creates a CTTS table if none found!!
unpack=1: set unpack off and repacks all table info
*/
GF_Err gf_isom_set_cts_packing(GF_ISOFile *the_file, u32 trackNumber, Bool unpack);
/*modify CTS offset of a given sample (used for B-frames) - MUST be called in unpack mode only*/
GF_Err gf_isom_modify_cts_offset(GF_ISOFile *the_file, u32 trackNumber, u32 sample_number, u32 offset);
/*remove CTS offset table (used for B-frames)*/
GF_Err gf_isom_remove_cts_info(GF_ISOFile *the_file, u32 trackNumber);

/*set 3char code media language*/
GF_Err gf_isom_set_media_language(GF_ISOFile *the_file, u32 trackNumber, char *three_char_code);

/*removes given stream description*/
GF_Err gf_isom_remove_sample_description(GF_ISOFile *the_file, u32 trackNumber, u32 streamDescIndex);

/*
	some authoring extensions
*/
/*sets name for authoring - if name is NULL reset authoring name*/
GF_Err gf_isom_set_track_name(GF_ISOFile *the_file, u32 trackNumber, char *name);
/*gets authoring name*/
const char *gf_isom_get_track_name(GF_ISOFile *the_file, u32 trackNumber);

/*
			MPEG-4 Extensions
*/

/*set a profile and level indication for the movie iod (created if needed)
if the flag is ProfileLevel is 0 this means the movie doesn't require
the specific codec (equivalent to 0xFF value in MPEG profiles)*/
GF_Err gf_isom_set_pl_indication(GF_ISOFile *the_file, u8 PL_Code, u8 ProfileLevel);

/*set the rootOD ID of the movie if you need it. By default, movies are created without root ODs*/
GF_Err gf_isom_set_root_od_id(GF_ISOFile *the_file, u32 OD_ID);

/*set the rootOD URL of the movie if you need it (only needed to create empty file pointing 
to external ressource)*/
GF_Err gf_isom_set_root_od_url(GF_ISOFile *the_file, char *url_string);

/*remove the root OD*/
GF_Err gf_isom_remove_root_od(GF_ISOFile *the_file);

/*Add a system descriptor to the OD of the movie*/
GF_Err gf_isom_add_desc_to_root_od(GF_ISOFile *the_file, GF_Descriptor *theDesc);

/*add a track to the root OD*/
GF_Err gf_isom_add_track_to_root_od(GF_ISOFile *the_file, u32 trackNumber);

/*remove a track to the root OD*/
GF_Err gf_isom_remove_track_from_root_od(GF_ISOFile *the_file, u32 trackNumber);

/*Create a new StreamDescription (GF_ESD) in the file. The URL and URN are used to 
describe external media, this will creat a data reference for the media*/
GF_Err gf_isom_new_mpeg4_description(GF_ISOFile *the_file, u32 trackNumber, GF_ESD *esd, char *URLname, char *URNname, u32 *outDescriptionIndex);

/*use carefully. Very usefull when you made a lot of changes (IPMP, IPI, OCI, ...)
THIS WILL REPLACE THE WHOLE DESCRIPTOR ...*/
GF_Err gf_isom_change_mpeg4_description(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex, GF_ESD *newESD);

/*Add a system descriptor to the ESD of a stream - you have to delete the descriptor*/
GF_Err gf_isom_add_desc_to_description(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex, GF_Descriptor *theDesc);


/*Default extensions*/

/*Create a new unknown StreamDescription in the file. The URL and URN are used to 
describe external media, this will creat a data reference for the media
use this to store media not currently supported by the ISO media format
*/
GF_Err gf_isom_new_generic_sample_description(GF_ISOFile *the_file, u32 trackNumber, char *URLname, char *URNname, GF_GenericSampleDescription *udesc, u32 *outDescriptionIndex);

/*change the data field of an unknown sample description*/
GF_Err gf_isom_change_generic_sample_description(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex, GF_GenericSampleDescription *udesc);

/*
special shortcut for stream description cloning from a given input file (this avoids inspecting for media type)
@the_file, @trackNumber: destination file and track
@orig_file, @orig_track, @orig_desc_index: orginal file, track and sample description
@URLname, @URNname, @outDescriptionIndex: same usage as with gf_isom_new_mpeg4_description
*/
GF_Err gf_isom_clone_sample_description(GF_ISOFile *the_file, u32 trackNumber, GF_ISOFile *orig_file, u32 orig_track, u32 orig_desc_index, char *URLname, char *URNname, u32 *outDescriptionIndex);

/*special shortcut: clones a track (everything except media data and sample info (DTS? CTS, RAPs, etc...) 
also clones sampleDescriptions
@keep_data_ref: if set, external data references are kept, otherwise they are removed (track media data will be self-contained)
@dest_track: track number of cloned track*/
GF_Err gf_isom_clone_track(GF_ISOFile *orig_file, u32 orig_track, GF_ISOFile *dest_file, Bool keep_data_ref, u32 *dest_track);
/*special shortcut: clones IOD PLs from orig to dest if any*/
GF_Err gf_isom_clone_pl_indications(GF_ISOFile *orig, GF_ISOFile *dest);
/*clones root OD from input to output file, without copying root OD track references*/
GF_Err gf_isom_clone_root_od(GF_ISOFile *input, GF_ISOFile *output);

/*clones the entire movie file to destination. Tracks can be cloned if clone_tracks is set, in which case hint tracks can be
kept if keep_hint_tracks is set*/
GF_Err gf_isom_clone_movie(GF_ISOFile *orig_file, GF_ISOFile *dest_file, Bool clone_tracks, Bool keep_hint_tracks);

/*returns true if same set of sample description in both tracks - this does include self-contained checking
and reserved flags. The specific media cfg (DSI & co) is not analysed, only
a brutal memory comparaison is done*/
Bool gf_isom_is_same_sample_description(GF_ISOFile *f1, u32 tk1, GF_ISOFile *f2, u32 tk2);

GF_Err gf_isom_set_JPEG2000(GF_ISOFile *mov, Bool set_on);

/*releases current movie segment - this closes the associated file IO object.
If reset_tables is set, sample information for all tracks setup as segment are destroyed. This allows keeping the memory
footprint low when playing segments. Note however that seeking in the file is then no longer possible*/
GF_Err gf_isom_release_segment(GF_ISOFile *movie, Bool reset_tables);
/*opens a new segment file. Access to samples in previous segments is no longer possible*/
GF_Err gf_isom_open_segment(GF_ISOFile *movie, const char *fileName);

#ifndef GPAC_DISBALE_ISOM_FRAGMENTS

/*
			Movie Fragments Writing API
		Movie Fragments is a feature of ISO media files for fragmentation
	of a presentation meta-data and interleaving with its media data.
	This enables faster http fast start for big movies, and also reduces the risk
	of data loss in case of a recording crash, because meta data and media data
	can be written to disk at regular times
		This API provides simple function calls to setup such a movie and write it
	The process implies:
		1- creating a movie in the usual way (track, stream descriptions, (IOD setup
	copyright, ...)
		2- possibly add some samples in the regular fashion
		3- setup track fragments for all track that will be written in a fragmented way
	(note that you can create/write a track that has no fragmentation at all)
		4- finalize the movie for fragmentation (this will flush all meta-data and 
	any media-data added to disk, ensuring all vital information for the presentation
	is stored on file and not lost in case of crash/poweroff)
	
	  then 5-6 as often as desired
		5- start a new movie fragment
		6- add samples to each setup track


  IMPORTANT NOTES:
		* Movie Fragments can only be used in GF_ISOM_OPEN_WRITE mode (capturing)
  and no editing functionalities can be used
		* the fragmented movie API uses TrackID and not TrackNumber 
*/

/*
setup a track for fragmentation by specifying some default values for 
storage efficiency
*TrackID: track identifier
*DefaultStreamDescriptionIndex: the default description used by samples in this track
*DefaultSampleDuration: default duration of samples in this track
*DefaultSampleSize: default size of samples in this track (0 if unknown)
*DefaultSampleIsSync: default key-flag (RAP) of samples in this track
*DefaultSamplePadding: default padding bits for samples in this track
*DefaultDegradationPriority: default degradation priority for samples in this track

*/
GF_Err gf_isom_setup_track_fragment(GF_ISOFile *the_file, u32 TrackID, 
							 u32 DefaultStreamDescriptionIndex,
							 u32 DefaultSampleDuration,
							 u32 DefaultSampleSize,
							 u8 DefaultSampleIsSync,
							 u8 DefaultSamplePadding,
							 u16 DefaultDegradationPriority);

/*flushes data to disk and prepare movie fragmentation*/
GF_Err gf_isom_finalize_for_fragment(GF_ISOFile *the_file, Bool use_segments);

/*starts a new movie fragment - if force_cache is set, fragment metadata will be written before
fragment media data for all tracks*/
GF_Err gf_isom_start_fragment(GF_ISOFile *movie, Bool moof_first);

/*starts a new segment in the file. If SegName is given, the output will be written in the SegName file*/
GF_Err gf_isom_start_segment(GF_ISOFile *movie, char *SegName);

/*sets the baseMediaDecodeTime of the first sample of the given track*/
GF_Err gf_isom_set_traf_base_media_decode_time(GF_ISOFile *movie, u32 TrackID, u64 decode_time);

/*closes current segment*/
GF_Err gf_isom_close_segment(GF_ISOFile *movie, u32 fragments_per_sidx, u32 referenceTrackID, u64 ref_track_decode_time, Bool daisy_chain_sidx, Bool last_segment);

enum
{
	/*indicates that the track fragment has no samples but still has a duration
	(silence-detection in audio codecs, ...). 
	param: indicates duration*/
	GF_ISOM_TRAF_EMPTY,
	/*I-Frame detection: this can reduce file size by detecting I-frames and
	optimizing sample flags (padding, priority, ..)
	param: on/off (0/1)*/
	GF_ISOM_TRAF_RANDOM_ACCESS,
	/*activate data cache on track fragment. This is usefull when writing interleaved
	media from a live source (typically audio-video), and greatly reduces file size
	param: Number of samples (> 1) to cache before disk flushing. You shouldn't try 
	to cache too many samples since this will load your memory. base that on FPS/SR*/
	GF_ISOM_TRAF_DATA_CACHE
};

/*set options. Options can be set at the begining of each new fragment only, and for the
lifetime of the fragment*/
GF_Err gf_isom_set_fragment_option(GF_ISOFile *the_file, u32 TrackID, u32 Code, u32 param);


/*adds a sample to a fragmented track

*TrackID: destination track
*sample: sample to add
*StreamDescriptionIndex: stream description for this sample. If 0, the default one 
is used
*Duration: sample duration.
Note: because of the interleaved nature of the meta/media data, the sample duration
MUST be provided (in case of regular tracks, this was computed internally by the lib)
*PaddingBits: padding bits for the sample, or 0
*DegradationPriority for the sample, or 0

*/

GF_Err gf_isom_fragment_add_sample(GF_ISOFile *the_file, u32 TrackID, GF_ISOSample *sample, 
								 u32 StreamDescriptionIndex, 
								 u32 Duration,
								 u8 PaddingBits, u16 DegradationPriority);

/*appends data into last sample of track for video fragments/other media
CANNOT be used with OD tracks*/
GF_Err gf_isom_fragment_append_data(GF_ISOFile *the_file, u32 TrackID, char *data, u32 data_size, u8 PaddingBits);

#endif /*GPAC_DISBALE_ISOM_FRAGMENTS*/


/******************************************************************
		GENERIC Publishing API
******************************************************************/

/*Removes all sync shadow entries for a given track. The shadow samples are NOT removed; they must be removed
by the user app*/
GF_Err gf_isom_remove_sync_shadows(GF_ISOFile *the_file, u32 trackNumber);

/*Use this function to do the shadowing if you use shadowing.
the sample to be shadowed MUST be a non-sync sample (ignored if not)
the sample shadowing must be a Sync sample (error if not)*/
GF_Err gf_isom_set_sync_shadow(GF_ISOFile *the_file, u32 trackNumber, u32 sampleNumber, u32 syncSample);

/*set the GroupID of a track (only used for optimized interleaving). By setting GroupIDs
you can specify the storage order for media data of a group of streams. This is usefull
for BIFS presentation so that static resources of the scene can be downloaded before BIFS*/
GF_Err gf_isom_set_track_group(GF_ISOFile *the_file, u32 trackNumber, u32 GroupID);

/*set the priority of a track within a Group (used for optimized interleaving and hinting). 
This allows tracks to be stored before other within a same group, for instance the 
hint track data can be stored just before the media data, reducing disk seeking
for a same time, within a group of tracks, the track with the lowest inversePriority will 
be written first*/
GF_Err gf_isom_set_track_priority_in_group(GF_ISOFile *the_file, u32 trackNumber, u32 InversePriority);

/*set the max SamplesPerChunk (for file optimization, mainly in FLAT and STREAMABLE modes)*/
GF_Err gf_isom_set_max_samples_per_chunk(GF_ISOFile *the_file, u32 trackNumber, u32 maxSamplesPerChunk);

/*associate a given SL config with a given ESD while extracting the OD information
all the SL params must be fixed by the calling app!
The SLConfig is stored by the API for further use. A NULL pointer will result
in using the default SLConfig (predefined = 2) remapped to predefined = 0
This is usefull while reading the IOD / OD stream of an MP4 file. Note however that
only full AUs are extracted, therefore the calling application must SL-packetize the streams*/
GF_Err gf_isom_set_extraction_slc(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex, GF_SLConfig *slConfig);

GF_Err gf_isom_get_extraction_slc(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex, GF_SLConfig **slConfig);

u32 gf_isom_get_track_group(GF_ISOFile *the_file, u32 trackNumber);
u32 gf_isom_get_track_priority_in_group(GF_ISOFile *the_file, u32 trackNumber);

/*stores movie config (storage mode, interleave time, track groupIDs, priorities and names) in UDTA(kept on disk)
if @remove_all is set, removes all stored info, otherwise recompute all stored info*/
GF_Err gf_isom_store_movie_config(GF_ISOFile *the_file, Bool remove_all);
/*restores movie config (storage mode, interleave time, track groupIDs, priorities and names) if found*/
GF_Err gf_isom_load_movie_config(GF_ISOFile *the_file);

/*setup interleaving for storage (shortcut for storeage mode + interleave_time)*/
GF_Err gf_isom_make_interleave(GF_ISOFile *mp4file, Double TimeInSec);


/******************************************************************
		GENERIC HINTING WRITING API
******************************************************************/

/*supported hint formats - ONLY RTP now*/
enum
{
	GF_ISOM_HINT_RTP = GF_4CC('r', 't', 'p', ' '),
};

#ifndef GPAC_DISABLE_ISOM_HINTING


/*Setup the resources based on the hint format
This function MUST be called after creating a new hint track and before
any other calls on this track*/
GF_Err gf_isom_setup_hint_track(GF_ISOFile *the_file, u32 trackNumber, u32 HintType);

/*Create a HintDescription for the HintTrack
the rely flag indicates whether a reliable transport protocol is desired/required
for data transport
	0: not desired (UDP/IP). NB: most RTP streaming servers only support UDP/IP for data
	1: preferable (TCP/IP if possible or UDP/IP)
	2: required (TCP/IP only)
The HintDescriptionIndex is set, to be used when creating a HINT sample
*/
GF_Err gf_isom_new_hint_description(GF_ISOFile *the_file, u32 trackNumber, s32 HintTrackVersion, s32 LastCompatibleVersion, u8 Rely, u32 *HintDescriptionIndex);

/*Starts a new sample for the hint track. A sample is just a collection of packets
the transmissionTime is indicated in the media timeScale of the hint track*/
GF_Err gf_isom_begin_hint_sample(GF_ISOFile *the_file, u32 trackNumber, u32 HintDescriptionIndex, u32 TransmissionTime);

/*stores the hint sample in the file once all your packets for this sample are done
set IsRandomAccessPoint if you want to indicate that this is a random access point 
in the stream*/
GF_Err gf_isom_end_hint_sample(GF_ISOFile *the_file, u32 trackNumber, u8 IsRandomAccessPoint);


/******************************************************************
		PacketHandling functions
		Data can be added at the end or at the beginning of the current packet
		by setting AtBegin to 1 the data will be added at the begining
		This allows constructing the packet payload before any meta-data
******************************************************************/

/*adds a blank chunk of data in the sample that is skipped while streaming*/
GF_Err gf_isom_hint_blank_data(GF_ISOFile *the_file, u32 trackNumber, u8 AtBegin);

/*adds a chunk of data in the packet that is directly copied while streaming
NOTE: dataLength MUST BE <= 14 bytes, and you should only use this function
to add small blocks of data (encrypted parts, specific headers, ...)*/
GF_Err gf_isom_hint_direct_data(GF_ISOFile *the_file, u32 trackNumber, char *data, u32 dataLength, u8 AtBegin);

/*adds a reference to some sample data in the packet
SourceTrackID: the ID of the track where the referenced sample is
SampleNumber: the sample number containing the data to be added
DataLength: the length of bytes to copy in the packet
offsetInSample: the offset in bytes in the sample at which to begin copying data

extra_data: only used when the sample is actually the sample that will contain this packet
(usefull to store en encrypted version of a packet only available while streaming)
	In this case, set SourceTrackID to the HintTrack ID and SampleNumber to 0
	In this case, the DataOffset MUST BE NULL and length will indicate the extra_data size

Note that if you want to reference a previous HintSample in the hintTrack, you will 
have to parse the sample yourself ...
*/
GF_Err gf_isom_hint_sample_data(GF_ISOFile *the_file, u32 trackNumber, u32 SourceTrackID, u32 SampleNumber, u16 DataLength, u32 offsetInSample, char *extra_data, u8 AtBegin);


/*adds a reference to some stream description data in the packet (headers, ...)
SourceTrackID: the ID of the track where the referenced sample is
StreamDescriptionIndex: the index of the stream description in the desired track
DataLength: the length of bytes to copy in the packet
offsetInDescription: the offset in bytes in the description at which to begin copying data

Since it is far from being obvious what this offset is, we recommend not using this 
function. The ISO Media Format specification is currently being updated to solve
this issue*/
GF_Err gf_isom_hint_sample_description_data(GF_ISOFile *the_file, u32 trackNumber, u32 SourceTrackID, u32 StreamDescriptionIndex, u16 DataLength, u32 offsetInDescription, u8 AtBegin);


/******************************************************************
		RTP SPECIFIC WRITING API
******************************************************************/

/*Creates a new RTP packet in the HintSample. If a previous packet was created, 
it is stored in the hint sample and a new packet is created.
- relativeTime: RTP time offset of this packet in the HintSample if any - in hint track 
time scale. Used for data smoothing by servers.
- PackingBit: the 'P' bit of the RTP packet header
- eXtensionBit: the'X' bit of the RTP packet header
- MarkerBit: the 'M' bit of the RTP packet header
- PayloadType: the payload type, on 7 bits, format 0x0XXXXXXX
- B_frame: indicates if this is a B-frame packet. Can be skipped by a server
- IsRepeatedPacket: indicates if this is a duplicate packet of a previous one.
Can be skipped by a server
- SequenceNumber: the RTP base sequence number of the packet. Because of support for repeated
packets, you have to set the sequence number yourself.*/
GF_Err gf_isom_rtp_packet_begin(GF_ISOFile *the_file, u32 trackNumber, s32 relativeTime, u8 PackingBit, u8 eXtensionBit, u8 MarkerBit, u8 PayloadType, u8 B_frame, u8 IsRepeatedPacket, u16 SequenceNumber);

/*set the flags of the RTP packet*/
GF_Err gf_isom_rtp_packet_set_flags(GF_ISOFile *the_file, u32 trackNumber, u8 PackingBit, u8 eXtensionBit, u8 MarkerBit, u8 disposable_packet, u8 IsRepeatedPacket);

/*set the time offset of this packet. This enables packets to be placed in the hint track 
in decoding order, but have their presentation time-stamp in the transmitted 
packet in a different order. Typically used for MPEG video with B-frames
*/
GF_Err gf_isom_rtp_packet_set_offset(GF_ISOFile *the_file, u32 trackNumber, s32 timeOffset);

								   
/*set some specific info in the HintDescription for RTP*/

/*sets the RTP TimeScale that the server use to send packets
some RTP payloads may need a specific timeScale that is not the timeScale in the file format
the default timeScale choosen by the API is the MediaTimeScale of the hint track*/
GF_Err gf_isom_rtp_set_timescale(GF_ISOFile *the_file, u32 trackNumber, u32 HintDescriptionIndex, u32 TimeScale);
/*sets the RTP TimeOffset that the server will add to the packets
if not set, the server adds a random offset*/
GF_Err gf_isom_rtp_set_time_offset(GF_ISOFile *the_file, u32 trackNumber, u32 HintDescriptionIndex, u32 TimeOffset);
/*sets the RTP SequenceNumber Offset that the server will add to the packets
if not set, the server adds a random offset*/
GF_Err gf_isom_rtp_set_time_sequence_offset(GF_ISOFile *the_file, u32 trackNumber, u32 HintDescriptionIndex, u32 SequenceNumberOffset);



/******************************************************************
		SDP SPECIFIC WRITING API
******************************************************************/
/*add an SDP line to the SDP container at the track level (media-specific SDP info)
NOTE: the \r\n end of line for SDP is automatically inserted*/
GF_Err gf_isom_sdp_add_track_line(GF_ISOFile *the_file, u32 trackNumber, const char *text);
/*remove all SDP info at the track level*/
GF_Err gf_isom_sdp_clean_track(GF_ISOFile *the_file, u32 trackNumber);

/*add an SDP line to the SDP container at the movie level (presentation SDP info)
NOTE: the \r\n end of line for SDP is automatically inserted*/
GF_Err gf_isom_sdp_add_line(GF_ISOFile *the_file, const char *text);
/*remove all SDP info at the movie level*/
GF_Err gf_isom_sdp_clean(GF_ISOFile *the_file);

#endif /*GPAC_DISABLE_ISOM_HINTING*/


#endif	/*GPAC_DISABLE_ISOM_WRITE*/

#ifndef GPAC_DISABLE_ISOM_DUMP

/*dumps file structures into XML trace file */
GF_Err gf_isom_dump(GF_ISOFile *file, FILE *trace);

#endif /*GPAC_DISABLE_ISOM_DUMP*/


#ifndef GPAC_DISABLE_ISOM_HINTING

#ifndef GPAC_DISABLE_ISOM_DUMP
/*dumps RTP hint samples structure into XML trace file
	@trackNumber, @SampleNum: hint track and hint sample number
	@trace: output
*/
GF_Err gf_isom_dump_hint_sample(GF_ISOFile *the_file, u32 trackNumber, u32 SampleNum, FILE * trace);
#endif

/*Get SDP info at the movie level*/
GF_Err gf_isom_sdp_get(GF_ISOFile *the_file, const char **sdp, u32 *length);
/*Get SDP info at the track level*/
GF_Err gf_isom_sdp_track_get(GF_ISOFile *the_file, u32 trackNumber, const char **sdp, u32 *length);

u32 gf_isom_get_payt_count(GF_ISOFile *the_file, u32 trackNumber);
const char *gf_isom_get_payt_info(GF_ISOFile *the_file, u32 trackNumber, u32 index, u32 *payID);




/*small hint reader - performs data caching*/

/*resets hint reading parameters, returns an error if the hint type is not supported for reading
packet sequence number is always reseted to 0
@sample_start: indicates from where the packets should be read (regular 1-based sample number)
@ts_offset: constant offset for timestamps, must be expressed in media timescale (which is the hint timescale).
	usually 0 (no offset)
@sn_offset: offset for packet sequence number (first packet will have a SN of 1 + sn_offset)
	usually 0
@ssrc: sync source identifier for RTP
*/
GF_Err gf_isom_reset_hint_reader(GF_ISOFile *the_file, u32 trackNumber, u32 sample_start, u32 ts_offset, u32 sn_offset, u32 ssrc);

/*reads next hint packet. ALl packets are read in transmission (decoding) order
returns an error if not supported, or GF_EOS when no more packets are available
currently only RTP reader is supported
@pck_data, @pck_size: output packet data (must be freed by caller) - contains all info to be sent 
	on the wire, eg for RTP contains the RTP header and the data
@disposable (optional): indicates that the packet can be droped when late (B-frames & co)
@repeated (optional): indicates this is a repeated packet (same one has already been sent)
@trans_ts (optional): indicates the transmission time of the packet, expressed in hint timescale, taking into account
the ts_offset specified in gf_isom_reset_hint_reader. Depending on packets this may not be the same
as the hint sample timestamp + ts_offset, some packets may need to be sent earlier (B-frames)
@sample_num (optional): indicates hint sample number the packet belongs to
*/
GF_Err gf_isom_next_hint_packet(GF_ISOFile *the_file, u32 trackNumber, char **pck_data, u32 *pck_size, Bool *disposable, Bool *repeated, u32 *trans_ts, u32 *sample_num);

#endif /*GPAC_DISABLE_ISOM_HINTING*/



/*
				3GPP specific extensions
	NOTE: MPEG-4 OD Framework cannot be used with 3GPP files.
	Stream Descriptions are not GF_ESD, just generic config options as specified in this file
*/

/*Generic 3GP/3GP2 config record*/
typedef struct 
{
	/*GF_4CC record type, one fo the above GF_ISOM_SUBTYPE_3GP_ * subtypes*/
	u32 type;
	/*4CC vendor name*/
	u32 vendor;
	/*codec version*/
	u8 decoder_version;
	/*number of sound frames per IsoMedia sample, >0 and <=15. The very last sample may contain less frames. */
	u8 frames_per_sample;

	/*H263 ONLY - Level and profile*/
	u8 H263_level, H263_profile;

	/*AMR(WB) ONLY - num of mode for the codec*/
	u16 AMR_mode_set;
	/*AMR(WB) ONLY - changes in codec mode per sample*/
	u8 AMR_mode_change_period;
} GF_3GPConfig;


/*return the 3GP config for this tream description, NULL if not a 3GPP track*/
GF_3GPConfig *gf_isom_3gp_config_get(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex);
#ifndef GPAC_DISABLE_ISOM_WRITE
/*create the track config*/
GF_Err gf_isom_3gp_config_new(GF_ISOFile *the_file, u32 trackNumber, GF_3GPConfig *config, char *URLname, char *URNname, u32 *outDescriptionIndex);
/*update the track config - subtypes shall NOT differ*/
GF_Err gf_isom_3gp_config_update(GF_ISOFile *the_file, u32 trackNumber, GF_3GPConfig *config, u32 DescriptionIndex);
#endif	/*GPAC_DISABLE_ISOM_WRITE*/

/*AVC/H264 extensions - GF_AVCConfig is defined in mpeg4_odf.h*/

/*gets uncompressed AVC config - user is responsible for deleting it*/
GF_AVCConfig *gf_isom_avc_config_get(GF_ISOFile *the_file, u32 trackNumber, u32 DescriptionIndex);
/*gets uncompressed AVC config - user is responsible for deleting it*/
GF_AVCConfig *gf_isom_svc_config_get(GF_ISOFile *the_file, u32 trackNumber, u32 DescriptionIndex);

#ifndef GPAC_DISABLE_ISOM_WRITE
/*creates new AVC config*/
GF_Err gf_isom_avc_config_new(GF_ISOFile *the_file, u32 trackNumber, GF_AVCConfig *cfg, char *URLname, char *URNname, u32 *outDescriptionIndex);
/*updates AVC config*/
GF_Err gf_isom_avc_config_update(GF_ISOFile *the_file, u32 trackNumber, u32 DescriptionIndex, GF_AVCConfig *cfg);
/*updates SVC config. If is_additional is set, the SVCConfig will be added to the AVC sample description, otherwise the sample description will be SVC-only*/
GF_Err gf_isom_svc_config_update(GF_ISOFile *the_file, u32 trackNumber, u32 DescriptionIndex, GF_AVCConfig *cfg, Bool is_additional);
#endif /*GPAC_DISABLE_ISOM_WRITE*/


/*
	3GP timed text handling

	NOTE: currently only writing API is developped, the reading one is not used in MPEG-4 since
	MPEG-4 maps 3GP timed text to MPEG-4 Streaming Text (part 17)
*/

/*set streamihng text reading mode: if do_convert is set, all text samples will be retrieved as TTUs
and ESD will be emulated for text tracks.*/
GF_Err gf_isom_text_set_streaming_mode(GF_ISOFile *the_file, Bool do_convert);


#ifndef GPAC_DISABLE_ISOM_DUMP
/*exports text track to given format
@dump_type: 0 for TTXT, 1 for srt, 2 for SVG
*/
GF_Err gf_isom_text_dump(GF_ISOFile *the_file, u32 track, FILE *dump, u32 dump_type);
#endif

/*returns encoded TX3G box (text sample description for 3GPP text streams) as needed by RTP or other standards:
	@sidx: 1-based stream description index
	@sidx_offset: 
		if 0, the sidx will NOT be written before the encoded TX3G
		if not 0, the sidx will be written before the encoded TX3G, with the given offset. Offset sshould be at 
		least 128 for most commmon usage of TX3G (RTP, MPEG-4 timed text, etc)

*/
GF_Err gf_isom_text_get_encoded_tx3g(GF_ISOFile *file, u32 track, u32 sidx, u32 sidx_offset, char **tx3g, u32 *tx3g_size);

/*checks if this text description is already inserted
@outDescIdx: set to 0 if not found, or descIndex
@same_style, @same_box: indicates if default styles and box are used
*/
GF_Err gf_isom_text_has_similar_description(GF_ISOFile *the_file, u32 trackNumber, GF_TextSampleDescriptor *desc, u32 *outDescIdx, Bool *same_box, Bool *same_styles);

/*text sample formatting*/
typedef struct _3gpp_text_sample GF_TextSample;
/*creates text sample handle*/
GF_TextSample *gf_isom_new_text_sample();
/*destroy text sample handle*/
void gf_isom_delete_text_sample(GF_TextSample *tx_samp);

#ifndef GPAC_DISABLE_ISOM_WRITE

/*Create a new TextSampleDescription in the file. 
The URL and URN are used to describe external media, this will create a data reference for the media
GF_TextSampleDescriptor is defined in mpeg4_odf.h
*/
GF_Err gf_isom_new_text_description(GF_ISOFile *the_file, u32 trackNumber, GF_TextSampleDescriptor *desc, char *URLname, char *URNname, u32 *outDescriptionIndex);
/*change the text sample description*/
GF_Err gf_isom_update_text_description(GF_ISOFile *movie, u32 trackNumber, u32 descriptionIndex, GF_TextSampleDescriptor *desc);

/*reset text sample content*/
GF_Err gf_isom_text_reset(GF_TextSample * tx_samp);
/*reset text sample styles but keep text*/
GF_Err gf_isom_text_reset_styles(GF_TextSample * samp);

/*sets UTF16 marker for text data. This MUST be called on an empty sample. If text data added later 
on (cf below) is not formatted as UTF16 data(2 bytes char) the resulting text sample won't be compliant, 
but this library WON'T WARN*/
GF_Err gf_isom_text_set_utf16_marker(GF_TextSample * samp);
/*append text to sample - text_len is the number of bytes to be written from text_data. This allows 
handling UTF8 and UTF16 strings in a transparent manner*/
GF_Err gf_isom_text_add_text(GF_TextSample * tx_samp, char *text_data, u32 text_len);
/*append style modifyer to sample*/
GF_Err gf_isom_text_add_style(GF_TextSample * tx_samp, GF_StyleRecord *rec);
/*appends highlight modifier for the sample 
	@start_char: first char highlighted, 
	@end_char: first char not highlighted*/
GF_Err gf_isom_text_add_highlight(GF_TextSample * samp, u16 start_char, u16 end_char);
/*sets highlight color for the whole sample*/
GF_Err gf_isom_text_set_highlight_color(GF_TextSample * samp, u8 r, u8 g, u8 b, u8 a);
GF_Err gf_isom_text_set_highlight_color_argb(GF_TextSample * samp, u32 argb);
/*appends a new karaoke sequence in the sample
	@start_time: karaoke start time expressed in text stream timescale, but relative to the sample media time
*/
GF_Err gf_isom_text_add_karaoke(GF_TextSample * samp, u32 start_time);
/*appends a new segment in the current karaoke sequence - you must build sequences in order to be compliant
	@end_time: segment end time expressed in text stream timescale, but relative to the sample media time
	@start_char: first char highlighted, 
	@end_char: first char not highlighted
*/
GF_Err gf_isom_text_set_karaoke_segment(GF_TextSample * samp, u32 end_time, u16 start_char, u16 end_char);
/*sets scroll delay for the whole sample (scrolling is enabled through GF_TextSampleDescriptor.DisplayFlags)
	@scroll_delay: delay for scrolling expressed in text stream timescale
*/
GF_Err gf_isom_text_set_scroll_delay(GF_TextSample * samp, u32 scroll_delay);
/*appends hyperlinking for the sample
	@URL: ASCII url
	@altString: ASCII hint (tooltip, ...) for end user
	@start_char: first char hyperlinked, 
	@end_char: first char not hyperlinked
*/
GF_Err gf_isom_text_add_hyperlink(GF_TextSample * samp, char *URL, char *altString, u16 start_char, u16 end_char);
/*sets current text box (display pos&size within the text track window) for the sample*/
GF_Err gf_isom_text_set_box(GF_TextSample * samp, s16 top, s16 left, s16 bottom, s16 right);
/*appends blinking for the sample
	@start_char: first char blinking, 
	@end_char: first char not blinking
*/
GF_Err gf_isom_text_add_blink(GF_TextSample * samp, u16 start_char, u16 end_char);
/*sets wrap flag for the sample - currently only 0 (no wrap) and 1 ("soft wrap") are allowed in 3GP*/
GF_Err gf_isom_text_set_wrap(GF_TextSample * samp, u8 wrap_flags);

/*formats sample as a regular GF_ISOSample. The resulting sample will always be marked as random access
text sample content is kept untouched*/
GF_ISOSample *gf_isom_text_to_sample(GF_TextSample * tx_samp);

#endif	/*GPAC_DISABLE_ISOM_WRITE*/

/*****************************************************
		ISMACryp Samples
*****************************************************/
/*flags for GF_ISMASample*/
enum 
{
	/*signals the stream the sample belongs to uses selective encryption*/
	GF_ISOM_ISMA_USE_SEL_ENC = 1,
	/*signals the sample is encrypted*/
	GF_ISOM_ISMA_IS_ENCRYPTED = 2,
};

typedef struct
{
	/*IV in ISMACryp is Byte Stream Offset*/
	u64 IV;
	u8 IV_length;/*repeated from sampleDesc for convenience*/
	u8 *key_indicator;
	u8 KI_length;/*repeated from sampleDesc for convenience*/
	u32 dataLength;
	char *data;
	u32 flags;
} GF_ISMASample;
/**
 * creates a new empty ISMA sample
 */
GF_ISMASample *gf_isom_ismacryp_new_sample();

/*delete an ISMA sample. NOTE:the buffers content will be destroyed by default.
if you wish to keep the buffer, set dataLength to 0 in the sample before deleting it*/
void gf_isom_ismacryp_delete_sample(GF_ISMASample *samp);

/*decodes ISMACryp sample based on all info in ISMACryp sample description*/
GF_ISMASample *gf_isom_ismacryp_sample_from_data(char *data, u32 dataLength, Bool use_selective_encryption, u8 KI_length, u8 IV_length);
/*rewrites samp content from s content*/
GF_Err gf_isom_ismacryp_sample_to_sample(GF_ISMASample *s, GF_ISOSample *dest);

/*decodes ISMACryp sample based on sample and its descrition index - returns NULL if not an ISMA sample 
Note: input sample is NOT destroyed*/
GF_ISMASample *gf_isom_get_ismacryp_sample(GF_ISOFile *the_file, u32 trackNumber, GF_ISOSample *samp, u32 sampleDescriptionIndex);

/*returns whether the given media is a protected one or not - return scheme protection 4CC*/
u32 gf_isom_is_media_encrypted(GF_ISOFile *the_file, u32 trackNumber, u32 sampleDescriptionIndex);

/*returns whether the given media is a protected ISMACryp one or not*/
Bool gf_isom_is_ismacryp_media(GF_ISOFile *the_file, u32 trackNumber, u32 sampleDescriptionIndex);

/*returns whether the given media is a protected ISMACryp one or not*/
Bool gf_isom_is_omadrm_media(GF_ISOFile *the_file, u32 trackNumber, u32 sampleDescriptionIndex);

GF_Err gf_isom_get_omadrm_info(GF_ISOFile *the_file, u32 trackNumber, u32 sampleDescriptionIndex, u32 *outOriginalFormat,
							   u32 *outSchemeType, u32 *outSchemeVersion,
							   const char **outContentID, const char **outRightsIssuerURL, const char **outTextualHeaders, u32 *outTextualHeadersLen, u64 *outPlaintextLength, u32 *outEncryptionType, Bool *outSelectiveEncryption, u32 *outIVLength, u32 *outKeyIndicationLength);
/*retrieves ISMACryp info for the given track & SDI - all output parameters are optional - URIs SHALL NOT BE MODIFIED BY USER
	@outOriginalFormat: retrieves orginal protected media format - usually GF_ISOM_SUBTYPE_MPEG4
	@outSchemeType: retrieves 4CC of protection scheme (GF_ISOM_ISMACRYP_SCHEME = iAEC in ISMACryp 1.0)
	outSchemeVersion: retrieves version of protection scheme (1 in ISMACryp 1.0)
	outSchemeURI: retrieves URI location of scheme 
	outKMS_URI: retrieves URI location of key management system - only valid with ISMACryp 1.0
	outSelectiveEncryption: specifies whether sample-based encryption is used in media - only valid with ISMACryp 1.0
	outIVLength: specifies length of Initial Vector - only valid with ISMACryp 1.0
	outKeyIndicationLength: specifies length of key indicator - only valid with ISMACryp 1.0

  outSelectiveEncryption, outIVLength and outKeyIndicationLength are usually not needed to decode an 
  ISMA sample when using gf_isom_get_ismacryp_sample fct above
*/
GF_Err gf_isom_get_ismacryp_info(GF_ISOFile *the_file, u32 trackNumber, u32 sampleDescriptionIndex, u32 *outOriginalFormat, u32 *outSchemeType, u32 *outSchemeVersion, const char **outSchemeURI, const char **outKMS_URI, Bool *outSelectiveEncryption, u32 *outIVLength, u32 *outKeyIndicationLength);


#ifndef GPAC_DISABLE_ISOM_WRITE
/*removes ISMACryp protection info (does not perform decryption :)*/
GF_Err gf_isom_remove_ismacryp_protection(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex);

/*creates ISMACryp protection info (does not perform encryption :)*/
GF_Err gf_isom_set_ismacryp_protection(GF_ISOFile *the_file, u32 trackNumber, u32 desc_index, u32 scheme_type, 
						   u32 scheme_version, char *scheme_uri, char *kms_URI,
						   Bool selective_encryption, u32 KI_length, u32 IV_length);

/*change scheme URI and/or KMS URI for crypted files. Other params cannot be changed once the media is crypted
	@scheme_uri: new scheme URI, or NULL to keep previous
	@kms_uri: new KMS URI, or NULL to keep previous
*/
GF_Err gf_isom_change_ismacryp_protection(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex, char *scheme_uri, char *kms_uri);


GF_Err gf_isom_set_oma_protection(GF_ISOFile *the_file, u32 trackNumber, u32 desc_index,
						   char *contentID, char *kms_URI, u32 encryption_type, u64 plainTextLength, char *textual_headers, u32 textual_headers_len,
						   Bool selective_encryption, u32 KI_length, u32 IV_length);

#endif /*GPAC_DISABLE_ISOM_WRITE*/

#ifndef GPAC_DISABLE_ISOM_DUMP
/*xml dumpers*/
GF_Err gf_isom_dump_ismacryp_protection(GF_ISOFile *the_file, u32 trackNumber, FILE * trace);
GF_Err gf_isom_dump_ismacryp_sample(GF_ISOFile *the_file, u32 trackNumber, u32 SampleNum, FILE *trace);
#endif


/********************************************************************
				GENERAL META API FUNCTIONS
	
	  Meta can be stored at several places in the file layout:
		* root level (like moov, ftyp and co)
		* moov level
		* track level
	Meta API uses the following parameters for all functions:
	
	 gf_isom_*_meta_*(GF_ISOFile *file, Bool root_meta, u32 track_num, ....) with:
		@root_meta: if set, accesses file root meta
		@track_num: if root_meta not set, specifies whether the target meta is at the
			moov level (track_num=0) or at the track level.

********************************************************************/

/*gets meta type. Returned value: 0 if no meta found, or four char code of meta (eg, "mp21", "smil", ...)*/
u32 gf_isom_get_meta_type(GF_ISOFile *file, Bool root_meta, u32 track_num);

/*indicates if the meta has an XML container (note that XML data can also be included as items). 
return value: 0 (no XML or error), 1 (XML text), 2 (BinaryXML, eg BiM) */
u32 gf_isom_has_meta_xml(GF_ISOFile *file, Bool root_meta, u32 track_num);

/*extracts XML (if any) from given meta
	@outName: output file path and location for writing
	@is_binary: indicates if XML is Bim or regular XML
*/
GF_Err gf_isom_extract_meta_xml(GF_ISOFile *file, Bool root_meta, u32 track_num, char *outName, Bool *is_binary);

/*returns number of items described in this meta*/
u32 gf_isom_get_meta_item_count(GF_ISOFile *file, Bool root_meta, u32 track_num);

/*gets item info for the given item
	@item_num: 1-based index of item to query
	@itemID (optional): item ID in file
	@is_self_reference: item is the file itself
	@item_name (optional): item name
	@item_mime_type (optional): item mime type
	@item_encoding (optional): item content encoding type
	@item_url, @item_urn (optional): url/urn of external resource containing this item data if any. 
		When item is fully contained in file, these are set to NULL

*/
GF_Err gf_isom_get_meta_item_info(GF_ISOFile *file, Bool root_meta, u32 track_num, u32 item_num, 
							u32 *itemID, u32 *protection_idx, Bool *is_self_reference,
							const char **item_name, const char **item_mime_type, const char **item_encoding,
							const char **item_url, const char **item_urn);


/*gets item idx from item ID*/
u32 gf_isom_get_meta_item_by_id(GF_ISOFile *file, Bool root_meta, u32 track_num, u32 item_ID);

/*extracts item from given meta
	@item_num: 1-based index of item to query
*/
GF_Err gf_isom_extract_meta_item(GF_ISOFile *file, Bool root_meta, u32 track_num, u32 item_num, const char *dump_file_name);

/*retirves primary item ID, 0 if none found (primary can also be stored through meta XML)*/
u32 gf_isom_get_meta_primary_item_id(GF_ISOFile *file, Bool root_meta, u32 track_num);

#ifndef GPAC_DISABLE_ISOM_WRITE

/*sets meta type (four char int, eg "mp21", ... 
	Creates a meta box if none found
	if metaType is 0, REMOVES META 
*/
GF_Err gf_isom_set_meta_type(GF_ISOFile *file, Bool root_meta, u32 track_num, u32 metaType);

/*removes meta XML info if any*/
GF_Err gf_isom_remove_meta_xml(GF_ISOFile *file, Bool root_meta, u32 track_num);

/*set meta XML data from file - erase any previously (Binary)XML info*/
GF_Err gf_isom_set_meta_xml(GF_ISOFile *file, Bool root_meta, u32 track_num, char *XMLFileName, Bool IsBinaryXML);
/*set meta XML data from memory - erase any previously (Binary)XML info*/
GF_Err gf_isom_set_meta_xml_memory(GF_ISOFile *file, Bool root_meta, u32 track_num, unsigned char *data, u32 data_size, Bool IsBinaryXML);

/*adds item to meta:
	@self_reference: indicates this item is the file itself
	@resource_path: file to add - can be NULL when URL/URN is used
	@item_name: item name - if NULL, use file name. CANNOT BE NULL if resource_path is not set
	@mime_type: item mime type - if NULL, use "application/octet-stream"
	@content_encoding: content encoding type - if NULL, none specified
	@URL, @URN: if set, resource will be remote (same as stream descriptions)
*/
GF_Err gf_isom_add_meta_item(GF_ISOFile *file, Bool root_meta, u32 track_num, Bool self_reference, char *resource_path, const char *item_name, const char *mime_type, const char *content_encoding, const char *URL, const char *URN);

/*removes item from meta*/
GF_Err gf_isom_remove_meta_item(GF_ISOFile *file, Bool root_meta, u32 track_num, u32 item_num);

/*sets the given item as the primary one. You SHALL NOT use this if the meta has a valid XML data*/
GF_Err gf_isom_set_meta_primary_item(GF_ISOFile *file, Bool root_meta, u32 track_num, u32 item_num);

#endif /*GPAC_DISABLE_ISOM_WRITE*/


/********************************************************************
				Timed Meta-Data extensions
********************************************************************/

GF_Err gf_isom_get_timed_meta_data_info(GF_ISOFile *file, u32 track, u32 sampleDescription, Bool *is_xml, const char **mime_or_namespace, const char **content_encoding, const char **schema_loc);

#ifndef GPAC_DISABLE_ISOM_WRITE
/*create a new timed metat data sample description for this track*/
GF_Err gf_isom_timed_meta_data_config_new(GF_ISOFile *movie, u32 trackNumber, Bool is_xml, char *mime_or_namespace, char *content_encoding, char *schema_loc, char *URLname, char *URNname, u32 *outDescriptionIndex);
#endif /*GPAC_DISABLE_ISOM_WRITE*/


/********************************************************************
				iTunes info tags
********************************************************************/
enum
{
	/*probe is only used ti check if iTunes info are present*/
	GF_ISOM_ITUNE_PROBE = 0,
	GF_ISOM_ITUNE_ALBUM	= GF_4CC( 0xA9, 'a', 'l', 'b' ),
	GF_ISOM_ITUNE_ARTIST = GF_4CC( 0xA9, 'A', 'R', 'T' ),
	GF_ISOM_ITUNE_COMMENT = GF_4CC( 0xA9, 'c', 'm', 't' ),
	GF_ISOM_ITUNE_COMPILATION = GF_4CC( 'c', 'p', 'i', 'l' ),
	GF_ISOM_ITUNE_COMPOSER = GF_4CC( 0xA9, 'c', 'o', 'm' ),
	GF_ISOM_ITUNE_COVER_ART = GF_4CC( 'c', 'o', 'v', 'r' ),
	GF_ISOM_ITUNE_CREATED = GF_4CC( 0xA9, 'd', 'a', 'y' ),
	GF_ISOM_ITUNE_DISK = GF_4CC( 'd', 'i', 's', 'k' ),
	GF_ISOM_ITUNE_TOOL = GF_4CC( 0xA9, 't', 'o', 'o' ),
	GF_ISOM_ITUNE_GENRE = GF_4CC( 'g', 'n', 'r', 'e' ),
	GF_ISOM_ITUNE_GROUP = GF_4CC( 0xA9, 'g', 'r', 'p' ),
	GF_ISOM_ITUNE_ITUNES_DATA = GF_4CC( '-', '-', '-', '-' ),
	GF_ISOM_ITUNE_NAME = GF_4CC( 0xA9, 'n', 'a', 'm' ),
	GF_ISOM_ITUNE_TEMPO = GF_4CC( 't', 'm', 'p', 'o' ),
	GF_ISOM_ITUNE_TRACK = GF_4CC( 0xA9, 't', 'r', 'k' ),
	GF_ISOM_ITUNE_TRACKNUMBER = GF_4CC( 't', 'r', 'k', 'n' ),
	GF_ISOM_ITUNE_WRITER = GF_4CC( 0xA9, 'w', 'r', 't' ),
	GF_ISOM_ITUNE_ENCODER = GF_4CC( 0xA9, 'e', 'n', 'c' ),
	GF_ISOM_ITUNE_ALBUM_ARTIST = GF_4CC( 'a', 'A', 'R', 'T' ),
	GF_ISOM_ITUNE_GAPLESS = GF_4CC( 'p', 'g', 'a', 'p' ),
};
/*get the given tag info. 
!! 'genre' may be coded by ID, the libisomedia doesn't translate the ID. In such a case, the result data is set to NULL 
and the data_len to the genre ID
returns GF_URL_ERROR if no tag is present in the file
*/
GF_Err gf_isom_apple_get_tag(GF_ISOFile *mov, u32 tag, const char **data, u32 *data_len);
#ifndef GPAC_DISABLE_ISOM_WRITE
/*set the given tag info. If data and data_len are 0, removes the given tag
For 'genre', data may be NULL in which case the genre ID taken from the data_len parameter
*/
GF_Err gf_isom_apple_set_tag(GF_ISOFile *mov, u32 tag, const char *data, u32 data_len);

/*sets compatibility tag on AVC tracks (needed by iPod to play files... hurray for standards)*/
GF_Err gf_isom_set_ipod_compatible(GF_ISOFile *the_file, u32 trackNumber);
#endif /*GPAC_DISABLE_ISOM_WRITE*/


/*3GPP Alternate Group API - (c) 2007 ENST & ResonateMP4*/

/*gets the number of switching groups declared in this track if any:
trackNumber: track number
alternateGroupID: alternate group id of track if speciifed, 0 otherwise
nb_groups: number of switching groups defined for this track
*/
GF_Err gf_isom_get_track_switch_group_count(GF_ISOFile *movie, u32 trackNumber, u32 *alternateGroupID, u32 *nb_groups);

/*returns the list of criteria (expressed as 4CC IDs, cf 3GPP TS 26.244)
trackNumber: track number
group_index: 1-based index of the group to inspect
switchGroupID: ID of the switch group if any, 0 otherwise (alternate-only group)
criteriaListSize: number of criteria items in returned list
*/
const u32 *gf_isom_get_track_switch_parameter(GF_ISOFile *movie, u32 trackNumber, u32 group_index, u32 *switchGroupID, u32 *criteriaListSize);

#ifndef GPAC_DISABLE_ISOM_WRITE
/*sets a new (switch) group for this track
trackNumber: track
trackRefGroup: number of a track belonging to the same alternate group. If 0, a new alternate group will be created for this track
is_switch_group: if set, indicates that a switch group identifier shall be assigned to the created group. Otherwise, the criteria list is associated with the entire alternate group
switchGroupID: SHALL NOT BE NULL
	input: specifies the desired switchGroupID to use. If value is 0, next available switchGroupID in file is used.
	output: indicates the switchGroupID used.
criteriaList, criteriaListCount: criteria list and size. Criterias are expressed as 4CC IDs, cf 3GPP TS 26.244
*/
GF_Err gf_isom_set_track_switch_parameter(GF_ISOFile *movie, u32 trackNumber, u32 trackRefGroup, Bool is_switch_group, u32 *switchGroupID, u32 *criteriaList, u32 criteriaListCount);

/*resets track switch group information for the track or for the entire alternate group this track belongs to if reset_all_group is set*/
GF_Err gf_isom_reset_track_switch_parameter(GF_ISOFile *movie, u32 trackNumber, Bool reset_all_group);

/*resets ALL track switch group information in the entire movie*/
GF_Err gf_isom_reset_switch_parameters(GF_ISOFile *movie);

#endif /*GPAC_DISABLE_ISOM_WRITE*/


typedef struct
{
	u8 profile;
	u8 level;
	u8 pathComponents;
	Bool fullRequestHost;
	Bool streamType;
	u8 containsRedundant;
	const char *textEncoding;
	const char *contentEncoding;
	const char *content_script_types;
} GF_DIMSDescription;

GF_Err gf_isom_get_dims_description(GF_ISOFile *movie, u32 trackNumber, u32 descriptionIndex, GF_DIMSDescription *desc);
#ifndef GPAC_DISABLE_ISOM_WRITE
GF_Err gf_isom_new_dims_description(GF_ISOFile *movie, u32 trackNumber, GF_DIMSDescription *desc, char *URLname, char *URNname, u32 *outDescriptionIndex);
GF_Err gf_isom_update_dims_description(GF_ISOFile *movie, u32 trackNumber, GF_DIMSDescription *desc, char *URLname, char *URNname, u32 DescriptionIndex);
#endif /*GPAC_DISABLE_ISOM_WRITE*/




/*AC3 config record*/
typedef struct 
{
	u8 fscod;
	u8 bsid;
	u8 bsmod;
	u8 acmod;
	u8 lfon;
	u8 brcode;
} GF_AC3Config;

GF_AC3Config *gf_isom_ac3_config_get(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex);

#ifndef GPAC_DISABLE_ISOM_WRITE
GF_Err gf_isom_ac3_config_new(GF_ISOFile *the_file, u32 trackNumber, GF_AC3Config *cfg, char *URLname, char *URNname, u32 *outDescriptionIndex);
#endif /*GPAC_DISABLE_ISOM_WRITE*/


/*returns the number of subsamples in the given sample */
u32 gf_isom_sample_has_subsamples(GF_ISOFile *movie, u32 track, u32 sampleNumber);
GF_Err gf_isom_sample_get_subsample(GF_ISOFile *movie, u32 track, u32 sampleNumber, u32 subSampleNumber, u32 *size, u8 *priority, u32 *reserved, Bool *discardable);
#ifndef GPAC_DISABLE_ISOM_WRITE
/*adds subsample information to a given sample. Subsample information shall be added in increasing order of sampleNumbers, insertion of information is not supported.
Note that you may add subsample information for samples not yet added to the file
specifying 0 as subSampleSize will remove the last subsample information if any*/
GF_Err gf_isom_add_subsample(GF_ISOFile *movie, u32 track, u32 sampleNumber, u32 subSampleSize, u8 priority, u32 reserved, Bool discardable);
#endif
/*add subsample information for the latest sample added to the current track fragment*/
GF_Err gf_isom_fragment_add_subsample(GF_ISOFile *movie, u32 TrackID, u32 subSampleSize, u8 priority, u32 reserved, Bool discardable);
/*copy over the subsample information of the given sample from the source track/file to the last sample added to the current track fragment of the destination file*/
GF_Err gf_isom_fragment_copy_subsample(GF_ISOFile *dest, u32 TrackID, GF_ISOFile *orig, u32 track, u32 sampleNumber);


#endif /*GPAC_DISABLE_ISOM*/

#ifdef __cplusplus
}
#endif


#endif	/*_GF_ISOMEDIA_H_*/


