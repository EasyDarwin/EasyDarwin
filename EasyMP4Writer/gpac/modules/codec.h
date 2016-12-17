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



#ifndef _GF_MODULE_CODEC_H_
#define _GF_MODULE_CODEC_H_


#include <gpac/module.h>
#include <gpac/mpeg4_odf.h>
#include <gpac/color.h>

#ifdef __cplusplus
extern "C" {
#endif

/*multimedia processing levels*/
enum
{
	/*normal, full processing*/
	GF_CODEC_LEVEL_NORMAL,
	/*codec is late, should scale down processing*/
	GF_CODEC_LEVEL_LATE,
	/*codec is very late, should turn off post-processing, even drop*/
	GF_CODEC_LEVEL_VERY_LATE,
	/*input frames are already late before decoding*/
	GF_CODEC_LEVEL_DROP,
	/*this is a special level indicating that a seek is happening (decode but no dispatch)
	it is set dynamically*/
	GF_CODEC_LEVEL_SEEK
};

/*the structure for capabilities*/
typedef struct 
{
	/*cap code cf below*/
	u16 CapCode;
	union {
		u32 valueInt;
		Float valueFloat;
	} cap;
} GF_CodecCapability;


/*
			all codecs capabilities
*/

enum
{
	/*size of a single composition unit */
	GF_CODEC_OUTPUT_SIZE =	0x01,
	/*resilency: if packets are lost within an AU, resilience means the AU won't be discarded and the codec
	will try to decode */
	GF_CODEC_RESILIENT,
	/*critical level of composition memory - if below, media management for the object */
	GF_CODEC_BUFFER_MIN,
	/*maximum size in CU of composition memory */
	GF_CODEC_BUFFER_MAX,
	/*flags that all AUs should be discarded till next RAP (needed in case RAPs are not carried by the transport
	protocol */
	GF_CODEC_WAIT_RAP,
	/*number of padding bytes needed - if the decoder needs padding input cannot be pulled and data is duplicated*/
	GF_CODEC_PADDING_BYTES,
	/*codecs can be threaded at will - by default a single thread is used for all decoders and priority is handled
	by the app, but a codec can configure itself to run in a dedicated thread*/
	GF_CODEC_WANTS_THREAD,

	/*video width and height and horizontal pitch (in YV12 we assume half Y pitch for U and V planes) */
	GF_CODEC_WIDTH,
	GF_CODEC_HEIGHT,
	GF_CODEC_STRIDE,
	GF_CODEC_FPS,
	/*Pixel Aspect Ratio, expressed as (par.num<<16) | par.den*/
	GF_CODEC_PAR,
	/*video color mode - color modes are defined in constants.h*/
	GF_CODEC_PIXEL_FORMAT,
	/*isgnal decoder performs frame re-ordering in temporal scalability*/
	GF_CODEC_REORDER,
	
	/*Audio sample rate*/
	GF_CODEC_SAMPLERATE,
	/*Audio num channels*/
	GF_CODEC_NB_CHAN,
	/*Audio bps*/
	GF_CODEC_BITS_PER_SAMPLE,
	/*audio frame format*/
	GF_CODEC_CHANNEL_CONFIG,
	/*this is only used for audio in case transport mapping relies on sampleRate (RTP)
	gets the CU duration in samplerate unit (type: int) */
	GF_CODEC_CU_DURATION,
	/*queries whether data is RAW (directly dispatched to CompositionMemory) or not*/
	GF_CODEC_RAW_MEDIA,

	/*This is only called on scene decoders to signal that potential overlay scene should be 
	showed (cap.valueINT=1) or hidden (cap.valueINT=0). Currently only used with SetCap*/
	GF_CODEC_SHOW_SCENE,
	/*This is only called on scene decoders, GetCap only. If the decoder may continue modifying the scene once the last AU is received,
	it must set cap.valueINT to 1 (typically, text stream decoder will hold the scene for a given duration
	after the last AU). Otherwise the decoder will be stopped and ask to remove any extra scene being displayed*/
	GF_CODEC_MEDIA_NOT_OVER,

	/*switches up or down media quality for scalable coding*/
	GF_CODEC_MEDIA_SWITCH_QUALITY,

	/*special cap indicating the codec should abort processing as soon as possible because it is about to be destroyed*/
	GF_CODEC_ABORT
};


enum
{
	/*stream format is NOT supported by this codec*/
	GF_CODEC_NOT_SUPPORTED = 0,
	/*stream type (eg audio, video) is supported by this codec*/
	GF_CODEC_STREAM_TYPE_SUPPORTED = 1,
	/*stream format may be (partially) supported by this codec*/
	GF_CODEC_MAYBE_SUPPORTED = 127,
	/*stream format is supported by this codec*/
	GF_CODEC_SUPPORTED = 255,
};

	/* Generic interface used by both media decoders and scene decoders
@AttachStream:
	Add a Stream to the codec. If DependsOnESID is NULL, the stream is a base layer
	UpStream means that the decoder should send feedback on this channel. 
	WARNING: Feedback format is not standardized by MPEG
	the same API is used for both encoder and decoder (decSpecInfo is ignored
	for an encoder) 
@DetachStream:
	Remove stream
@GetCapabilities:
	Get the desired capability given its code
@SetCapabilities
	Set the desired capability given its code if possible
	if the codec does not support the request capability, return GF_NOT_SUPPORTED
@CanHandleStream
	Can module handle this codec? Return one of GF_CODEC_NOT_SUPPORTED, GF_CODEC_MAYBE_SUPPORTED or GF_CODEC_SUPPORTED
	esd is provided for more advanced inspection ( eg MPEG4 audio/visual where a bunch of codecs are defined with same objectType). If esd is NULL, only 
	decoder type is checked (audio or video), not codec type
@GetDecoderName
	returns codec name - only called once the stream is successfully attached
@privateStack
	user defined.
*/

#define GF_CODEC_BASE_INTERFACE(IFCE_NAME)		\
	GF_DECL_MODULE_INTERFACE	\
	GF_Err (*AttachStream)(IFCE_NAME, GF_ESD *esd);\
	GF_Err (*DetachStream)(IFCE_NAME, u16 ES_ID);\
	GF_Err (*GetCapabilities)(IFCE_NAME, GF_CodecCapability *capability);\
	GF_Err (*SetCapabilities)(IFCE_NAME, GF_CodecCapability capability);\
	u32 (*CanHandleStream)(IFCE_NAME, u32 StreamType, GF_ESD *esd, u8 ProfileLevelIndication);\
	const char *(*GetName)(IFCE_NAME);\
	void *privateStack;	\


typedef struct _basedecoder
{
	GF_CODEC_BASE_INTERFACE(struct _basedecoder *)
} GF_BaseDecoder;

/*interface name and version for media decoder */
#define GF_MEDIA_DECODER_INTERFACE		GF_4CC('G', 'M', 'D', '2')

/*the media module interface. A media module MUST be implemented in synchronous mode as time 
and resources management is done by the terminal*/
typedef struct _mediadecoder
{
	GF_CODEC_BASE_INTERFACE(struct _basedecoder *)

	/*Process the media data in inAU. 
	@inBuffer, inBufferLength: encoded input data (complete framing of encoded data)
	@ES_ID: stream this data belongs too (scalable object)
	@outBuffer, outBufferLength: allocated data for decoding - if outBufferLength is not enough
		you must set the size in outBufferLength and GF_BUFFER_TOO_SMALL 

	@PaddingBits is the padding at the end of the buffer (some codecs need this info)
	@mmlevel: speed indicator for the decoding - cf above for values*/
	GF_Err (*ProcessData)(struct _mediadecoder *, 
			char *inBuffer, u32 inBufferLength,
			u16 ES_ID,
			char *outBuffer, u32 *outBufferLength,
			u8 PaddingBits, u32 mmlevel);
} GF_MediaDecoder;


/*
				WARNING - DO NOT MODIFY THE POSITION OF ProcessData IN SCENE OR NODE DECODER, AS THE BOTH STRUCTURES
		ARE TYPE_CASTED BY THE TERMINAL WHEN CALLING ProcessData
*/
typedef struct _scene *LPSCENE;

/*interface name and version for scene decoder */
#define GF_SCENE_DECODER_INTERFACE		GF_4CC('G', 'S', 'D', '3')

typedef struct _scenedecoder
{
	GF_CODEC_BASE_INTERFACE(struct _basedecoder *)
	
	/*Process the scene data in inAU. 
	@inBuffer, inBufferLength: encoded input data (complete framing of encoded data)
	@ES_ID: stream this data belongs too (scalable object)
	@AU_Time: specifies the current AU time. This is usually unused, however is needed for decoder
	handling the scene graph without input data (cf below). In this case the buffer passed is always NULL and the AU
	time caries the time of the scene (or of the stream object attached to the scene decoder, cf below)
	@mmlevel: speed indicator for the decoding - cf above for values*/
	GF_Err (*ProcessData)(struct _scenedecoder *, const char *inBuffer, u32 inBufferLength,
					u16 ES_ID, u32 AU_Time, u32 mmlevel);


	/*attaches scene to the decoder - a scene may be attached to several decoders of several types
	(BIFS or others scene dec, ressource decoders (OD), etc. 
	is: inline scene owning graph (and not just graph), defined in intern/terminal_dev.h. With inline scene
	the complete terminal is exposed so there's pretty much everything doable in a scene decoder
	@is_scene_root: set to true if this decoder is the root of the scene, false otherwise (either another decoder
	or a re-entrant call, cf below)
	This is called once upon creation of the decoder (several times if re-entrant)
	*/
	GF_Err (*AttachScene)(struct _scenedecoder *, LPSCENE scene, Bool is_scene_root);
	/*releases scene. If the decoder manages nodes / resources in the scene, 
	THESE MUST BE DESTROYED. May be NULL if decoder doesn't manage nodes but only create them (like BIFS, OD) and
	doesn't have to be instructed the scene is about to be resumed
	This is called each time the scene is about to be reseted (eg, seek and destroy)
	*/
	GF_Err (*ReleaseScene)(struct _scenedecoder *);
} GF_SceneDecoder;


/*interface name and version for node decoder mainly used by AFX*/
#define GF_NODE_DECODER_INTERFACE		GF_4CC('G', 'N', 'D', '3')

typedef struct _base_node *LPNODE;

typedef struct _nodedecoder
{
	GF_CODEC_BASE_INTERFACE(struct _basedecoder *)
	
	/*Process the node data in inAU. 
	@inBuffer, inBufferLength: encoded input data (complete framing of encoded data)
	@ES_ID: stream this data belongs too (scalable object)
	@AU_Time: specifies the current AU time. This is usually unused, however is needed for decoder
	handling the scene graph without input data (cf below). In this case the buffer passed is always NULL and the AU
	time caries the time of the scene (or of the stream object attached to the scene decoder, cf below)
	@mmlevel: speed indicator for the decoding - cf above for values*/
	GF_Err (*ProcessData)(struct _nodedecoder *, const char *inBuffer, u32 inBufferLength,
					u16 ES_ID, u32 AU_Time, u32 mmlevel);

	/*attaches node to the decoder - currently only one node is only attached to a single decoder*/
	GF_Err (*AttachNode)(struct _nodedecoder *, LPNODE node);
} GF_NodeDecoder;



/*interface name and version for scene decoder */
#define GF_INPUT_DEVICE_INTERFACE		GF_4CC('G', 'I', 'D', '1')

typedef struct __input_device
{
	/* interface declaration*/
	GF_DECL_MODULE_INTERFACE

	Bool (*RegisterDevice)(struct __input_device *, const char *urn, GF_BitStream *dsi, void (*AddField)(struct __input_device *_this, u32 fieldType, const char *name));
	void (*Start)(struct __input_device *);
	void (*Stop)(struct __input_device *);

	void *udta;

	/*this is set upon loading and shall not be modified*/
	GF_BaseDecoder *input_decoder;
	void (*DispatchFrame)(struct __input_device *, u8 *data, u32 data_len);
} GF_InputSensorDevice;




/*interface name and version for media decoder */
#define GF_PRIVATE_MEDIA_DECODER_INTERFACE		GF_4CC('G', 'P', 'M', '2')

/*the media module interface. A media module MUST be implemented in synchronous mode as time 
and resources management is done by the terminal*/
typedef struct _private_mediadecoder
{
	GF_CODEC_BASE_INTERFACE(struct _basedecoder *)

	/*Control media decoder. 
	@mute: set mute or not
	@x, y, w, h: video output position in screen coordinate
	*/
	GF_Err (*Control)(struct _private_mediadecoder *, Bool mute, GF_Window *src_rect, GF_Window *dst_rect);
} GF_PrivateMediaDecoder;

#ifdef __cplusplus
}
#endif

#endif	/*_GF_MODULE_CODEC_H_*/

