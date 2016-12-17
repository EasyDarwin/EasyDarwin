/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Authors: Cyril Concolato - Jean Le Feuvre
 *    Copyright (c)2004-200X ENST - All rights reserved
 *
 *  This file is part of GPAC / XML-based Scene Graph sub-project
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

#ifndef _GF_XML_NODES_H
#define _GF_XML_NODES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/scenegraph_svg.h>

enum {
	TAG_SVG_a = GF_NODE_RANGE_FIRST_SVG,
	TAG_SVG_animate,
	TAG_SVG_animateColor,
	TAG_SVG_animateMotion,
	TAG_SVG_animateTransform,
	TAG_SVG_animation,
	TAG_SVG_audio,
	TAG_SVG_circle,
	TAG_SVG_defs,
	TAG_SVG_desc,
	TAG_SVG_discard,
	TAG_SVG_ellipse,
	TAG_SVG_font,
	TAG_SVG_font_face,
	TAG_SVG_font_face_src,
	TAG_SVG_font_face_uri,
	TAG_SVG_foreignObject,
	TAG_SVG_g,
	TAG_SVG_glyph,
	TAG_SVG_handler,
	TAG_SVG_hkern,
	TAG_SVG_image,
	TAG_SVG_line,
	TAG_SVG_linearGradient,
	TAG_SVG_listener,
	TAG_SVG_metadata,
	TAG_SVG_missing_glyph,
	TAG_SVG_mpath,
	TAG_SVG_path,
	TAG_SVG_polygon,
	TAG_SVG_polyline,
	TAG_SVG_prefetch,
	TAG_SVG_radialGradient,
	TAG_SVG_rect,
	TAG_SVG_script,
	TAG_SVG_set,
	TAG_SVG_solidColor,
	TAG_SVG_stop,
	TAG_SVG_svg,
	TAG_SVG_switch,
	TAG_SVG_tbreak,
	TAG_SVG_text,
	TAG_SVG_textArea,
	TAG_SVG_title,
	TAG_SVG_tspan,
	TAG_SVG_use,
	TAG_SVG_video,

	TAG_SVG_filter,
	TAG_SVG_feDistantLight,
	TAG_SVG_fePointLight,
	TAG_SVG_feSpotLight,
	TAG_SVG_feBlend,
	TAG_SVG_feColorMatrix,
	TAG_SVG_feComponentTransfer,
	TAG_SVG_feFuncR,
	TAG_SVG_feFuncG,
	TAG_SVG_feFuncB,
	TAG_SVG_feFuncA,
	TAG_SVG_feComposite,
	TAG_SVG_feConvolveMatrix,
	TAG_SVG_feDiffuseLighting,
	TAG_SVG_feDisplacementMap,
	TAG_SVG_feFlood,
	TAG_SVG_feGaussianBlur,
	TAG_SVG_feImage,
	TAG_SVG_feMerge,
	TAG_SVG_feMorphology,
	TAG_SVG_feOffset,
	TAG_SVG_feSpecularLighting,
	TAG_SVG_feTile,
	TAG_SVG_feTurbulence,
	
	TAG_LSR_conditional,
	TAG_LSR_cursorManager,
	TAG_LSR_rectClip,
	TAG_LSR_selector,
	TAG_LSR_simpleLayout,
	TAG_LSR_updates,

	/*undefined elements (when parsing) use this tag*/
	TAG_SVG_UndefinedElement
};

/* Definition of SVG attribute internal tags - 200 defined */
/* TAG names are made of "TAG_SVG_ATT_" + SVG attribute name (with - replaced by _) */
enum {
	TAG_SVG_ATT_id = TAG_SVG_ATT_RANGE_FIRST,
	TAG_SVG_ATT__class,	

	TAG_SVG_ATT_requiredFeatures,
	TAG_SVG_ATT_requiredExtensions,
	TAG_SVG_ATT_requiredFormats,
	TAG_SVG_ATT_requiredFonts,
	TAG_SVG_ATT_systemLanguage,
	TAG_SVG_ATT_display,
	TAG_SVG_ATT_visibility,
	TAG_SVG_ATT_image_rendering,
	TAG_SVG_ATT_pointer_events,
	TAG_SVG_ATT_shape_rendering,
	TAG_SVG_ATT_text_rendering,
	TAG_SVG_ATT_audio_level,
	TAG_SVG_ATT_viewport_fill,
	TAG_SVG_ATT_viewport_fill_opacity,
	TAG_SVG_ATT_overflow,
	TAG_SVG_ATT_fill_opacity,
	TAG_SVG_ATT_stroke_opacity,
	TAG_SVG_ATT_fill,
	TAG_SVG_ATT_fill_rule,
	TAG_SVG_ATT_filter,
	TAG_SVG_ATT_stroke,
	TAG_SVG_ATT_stroke_dasharray,
	TAG_SVG_ATT_stroke_dashoffset,
	TAG_SVG_ATT_stroke_linecap,
	TAG_SVG_ATT_stroke_linejoin,
	TAG_SVG_ATT_stroke_miterlimit,
	TAG_SVG_ATT_stroke_width,
	TAG_SVG_ATT_color,
	TAG_SVG_ATT_color_rendering,
	TAG_SVG_ATT_vector_effect,
	TAG_SVG_ATT_solid_color,
	TAG_SVG_ATT_solid_opacity,
	TAG_SVG_ATT_display_align,
	TAG_SVG_ATT_line_increment,
	TAG_SVG_ATT_stop_color,
	TAG_SVG_ATT_stop_opacity,
	TAG_SVG_ATT_font_family,
	TAG_SVG_ATT_font_size,
	TAG_SVG_ATT_font_style,
	TAG_SVG_ATT_font_variant,
	TAG_SVG_ATT_font_weight,
	TAG_SVG_ATT_text_anchor,
	TAG_SVG_ATT_text_align,
	TAG_SVG_ATT_text_decoration,
	TAG_SVG_ATT_focusHighlight,
	TAG_SVG_ATT_externalResourcesRequired,
	TAG_SVG_ATT_focusable,
	TAG_SVG_ATT_nav_next,
	TAG_SVG_ATT_nav_prev,
	TAG_SVG_ATT_nav_up,
	TAG_SVG_ATT_nav_up_right,
	TAG_SVG_ATT_nav_right,
	TAG_SVG_ATT_nav_down_right,
	TAG_SVG_ATT_nav_down,
	TAG_SVG_ATT_nav_down_left,
	TAG_SVG_ATT_nav_left,
	TAG_SVG_ATT_nav_up_left,
	TAG_SVG_ATT_transform,
	TAG_SVG_ATT_target,
	TAG_SVG_ATT_attributeName,
	TAG_SVG_ATT_attributeType,
	TAG_SVG_ATT_begin,
	TAG_SVG_ATT_dur,
	TAG_SVG_ATT_end,
	TAG_SVG_ATT_repeatCount,
	TAG_SVG_ATT_repeatDur,
	TAG_SVG_ATT_restart,
	TAG_SVG_ATT_smil_fill,
	TAG_SVG_ATT_min,
	TAG_SVG_ATT_max,
	TAG_SVG_ATT_to,
	TAG_SVG_ATT_calcMode,
	TAG_SVG_ATT_values,
	TAG_SVG_ATT_keyTimes,
	TAG_SVG_ATT_keySplines,
	TAG_SVG_ATT_from,
	TAG_SVG_ATT_by,
	TAG_SVG_ATT_additive,
	TAG_SVG_ATT_accumulate,
	TAG_SVG_ATT_path,
	TAG_SVG_ATT_keyPoints,
	TAG_SVG_ATT_rotate,
	TAG_SVG_ATT_origin,
	TAG_SVG_ATT_transform_type,
	TAG_SVG_ATT_clipBegin,
	TAG_SVG_ATT_clipEnd,
	TAG_SVG_ATT_syncBehavior,
	TAG_SVG_ATT_syncTolerance,
	TAG_SVG_ATT_syncMaster,
	TAG_SVG_ATT_syncReference,
	TAG_SVG_ATT_x,
	TAG_SVG_ATT_y,
	TAG_SVG_ATT_width,
	TAG_SVG_ATT_height,
	TAG_SVG_ATT_preserveAspectRatio,
	TAG_SVG_ATT_initialVisibility,
	TAG_SVG_ATT_type,
	TAG_SVG_ATT_cx,
	TAG_SVG_ATT_cy,
	TAG_SVG_ATT_r,
	TAG_SVG_ATT_cursorManager_x,
	TAG_SVG_ATT_cursorManager_y,
	TAG_SVG_ATT_rx,
	TAG_SVG_ATT_ry,
	TAG_SVG_ATT_horiz_adv_x,
	TAG_SVG_ATT_horiz_origin_x,
	TAG_SVG_ATT_font_stretch,
	TAG_SVG_ATT_unicode_range,
	TAG_SVG_ATT_panose_1,
	TAG_SVG_ATT_widths,
	TAG_SVG_ATT_bbox,
	TAG_SVG_ATT_units_per_em,
	TAG_SVG_ATT_stemv,
	TAG_SVG_ATT_stemh,
	TAG_SVG_ATT_slope,
	TAG_SVG_ATT_cap_height,
	TAG_SVG_ATT_x_height,
	TAG_SVG_ATT_accent_height,
	TAG_SVG_ATT_ascent,
	TAG_SVG_ATT_descent,
	TAG_SVG_ATT_ideographic,
	TAG_SVG_ATT_alphabetic,
	TAG_SVG_ATT_mathematical,
	TAG_SVG_ATT_hanging,
	TAG_SVG_ATT_underline_position,
	TAG_SVG_ATT_underline_thickness,
	TAG_SVG_ATT_strikethrough_position,
	TAG_SVG_ATT_strikethrough_thickness,
	TAG_SVG_ATT_overline_position,
	TAG_SVG_ATT_overline_thickness,
	TAG_SVG_ATT_d,
	TAG_SVG_ATT_unicode,
	TAG_SVG_ATT_glyph_name,
	TAG_SVG_ATT_arabic_form,
	TAG_SVG_ATT_lang,
	TAG_SVG_ATT_u1,
	TAG_SVG_ATT_g1,
	TAG_SVG_ATT_u2,
	TAG_SVG_ATT_g2,
	TAG_SVG_ATT_k,
	TAG_SVG_ATT_opacity,
	TAG_SVG_ATT_x1,
	TAG_SVG_ATT_y1,
	TAG_SVG_ATT_x2,
	TAG_SVG_ATT_y2,
	TAG_SVG_ATT_filterUnits,
	TAG_SVG_ATT_gradientUnits,
	TAG_SVG_ATT_spreadMethod,
	TAG_SVG_ATT_gradientTransform,
	TAG_SVG_ATT_pathLength,
	TAG_SVG_ATT_points,
	TAG_SVG_ATT_mediaSize,
	TAG_SVG_ATT_mediaTime,
	TAG_SVG_ATT_mediaCharacterEncoding,
	TAG_SVG_ATT_mediaContentEncodings,
	TAG_SVG_ATT_bandwidth,
	TAG_SVG_ATT_fx,
	TAG_SVG_ATT_fy,
	TAG_SVG_ATT_size,
	TAG_SVG_ATT_choice,
	TAG_SVG_ATT_delta,
	TAG_SVG_ATT_offset,
	TAG_SVG_ATT_syncBehaviorDefault,
	TAG_SVG_ATT_syncToleranceDefault,
	TAG_SVG_ATT_viewBox,
	TAG_SVG_ATT_zoomAndPan,
	TAG_SVG_ATT_version,
	TAG_SVG_ATT_baseProfile,
	TAG_SVG_ATT_contentScriptType,
	TAG_SVG_ATT_snapshotTime,
	TAG_SVG_ATT_timelineBegin,
	TAG_SVG_ATT_playbackOrder,
	TAG_SVG_ATT_editable,
	TAG_SVG_ATT_text_x,
	TAG_SVG_ATT_text_y,
	TAG_SVG_ATT_text_rotate,
	TAG_SVG_ATT_transformBehavior,
	TAG_SVG_ATT_overlay,
	TAG_SVG_ATT_fullscreen,
	TAG_SVG_ATT_motionTransform,

	TAG_SVG_ATT_filter_transfer_type,
	TAG_SVG_ATT_filter_table_values,
	TAG_SVG_ATT_filter_intercept,
	TAG_SVG_ATT_filter_amplitude,
	TAG_SVG_ATT_filter_exponent

};


struct _all_atts {
	XML_Space *xml_space;
	XMLRI *xml_base;
	SVG_ID *xml_id;
	SVG_LanguageID *xml_lang;

	DOM_String *xlink_type;
	XMLRI *xlink_role;
	XMLRI *xlink_arcrole;
	DOM_String *xlink_title;
	XMLRI *xlink_href;
	DOM_String *xlink_show;
	DOM_String *xlink_actuate;

	XMLEV_Event *event;
	XMLEV_Phase *phase;
	XMLEV_Propagate *propagate;
	XMLEV_DefaultAction *defaultAction;
	XML_IDREF *observer;
	XML_IDREF *listener_target;
	XMLRI *handler;

	SVG_ID *id;
	SVG_String *_class;
	SVG_ListOfIRI *requiredFeatures;
	SVG_ListOfIRI *requiredExtensions;
	SVG_FormatList *requiredFormats;
	SVG_FontList *requiredFonts;
	SVG_LanguageIDs *systemLanguage;
	SVG_Display *display;
	SVG_Visibility *visibility;
	SVG_RenderingHint *image_rendering;
	SVG_PointerEvents *pointer_events;
	SVG_RenderingHint *shape_rendering;
	SVG_RenderingHint *text_rendering;
	SVG_Number *audio_level;
	SVG_Paint *viewport_fill;
	SVG_Number *viewport_fill_opacity;
	SVG_String *overflow;
	SVG_Number *fill_opacity;
	SVG_Number *stroke_opacity;
	SVG_Paint *fill;
	SVG_FillRule *fill_rule;
	SVG_Paint *filter;
	SVG_Paint *stroke;
	SVG_StrokeDashArray *stroke_dasharray;
	SVG_Length *stroke_dashoffset;
	SVG_StrokeLineCap *stroke_linecap;
	SVG_StrokeLineJoin *stroke_linejoin;
	SVG_Number *stroke_miterlimit;
	SVG_Length *stroke_width;
	SVG_Paint *color;
	SVG_RenderingHint *color_rendering;
	SVG_VectorEffect *vector_effect;
	SVG_SVGColor *solid_color;
	SVG_Number *solid_opacity;
	SVG_DisplayAlign *display_align;
	SVG_Number *line_increment;
	SVG_SVGColor *stop_color;
	SVG_Number *stop_opacity;
	SVG_FontFamily *font_family;
	SVG_FontSize *font_size;
	SVG_FontStyle *font_style;
	SVG_FontVariant *font_variant;
	SVG_FontWeight *font_weight;
	SVG_TextAnchor *text_anchor;
	SVG_TextAlign *text_align;
	SVG_String *text_decoration;
	SVG_FocusHighlight *focusHighlight;
	SVG_Boolean *externalResourcesRequired;
	SVG_Focusable *focusable;
	SVG_Focus *nav_next;
	SVG_Focus *nav_prev;
	SVG_Focus *nav_up;
	SVG_Focus *nav_up_right;
	SVG_Focus *nav_right;
	SVG_Focus *nav_down_right;
	SVG_Focus *nav_down;
	SVG_Focus *nav_down_left;
	SVG_Focus *nav_left;
	SVG_Focus *nav_up_left;
	SVG_Transform *transform;
	SVG_String *target;
	SMIL_AttributeName *attributeName;
	SMIL_AttributeType *attributeType;
	SMIL_Times *begin;
	SVG_Boolean *lsr_enabled;
	SMIL_Duration *dur;
	SMIL_Times *end;
	SMIL_RepeatCount *repeatCount;
	SMIL_Duration *repeatDur;
	SMIL_Restart *restart;
	SMIL_Fill *smil_fill;
	SMIL_Duration *min;
	SMIL_Duration *max;
	SMIL_AnimateValue *to;
	SMIL_CalcMode *calcMode;
	SMIL_AnimateValues *values;
	SMIL_KeyTimes *keyTimes;
	SMIL_KeySplines *keySplines;
	SMIL_AnimateValue *from;
	SMIL_AnimateValue *by;
	SMIL_Additive *additive;
	SMIL_Accumulate *accumulate;
	SVG_PathData *path;
	SMIL_KeyPoints *keyPoints;
	SVG_Rotate *rotate;
	SVG_String *origin;
	SVG_TransformType *transform_type;
	SVG_Clock *clipBegin;
	SVG_Clock *clipEnd;
	SMIL_SyncBehavior *syncBehavior;
	SMIL_SyncTolerance *syncTolerance;
	SVG_Boolean *syncMaster;
	XMLRI *syncReference;
	SVG_Coordinate *x;
	SVG_Coordinate *y;
	SVG_Length *width;
	SVG_Length *height;
	SVG_PreserveAspectRatio *preserveAspectRatio;
	SVG_InitialVisibility *initialVisibility;
	SVG_ContentType *type;
	SVG_Coordinate *cx;
	SVG_Coordinate *cy;
	SVG_Length *r;
	SVG_Length *cursorManager_x;
	SVG_Length *cursorManager_y;
	SVG_Length *rx;
	SVG_Length *ry;
	SVG_Number *horiz_adv_x;
	SVG_Number *horiz_origin_x;
	SVG_String *font_stretch;
	SVG_String *unicode_range;
	SVG_String *panose_1;
	SVG_String *widths;
	SVG_String *bbox;
	SVG_Number *units_per_em;
	SVG_Number *stemv;
	SVG_Number *stemh;
	SVG_Number *slope;
	SVG_Number *cap_height;
	SVG_Number *x_height;
	SVG_Number *accent_height;
	SVG_Number *ascent;
	SVG_Number *descent;
	SVG_Number *ideographic;
	SVG_Number *alphabetic;
	SVG_Number *mathematical;
	SVG_Number *hanging;
	SVG_Number *underline_position;
	SVG_Number *underline_thickness;
	SVG_Number *strikethrough_position;
	SVG_Number *strikethrough_thickness;
	SVG_Number *overline_position;
	SVG_Number *overline_thickness;
	SVG_PathData *d;
	SVG_String *unicode;
	SVG_String *glyph_name;
	SVG_String *arabic_form;
	SVG_LanguageIDs *lang;
	SVG_String *u1;
	SVG_String *g1;
	SVG_String *u2;
	SVG_String *g2;
	SVG_Number *k;
	SVG_Number *opacity;
	SVG_Coordinate *x1;
	SVG_Coordinate *y1;
	SVG_Coordinate *x2;
	SVG_Coordinate *y2;
	SVG_GradientUnit *gradientUnits;
	SVG_GradientUnit *filterUnits;
	SVG_SpreadMethod *spreadMethod;
	SVG_Transform *gradientTransform;
	SVG_Number *pathLength;
	SVG_Points *points;
	SVG_Number *mediaSize;
	SVG_String *mediaTime;
	SVG_String *mediaCharacterEncoding;
	SVG_String *mediaContentEncodings;
	SVG_Number *bandwidth;
	SVG_Coordinate *fx;
	SVG_Coordinate *fy;
	LASeR_Size *size;
	LASeR_Choice *choice;
	LASeR_Size *delta;
	SVG_Number *offset;
	SMIL_SyncBehavior *syncBehaviorDefault;
	SMIL_SyncTolerance *syncToleranceDefault;
	SVG_ViewBox *viewBox;
	SVG_ZoomAndPan *zoomAndPan;
	SVG_String *version;
	SVG_String *baseProfile;
	SVG_ContentType *contentScriptType;
	SVG_Clock *snapshotTime;
	SVG_TimelineBegin *timelineBegin;
	SVG_PlaybackOrder *playbackOrder;
	SVG_Boolean *editable;
	SVG_Coordinates *text_x;
	SVG_Coordinates *text_y;
	SVG_Numbers *text_rotate;
	SVG_TransformBehavior *transformBehavior;
	SVG_Overlay *overlay;
	SVG_Boolean *fullscreen;
	SVG_Motion *motionTransform;

    SVG_Boolean *gpac_useAsPrimary;
    SVG_Number *gpac_depthOffset;
    SVG_Number *gpac_depthGain;
};
#ifdef __cplusplus
}
#endif



#endif		/*_GF_SVG_NODES_H*/

