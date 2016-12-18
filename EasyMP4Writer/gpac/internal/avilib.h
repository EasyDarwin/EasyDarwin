/*
 *  avilib.h
 *
 *  Copyright (C) Thomas Östreich - June 2001
 *  multiple audio track support Copyright (C) 2002 Thomas Östreich
 *
 *  Original code:
 *  Copyright (C) 1999 Rainer Johanni <Rainer@Johanni.de> 
 *
 *  This file is part of transcode, a linux video stream processing tool
 *      
 *  transcode is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  transcode is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *   
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */

#ifndef _GF_AVILIB_H_
#define _GF_AVILIB_H_

#include <gpac/tools.h>

#ifndef GPAC_DISABLE_AVILIB

#define AVI_MAX_TRACKS 8

typedef struct
{
  u64 key;
  u64 pos;
  u64 len;
} video_index_entry;

typedef struct
{
   u64 pos;
   u64 len;
   u64 tot;
} audio_index_entry;


// Index types


#define AVI_INDEX_OF_INDEXES 0x00             // when each entry in aIndex
                                              // array points to an index chunk
#define AVI_INDEX_OF_CHUNKS  0x01             // when each entry in aIndex
                                              // array points to a chunk in the file
#define AVI_INDEX_IS_DATA    0x80             // when each entry is aIndex is
                                              // really the data
// bIndexSubtype codes for INDEX_OF_CHUNKS
//
#define AVI_INDEX_2FIELD     0x01             // when fields within frames
                                              // are also indexed



typedef struct _avisuperindex_entry {
    u64 qwOffset;           // absolute file offset
    u32 dwSize;                  // size of index chunk at this offset
    u32 dwDuration;              // time span in stream ticks
} avisuperindex_entry;

typedef struct _avistdindex_entry {
    u32 dwOffset;                // qwBaseOffset + this is absolute file offset
    u32 dwSize;                  // bit 31 is set if this is NOT a keyframe
} avistdindex_entry;

// Standard index 
typedef struct _avistdindex_chunk {
    char           fcc[4];                 // ix##
    u32 dwSize;                 // size of this chunk
    u16 wLongsPerEntry;         // must be sizeof(aIndex[0])/sizeof(DWORD)
    u8  bIndexSubType;          // must be 0
    u8  bIndexType;             // must be AVI_INDEX_OF_CHUNKS
    u32  nEntriesInUse;          //
    char           dwChunkId[4];           // '##dc' or '##db' or '##wb' etc..
    u64 qwBaseOffset;       // all dwOffsets in aIndex array are relative to this
    u32  dwReserved3;            // must be 0
    avistdindex_entry *aIndex;
} avistdindex_chunk;
    

// Base Index Form 'indx'
typedef struct _avisuperindex_chunk {
    char           fcc[4];
    u32  dwSize;                 // size of this chunk
    u16 wLongsPerEntry;         // size of each entry in aIndex array (must be 8 for us)
    u8  bIndexSubType;          // future use. must be 0
    u8 bIndexType;             // one of AVI_INDEX_* codes
    u32 nEntriesInUse;          // index of first unused member in aIndex array
    char           dwChunkId[4];           // fcc of what is indexed
    u32 dwReserved[3];          // meaning differs for each index type/subtype.
                                           // 0 if unused
    avisuperindex_entry *aIndex;           // where are the ix## chunks
    avistdindex_chunk **stdindex;          // the ix## chunks itself (array)
} avisuperindex_chunk;
    


typedef struct track_s
{

    long   a_fmt;             /* Audio format, see #defines below */
    long   a_chans;           /* Audio channels, 0 for no audio */
    long   a_rate;            /* Rate in Hz */
    long   a_bits;            /* bits per audio sample */
    long   mp3rate;           /* mp3 bitrate kbs*/
    long   a_vbr;             /* 0 == no Variable BitRate */
    long   padrate;	      /* byte rate used for zero padding */

    long   audio_strn;        /* Audio stream number */
    u64  audio_bytes;       /* Total number of bytes of audio data */
    long   audio_chunks;      /* Chunks of audio data in the file */

    char   audio_tag[4];      /* Tag of audio data */
    long   audio_posc;        /* Audio position: chunk */
    long   audio_posb;        /* Audio position: byte within chunk */
 
    u64  a_codech_off;       /* absolut offset of audio codec information */ 
    u64  a_codecf_off;       /* absolut offset of audio codec information */ 

    audio_index_entry *audio_index;
    avisuperindex_chunk *audio_superindex;

} track_t;

typedef struct
{
  u32  bi_size;
  u32  bi_width;
  u32  bi_height;
  u16  bi_planes;
  u16  bi_bit_count;
  u32  bi_compression;
  u32  bi_size_image;
  u32  bi_x_pels_per_meter;
  u32  bi_y_pels_per_meter;
  u32  bi_clr_used;
  u32  bi_clr_important;
} alBITMAPINFOHEADER;

typedef struct
{
  u16  w_format_tag;
  u16  n_channels;
  u32  n_samples_per_sec;
  u32  n_avg_bytes_per_sec;
  u16  n_block_align;
  u16  w_bits_per_sample;
  u16  cb_size;
} alWAVEFORMATEX;

typedef struct
{
  u32 fcc_type; 
  u32 fcc_handler; 
  u32 dw_flags; 
  u32 dw_caps; 
  u16 w_priority;
  u16 w_language;
  u32 dw_scale;
  u32 dw_rate;
  u32 dw_start;
  u32 dw_length;
  u32 dw_initial_frames;
  u32 dw_suggested_buffer_size;
  u32 dw_quality;
  u32 dw_sample_size;
  u16 dw_left;
  u16 dw_top;
  u16 dw_right;
  u16 dw_bottom;
} alAVISTREAMHEADER;

typedef struct
{
  
  FILE *fdes;              /* File descriptor of AVI file */
  long   mode;              /* 0 for reading, 1 for writing */
  
  long   width;             /* Width  of a video frame */
  long   height;            /* Height of a video frame */
  double fps;               /* Frames per second */
  char   compressor[8];     /* Type of compressor, 4 bytes + padding for 0 byte */
  char   compressor2[8];     /* Type of compressor, 4 bytes + padding for 0 byte */
  u32   video_strn;        /* Video stream number */
  long   video_frames;      /* Number of video frames */
  char   video_tag[4];      /* Tag of video data */
  long   video_pos;         /* Number of next frame to be read
			       (if index present) */
  alAVISTREAMHEADER video_stream_header;
  
  u32 max_len;    /* maximum video chunk present */
  
  track_t track[AVI_MAX_TRACKS];  // up to AVI_MAX_TRACKS audio tracks supported
  
  s64  pos;               /* position in file */
  long   n_idx;             /* number of index entries actually filled */
  long   max_idx;           /* number of index entries actually allocated */
  
  s64  v_codech_off;      /* absolut offset of video codec (strh) info */ 
  s64  v_codecf_off;      /* absolut offset of video codec (strf) info */ 
  
  u8 (*idx)[16]; /* index entries (AVI idx1 tag) */

  video_index_entry *video_index;
  avisuperindex_chunk *video_superindex;  /* index of indices */
  int is_opendml;           /* set to 1 if this is an odml file with multiple index chunks */
  
  s64  last_pos;          /* Position of last frame written */
  u32 last_len;   /* Length of last frame written */
  int must_use_index;       /* Flag if frames are duplicated */
  s64  movi_start;
  int total_frames;         /* total number of frames if dmlh is present */
  
  u32 anum;            // total number of audio tracks 
  u32 aptr;            // current audio working track 
  char *index_file;    // read the avi index from this file
  
  alBITMAPINFOHEADER *bitmap_info_header;
  alWAVEFORMATEX *wave_format_ex[AVI_MAX_TRACKS];
  alAVISTREAMHEADER stream_headers[AVI_MAX_TRACKS];

  void*		extradata;
  unsigned long	extradata_size;
} avi_t;

#define AVI_MODE_WRITE  0
#define AVI_MODE_READ   1

/* The error codes delivered by avi_open_input_file */

#define AVI_ERR_SIZELIM      1     /* The write of the data would exceed
                                      the maximum size of the AVI file.
                                      This is more a warning than an error
                                      since the file may be closed safely */

#define AVI_ERR_OPEN         2     /* Error opening the AVI file - wrong path
                                      name or file nor readable/writable */

#define AVI_ERR_READ         3     /* Error reading from AVI File */

#define AVI_ERR_WRITE        4     /* Error writing to AVI File,
                                      disk full ??? */

#define AVI_ERR_WRITE_INDEX  5     /* Could not write index to AVI file
                                      during close, file may still be
                                      usable */

#define AVI_ERR_CLOSE        6     /* Could not write header to AVI file
                                      or not truncate the file during close,
                                      file is most probably corrupted */

#define AVI_ERR_NOT_PERM     7     /* Operation not permitted:
                                      trying to read from a file open
                                      for writing or vice versa */

#define AVI_ERR_NO_MEM       8     /* malloc failed */

#define AVI_ERR_NO_AVI       9     /* Not an AVI file */

#define AVI_ERR_NO_HDRL     10     /* AVI file has no has no header list,
                                      corrupted ??? */

#define AVI_ERR_NO_MOVI     11     /* AVI file has no has no MOVI list,
                                      corrupted ??? */

#define AVI_ERR_NO_VIDS     12     /* AVI file contains no video data */

#define AVI_ERR_NO_IDX      13     /* The file has been opened with
                                      getIndex==0, but an operation has been
                                      performed that needs an index */

/* Possible Audio formats */

#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_UNKNOWN             (0x0000)
#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM                 (0x0001)
#endif
#define WAVE_FORMAT_ADPCM               (0x0002)
#define WAVE_FORMAT_IBM_CVSD            (0x0005)
#define WAVE_FORMAT_ALAW                (0x0006)
#define WAVE_FORMAT_MULAW               (0x0007)
#define WAVE_FORMAT_OKI_ADPCM           (0x0010)
#define WAVE_FORMAT_DVI_ADPCM           (0x0011)
#define WAVE_FORMAT_DIGISTD             (0x0015)
#define WAVE_FORMAT_DIGIFIX             (0x0016)
#define WAVE_FORMAT_YAMAHA_ADPCM        (0x0020)
#define WAVE_FORMAT_DSP_TRUESPEECH      (0x0022)
#define WAVE_FORMAT_GSM610              (0x0031)
#define WAVE_FORMAT_MP3                 (0x0055)
#define IBM_FORMAT_MULAW                (0x0101)
#define IBM_FORMAT_ALAW                 (0x0102)
#define IBM_FORMAT_ADPCM                (0x0103)
#define WAVE_FORMAT_AC3                 (0x2000)
#endif

avi_t* AVI_open_output_file(char * filename);
void AVI_set_video(avi_t *AVI, int width, int height, double fps, char *compressor);
void AVI_set_audio(avi_t *AVI, int channels, long rate, int bits, int format, long mp3rate);
int  AVI_write_frame(avi_t *AVI, char *data, long bytes, int keyframe);
int  AVI_dup_frame(avi_t *AVI);
int  AVI_write_audio(avi_t *AVI, char *data, long bytes);
int  AVI_append_audio(avi_t *AVI, char *data, long bytes);
u64 AVI_bytes_remain(avi_t *AVI);
int  AVI_close(avi_t *AVI);
u64 AVI_bytes_written(avi_t *AVI);

avi_t *AVI_open_input_file(char *filename, int getIndex);
avi_t *AVI_open_input_indexfile(char *filename, int getIndex, char *indexfile);
avi_t *AVI_open_fd(FILE *fd, int getIndex);
avi_t *AVI_open_indexfd(FILE *fd, int getIndex, char *indexfile);
int avi_parse_input_file(avi_t *AVI, int getIndex);
int avi_parse_index_from_file(avi_t *AVI, char *filename);
long AVI_audio_mp3rate(avi_t *AVI);
long AVI_audio_padrate(avi_t *AVI);
long AVI_video_frames(avi_t *AVI);
int  AVI_video_width(avi_t *AVI);
int  AVI_video_height(avi_t *AVI);
double AVI_frame_rate(avi_t *AVI);
char* AVI_video_compressor(avi_t *AVI);

int  AVI_audio_channels(avi_t *AVI);
int  AVI_audio_bits(avi_t *AVI);
int  AVI_audio_format(avi_t *AVI);
long AVI_audio_rate(avi_t *AVI);
u64 AVI_audio_bytes(avi_t *AVI);
long AVI_audio_chunks(avi_t *AVI);
int  AVI_can_read_audio(avi_t *AVI);

long AVI_max_video_chunk(avi_t *AVI);

long AVI_frame_size(avi_t *AVI, long frame);
long AVI_audio_size(avi_t *AVI, long frame);
int  AVI_seek_start(avi_t *AVI);
int  AVI_set_video_position(avi_t *AVI, long frame);
u64 AVI_get_video_position(avi_t *AVI, long frame);
long AVI_read_frame(avi_t *AVI, char *vidbuf, int *keyframe);

int  AVI_set_audio_position(avi_t *AVI, long byte);
int  AVI_set_audio_bitrate(avi_t *AVI, long bitrate);

long AVI_get_audio_position_index(avi_t *AVI);
int  AVI_set_audio_position_index(avi_t *AVI, long indexpos);

long AVI_read_audio(avi_t *AVI, char *audbuf, long bytes, int *continuous);

int  AVI_read_data(avi_t *AVI, char *vidbuf, long max_vidbuf,
                               char *audbuf, long max_audbuf,
                               long *len);

int AVI_scan(char *name);
int AVI_dump(char *name, int mode);

char *AVI_codec2str(short cc);
int AVI_file_check(char *import_file);

void AVI_info(avi_t *avifile);
u64 AVI_max_size(void);
int avi_update_header(avi_t *AVI);

int AVI_set_audio_track(avi_t *AVI, u32 track);
int AVI_get_audio_track(avi_t *AVI);
int AVI_audio_tracks(avi_t *AVI);

void AVI_set_audio_vbr(avi_t *AVI, long is_vbr);
long AVI_get_audio_vbr(avi_t *AVI);

void AVI_set_comment_fd(avi_t *AVI, int fd);
int  AVI_get_comment_fd(avi_t *AVI);

struct riff_struct 
{
  u8 id[4];   /* RIFF */
  u32 len;
  u8 wave_id[4]; /* WAVE */
};


struct chunk_struct 
{
	u8 id[4];
	u32 len;
};

struct common_struct 
{
	u16 wFormatTag;
	u16 wChannels;
	u32 dwSamplesPerSec;
	u32 dwAvgBytesPerSec;
	u16 wBlockAlign;
	u16 wBitsPerSample;  /* Only for PCM */
};

struct wave_header 
{
	struct riff_struct   riff;
	struct chunk_struct  format;
	struct common_struct common;
	struct chunk_struct  data;
};

// Simple WAV IO
int AVI_read_wave_header( int fd, struct wave_header * wave );
int AVI_write_wave_header( int fd, const struct wave_header * wave );
size_t AVI_read_wave_pcm_data( int fd, void * buffer, size_t buflen );
size_t AVI_write_wave_pcm_data( int fd, const void * buffer, size_t buflen );


struct AVIStreamHeader {
  long  fccType;
  long  fccHandler;
  long  dwFlags;
  long  dwPriority;
  long  dwInitialFrames;
  long  dwScale;
  long  dwRate;
  long  dwStart;
  long  dwLength;
  long  dwSuggestedBufferSize;
  long  dwQuality;
  long  dwSampleSize;
};

#endif /*GPAC_DISABLE_AVILIB*/

#endif	/*_GF_AVILIB_H_*/

