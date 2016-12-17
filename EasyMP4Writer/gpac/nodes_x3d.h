/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005
 *					All rights reserved
 *
 *  This file is part of GPAC / X3D Scene Graph sub-project
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


/*
	DO NOT MOFIFY - File generated on GMT Fri Jul 31 16:39:50 2009

	BY X3DGen for GPAC Version 0.4.6-DEV
*/

#ifndef _GF_X3D_NODES_H
#define _GF_X3D_NODES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/scenegraph_vrml.h>

#ifndef GPAC_DISABLE_X3D



enum {
	TAG_X3D_Anchor = GF_NODE_RANGE_FIRST_X3D,
	TAG_X3D_Appearance,
	TAG_X3D_Arc2D,
	TAG_X3D_ArcClose2D,
	TAG_X3D_AudioClip,
	TAG_X3D_Background,
	TAG_X3D_Billboard,
	TAG_X3D_BooleanFilter,
	TAG_X3D_BooleanSequencer,
	TAG_X3D_BooleanToggle,
	TAG_X3D_BooleanTrigger,
	TAG_X3D_Box,
	TAG_X3D_Circle2D,
	TAG_X3D_Collision,
	TAG_X3D_Color,
	TAG_X3D_ColorInterpolator,
	TAG_X3D_ColorRGBA,
	TAG_X3D_Cone,
	TAG_X3D_Contour2D,
	TAG_X3D_ContourPolyline2D,
	TAG_X3D_Coordinate,
	TAG_X3D_CoordinateDouble,
	TAG_X3D_Coordinate2D,
	TAG_X3D_CoordinateInterpolator,
	TAG_X3D_CoordinateInterpolator2D,
	TAG_X3D_Cylinder,
	TAG_X3D_CylinderSensor,
	TAG_X3D_DirectionalLight,
	TAG_X3D_Disk2D,
	TAG_X3D_ElevationGrid,
	TAG_X3D_EspduTransform,
	TAG_X3D_Extrusion,
	TAG_X3D_FillProperties,
	TAG_X3D_Fog,
	TAG_X3D_FontStyle,
	TAG_X3D_GeoCoordinate,
	TAG_X3D_GeoElevationGrid,
	TAG_X3D_GeoLocation,
	TAG_X3D_GeoLOD,
	TAG_X3D_GeoMetadata,
	TAG_X3D_GeoOrigin,
	TAG_X3D_GeoPositionInterpolator,
	TAG_X3D_GeoTouchSensor,
	TAG_X3D_GeoViewpoint,
	TAG_X3D_Group,
	TAG_X3D_HAnimDisplacer,
	TAG_X3D_HAnimHumanoid,
	TAG_X3D_HAnimJoint,
	TAG_X3D_HAnimSegment,
	TAG_X3D_HAnimSite,
	TAG_X3D_ImageTexture,
	TAG_X3D_IndexedFaceSet,
	TAG_X3D_IndexedLineSet,
	TAG_X3D_IndexedTriangleFanSet,
	TAG_X3D_IndexedTriangleSet,
	TAG_X3D_IndexedTriangleStripSet,
	TAG_X3D_Inline,
	TAG_X3D_IntegerSequencer,
	TAG_X3D_IntegerTrigger,
	TAG_X3D_KeySensor,
	TAG_X3D_LineProperties,
	TAG_X3D_LineSet,
	TAG_X3D_LoadSensor,
	TAG_X3D_LOD,
	TAG_X3D_Material,
	TAG_X3D_MetadataDouble,
	TAG_X3D_MetadataFloat,
	TAG_X3D_MetadataInteger,
	TAG_X3D_MetadataSet,
	TAG_X3D_MetadataString,
	TAG_X3D_MovieTexture,
	TAG_X3D_MultiTexture,
	TAG_X3D_MultiTextureCoordinate,
	TAG_X3D_MultiTextureTransform,
	TAG_X3D_NavigationInfo,
	TAG_X3D_Normal,
	TAG_X3D_NormalInterpolator,
	TAG_X3D_NurbsCurve,
	TAG_X3D_NurbsCurve2D,
	TAG_X3D_NurbsOrientationInterpolator,
	TAG_X3D_NurbsPatchSurface,
	TAG_X3D_NurbsPositionInterpolator,
	TAG_X3D_NurbsSet,
	TAG_X3D_NurbsSurfaceInterpolator,
	TAG_X3D_NurbsSweptSurface,
	TAG_X3D_NurbsSwungSurface,
	TAG_X3D_NurbsTextureCoordinate,
	TAG_X3D_NurbsTrimmedSurface,
	TAG_X3D_OrientationInterpolator,
	TAG_X3D_PixelTexture,
	TAG_X3D_PlaneSensor,
	TAG_X3D_PointLight,
	TAG_X3D_PointSet,
	TAG_X3D_Polyline2D,
	TAG_X3D_Polypoint2D,
	TAG_X3D_PositionInterpolator,
	TAG_X3D_PositionInterpolator2D,
	TAG_X3D_ProximitySensor,
	TAG_X3D_ReceiverPdu,
	TAG_X3D_Rectangle2D,
	TAG_X3D_ScalarInterpolator,
	TAG_X3D_Script,
	TAG_X3D_Shape,
	TAG_X3D_SignalPdu,
	TAG_X3D_Sound,
	TAG_X3D_Sphere,
	TAG_X3D_SphereSensor,
	TAG_X3D_SpotLight,
	TAG_X3D_StaticGroup,
	TAG_X3D_StringSensor,
	TAG_X3D_Switch,
	TAG_X3D_Text,
	TAG_X3D_TextureBackground,
	TAG_X3D_TextureCoordinate,
	TAG_X3D_TextureCoordinateGenerator,
	TAG_X3D_TextureTransform,
	TAG_X3D_TimeSensor,
	TAG_X3D_TimeTrigger,
	TAG_X3D_TouchSensor,
	TAG_X3D_Transform,
	TAG_X3D_TransmitterPdu,
	TAG_X3D_TriangleFanSet,
	TAG_X3D_TriangleSet,
	TAG_X3D_TriangleSet2D,
	TAG_X3D_TriangleStripSet,
	TAG_X3D_Viewpoint,
	TAG_X3D_VisibilitySensor,
	TAG_X3D_WorldInfo,
	TAG_LastImplementedX3D
};

typedef struct _tagX3DAnchor
{
	BASE_NODE
	VRML_CHILDREN
	SFString description;	/*exposedField*/
	MFString parameter;	/*exposedField*/
	MFURL url;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_Anchor;


typedef struct _tagX3DAppearance
{
	BASE_NODE
	GF_Node *material;	/*exposedField*/
	GF_Node *texture;	/*exposedField*/
	GF_Node *textureTransform;	/*exposedField*/
	GF_Node *fillProperties;	/*exposedField*/
	GF_Node *lineProperties;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_Appearance;


typedef struct _tagX3DArc2D
{
	BASE_NODE
	SFFloat endAngle;	/*field*/
	SFFloat radius;	/*field*/
	SFFloat startAngle;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_Arc2D;


typedef struct _tagX3DArcClose2D
{
	BASE_NODE
	SFString closureType;	/*field*/
	SFFloat endAngle;	/*field*/
	SFFloat radius;	/*field*/
	SFFloat startAngle;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_ArcClose2D;


typedef struct _tagX3DAudioClip
{
	BASE_NODE
	SFString description;	/*exposedField*/
	SFBool loop;	/*exposedField*/
	SFFloat pitch;	/*exposedField*/
	SFTime startTime;	/*exposedField*/
	SFTime stopTime;	/*exposedField*/
	MFURL url;	/*exposedField*/
	SFTime duration_changed;	/*eventOut*/
	SFBool isActive;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
	SFTime pauseTime;	/*exposedField*/
	SFTime resumeTime;	/*exposedField*/
	SFTime elapsedTime;	/*eventOut*/
	SFBool isPaused;	/*eventOut*/
} X_AudioClip;


typedef struct _tagX3DBackground
{
	BASE_NODE
	SFBool set_bind;	/*eventIn*/
	void (*on_set_bind)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat groundAngle;	/*exposedField*/
	MFColor groundColor;	/*exposedField*/
	MFURL backUrl;	/*exposedField*/
	MFURL bottomUrl;	/*exposedField*/
	MFURL frontUrl;	/*exposedField*/
	MFURL leftUrl;	/*exposedField*/
	MFURL rightUrl;	/*exposedField*/
	MFURL topUrl;	/*exposedField*/
	MFFloat skyAngle;	/*exposedField*/
	MFColor skyColor;	/*exposedField*/
	SFBool isBound;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
	SFTime bindTime;	/*eventOut*/
} X_Background;


typedef struct _tagX3DBillboard
{
	BASE_NODE
	VRML_CHILDREN
	SFVec3f axisOfRotation;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_Billboard;


typedef struct _tagX3DBooleanFilter
{
	BASE_NODE
	SFBool set_boolean;	/*eventIn*/
	void (*on_set_boolean)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFBool inputFalse;	/*eventOut*/
	SFBool inputNegate;	/*eventOut*/
	SFBool inputTrue;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_BooleanFilter;


typedef struct _tagX3DBooleanSequencer
{
	BASE_NODE
	SFBool next;	/*eventIn*/
	void (*on_next)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFBool previous;	/*eventIn*/
	void (*on_previous)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat key;	/*exposedField*/
	MFBool keyValue;	/*exposedField*/
	SFBool value_changed;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_BooleanSequencer;


typedef struct _tagX3DBooleanToggle
{
	BASE_NODE
	SFBool set_boolean;	/*eventIn*/
	void (*on_set_boolean)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFBool toggle;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_BooleanToggle;


typedef struct _tagX3DBooleanTrigger
{
	BASE_NODE
	SFTime set_triggerTime;	/*eventIn*/
	void (*on_set_triggerTime)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFBool triggerTrue;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_BooleanTrigger;


typedef struct _tagX3DBox
{
	BASE_NODE
	SFVec3f size;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_Box;


typedef struct _tagX3DCircle2D
{
	BASE_NODE
	SFFloat radius;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_Circle2D;


typedef struct _tagX3DCollision
{
	BASE_NODE
	VRML_CHILDREN
	SFBool enabled;	/*exposedField*/
	GF_Node *proxy;	/*field*/
	SFTime collideTime;	/*eventOut*/
	SFBool isActive;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_Collision;


typedef struct _tagX3DColor
{
	BASE_NODE
	MFColor color;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_Color;


typedef struct _tagX3DColorInterpolator
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat key;	/*exposedField*/
	MFColor keyValue;	/*exposedField*/
	SFColor value_changed;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_ColorInterpolator;


typedef struct _tagX3DColorRGBA
{
	BASE_NODE
	MFColorRGBA color;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_ColorRGBA;


typedef struct _tagX3DCone
{
	BASE_NODE
	SFFloat bottomRadius;	/*field*/
	SFFloat height;	/*field*/
	SFBool side;	/*field*/
	SFBool bottom;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_Cone;


typedef struct _tagX3DContour2D
{
	BASE_NODE
	VRML_CHILDREN
	GF_Node *metadata;	/*exposedField*/
} X_Contour2D;


typedef struct _tagX3DContourPolyline2D
{
	BASE_NODE
	MFVec2f point;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_ContourPolyline2D;


typedef struct _tagX3DCoordinate
{
	BASE_NODE
	MFVec3f point;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_Coordinate;


typedef struct _tagX3DCoordinateDouble
{
	BASE_NODE
	MFVec3d point;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_CoordinateDouble;


typedef struct _tagX3DCoordinate2D
{
	BASE_NODE
	MFVec2f point;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_Coordinate2D;


typedef struct _tagX3DCoordinateInterpolator
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat key;	/*exposedField*/
	MFVec3f keyValue;	/*exposedField*/
	MFVec3f value_changed;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_CoordinateInterpolator;


typedef struct _tagX3DCoordinateInterpolator2D
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat key;	/*exposedField*/
	MFVec2f keyValue;	/*exposedField*/
	MFVec2f value_changed;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_CoordinateInterpolator2D;


typedef struct _tagX3DCylinder
{
	BASE_NODE
	SFBool bottom;	/*field*/
	SFFloat height;	/*field*/
	SFFloat radius;	/*field*/
	SFBool side;	/*field*/
	SFBool top;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_Cylinder;


typedef struct _tagX3DCylinderSensor
{
	BASE_NODE
	SFBool autoOffset;	/*exposedField*/
	SFFloat diskAngle;	/*exposedField*/
	SFBool enabled;	/*exposedField*/
	SFFloat maxAngle;	/*exposedField*/
	SFFloat minAngle;	/*exposedField*/
	SFFloat offset;	/*exposedField*/
	SFBool isActive;	/*eventOut*/
	SFRotation rotation_changed;	/*eventOut*/
	SFVec3f trackPoint_changed;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
	SFString description;	/*exposedField*/
	SFBool isOver;	/*eventOut*/
} X_CylinderSensor;


typedef struct _tagX3DDirectionalLight
{
	BASE_NODE
	SFFloat ambientIntensity;	/*exposedField*/
	SFColor color;	/*exposedField*/
	SFVec3f direction;	/*exposedField*/
	SFFloat intensity;	/*exposedField*/
	SFBool on;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_DirectionalLight;


typedef struct _tagX3DDisk2D
{
	BASE_NODE
	SFFloat innerRadius;	/*field*/
	SFFloat outerRadius;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_Disk2D;


typedef struct _tagX3DElevationGrid
{
	BASE_NODE
	MFFloat set_height;	/*eventIn*/
	void (*on_set_height)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	GF_Node *color;	/*exposedField*/
	GF_Node *normal;	/*exposedField*/
	GF_Node *texCoord;	/*exposedField*/
	MFFloat height;	/*field*/
	SFBool ccw;	/*field*/
	SFBool colorPerVertex;	/*field*/
	SFFloat creaseAngle;	/*field*/
	SFBool normalPerVertex;	/*field*/
	SFBool solid;	/*field*/
	SFInt32 xDimension;	/*field*/
	SFFloat xSpacing;	/*field*/
	SFInt32 zDimension;	/*field*/
	SFFloat zSpacing;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_ElevationGrid;


typedef struct _tagX3DEspduTransform
{
	BASE_NODE
	VRML_CHILDREN
	SFFloat set_articulationParameterValue0;	/*eventIn*/
	void (*on_set_articulationParameterValue0)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFFloat set_articulationParameterValue1;	/*eventIn*/
	void (*on_set_articulationParameterValue1)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFFloat set_articulationParameterValue2;	/*eventIn*/
	void (*on_set_articulationParameterValue2)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFFloat set_articulationParameterValue3;	/*eventIn*/
	void (*on_set_articulationParameterValue3)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFFloat set_articulationParameterValue4;	/*eventIn*/
	void (*on_set_articulationParameterValue4)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFFloat set_articulationParameterValue5;	/*eventIn*/
	void (*on_set_articulationParameterValue5)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFFloat set_articulationParameterValue6;	/*eventIn*/
	void (*on_set_articulationParameterValue6)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFFloat set_articulationParameterValue7;	/*eventIn*/
	void (*on_set_articulationParameterValue7)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFString address;	/*exposedField*/
	SFInt32 applicationID;	/*exposedField*/
	SFInt32 articulationParameterCount;	/*exposedField*/
	MFInt32 articulationParameterDesignatorArray;	/*exposedField*/
	MFInt32 articulationParameterChangeIndicatorArray;	/*exposedField*/
	MFInt32 articulationParameterIdPartAttachedToArray;	/*exposedField*/
	MFInt32 articulationParameterTypeArray;	/*exposedField*/
	MFFloat articulationParameterArray;	/*exposedField*/
	SFVec3f center;	/*exposedField*/
	SFInt32 collisionType;	/*exposedField*/
	SFInt32 deadReckoning;	/*exposedField*/
	SFVec3f detonationLocation;	/*exposedField*/
	SFVec3f detonationRelativeLocation;	/*exposedField*/
	SFInt32 detonationResult;	/*exposedField*/
	SFInt32 entityCategory;	/*exposedField*/
	SFInt32 entityCountry;	/*exposedField*/
	SFInt32 entityDomain;	/*exposedField*/
	SFInt32 entityExtra;	/*exposedField*/
	SFInt32 entityID;	/*exposedField*/
	SFInt32 entityKind;	/*exposedField*/
	SFInt32 entitySpecific;	/*exposedField*/
	SFInt32 entitySubCategory;	/*exposedField*/
	SFInt32 eventApplicationID;	/*exposedField*/
	SFInt32 eventEntityID;	/*exposedField*/
	SFInt32 eventNumber;	/*exposedField*/
	SFInt32 eventSiteID;	/*exposedField*/
	SFBool fired1;	/*exposedField*/
	SFBool fired2;	/*exposedField*/
	SFInt32 fireMissionIndex;	/*exposedField*/
	SFFloat firingRange;	/*exposedField*/
	SFInt32 firingRate;	/*exposedField*/
	SFInt32 forceID;	/*exposedField*/
	SFInt32 fuse;	/*exposedField*/
	SFVec3f linearVelocity;	/*exposedField*/
	SFVec3f linearAcceleration;	/*exposedField*/
	SFString marking;	/*exposedField*/
	SFString multicastRelayHost;	/*exposedField*/
	SFInt32 multicastRelayPort;	/*exposedField*/
	SFInt32 munitionApplicationID;	/*exposedField*/
	SFVec3f munitionEndPoint;	/*exposedField*/
	SFInt32 munitionEntityID;	/*exposedField*/
	SFInt32 munitionQuantity;	/*exposedField*/
	SFInt32 munitionSiteID;	/*exposedField*/
	SFVec3f munitionStartPoint;	/*exposedField*/
	SFString networkMode;	/*exposedField*/
	SFInt32 port;	/*exposedField*/
	SFTime readInterval;	/*exposedField*/
	SFRotation rotation;	/*exposedField*/
	SFVec3f scale;	/*exposedField*/
	SFRotation scaleOrientation;	/*exposedField*/
	SFInt32 siteID;	/*exposedField*/
	SFVec3f translation;	/*exposedField*/
	SFInt32 warhead;	/*exposedField*/
	SFTime writeInterval;	/*exposedField*/
	SFBool rtpHeaderExpected;	/*field*/
	SFFloat articulationParameterValue0_changed;	/*eventOut*/
	SFFloat articulationParameterValue1_changed;	/*eventOut*/
	SFFloat articulationParameterValue2_changed;	/*eventOut*/
	SFFloat articulationParameterValue3_changed;	/*eventOut*/
	SFFloat articulationParameterValue4_changed;	/*eventOut*/
	SFFloat articulationParameterValue5_changed;	/*eventOut*/
	SFFloat articulationParameterValue6_changed;	/*eventOut*/
	SFFloat articulationParameterValue7_changed;	/*eventOut*/
	SFTime collideTime;	/*eventOut*/
	SFTime detonateTime;	/*eventOut*/
	SFTime firedTime;	/*eventOut*/
	SFBool isActive;	/*eventOut*/
	SFBool isCollided;	/*eventOut*/
	SFBool isDetonated;	/*eventOut*/
	SFBool isNetworkReader;	/*eventOut*/
	SFBool isNetworkWriter;	/*eventOut*/
	SFBool isRtpHeaderHeard;	/*eventOut*/
	SFBool isStandAlone;	/*eventOut*/
	SFTime timestamp;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_EspduTransform;


typedef struct _tagX3DExtrusion
{
	BASE_NODE
	MFVec2f set_crossSection;	/*eventIn*/
	void (*on_set_crossSection)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFRotation set_orientation;	/*eventIn*/
	void (*on_set_orientation)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFVec2f set_scale;	/*eventIn*/
	void (*on_set_scale)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFVec3f set_spine;	/*eventIn*/
	void (*on_set_spine)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFBool beginCap;	/*field*/
	SFBool ccw;	/*field*/
	SFBool convex;	/*field*/
	SFFloat creaseAngle;	/*field*/
	MFVec2f crossSection;	/*field*/
	SFBool endCap;	/*field*/
	MFRotation orientation;	/*field*/
	MFVec2f scale;	/*field*/
	SFBool solid;	/*field*/
	MFVec3f spine;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_Extrusion;


typedef struct _tagX3DFillProperties
{
	BASE_NODE
	SFBool filled;	/*exposedField*/
	SFColor hatchColor;	/*exposedField*/
	SFBool hatched;	/*exposedField*/
	SFInt32 hatchStyle;	/*exposedField*/
} X_FillProperties;


typedef struct _tagX3DFog
{
	BASE_NODE
	SFColor color;	/*exposedField*/
	SFString fogType;	/*exposedField*/
	SFFloat visibilityRange;	/*exposedField*/
	SFBool set_bind;	/*eventIn*/
	void (*on_set_bind)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFBool isBound;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
	SFTime bindTime;	/*eventOut*/
} X_Fog;


typedef struct _tagX3DFontStyle
{
	BASE_NODE
	MFString family;	/*exposedField*/
	SFBool horizontal;	/*exposedField*/
	MFString justify;	/*exposedField*/
	SFString language;	/*exposedField*/
	SFBool leftToRight;	/*exposedField*/
	SFFloat size;	/*exposedField*/
	SFFloat spacing;	/*exposedField*/
	SFString style;	/*exposedField*/
	SFBool topToBottom;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_FontStyle;


typedef struct _tagX3DGeoCoordinate
{
	BASE_NODE
	MFVec3d point;	/*exposedField*/
	GF_Node *geoOrigin;	/*field*/
	MFString geoSystem;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_GeoCoordinate;


typedef struct _tagX3DGeoElevationGrid
{
	BASE_NODE
	MFDouble set_height;	/*eventIn*/
	void (*on_set_height)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	GF_Node *color;	/*exposedField*/
	GF_Node *normal;	/*exposedField*/
	GF_Node *texCoord;	/*exposedField*/
	SFFloat yScale;	/*exposedField*/
	SFBool ccw;	/*field*/
	SFBool colorPerVertex;	/*field*/
	SFFloat creaseAngle;	/*field*/
	SFString geoGridOrigin;	/*field*/
	GF_Node *geoOrigin;	/*field*/
	MFString geoSystem;	/*field*/
	MFDouble height;	/*field*/
	SFBool normalPerVertex;	/*field*/
	SFBool solid;	/*field*/
	SFInt32 xDimension;	/*field*/
	SFDouble xSpacing;	/*field*/
	SFInt32 zDimension;	/*field*/
	SFDouble zSpacing;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_GeoElevationGrid;


typedef struct _tagX3DGeoLocation
{
	BASE_NODE
	VRML_CHILDREN
	SFVec3d geoCoords;	/*exposedField*/
	GF_Node *geoOrigin;	/*field*/
	MFString geoSystem;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_GeoLocation;


typedef struct _tagX3DGeoLOD
{
	BASE_NODE
	SFVec3d center;	/*field*/
	MFURL child1Url;	/*field*/
	MFURL child2Url;	/*field*/
	MFURL child3Url;	/*field*/
	MFURL child4Url;	/*field*/
	GF_Node *geoOrigin;	/*field*/
	MFString geoSystem;	/*field*/
	SFFloat range;	/*field*/
	MFURL rootUrl;	/*field*/
	GF_ChildNodeItem *rootNode;	/*field*/
	GF_ChildNodeItem *children;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_GeoLOD;


typedef struct _tagX3DGeoMetadata
{
	BASE_NODE
	GF_ChildNodeItem *data;	/*exposedField*/
	MFString summary;	/*exposedField*/
	MFURL url;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_GeoMetadata;


typedef struct _tagX3DGeoOrigin
{
	BASE_NODE
	SFVec3d geoCoords;	/*exposedField*/
	MFString geoSystem;	/*exposedField*/
	SFBool rotateYUp;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_GeoOrigin;


typedef struct _tagX3DGeoPositionInterpolator
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat key;	/*exposedField*/
	MFVec3d keyValue;	/*exposedField*/
	GF_Node *geoOrigin;	/*field*/
	MFString geoSystem;	/*field*/
	SFVec3d geovalue_changed;	/*eventOut*/
	SFVec3f value_changed;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_GeoPositionInterpolator;


typedef struct _tagX3DGeoTouchSensor
{
	BASE_NODE
	SFBool enabled;	/*exposedField*/
	GF_Node *geoOrigin;	/*field*/
	MFString geoSystem;	/*field*/
	SFVec3f hitNormal_changed;	/*eventOut*/
	SFVec3f hitPoint_changed;	/*eventOut*/
	SFVec2f hitTexCoord_changed;	/*eventOut*/
	SFVec3d hitGeoCoord_changed;	/*eventOut*/
	SFBool isActive;	/*eventOut*/
	SFBool isOver;	/*eventOut*/
	SFTime touchTime;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_GeoTouchSensor;


typedef struct _tagX3DGeoViewpoint
{
	BASE_NODE
	SFBool set_bind;	/*eventIn*/
	void (*on_set_bind)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFString set_orientation;	/*eventIn*/
	void (*on_set_orientation)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFString set_position;	/*eventIn*/
	void (*on_set_position)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFString description;	/*exposedField*/
	SFFloat fieldOfView;	/*exposedField*/
	SFBool headlight;	/*exposedField*/
	SFBool jump;	/*exposedField*/
	MFString navType;	/*exposedField*/
	SFTime bindTime;	/*eventOut*/
	SFBool isBound;	/*eventOut*/
	GF_Node *geoOrigin;	/*field*/
	MFString geoSystem;	/*field*/
	SFRotation orientation;	/*field*/
	SFVec3d position;	/*field*/
	SFFloat speedFactor;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_GeoViewpoint;


typedef struct _tagX3DGroup
{
	BASE_NODE
	VRML_CHILDREN
	GF_Node *metadata;	/*exposedField*/
} X_Group;


typedef struct _tagX3DHAnimDisplacer
{
	BASE_NODE
	MFInt32 coordIndex;	/*exposedField*/
	MFVec3f displacements;	/*exposedField*/
	SFString name;	/*exposedField*/
	SFFloat weight;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_HAnimDisplacer;


typedef struct _tagX3DHAnimHumanoid
{
	BASE_NODE
	SFVec3f center;	/*exposedField*/
	MFString info;	/*exposedField*/
	GF_ChildNodeItem *joints;	/*exposedField*/
	SFString name;	/*exposedField*/
	SFRotation rotation;	/*exposedField*/
	SFVec3f scale;	/*exposedField*/
	SFRotation scaleOrientation;	/*exposedField*/
	GF_ChildNodeItem *segments;	/*exposedField*/
	GF_ChildNodeItem *sites;	/*exposedField*/
	GF_ChildNodeItem *skeleton;	/*exposedField*/
	GF_ChildNodeItem *skin;	/*exposedField*/
	GF_Node *skinCoord;	/*exposedField*/
	GF_Node *skinNormal;	/*exposedField*/
	SFVec3f translation;	/*exposedField*/
	SFString version;	/*exposedField*/
	GF_ChildNodeItem *viewpoints;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_HAnimHumanoid;


typedef struct _tagX3DHAnimJoint
{
	BASE_NODE
	VRML_CHILDREN
	SFVec3f center;	/*exposedField*/
	GF_ChildNodeItem *displacers;	/*exposedField*/
	SFRotation limitOrientation;	/*exposedField*/
	MFFloat llimit;	/*exposedField*/
	SFString name;	/*exposedField*/
	SFRotation rotation;	/*exposedField*/
	SFVec3f scale;	/*exposedField*/
	SFRotation scaleOrientation;	/*exposedField*/
	MFInt32 skinCoordIndex;	/*exposedField*/
	MFFloat skinCoordWeight;	/*exposedField*/
	MFFloat stiffness;	/*exposedField*/
	SFVec3f translation;	/*exposedField*/
	MFFloat ulimit;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_HAnimJoint;


typedef struct _tagX3DHAnimSegment
{
	BASE_NODE
	VRML_CHILDREN
	SFVec3f centerOfMass;	/*exposedField*/
	GF_Node *coord;	/*exposedField*/
	GF_ChildNodeItem *displacers;	/*exposedField*/
	SFFloat mass;	/*exposedField*/
	MFFloat momentsOfInertia;	/*exposedField*/
	SFString name;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_HAnimSegment;


typedef struct _tagX3DHAnimSite
{
	BASE_NODE
	VRML_CHILDREN
	SFVec3f center;	/*exposedField*/
	SFString name;	/*exposedField*/
	SFRotation rotation;	/*exposedField*/
	SFVec3f scale;	/*exposedField*/
	SFRotation scaleOrientation;	/*exposedField*/
	SFVec3f translation;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_HAnimSite;


typedef struct _tagX3DImageTexture
{
	BASE_NODE
	MFURL url;	/*exposedField*/
	SFBool repeatS;	/*field*/
	SFBool repeatT;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_ImageTexture;


typedef struct _tagX3DIndexedFaceSet
{
	BASE_NODE
	MFInt32 set_colorIndex;	/*eventIn*/
	void (*on_set_colorIndex)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFInt32 set_coordIndex;	/*eventIn*/
	void (*on_set_coordIndex)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFInt32 set_normalIndex;	/*eventIn*/
	void (*on_set_normalIndex)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFInt32 set_texCoordIndex;	/*eventIn*/
	void (*on_set_texCoordIndex)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	GF_Node *color;	/*exposedField*/
	GF_Node *coord;	/*exposedField*/
	GF_Node *normal;	/*exposedField*/
	GF_Node *texCoord;	/*exposedField*/
	SFBool ccw;	/*field*/
	MFInt32 colorIndex;	/*field*/
	SFBool colorPerVertex;	/*field*/
	SFBool convex;	/*field*/
	MFInt32 coordIndex;	/*field*/
	SFFloat creaseAngle;	/*field*/
	MFInt32 normalIndex;	/*field*/
	SFBool normalPerVertex;	/*field*/
	SFBool solid;	/*field*/
	MFInt32 texCoordIndex;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_IndexedFaceSet;


typedef struct _tagX3DIndexedLineSet
{
	BASE_NODE
	MFInt32 set_colorIndex;	/*eventIn*/
	void (*on_set_colorIndex)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFInt32 set_coordIndex;	/*eventIn*/
	void (*on_set_coordIndex)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	GF_Node *color;	/*exposedField*/
	GF_Node *coord;	/*exposedField*/
	MFInt32 colorIndex;	/*field*/
	SFBool colorPerVertex;	/*field*/
	MFInt32 coordIndex;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_IndexedLineSet;


typedef struct _tagX3DIndexedTriangleFanSet
{
	BASE_NODE
	MFInt32 set_index;	/*eventIn*/
	void (*on_set_index)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	GF_Node *color;	/*exposedField*/
	GF_Node *coord;	/*exposedField*/
	GF_Node *normal;	/*exposedField*/
	GF_Node *texCoord;	/*exposedField*/
	SFBool ccw;	/*field*/
	SFBool colorPerVertex;	/*field*/
	SFBool normalPerVertex;	/*field*/
	SFBool solid;	/*field*/
	MFInt32 index;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_IndexedTriangleFanSet;


typedef struct _tagX3DIndexedTriangleSet
{
	BASE_NODE
	MFInt32 set_index;	/*eventIn*/
	void (*on_set_index)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	GF_Node *color;	/*exposedField*/
	GF_Node *coord;	/*exposedField*/
	GF_Node *normal;	/*exposedField*/
	GF_Node *texCoord;	/*exposedField*/
	SFBool ccw;	/*field*/
	SFBool colorPerVertex;	/*field*/
	SFBool normalPerVertex;	/*field*/
	SFBool solid;	/*field*/
	MFInt32 index;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_IndexedTriangleSet;


typedef struct _tagX3DIndexedTriangleStripSet
{
	BASE_NODE
	MFInt32 set_index;	/*eventIn*/
	void (*on_set_index)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	GF_Node *color;	/*exposedField*/
	GF_Node *coord;	/*exposedField*/
	SFFloat creaseAngle;	/*exposedField*/
	GF_Node *normal;	/*exposedField*/
	GF_Node *texCoord;	/*exposedField*/
	SFBool ccw;	/*field*/
	SFBool normalPerVertex;	/*field*/
	SFBool solid;	/*field*/
	MFInt32 index;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_IndexedTriangleStripSet;


typedef struct _tagX3DInline
{
	BASE_NODE
	MFURL url;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
	SFBool load;	/*exposedField*/
} X_Inline;


typedef struct _tagX3DIntegerSequencer
{
	BASE_NODE
	SFBool next;	/*eventIn*/
	void (*on_next)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFBool previous;	/*eventIn*/
	void (*on_previous)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat key;	/*exposedField*/
	MFInt32 keyValue;	/*exposedField*/
	SFInt32 value_changed;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_IntegerSequencer;


typedef struct _tagX3DIntegerTrigger
{
	BASE_NODE
	SFBool set_boolean;	/*eventIn*/
	void (*on_set_boolean)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFInt32 integerKey;	/*exposedField*/
	SFInt32 triggerValue;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_IntegerTrigger;


typedef struct _tagX3DKeySensor
{
	BASE_NODE
	SFBool enabled;	/*exposedField*/
	SFInt32 actionKeyPress;	/*eventOut*/
	SFInt32 actionKeyRelease;	/*eventOut*/
	SFBool altKey;	/*eventOut*/
	SFBool controlKey;	/*eventOut*/
	SFBool isActive;	/*eventOut*/
	SFString keyPress;	/*eventOut*/
	SFString keyRelease;	/*eventOut*/
	SFBool shiftKey;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_KeySensor;


typedef struct _tagX3DLineProperties
{
	BASE_NODE
	SFBool applied;	/*exposedField*/
	SFInt32 linetype;	/*exposedField*/
	SFFloat linewidthScaleFactor;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_LineProperties;


typedef struct _tagX3DLineSet
{
	BASE_NODE
	GF_Node *color;	/*exposedField*/
	GF_Node *coord;	/*exposedField*/
	MFInt32 vertexCount;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_LineSet;


typedef struct _tagX3DLoadSensor
{
	BASE_NODE
	SFBool enabled;	/*exposedField*/
	SFTime timeOut;	/*exposedField*/
	GF_ChildNodeItem *watchList;	/*exposedField*/
	SFBool isActive;	/*eventOut*/
	SFBool isLoaded;	/*eventOut*/
	SFTime loadTime;	/*eventOut*/
	SFFloat progress;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_LoadSensor;


typedef struct _tagX3DLOD
{
	BASE_NODE
	VRML_CHILDREN
	SFVec3f center;	/*field*/
	MFFloat range;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_LOD;


typedef struct _tagX3DMaterial
{
	BASE_NODE
	SFFloat ambientIntensity;	/*exposedField*/
	SFColor diffuseColor;	/*exposedField*/
	SFColor emissiveColor;	/*exposedField*/
	SFFloat shininess;	/*exposedField*/
	SFColor specularColor;	/*exposedField*/
	SFFloat transparency;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_Material;


typedef struct _tagX3DMetadataDouble
{
	BASE_NODE
	SFString name;	/*exposedField*/
	SFString reference;	/*exposedField*/
	MFDouble value;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_MetadataDouble;


typedef struct _tagX3DMetadataFloat
{
	BASE_NODE
	SFString name;	/*exposedField*/
	SFString reference;	/*exposedField*/
	MFFloat value;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_MetadataFloat;


typedef struct _tagX3DMetadataInteger
{
	BASE_NODE
	SFString name;	/*exposedField*/
	SFString reference;	/*exposedField*/
	MFInt32 value;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_MetadataInteger;


typedef struct _tagX3DMetadataSet
{
	BASE_NODE
	SFString name;	/*exposedField*/
	SFString reference;	/*exposedField*/
	GF_ChildNodeItem *value;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_MetadataSet;


typedef struct _tagX3DMetadataString
{
	BASE_NODE
	SFString name;	/*exposedField*/
	SFString reference;	/*exposedField*/
	MFString value;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_MetadataString;


typedef struct _tagX3DMovieTexture
{
	BASE_NODE
	SFBool loop;	/*exposedField*/
	SFFloat speed;	/*exposedField*/
	SFTime startTime;	/*exposedField*/
	SFTime stopTime;	/*exposedField*/
	MFURL url;	/*exposedField*/
	SFBool repeatS;	/*field*/
	SFBool repeatT;	/*field*/
	SFTime duration_changed;	/*eventOut*/
	SFBool isActive;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
	SFTime resumeTime;	/*exposedField*/
	SFTime pauseTime;	/*exposedField*/
	SFTime elapsedTime;	/*eventOut*/
	SFBool isPaused;	/*eventOut*/
} X_MovieTexture;


typedef struct _tagX3DMultiTexture
{
	BASE_NODE
	SFFloat alpha;	/*exposedField*/
	SFColor color;	/*exposedField*/
	MFString function;	/*exposedField*/
	MFString mode;	/*exposedField*/
	MFString source;	/*exposedField*/
	GF_ChildNodeItem *texture;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_MultiTexture;


typedef struct _tagX3DMultiTextureCoordinate
{
	BASE_NODE
	GF_ChildNodeItem *texCoord;	/*MultiTextureCoordinate*/
	GF_Node *metadata;	/*exposedField*/
} X_MultiTextureCoordinate;


typedef struct _tagX3DMultiTextureTransform
{
	BASE_NODE
	GF_ChildNodeItem *textureTransform;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_MultiTextureTransform;


typedef struct _tagX3DNavigationInfo
{
	BASE_NODE
	SFBool set_bind;	/*eventIn*/
	void (*on_set_bind)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat avatarSize;	/*exposedField*/
	SFBool headlight;	/*exposedField*/
	SFFloat speed;	/*exposedField*/
	MFString type;	/*exposedField*/
	SFFloat visibilityLimit;	/*exposedField*/
	SFBool isBound;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
	MFString transitionType;	/*exposedField*/
	SFTime bindTime;	/*eventOut*/
} X_NavigationInfo;


typedef struct _tagX3DNormal
{
	BASE_NODE
	MFVec3f vector;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_Normal;


typedef struct _tagX3DNormalInterpolator
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat key;	/*exposedField*/
	MFVec3f keyValue;	/*exposedField*/
	MFVec3f value_changed;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_NormalInterpolator;


typedef struct _tagX3DNurbsCurve
{
	BASE_NODE
	MFVec3f controlPoint;	/*exposedField*/
	SFInt32 tessellation;	/*exposedField*/
	MFDouble weight;	/*exposedField*/
	SFBool closed;	/*field*/
	MFFloat knot;	/*field*/
	SFInt32 order;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_NurbsCurve;


typedef struct _tagX3DNurbsCurve2D
{
	BASE_NODE
	MFVec2f controlPoint;	/*exposedField*/
	SFInt32 tessellation;	/*exposedField*/
	MFFloat weight;	/*exposedField*/
	MFFloat knot;	/*field*/
	SFInt32 order;	/*field*/
	SFBool closed;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_NurbsCurve2D;


typedef struct _tagX3DNurbsOrientationInterpolator
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	GF_Node *controlPoints;	/*exposedField*/
	MFDouble knot;	/*exposedField*/
	SFInt32 order;	/*exposedField*/
	MFDouble weight;	/*exposedField*/
	SFRotation value_changed;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_NurbsOrientationInterpolator;


typedef struct _tagX3DNurbsPatchSurface
{
	BASE_NODE
	GF_Node *controlPoint;	/*exposedField*/
	GF_Node *texCoord;	/*exposedField*/
	SFInt32 uTessellation;	/*exposedField*/
	SFInt32 vTessellation;	/*exposedField*/
	MFDouble weight;	/*exposedField*/
	SFBool solid;	/*field*/
	SFBool uClosed;	/*field*/
	SFInt32 uDimension;	/*field*/
	MFDouble uKnot;	/*field*/
	SFInt32 uOrder;	/*field*/
	SFBool vClosed;	/*field*/
	SFInt32 vDimension;	/*field*/
	MFDouble vKnot;	/*field*/
	SFInt32 vOrder;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_NurbsPatchSurface;


typedef struct _tagX3DNurbsPositionInterpolator
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	GF_Node *controlPoints;	/*exposedField*/
	MFDouble knot;	/*exposedField*/
	SFInt32 order;	/*exposedField*/
	MFDouble weight;	/*exposedField*/
	SFVec3f value_changed;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_NurbsPositionInterpolator;


typedef struct _tagX3DNurbsSet
{
	BASE_NODE
	GF_ChildNodeItem *addGeometry;	/*eventIn*/
	void (*on_addGeometry)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	GF_ChildNodeItem *removeGeometry;	/*eventIn*/
	void (*on_removeGeometry)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	GF_ChildNodeItem *geometry;	/*exposedField*/
	SFFloat tessellationScale;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_NurbsSet;


typedef struct _tagX3DNurbsSurfaceInterpolator
{
	BASE_NODE
	SFVec2f set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	GF_Node *controlPoints;	/*exposedField*/
	MFDouble weight;	/*exposedField*/
	SFVec3f position_changed;	/*eventOut*/
	SFVec3f normal_changed;	/*eventOut*/
	SFInt32 uDimension;	/*field*/
	MFDouble uKnot;	/*field*/
	SFInt32 uOrder;	/*field*/
	SFInt32 vDimension;	/*field*/
	MFDouble vKnot;	/*field*/
	SFInt32 vOrder;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_NurbsSurfaceInterpolator;


typedef struct _tagX3DNurbsSweptSurface
{
	BASE_NODE
	GF_Node *crossSectionCurve;	/*exposedField*/
	GF_Node *trajectoryCurve;	/*exposedField*/
	SFBool ccw;	/*field*/
	SFBool solid;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_NurbsSweptSurface;


typedef struct _tagX3DNurbsSwungSurface
{
	BASE_NODE
	GF_Node *profileCurve;	/*exposedField*/
	GF_Node *trajectoryCurve;	/*exposedField*/
	SFBool ccw;	/*field*/
	SFBool solid;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_NurbsSwungSurface;


typedef struct _tagX3DNurbsTextureCoordinate
{
	BASE_NODE
	MFVec2f controlPoint;	/*exposedField*/
	MFFloat weight;	/*exposedField*/
	SFInt32 uDimension;	/*field*/
	MFDouble uKnot;	/*field*/
	SFInt32 uOrder;	/*field*/
	SFInt32 vDimension;	/*field*/
	MFDouble vKnot;	/*field*/
	SFInt32 vOrder;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_NurbsTextureCoordinate;


typedef struct _tagX3DNurbsTrimmedSurface
{
	BASE_NODE
	GF_ChildNodeItem *addTrimmingContour;	/*eventIn*/
	void (*on_addTrimmingContour)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	GF_ChildNodeItem *removeTrimmingContour;	/*eventIn*/
	void (*on_removeTrimmingContour)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	GF_ChildNodeItem *trimmingContour;	/*exposedField*/
	GF_Node *controlPoint;	/*exposedField*/
	GF_Node *texCoord;	/*exposedField*/
	SFInt32 uTessellation;	/*exposedField*/
	SFInt32 vTessellation;	/*exposedField*/
	MFDouble weight;	/*exposedField*/
	SFBool solid;	/*field*/
	SFBool uClosed;	/*field*/
	SFInt32 uDimension;	/*field*/
	MFDouble uKnot;	/*field*/
	SFInt32 uOrder;	/*field*/
	SFBool vClosed;	/*field*/
	SFInt32 vDimension;	/*field*/
	MFDouble vKnot;	/*field*/
	SFInt32 vOrder;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_NurbsTrimmedSurface;


typedef struct _tagX3DOrientationInterpolator
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat key;	/*exposedField*/
	MFRotation keyValue;	/*exposedField*/
	SFRotation value_changed;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_OrientationInterpolator;


typedef struct _tagX3DPixelTexture
{
	BASE_NODE
	SFImage image;	/*exposedField*/
	SFBool repeatS;	/*field*/
	SFBool repeatT;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_PixelTexture;


typedef struct _tagX3DPlaneSensor
{
	BASE_NODE
	SFBool autoOffset;	/*exposedField*/
	SFBool enabled;	/*exposedField*/
	SFVec2f maxPosition;	/*exposedField*/
	SFVec2f minPosition;	/*exposedField*/
	SFVec3f offset;	/*exposedField*/
	SFBool isActive;	/*eventOut*/
	SFVec3f trackPoint_changed;	/*eventOut*/
	SFVec3f translation_changed;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
	SFString description;	/*exposedField*/
	SFBool isOver;	/*eventOut*/
} X_PlaneSensor;


typedef struct _tagX3DPointLight
{
	BASE_NODE
	SFFloat ambientIntensity;	/*exposedField*/
	SFVec3f attenuation;	/*exposedField*/
	SFColor color;	/*exposedField*/
	SFFloat intensity;	/*exposedField*/
	SFVec3f location;	/*exposedField*/
	SFBool on;	/*exposedField*/
	SFFloat radius;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_PointLight;


typedef struct _tagX3DPointSet
{
	BASE_NODE
	GF_Node *color;	/*exposedField*/
	GF_Node *coord;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_PointSet;


typedef struct _tagX3DPolyline2D
{
	BASE_NODE
	MFVec2f lineSegments;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_Polyline2D;


typedef struct _tagX3DPolypoint2D
{
	BASE_NODE
	MFVec2f point;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_Polypoint2D;


typedef struct _tagX3DPositionInterpolator
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat key;	/*exposedField*/
	MFVec3f keyValue;	/*exposedField*/
	SFVec3f value_changed;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_PositionInterpolator;


typedef struct _tagX3DPositionInterpolator2D
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat key;	/*exposedField*/
	MFVec2f keyValue;	/*exposedField*/
	SFVec2f value_changed;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_PositionInterpolator2D;


typedef struct _tagX3DProximitySensor
{
	BASE_NODE
	SFVec3f center;	/*exposedField*/
	SFVec3f size;	/*exposedField*/
	SFBool enabled;	/*exposedField*/
	SFBool isActive;	/*eventOut*/
	SFVec3f position_changed;	/*eventOut*/
	SFRotation orientation_changed;	/*eventOut*/
	SFTime enterTime;	/*eventOut*/
	SFTime exitTime;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
	SFVec3f centerOfRotation_changed;	/*eventOut*/
} X_ProximitySensor;


typedef struct _tagX3DReceiverPdu
{
	BASE_NODE
	SFString address;	/*exposedField*/
	SFInt32 applicationID;	/*exposedField*/
	SFInt32 entityID;	/*exposedField*/
	SFString multicastRelayHost;	/*exposedField*/
	SFInt32 multicastRelayPort;	/*exposedField*/
	SFString networkMode;	/*exposedField*/
	SFInt32 port;	/*exposedField*/
	SFInt32 radioID;	/*exposedField*/
	SFFloat readInterval;	/*exposedField*/
	SFFloat receivedPower;	/*exposedField*/
	SFInt32 receiverState;	/*exposedField*/
	SFBool rtpHeaderExpected;	/*exposedField*/
	SFInt32 siteID;	/*exposedField*/
	SFInt32 transmitterApplicationID;	/*exposedField*/
	SFInt32 transmitterEntityID;	/*exposedField*/
	SFInt32 transmitterRadioID;	/*exposedField*/
	SFInt32 transmitterSiteID;	/*exposedField*/
	SFInt32 whichGeometry;	/*exposedField*/
	SFFloat writeInterval;	/*exposedField*/
	SFBool isActive;	/*eventOut*/
	SFBool isNetworkReader;	/*eventOut*/
	SFBool isNetworkWriter;	/*eventOut*/
	SFBool isRtpHeaderHeard;	/*eventOut*/
	SFBool isStandAlone;	/*eventOut*/
	SFTime timestamp;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_ReceiverPdu;


typedef struct _tagX3DRectangle2D
{
	BASE_NODE
	SFVec2f size;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_Rectangle2D;


typedef struct _tagX3DScalarInterpolator
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat key;	/*exposedField*/
	MFFloat keyValue;	/*exposedField*/
	SFFloat value_changed;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_ScalarInterpolator;


typedef struct _tagX3DScript
{
	BASE_NODE
	MFScript url;	/*exposedField*/
	SFBool directOutput;	/*field*/
	SFBool mustEvaluate;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_Script;


typedef struct _tagX3DShape
{
	BASE_NODE
	GF_Node *appearance;	/*exposedField*/
	GF_Node *geometry;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_Shape;


typedef struct _tagX3DSignalPdu
{
	BASE_NODE
	SFString address;	/*exposedField*/
	SFInt32 applicationID;	/*exposedField*/
	MFInt32 data;	/*exposedField*/
	SFInt32 dataLength;	/*exposedField*/
	SFInt32 encodingScheme;	/*exposedField*/
	SFInt32 entityID;	/*exposedField*/
	SFString multicastRelayHost;	/*exposedField*/
	SFInt32 multicastRelayPort;	/*exposedField*/
	SFString networkMode;	/*exposedField*/
	SFInt32 port;	/*exposedField*/
	SFInt32 radioID;	/*exposedField*/
	SFFloat readInterval;	/*exposedField*/
	SFBool rtpHeaderExpected;	/*exposedField*/
	SFInt32 sampleRate;	/*exposedField*/
	SFInt32 samples;	/*exposedField*/
	SFInt32 siteID;	/*exposedField*/
	SFInt32 tdlType;	/*exposedField*/
	SFInt32 whichGeometry;	/*exposedField*/
	SFFloat writeInterval;	/*exposedField*/
	SFBool isActive;	/*eventOut*/
	SFBool isNetworkReader;	/*eventOut*/
	SFBool isNetworkWriter;	/*eventOut*/
	SFBool isRtpHeaderHeard;	/*eventOut*/
	SFBool isStandAlone;	/*eventOut*/
	SFTime timestamp;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_SignalPdu;


typedef struct _tagX3DSound
{
	BASE_NODE
	SFVec3f direction;	/*exposedField*/
	SFFloat intensity;	/*exposedField*/
	SFVec3f location;	/*exposedField*/
	SFFloat maxBack;	/*exposedField*/
	SFFloat maxFront;	/*exposedField*/
	SFFloat minBack;	/*exposedField*/
	SFFloat minFront;	/*exposedField*/
	SFFloat priority;	/*exposedField*/
	GF_Node *source;	/*exposedField*/
	SFBool spatialize;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_Sound;


typedef struct _tagX3DSphere
{
	BASE_NODE
	SFFloat radius;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_Sphere;


typedef struct _tagX3DSphereSensor
{
	BASE_NODE
	SFBool autoOffset;	/*exposedField*/
	SFBool enabled;	/*exposedField*/
	SFRotation offset;	/*exposedField*/
	SFBool isActive;	/*eventOut*/
	SFRotation rotation_changed;	/*eventOut*/
	SFVec3f trackPoint_changed;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
	SFString description;	/*exposedField*/
	SFBool isOver;	/*eventOut*/
} X_SphereSensor;


typedef struct _tagX3DSpotLight
{
	BASE_NODE
	SFFloat ambientIntensity;	/*exposedField*/
	SFVec3f attenuation;	/*exposedField*/
	SFFloat beamWidth;	/*exposedField*/
	SFColor color;	/*exposedField*/
	SFFloat cutOffAngle;	/*exposedField*/
	SFVec3f direction;	/*exposedField*/
	SFFloat intensity;	/*exposedField*/
	SFVec3f location;	/*exposedField*/
	SFBool on;	/*exposedField*/
	SFFloat radius;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_SpotLight;


typedef struct _tagX3DStaticGroup
{
	BASE_NODE
	VRML_CHILDREN
	GF_Node *metadata;	/*exposedField*/
} X_StaticGroup;


typedef struct _tagX3DStringSensor
{
	BASE_NODE
	SFBool deletionAllowed;	/*exposedField*/
	SFBool enabled;	/*exposedField*/
	SFString enteredText;	/*eventOut*/
	SFString finalText;	/*eventOut*/
	SFBool isActive;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_StringSensor;


typedef struct _tagX3DSwitch
{
	BASE_NODE
	VRML_CHILDREN
	SFInt32 whichChoice;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_Switch;


typedef struct _tagX3DText
{
	BASE_NODE
	MFString string;	/*exposedField*/
	MFFloat length;	/*exposedField*/
	GF_Node *fontStyle;	/*exposedField*/
	SFFloat maxExtent;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_Text;


typedef struct _tagX3DTextureBackground
{
	BASE_NODE
	SFBool set_bind;	/*eventIn*/
	void (*on_set_bind)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat groundAngle;	/*exposedField*/
	MFColor groundColor;	/*exposedField*/
	GF_Node *backTexture;	/*exposedField*/
	GF_Node *bottomTexture;	/*exposedField*/
	GF_Node *frontTexture;	/*exposedField*/
	GF_Node *leftTexture;	/*exposedField*/
	GF_Node *rightTexture;	/*exposedField*/
	GF_Node *topTexture;	/*exposedField*/
	MFFloat skyAngle;	/*exposedField*/
	MFColor skyColor;	/*exposedField*/
	MFFloat transparency;	/*exposedField*/
	SFTime bindTime;	/*exposedField*/
	SFBool isBound;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_TextureBackground;


typedef struct _tagX3DTextureCoordinate
{
	BASE_NODE
	MFVec2f point;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_TextureCoordinate;


typedef struct _tagX3DTextureCoordinateGenerator
{
	BASE_NODE
	SFString mode;	/*exposedField*/
	MFFloat parameter;	/*TextureCoordinateGenerator*/
	GF_Node *metadata;	/*exposedField*/
} X_TextureCoordinateGenerator;


typedef struct _tagX3DTextureTransform
{
	BASE_NODE
	SFVec2f center;	/*exposedField*/
	SFFloat rotation;	/*exposedField*/
	SFVec2f scale;	/*exposedField*/
	SFVec2f translation;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_TextureTransform;


typedef struct _tagX3DTimeSensor
{
	BASE_NODE
	SFTime cycleInterval;	/*exposedField*/
	SFBool enabled;	/*exposedField*/
	SFBool loop;	/*exposedField*/
	SFTime startTime;	/*exposedField*/
	SFTime stopTime;	/*exposedField*/
	SFTime cycleTime;	/*eventOut*/
	SFFloat fraction_changed;	/*eventOut*/
	SFBool isActive;	/*eventOut*/
	SFTime time;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
	SFTime pauseTime;	/*exposedField*/
	SFTime resumeTime;	/*exposedField*/
	SFTime elapsedTime;	/*eventOut*/
	SFBool isPaused;	/*eventOut*/
} X_TimeSensor;


typedef struct _tagX3DTimeTrigger
{
	BASE_NODE
	SFBool set_boolean;	/*eventIn*/
	void (*on_set_boolean)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFTime triggerTime;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_TimeTrigger;


typedef struct _tagX3DTouchSensor
{
	BASE_NODE
	SFBool enabled;	/*exposedField*/
	SFVec3f hitNormal_changed;	/*eventOut*/
	SFVec3f hitPoint_changed;	/*eventOut*/
	SFVec2f hitTexCoord_changed;	/*eventOut*/
	SFBool isActive;	/*eventOut*/
	SFBool isOver;	/*eventOut*/
	SFTime touchTime;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
	SFString description;	/*exposedField*/
} X_TouchSensor;


typedef struct _tagX3DTransform
{
	BASE_NODE
	VRML_CHILDREN
	SFVec3f center;	/*exposedField*/
	SFRotation rotation;	/*exposedField*/
	SFVec3f scale;	/*exposedField*/
	SFRotation scaleOrientation;	/*exposedField*/
	SFVec3f translation;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_Transform;


typedef struct _tagX3DTransmitterPdu
{
	BASE_NODE
	SFString address;	/*exposedField*/
	SFVec3f antennaLocation;	/*exposedField*/
	SFInt32 antennaPatternLength;	/*exposedField*/
	SFInt32 antennaPatternType;	/*exposedField*/
	SFInt32 applicationID;	/*exposedField*/
	SFInt32 cryptoKeyID;	/*exposedField*/
	SFInt32 cryptoSystem;	/*exposedField*/
	SFInt32 entityID;	/*exposedField*/
	SFInt32 frequency;	/*exposedField*/
	SFInt32 inputSource;	/*exposedField*/
	SFInt32 lengthOfModulationParameters;	/*exposedField*/
	SFInt32 modulationTypeDetail;	/*exposedField*/
	SFInt32 modulationTypeMajor;	/*exposedField*/
	SFInt32 modulationTypeSpreadSpectrum;	/*exposedField*/
	SFInt32 modulationTypeSystem;	/*exposedField*/
	SFString multicastRelayHost;	/*exposedField*/
	SFInt32 multicastRelayPort;	/*exposedField*/
	SFString networkMode;	/*exposedField*/
	SFInt32 port;	/*exposedField*/
	SFFloat power;	/*exposedField*/
	SFInt32 radioEntityTypeCategory;	/*exposedField*/
	SFInt32 radioEntityTypeCountry;	/*exposedField*/
	SFInt32 radioEntityTypeDomain;	/*exposedField*/
	SFInt32 radioEntityTypeKind;	/*exposedField*/
	SFInt32 radioEntityTypeNomenclature;	/*exposedField*/
	SFInt32 radioEntityTypeNomenclatureVersion;	/*exposedField*/
	SFInt32 radioID;	/*exposedField*/
	SFFloat readInterval;	/*exposedField*/
	SFVec3f relativeAntennaLocation;	/*exposedField*/
	SFBool rtpHeaderExpected;	/*exposedField*/
	SFInt32 siteID;	/*exposedField*/
	SFFloat transmitFrequencyBandwidth;	/*exposedField*/
	SFInt32 transmitState;	/*exposedField*/
	SFInt32 whichGeometry;	/*exposedField*/
	SFFloat writeInterval;	/*exposedField*/
	SFBool isActive;	/*eventOut*/
	SFBool isNetworkReader;	/*eventOut*/
	SFBool isNetworkWriter;	/*eventOut*/
	SFBool isRtpHeaderHeard;	/*eventOut*/
	SFBool isStandAlone;	/*eventOut*/
	SFTime timestamp;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_TransmitterPdu;


typedef struct _tagX3DTriangleFanSet
{
	BASE_NODE
	GF_Node *color;	/*exposedField*/
	GF_Node *coord;	/*exposedField*/
	MFInt32 fanCount;	/*exposedField*/
	GF_Node *normal;	/*exposedField*/
	GF_Node *texCoord;	/*exposedField*/
	SFBool ccw;	/*field*/
	SFBool colorPerVertex;	/*field*/
	SFBool normalPerVertex;	/*field*/
	SFBool solid;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_TriangleFanSet;


typedef struct _tagX3DTriangleSet
{
	BASE_NODE
	GF_Node *color;	/*exposedField*/
	GF_Node *coord;	/*exposedField*/
	GF_Node *normal;	/*exposedField*/
	GF_Node *texCoord;	/*exposedField*/
	SFBool ccw;	/*field*/
	SFBool colorPerVertex;	/*field*/
	SFBool normalPerVertex;	/*field*/
	SFBool solid;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_TriangleSet;


typedef struct _tagX3DTriangleSet2D
{
	BASE_NODE
	MFVec2f vertices;	/*exposedField*/
	GF_Node *metadata;	/*exposedField*/
} X_TriangleSet2D;


typedef struct _tagX3DTriangleStripSet
{
	BASE_NODE
	GF_Node *color;	/*exposedField*/
	GF_Node *coord;	/*exposedField*/
	GF_Node *normal;	/*exposedField*/
	MFInt32 stripCount;	/*exposedField*/
	GF_Node *texCoord;	/*exposedField*/
	SFBool ccw;	/*field*/
	SFBool colorPerVertex;	/*field*/
	SFBool normalPerVertex;	/*field*/
	SFBool solid;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_TriangleStripSet;


typedef struct _tagX3DViewpoint
{
	BASE_NODE
	SFBool set_bind;	/*eventIn*/
	void (*on_set_bind)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFFloat fieldOfView;	/*exposedField*/
	SFBool jump;	/*exposedField*/
	SFRotation orientation;	/*exposedField*/
	SFVec3f position;	/*exposedField*/
	SFString description;	/*field*/
	SFTime bindTime;	/*eventOut*/
	SFBool isBound;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
	SFVec3f centerOfRotation;	/*exposedField*/
} X_Viewpoint;


typedef struct _tagX3DVisibilitySensor
{
	BASE_NODE
	SFVec3f center;	/*exposedField*/
	SFBool enabled;	/*exposedField*/
	SFVec3f size;	/*exposedField*/
	SFTime enterTime;	/*eventOut*/
	SFTime exitTime;	/*eventOut*/
	SFBool isActive;	/*eventOut*/
	GF_Node *metadata;	/*exposedField*/
} X_VisibilitySensor;


typedef struct _tagX3DWorldInfo
{
	BASE_NODE
	MFString info;	/*field*/
	SFString title;	/*field*/
	GF_Node *metadata;	/*exposedField*/
} X_WorldInfo;


#endif /*GPAC_DISABLE_X3D*/

#ifdef __cplusplus
}
#endif



#endif		/*_GF_X3D_NODES_H*/

