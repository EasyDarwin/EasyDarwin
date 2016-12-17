/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005 
 *					All rights reserved
 *
 *  This file is part of GPAC / MPEG-4 Object Descriptor sub-project
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

#ifndef _GF_MPEG4_ODF_H_
#define _GF_MPEG4_ODF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/list.h>
#include <gpac/bitstream.h>
#include <gpac/sync_layer.h>

/***************************************
			Descriptors Tag
***************************************/
enum
{
	GF_ODF_OD_TAG			= 0x01,
	GF_ODF_IOD_TAG			= 0x02,
	GF_ODF_ESD_TAG			= 0x03,
	GF_ODF_DCD_TAG			= 0x04,
	GF_ODF_DSI_TAG			= 0x05,
	GF_ODF_SLC_TAG			= 0x06,
	GF_ODF_CI_TAG			= 0x07,
	GF_ODF_SCI_TAG			= 0x08,
	GF_ODF_IPI_PTR_TAG		= 0x09,
	GF_ODF_IPMP_PTR_TAG		= 0x0A,
	GF_ODF_IPMP_TAG			= 0x0B,
	GF_ODF_QOS_TAG			= 0x0C,
	GF_ODF_REG_TAG			= 0x0D,

	/*FILE FORMAT RESERVED IDs - NEVER CREATE / USE THESE DESCRIPTORS*/
	GF_ODF_ESD_INC_TAG		= 0x0E,
	GF_ODF_ESD_REF_TAG		= 0x0F,
	GF_ODF_ISOM_IOD_TAG		= 0x10,
	GF_ODF_ISOM_OD_TAG		= 0x11,
	GF_ODF_ISOM_IPI_PTR_TAG	= 0x12,
	/*END FILE FORMAT RESERVED*/
	
	GF_ODF_EXT_PL_TAG		= 0x13,
	GF_ODF_PL_IDX_TAG		= 0x14,
	
	GF_ODF_ISO_BEGIN_TAG	= 0x15,
	GF_ODF_ISO_END_TAG		= 0x3F,

	GF_ODF_CC_TAG			= 0x40,
	GF_ODF_KW_TAG			= 0x41,
	GF_ODF_RATING_TAG		= 0x42,
	GF_ODF_LANG_TAG			= 0x43,
	GF_ODF_SHORT_TEXT_TAG	= 0x44,
	GF_ODF_TEXT_TAG			= 0x45,
	GF_ODF_CC_NAME_TAG		= 0x46,
	GF_ODF_CC_DATE_TAG		= 0x47,
	GF_ODF_OCI_NAME_TAG		= 0x48,
	GF_ODF_OCI_DATE_TAG		= 0x49,
	GF_ODF_SMPTE_TAG		= 0x4A,

	GF_ODF_SEGMENT_TAG		= 0x4B,
	GF_ODF_MEDIATIME_TAG	= 0x4C,

	GF_ODF_IPMP_TL_TAG		= 0x60,
	GF_ODF_IPMP_TOOL_TAG	= 0x61,

	GF_ODF_ISO_RES_BEGIN_TAG	= 0x62,
	GF_ODF_ISO_RES_END_TAG		= 0xBF,
	
	GF_ODF_USER_BEGIN_TAG	= 0xC0,

	/*internal descriptor for mux input description*/
	GF_ODF_MUXINFO_TAG		= GF_ODF_USER_BEGIN_TAG,
	/*internal descriptor for bifs config input description*/
	GF_ODF_BIFS_CFG_TAG		= GF_ODF_USER_BEGIN_TAG + 1,
	/*internal descriptor for UI config input description*/
	GF_ODF_UI_CFG_TAG		= GF_ODF_USER_BEGIN_TAG + 2,
	/*internal descriptor for TextConfig description*/
	GF_ODF_TEXT_CFG_TAG		= GF_ODF_USER_BEGIN_TAG + 3,
	GF_ODF_TX3G_TAG			= GF_ODF_USER_BEGIN_TAG + 4,
	GF_ODF_ELEM_MASK_TAG	= GF_ODF_USER_BEGIN_TAG + 5,
	/*internal descriptor for LASeR config input description*/
	GF_ODF_LASER_CFG_TAG	= GF_ODF_USER_BEGIN_TAG + 6,

	GF_ODF_USER_END_TAG		= 0xFE,

	GF_ODF_OCI_BEGIN_TAG	= 0x40,
	GF_ODF_OCI_END_TAG		= (GF_ODF_ISO_RES_BEGIN_TAG - 1),

	GF_ODF_EXT_BEGIN_TAG	= 0x80,
	GF_ODF_EXT_END_TAG		= 0xFE,


	/*descriptor for aucilary video data*/
	GF_ODF_AUX_VIDEO_DATA	= GF_ODF_EXT_BEGIN_TAG + 1
};


/***************************************
			Descriptors
***************************************/

#define BASE_DESCRIPTOR \
		u8 tag;

typedef struct
{
	BASE_DESCRIPTOR
} GF_Descriptor;


/*	default descriptor. 
	NOTE: The decoderSpecificInfo is used as a default desc with tag 0x05 */
typedef struct
{
	BASE_DESCRIPTOR
	u32 dataLength;
	char *data;
} GF_DefaultDescriptor;

/*Object Descriptor*/
typedef struct
{
	BASE_DESCRIPTOR
	GF_List *ipmp_tools;
} GF_IPMP_ToolList;

/*ObjectDescriptor*/
typedef struct
{
	BASE_DESCRIPTOR
	u16 objectDescriptorID;
	char *URLString;
	GF_List *ESDescriptors;
	GF_List *OCIDescriptors;
	/*includes BOTH IPMP_DescriptorPointer (IPMP & IPMPX) and GF_IPMP_Descriptor (IPMPX only)*/
	GF_List *IPMP_Descriptors;
	GF_List *extensionDescriptors;
	/*MPEG-2 (or other service mux formats) service ID*/
	u16 ServiceID;
	/*pointer to the service interface (GF_InputService) of the service having declared the object 
	only used for DASH*/
	void *service_ifce;
} GF_ObjectDescriptor;

/*GF_InitialObjectDescriptor - WARNING: even though the bitstream IOD is not
a bit extension of OD, internally it is a real overclass of OD
we usually typecast IOD to OD when flags are not needed !!!*/
typedef struct
{
	BASE_DESCRIPTOR
	u16 objectDescriptorID;
	char *URLString;
	GF_List *ESDescriptors;
	GF_List *OCIDescriptors;
	/*includes BOTH IPMP_DescriptorPointer (IPMP & IPMPX) and GF_IPMP_Descriptor (IPMPX only)*/
	GF_List *IPMP_Descriptors;
	GF_List *extensionDescriptors;
	/*MPEG-2 (or other service mux formats) service ID*/
	u16 ServiceID;
	/*pointer to the service interface (GF_InputService) of the service having declared the object 
	only used for DASH*/
	void *service_ifce;

	/*IOD extensions*/
	u8 inlineProfileFlag;
	u8 OD_profileAndLevel;
	u8 scene_profileAndLevel;
	u8 audio_profileAndLevel;
	u8 visual_profileAndLevel;
	u8 graphics_profileAndLevel;

	GF_IPMP_ToolList *IPMPToolList;
} GF_InitialObjectDescriptor;

/*File Format Object Descriptor*/
typedef struct
{
	BASE_DESCRIPTOR
	u16 objectDescriptorID;
	char *URLString;
	GF_List *ES_ID_RefDescriptors;
	GF_List *OCIDescriptors;
	GF_List *IPMP_Descriptors;
	GF_List *extensionDescriptors;
	GF_List *ES_ID_IncDescriptors;
} GF_IsomObjectDescriptor;

/*File Format Initial Object Descriptor - same remark as IOD*/
typedef struct
{
	BASE_DESCRIPTOR
	u16 objectDescriptorID;
	char *URLString;
	GF_List *ES_ID_RefDescriptors;
	GF_List *OCIDescriptors;
	GF_List *IPMP_Descriptors;
	GF_List *extensionDescriptors;
	GF_List *ES_ID_IncDescriptors;

	u8 inlineProfileFlag;
	u8 OD_profileAndLevel;
	u8 scene_profileAndLevel;
	u8 audio_profileAndLevel;
	u8 visual_profileAndLevel;
	u8 graphics_profileAndLevel;

	GF_IPMP_ToolList *IPMPToolList;
} GF_IsomInitialObjectDescriptor;


/*File Format ES Descriptor for IOD*/
typedef struct {
	BASE_DESCRIPTOR
	u32 trackID;
} GF_ES_ID_Inc;

/*File Format ES Descriptor for OD*/
typedef struct {
	BASE_DESCRIPTOR
	u16 trackRef;
} GF_ES_ID_Ref;

/*Decoder config Descriptor*/
typedef struct
{
	BASE_DESCRIPTOR
	/*coded on 8 bit, but we use 32 bits for internal signaling in GPAC to enable usage of 4CC*/
	u32 objectTypeIndication;
	u8 streamType;
	u8 upstream;
	u32 bufferSizeDB;
	u32 maxBitrate;
	u32 avgBitrate;
	GF_DefaultDescriptor *decoderSpecificInfo;
	
	/*placeholder for RVC decoder config*/
	GF_DefaultDescriptor *rvc_config;

	GF_List *profileLevelIndicationIndexDescriptor;
	/*pass through data for some modules*/
	void *udta;
} GF_DecoderConfig;


/*Content Identification Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u8 compatibility;
	u8 protectedContent;
	u8 contentTypeFlag;
	u8 contentIdentifierFlag;
	u8 contentType;
	u8 contentIdentifierType;
	/*international code string*/
	char *contentIdentifier;	
} GF_CIDesc;

/*Supplementary Content Identification Descriptor)*/
typedef struct {
	BASE_DESCRIPTOR
	u32 languageCode;
	char *supplContentIdentifierTitle;
	char *supplContentIdentifierValue;
} GF_SCIDesc;

/*IPI (Intelectual Property Identification) Descriptor Pointer*/
typedef struct {
	BASE_DESCRIPTOR
	u16 IPI_ES_Id;
} GF_IPIPtr;

/*IPMP Descriptor Pointer*/
typedef struct {
	BASE_DESCRIPTOR
	u8 IPMP_DescriptorID;
	u16 IPMP_DescriptorIDEx;
	u16 IPMP_ES_ID;	
} GF_IPMPPtr;

/*IPMPX control points*/
enum
{
	/*no control point*/
	IPMP_CP_NONE = 0,
	/*control point between DB and decoder*/
	IPMP_CP_DB = 1,
	/*control point between decoder and CB*/
	IPMP_CP_CB = 2,
	/*control point between CB and render*/
	IPMP_CP_CM = 3,
	/*control point in BIFS tree (???)*/
	IPMP_CP_BIFS = 4
	/*the rest is reserved or forbidden(0xFF)*/
};

/*IPMPX base classe*/
#define GF_IPMPX_BASE	\
	u8 tag;	\
	u8 version;	\
	u32 dataID;	\

typedef struct 
{
	GF_IPMPX_BASE
} GF_GF_IPMPX_Base;

/*IPMP descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u8 IPMP_DescriptorID;
	u16 IPMPS_Type;
	/*if IPMPS_Type=0, NULL-terminated URL, else if IPMPS_Type is not IPMPX, opaque data*/
	char *opaque_data;
	/*if IPMPS_Type=0, irrelevant (strlen(URL)), else if IPMPS_Type is not IPMPX, opaque data size*/
	u32 opaque_data_size;

	/*IPMPX specific*/
	u16 IPMP_DescriptorIDEx;
	bin128 IPMP_ToolID;
	u8 control_point;
	u8 cp_sequence_code;
	GF_List *ipmpx_data;
} GF_IPMP_Descriptor;


/*IPMPTool*/
#define MAX_IPMP_ALT_TOOLS	20
typedef struct
{
	BASE_DESCRIPTOR
	bin128 IPMP_ToolID;
	/*if set, this is an alternate tool*/
	u32 num_alternate;
	bin128 specificToolID[MAX_IPMP_ALT_TOOLS];

	struct _tagIPMPXParamDesc *toolParamDesc;
	char *tool_url;
} GF_IPMP_Tool;


/* Elementary Mask of Bifs Config - parsing only */
typedef struct {
	BASE_DESCRIPTOR
	u32 node_id;			/* referenced nodeID */
	char *node_name;		/* referenced node name */
} GF_ElementaryMask;

/*BIFSConfig - parsing only, STORED IN ESD:DCD:DSI*/
typedef struct __tag_bifs_config
{
	BASE_DESCRIPTOR
	u32 version;
	u16 nodeIDbits;
	u16 routeIDbits;
	u16 protoIDbits;
	Bool pixelMetrics;
	u16 pixelWidth, pixelHeight;
	/*BIFS-Anim stuff*/
	Bool randomAccess;
	GF_List *elementaryMasks;
	/*internal extensions for encoding*/
	Bool useNames;
} GF_BIFSConfig;

/*flags for style*/
enum
{
	GF_TXT_STYLE_NORMAL = 0,
	GF_TXT_STYLE_BOLD = 1,
	GF_TXT_STYLE_ITALIC = 2,
	GF_TXT_STYLE_UNDERLINED = 4
};

typedef struct
{
	u16 startCharOffset;
	u16 endCharOffset;
	u16 fontID;
	u8 style_flags;
	u8 font_size;
	/*ARGB*/
	u32 text_color;
} GF_StyleRecord;

typedef struct
{
	u16 fontID;
	char *fontName;
} GF_FontRecord;

typedef struct
{
	s16 top, left, bottom, right;
} GF_BoxRecord;

/*scroll flags*/
enum
{
	GF_TXT_SCROLL_CREDITS = 0,
	GF_TXT_SCROLL_MARQUEE = 1,
	GF_TXT_SCROLL_DOWN = 2,
	GF_TXT_SCROLL_RIGHT = 3
};

/* display flags*/
enum
{
	GF_TXT_SCROLL_IN = 0x00000020,
	GF_TXT_SCROLL_OUT = 0x00000040,
	/*use one of the scroll flags, eg GF_TXT_SCROLL_DIRECTION | GF_TXT_SCROLL_CREDITS*/
	GF_TXT_SCROLL_DIRECTION = 0x00000180,
	GF_TXT_KARAOKE	= 0x00000800,
	GF_TXT_VERTICAL = 0x00020000,
	GF_TXT_FILL_REGION = 0x00040000
};

typedef struct
{
	/*this is defined as a descriptor for parsing*/
	BASE_DESCRIPTOR

	u32 displayFlags;
	/*left, top: 0 -  centered: 1 - bottom, right: -1*/
	s8 horiz_justif, vert_justif;
	/*ARGB*/
	u32 back_color;
	GF_BoxRecord default_pos;
	GF_StyleRecord	default_style;

	u32 font_count;
	GF_FontRecord *fonts;

	/*unused in isomedia but needed for streamingText*/
	u8 sample_index;
} GF_TextSampleDescriptor;

typedef struct
{
	BASE_DESCRIPTOR
	/*only 0x10 shall be used for 3GP text stream*/
	u8 Base3GPPFormat;
	/*only 0x10 shall be used for StreamingText*/
	u8 MPEGExtendedFormat;
	/*only 0x10 shall be used for StreamingText (base profile, base level)*/
	u8 profileLevel;
	u32 timescale;
	/*0 forbidden, 1: out-of-band desc only, 2: in-band desc only, 3: both*/
	u8 sampleDescriptionFlags;
	/*More negative layer values are towards the viewer*/
	s16 layer;
	/*text track width & height*/
	u16 text_width;
	u16 text_height;
	/*compatible 3GP formats, same coding as 3GPPBaseFormat*/
	u8 nb_compatible_formats;
	u8 compatible_formats[20];
	/*defined in isomedia.h*/
	GF_List *sample_descriptions;

	/*if true info below are valid (cf 3GPP for their meaning)*/
	Bool has_vid_info;
	u16 video_width;
	u16 video_height;
	s16 horiz_offset;
	s16 vert_offset;
} GF_TextConfig;


/*MuxInfo descriptor - parsing only, stored in ESD:extDescr*/
typedef struct {
	BASE_DESCRIPTOR
	/*input location*/
	char *file_name;
	/*input groupID for interleaving*/
	u32 GroupID;
	/*input stream format (not required, guessed from file_name)*/
	char *streamFormat;
	/*time offset in ms from first TS (appends an edit list in mp4)*/
	u32 startTime;

	/*media length to import in ms (from 0)*/
	u32 duration;

	/*SRT/SUB import extensions - only support for text and italic style*/
	char *textNode;
	char *fontNode;

	/*video and SUB import*/
	Double frame_rate;

	/*same as importer flags, cf media.h*/
	u32 import_flags;

	/*indicates input file shall be destryed - used during SWF import*/
	Bool delete_file;

	/*carousel configuration*/
	u32 carousel_period_plus_one;
	u16 aggregate_on_esid;
} GF_MuxInfo;

typedef struct
{
	BASE_DESCRIPTOR
	/*input type*/
	char *deviceName;
	/*string sensor terminaison (validation) char*/
	char termChar;
	/*string sensor deletion char*/
	char delChar;
	/*device-specific data*/
	char *ui_data;
	u32 ui_data_length;
} GF_UIConfig;

/*LASERConfig - parsing only, STORED IN ESD:DCD:DSI*/
typedef struct __tag_laser_config
{
	BASE_DESCRIPTOR
	u8 profile;
	u8 level;
	u8 pointsCodec;
	u8 pathComponents;
	u8 fullRequestHost;
	u16 time_resolution;
	u8 colorComponentBits;
	s8 resolution;
	u8 coord_bits;
	u8 scale_bits_minus_coord_bits;
	u8 newSceneIndicator;
	u8 extensionIDBits;

	/*the rest of the structure is never coded, only used for the config of GPAC...*/
	Bool force_string_ids;/*forces all nodes to be defined with string IDs*/
} GF_LASERConfig;


/***************************************
			QoS Tags
***************************************/
enum
{
	QoSMaxDelayTag = 0x01,
	QoSPrefMaxDelayTag = 0x02,
	QoSLossProbTag = 0x03,
	QoSMaxGapLossTag = 0x04,
	QoSMaxAUSizeTag = 0x41,
	QoSAvgAUSizeTag = 0x42,
	QoSMaxAURateTag = 0x43
};

/***************************************
			QoS Qualifiers
***************************************/
typedef struct {
	BASE_DESCRIPTOR
	u8 predefined;
	GF_List *QoS_Qualifiers;
} GF_QoS_Descriptor;


#define QOS_BASE_QUALIFIER \
	u8 tag;	\
	u32 size;

typedef struct {
	QOS_BASE_QUALIFIER
} GF_QoS_Default;

typedef struct {
	QOS_BASE_QUALIFIER
	u32 MaxDelay;
} GF_QoS_MaxDelay;

typedef struct {
	QOS_BASE_QUALIFIER
	u32 PrefMaxDelay;
} GF_QoS_PrefMaxDelay;

typedef struct {
	QOS_BASE_QUALIFIER
	Float LossProb;
} GF_QoS_LossProb;

typedef struct {
	QOS_BASE_QUALIFIER
	u32 MaxGapLoss;
} GF_QoS_MaxGapLoss;

typedef struct {
	QOS_BASE_QUALIFIER
	u32 MaxAUSize;
} GF_QoS_MaxAUSize;

typedef struct {
	QOS_BASE_QUALIFIER
	u32 AvgAUSize;
} GF_QoS_AvgAUSize;

typedef struct {
	QOS_BASE_QUALIFIER
	u32 MaxAURate;
} GF_QoS_MaxAURate;

typedef struct {
	QOS_BASE_QUALIFIER
	u32 DataLength;		/*max size class : 2^28 - 1*/
	char *Data;
} GF_QoS_Private;


/*Registration Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u32 formatIdentifier;
	u32 dataLength;
	char *additionalIdentificationInfo;
} GF_Registration;

/*Language Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u32 langCode;
} GF_Language;

/*Elementary Stream Descriptor*/
typedef struct
{
	BASE_DESCRIPTOR
	u16 ESID;
	u16 OCRESID;
	u16 dependsOnESID;
	u8 streamPriority;
	char *URLString;
	GF_DecoderConfig *decoderConfig;
	GF_SLConfig *slConfig;
	GF_IPIPtr *ipiPtr;
	GF_QoS_Descriptor *qos;
	GF_Registration *RegDescriptor;
	/*0 or 1 lang desc*/
	GF_Language *langDesc;
	
	GF_List *IPIDataSet;
	GF_List *IPMPDescriptorPointers;
	GF_List *extensionDescriptors;
} GF_ESD;


/*Auxiliary Video Data Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u32 aux_video_type;
	u32 position_offset_h;
	u32 position_offset_v;
	u32 knear;
	u32 kfar;
	u32 parallax_zero;
	u32 parallax_scale;
	u32 dref;
	u32 wref;
} GF_AuxVideoDescriptor;

/*Content Classification Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u32 classificationEntity;
	u16 classificationTable;
	u32 dataLength;
	char *contentClassificationData;
} GF_CCDescriptor;


/*this structure is used in GF_KeyWord*/
typedef struct {
	char *keyWord;
} GF_KeyWordItem;

/*Key Word Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u32 languageCode;
	u8 isUTF8;
	GF_List *keyWordsList;
} GF_KeyWord;

/*Rating Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u32 ratingEntity;
	u16 ratingCriteria;
	u32 infoLength;
	char *ratingInfo;
} GF_Rating;


/*Short Textual Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u32 langCode;
	u8 isUTF8;
	char *eventName;
	char *eventText;
} GF_ShortTextual;


/*this structure is used in GF_ExpandedTextual*/
typedef struct {
	char *text;
} GF_ETD_ItemText;

/*Expanded Textual Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u32 langCode;
	u8 isUTF8;
	GF_List *itemDescriptionList;
	GF_List *itemTextList;
	char *NonItemText;
} GF_ExpandedTextual;

/*this structure is used in GF_CC_Name*/
typedef struct {
	u32 langCode;
	u8 isUTF8;
	char *contentCreatorName;
} GF_ContentCreatorInfo;

/*Content Creator Name GF_Descriptor
NOTE: the desctructor will delete all the items in the list
(GF_ContentCreatorInfo items) */
typedef struct {
	BASE_DESCRIPTOR
	GF_List *ContentCreators;
} GF_CC_Name;

/*Content Creation Date Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	char contentCreationDate[5];
} GF_CC_Date;


/*this structure is used in GF_OCICreators*/
typedef struct {
	u32 langCode;
	u8 isUTF8;
	char *OCICreatorName;
} GF_OCICreator_item;

/*OCI Creator Name Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	GF_List *OCICreators;
} GF_OCICreators;

/*OCI Creation Date Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	char OCICreationDate[5];
} GF_OCI_Data;


/*this structure is used in GF_SMPTECamera*/
typedef struct {
	u8 paramID;
	u32 param;
} GF_SmpteParam;

/*Smpte Camera Position Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u8 cameraID;
	GF_List *ParamList;
} GF_SMPTECamera;


/*Extension Profile Level Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u8 profileLevelIndicationIndex;
	u8 ODProfileLevelIndication;
	u8 SceneProfileLevelIndication;
	u8 AudioProfileLevelIndication;
	u8 VisualProfileLevelIndication;
	u8 GraphicsProfileLevelIndication;
	u8 MPEGJProfileLevelIndication;
} GF_PLExt;

/*Profile Level Indication Index Descriptor*/
typedef struct {
	BASE_DESCRIPTOR
	u8 profileLevelIndicationIndex;
} GF_PL_IDX;


/*AVC config descriptor - not a real MPEG-4 descriptor */
/*used for sequenceParameterSetNALUnit and pictureParameterSetNALUnit*/
typedef struct
{
	u16 size;
	char *data;
    /* used of AVC/SVC detection */
    s32 id;
} GF_AVCConfigSlot;

typedef struct 
{
	u8 configurationVersion;
	u8 AVCProfileIndication;
	u8 profile_compatibility;
	u8 AVCLevelIndication; 
	u8 nal_unit_size;
	
	GF_List *sequenceParameterSets;
	GF_List *pictureParameterSets;

	/*for SVC*/
	u8 complete_representation;
} GF_AVCConfig;


/************************************************************
				Media Control Extensions
************************************************************/
typedef struct
{
	BASE_DESCRIPTOR
	Double startTime;
	Double Duration;
	char *SegmentName;
} GF_Segment;

typedef struct
{
	BASE_DESCRIPTOR
	Double mediaTimeStamp;
} GF_MediaTime;


/****************************************************************************

			MPEG-4 SYSTEM - OBJECT DESCRIPTORS COMMANDS DECLARATION

****************************************************************************/


/***************************************
			Commands Tags
***************************************/
enum
{
	GF_ODF_OD_UPDATE_TAG					= 0x01,
	GF_ODF_OD_REMOVE_TAG					= 0x02,
	GF_ODF_ESD_UPDATE_TAG					= 0x03,
	GF_ODF_ESD_REMOVE_TAG					= 0x04,
	GF_ODF_IPMP_UPDATE_TAG					= 0x05,
	GF_ODF_IPMP_REMOVE_TAG					= 0x06,

	/*file format reserved*/
	GF_ODF_ESD_REMOVE_REF_TAG				= 0x07,

	GF_ODF_COM_ISO_BEGIN_TAG		= 0x0D,
	GF_ODF_COM_ISO_END_TAG		= 0xBF,
	
	GF_ODF_COM_USER_BEGIN_TAG	= 0xC0,
	GF_ODF_COM_USER_END_TAG		= 0xFE
};

/***************************************
			OD commands
***************************************/
#define BASE_OD_COMMAND \
	u8 tag;

/*the (abstract) base command. */
typedef struct {
	BASE_OD_COMMAND
} GF_ODCom;

/*the default bcommand*/
typedef struct {
	BASE_OD_COMMAND
	u32 dataSize;
	char *data;
} GF_BaseODCom;

/*Object Descriptor Update
NB: the list can contain OD or IOD, except internally in the File Format (only MP4_OD)*/
typedef struct
{
	BASE_OD_COMMAND
	GF_List *objectDescriptors;
} GF_ODUpdate;

/*Object Descriptor Remove*/
typedef struct
{
	BASE_OD_COMMAND
	u32 NbODs;
	u16 *OD_ID;
} GF_ODRemove;

/*Elementary Stream Descriptor Update*/
typedef struct
{
	BASE_OD_COMMAND
	u16 ODID;
	GF_List *ESDescriptors;
} GF_ESDUpdate;

/*Elementary Stream Descriptor Remove*/
typedef struct {
	BASE_OD_COMMAND
	u16 ODID;
	u32 NbESDs;
	u16 *ES_ID;
} GF_ESDRemove;

/*IPMP Descriptor Update*/
typedef struct {
	BASE_OD_COMMAND
	GF_List *IPMPDescList;
} GF_IPMPUpdate;

/*IPMP Descriptor Remove*/
typedef struct {
	BASE_OD_COMMAND
	u32 NbIPMPDs;
	/*now this is bad: only IPMPv1 descriptors can be removed at run tim...*/
	u8 *IPMPDescID;
} GF_IPMPRemove;






/********************************************************************
	OD Exports
********************************************************************/

/*OD CODEC object - just a simple reader/writer*/
typedef struct tagODCoDec
{
	GF_BitStream *bs;
	GF_List *CommandList;
} GF_ODCodec;


/*construction / destruction*/
GF_ODCodec *gf_odf_codec_new();
void gf_odf_codec_del(GF_ODCodec *codec);
/* add a command to the codec command list. */
GF_Err gf_odf_codec_add_com(GF_ODCodec *codec, GF_ODCom *command);
/*encode the current command list - once called the commands are removed or destroyed depending on @cleanup_type: 
	0: commands are removed from the list but not destroyed
	1: commands are removed from the list and destroyed
	2: commands are kept in the list and not destroyed
if delete_content is set*/
GF_Err gf_odf_codec_encode(GF_ODCodec *codec, u32 cleanup_type);
/*get the encoded AU. user is responsible of allocated space*/
GF_Err gf_odf_codec_get_au(GF_ODCodec *codec, char **outAU, u32 *au_length);
/* set the encoded AU to the codec*/
GF_Err gf_odf_codec_set_au(GF_ODCodec *codec, const char *au, u32 au_length);
/*decode the previously set-up AU*/
GF_Err gf_odf_codec_decode(GF_ODCodec *codec);
/*get the first OD command in the list. Once called, the command is removed 
from the command list. Return NULL when commandList is empty*/
GF_ODCom *gf_odf_codec_get_com(GF_ODCodec *codec);

/*apply a command to the codec command list. Command is duplicated if needed
This is used for state maintenance and RAP generation.*/
GF_Err gf_odf_codec_apply_com(GF_ODCodec *codec, GF_ODCom *command);

/************************************************************
		GF_ODCom Functions
************************************************************/

/*Commands Creation / Destruction*/
GF_ODCom *gf_odf_com_new(u8 tag);
GF_Err gf_odf_com_del(GF_ODCom **com);


/************************************************************
		Descriptors Functions
************************************************************/

/*Descriptors Creation / Destruction*/
GF_Descriptor *gf_odf_desc_new(u8 tag);
void gf_odf_desc_del(GF_Descriptor *desc);

/*this is a helper for building a preformatted GF_ESD with decoderConfig, decoderSpecificInfo with no data and 
SLConfig descriptor with predefined*/
GF_ESD *gf_odf_desc_esd_new(u32 sl_predefined);

/*special function for authoring - convert DSI to BIFSConfig*/
GF_BIFSConfig *gf_odf_get_bifs_config(GF_DefaultDescriptor *dsi, u8 oti);
/*special function for authoring - convert DSI to LASERConfig*/
GF_Err gf_odf_get_laser_config(GF_DefaultDescriptor *dsi, GF_LASERConfig *cfg);
/*sepcial function for authoring - convert DSI to TextConfig*/
GF_Err gf_odf_get_text_config(GF_DefaultDescriptor *dsi, u8 oti, GF_TextConfig *cfg);
/*special function for authoring - convert DSI to UIConfig*/
GF_Err gf_odf_get_ui_config(GF_DefaultDescriptor *dsi, GF_UIConfig *cfg);
/*converts UIConfig to dsi - does not destroy input descr but does create output one*/
GF_Err gf_odf_encode_ui_config(GF_UIConfig *cfg, GF_DefaultDescriptor **out_dsi);

/*simple constructor/destructor*/
GF_AVCConfig *gf_odf_avc_cfg_new();
void gf_odf_avc_cfg_del(GF_AVCConfig *cfg);
/*gets GF_AVCConfig from MPEG-4 DSI*/
GF_AVCConfig *gf_odf_avc_cfg_read(char *dsi, u32 dsi_size);
/*writes GF_AVCConfig as MPEG-4 DSI*/
GF_Err gf_odf_avc_cfg_write(GF_AVCConfig *cfg, char **outData, u32 *outSize);

/*destroy the descriptors in a list but not the list*/
GF_Err gf_odf_desc_list_del(GF_List *descList);

/*use this function to decode a standalone descriptor
the raw descriptor MUST be formatted with tag and size field!!!
a new desc is created and you must delete it when done*/
GF_Err gf_odf_desc_read(char *raw_desc, u32 descSize, GF_Descriptor * *outDesc);

/*use this function to encode a standalone descriptor
the desc will be formatted with tag and size field
the output buffer is allocated and you must delete it when done*/
GF_Err gf_odf_desc_write(GF_Descriptor *desc, char **outEncDesc, u32 *outSize);

/*use this function to get the size of a standalone descriptor (including tag and size fields)
return 0 if error*/
u32 gf_odf_desc_size(GF_Descriptor *desc);

/*this is usefull to duplicate on the fly a descriptor*/
GF_Err gf_odf_desc_copy(GF_Descriptor *inDesc, GF_Descriptor **outDesc);

/*This functions handles internally what desc can be added to another desc
and adds it. NO DUPLICATION of the descriptor, so
once a desc is added to its parent, destroying the parent WILL DESTROY 
this descriptor*/
GF_Err gf_odf_desc_add_desc(GF_Descriptor *parentDesc, GF_Descriptor *newDesc);

/*returns complete textual description of stream*/
const char *gf_esd_get_textual_description(GF_ESD *esd);

const char *gf_afx_get_type_description(u8 afx_code);


/*Since IPMP V2, we introduce a new set of functions to read / write a list of descriptors
that have no containers (a bit like an OD command, but for descriptors)
This is usefull for IPMPv2 DecoderSpecificInfo which contains a set of IPMP_Declarators
As it could be used for other purposes we keep it generic
you must create the list yourself, the functions just encode/decode from/to the list*/

/*uncompress an encoded list of descriptors. You must pass an empty GF_List structure
to know exactly what was in the buffer*/
GF_Err gf_odf_desc_list_read(char *raw_list, u32 raw_size, GF_List *descList);
/*compress all descriptors in the list into a single buffer. The buffer is allocated
by the lib and must be destroyed by your app
you must pass (outEncList != NULL  && *outEncList == NULL)*/
GF_Err gf_odf_desc_list_write(GF_List *descList, char **outEncList, u32 *outSize);
/*returns size of encoded desc list*/
GF_Err gf_odf_desc_list_size(GF_List *descList, u32 *outSize);

/*retuns NULL if unknown, otherwise value*/
const char *gf_odf_stream_type_name(u32 streamType);
u32 gf_odf_stream_type_by_name(const char *streamType);


#ifndef GPAC_MINIMAL_ODF


/************************************************************
		QoS Qualifiers Functions
************************************************************/

/*QoS Qualifiers Creation / Destruction*/
GF_QoS_Default *gf_odf_qos_new(u8 tag);
GF_Err gf_odf_qos_del(GF_QoS_Default **qos);

/*READ/WRITE functions: QoS qualifiers are special descriptors but follow the same rules as descriptors.
therefore, use gf_odf_desc_read and gf_odf_desc_write for QoS*/

/*same function, but for QoS, as a Qualifier IS NOT a descriptor*/
GF_Err gf_odf_qos_add_qualif(GF_QoS_Descriptor *desc, GF_QoS_Default *qualif);



/*
	OCI Stream AU is a list of OCI event (like OD AU is a list of OD commands)
*/

typedef struct __tag_oci_event OCIEvent;

OCIEvent *gf_oci_event_new(u16 EventID);
void gf_oci_event_del(OCIEvent *event);

GF_Err gf_oci_event_set_start_time(OCIEvent *event, u8 Hours, u8 Minutes, u8 Seconds, u8 HundredSeconds, u8 IsAbsoluteTime);
GF_Err gf_oci_event_set_duration(OCIEvent *event, u8 Hours, u8 Minutes, u8 Seconds, u8 HundredSeconds);
GF_Err gf_oci_event_add_desc(OCIEvent *event, GF_Descriptor *oci_desc);

GF_Err gf_oci_event_get_id(OCIEvent *event, u16 *ID);
GF_Err gf_oci_event_get_start_time(OCIEvent *event, u8 *Hours, u8 *Minutes, u8 *Seconds, u8 *HundredSeconds, u8 *IsAbsoluteTime);
GF_Err gf_oci_event_get_duration(OCIEvent *event, u8 *Hours, u8 *Minutes, u8 *Seconds, u8 *HundredSeconds);
u32 gf_oci_event_get_desc_count(OCIEvent *event);
GF_Descriptor *gf_oci_event_get_desc(OCIEvent *event, u32 DescIndex);
GF_Err gf_oci_event_rem_desc(OCIEvent *event, u32 DescIndex);



typedef struct __tag_oci_codec OCICodec;

/*construction / destruction
IsEncoder specifies an OCI Event encoder
version is for future extensions, and only 0x01 is valid for now*/
OCICodec *gf_oci_codec_new(u8 IsEncoder, u8 Version);
void gf_oci_codec_del(OCICodec *codec);

/*				ENCODER FUNCTIONS
add a command to the codec event list. 
The event WILL BE DESTROYED BY THE CODEC after encoding*/
GF_Err gf_oci_codec_add_event(OCICodec *codec, OCIEvent *event);

/*encode AU. The memory allocation is done in place
WARNING: once this function called, the codec event List is empty 
and events destroyed
you must set *outAU = NULL*/
GF_Err gf_oci_codec_encode(OCICodec *codec, char **outAU, u32 *au_length);



/*Decoder: decode the previously set-up AU
the input buffer is cleared once decoded*/
GF_Err gf_oci_codec_decode(OCICodec *codec, char *au, u32 au_length);

/*get the first OCI Event in the list. Once called, the event is removed 
from the event list. Return NULL when the event List is empty
you MUST delete events */
OCIEvent *gf_oci_codec_get_event(OCICodec *codec);


#ifndef GPAC_DISABLE_OD_DUMP

/*OD dump tools*/
GF_Err gf_odf_dump_au(char *data, u32 dataLength, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_com(void *p, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_desc(void *ptr, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_com_list(GF_List *commandList, FILE *trace, u32 indent, Bool XMTDump);

/*OCI dump tools*/
GF_Err gf_oci_dump_event(OCIEvent *ev, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_oci_dump_au(u8 version, char *au, u32 au_length, FILE *trace, u32 indent, Bool XMTDump);

#endif /*GPAC_DISABLE_OD_DUMP*/


#endif /*GPAC_MINIMAL_ODF*/

/*OD parsing tools (XMT/BT)*/
/*returns desc tag based on name*/
u32 gf_odf_get_tag_by_name(char *descName);

/*field type for OD/QoS/IPMPX/etc*/
enum
{
	/*regular type*/
	GF_ODF_FT_DEFAULT = 0,
	/*single descriptor type*/
	GF_ODF_FT_OD = 1,
	/*descriptor list type*/
	GF_ODF_FT_OD_LIST = 2,
	/*IPMP Data type*/
	GF_ODF_FT_IPMPX = 3,
	/*IPMP Data list type*/
	GF_ODF_FT_IPMPX_LIST = 4,
	/*IPMP ByteArray type*/
	GF_ODF_FT_IPMPX_BA = 5,
	/*IPMP ByteArray list type*/
	GF_ODF_FT_IPMPX_BA_LIST = 6
};
u32 gf_odf_get_field_type(GF_Descriptor *desc, char *fieldName);

/*set non-descriptor field value - value string shall be presented without ' or " characters*/
GF_Err gf_odf_set_field(GF_Descriptor *desc, char *fieldName, char *val);

#ifndef GPAC_MINIMAL_ODF



/*
	IPMPX extensions - IPMP Data only (messages are not supported yet)
*/

typedef struct
{
	u32 length;
	char *data;
} GF_IPMPX_ByteArray;

/*IPMPX authentication descriptors*/
#define GF_IPMPX_AUTH_DESC	\
	u8 tag;	\

typedef struct
{
	GF_IPMPX_AUTH_DESC
} GF_IPMPX_Authentication;

enum
{
	GF_IPMPX_AUTH_Forbidden_Tag = 0x00,
	GF_IPMPX_AUTH_AlgorithmDescr_Tag = 0x01,
	GF_IPMPX_AUTH_KeyDescr_Tag = 0x02
};

typedef struct
{
	GF_IPMPX_AUTH_DESC
	char *keyBody;
	u32 keyBodyLength;
} GF_IPMPX_AUTH_KeyDescriptor;

typedef struct
{
	GF_IPMPX_AUTH_DESC
	/*used if no specAlgoID*/
	u16 regAlgoID;
	GF_IPMPX_ByteArray *specAlgoID;
	GF_IPMPX_ByteArray *OpaqueData;
} GF_IPMPX_AUTH_AlgorithmDescriptor;


/*IPMP data messages*/
enum 
{
	GF_IPMPX_OPAQUE_DATA_TAG = 0x01,
	GF_IPMPX_AUDIO_WM_INIT_TAG = 0x02,
	GF_IPMPX_VIDEO_WM_INIT_TAG = 0x03,
	GF_IPMPX_SEL_DEC_INIT_TAG = 0x04,
	GF_IPMPX_KEY_DATA_TAG = 0x05,
	GF_IPMPX_AUDIO_WM_SEND_TAG = 0x06,
	GF_IPMPX_VIDEO_WM_SEND_TAG = 0x07,
	GF_IPMPX_RIGHTS_DATA_TAG = 0x08,
	GF_IPMPX_SECURE_CONTAINER_TAG = 0x09,
	GF_IPMPX_ADD_TOOL_LISTENER_TAG = 0x0A,
	GF_IPMPX_REMOVE_TOOL_LISTENER_TAG = 0x0B,
	GF_IPMPX_INIT_AUTHENTICATION_TAG = 0x0C,
	GF_IPMPX_MUTUAL_AUTHENTICATION_TAG = 0x0D,
	GF_IPMPX_USER_QUERY_TAG = 0x0E,
	GF_IPMPX_USER_RESPONSE_TAG = 0x0F,
	GF_IPMPX_PARAMETRIC_DESCRIPTION_TAG = 0x10,
	GF_IPMPX_PARAMETRIC_CAPS_QUERY_TAG = 0x11,
	GF_IPMPX_PARAMETRIC_CAPS_RESPONSE_TAG = 0x12,
	/*NO ASSOCIATED STRUCTURE*/
	GF_IPMPX_GET_TOOLS_TAG = 0x13,
	GF_IPMPX_GET_TOOLS_RESPONSE_TAG = 0x14,
	GF_IPMPX_GET_TOOL_CONTEXT_TAG = 0x15,
	GF_IPMPX_GET_TOOL_CONTEXT_RESPONSE_TAG = 0x16,
	GF_IPMPX_CONNECT_TOOL_TAG = 0x17,
	GF_IPMPX_DISCONNECT_TOOL_TAG = 0x18,
	GF_IPMPX_NOTIFY_TOOL_EVENT_TAG = 0x19,
	GF_IPMPX_CAN_PROCESS_TAG = 0x1A,
	GF_IPMPX_TRUST_SECURITY_METADATA_TAG = 0x1B,
	GF_IPMPX_TOOL_API_CONFIG_TAG = 0x1C,

	/*ISMA*/
	GF_IPMPX_ISMACRYP_TAG = 0xD0, 

	/*intern ones for parsing (not real datas)*/
	GF_IPMPX_TRUSTED_TOOL_TAG = 0xA1,
	GF_IPMPX_TRUST_SPECIFICATION_TAG = 0xA2,
	/*emulate algo descriptors as base IPMP classes for parsing...*/
	GF_IPMPX_ALGORITHM_DESCRIPTOR_TAG = 0xA3,
	GF_IPMPX_KEY_DESCRIPTOR_TAG = 0xA4,
	GF_IPMPX_PARAM_DESCRIPTOR_ITEM_TAG = 0xA5,
	GF_IPMPX_SEL_ENC_BUFFER_TAG = 0xA6,
	GF_IPMPX_SEL_ENC_FIELD_TAG = 0xA7
};

typedef char GF_IPMPX_Date[5];


#define GF_IPMPX_DATA_BASE	\
	u8 tag;	\
	u8 Version;	\
	u8 dataID;	\

typedef struct
{
	GF_IPMPX_DATA_BASE
} GF_IPMPX_Data;

typedef struct
{
	GF_IPMPX_DATA_BASE
	u32 Context;
	u8 AuthType;
} GF_IPMPX_InitAuthentication;

/*NOT a real DATA, only used as data for parsing*/
typedef struct
{
	GF_IPMPX_DATA_BASE
	GF_IPMPX_Date startDate;
	u8 attackerProfile;
	u32 trustedDuration;
	GF_IPMPX_ByteArray	*CCTrustMetadata;
} GF_IPMPX_TrustSpecification;

/*NOT a real DATA, only used as data for parsing*/
typedef struct
{
	GF_IPMPX_DATA_BASE
	bin128 toolID;	
	GF_IPMPX_Date AuditDate;
	GF_List *trustSpecifications;
} GF_IPMPX_TrustedTool;

typedef struct _ipmpx_TrustSecurityMetadata
{
	GF_IPMPX_DATA_BASE
	GF_List *TrustedTools;
} GF_IPMPX_TrustSecurityMetadata;


typedef struct
{
	GF_IPMPX_DATA_BASE
	Bool failedNegotiation;

	GF_List *candidateAlgorithms;
	GF_List *agreedAlgorithms;
	GF_IPMPX_ByteArray *AuthenticationData;

	/*inclAuthCodes will be set if any of the members is set (cf spec...)*/
	u32 certType;
	/*GF_IPMPX_ByteArray list*/
	GF_List *certificates;
	GF_IPMPX_AUTH_KeyDescriptor *publicKey;
	GF_IPMPX_ByteArray *opaque;
	GF_IPMPX_TrustSecurityMetadata *trustData;
	GF_IPMPX_ByteArray *authCodes;
} GF_IPMPX_MutualAuthentication;

typedef struct
{
	GF_IPMPX_DATA_BASE
	/*if set MAC is part of the encrypted data*/
	Bool isMACEncrypted;

	GF_IPMPX_ByteArray *encryptedData;
	GF_IPMPX_Data *protectedMsg;
	GF_IPMPX_ByteArray *MAC;
} GF_IPMPX_SecureContainer;

typedef struct
{
	GF_List *ipmp_tools;
} GF_IPMPX_GetToolsResponse;

typedef struct
{
	GF_IPMPX_DATA_BASE
	GF_IPMPX_ByteArray	*main_class;
	GF_IPMPX_ByteArray	*subClass;
	GF_IPMPX_ByteArray	*typeData;
	GF_IPMPX_ByteArray	*type;
	GF_IPMPX_ByteArray	*addedData;
} GF_IPMPX_ParametricDescriptionItem;

typedef struct _tagIPMPXParamDesc
{
	GF_IPMPX_DATA_BASE
	GF_IPMPX_ByteArray *descriptionComment;
	u8 majorVersion;
	u8 minorVersion;
	/*list of GF_IPMPX_ParametricDescriptionItem*/
	GF_List *descriptions;
} GF_IPMPX_ParametricDescription;

typedef struct
{
	GF_IPMPX_DATA_BASE
	GF_IPMPX_ParametricDescription *description;
} GF_IPMPX_ToolParamCapabilitiesQuery;

typedef struct
{
	GF_IPMPX_DATA_BASE
	Bool capabilitiesSupported;
} GF_IPMPX_ToolParamCapabilitiesResponse;


typedef struct
{
	GF_IPMPX_DATA_BASE
	GF_IPMP_Descriptor *toolDescriptor;
} GF_IPMPX_ConnectTool;

typedef struct
{
	GF_IPMPX_DATA_BASE
	u32 IPMP_ToolContextID;
} GF_IPMPX_DisconnectTool;


typedef struct
{
	GF_IPMPX_DATA_BASE
	u8 scope;
	u16 IPMP_DescriptorIDEx;
} GF_IPMPX_GetToolContext;


typedef struct
{
	GF_IPMPX_DATA_BASE
	u16 OD_ID;
	u16 ESD_ID;
	u32 IPMP_ToolContextID;
} GF_IPMPX_GetToolContextResponse;

/*GF_IPMPX_LISTEN_Types*/
enum
{
	GF_IPMPX_LISTEN_CONNECTED = 0x00,
	GF_IPMPX_LISTEN_CONNECTIONFAILED = 0x01,
	GF_IPMPX_LISTEN_DISCONNECTED = 0x02,
	GF_IPMPX_LISTEN_DISCONNECTIONFAILED = 0x03,
	GF_IPMPX_LISTEN_WATERMARKDETECTED = 0x04
};

typedef struct
{
	GF_IPMPX_DATA_BASE
	u8 scope;
	/*events to listen to*/
	u8 eventTypeCount;
	u8 eventType[10];
} GF_IPMPX_AddToolNotificationListener;

typedef struct
{
	GF_IPMPX_DATA_BASE
	u8 eventTypeCount;
	u8 eventType[10];
} GF_IPMPX_RemoveToolNotificationListener;

typedef struct
{
	GF_IPMPX_DATA_BASE
	u16 OD_ID;
	u16 ESD_ID;
	u8 eventType;
	u32 IPMP_ToolContextID;
} GF_IPMPX_NotifyToolEvent;

typedef struct
{
	GF_IPMPX_DATA_BASE
	Bool canProcess;
} GF_IPMPX_CanProcess;

typedef struct
{
	GF_IPMPX_DATA_BASE
	GF_IPMPX_ByteArray *opaqueData;
} GF_IPMPX_OpaqueData;


typedef struct
{
	GF_IPMPX_DATA_BASE
	GF_IPMPX_ByteArray *keyBody;
	/*flags meaning
	hasStartDTS = 1;
	hasStartPacketID = 1<<1;
	hasExpireDTS = 1<<2;
	hasExpirePacketID = 1<<3
	*/
	u32 flags;

	u64 startDTS;
	u32 startPacketID;
	u64 expireDTS;
	u32 expirePacketID;
	GF_IPMPX_ByteArray *OpaqueData;
} GF_IPMPX_KeyData;

typedef struct
{
	GF_IPMPX_DATA_BASE
	GF_IPMPX_ByteArray *rightsInfo;	
} GF_IPMPX_RightsData;


/*not a real GF_IPMPX_Data in spec, but emulated as if for parsing*/
typedef struct
{
	GF_IPMPX_DATA_BASE
	bin128 cipher_Id; 
	u8 syncBoundary;
	/*block mode if stream cypher info is NULL*/
	u8 mode;  
	u16 blockSize;
	u16 keySize;
	GF_IPMPX_ByteArray *Stream_Cipher_Specific_Init_Info; 
} GF_IPMPX_SelEncBuffer;

/*not a real GF_IPMPX_Data in spec, but emulated as if for parsing*/
typedef struct
{
	GF_IPMPX_DATA_BASE
	u8 field_Id; 
	u8 field_Scope;
	u8 buf; 

	u16 mappingTableSize;
	u16 *mappingTable;
	GF_IPMPX_ByteArray *shuffleSpecificInfo;
} GF_IPMPX_SelEncField;


/*mediaTypeExtension*/
enum
{
	GF_IPMPX_SE_MT_ISO_IEC = 0x00,
	GF_IPMPX_SE_MT_ITU = 0x01
	/*the rest is reserved or forbidden*/
};

/*compliance*/
enum
{
	GF_IPMPX_SE_COMP_FULLY = 0x00,
	GF_IPMPX_SE_COMP_VIDEO_PACKETS = 0x01,
	GF_IPMPX_SE_COMP_VIDEO_VOP = 0x02,
	GF_IPMPX_SE_COMP_VIDEO_NONE = 0x03,
	GF_IPMPX_SE_COMP_VIDEO_GOB = 0x04,
	/*0x05-2F	ISO Reserved for video*/
	GF_IPMPX_SE_COMP_AAC_DF = 0x30,
	GF_IPMPX_SE_COMP_AAC_NONE = 0x31
	/*
	0x32 -  0x5F	ISO Reserved for audio
	0x60 - 0xCF	ISO Reserved
	0xD0 - 0xFE	User Defined
	0xFF	Forbidden
	*/
};

/*syncBoundary*/
enum
{
	GF_IPMPX_SE_SYNC_VID7EO_PACKETS = 0x00,
	GF_IPMPX_SE_SYNC_VIDEO_VOP = 0x01,
	GF_IPMPX_SE_SYNC_VIDEO_GOV = 0x02,
	/*0x03-2F	ISO Reserved for video,*/
	GF_IPMPX_SE_SYNC_AAC_DF = 0x30
	/*0x31 -  0x5F	ISO Reserved for audio
	0x60 - 0xCF	ISO Reserved
	0xD0 - 0xFE	User Defined
	0xFF	Forbidden
	*/
};

/*field_Id*/
enum
{
	GF_IPMPX_SE_FID_VIDEO_MV = 0x00,
	GF_IPMPX_SE_FID_VIDEO_DC = 0x01,
	GF_IPMPX_SE_FID_VIDEO_DCT_SIGN = 0x02,
	GF_IPMPX_SE_FID_VIDEO_DQUANT = 0x03,
	GF_IPMPX_SE_FID_VIDEO_DCT_COEF = 0x04,
	GF_IPMPX_SE_FID_VIDEO_ALL = 0x05,
	/*0x06-2F	ISO Reserved for video*/
	GF_IPMPX_SE_FID_AAC_SIGN = 0x30,
	GF_IPMPX_SE_FID_AAC_CODEWORDS = 0x31,
	GF_IPMPX_SE_FID_AAC_SCALE = 0x32
	/*0x32 -  0x5F	ISO Reserved for audio
	0x60 - 0xCF	ISO Reserved
	0xD0 - 0xFE	User Defined
	0xFF	Forbidden*/
};


typedef struct
{
	GF_IPMPX_DATA_BASE
	u8 mediaTypeExtension; 
	u8 mediaTypeIndication;
	u8 profileLevelIndication; 
	u8 compliance;

	GF_List *SelEncBuffer;

	GF_List *SelEncFields;

	u16 RLE_DataLength;
	u16 *RLE_Data;
} GF_IPMPX_SelectiveDecryptionInit;


/*watermark init ops*/
enum
{
	GF_IPMPX_WM_INSERT = 0,
	GF_IPMPX_WM_EXTRACT = 1,
	GF_IPMPX_WM_REMARK = 2,
	GF_IPMPX_WM_DETECT_COMPRESSION = 3
};

/*used for both audio and video WM init*/
typedef struct
{
	GF_IPMPX_DATA_BASE
	/*
	for audio: PCM defined (0x01) and all audio objectTypeIndications
	for video: YUV defined (0x01) and all visual objectTypeIndications
	*/
	u8 inputFormat;
	u8 requiredOp;

	/*valid for audio WM, inputFormat=0x01*/
	u8 nChannels;
	u8 bitPerSample;
	u32 frequency;

	/*valid for video WM, inputFormat=0x01*/
	u16 frame_horizontal_size;
	u16 frame_vertical_size;
	u8 chroma_format;

	u32 wmPayloadLen;
	char *wmPayload;

	u16 wmRecipientId;
	
	u32 opaqueDataSize;
	char *opaqueData;
} GF_IPMPX_WatermarkingInit;



/*WM status*/
enum
{
	GF_IPMPX_WM_PAYLOAD = 0,
	GF_IPMPX_WM_NOPAYLOAD = 1,
	GF_IPMPX_WM_NONE = 2,
	GF_IPMPX_WM_UNKNOWN = 3
};

/*compression status*/
enum
{
	GF_IPMPX_WM_COMPRESSION = 0,
	GF_IPMPX_WM_NO_COMPRESSION = 1,
	GF_IPMPX_WM_COMPRESSION_UNKNOWN = 2
};

typedef struct
{
	GF_IPMPX_DATA_BASE
	u8 wm_status;
	u8 compression_status;
	/*if payload is set, status is FORCED to AUDIO_GF_IPMPX_WM_PAYLOAD*/
	GF_IPMPX_ByteArray *payload;
	GF_IPMPX_ByteArray *opaqueData;
} GF_IPMPX_SendWatermark;


typedef struct
{
	GF_IPMPX_DATA_BASE
	/*GPAC only supports non-0 IDs*/
	u32 Instantiation_API_ID;
	u32 Messaging_API_ID;
	GF_IPMPX_ByteArray *opaqueData;
} GF_IPMPX_ToolAPI_Config;

typedef struct
{
	GF_IPMPX_DATA_BASE
	u8 cryptoSuite;
	u8 IV_length;
	Bool use_selective_encryption;
	u8 key_indicator_length;
} GF_IPMPX_ISMACryp;


/*constructor/destructor*/
GF_IPMPX_Data *gf_ipmpx_data_new(u8 tag);
void gf_ipmpx_data_del(GF_IPMPX_Data *p);

/*parse from bitstream*/
GF_Err gf_ipmpx_data_parse(GF_BitStream *bs, GF_IPMPX_Data **out_data);
/*get IPMP_Data contained size (eg without tag & sizeofinstance)*/
u32 gf_ipmpx_data_size(GF_IPMPX_Data *p);
/*get fulml IPMP_Data encoded size (eg with tag & sizeofinstance)*/
u32 gf_ipmpx_data_full_size(GF_IPMPX_Data *p);
/*writes IPMP_Data to buffer*/
GF_Err gf_ipmpx_data_write(GF_BitStream *bs, GF_IPMPX_Data *_p);

/*returns GF_IPMPX_Tag based on name*/
u8 gf_ipmpx_get_tag(char *dataName);
/*return values: cf above */
u32 gf_ipmpx_get_field_type(GF_IPMPX_Data *p, char *fieldName);
GF_Err gf_ipmpx_set_field(GF_IPMPX_Data *desc, char *fieldName, char *val);
/*assign subdata*/
GF_Err gf_ipmpx_set_sub_data(GF_IPMPX_Data *desc, char *fieldName, GF_IPMPX_Data *subdesc);
/*assign bytearray*/
GF_Err gf_ipmpx_set_byte_array(GF_IPMPX_Data *p, char *field, char *str);

/*ipmpx dumper*/
GF_Err gf_ipmpx_dump_data(GF_IPMPX_Data *_p, FILE *trace, u32 indent, Bool XMTDump);


#endif /*GPAC_MINIMAL_ODF*/

#ifdef __cplusplus
}
#endif

#endif	/*_GF_MPEG4_ODF_H_*/
