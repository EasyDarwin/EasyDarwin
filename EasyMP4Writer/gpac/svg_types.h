/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Authors: Cyril Concolato - Jean Le Feuvre
 *			Copyright (c) 2004-200X ENST
 *			All rights reserved
 *
 *  This file is part of GPAC / Scene Graph sub-project
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

#ifndef _GF_SVG_SVG_TYPES_H_
#define _GF_SVG_SVG_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/path2d.h>



/* Attributes in SVG can be accessed using a GF_FieldInfo structure 
   like it is done in the BIFS part of the implementation:

	fieldIndex:		attribute tag to identify the attribute in the element in the case of dynamic alloc (default)
	             or index of the attribute in the element in the case of static allocation of attributes

	fieldType:		attribute data type as in the enumeration below
	
	name:			attribute name (WARNING: this may be NULL)

	far_ptr:		pointer to the actual data with one of the type given in this file

	NDTType:		unused in SVG
	eventType:		unused in SVG
	on_event_in:	unused in SVG 
*/

/* SVG attribute types */
enum {
	SVG_Unknown_datatype					= 0,

	/* keyword enum types */
	XML_Space_datatype,
	XMLEV_Propagate_datatype,
	XMLEV_DefaultAction_datatype,
	XMLEV_Phase_datatype,
	SVG_FillRule_datatype,
	SVG_StrokeLineJoin_datatype,
	SVG_StrokeLineCap_datatype,
	SVG_FontStyle_datatype,
	SVG_FontWeight_datatype,
	SVG_FontVariant_datatype,
	SVG_TextAnchor_datatype,
	SVG_TransformType_datatype, 
	SVG_Display_datatype, 
	SVG_Visibility_datatype,
	SVG_Overflow_datatype,
	SVG_ZoomAndPan_datatype,
	SVG_DisplayAlign_datatype,
	SVG_PointerEvents_datatype,
	SVG_RenderingHint_datatype,
	SVG_VectorEffect_datatype,
	SVG_PlaybackOrder_datatype,
	SVG_TimelineBegin_datatype,
	SVG_GradientUnit_datatype,
	SVG_InitialVisibility_datatype,
	SVG_FocusHighlight_datatype,
	SVG_Overlay_datatype,
	SVG_TransformBehavior_datatype,
	SVG_SpreadMethod_datatype,
	SVG_TextAlign_datatype,
	SVG_Focusable_datatype,
	SVG_Filter_TransferType_datatype,
	SMIL_SyncBehavior_datatype,
	SMIL_SyncTolerance_datatype,
	SMIL_AttributeType_datatype,
	SMIL_CalcMode_datatype,
	SMIL_Additive_datatype,
	SMIL_Accumulate_datatype,
	SMIL_Restart_datatype,
	SMIL_Fill_datatype,	

	SVG_LAST_U8_PROPERTY,

	DOM_String_datatype,
	DOM_StringList_datatype,

	XMLEV_Event_datatype,
	XMLRI_datatype,
	XMLRI_List_datatype,
	XML_IDREF_datatype,

	SMIL_KeyTimes_datatype,
	SMIL_KeySplines_datatype,
	SMIL_KeyPoints_datatype,
	SMIL_Times_datatype,

	/* animated (untyped) value */
	SMIL_AnimateValue_datatype,
	SMIL_AnimateValues_datatype,
	SMIL_Duration_datatype,
	SMIL_RepeatCount_datatype,
	SMIL_AttributeName_datatype,

	/* SVG Number */
	SVG_Number_datatype,
	SVG_FontSize_datatype,
	SVG_Length_datatype,
	SVG_Coordinate_datatype,
	SVG_Rotate_datatype,

	/* List of */
	SVG_Numbers_datatype,
	SVG_Points_datatype,
	SVG_Coordinates_datatype,

	/*all other types*/
	SVG_Boolean_datatype,
	SVG_Color_datatype,
	SVG_Paint_datatype,
	SVG_PathData_datatype,
	SVG_FontFamily_datatype,
	SVG_ID_datatype,

	SVG_StrokeDashArray_datatype,
	SVG_PreserveAspectRatio_datatype,
	SVG_ViewBox_datatype,
	SVG_GradientOffset_datatype,
	SVG_Focus_datatype,
	SVG_Clock_datatype,
	SVG_ContentType_datatype,
	SVG_LanguageID_datatype,

	/* Matrix related types */
	SVG_Transform_datatype,
	SVG_Transform_Translate_datatype,
	SVG_Transform_Scale_datatype,
	SVG_Transform_SkewX_datatype,
	SVG_Transform_SkewY_datatype,
	SVG_Transform_Rotate_datatype,
	SVG_Motion_datatype,

	/*LASeR types*/
	LASeR_Choice_datatype,
	LASeR_Size_datatype,
};

/* Definition of SVG base data types */
typedef char *DOM_String;
typedef DOM_String SVG_String;
typedef DOM_String SVG_ContentType;
typedef DOM_String SVG_LanguageID;
typedef DOM_String SVG_TextContent;

/* types not yet handled properly, i.e. for the moment using strings */
typedef DOM_String SVG_ID;
typedef DOM_String SVG_LinkTarget;
typedef DOM_String SVG_GradientOffset;

typedef Double SVG_Clock;

typedef GF_List *ListOfXXX;
typedef GF_List *SVG_Numbers;
typedef GF_List *SVG_Coordinates;
typedef GF_List	*SVG_FeatureList;
typedef GF_List	*SVG_ExtensionList;
typedef GF_List	*SVG_FormatList;
typedef GF_List	*SVG_ListOfIRI;
typedef GF_List	*SVG_LanguageIDs;
typedef GF_List	*SVG_FontList;
typedef GF_List *SVG_TransformList;
typedef GF_List *SVG_Points;
typedef GF_List *SMIL_Times;
typedef GF_List *SMIL_KeyTimes;
typedef GF_List *SMIL_KeyPoints;
/* Fixed between 0 and 1 */
typedef GF_List *SMIL_KeySplines;

typedef GF_Matrix2D SVG_Motion;

/* SMIL Anim types */
typedef struct {
	/*field type*/
	u32 type;
	/*field pointer*/
	void *field_ptr;
	/*attribute name for textual parsing*/
	char *name;
	/*attribute tag for live transcoding*/
	u32 tag;
} SMIL_AttributeName;

enum {
	/*clock time*/
	GF_SMIL_TIME_CLOCK			= 0,
	/*wallclock time*/
	GF_SMIL_TIME_WALLCLOCK		= 1,
	/*resolved time of an event, discarded when restarting animation.*/
	GF_SMIL_TIME_EVENT_RESOLVED	= 2,
	/*event time*/
	GF_SMIL_TIME_EVENT			= 3,
	/*indefinite time*/
	GF_SMIL_TIME_INDEFINITE		= 4,
};

#define GF_SMIL_TIME_IS_CLOCK(v) (v<=GF_SMIL_TIME_EVENT_RESOLVED)
#define GF_SMIL_TIME_IS_SPECIFIED_CLOCK(v) (v<GF_SMIL_TIME_EVENT_RESOLVED)

typedef struct
{
	u32 type;
	/*for accessKey and mouse button, or repeatCount when the event is a SMIL repeat */
	u32 parameter;
} XMLEV_Event;

typedef struct {
	/* Type of timing value*/
	u8 type;
	/* in case of syncbase, event, repeat value: this is the pointer to the source of the event */
	GF_Node *element; 
	/* id of the element before resolution of the pointer to the element */
	char *element_id; 
	/* listener associated with event */
	GF_Node *listener; 

	/* event type and parameter */
	XMLEV_Event event; 
	/*set if event is 
		begin rather than beginEvent,
		end rather than endEvent,
		repeat rather than repeatEvent */
	Bool is_absolute_event;
	/*clock offset (absolute or relative to event)*/
	Double clock;

} SMIL_Time;

enum {
	SMIL_DURATION_AUTO  = 0,
	SMIL_DURATION_INDEFINITE,
	SMIL_DURATION_MEDIA,
	SMIL_DURATION_NONE,
	SMIL_DURATION_DEFINED,
};
typedef struct {
	u8 type;
	Double clock_value;
} SMIL_Duration;

enum {
	SMIL_RESTART_ALWAYS = 0,
	SMIL_RESTART_NEVER,
	SMIL_RESTART_WHENNOTACTIVE,
};
typedef u8 SMIL_Restart;

enum {
	SMIL_FILL_FREEZE=0,
	SMIL_FILL_REMOVE,
	
};
typedef u8 SMIL_Fill;

enum {
	SMIL_REPEATCOUNT_INDEFINITE  = 0,
	SMIL_REPEATCOUNT_DEFINED	 = 1,
	/* used only for static allocation of SVG attributes */
	SMIL_REPEATCOUNT_UNSPECIFIED = 2
};
typedef struct {
	u8 type;
	Fixed count;
} SMIL_RepeatCount;

typedef struct {
	u8 type;
	void *value;
} SMIL_AnimateValue;

typedef struct {
	u8 type;
	GF_List *values;
} SMIL_AnimateValues;

enum {
	SMIL_ADDITIVE_REPLACE = 0,
	SMIL_ADDITIVE_SUM
}; 
typedef u8 SMIL_Additive;

enum {
	SMIL_ACCUMULATE_NONE = 0,
	SMIL_ACCUMULATE_SUM
}; 
typedef u8 SMIL_Accumulate;

enum {
	/*WARNING: default value is linear, order changed for LASeR coding*/
	SMIL_CALCMODE_DISCRETE = 0,
	SMIL_CALCMODE_LINEAR,
	SMIL_CALCMODE_PACED,
	SMIL_CALCMODE_SPLINE
};
typedef u8 SMIL_CalcMode;
/* end of SMIL Anim types */

enum {
	XMLRI_ELEMENTID = 0,
	XMLRI_STRING,
	XMLRI_STREAMID
};
typedef struct __xml_ri {
	u8 type;
	char *string;
	void *target;
	u32 lsr_stream_id;
} XMLRI;

/*the same structure is used to watch for IDREF changes (LASeR node replace)*/
typedef struct __xml_ri XML_IDREF;

enum
{
	SVG_FOCUS_AUTO = 0, 
	SVG_FOCUS_SELF, 
	SVG_FOCUS_IRI, 
};

typedef struct
{
	u8 type;
	XMLRI target;
} SVG_Focus;

enum {
	SVG_FONTFAMILY_INHERIT = 0,
	SVG_FONTFAMILY_VALUE
};

typedef struct {
	u8 type;
	SVG_String value;
} SVG_FontFamily;

enum {
	SVG_FONTSTYLE_INHERIT = 0,
	SVG_FONTSTYLE_ITALIC = 1,  
	SVG_FONTSTYLE_NORMAL = 2,
	SVG_FONTSTYLE_OBLIQUE = 3
}; 
typedef u8 SVG_FontStyle;

/*the values are chosen to match LASeR code points*/
enum {
	SVG_PATHCOMMAND_M = 3,
	SVG_PATHCOMMAND_L = 2,
	SVG_PATHCOMMAND_C = 0,
	SVG_PATHCOMMAND_S = 5,
	SVG_PATHCOMMAND_Q = 4,
	SVG_PATHCOMMAND_T = 6,
	SVG_PATHCOMMAND_A = 20,
	SVG_PATHCOMMAND_Z = 8
};

#define USE_GF_PATH 1

#if USE_GF_PATH
typedef GF_Path SVG_PathData;
#else
typedef struct {
	GF_List *commands;
	GF_List *points;
} SVG_PathData;
#endif

typedef struct {
	Fixed x, y;
} SVG_Point;

typedef struct {
	Fixed x, y, angle;
} SVG_Point_Angle;

typedef struct {
	Bool is_set;
	Fixed x, y, width, height;
} SVG_ViewBox;

typedef Bool SVG_Boolean;

/*WARNING - THESE ARE PATH FLAGS, CHECK IF WORKING*/
enum {
	SVG_FILLRULE_EVENODD= 0,
	SVG_FILLRULE_NONZERO,
	SVG_FILLRULE_INHERIT
};
typedef u8 SVG_FillRule;
	
enum {
	SVG_STROKELINEJOIN_MITER = GF_LINE_JOIN_MITER_SVG,
	SVG_STROKELINEJOIN_ROUND = GF_LINE_JOIN_ROUND,
	SVG_STROKELINEJOIN_BEVEL = GF_LINE_JOIN_BEVEL,
	SVG_STROKELINEJOIN_INHERIT = 100
};
typedef u8 SVG_StrokeLineJoin;

/* Warning: GPAC naming is not the same as SVG naming for line cap Flat = butt and Butt = square*/
enum {
	SVG_STROKELINECAP_BUTT = GF_LINE_CAP_FLAT,
	SVG_STROKELINECAP_ROUND = GF_LINE_CAP_ROUND,
	SVG_STROKELINECAP_SQUARE = GF_LINE_CAP_SQUARE,
	SVG_STROKELINECAP_INHERIT = 100
};
typedef u8 SVG_StrokeLineCap;

enum {
	SVG_OVERFLOW_INHERIT	= 0,
	SVG_OVERFLOW_VISIBLE	= 1,
	SVG_OVERFLOW_HIDDEN		= 2,
	SVG_OVERFLOW_SCROLL		= 3,
	SVG_OVERFLOW_AUTO		= 4
};
typedef u8 SVG_Overflow;

enum {
	SVG_COLOR_RGBCOLOR = 0,
	SVG_COLOR_INHERIT,
	SVG_COLOR_CURRENTCOLOR,
	SVG_COLOR_ACTIVE_BORDER, /*Active window border*/
	SVG_COLOR_ACTIVE_CAPTION, /*Active window caption. */
	SVG_COLOR_APP_WORKSPACE, /*Background color of multiple document interface. */
	SVG_COLOR_BACKGROUND, /*Desktop background. */
	SVG_COLOR_BUTTON_FACE, /* Face color for three-dimensional display elements. */
	SVG_COLOR_BUTTON_HIGHLIGHT, /* Dark shadow for three-dimensional display elements (for edges facing away from the light source). */
	SVG_COLOR_BUTTON_SHADOW, /* Shadow color for three-dimensional display elements. */
	SVG_COLOR_BUTTON_TEXT, /*Text on push buttons. */
	SVG_COLOR_CAPTION_TEXT, /* Text in caption, size box, and scrollbar arrow box. */
	SVG_COLOR_GRAY_TEXT, /* Disabled ('grayed') text. */
	SVG_COLOR_HIGHLIGHT, /* Item(s) selected in a control. */
	SVG_COLOR_HIGHLIGHT_TEXT, /*Text of item(s) selected in a control. */
	SVG_COLOR_INACTIVE_BORDER, /* Inactive window border. */
	SVG_COLOR_INACTIVE_CAPTION, /* Inactive window caption. */
	SVG_COLOR_INACTIVE_CAPTION_TEXT, /*Color of text in an inactive caption. */
	SVG_COLOR_INFO_BACKGROUND, /* Background color for tooltip controls. */
	SVG_COLOR_INFO_TEXT,  /*Text color for tooltip controls. */
	SVG_COLOR_MENU, /*Menu background. */
	SVG_COLOR_MENU_TEXT, /* Text in menus. */
	SVG_COLOR_SCROLLBAR, /* Scroll bar gray area. */
	SVG_COLOR_3D_DARK_SHADOW, /* Dark shadow for three-dimensional display elements. */
	SVG_COLOR_3D_FACE, /* Face color for three-dimensional display elements. */
	SVG_COLOR_3D_HIGHLIGHT, /* Highlight color for three-dimensional display elements. */
	SVG_COLOR_3D_LIGHT_SHADOW, /* Light color for three-dimensional display elements (for edges facing the light source). */
	SVG_COLOR_3D_SHADOW, /* Dark shadow for three-dimensional display elements. */
	SVG_COLOR_WINDOW, /* Window background. */
	SVG_COLOR_WINDOW_FRAME, /* Window frame. */
	SVG_COLOR_WINDOW_TEXT /* Text in windows.*/
};

typedef struct {
	u8 type;
	Fixed red, green, blue;
} SVG_Color;

enum {
	SVG_PAINT_NONE		= 0,
	SVG_PAINT_COLOR		= 1,
	SVG_PAINT_URI		= 2,
	SVG_PAINT_INHERIT	= 3
};

typedef struct {
	u8 type;
	SVG_Color color;
	XMLRI iri;
} SVG_Paint, SVG_SVGColor;

enum {
	SVG_NUMBER_VALUE		= 0,
	SVG_NUMBER_PERCENTAGE	= 1,
	SVG_NUMBER_EMS			= 2,
	SVG_NUMBER_EXS			= 3,
	SVG_NUMBER_PX			= 4,
	SVG_NUMBER_CM			= 5,
	SVG_NUMBER_MM			= 6,
	SVG_NUMBER_IN			= 7,
	SVG_NUMBER_PT			= 8,
	SVG_NUMBER_PC			= 9,
	SVG_NUMBER_INHERIT		= 10,
	SVG_NUMBER_AUTO			= 11,
	SVG_NUMBER_AUTO_REVERSE	= 12
};

typedef struct {
	u8 type;
	Fixed value;
} SVG_Number, 
  SVG_FontSize, 
  SVG_Length, 
  SVG_Coordinate, 
  SVG_Rotate;

typedef struct {
	u8 is_ref;
	GF_Matrix2D mat;
} SVG_Transform;

enum {
	SVG_TRANSFORM_MATRIX	= 0,
	SVG_TRANSFORM_TRANSLATE = 1,
	SVG_TRANSFORM_SCALE		= 2,
	SVG_TRANSFORM_ROTATE	= 3,
	SVG_TRANSFORM_SKEWX		= 4,
	SVG_TRANSFORM_SKEWY		= 5
};

typedef u8 SVG_TransformType; 

enum {
	SVG_FONTWEIGHT_100 = 0, 
	SVG_FONTWEIGHT_200,
	SVG_FONTWEIGHT_300, 
	SVG_FONTWEIGHT_400,
	SVG_FONTWEIGHT_500,
	SVG_FONTWEIGHT_600,
	SVG_FONTWEIGHT_700,
	SVG_FONTWEIGHT_800,
	SVG_FONTWEIGHT_900,
	SVG_FONTWEIGHT_BOLD, 
	SVG_FONTWEIGHT_BOLDER, 
	SVG_FONTWEIGHT_INHERIT,
	SVG_FONTWEIGHT_LIGHTER, 
	SVG_FONTWEIGHT_NORMAL
};
typedef u8 SVG_FontWeight;

enum {
	SVG_FONTVARIANT_INHERIT		= 0,
	SVG_FONTVARIANT_NORMAL		= 1,
	SVG_FONTVARIANT_SMALLCAPS	= 2
};
typedef u8 SVG_FontVariant;

enum {
	SVG_VISIBILITY_HIDDEN   = 0,
	SVG_VISIBILITY_INHERIT	= 1,
	SVG_VISIBILITY_VISIBLE  = 2,
	SVG_VISIBILITY_COLLAPSE = 3
};
typedef u8 SVG_Visibility;

enum {
	SVG_DISPLAY_INHERIT = 0,
	SVG_DISPLAY_NONE    = 1,
	SVG_DISPLAY_INLINE  = 2,
	SVG_DISPLAY_BLOCK,
	SVG_DISPLAY_LIST_ITEM,
	SVG_DISPLAY_RUN_IN,
	SVG_DISPLAY_COMPACT,
	SVG_DISPLAY_MARKER,
	SVG_DISPLAY_TABLE,
	SVG_DISPLAY_INLINE_TABLE,
	SVG_DISPLAY_TABLE_ROW_GROUP,
	SVG_DISPLAY_TABLE_HEADER_GROUP,
	SVG_DISPLAY_TABLE_FOOTER_GROUP,
	SVG_DISPLAY_TABLE_ROW,
	SVG_DISPLAY_TABLE_COLUMN_GROUP,
	SVG_DISPLAY_TABLE_COLUMN,
	SVG_DISPLAY_TABLE_CELL,
	SVG_DISPLAY_TABLE_CAPTION
};
typedef u8 SVG_Display;

enum {
	SVG_DISPLAYALIGN_INHERIT	= 0,
	SVG_DISPLAYALIGN_AUTO		= 1,
	SVG_DISPLAYALIGN_AFTER		= 2,
	SVG_DISPLAYALIGN_BEFORE		= 3,
	SVG_DISPLAYALIGN_CENTER		= 4
};
typedef u8 SVG_DisplayAlign;

enum {
	SVG_TEXTALIGN_INHERIT	= 0,
	SVG_TEXTALIGN_START		= 1,
	SVG_TEXTALIGN_CENTER	= 2,
	SVG_TEXTALIGN_END		= 3
};
typedef u8 SVG_TextAlign;

enum {
	SVG_STROKEDASHARRAY_NONE	= 0,
	SVG_STROKEDASHARRAY_INHERIT = 1,
	SVG_STROKEDASHARRAY_ARRAY	= 2
};

typedef struct {
	u32 count;
	Fixed* vals;
    u8 *units;
} UnitArray;

typedef struct {
	u8 type;
	UnitArray array;
} SVG_StrokeDashArray;

enum {
	SVG_TEXTANCHOR_INHERIT	= 0,
	SVG_TEXTANCHOR_END		= 1,
	SVG_TEXTANCHOR_MIDDLE	= 2,
	SVG_TEXTANCHOR_START	= 3
};
typedef u8 SVG_TextAnchor;

enum {
	SVG_ANGLETYPE_UNKNOWN		= 0,
	SVG_ANGLETYPE_UNSPECIFIED	= 1,
	SVG_ANGLETYPE_DEG			= 2,
	SVG_ANGLETYPE_RAD			= 3,
	SVG_ANGLETYPE_GRAD			= 4
};

enum {
	SVG_UNIT_TYPE_UNKNOWN			= 0,
	SVG_UNIT_TYPE_USERSPACEONUSE	= 1,
	SVG_UNIT_TYPE_OBJECTBOUNDINGBOX = 2
};

enum {
	// Alignment Types
	SVG_PRESERVEASPECTRATIO_NONE = 1,
	SVG_PRESERVEASPECTRATIO_XMINYMIN = 2,
	SVG_PRESERVEASPECTRATIO_XMIDYMIN = 3,
	SVG_PRESERVEASPECTRATIO_XMAXYMIN = 4,
	SVG_PRESERVEASPECTRATIO_XMINYMID = 5,
	SVG_PRESERVEASPECTRATIO_XMIDYMID = 0, //default
	SVG_PRESERVEASPECTRATIO_XMAXYMID = 6,
	SVG_PRESERVEASPECTRATIO_XMINYMAX = 7,
	SVG_PRESERVEASPECTRATIO_XMIDYMAX = 8,
	SVG_PRESERVEASPECTRATIO_XMAXYMAX = 9
};

enum {
	// Meet_or_slice Types
	SVG_MEETORSLICE_MEET  = 0,
	SVG_MEETORSLICE_SLICE = 1
};

typedef struct {
	Bool defer;
	u8 align;
	u8 meetOrSlice;
} SVG_PreserveAspectRatio; 

enum {
	SVG_ZOOMANDPAN_DISABLE = 0,
	SVG_ZOOMANDPAN_MAGNIFY,
};

typedef u8 SVG_ZoomAndPan;

enum {
	LENGTHADJUST_UNKNOWN   = 0,
	LENGTHADJUST_SPACING     = 1,
	LENGTHADJUST_SPACINGANDGLYPHS     = 2
};

enum {
    // textPath Method Types
	TEXTPATH_METHODTYPE_UNKNOWN   = 0,
	TEXTPATH_METHODTYPE_ALIGN     = 1,
	TEXTPATH_METHODTYPE_STRETCH     = 2
};
enum {
    // textPath Spacing Types
	TEXTPATH_SPACINGTYPE_UNKNOWN   = 0,
	TEXTPATH_SPACINGTYPE_AUTO     = 1,
	TEXTPATH_SPACINGTYPE_EXACT     = 2
};

enum {
    // Marker Unit Types
	SVG_MARKERUNITS_UNKNOWN        = 0,
	SVG_MARKERUNITS_USERSPACEONUSE = 1,
	SVG_MARKERUNITS_STROKEWIDTH    = 2
};
enum {
    // Marker Orientation Types
	SVG_MARKER_ORIENT_UNKNOWN      = 0,
	SVG_MARKER_ORIENT_AUTO         = 1,
	SVG_MARKER_ORIENT_ANGLE        = 2
};

enum {
    // Spread Method Types
	SVG_SPREADMETHOD_UNKNOWN = 0,
	SVG_SPREADMETHOD_PAD     = 1,
	SVG_SPREADMETHOD_REFLECT = 2,
	SVG_SPREADMETHOD_REPEAT  = 3
};

enum {
	SVG_POINTEREVENTS_INHERIT			= 0,
	SVG_POINTEREVENTS_ALL				= 1,
	SVG_POINTEREVENTS_FILL				= 2,
	SVG_POINTEREVENTS_NONE				= 3,
	SVG_POINTEREVENTS_PAINTED			= 4,
	SVG_POINTEREVENTS_STROKE			= 5,
	SVG_POINTEREVENTS_VISIBLE			= 6,
	SVG_POINTEREVENTS_VISIBLEFILL		= 7,
	SVG_POINTEREVENTS_VISIBLEPAINTED	= 8,
	SVG_POINTEREVENTS_VISIBLESTROKE		= 9,
	SVG_POINTEREVENTS_BOUNDINGBOX		= 10
};
typedef u8 SVG_PointerEvents;

enum {
	SVG_RENDERINGHINT_INHERIT				= 0,
	SVG_RENDERINGHINT_AUTO					= 1,
	SVG_RENDERINGHINT_OPTIMIZEQUALITY		= 2,
	SVG_RENDERINGHINT_OPTIMIZESPEED			= 3,
	SVG_RENDERINGHINT_OPTIMIZELEGIBILITY	= 4,
	SVG_RENDERINGHINT_CRISPEDGES			= 5,
	SVG_RENDERINGHINT_GEOMETRICPRECISION	= 6,

};
typedef u8 SVG_RenderingHint;

enum {
	SVG_VECTOREFFECT_INHERIT			= 0,
	SVG_VECTOREFFECT_NONE				= 1,
	SVG_VECTOREFFECT_NONSCALINGSTROKE	= 2,
};
typedef u8 SVG_VectorEffect;

enum {
	XMLEVENT_PROPAGATE_CONTINUE = 0,
	XMLEVENT_PROPAGATE_STOP		= 1
};
typedef u8 XMLEV_Propagate;

enum {
	XMLEVENT_DEFAULTACTION_CANCEL = 0,
	XMLEVENT_DEFAULTACTION_PERFORM,
	
};
typedef u8 XMLEV_DefaultAction;

enum {
	XMLEVENT_PHASE_DEFAULT	= 0,
	XMLEVENT_PHASE_CAPTURE	= 1
};
typedef u8 XMLEV_Phase;

enum {
	SMIL_SYNCBEHAVIOR_INHERIT		= 0,
	/*LASeR order*/
	SMIL_SYNCBEHAVIOR_CANSLIP,
	SMIL_SYNCBEHAVIOR_DEFAULT,
	SMIL_SYNCBEHAVIOR_INDEPENDENT,
	SMIL_SYNCBEHAVIOR_LOCKED,
};
typedef u8 SMIL_SyncBehavior;

enum {
	SMIL_SYNCTOLERANCE_INHERIT		= 0,
	SMIL_SYNCTOLERANCE_DEFAULT		= 1,
	SMIL_SYNCTOLERANCE_VALUE		= 2
};

typedef struct {
	u8 type;
	SVG_Clock value;
} SMIL_SyncTolerance;

enum {
	SMIL_ATTRIBUTETYPE_CSS	= 0,
	SMIL_ATTRIBUTETYPE_XML,
	SMIL_ATTRIBUTETYPE_AUTO,
};
typedef u8 SMIL_AttributeType;

enum {
	SVG_PLAYBACKORDER_ALL			= 0,
	SVG_PLAYBACKORDER_FORWARDONLY	= 1,
};
typedef u8 SVG_PlaybackOrder;

enum {
	SVG_TIMELINEBEGIN_ONLOAD=0,
	SVG_TIMELINEBEGIN_ONSTART,
};
typedef u8 SVG_TimelineBegin;

enum {
	XML_SPACE_DEFAULT		= 0,
	XML_SPACE_PRESERVE		= 1
};
typedef u8 XML_Space;


enum {
	SVG_GRADIENTUNITS_OBJECT = 0,
	SVG_GRADIENTUNITS_USER = 1
};
typedef u8 SVG_GradientUnit;

enum {
	SVG_FOCUSHIGHLIGHT_AUTO = 0,
	SVG_FOCUSHIGHLIGHT_NONE	= 1
};
typedef u8 SVG_FocusHighlight;

enum {
	SVG_INITIALVISIBILTY_WHENSTARTED = 0,
	SVG_INITIALVISIBILTY_ALWAYS		 = 1
};
typedef u8 SVG_InitialVisibility;

enum {
	SVG_TRANSFORMBEHAVIOR_GEOMETRIC = 0,
	SVG_TRANSFORMBEHAVIOR_PINNED,
	SVG_TRANSFORMBEHAVIOR_PINNED180,
	SVG_TRANSFORMBEHAVIOR_PINNED270,
	SVG_TRANSFORMBEHAVIOR_PINNED90,
};
typedef u8 SVG_TransformBehavior;

enum {
	SVG_OVERLAY_NONE = 0,
	SVG_OVERLAY_TOP,
};
typedef u8 SVG_Overlay;

enum {
	SVG_FOCUSABLE_AUTO = 0,
	SVG_FOCUSABLE_TRUE,
	SVG_FOCUSABLE_FALSE,
};
typedef u8 SVG_Focusable;

enum
{
	SVG_FILTER_TRANSFER_IDENTITY,
	SVG_FILTER_TRANSFER_TABLE,
	SVG_FILTER_TRANSFER_DISCRETE,
	SVG_FILTER_TRANSFER_LINEAR,
	SVG_FILTER_TRANSFER_GAMMA
};
typedef u8 SVG_Filter_TransferType;

enum {
	SVG_SPREAD_PAD = 0,
	SVG_SPREAD_REFLECT,
	SVG_SPREAD_REPEAT,
};
typedef u8 SVG_SpreadMethod;

enum {
	LASeR_CHOICE_ALL   = 0,
	LASeR_CHOICE_NONE  = 1,
	LASeR_CHOICE_N	   = 2
};
typedef u8 LASeR_Choice_enum;

typedef struct {
	u32 type;
	u32 choice_index;
} LASeR_Choice;

typedef struct {
	Fixed width, height;
} LASeR_Size;


#ifdef __cplusplus
}
#endif

#endif /*_GF_SVG_SVG_TYPES_H_*/


