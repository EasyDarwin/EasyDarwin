/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2002             *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: code raw [Vorbis] packets into framed OggSquish stream and
           decode Ogg streams back into raw packets

 note: The CRC code is directly derived from public domain code by
 Ross Williams (ross@guest.adelaide.edu.au).  See docs/framing.html
 for details.

 ********************************************************************/


#ifndef _GF_OGG_H_
#define _GF_OGG_H_

#include <gpac/tools.h>

#ifndef GPAC_DISABLE_OGG

/*DON'T CLASH WITH OFFICIAL OGG IF ALREADY INCLUDED*/
#ifndef _OGG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  s32 endbyte;
  s32  endbit;

  unsigned char *buffer;
  unsigned char *ptr;
  s32 storage;
} oggpack_buffer;

/* ogg_page is used to encapsulate the data in one Ogg bitstream page *****/

typedef struct {
  unsigned char *header;
  s32 header_len;
  unsigned char *body;
  s32 body_len;
} ogg_page;

/* ogg_stream_state contains the current encode/decode state of a logical
   Ogg bitstream **********************************************************/

typedef struct {
  unsigned char   *body_data;    /* bytes from packet bodies */
  s32    body_storage;          /* storage elements allocated */
  s32    body_fill;             /* elements stored; fill mark */
  s32    body_returned;         /* elements of fill returned */


  s32     *lacing_vals;      /* The values that will go to the segment table */
  s64 *granule_vals; /* granulepos values for headers. Not compact
				this way, but it is simple coupled to the
				lacing fifo */
  s32    lacing_storage;
  s32    lacing_fill;
  s32    lacing_packet;
  s32    lacing_returned;

  unsigned char    header[282];      /* working space for header encode */
  s32              header_fill;

  s32     e_o_s;          /* set when we have buffered the last packet in the
                             logical bitstream */
  s32     b_o_s;          /* set after we've written the initial page
                             of a logical bitstream */
  s32    serialno;
  s32    pageno;
  s64  packetno;      /* sequence number for decode; the framing
                             knows where there's a hole in the data,
                             but we need coupling so that the codec
                             (which is in a seperate abstraction
                             layer) also knows about the gap */
  s64   granulepos;

} ogg_stream_state;

/* ogg_packet is used to encapsulate the data and metadata belonging
   to a single raw Ogg/Vorbis packet *************************************/

typedef struct {
  unsigned char *packet;
  s32  bytes;
  s32  b_o_s;
  s32  e_o_s;

  s64  granulepos;
  
  s64  packetno;     /* sequence number for decode; the framing
				knows where there's a hole in the data,
				but we need coupling so that the codec
				(which is in a seperate abstraction
				layer) also knows about the gap */
} ogg_packet;

typedef struct {
  unsigned char *data;
  s32 storage;
  s32 fill;
  s32 returned;

  s32 unsynced;
  s32 headerbytes;
  s32 bodybytes;
} ogg_sync_state;



/* Ogg BITSTREAM PRIMITIVES: bitstream ************************/

void oggpack_writeinit(oggpack_buffer *b);
void oggpack_writetrunc(oggpack_buffer *b,s32 bits);
void oggpack_writealign(oggpack_buffer *b);
void oggpack_writecopy(oggpack_buffer *b,void *source,s32 bits);
void oggpack_reset(oggpack_buffer *b);
void oggpack_writeclear(oggpack_buffer *b);
void oggpack_readinit(oggpack_buffer *b,unsigned char *buf,s32 bytes);
void oggpack_write(oggpack_buffer *b,u32 value,s32 bits);
s32 oggpack_look(oggpack_buffer *b,s32 bits);
s32 oggpack_look1(oggpack_buffer *b);
void oggpack_adv(oggpack_buffer *b,s32 bits);
void oggpack_adv1(oggpack_buffer *b);
s32 oggpack_read(oggpack_buffer *b,s32 bits);
s32 oggpack_read1(oggpack_buffer *b);
s32 oggpack_bytes(oggpack_buffer *b);
s32 oggpack_bits(oggpack_buffer *b);
unsigned char *oggpack_get_buffer(oggpack_buffer *b);

void oggpackB_writeinit(oggpack_buffer *b);
void oggpackB_writetrunc(oggpack_buffer *b,s32 bits);
void oggpackB_writealign(oggpack_buffer *b);
void oggpackB_writecopy(oggpack_buffer *b,void *source,s32 bits);
void oggpackB_reset(oggpack_buffer *b);
void oggpackB_writeclear(oggpack_buffer *b);
void oggpackB_readinit(oggpack_buffer *b,unsigned char *buf,s32 bytes);
void oggpackB_write(oggpack_buffer *b,u32 value,s32 bits);
s32 oggpackB_look(oggpack_buffer *b,s32 bits);
s32 oggpackB_look1(oggpack_buffer *b);
void oggpackB_adv(oggpack_buffer *b,s32 bits);
void oggpackB_adv1(oggpack_buffer *b);
s32 oggpackB_read(oggpack_buffer *b,s32 bits);
s32 oggpackB_read1(oggpack_buffer *b);
s32 oggpackB_bytes(oggpack_buffer *b);
s32 oggpackB_bits(oggpack_buffer *b);
unsigned char *oggpackB_get_buffer(oggpack_buffer *b);

/* Ogg BITSTREAM PRIMITIVES: encoding **************************/

s32 ogg_stream_packetin(ogg_stream_state *os, ogg_packet *op);
s32 ogg_stream_pageout(ogg_stream_state *os, ogg_page *og);
s32 ogg_stream_flush(ogg_stream_state *os, ogg_page *og);

/* Ogg BITSTREAM PRIMITIVES: decoding **************************/

s32 ogg_sync_init(ogg_sync_state *oy);
s32 ogg_sync_clear(ogg_sync_state *oy);
s32 ogg_sync_reset(ogg_sync_state *oy);
s32 ogg_sync_destroy(ogg_sync_state *oy);

char *ogg_sync_buffer(ogg_sync_state *oy, s32 size);
s32 ogg_sync_wrote(ogg_sync_state *oy, s32 bytes);
s32 ogg_sync_pageseek(ogg_sync_state *oy,ogg_page *og);
s32 ogg_sync_pageout(ogg_sync_state *oy, ogg_page *og);
s32 ogg_stream_pagein(ogg_stream_state *os, ogg_page *og);
s32 ogg_stream_packetout(ogg_stream_state *os,ogg_packet *op);
s32 ogg_stream_packetpeek(ogg_stream_state *os,ogg_packet *op);

/* Ogg BITSTREAM PRIMITIVES: general ***************************/

s32 ogg_stream_init(ogg_stream_state *os,s32 serialno);
s32 ogg_stream_clear(ogg_stream_state *os);
s32 ogg_stream_reset(ogg_stream_state *os);
s32 ogg_stream_reset_serialno(ogg_stream_state *os,s32 serialno);
s32 ogg_stream_destroy(ogg_stream_state *os);
s32 ogg_stream_eos(ogg_stream_state *os);
void ogg_page_checksum_set(ogg_page *og);
s32 ogg_page_version(ogg_page *og);
s32 ogg_page_continued(ogg_page *og);
s32 ogg_page_bos(ogg_page *og);
s32 ogg_page_eos(ogg_page *og);
s64 ogg_page_granulepos(ogg_page *og);
s32 ogg_page_serialno(ogg_page *og);
s32 ogg_page_pageno(ogg_page *og);
s32 ogg_page_packets(ogg_page *og);

void ogg_packet_clear(ogg_packet *op);


#ifdef __cplusplus
}
#endif


#endif	/*_OGG_H*/

#endif /*GPAC_DISABLE_OGG*/

#endif	/*_GF_OGG_H_*/

