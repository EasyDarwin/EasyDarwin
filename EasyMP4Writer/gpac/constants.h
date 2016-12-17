/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005
 *					All rights reserved
 *
 *  This file is part of GPAC / exported constants
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

#ifndef _GF_CONSTANTS_H_
#define _GF_CONSTANTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/tools.h>

/*! \addtogroup cst_grp constants
 *	\brief Constants used within GPAC
 *
 *	This section documents some constants used in the GPAC framework which are not related to
 *	any specific sub-project.
 *	\ingroup utils_grp
 *	@{
 */


/*!
 *	\brief Supported media stream types
 *	\hideinitializer
 *
 * Supported media stream types for media objects.
*/
enum
{
	/*!MPEG-4 Object Descriptor Stream*/
	GF_STREAM_OD		= 0x01,
	/*!MPEG-4 Object Clock Reference Stream*/
	GF_STREAM_OCR		= 0x02,
	/*!MPEG-4 Scene Description Stream*/
	GF_STREAM_SCENE		= 0x03,
	/*!Visual Stream (Video, Image or MPEG-4 SNHC Tools)*/
	GF_STREAM_VISUAL	= 0x04,
	/*!Audio Stream (Audio, MPEG-4 Structured-Audio Tools)*/
	GF_STREAM_AUDIO		= 0x05,
	/*!MPEG-7 Description Stream*/
	GF_STREAM_MPEG7		= 0x06,
	/*!MPEG-4 Intellectual Property Management and Protection Stream*/
	GF_STREAM_IPMP		= 0x07,
	/*!MPEG-4 Object Content Information Stream*/
	GF_STREAM_OCI		= 0x08,
	/*!MPEG-4 MPEGlet Stream*/
	GF_STREAM_MPEGJ		= 0x09,
	/*!MPEG-4 User Interaction Stream*/
	GF_STREAM_INTERACT	= 0x0A,
	/*!MPEG-4 IPMP Tool Stream*/
	GF_STREAM_IPMP_TOOL	= 0x0B,
	/*!MPEG-4 Font Data Stream*/
	GF_STREAM_FONT		= 0x0C,
	/*!MPEG-4 Streaming Text Stream*/
	GF_STREAM_TEXT		= 0x0D,
	/*!Nero Digital Subpicture Stream*/
	GF_STREAM_ND_SUBPIC = 0x38,

	/*GPAC internal stream types*/


	/*!GPAC Private Scene streams\n
	*\n\note
	*this stream type (MPEG-4 user-private) is reserved for streams only used to create a scene decoder
	*handling the scene without input streams, as is the case for file readers (BT/VRML/XML..).\n
	*The decoderSpecificInfo carried is as follows:
	 \code
		u32 file_size:	total file size
		char file_name[dsi_size - sizeof(u32)]: local file name.
		\n\note: File may be a cache file, it is the decoder responsability to check if the file is completely
		downloaded before parsing if needed.
	 \endcode
	*The inBufferLength param for decoders using these streams is the stream clock in ms (no input data is given).\n
	*The "dummy_in" module is available to generate these streams for common files, and also takes care of proper
	clock init in case of seeking.\n
	*This is a reentrant stream type: if any media object with this streamtype also exist in the scene, they will be
	*attached to the scene decoder (except when a new inline scene is detected, in which case a new decoder will
	*be created). This allows for animation/sprite usage along with the systems timing/stream management.\n
	*\n
	*the objectTypeIndication currently in use for these streams are documented below\n
	*/
	GF_STREAM_PRIVATE_SCENE	= 0x20,

	/*!GPAC Private Media streams\n
	*\n\note
	*this stream type (MPEG-4 user-private) is reserved for media streams bypassing GPAC for decoding
	and composition. The media decoder is only in charge of repositioning the video output, and the compositor will 
	draw an empty rectangle if using alpha composition

	*The decoderSpecificInfo carried only contains an opaque pointer in the data field, which depends on the underlying InputServce provider

	*the objectTypeIndication currently in use for these streams are documented below\n
	*/
	GF_STREAM_PRIVATE_MEDIA	= 0x21,

	/*used internally to signal the the OTI carries a 4CC code, typically media subtype (stsd entry in file format)*/
	GF_STREAM_4CC		= 0xF0
};


/*!
 *	Media Object types
 *
 *	This type provides a hint to network modules which may have to generate an service descriptor on the fly.
 *	They occur only if objects/services used in the scene are not referenced through ObjectDescriptors (MPEG-4)
 *	but direct through URL
*/
enum
{
	/*!service descriptor expected is of undefined type. This should be treated like GF_MEDIA_OBJECT_SCENE*/
	GF_MEDIA_OBJECT_UNDEF = 0,
	/*!service descriptor expected is of SCENE type and shall contain a scene stream and OD one if needed*/
	GF_MEDIA_OBJECT_SCENE,
	/*!service descriptor expected is of SCENE UPDATES type (animation streams)*/
	GF_MEDIA_OBJECT_UPDATES,
	/*!service descriptor expected is of VISUAL type*/
	GF_MEDIA_OBJECT_VIDEO,
	/*!service descriptor expected is of AUDIO type*/
	GF_MEDIA_OBJECT_AUDIO,
	/*!service descriptor expected is of TEXT type (3GPP/MPEG4)*/
	GF_MEDIA_OBJECT_TEXT,
	/*!service descriptor expected is of UserInteraction type (MPEG-4 InputSensor)*/
	GF_MEDIA_OBJECT_INTERACT
};

/*! All Media Objects inserted through URLs and not MPEG-4 OD Framework use this ODID*/
#define GF_MEDIA_EXTERNAL_ID		1050


/*!
 * \brief Pixel Formats
 *
 *	Supported pixel formats for everything using video
*/
typedef enum
{
	/*!8 bit GREY */
	GF_PIXEL_GREYSCALE	=	GF_4CC('G','R','E','Y'),
	/*!16 bit greyscale*/
	GF_PIXEL_ALPHAGREY	=	GF_4CC('G','R','A','L'),
	/*!12 bit RGB on 16 bits (4096 colors)*/
	GF_PIXEL_RGB_444	=	GF_4CC('R','4','4','4'),
	/*!15 bit RGB*/
	GF_PIXEL_RGB_555	=	GF_4CC('R','5','5','5'),
	/*!16 bit RGB*/
	GF_PIXEL_RGB_565	=	GF_4CC('R','5','6','5'),
	/*!24 bit RGB*/
	GF_PIXEL_RGB_24		=	GF_4CC('R','G','B','3'),
	/*!24 bit BGR*/
	GF_PIXEL_BGR_24		=	GF_4CC('B','G','R','3'),
	/*!32 bit RGB. Component ordering in bytes is B-G-R-X.*/
	GF_PIXEL_RGB_32		=	GF_4CC('R','G','B','4'),
	/*!32 bit BGR. Component ordering in bytes is R-G-B-X.*/
	GF_PIXEL_BGR_32		=	GF_4CC('B','G','R','4'),

	/*!32 bit ARGB. Component ordering in bytes is B-G-R-A.*/
	GF_PIXEL_ARGB		=	GF_4CC('A','R','G','B'),
	/*!32 bit RGBA (openGL like). Component ordering in bytes is R-G-B-A.*/
	GF_PIXEL_RGBA		=	GF_4CC('R','G','B', 'A'),
	/*!RGB24 + depth plane. Component ordering in bytes is R-G-B-D.*/
    GF_PIXEL_RGBD		=	GF_4CC('R', 'G', 'B', 'D'),
	/*!RGB24 + depth plane (7 lower bits) + shape mask. Component ordering in bytes is R-G-B-(S+D).*/
    GF_PIXEL_RGBDS		=	GF_4CC('3', 'C', 'D', 'S'),
	/*!Stereo RGB24 */
    GF_PIXEL_RGBS		=	GF_4CC('R', 'G', 'B', 'S'),
	/*!Stereo RGBA. Component ordering in bytes is R-G-B-A. */
    GF_PIXEL_RGBAS		=	GF_4CC('R', 'G', 'A', 'S'),

	/*internal format for OpenGL using pachek RGB 24 bit plus planaer depth plane at the end of the image*/
	GF_PIXEL_RGB_24_DEPTH = GF_4CC('R', 'G', 'B', 'd'),

	/*!YUV packed format*/
	GF_PIXEL_YUY2		=	GF_4CC('Y','U','Y','2'),
	/*!YUV packed format*/
	GF_PIXEL_YVYU		=	GF_4CC('Y','V','Y','U'),
	/*!YUV packed format*/
	GF_PIXEL_UYVY		=	GF_4CC('U','Y','V','Y'),
	/*!YUV packed format*/
	GF_PIXEL_VYUY		=	GF_4CC('V','Y','U','Y'),
	/*!YUV packed format*/
	GF_PIXEL_Y422		=	GF_4CC('Y','4','2','2'),
	/*!YUV packed format*/
	GF_PIXEL_UYNV		=	GF_4CC('U','Y','N','V'),
	/*!YUV packed format*/
	GF_PIXEL_YUNV		=	GF_4CC('Y','U','N','V'),
	/*!YUV packed format*/
	GF_PIXEL_V422		=	GF_4CC('V','4','2','2'),

	/*!YUV planar format*/
	GF_PIXEL_YV12		=	GF_4CC('Y','V','1','2'),
	/*!YUV planar format*/
	GF_PIXEL_IYUV		=	GF_4CC('I','Y','U','V'),
	/*!YUV planar format*/
	GF_PIXEL_I420		=	GF_4CC('I','4','2','0'),

	/*!YV12 + Alpha plane*/
	GF_PIXEL_YUVA		=	GF_4CC('Y', 'U', 'V', 'A'),

	/*!YV12 + Depth plane*/
	GF_PIXEL_YUVD		=	GF_4CC('Y', 'U', 'V', 'D')

} GF_PixelFormat;


/*!
 * \brief Scene ObjectTypeIndication Formats
 *
 *	Supported ObjectTypeIndication for scene description streams. *_FILE_* are only used with private scene streams
 * and only carry the file name for the scene. Other internal stream types can be used in a real streaming environment
*/
enum
{
	/*!OTI for BIFS v1*/
	GPAC_OTI_SCENE_BIFS = 0x01,
	/*!OTI for OD v1*/
	GPAC_OTI_OD_V1 = 0x01,
	/*!OTI for BIFS v2*/
	GPAC_OTI_SCENE_BIFS_V2 = 0x02,
	/*!OTI for OD v2*/
	GPAC_OTI_OD_V2 = 0x02,
	/*!OTI for BIFS InputSensor streams*/
	GPAC_OTI_SCENE_INTERACT = 0x03,
	/*!OTI for streams with extended BIFS config*/
	GPAC_OTI_SCENE_BIFS_EXTENDED = 0x04,
	/*!OTI for AFX streams with AFXConfig*/
	GPAC_OTI_SCENE_AFX = 0x05,
    /*!OTI for Font data streams */
	GPAC_OTI_FONT = 0x06,
    /*!OTI for synthesized texture streams */
	GPAC_OTI_SCENE_SYNTHESIZED_TEXTURE = 0x07,
    /*!OTI for streaming text streams */
	GPAC_OTI_TEXT_MPEG4 = 0x08,
	/*!OTI for LASeR streams*/
	GPAC_OTI_SCENE_LASER = 0x09,
	/*!OTI for SAF streams*/
	GPAC_OTI_SCENE_SAF = 0x0A,

	/*!OTI for MPEG-4 Video Part 2 streams*/
	GPAC_OTI_VIDEO_MPEG4_PART2 = 0x20,
	/*!OTI for MPEG-4 Video Part 10 (H.264 | AVC ) streams*/
	GPAC_OTI_VIDEO_AVC = 0x21,
	/*!OTI for AVC Parameter sets streams*/
	GPAC_OTI_VIDEO_AVC_PS = 0x22,
	/*!OTI for MPEG-4 AAC streams*/
    GPAC_OTI_AUDIO_AAC_MPEG4 = 0x40,

	/*!OTI for MPEG-2 Visual Simple Profile streams*/
    GPAC_OTI_VIDEO_MPEG2_SIMPLE = 0x60,
	/*!OTI for MPEG-2 Visual Main Profile streams*/
    GPAC_OTI_VIDEO_MPEG2_MAIN = 0x61,
	/*!OTI for MPEG-2 Visual SNR Profile streams*/
    GPAC_OTI_VIDEO_MPEG2_SNR = 0x62,
	/*!OTI for MPEG-2 Visual SNR Profile streams*/
    GPAC_OTI_VIDEO_MPEG2_SPATIAL = 0x63,
	/*!OTI for MPEG-2 Visual SNR Profile streams*/
    GPAC_OTI_VIDEO_MPEG2_HIGH = 0x64,
	/*!OTI for MPEG-2 Visual SNR Profile streams*/
    GPAC_OTI_VIDEO_MPEG2_422 = 0x65,


	/*!OTI for MPEG-2 AAC Main Profile streams*/
    GPAC_OTI_AUDIO_AAC_MPEG2_MP = 0x66,
	/*!OTI for MPEG-2 AAC Low Complexity Profile streams*/
    GPAC_OTI_AUDIO_AAC_MPEG2_LCP = 0x67,
	/*!OTI for MPEG-2 AAC Scaleable Sampling Rate Profile streams*/
    GPAC_OTI_AUDIO_AAC_MPEG2_SSRP = 0x68,
	/*!OTI for MPEG-2 Audio Part 3 streams*/
    GPAC_OTI_AUDIO_MPEG2_PART3 = 0x69,
	/*!OTI for MPEG-1 Video streams*/
    GPAC_OTI_VIDEO_MPEG1 = 0x6A,
	/*!OTI for MPEG-1 Audio streams*/
    GPAC_OTI_AUDIO_MPEG1 = 0x6B,
	/*!OTI for JPEG streams*/
    GPAC_OTI_IMAGE_JPEG = 0x6C,
	/*!OTI for PNG streams*/
    GPAC_OTI_IMAGE_PNG = 0x6D,
	/*!OTI for JPEG-2000 streams*/
    GPAC_OTI_IMAGE_JPEG_2000 = 0x6E,

/*!
 * \brief Extra ObjectTypeIndication
 *
 *	ObjectTypeIndication for media (audio/video) codecs not defined in MPEG-4. Since GPAC signals streams through MPEG-4 Descriptions,
 *	it needs extensions for non-MPEG-4 streams such as AMR, H263 , etc.\n
 *\note The decoder specific info for such streams is always carried encoded, with the following syntax:\n
 *	DSI Syntax for audio streams
 \code
 *	u32 codec_four_cc: the codec 4CC reg code / codec id for ffmpeg
 *	u32 sample_rate: sampling rate or 0 if unknown
 *	u16 nb_channels: num channels or 0 if unknown
 *	u16 frame_size: num audio samples per frame or 0 if unknown
 *	u8 nb_bits_per_sample: nb bits or 0 if unknown
 *	u8 num_frames_per_au: num audio frames per AU (used in 3GPP, max 15), 0 if unknown
 *	char *data: per-codec extensions till end of DSI bitstream
 \endcode
 \n
 *	DSI Syntax for video streams
 \code
 *	u32 codec_four_cc: the codec 4CC reg code  / codec id for ffmpeg
 *	u16 width: video width or 0 if unknown
 *	u16 height: video height or 0 if unknown
 *	char *data: per-codec extensions till end of DSI bitstream
 \endcode
*/
    GPAC_OTI_MEDIA_GENERIC = 0x80,
/*!
 * \brief FFMPEG ObjectTypeIndication
 *
 *	ObjectTypeIndication for FFMPEG codecs not defined in MPEG-4. FFMPEG uses the base GPAC_OTI_MEDIA_GENERIC specific info formats, and extends it as follows:
 \code
 *	u32 bit_rate: the stream rate or 0 if unknown
 *	u32 codec_tag: FFMPEG codec tag as defined in libavcodec
 *	char *data: codec extensions till end of DSI bitstream
 \endcode
 */
    GPAC_OTI_MEDIA_FFMPEG = 0x81,

    /*!OTI for EVRC Voice streams*/
    GPAC_OTI_AUDIO_EVRC_VOICE = 0xA0,
	/*!OTI for SMV Voice streams*/
    GPAC_OTI_AUDIO_SMV_VOICE = 0xA1,
	/*!OTI for 3GPP2 CMF streams*/
    GPAC_OTI_3GPP2_CMF = 0xA2,
	/*!OTI for SMPTE VC-1 Video streams*/
    GPAC_OTI_VIDEO_SMPTE_VC1 = 0xA3,
	/*!OTI for Dirac Video streams*/
    GPAC_OTI_VIDEO_DIRAC = 0xA4,
	/*!OTI for AC-3 audio streams*/
    GPAC_OTI_AUDIO_AC3 = 0xA5,
	/*!OTI for enhanced AC-3 audio streams*/
    GPAC_OTI_AUDIO_AC3_ENHANCED = 0xA6,
	/*!OTI for DRA audio streams*/
    GPAC_OTI_AUDIO_DRA = 0xA7,
	/*!OTI for ITU G719 audio streams*/
    GPAC_OTI_AUDIO_ITU_G719 = 0xA8,
	/*!OTI for DTS Coherent Acoustics audio streams*/
    GPAC_OTI_AUDIO_DTS_CA = 0xA9,
	/*!OTI for DTS-HD High Resolution audio streams*/
    GPAC_OTI_AUDIO_DTS_HD_HR = 0xAA,
	/*!OTI for DTS-HD Master audio streams*/
    GPAC_OTI_AUDIO_DTS_HD_MASTER = 0xAB,

    /*!OTI for dummy streams (dsi = file name) using the generic context loader (BIFS/VRML/SWF/...) - GPAC internal*/
	GPAC_OTI_PRIVATE_SCENE_GENERIC = 0xC0,
	/*!OTI for SVG dummy stream (dsi = file name) - GPAC internal*/
	GPAC_OTI_PRIVATE_SCENE_SVG = 0xC1,
	/*!OTI for LASeR/SAF+XML dummy stream (dsi = file name) - GPAC internal*/
	GPAC_OTI_PRIVATE_SCENE_LASER = 0xC2,
	/*!OTI for XBL dummy streams (dsi = file name) - GPAC internal*/
	GPAC_OTI_PRIVATE_SCENE_XBL = 0xC3,
	/*!OTI for EPG dummy streams (dsi = null) - GPAC internal*/
	GPAC_OTI_PRIVATE_SCENE_EPG = 0xC4,
	/*!OTI for WGT dummy streams (dsi = null) - GPAC internal*/
	GPAC_OTI_PRIVATE_SCENE_WGT = 0xC5,

	/*!OTI for streaming SVG - GPAC internal*/
	GPAC_OTI_SCENE_SVG = 0xD0,
	/*!OTI for streaming SVG + gz - GPAC internal*/
	GPAC_OTI_SCENE_SVG_GZ = 0xD1,
	/*!OTI for DIMS (dsi = 3GPP DIMS configuration) - GPAC internal*/
	GPAC_OTI_SCENE_DIMS = 0xD2,

/*!
 * \brief OGG ObjectTypeIndication
 *
 *	Object type indication for all OGG media. The DSI contains all intitialization ogg packets for the codec
 * and is formated as follows:\n
 *\code
	while (dsi_size) {
		bit(16) packet_size;
		char packet[packet_size];
		dsi_size -= packet_size;
	}\endcode
*/
    GPAC_OTI_MEDIA_OGG = 0xDD,
    GPAC_OTI_MEDIA_THEORA = 0xDF,

    GPAC_OTI_MEDIA_SUBPIC = 0xE0,

    /*!OTI for 13K Voice / QCELP audio streams*/
    GPAC_OTI_AUDIO_13K_VOICE = 0xE1,

	/*!OTI for LIBPLAYER private streams. The data pointer in the DSI is the libplayer handle object*/
    GPAC_OTI_PRIVATE_MEDIA_LIBPLAYER = 0xF1
};


/*!
 * \brief AFX Object Code
*/
enum
{
	/*!3D Mesh Compression*/
	GPAC_AFX_3DMC = 0x00,
	/*!Wavelet Subdivision Surface*/
	GPAC_AFX_WAVELET_SUBDIVISION = 0x01,
	/*!MeshGrid*/
	GPAC_AFX_MESHGRID = 0x02,
	/*!Coordinate Interpolator*/
	GPAC_AFX_COORDINATE_INTERPOLATOR = 0x03,
	/*!Orientation Interpolator*/
	GPAC_AFX_ORIENTATION_INTERPOLATOR = 0x04,
	/*!Position Interpolator*/
	GPAC_AFX_POSITION_INTERPOLATOR = 0x05,
	/*!Octree Image*/
	GPAC_AFX_OCTREE_IMAGE = 0x06,
	/*!BBA*/
	GPAC_AFX_BBA = 0x07,
	/*!PointTexture*/
	GPAC_AFX_POINT_TEXTURE = 0x08,
	/*!3DMC Extension*/
	GPAC_AFX_3DMC_EXT = 0x09,
	/*!FootPrint representation*/
	GPAC_AFX_FOOTPRINT = 0x0A,
	/*!Animated Mesh Compression*/
	GPAC_AFX_ANIMATED_MESH = 0x0B,
	/*!Scalable Complexity*/
	GPAC_AFX_SCALABLE_COMPLEXITY = 0x0C,
};


/*channel cfg flags - DECODERS MUST OUTPUT STEREO/MULTICHANNEL IN THIS ORDER*/
/*!
 * \brief Audio Channel Configuration
 *
 *	Audio channel flags for spatialization.
 \note Decoders must output stereo/multichannel audio channels in this order in the decoded audio frame.
 */
enum
{
	/*!Left Audio Channel*/
	GF_AUDIO_CH_FRONT_LEFT = (1),
	/*!Right Audio Channel*/
	GF_AUDIO_CH_FRONT_RIGHT = (1<<1),
	/*!Center Audio Channel - may also be used to signal monophonic audio*/
	GF_AUDIO_CH_FRONT_CENTER = (1<<2),
	/*!LFE Audio Channel*/
	GF_AUDIO_CH_LFE = (1<<3),
	/*!Back Left Audio Channel*/
	GF_AUDIO_CH_BACK_LEFT = (1<<4),
	/*!Back Right Audio Channel*/
	GF_AUDIO_CH_BACK_RIGHT = (1<<5),
	/*!Back Center Audio Channel*/
	GF_AUDIO_CH_BACK_CENTER = (1<<6),
	/*!Side Left Audio Channel*/
	GF_AUDIO_CH_SIDE_LEFT = (1<<7),
	/*!Side Right Audio Channel*/
	GF_AUDIO_CH_SIDE_RIGHT = (1<<8)
};



/*DIMS unit flags */
/*!
 * \brief DIMS Unit header flags
 *
 *	DIMS Unit header flags as 3GPP TS 26.142.
 */
enum
{
	/*!S: is-Scene: DIMS unit contains a complete document (<svg>*/
	GF_DIMS_UNIT_S = 1,
	/*!M: is-RAP: DIMS unit is a random access point*/
	GF_DIMS_UNIT_M = 1<<1,
	/*!I: is-Redundant: DIMS unit is made of redundant data*/
	GF_DIMS_UNIT_I = 1<<2,
	/*!D: redundant-exit: DIMS unit is the end of redundant data*/
	GF_DIMS_UNIT_D = 1<<3,
	/*!P: priority: DIMS unit is high priority*/
	GF_DIMS_UNIT_P = 1<<4,
	/*!C: compressed: DIMS unit is compressed*/
	GF_DIMS_UNIT_C = 1<<5
};


/*!
 \cond DUMMY_DOXY_SECTION
*/

/*AVC NAL unit types*/
#define GF_AVC_NALU_NON_IDR_SLICE	1
#define GF_AVC_NALU_DP_A_SLICE		2
#define GF_AVC_NALU_DP_B_SLICE		3
#define GF_AVC_NALU_DP_C_SLICE		4
#define GF_AVC_NALU_IDR_SLICE		5
#define GF_AVC_NALU_SEI				6
#define GF_AVC_NALU_SEQ_PARAM		7
#define GF_AVC_NALU_PIC_PARAM		8
#define GF_AVC_NALU_ACCESS_UNIT		9
#define GF_AVC_NALU_END_OF_SEQ		10
#define GF_AVC_NALU_END_OF_STREAM	11
#define GF_AVC_NALU_FILLER_DATA		12
#define GF_AVC_NALU_SEQ_PARAM_EXT	13

#define GF_AVC_NALU_SVC_PREFIX_NALU		14
#define GF_AVC_NALU_SVC_SUBSEQ_PARAM	15
#define GF_AVC_NALU_SLICE_AUX			19

#define GF_AVC_NALU_SVC_SLICE 0x14

/*#define GF_SVC_NALU_SLICE 0x14
#define GF_SVC_NALU_NAL_EXT_PARAM 14
#define GF_SVC_NALU_SEQ_EXT_PARAM 15*/

#define GF_AVC_TYPE_P 0
#define GF_AVC_TYPE_B 1
#define GF_AVC_TYPE_I 2
#define GF_AVC_TYPE_SP 3
#define GF_AVC_TYPE_SI 4
#define GF_AVC_TYPE2_P 5
#define GF_AVC_TYPE2_B 6
#define GF_AVC_TYPE2_I 7
#define GF_AVC_TYPE2_SP 8
#define GF_AVC_TYPE2_SI 9


/*rate sizes - note that these sizes INCLUDE the rate_type header byte*/
static const u32 GF_QCELP_RATE_TO_SIZE [] = {0, 1, 1, 4, 2, 8, 3, 17, 4, 35, 5, 8, 14, 1};
static const u32 GF_QCELP_RATE_TO_SIZE_NB = 7;
static const u32 GF_SMV_EVRC_RATE_TO_SIZE [] = {0, 1, 1, 3, 2, 6, 3, 11, 4, 23, 5, 1};
static const u32 GF_SMV_EVRC_RATE_TO_SIZE_NB = 6;
static const u32 GF_AMR_FRAME_SIZE[16] = { 12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0 };
static const u32 GF_AMR_WB_FRAME_SIZE[16] = { 17, 23, 32, 36, 40, 46, 50, 58, 60, 5, 5, 0, 0, 0, 0, 0 };


/*out-of-band sample desc (128 and 255 reserved in RFC)*/
#define GF_RTP_TX3G_SIDX_OFFSET	129

/*!
 \endcond
*/


/*! @} */

#ifdef __cplusplus
}
#endif

#endif	/*_GF_CONSTANTS_H_*/
