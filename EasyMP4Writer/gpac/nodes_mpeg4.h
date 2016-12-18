/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005
 *					All rights reserved
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


/*
	DO NOT MOFIFY - File generated on GMT Mon Jan 18 12:27:12 2010

	BY MPEG4Gen for GPAC Version 0.4.6-DEV
*/

#ifndef _nodes_mpeg4_H
#define _nodes_mpeg4_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/scenegraph_vrml.h>

#ifndef GPAC_DISABLE_VRML



enum {
	TAG_MPEG4_Anchor = GF_NODE_RANGE_FIRST_MPEG4,
	TAG_MPEG4_AnimationStream,
	TAG_MPEG4_Appearance,
	TAG_MPEG4_AudioBuffer,
	TAG_MPEG4_AudioClip,
	TAG_MPEG4_AudioDelay,
	TAG_MPEG4_AudioFX,
	TAG_MPEG4_AudioMix,
	TAG_MPEG4_AudioSource,
	TAG_MPEG4_AudioSwitch,
	TAG_MPEG4_Background,
	TAG_MPEG4_Background2D,
	TAG_MPEG4_Billboard,
	TAG_MPEG4_Bitmap,
	TAG_MPEG4_Box,
	TAG_MPEG4_Circle,
	TAG_MPEG4_Collision,
	TAG_MPEG4_Color,
	TAG_MPEG4_ColorInterpolator,
	TAG_MPEG4_CompositeTexture2D,
	TAG_MPEG4_CompositeTexture3D,
	TAG_MPEG4_Conditional,
	TAG_MPEG4_Cone,
	TAG_MPEG4_Coordinate,
	TAG_MPEG4_Coordinate2D,
	TAG_MPEG4_CoordinateInterpolator,
	TAG_MPEG4_CoordinateInterpolator2D,
	TAG_MPEG4_Curve2D,
	TAG_MPEG4_Cylinder,
	TAG_MPEG4_CylinderSensor,
	TAG_MPEG4_DirectionalLight,
	TAG_MPEG4_DiscSensor,
	TAG_MPEG4_ElevationGrid,
	TAG_MPEG4_Expression,
	TAG_MPEG4_Extrusion,
	TAG_MPEG4_Face,
	TAG_MPEG4_FaceDefMesh,
	TAG_MPEG4_FaceDefTables,
	TAG_MPEG4_FaceDefTransform,
	TAG_MPEG4_FAP,
	TAG_MPEG4_FDP,
	TAG_MPEG4_FIT,
	TAG_MPEG4_Fog,
	TAG_MPEG4_FontStyle,
	TAG_MPEG4_Form,
	TAG_MPEG4_Group,
	TAG_MPEG4_ImageTexture,
	TAG_MPEG4_IndexedFaceSet,
	TAG_MPEG4_IndexedFaceSet2D,
	TAG_MPEG4_IndexedLineSet,
	TAG_MPEG4_IndexedLineSet2D,
	TAG_MPEG4_Inline,
	TAG_MPEG4_LOD,
	TAG_MPEG4_Layer2D,
	TAG_MPEG4_Layer3D,
	TAG_MPEG4_Layout,
	TAG_MPEG4_LineProperties,
	TAG_MPEG4_ListeningPoint,
	TAG_MPEG4_Material,
	TAG_MPEG4_Material2D,
	TAG_MPEG4_MovieTexture,
	TAG_MPEG4_NavigationInfo,
	TAG_MPEG4_Normal,
	TAG_MPEG4_NormalInterpolator,
	TAG_MPEG4_OrderedGroup,
	TAG_MPEG4_OrientationInterpolator,
	TAG_MPEG4_PixelTexture,
	TAG_MPEG4_PlaneSensor,
	TAG_MPEG4_PlaneSensor2D,
	TAG_MPEG4_PointLight,
	TAG_MPEG4_PointSet,
	TAG_MPEG4_PointSet2D,
	TAG_MPEG4_PositionInterpolator,
	TAG_MPEG4_PositionInterpolator2D,
	TAG_MPEG4_ProximitySensor2D,
	TAG_MPEG4_ProximitySensor,
	TAG_MPEG4_QuantizationParameter,
	TAG_MPEG4_Rectangle,
	TAG_MPEG4_ScalarInterpolator,
	TAG_MPEG4_Script,
	TAG_MPEG4_Shape,
	TAG_MPEG4_Sound,
	TAG_MPEG4_Sound2D,
	TAG_MPEG4_Sphere,
	TAG_MPEG4_SphereSensor,
	TAG_MPEG4_SpotLight,
	TAG_MPEG4_Switch,
	TAG_MPEG4_TermCap,
	TAG_MPEG4_Text,
	TAG_MPEG4_TextureCoordinate,
	TAG_MPEG4_TextureTransform,
	TAG_MPEG4_TimeSensor,
	TAG_MPEG4_TouchSensor,
	TAG_MPEG4_Transform,
	TAG_MPEG4_Transform2D,
	TAG_MPEG4_Valuator,
	TAG_MPEG4_Viewpoint,
	TAG_MPEG4_VisibilitySensor,
	TAG_MPEG4_Viseme,
	TAG_MPEG4_WorldInfo,
	TAG_MPEG4_AcousticMaterial,
	TAG_MPEG4_AcousticScene,
	TAG_MPEG4_ApplicationWindow,
	TAG_MPEG4_BAP,
	TAG_MPEG4_BDP,
	TAG_MPEG4_Body,
	TAG_MPEG4_BodyDefTable,
	TAG_MPEG4_BodySegmentConnectionHint,
	TAG_MPEG4_DirectiveSound,
	TAG_MPEG4_Hierarchical3DMesh,
	TAG_MPEG4_MaterialKey,
	TAG_MPEG4_PerceptualParameters,
	TAG_MPEG4_TemporalTransform,
	TAG_MPEG4_TemporalGroup,
	TAG_MPEG4_ServerCommand,
	TAG_MPEG4_InputSensor,
	TAG_MPEG4_MatteTexture,
	TAG_MPEG4_MediaBuffer,
	TAG_MPEG4_MediaControl,
	TAG_MPEG4_MediaSensor,
	TAG_MPEG4_BitWrapper,
	TAG_MPEG4_CoordinateInterpolator4D,
	TAG_MPEG4_DepthImage,
	TAG_MPEG4_FFD,
	TAG_MPEG4_Implicit,
	TAG_MPEG4_XXLFM_Appearance,
	TAG_MPEG4_XXLFM_BlendList,
	TAG_MPEG4_XXLFM_FrameList,
	TAG_MPEG4_XXLFM_LightMap,
	TAG_MPEG4_XXLFM_SurfaceMapList,
	TAG_MPEG4_XXLFM_ViewMapList,
	TAG_MPEG4_MeshGrid,
	TAG_MPEG4_NonLinearDeformer,
	TAG_MPEG4_NurbsCurve,
	TAG_MPEG4_NurbsCurve2D,
	TAG_MPEG4_NurbsSurface,
	TAG_MPEG4_OctreeImage,
	TAG_MPEG4_XXParticles,
	TAG_MPEG4_XXParticleInitBox,
	TAG_MPEG4_XXPlanarObstacle,
	TAG_MPEG4_XXPointAttractor,
	TAG_MPEG4_PointTexture,
	TAG_MPEG4_PositionAnimator,
	TAG_MPEG4_PositionAnimator2D,
	TAG_MPEG4_PositionInterpolator4D,
	TAG_MPEG4_ProceduralTexture,
	TAG_MPEG4_Quadric,
	TAG_MPEG4_SBBone,
	TAG_MPEG4_SBMuscle,
	TAG_MPEG4_SBSegment,
	TAG_MPEG4_SBSite,
	TAG_MPEG4_SBSkinnedModel,
	TAG_MPEG4_SBVCAnimation,
	TAG_MPEG4_ScalarAnimator,
	TAG_MPEG4_SimpleTexture,
	TAG_MPEG4_SolidRep,
	TAG_MPEG4_SubdivisionSurface,
	TAG_MPEG4_SubdivSurfaceSector,
	TAG_MPEG4_WaveletSubdivisionSurface,
	TAG_MPEG4_Clipper2D,
	TAG_MPEG4_ColorTransform,
	TAG_MPEG4_Ellipse,
	TAG_MPEG4_LinearGradient,
	TAG_MPEG4_PathLayout,
	TAG_MPEG4_RadialGradient,
	TAG_MPEG4_SynthesizedTexture,
	TAG_MPEG4_TransformMatrix2D,
	TAG_MPEG4_Viewport,
	TAG_MPEG4_XCurve2D,
	TAG_MPEG4_XFontStyle,
	TAG_MPEG4_XLineProperties,
	TAG_MPEG4_AdvancedAudioBuffer,
	TAG_MPEG4_AudioChannelConfig,
	TAG_MPEG4_DepthImageV2,
	TAG_MPEG4_MorphShape,
	TAG_MPEG4_MultiTexture,
	TAG_MPEG4_PointTextureV2,
	TAG_MPEG4_SBVCAnimationV2,
	TAG_MPEG4_SimpleTextureV2,
	TAG_MPEG4_SurroundingSound,
	TAG_MPEG4_Transform3DAudio,
	TAG_MPEG4_WideSound,
	TAG_MPEG4_ScoreShape,
	TAG_MPEG4_MusicScore,
	TAG_MPEG4_FootPrintSetNode,
	TAG_MPEG4_FootPrintNode,
	TAG_MPEG4_BuildingPartNode,
	TAG_MPEG4_RoofNode,
	TAG_MPEG4_FacadeNode,
	TAG_MPEG4_Shadow,
	TAG_MPEG4_CacheTexture,
	TAG_MPEG4_EnvironmentTest,
	TAG_MPEG4_KeyNavigator,
	TAG_MPEG4_SpacePartition,
	TAG_MPEG4_Storage,
	TAG_LastImplementedMPEG4
};

typedef struct _tagAnchor
{
	BASE_NODE
	VRML_CHILDREN
	SFString description;	/*exposedField*/
	MFString parameter;	/*exposedField*/
	MFURL url;	/*exposedField*/
	SFBool activate;	/*eventIn*/
	void (*on_activate)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
} M_Anchor;


typedef struct _tagAnimationStream
{
	BASE_NODE
	SFBool loop;	/*exposedField*/
	SFFloat speed;	/*exposedField*/
	SFTime startTime;	/*exposedField*/
	SFTime stopTime;	/*exposedField*/
	MFURL url;	/*exposedField*/
	SFTime duration_changed;	/*eventOut*/
	SFBool isActive;	/*eventOut*/
} M_AnimationStream;


typedef struct _tagAppearance
{
	BASE_NODE
	GF_Node *material;	/*exposedField*/
	GF_Node *texture;	/*exposedField*/
	GF_Node *textureTransform;	/*exposedField*/
} M_Appearance;


typedef struct _tagAudioBuffer
{
	BASE_NODE
	SFBool loop;	/*exposedField*/
	SFFloat pitch;	/*exposedField*/
	SFTime startTime;	/*exposedField*/
	SFTime stopTime;	/*exposedField*/
	GF_ChildNodeItem *children;	/*exposedField*/
	SFInt32 numChan;	/*exposedField*/
	MFInt32 phaseGroup;	/*exposedField*/
	SFFloat length;	/*exposedField*/
	SFTime duration_changed;	/*eventOut*/
	SFBool isActive;	/*eventOut*/
} M_AudioBuffer;


typedef struct _tagAudioClip
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
} M_AudioClip;


typedef struct _tagAudioDelay
{
	BASE_NODE
	VRML_CHILDREN
	SFTime delay;	/*exposedField*/
	SFInt32 numChan;	/*field*/
	MFInt32 phaseGroup;	/*field*/
} M_AudioDelay;


typedef struct _tagAudioFX
{
	BASE_NODE
	VRML_CHILDREN
	SFString orch;	/*exposedField*/
	SFString score;	/*exposedField*/
	MFFloat params;	/*exposedField*/
	SFInt32 numChan;	/*field*/
	MFInt32 phaseGroup;	/*field*/
} M_AudioFX;


typedef struct _tagAudioMix
{
	BASE_NODE
	VRML_CHILDREN
	SFInt32 numInputs;	/*exposedField*/
	MFFloat matrix;	/*exposedField*/
	SFInt32 numChan;	/*field*/
	MFInt32 phaseGroup;	/*field*/
} M_AudioMix;


typedef struct _tagAudioSource
{
	BASE_NODE
	VRML_CHILDREN
	MFURL url;	/*exposedField*/
	SFFloat pitch;	/*exposedField*/
	SFFloat speed;	/*exposedField*/
	SFTime startTime;	/*exposedField*/
	SFTime stopTime;	/*exposedField*/
	SFInt32 numChan;	/*field*/
	MFInt32 phaseGroup;	/*field*/
} M_AudioSource;


typedef struct _tagAudioSwitch
{
	BASE_NODE
	VRML_CHILDREN
	MFInt32 whichChoice;	/*exposedField*/
	SFInt32 numChan;	/*field*/
	MFInt32 phaseGroup;	/*field*/
} M_AudioSwitch;


typedef struct _tagBackground
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
} M_Background;


typedef struct _tagBackground2D
{
	BASE_NODE
	SFBool set_bind;	/*eventIn*/
	void (*on_set_bind)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFColor backColor;	/*exposedField*/
	MFURL url;	/*exposedField*/
	SFBool isBound;	/*eventOut*/
} M_Background2D;


typedef struct _tagBillboard
{
	BASE_NODE
	VRML_CHILDREN
	SFVec3f axisOfRotation;	/*exposedField*/
} M_Billboard;


typedef struct _tagBitmap
{
	BASE_NODE
	SFVec2f scale;	/*exposedField*/
} M_Bitmap;


typedef struct _tagBox
{
	BASE_NODE
	SFVec3f size;	/*field*/
} M_Box;


typedef struct _tagCircle
{
	BASE_NODE
	SFFloat radius;	/*exposedField*/
} M_Circle;


typedef struct _tagCollision
{
	BASE_NODE
	VRML_CHILDREN
	SFBool collide;	/*exposedField*/
	GF_Node *proxy;	/*field*/
	SFTime collideTime;	/*eventOut*/
} M_Collision;


typedef struct _tagColor
{
	BASE_NODE
	MFColor color;	/*exposedField*/
} M_Color;


typedef struct _tagColorInterpolator
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat key;	/*exposedField*/
	MFColor keyValue;	/*exposedField*/
	SFColor value_changed;	/*eventOut*/
} M_ColorInterpolator;


typedef struct _tagCompositeTexture2D
{
	BASE_NODE
	VRML_CHILDREN
	SFInt32 pixelWidth;	/*exposedField*/
	SFInt32 pixelHeight;	/*exposedField*/
	GF_Node *background;	/*exposedField*/
	GF_Node *viewport;	/*exposedField*/
	SFInt32 repeatSandT;	/*field*/
} M_CompositeTexture2D;


typedef struct _tagCompositeTexture3D
{
	BASE_NODE
	VRML_CHILDREN
	SFInt32 pixelWidth;	/*exposedField*/
	SFInt32 pixelHeight;	/*exposedField*/
	GF_Node *background;	/*exposedField*/
	GF_Node *fog;	/*exposedField*/
	GF_Node *navigationInfo;	/*exposedField*/
	GF_Node *viewpoint;	/*exposedField*/
	SFBool repeatS;	/*field*/
	SFBool repeatT;	/*field*/
} M_CompositeTexture3D;


typedef struct _tagConditional
{
	BASE_NODE
	SFBool activate;	/*eventIn*/
	void (*on_activate)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFBool reverseActivate;	/*eventIn*/
	void (*on_reverseActivate)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFCommandBuffer buffer;	/*exposedField*/
	SFBool isActive;	/*eventOut*/
} M_Conditional;


typedef struct _tagCone
{
	BASE_NODE
	SFFloat bottomRadius;	/*field*/
	SFFloat height;	/*field*/
	SFBool side;	/*field*/
	SFBool bottom;	/*field*/
} M_Cone;


typedef struct _tagCoordinate
{
	BASE_NODE
	MFVec3f point;	/*exposedField*/
} M_Coordinate;


typedef struct _tagCoordinate2D
{
	BASE_NODE
	MFVec2f point;	/*exposedField*/
} M_Coordinate2D;


typedef struct _tagCoordinateInterpolator
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat key;	/*exposedField*/
	MFVec3f keyValue;	/*exposedField*/
	MFVec3f value_changed;	/*eventOut*/
} M_CoordinateInterpolator;


typedef struct _tagCoordinateInterpolator2D
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat key;	/*exposedField*/
	MFVec2f keyValue;	/*exposedField*/
	MFVec2f value_changed;	/*eventOut*/
} M_CoordinateInterpolator2D;


typedef struct _tagCurve2D
{
	BASE_NODE
	GF_Node *point;	/*exposedField*/
	SFFloat fineness;	/*exposedField*/
	MFInt32 type;	/*exposedField*/
} M_Curve2D;


typedef struct _tagCylinder
{
	BASE_NODE
	SFBool bottom;	/*field*/
	SFFloat height;	/*field*/
	SFFloat radius;	/*field*/
	SFBool side;	/*field*/
	SFBool top;	/*field*/
} M_Cylinder;


typedef struct _tagCylinderSensor
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
} M_CylinderSensor;


typedef struct _tagDirectionalLight
{
	BASE_NODE
	SFFloat ambientIntensity;	/*exposedField*/
	SFColor color;	/*exposedField*/
	SFVec3f direction;	/*exposedField*/
	SFFloat intensity;	/*exposedField*/
	SFBool on;	/*exposedField*/
} M_DirectionalLight;


typedef struct _tagDiscSensor
{
	BASE_NODE
	SFBool autoOffset;	/*exposedField*/
	SFBool enabled;	/*exposedField*/
	SFFloat maxAngle;	/*exposedField*/
	SFFloat minAngle;	/*exposedField*/
	SFFloat offset;	/*exposedField*/
	SFBool isActive;	/*eventOut*/
	SFFloat rotation_changed;	/*eventOut*/
	SFVec2f trackPoint_changed;	/*eventOut*/
} M_DiscSensor;


typedef struct _tagElevationGrid
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
} M_ElevationGrid;


typedef struct _tagExpression
{
	BASE_NODE
	SFInt32 expression_select1;	/*exposedField*/
	SFInt32 expression_intensity1;	/*exposedField*/
	SFInt32 expression_select2;	/*exposedField*/
	SFInt32 expression_intensity2;	/*exposedField*/
	SFBool init_face;	/*exposedField*/
	SFBool expression_def;	/*exposedField*/
} M_Expression;


typedef struct _tagExtrusion
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
} M_Extrusion;


typedef struct _tagFace
{
	BASE_NODE
	GF_Node *fap;	/*exposedField*/
	GF_Node *fdp;	/*exposedField*/
	GF_Node *fit;	/*exposedField*/
	GF_Node *ttsSource;	/*exposedField*/
	GF_ChildNodeItem *renderedFace;	/*exposedField*/
} M_Face;


typedef struct _tagFaceDefMesh
{
	BASE_NODE
	GF_Node *faceSceneGraphNode;	/*field*/
	MFInt32 intervalBorders;	/*field*/
	MFInt32 coordIndex;	/*field*/
	MFVec3f displacements;	/*field*/
} M_FaceDefMesh;


typedef struct _tagFaceDefTables
{
	BASE_NODE
	SFInt32 fapID;	/*field*/
	SFInt32 highLevelSelect;	/*field*/
	GF_ChildNodeItem *faceDefMesh;	/*exposedField*/
	GF_ChildNodeItem *faceDefTransform;	/*exposedField*/
} M_FaceDefTables;


typedef struct _tagFaceDefTransform
{
	BASE_NODE
	GF_Node *faceSceneGraphNode;	/*field*/
	SFInt32 fieldId;	/*field*/
	SFRotation rotationDef;	/*field*/
	SFVec3f scaleDef;	/*field*/
	SFVec3f translationDef;	/*field*/
} M_FaceDefTransform;


typedef struct _tagFAP
{
	BASE_NODE
	GF_Node *viseme;	/*exposedField*/
	GF_Node *expression;	/*exposedField*/
	SFInt32 open_jaw;	/*exposedField*/
	SFInt32 lower_t_midlip;	/*exposedField*/
	SFInt32 raise_b_midlip;	/*exposedField*/
	SFInt32 stretch_l_corner;	/*exposedField*/
	SFInt32 stretch_r_corner;	/*exposedField*/
	SFInt32 lower_t_lip_lm;	/*exposedField*/
	SFInt32 lower_t_lip_rm;	/*exposedField*/
	SFInt32 lower_b_lip_lm;	/*exposedField*/
	SFInt32 lower_b_lip_rm;	/*exposedField*/
	SFInt32 raise_l_cornerlip;	/*exposedField*/
	SFInt32 raise_r_cornerlip;	/*exposedField*/
	SFInt32 thrust_jaw;	/*exposedField*/
	SFInt32 shift_jaw;	/*exposedField*/
	SFInt32 push_b_lip;	/*exposedField*/
	SFInt32 push_t_lip;	/*exposedField*/
	SFInt32 depress_chin;	/*exposedField*/
	SFInt32 close_t_l_eyelid;	/*exposedField*/
	SFInt32 close_t_r_eyelid;	/*exposedField*/
	SFInt32 close_b_l_eyelid;	/*exposedField*/
	SFInt32 close_b_r_eyelid;	/*exposedField*/
	SFInt32 yaw_l_eyeball;	/*exposedField*/
	SFInt32 yaw_r_eyeball;	/*exposedField*/
	SFInt32 pitch_l_eyeball;	/*exposedField*/
	SFInt32 pitch_r_eyeball;	/*exposedField*/
	SFInt32 thrust_l_eyeball;	/*exposedField*/
	SFInt32 thrust_r_eyeball;	/*exposedField*/
	SFInt32 dilate_l_pupil;	/*exposedField*/
	SFInt32 dilate_r_pupil;	/*exposedField*/
	SFInt32 raise_l_i_eyebrow;	/*exposedField*/
	SFInt32 raise_r_i_eyebrow;	/*exposedField*/
	SFInt32 raise_l_m_eyebrow;	/*exposedField*/
	SFInt32 raise_r_m_eyebrow;	/*exposedField*/
	SFInt32 raise_l_o_eyebrow;	/*exposedField*/
	SFInt32 raise_r_o_eyebrow;	/*exposedField*/
	SFInt32 squeeze_l_eyebrow;	/*exposedField*/
	SFInt32 squeeze_r_eyebrow;	/*exposedField*/
	SFInt32 puff_l_cheek;	/*exposedField*/
	SFInt32 puff_r_cheek;	/*exposedField*/
	SFInt32 lift_l_cheek;	/*exposedField*/
	SFInt32 lift_r_cheek;	/*exposedField*/
	SFInt32 shift_tongue_tip;	/*exposedField*/
	SFInt32 raise_tongue_tip;	/*exposedField*/
	SFInt32 thrust_tongue_tip;	/*exposedField*/
	SFInt32 raise_tongue;	/*exposedField*/
	SFInt32 tongue_roll;	/*exposedField*/
	SFInt32 head_pitch;	/*exposedField*/
	SFInt32 head_yaw;	/*exposedField*/
	SFInt32 head_roll;	/*exposedField*/
	SFInt32 lower_t_midlip_o;	/*exposedField*/
	SFInt32 raise_b_midlip_o;	/*exposedField*/
	SFInt32 stretch_l_cornerlip;	/*exposedField*/
	SFInt32 stretch_r_cornerlip;	/*exposedField*/
	SFInt32 lower_t_lip_lm_o;	/*exposedField*/
	SFInt32 lower_t_lip_rm_o;	/*exposedField*/
	SFInt32 raise_b_lip_lm_o;	/*exposedField*/
	SFInt32 raise_b_lip_rm_o;	/*exposedField*/
	SFInt32 raise_l_cornerlip_o;	/*exposedField*/
	SFInt32 raise_r_cornerlip_o;	/*exposedField*/
	SFInt32 stretch_l_nose;	/*exposedField*/
	SFInt32 stretch_r_nose;	/*exposedField*/
	SFInt32 raise_nose;	/*exposedField*/
	SFInt32 bend_nose;	/*exposedField*/
	SFInt32 raise_l_ear;	/*exposedField*/
	SFInt32 raise_r_ear;	/*exposedField*/
	SFInt32 pull_l_ear;	/*exposedField*/
	SFInt32 pull_r_ear;	/*exposedField*/
} M_FAP;


typedef struct _tagFDP
{
	BASE_NODE
	GF_Node *featurePointsCoord;	/*exposedField*/
	GF_Node *textureCoord;	/*exposedField*/
	GF_ChildNodeItem *faceDefTables;	/*exposedField*/
	GF_ChildNodeItem *faceSceneGraph;	/*exposedField*/
	SFBool useOrthoTexture;	/*field*/
} M_FDP;


typedef struct _tagFIT
{
	BASE_NODE
	MFInt32 FAPs;	/*exposedField*/
	MFInt32 Graph;	/*exposedField*/
	MFInt32 numeratorExp;	/*exposedField*/
	MFInt32 denominatorExp;	/*exposedField*/
	MFInt32 numeratorImpulse;	/*exposedField*/
	MFInt32 numeratorTerms;	/*exposedField*/
	MFInt32 denominatorTerms;	/*exposedField*/
	MFFloat numeratorCoefs;	/*exposedField*/
	MFFloat denominatorCoefs;	/*exposedField*/
} M_FIT;


typedef struct _tagFog
{
	BASE_NODE
	SFColor color;	/*exposedField*/
	SFString fogType;	/*exposedField*/
	SFFloat visibilityRange;	/*exposedField*/
	SFBool set_bind;	/*eventIn*/
	void (*on_set_bind)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFBool isBound;	/*eventOut*/
} M_Fog;


typedef struct _tagFontStyle
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
} M_FontStyle;


typedef struct _tagForm
{
	BASE_NODE
	VRML_CHILDREN
	SFVec2f size;	/*exposedField*/
	MFInt32 groups;	/*exposedField*/
	MFString constraints;	/*exposedField*/
	MFInt32 groupsIndex;	/*exposedField*/
} M_Form;


typedef struct _tagGroup
{
	BASE_NODE
	VRML_CHILDREN
} M_Group;


typedef struct _tagImageTexture
{
	BASE_NODE
	MFURL url;	/*exposedField*/
	SFBool repeatS;	/*field*/
	SFBool repeatT;	/*field*/
} M_ImageTexture;


typedef struct _tagIndexedFaceSet
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
} M_IndexedFaceSet;


typedef struct _tagIndexedFaceSet2D
{
	BASE_NODE
	MFInt32 set_colorIndex;	/*eventIn*/
	void (*on_set_colorIndex)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFInt32 set_coordIndex;	/*eventIn*/
	void (*on_set_coordIndex)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFInt32 set_texCoordIndex;	/*eventIn*/
	void (*on_set_texCoordIndex)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	GF_Node *color;	/*exposedField*/
	GF_Node *coord;	/*exposedField*/
	GF_Node *texCoord;	/*exposedField*/
	MFInt32 colorIndex;	/*field*/
	SFBool colorPerVertex;	/*field*/
	SFBool convex;	/*field*/
	MFInt32 coordIndex;	/*field*/
	MFInt32 texCoordIndex;	/*field*/
} M_IndexedFaceSet2D;


typedef struct _tagIndexedLineSet
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
} M_IndexedLineSet;


typedef struct _tagIndexedLineSet2D
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
} M_IndexedLineSet2D;


typedef struct _tagInline
{
	BASE_NODE
	MFURL url;	/*exposedField*/
} M_Inline;


typedef struct _tagLOD
{
	BASE_NODE
	GF_ChildNodeItem *level;	/*exposedField*/
	SFVec3f center;	/*field*/
	MFFloat range;	/*field*/
} M_LOD;


typedef struct _tagLayer2D
{
	BASE_NODE
	VRML_CHILDREN
	SFVec2f size;	/*exposedField*/
	GF_Node *background;	/*exposedField*/
	GF_Node *viewport;	/*exposedField*/
} M_Layer2D;


typedef struct _tagLayer3D
{
	BASE_NODE
	VRML_CHILDREN
	SFVec2f size;	/*exposedField*/
	GF_Node *background;	/*exposedField*/
	GF_Node *fog;	/*exposedField*/
	GF_Node *navigationInfo;	/*exposedField*/
	GF_Node *viewpoint;	/*exposedField*/
} M_Layer3D;


typedef struct _tagLayout
{
	BASE_NODE
	VRML_CHILDREN
	SFBool wrap;	/*exposedField*/
	SFVec2f size;	/*exposedField*/
	SFBool horizontal;	/*exposedField*/
	MFString justify;	/*exposedField*/
	SFBool leftToRight;	/*exposedField*/
	SFBool topToBottom;	/*exposedField*/
	SFFloat spacing;	/*exposedField*/
	SFBool smoothScroll;	/*exposedField*/
	SFBool loop;	/*exposedField*/
	SFBool scrollVertical;	/*exposedField*/
	SFFloat scrollRate;	/*exposedField*/
	SFInt32 scrollMode;	/*exposedField*/
} M_Layout;


typedef struct _tagLineProperties
{
	BASE_NODE
	SFColor lineColor;	/*exposedField*/
	SFInt32 lineStyle;	/*exposedField*/
	SFFloat width;	/*exposedField*/
} M_LineProperties;


typedef struct _tagListeningPoint
{
	BASE_NODE
	SFBool set_bind;	/*eventIn*/
	void (*on_set_bind)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFBool jump;	/*exposedField*/
	SFRotation orientation;	/*exposedField*/
	SFVec3f position;	/*exposedField*/
	SFString description;	/*field*/
	SFTime bindTime;	/*eventOut*/
	SFBool isBound;	/*eventOut*/
} M_ListeningPoint;


typedef struct _tagMaterial
{
	BASE_NODE
	SFFloat ambientIntensity;	/*exposedField*/
	SFColor diffuseColor;	/*exposedField*/
	SFColor emissiveColor;	/*exposedField*/
	SFFloat shininess;	/*exposedField*/
	SFColor specularColor;	/*exposedField*/
	SFFloat transparency;	/*exposedField*/
} M_Material;


typedef struct _tagMaterial2D
{
	BASE_NODE
	SFColor emissiveColor;	/*exposedField*/
	SFBool filled;	/*exposedField*/
	GF_Node *lineProps;	/*exposedField*/
	SFFloat transparency;	/*exposedField*/
} M_Material2D;


typedef struct _tagMovieTexture
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
} M_MovieTexture;


typedef struct _tagNavigationInfo
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
} M_NavigationInfo;


typedef struct _tagNormal
{
	BASE_NODE
	MFVec3f vector;	/*exposedField*/
} M_Normal;


typedef struct _tagNormalInterpolator
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat key;	/*exposedField*/
	MFVec3f keyValue;	/*exposedField*/
	MFVec3f value_changed;	/*eventOut*/
} M_NormalInterpolator;


typedef struct _tagOrderedGroup
{
	BASE_NODE
	VRML_CHILDREN
	MFFloat order;	/*exposedField*/
} M_OrderedGroup;


typedef struct _tagOrientationInterpolator
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat key;	/*exposedField*/
	MFRotation keyValue;	/*exposedField*/
	SFRotation value_changed;	/*eventOut*/
} M_OrientationInterpolator;


typedef struct _tagPixelTexture
{
	BASE_NODE
	SFImage image;	/*exposedField*/
	SFBool repeatS;	/*field*/
	SFBool repeatT;	/*field*/
} M_PixelTexture;


typedef struct _tagPlaneSensor
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
} M_PlaneSensor;


typedef struct _tagPlaneSensor2D
{
	BASE_NODE
	SFBool autoOffset;	/*exposedField*/
	SFBool enabled;	/*exposedField*/
	SFVec2f maxPosition;	/*exposedField*/
	SFVec2f minPosition;	/*exposedField*/
	SFVec2f offset;	/*exposedField*/
	SFBool isActive;	/*eventOut*/
	SFVec2f trackPoint_changed;	/*eventOut*/
	SFVec2f translation_changed;	/*eventOut*/
} M_PlaneSensor2D;


typedef struct _tagPointLight
{
	BASE_NODE
	SFFloat ambientIntensity;	/*exposedField*/
	SFVec3f attenuation;	/*exposedField*/
	SFColor color;	/*exposedField*/
	SFFloat intensity;	/*exposedField*/
	SFVec3f location;	/*exposedField*/
	SFBool on;	/*exposedField*/
	SFFloat radius;	/*exposedField*/
} M_PointLight;


typedef struct _tagPointSet
{
	BASE_NODE
	GF_Node *color;	/*exposedField*/
	GF_Node *coord;	/*exposedField*/
} M_PointSet;


typedef struct _tagPointSet2D
{
	BASE_NODE
	GF_Node *color;	/*exposedField*/
	GF_Node *coord;	/*exposedField*/
} M_PointSet2D;


typedef struct _tagPositionInterpolator
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat key;	/*exposedField*/
	MFVec3f keyValue;	/*exposedField*/
	SFVec3f value_changed;	/*eventOut*/
} M_PositionInterpolator;


typedef struct _tagPositionInterpolator2D
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat key;	/*exposedField*/
	MFVec2f keyValue;	/*exposedField*/
	SFVec2f value_changed;	/*eventOut*/
} M_PositionInterpolator2D;


typedef struct _tagProximitySensor2D
{
	BASE_NODE
	SFVec2f center;	/*exposedField*/
	SFVec2f size;	/*exposedField*/
	SFBool enabled;	/*exposedField*/
	SFBool isActive;	/*eventOut*/
	SFVec2f position_changed;	/*eventOut*/
	SFFloat orientation_changed;	/*eventOut*/
	SFTime enterTime;	/*eventOut*/
	SFTime exitTime;	/*eventOut*/
} M_ProximitySensor2D;


typedef struct _tagProximitySensor
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
} M_ProximitySensor;


typedef struct _tagQuantizationParameter
{
	BASE_NODE
	SFBool isLocal;	/*field*/
	SFBool position3DQuant;	/*field*/
	SFVec3f position3DMin;	/*field*/
	SFVec3f position3DMax;	/*field*/
	SFInt32 position3DNbBits;	/*field*/
	SFBool position2DQuant;	/*field*/
	SFVec2f position2DMin;	/*field*/
	SFVec2f position2DMax;	/*field*/
	SFInt32 position2DNbBits;	/*field*/
	SFBool drawOrderQuant;	/*field*/
	SFFloat drawOrderMin;	/*field*/
	SFFloat drawOrderMax;	/*field*/
	SFInt32 drawOrderNbBits;	/*field*/
	SFBool colorQuant;	/*field*/
	SFFloat colorMin;	/*field*/
	SFFloat colorMax;	/*field*/
	SFInt32 colorNbBits;	/*field*/
	SFBool textureCoordinateQuant;	/*field*/
	SFFloat textureCoordinateMin;	/*field*/
	SFFloat textureCoordinateMax;	/*field*/
	SFInt32 textureCoordinateNbBits;	/*field*/
	SFBool angleQuant;	/*field*/
	SFFloat angleMin;	/*field*/
	SFFloat angleMax;	/*field*/
	SFInt32 angleNbBits;	/*field*/
	SFBool scaleQuant;	/*field*/
	SFFloat scaleMin;	/*field*/
	SFFloat scaleMax;	/*field*/
	SFInt32 scaleNbBits;	/*field*/
	SFBool keyQuant;	/*field*/
	SFFloat keyMin;	/*field*/
	SFFloat keyMax;	/*field*/
	SFInt32 keyNbBits;	/*field*/
	SFBool normalQuant;	/*field*/
	SFInt32 normalNbBits;	/*field*/
	SFBool sizeQuant;	/*field*/
	SFFloat sizeMin;	/*field*/
	SFFloat sizeMax;	/*field*/
	SFInt32 sizeNbBits;	/*field*/
	SFBool useEfficientCoding;	/*field*/
} M_QuantizationParameter;


typedef struct _tagRectangle
{
	BASE_NODE
	SFVec2f size;	/*exposedField*/
} M_Rectangle;


typedef struct _tagScalarInterpolator
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat key;	/*exposedField*/
	MFFloat keyValue;	/*exposedField*/
	SFFloat value_changed;	/*eventOut*/
} M_ScalarInterpolator;


typedef struct _tagScript
{
	BASE_NODE
	MFScript url;	/*exposedField*/
	SFBool directOutput;	/*field*/
	SFBool mustEvaluate;	/*field*/
} M_Script;


typedef struct _tagShape
{
	BASE_NODE
	GF_Node *appearance;	/*exposedField*/
	GF_Node *geometry;	/*exposedField*/
} M_Shape;


typedef struct _tagSound
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
} M_Sound;


typedef struct _tagSound2D
{
	BASE_NODE
	SFFloat intensity;	/*exposedField*/
	SFVec2f location;	/*exposedField*/
	GF_Node *source;	/*exposedField*/
	SFBool spatialize;	/*field*/
} M_Sound2D;


typedef struct _tagSphere
{
	BASE_NODE
	SFFloat radius;	/*field*/
} M_Sphere;


typedef struct _tagSphereSensor
{
	BASE_NODE
	SFBool autoOffset;	/*exposedField*/
	SFBool enabled;	/*exposedField*/
	SFRotation offset;	/*exposedField*/
	SFBool isActive;	/*eventOut*/
	SFRotation rotation_changed;	/*eventOut*/
	SFVec3f trackPoint_changed;	/*eventOut*/
} M_SphereSensor;


typedef struct _tagSpotLight
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
} M_SpotLight;


typedef struct _tagSwitch
{
	BASE_NODE
	GF_ChildNodeItem *choice;	/*exposedField*/
	SFInt32 whichChoice;	/*exposedField*/
} M_Switch;


typedef struct _tagTermCap
{
	BASE_NODE
	SFTime evaluate;	/*eventIn*/
	void (*on_evaluate)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFInt32 capability;	/*exposedField*/
	SFInt32 value;	/*eventOut*/
} M_TermCap;


typedef struct _tagText
{
	BASE_NODE
	MFString string;	/*exposedField*/
	MFFloat length;	/*exposedField*/
	GF_Node *fontStyle;	/*exposedField*/
	SFFloat maxExtent;	/*exposedField*/
} M_Text;


typedef struct _tagTextureCoordinate
{
	BASE_NODE
	MFVec2f point;	/*exposedField*/
} M_TextureCoordinate;


typedef struct _tagTextureTransform
{
	BASE_NODE
	SFVec2f center;	/*exposedField*/
	SFFloat rotation;	/*exposedField*/
	SFVec2f scale;	/*exposedField*/
	SFVec2f translation;	/*exposedField*/
} M_TextureTransform;


typedef struct _tagTimeSensor
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
} M_TimeSensor;


typedef struct _tagTouchSensor
{
	BASE_NODE
	SFBool enabled;	/*exposedField*/
	SFVec3f hitNormal_changed;	/*eventOut*/
	SFVec3f hitPoint_changed;	/*eventOut*/
	SFVec2f hitTexCoord_changed;	/*eventOut*/
	SFBool isActive;	/*eventOut*/
	SFBool isOver;	/*eventOut*/
	SFTime touchTime;	/*eventOut*/
} M_TouchSensor;


typedef struct _tagTransform
{
	BASE_NODE
	VRML_CHILDREN
	SFVec3f center;	/*exposedField*/
	SFRotation rotation;	/*exposedField*/
	SFVec3f scale;	/*exposedField*/
	SFRotation scaleOrientation;	/*exposedField*/
	SFVec3f translation;	/*exposedField*/
} M_Transform;


typedef struct _tagTransform2D
{
	BASE_NODE
	VRML_CHILDREN
	SFVec2f center;	/*exposedField*/
	SFFloat rotationAngle;	/*exposedField*/
	SFVec2f scale;	/*exposedField*/
	SFFloat scaleOrientation;	/*exposedField*/
	SFVec2f translation;	/*exposedField*/
} M_Transform2D;


typedef struct _tagValuator
{
	BASE_NODE
	SFBool inSFBool;	/*eventIn*/
	void (*on_inSFBool)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFColor inSFColor;	/*eventIn*/
	void (*on_inSFColor)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFColor inMFColor;	/*eventIn*/
	void (*on_inMFColor)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFFloat inSFFloat;	/*eventIn*/
	void (*on_inSFFloat)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat inMFFloat;	/*eventIn*/
	void (*on_inMFFloat)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFInt32 inSFInt32;	/*eventIn*/
	void (*on_inSFInt32)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFInt32 inMFInt32;	/*eventIn*/
	void (*on_inMFInt32)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFRotation inSFRotation;	/*eventIn*/
	void (*on_inSFRotation)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFRotation inMFRotation;	/*eventIn*/
	void (*on_inMFRotation)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFString inSFString;	/*eventIn*/
	void (*on_inSFString)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFString inMFString;	/*eventIn*/
	void (*on_inMFString)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFTime inSFTime;	/*eventIn*/
	void (*on_inSFTime)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFVec2f inSFVec2f;	/*eventIn*/
	void (*on_inSFVec2f)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFVec2f inMFVec2f;	/*eventIn*/
	void (*on_inMFVec2f)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFVec3f inSFVec3f;	/*eventIn*/
	void (*on_inSFVec3f)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFVec3f inMFVec3f;	/*eventIn*/
	void (*on_inMFVec3f)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFBool outSFBool;	/*eventOut*/
	SFColor outSFColor;	/*eventOut*/
	MFColor outMFColor;	/*eventOut*/
	SFFloat outSFFloat;	/*eventOut*/
	MFFloat outMFFloat;	/*eventOut*/
	SFInt32 outSFInt32;	/*eventOut*/
	MFInt32 outMFInt32;	/*eventOut*/
	SFRotation outSFRotation;	/*eventOut*/
	MFRotation outMFRotation;	/*eventOut*/
	SFString outSFString;	/*eventOut*/
	MFString outMFString;	/*eventOut*/
	SFTime outSFTime;	/*eventOut*/
	SFVec2f outSFVec2f;	/*eventOut*/
	MFVec2f outMFVec2f;	/*eventOut*/
	SFVec3f outSFVec3f;	/*eventOut*/
	MFVec3f outMFVec3f;	/*eventOut*/
	SFFloat Factor1;	/*exposedField*/
	SFFloat Factor2;	/*exposedField*/
	SFFloat Factor3;	/*exposedField*/
	SFFloat Factor4;	/*exposedField*/
	SFFloat Offset1;	/*exposedField*/
	SFFloat Offset2;	/*exposedField*/
	SFFloat Offset3;	/*exposedField*/
	SFFloat Offset4;	/*exposedField*/
	SFBool Sum;	/*exposedField*/
} M_Valuator;


typedef struct _tagViewpoint
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
} M_Viewpoint;


typedef struct _tagVisibilitySensor
{
	BASE_NODE
	SFVec3f center;	/*exposedField*/
	SFBool enabled;	/*exposedField*/
	SFVec3f size;	/*exposedField*/
	SFTime enterTime;	/*eventOut*/
	SFTime exitTime;	/*eventOut*/
	SFBool isActive;	/*eventOut*/
} M_VisibilitySensor;


typedef struct _tagViseme
{
	BASE_NODE
	SFInt32 viseme_select1;	/*exposedField*/
	SFInt32 viseme_select2;	/*exposedField*/
	SFInt32 viseme_blend;	/*exposedField*/
	SFBool viseme_def;	/*exposedField*/
} M_Viseme;


typedef struct _tagWorldInfo
{
	BASE_NODE
	MFString info;	/*field*/
	SFString title;	/*field*/
} M_WorldInfo;


typedef struct _tagAcousticMaterial
{
	BASE_NODE
	SFFloat ambientIntensity;	/*exposedField*/
	SFColor diffuseColor;	/*exposedField*/
	SFColor emissiveColor;	/*exposedField*/
	SFFloat shininess;	/*exposedField*/
	SFColor specularColor;	/*exposedField*/
	SFFloat transparency;	/*exposedField*/
	MFFloat reffunc;	/*field*/
	MFFloat transfunc;	/*field*/
	MFFloat refFrequency;	/*field*/
	MFFloat transFrequency;	/*field*/
} M_AcousticMaterial;


typedef struct _tagAcousticScene
{
	BASE_NODE
	SFVec3f center;	/*field*/
	SFVec3f Size;	/*field*/
	MFTime reverbTime;	/*field*/
	MFFloat reverbFreq;	/*field*/
	SFFloat reverbLevel;	/*exposedField*/
	SFTime reverbDelay;	/*exposedField*/
} M_AcousticScene;


typedef struct _tagApplicationWindow
{
	BASE_NODE
	SFBool isActive;	/*exposedField*/
	SFTime startTime;	/*exposedField*/
	SFTime stopTime;	/*exposedField*/
	SFString description;	/*exposedField*/
	MFString parameter;	/*exposedField*/
	MFURL url;	/*exposedField*/
	SFVec2f size;	/*exposedField*/
} M_ApplicationWindow;


typedef struct _tagBAP
{
	BASE_NODE
	SFInt32 sacroiliac_tilt;	/*exposedField*/
	SFInt32 sacroiliac_torsion;	/*exposedField*/
	SFInt32 sacroiliac_roll;	/*exposedField*/
	SFInt32 l_hip_flexion;	/*exposedField*/
	SFInt32 r_hip_flexion;	/*exposedField*/
	SFInt32 l_hip_abduct;	/*exposedField*/
	SFInt32 r_hip_abduct;	/*exposedField*/
	SFInt32 l_hip_twisting;	/*exposedField*/
	SFInt32 r_hip_twisting;	/*exposedField*/
	SFInt32 l_knee_flexion;	/*exposedField*/
	SFInt32 r_knee_flexion;	/*exposedField*/
	SFInt32 l_knee_twisting;	/*exposedField*/
	SFInt32 r_knee_twisting;	/*exposedField*/
	SFInt32 l_ankle_flexion;	/*exposedField*/
	SFInt32 r_ankle_flexion;	/*exposedField*/
	SFInt32 l_ankle_twisting;	/*exposedField*/
	SFInt32 r_ankle_twisting;	/*exposedField*/
	SFInt32 l_subtalar_flexion;	/*exposedField*/
	SFInt32 r_subtalar_flexion;	/*exposedField*/
	SFInt32 l_midtarsal_flexion;	/*exposedField*/
	SFInt32 r_midtarsal_flexion;	/*exposedField*/
	SFInt32 l_metatarsal_flexion;	/*exposedField*/
	SFInt32 r_metatarsal_flexion;	/*exposedField*/
	SFInt32 l_sternoclavicular_abduct;	/*exposedField*/
	SFInt32 r_sternoclavicular_abduct;	/*exposedField*/
	SFInt32 l_sternoclavicular_rotate;	/*exposedField*/
	SFInt32 r_sternoclavicular_rotate;	/*exposedField*/
	SFInt32 l_acromioclavicular_abduct;	/*exposedField*/
	SFInt32 r_acromioclavicular_abduct;	/*exposedField*/
	SFInt32 l_acromioclavicular_rotate;	/*exposedField*/
	SFInt32 r_acromioclavicular_rotate;	/*exposedField*/
	SFInt32 l_shoulder_flexion;	/*exposedField*/
	SFInt32 r_shoulder_flexion;	/*exposedField*/
	SFInt32 l_shoulder_abduct;	/*exposedField*/
	SFInt32 r_shoulder_abduct;	/*exposedField*/
	SFInt32 l_shoulder_twisting;	/*exposedField*/
	SFInt32 r_shoulder_twisting;	/*exposedField*/
	SFInt32 l_elbow_flexion;	/*exposedField*/
	SFInt32 r_elbow_flexion;	/*exposedField*/
	SFInt32 l_elbow_twisting;	/*exposedField*/
	SFInt32 r_elbow_twisting;	/*exposedField*/
	SFInt32 l_wrist_flexion;	/*exposedField*/
	SFInt32 r_wrist_flexion;	/*exposedField*/
	SFInt32 l_wrist_pivot;	/*exposedField*/
	SFInt32 r_wrist_pivot;	/*exposedField*/
	SFInt32 l_wrist_twisting;	/*exposedField*/
	SFInt32 r_wrist_twisting;	/*exposedField*/
	SFInt32 skullbase_roll;	/*exposedField*/
	SFInt32 skullbase_torsion;	/*exposedField*/
	SFInt32 skullbase_tilt;	/*exposedField*/
	SFInt32 vc1roll;	/*exposedField*/
	SFInt32 vc1torsion;	/*exposedField*/
	SFInt32 vc1tilt;	/*exposedField*/
	SFInt32 vc2roll;	/*exposedField*/
	SFInt32 vc2torsion;	/*exposedField*/
	SFInt32 vc2tilt;	/*exposedField*/
	SFInt32 vc3roll;	/*exposedField*/
	SFInt32 vc3torsion;	/*exposedField*/
	SFInt32 vc3tilt;	/*exposedField*/
	SFInt32 vc4roll;	/*exposedField*/
	SFInt32 vc4torsion;	/*exposedField*/
	SFInt32 vc4tilt;	/*exposedField*/
	SFInt32 vc5roll;	/*exposedField*/
	SFInt32 vc5torsion;	/*exposedField*/
	SFInt32 vc5tilt;	/*exposedField*/
	SFInt32 vc6roll;	/*exposedField*/
	SFInt32 vc6torsion;	/*exposedField*/
	SFInt32 vc6tilt;	/*exposedField*/
	SFInt32 vc7roll;	/*exposedField*/
	SFInt32 vc7torsion;	/*exposedField*/
	SFInt32 vc7tilt;	/*exposedField*/
	SFInt32 vt1roll;	/*exposedField*/
	SFInt32 vt1torsion;	/*exposedField*/
	SFInt32 vt1tilt;	/*exposedField*/
	SFInt32 vt2roll;	/*exposedField*/
	SFInt32 vt2torsion;	/*exposedField*/
	SFInt32 vt2tilt;	/*exposedField*/
	SFInt32 vt3roll;	/*exposedField*/
	SFInt32 vt3torsion;	/*exposedField*/
	SFInt32 vt3tilt;	/*exposedField*/
	SFInt32 vt4roll;	/*exposedField*/
	SFInt32 vt4torsion;	/*exposedField*/
	SFInt32 vt4tilt;	/*exposedField*/
	SFInt32 vt5roll;	/*exposedField*/
	SFInt32 vt5torsion;	/*exposedField*/
	SFInt32 vt5tilt;	/*exposedField*/
	SFInt32 vt6roll;	/*exposedField*/
	SFInt32 vt6torsion;	/*exposedField*/
	SFInt32 vt6tilt;	/*exposedField*/
	SFInt32 vt7roll;	/*exposedField*/
	SFInt32 vt7torsion;	/*exposedField*/
	SFInt32 vt7tilt;	/*exposedField*/
	SFInt32 vt8roll;	/*exposedField*/
	SFInt32 vt8torsion;	/*exposedField*/
	SFInt32 vt8tilt;	/*exposedField*/
	SFInt32 vt9roll;	/*exposedField*/
	SFInt32 vt9torsion;	/*exposedField*/
	SFInt32 vt9tilt;	/*exposedField*/
	SFInt32 vt10roll;	/*exposedField*/
	SFInt32 vt10torsion;	/*exposedField*/
	SFInt32 vt10tilt;	/*exposedField*/
	SFInt32 vt11roll;	/*exposedField*/
	SFInt32 vt11torsion;	/*exposedField*/
	SFInt32 vt11tilt;	/*exposedField*/
	SFInt32 vt12roll;	/*exposedField*/
	SFInt32 vt12torsion;	/*exposedField*/
	SFInt32 vt12tilt;	/*exposedField*/
	SFInt32 vl1roll;	/*exposedField*/
	SFInt32 vl1torsion;	/*exposedField*/
	SFInt32 vl1tilt;	/*exposedField*/
	SFInt32 vl2roll;	/*exposedField*/
	SFInt32 vl2torsion;	/*exposedField*/
	SFInt32 vl2tilt;	/*exposedField*/
	SFInt32 vl3roll;	/*exposedField*/
	SFInt32 vl3torsion;	/*exposedField*/
	SFInt32 vl3tilt;	/*exposedField*/
	SFInt32 vl4roll;	/*exposedField*/
	SFInt32 vl4torsion;	/*exposedField*/
	SFInt32 vl4tilt;	/*exposedField*/
	SFInt32 vl5roll;	/*exposedField*/
	SFInt32 vl5torsion;	/*exposedField*/
	SFInt32 vl5tilt;	/*exposedField*/
	SFInt32 l_pinky0_flexion;	/*exposedField*/
	SFInt32 r_pinky0_flexion;	/*exposedField*/
	SFInt32 l_pinky1_flexion;	/*exposedField*/
	SFInt32 r_pinky1_flexion;	/*exposedField*/
	SFInt32 l_pinky1_pivot;	/*exposedField*/
	SFInt32 r_pinky1_pivot;	/*exposedField*/
	SFInt32 l_pinky1_twisting;	/*exposedField*/
	SFInt32 r_pinky1_twisting;	/*exposedField*/
	SFInt32 l_pinky2_flexion;	/*exposedField*/
	SFInt32 r_pinky2_flexion;	/*exposedField*/
	SFInt32 l_pinky3_flexion;	/*exposedField*/
	SFInt32 r_pinky3_flexion;	/*exposedField*/
	SFInt32 l_ring0_flexion;	/*exposedField*/
	SFInt32 r_ring0_flexion;	/*exposedField*/
	SFInt32 l_ring1_flexion;	/*exposedField*/
	SFInt32 r_ring1_flexion;	/*exposedField*/
	SFInt32 l_ring1_pivot;	/*exposedField*/
	SFInt32 r_ring1_pivot;	/*exposedField*/
	SFInt32 l_ring1_twisting;	/*exposedField*/
	SFInt32 r_ring1_twisting;	/*exposedField*/
	SFInt32 l_ring2_flexion;	/*exposedField*/
	SFInt32 r_ring2_flexion;	/*exposedField*/
	SFInt32 l_ring3_flexion;	/*exposedField*/
	SFInt32 r_ring3_flexion;	/*exposedField*/
	SFInt32 l_middle0_flexion;	/*exposedField*/
	SFInt32 r_middle0_flexion;	/*exposedField*/
	SFInt32 l_middle1_flexion;	/*exposedField*/
	SFInt32 r_middle1_flexion;	/*exposedField*/
	SFInt32 l_middle1_pivot;	/*exposedField*/
	SFInt32 r_middle1_pivot;	/*exposedField*/
	SFInt32 l_middle1_twisting;	/*exposedField*/
	SFInt32 r_middle1_twisting;	/*exposedField*/
	SFInt32 l_middle2_flexion;	/*exposedField*/
	SFInt32 r_middle2_flexion;	/*exposedField*/
	SFInt32 l_middle3_flexion;	/*exposedField*/
	SFInt32 r_middle3_flexion;	/*exposedField*/
	SFInt32 l_index0_flexion;	/*exposedField*/
	SFInt32 r_index0_flexion;	/*exposedField*/
	SFInt32 l_index1_flexion;	/*exposedField*/
	SFInt32 r_index1_flexion;	/*exposedField*/
	SFInt32 l_index1_pivot;	/*exposedField*/
	SFInt32 r_index1_pivot;	/*exposedField*/
	SFInt32 l_index1_twisting;	/*exposedField*/
	SFInt32 r_index1_twisting;	/*exposedField*/
	SFInt32 l_index2_flexion;	/*exposedField*/
	SFInt32 r_index2_flexion;	/*exposedField*/
	SFInt32 l_index3_flexion;	/*exposedField*/
	SFInt32 r_index3_flexion;	/*exposedField*/
	SFInt32 l_thumb1_flexion;	/*exposedField*/
	SFInt32 r_thumb1_flexion;	/*exposedField*/
	SFInt32 l_thumb1_pivot;	/*exposedField*/
	SFInt32 r_thumb1_pivot;	/*exposedField*/
	SFInt32 l_thumb1_twisting;	/*exposedField*/
	SFInt32 r_thumb1_twisting;	/*exposedField*/
	SFInt32 l_thumb2_flexion;	/*exposedField*/
	SFInt32 r_thumb2_flexion;	/*exposedField*/
	SFInt32 l_thumb3_flexion;	/*exposedField*/
	SFInt32 r_thumb3_flexion;	/*exposedField*/
	SFInt32 HumanoidRoot_tr_vertical;	/*exposedField*/
	SFInt32 HumanoidRoot_tr_lateral;	/*exposedField*/
	SFInt32 HumanoidRoot_tr_frontal;	/*exposedField*/
	SFInt32 HumanoidRoot_rt_body_turn;	/*exposedField*/
	SFInt32 HumanoidRoot_rt_body_roll;	/*exposedField*/
	SFInt32 HumanoidRoot_rt_body_tilt;	/*exposedField*/
	SFInt32 extensionBap187;	/*exposedField*/
	SFInt32 extensionBap188;	/*exposedField*/
	SFInt32 extensionBap189;	/*exposedField*/
	SFInt32 extensionBap190;	/*exposedField*/
	SFInt32 extensionBap191;	/*exposedField*/
	SFInt32 extensionBap192;	/*exposedField*/
	SFInt32 extensionBap193;	/*exposedField*/
	SFInt32 extensionBap194;	/*exposedField*/
	SFInt32 extensionBap195;	/*exposedField*/
	SFInt32 extensionBap196;	/*exposedField*/
	SFInt32 extensionBap197;	/*exposedField*/
	SFInt32 extensionBap198;	/*exposedField*/
	SFInt32 extensionBap199;	/*exposedField*/
	SFInt32 extensionBap200;	/*exposedField*/
	SFInt32 extensionBap201;	/*exposedField*/
	SFInt32 extensionBap202;	/*exposedField*/
	SFInt32 extensionBap203;	/*exposedField*/
	SFInt32 extensionBap204;	/*exposedField*/
	SFInt32 extensionBap205;	/*exposedField*/
	SFInt32 extensionBap206;	/*exposedField*/
	SFInt32 extensionBap207;	/*exposedField*/
	SFInt32 extensionBap208;	/*exposedField*/
	SFInt32 extensionBap209;	/*exposedField*/
	SFInt32 extensionBap210;	/*exposedField*/
	SFInt32 extensionBap211;	/*exposedField*/
	SFInt32 extensionBap212;	/*exposedField*/
	SFInt32 extensionBap213;	/*exposedField*/
	SFInt32 extensionBap214;	/*exposedField*/
	SFInt32 extensionBap215;	/*exposedField*/
	SFInt32 extensionBap216;	/*exposedField*/
	SFInt32 extensionBap217;	/*exposedField*/
	SFInt32 extensionBap218;	/*exposedField*/
	SFInt32 extensionBap219;	/*exposedField*/
	SFInt32 extensionBap220;	/*exposedField*/
	SFInt32 extensionBap221;	/*exposedField*/
	SFInt32 extensionBap222;	/*exposedField*/
	SFInt32 extensionBap223;	/*exposedField*/
	SFInt32 extensionBap224;	/*exposedField*/
	SFInt32 extensionBap225;	/*exposedField*/
	SFInt32 extensionBap226;	/*exposedField*/
	SFInt32 extensionBap227;	/*exposedField*/
	SFInt32 extensionBap228;	/*exposedField*/
	SFInt32 extensionBap229;	/*exposedField*/
	SFInt32 extensionBap230;	/*exposedField*/
	SFInt32 extensionBap231;	/*exposedField*/
	SFInt32 extensionBap232;	/*exposedField*/
	SFInt32 extensionBap233;	/*exposedField*/
	SFInt32 extensionBap234;	/*exposedField*/
	SFInt32 extensionBap235;	/*exposedField*/
	SFInt32 extensionBap236;	/*exposedField*/
	SFInt32 extensionBap237;	/*exposedField*/
	SFInt32 extensionBap238;	/*exposedField*/
	SFInt32 extensionBap239;	/*exposedField*/
	SFInt32 extensionBap240;	/*exposedField*/
	SFInt32 extensionBap241;	/*exposedField*/
	SFInt32 extensionBap242;	/*exposedField*/
	SFInt32 extensionBap243;	/*exposedField*/
	SFInt32 extensionBap244;	/*exposedField*/
	SFInt32 extensionBap245;	/*exposedField*/
	SFInt32 extensionBap246;	/*exposedField*/
	SFInt32 extensionBap247;	/*exposedField*/
	SFInt32 extensionBap248;	/*exposedField*/
	SFInt32 extensionBap249;	/*exposedField*/
	SFInt32 extensionBap250;	/*exposedField*/
	SFInt32 extensionBap251;	/*exposedField*/
	SFInt32 extensionBap252;	/*exposedField*/
	SFInt32 extensionBap253;	/*exposedField*/
	SFInt32 extensionBap254;	/*exposedField*/
	SFInt32 extensionBap255;	/*exposedField*/
	SFInt32 extensionBap256;	/*exposedField*/
	SFInt32 extensionBap257;	/*exposedField*/
	SFInt32 extensionBap258;	/*exposedField*/
	SFInt32 extensionBap259;	/*exposedField*/
	SFInt32 extensionBap260;	/*exposedField*/
	SFInt32 extensionBap261;	/*exposedField*/
	SFInt32 extensionBap262;	/*exposedField*/
	SFInt32 extensionBap263;	/*exposedField*/
	SFInt32 extensionBap264;	/*exposedField*/
	SFInt32 extensionBap265;	/*exposedField*/
	SFInt32 extensionBap266;	/*exposedField*/
	SFInt32 extensionBap267;	/*exposedField*/
	SFInt32 extensionBap268;	/*exposedField*/
	SFInt32 extensionBap269;	/*exposedField*/
	SFInt32 extensionBap270;	/*exposedField*/
	SFInt32 extensionBap271;	/*exposedField*/
	SFInt32 extensionBap272;	/*exposedField*/
	SFInt32 extensionBap273;	/*exposedField*/
	SFInt32 extensionBap274;	/*exposedField*/
	SFInt32 extensionBap275;	/*exposedField*/
	SFInt32 extensionBap276;	/*exposedField*/
	SFInt32 extensionBap277;	/*exposedField*/
	SFInt32 extensionBap278;	/*exposedField*/
	SFInt32 extensionBap279;	/*exposedField*/
	SFInt32 extensionBap280;	/*exposedField*/
	SFInt32 extensionBap281;	/*exposedField*/
	SFInt32 extensionBap282;	/*exposedField*/
	SFInt32 extensionBap283;	/*exposedField*/
	SFInt32 extensionBap284;	/*exposedField*/
	SFInt32 extensionBap285;	/*exposedField*/
	SFInt32 extensionBap286;	/*exposedField*/
	SFInt32 extensionBap287;	/*exposedField*/
	SFInt32 extensionBap288;	/*exposedField*/
	SFInt32 extensionBap289;	/*exposedField*/
	SFInt32 extensionBap290;	/*exposedField*/
	SFInt32 extensionBap291;	/*exposedField*/
	SFInt32 extensionBap292;	/*exposedField*/
	SFInt32 extensionBap293;	/*exposedField*/
	SFInt32 extensionBap294;	/*exposedField*/
	SFInt32 extensionBap295;	/*exposedField*/
	SFInt32 extensionBap296;	/*exposedField*/
} M_BAP;


typedef struct _tagBDP
{
	BASE_NODE
	GF_ChildNodeItem *bodyDefTables;	/*exposedField*/
	GF_ChildNodeItem *bodySceneGraph;	/*exposedField*/
} M_BDP;


typedef struct _tagBody
{
	BASE_NODE
	GF_Node *bdp;	/*exposedField*/
	GF_Node *bap;	/*exposedField*/
	GF_ChildNodeItem *renderedBody;	/*exposedField*/
} M_Body;


typedef struct _tagBodyDefTable
{
	BASE_NODE
	SFString bodySceneGraphNodeName;	/*exposedField*/
	MFInt32 bapIDs;	/*exposedField*/
	MFInt32 vertexIds;	/*exposedField*/
	MFInt32 bapCombinations;	/*exposedField*/
	MFVec3f displacements;	/*exposedField*/
	SFInt32 numInterpolateKeys;	/*exposedField*/
} M_BodyDefTable;


typedef struct _tagBodySegmentConnectionHint
{
	BASE_NODE
	SFString firstSegmentNodeName;	/*exposedField*/
	SFString secondSegmentNodeName;	/*exposedField*/
	MFInt32 firstVertexIdList;	/*exposedField*/
	MFInt32 secondVertexIdList;	/*exposedField*/
} M_BodySegmentConnectionHint;


typedef struct _tagDirectiveSound
{
	BASE_NODE
	SFVec3f direction;	/*exposedField*/
	SFFloat intensity;	/*exposedField*/
	SFVec3f location;	/*exposedField*/
	GF_Node *source;	/*exposedField*/
	GF_Node *perceptualParameters;	/*exposedField*/
	SFBool roomEffect;	/*exposedField*/
	SFBool spatialize;	/*exposedField*/
	MFFloat directivity;	/*field*/
	MFFloat angles;	/*field*/
	MFFloat frequency;	/*field*/
	SFFloat speedOfSound;	/*field*/
	SFFloat distance;	/*field*/
	SFBool useAirabs;	/*field*/
} M_DirectiveSound;


typedef struct _tagHierarchical3DMesh
{
	BASE_NODE
	SFInt32 triangleBudget;	/*eventIn*/
	void (*on_triangleBudget)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFFloat level;	/*exposedField*/
	MFURL url;	/*field*/
	SFBool doneLoading;	/*eventOut*/
} M_Hierarchical3DMesh;


typedef struct _tagMaterialKey
{
	BASE_NODE
	SFBool isKeyed;	/*exposedField*/
	SFBool isRGB;	/*exposedField*/
	SFColor keyColor;	/*exposedField*/
	SFFloat lowThreshold;	/*exposedField*/
	SFFloat highThreshold;	/*exposedField*/
	SFFloat transparency;	/*exposedField*/
} M_MaterialKey;


typedef struct _tagPerceptualParameters
{
	BASE_NODE
	SFFloat sourcePresence;	/*exposedField*/
	SFFloat sourceWarmth;	/*exposedField*/
	SFFloat sourceBrilliance;	/*exposedField*/
	SFFloat roomPresence;	/*exposedField*/
	SFFloat runningReverberance;	/*exposedField*/
	SFFloat envelopment;	/*exposedField*/
	SFFloat lateReverberance;	/*exposedField*/
	SFFloat heavyness;	/*exposedField*/
	SFFloat liveness;	/*exposedField*/
	MFFloat omniDirectivity;	/*exposedField*/
	MFFloat directFilterGains;	/*exposedField*/
	MFFloat inputFilterGains;	/*exposedField*/
	SFFloat refDistance;	/*exposedField*/
	SFFloat freqLow;	/*exposedField*/
	SFFloat freqHigh;	/*exposedField*/
	SFTime timeLimit1;	/*exposedField*/
	SFTime timeLimit2;	/*exposedField*/
	SFTime timeLimit3;	/*exposedField*/
	SFTime modalDensity;	/*exposedField*/
} M_PerceptualParameters;


typedef struct _tagTemporalTransform
{
	BASE_NODE
	VRML_CHILDREN
	MFURL url;	/*exposedField*/
	SFTime startTime;	/*exposedField*/
	SFTime optimalDuration;	/*exposedField*/
	SFBool active;	/*exposedField*/
	SFFloat speed;	/*exposedField*/
	SFVec2f scalability;	/*exposedField*/
	MFInt32 stretchMode;	/*exposedField*/
	MFInt32 shrinkMode;	/*exposedField*/
	SFTime maxDelay;	/*exposedField*/
	SFTime actualDuration;	/*eventOut*/
} M_TemporalTransform;


typedef struct _tagTemporalGroup
{
	BASE_NODE
	VRML_CHILDREN
	SFBool costart;	/*field*/
	SFBool coend;	/*field*/
	SFBool meet;	/*field*/
	MFFloat priority;	/*exposedField*/
	SFBool isActive;	/*eventOut*/
	SFInt32 activeChild;	/*eventOut*/
} M_TemporalGroup;


typedef struct _tagServerCommand
{
	BASE_NODE
	SFBool trigger;	/*eventIn*/
	void (*on_trigger)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFBool enable;	/*exposedField*/
	MFURL url;	/*exposedField*/
	SFString command;	/*exposedField*/
} M_ServerCommand;


typedef struct _tagInputSensor
{
	BASE_NODE
	SFBool enabled;	/*exposedField*/
	SFCommandBuffer buffer;	/*exposedField*/
	MFURL url;	/*exposedField*/
	SFTime eventTime;	/*eventOut*/
} M_InputSensor;


typedef struct _tagMatteTexture
{
	BASE_NODE
	GF_Node *surfaceA;	/*field*/
	GF_Node *surfaceB;	/*field*/
	GF_Node *alphaSurface;	/*field*/
	SFString operation;	/*exposedField*/
	SFBool overwrite;	/*field*/
	SFFloat fraction;	/*exposedField*/
	MFFloat parameter;	/*exposedField*/
} M_MatteTexture;


typedef struct _tagMediaBuffer
{
	BASE_NODE
	SFFloat bufferSize;	/*exposedField*/
	MFURL url;	/*exposedField*/
	SFTime mediaStartTime;	/*exposedField*/
	SFTime mediaStopTime;	/*exposedField*/
	SFBool isBuffered;	/*eventOut*/
	SFBool enabled;	/*exposedField*/
} M_MediaBuffer;


typedef struct _tagMediaControl
{
	BASE_NODE
	MFURL url;	/*exposedField*/
	SFTime mediaStartTime;	/*exposedField*/
	SFTime mediaStopTime;	/*exposedField*/
	SFFloat mediaSpeed;	/*exposedField*/
	SFBool loop;	/*exposedField*/
	SFBool preRoll;	/*exposedField*/
	SFBool mute;	/*exposedField*/
	SFBool enabled;	/*exposedField*/
	SFBool isPreRolled;	/*eventOut*/
} M_MediaControl;


typedef struct _tagMediaSensor
{
	BASE_NODE
	MFURL url;	/*exposedField*/
	SFTime mediaCurrentTime;	/*eventOut*/
	SFTime streamObjectStartTime;	/*eventOut*/
	SFTime mediaDuration;	/*eventOut*/
	SFBool isActive;	/*eventOut*/
	MFString info;	/*eventOut*/
} M_MediaSensor;


typedef struct _tagBitWrapper
{
	BASE_NODE
	GF_Node *node;	/*field*/
	SFInt32 type;	/*field*/
	MFURL url;	/*field*/
	SFString buffer;	/*field*/
} M_BitWrapper;


typedef struct _tagCoordinateInterpolator4D
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat key;	/*exposedField*/
	MFVec4f keyValue;	/*exposedField*/
	MFVec4f value_changed;	/*eventOut*/
} M_CoordinateInterpolator4D;


typedef struct _tagDepthImage
{
	BASE_NODE
	GF_Node *diTexture;	/*field*/
	SFFloat farPlane;	/*field*/
	SFVec2f fieldOfView;	/*field*/
	SFFloat nearPlane;	/*field*/
	SFRotation orientation;	/*field*/
	SFBool orthographic;	/*field*/
	SFVec3f position;	/*field*/
} M_DepthImage;


typedef struct _tagFFD
{
	BASE_NODE
	VRML_CHILDREN
	MFVec4f controlPoint;	/*exposedField*/
	SFInt32 uDimension;	/*field*/
	MFFloat uKnot;	/*field*/
	SFInt32 uOrder;	/*field*/
	SFInt32 vDimension;	/*field*/
	MFFloat vKnot;	/*field*/
	SFInt32 vOrder;	/*field*/
	SFInt32 wDimension;	/*field*/
	MFFloat wKnot;	/*field*/
	SFInt32 wOrder;	/*field*/
} M_FFD;


typedef struct _tagImplicit
{
	BASE_NODE
	SFVec3f bboxSize;	/*exposedField*/
	MFFloat c;	/*exposedField*/
	MFInt32 densities;	/*exposedField*/
	SFBool dual;	/*exposedField*/
	SFBool solid;	/*exposedField*/
} M_Implicit;


typedef struct _tagXXLFM_Appearance
{
	BASE_NODE
	GF_Node *blendList;	/*exposedField*/
	GF_ChildNodeItem *lightMapList;	/*exposedField*/
	GF_ChildNodeItem *tileList;	/*exposedField*/
	GF_Node *vertexFrameList;	/*exposedField*/
} M_XXLFM_Appearance;


typedef struct _tagXXLFM_BlendList
{
	BASE_NODE
	MFInt32 blendMode;	/*exposedField*/
	MFInt32 lightMapIndex;	/*exposedField*/
} M_XXLFM_BlendList;


typedef struct _tagXXLFM_FrameList
{
	BASE_NODE
	MFInt32 index;	/*exposedField*/
	MFVec3f frame;	/*exposedField*/
} M_XXLFM_FrameList;


typedef struct _tagXXLFM_LightMap
{
	BASE_NODE
	SFVec3f biasRGB;	/*exposedField*/
	SFInt32 priorityLevel;	/*exposedField*/
	SFVec3f scaleRGB;	/*exposedField*/
	GF_Node *surfaceMapList;	/*exposedField*/
	GF_Node *viewMapList;	/*exposedField*/
} M_XXLFM_LightMap;


typedef struct _tagXXLFM_SurfaceMapList
{
	BASE_NODE
	MFInt32 tileIndex;	/*exposedField*/
	GF_Node *triangleCoordinate;	/*exposedField*/
	MFInt32 triangleIndex;	/*exposedField*/
	MFInt32 viewMapIndex;	/*exposedField*/
} M_XXLFM_SurfaceMapList;


typedef struct _tagXXLFM_ViewMapList
{
	BASE_NODE
	GF_Node *textureOrigin;	/*exposedField*/
	GF_Node *textureSize;	/*exposedField*/
	MFInt32 tileIndex;	/*exposedField*/
	MFInt32 vertexIndex;	/*exposedField*/
} M_XXLFM_ViewMapList;


typedef struct _tagMeshGrid
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
	SFInt32 displayLevel;	/*exposedField*/
	SFInt32 filterType;	/*exposedField*/
	GF_Node *gridCoord;	/*exposedField*/
	SFInt32 hierarchicalLevel;	/*exposedField*/
	MFInt32 nLevels;	/*exposedField*/
	GF_Node *normal;	/*exposedField*/
	MFInt32 nSlices;	/*exposedField*/
	GF_Node *texCoord;	/*exposedField*/
	MFFloat vertexOffset;	/*exposedField*/
	MFInt32 vertexLink;	/*exposedField*/
	MFInt32 colorIndex;	/*field*/
	MFInt32 coordIndex;	/*field*/
	MFInt32 normalIndex;	/*field*/
	SFBool solid;	/*field*/
	MFInt32 texCoordIndex;	/*field*/
	SFBool isLoading;	/*eventOut*/
	MFInt32 nVertices;	/*eventOut*/
} M_MeshGrid;


typedef struct _tagNonLinearDeformer
{
	BASE_NODE
	SFVec3f axis;	/*exposedField*/
	MFFloat extend;	/*exposedField*/
	GF_Node *geometry;	/*exposedField*/
	SFFloat param;	/*exposedField*/
	SFInt32 type;	/*exposedField*/
} M_NonLinearDeformer;


typedef struct _tagNurbsCurve
{
	BASE_NODE
	MFInt32 set_colorIndex;	/*eventIn*/
	void (*on_set_colorIndex)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	GF_Node *color;	/*exposedField*/
	MFVec4f controlPoint;	/*exposedField*/
	SFInt32 tessellation;	/*exposedField*/
	MFInt32 colorIndex;	/*field*/
	SFBool colorPerVertex;	/*field*/
	MFFloat knot;	/*field*/
	SFInt32 order;	/*field*/
} M_NurbsCurve;


typedef struct _tagNurbsCurve2D
{
	BASE_NODE
	MFInt32 set_colorIndex;	/*eventIn*/
	void (*on_set_colorIndex)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	GF_Node *color;	/*exposedField*/
	MFVec3f controlPoint;	/*exposedField*/
	SFInt32 tessellation;	/*exposedField*/
	MFInt32 colorIndex;	/*field*/
	SFBool colorPerVertex;	/*field*/
	MFFloat knot;	/*field*/
	SFInt32 order;	/*field*/
} M_NurbsCurve2D;


typedef struct _tagNurbsSurface
{
	BASE_NODE
	MFInt32 set_colorIndex;	/*eventIn*/
	void (*on_set_colorIndex)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFInt32 set_texColorIndex;	/*eventIn*/
	void (*on_set_texColorIndex)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	GF_Node *color;	/*exposedField*/
	MFVec4f controlPoint;	/*exposedField*/
	GF_Node *texCoord;	/*exposedField*/
	SFInt32 uTessellation;	/*exposedField*/
	SFInt32 vTessellation;	/*exposedField*/
	SFBool ccw;	/*field*/
	MFInt32 colorIndex;	/*field*/
	SFBool colorPerVertex;	/*field*/
	SFBool solid;	/*field*/
	MFInt32 texColorIndex;	/*field*/
	SFInt32 uDimension;	/*field*/
	MFFloat uKnot;	/*field*/
	SFInt32 uOrder;	/*field*/
	SFInt32 vDimension;	/*field*/
	MFFloat vKnot;	/*field*/
	SFInt32 vOrder;	/*field*/
} M_NurbsSurface;


typedef struct _tagOctreeImage
{
	BASE_NODE
	GF_ChildNodeItem *images;	/*field*/
	MFInt32 octree;	/*field*/
	SFInt32 octreeResolution;	/*field*/
	MFInt32 voxelImageIndex;	/*field*/
} M_OctreeImage;


typedef struct _tagXXParticles
{
	BASE_NODE
	SFFloat creationRate;	/*exposedField*/
	SFFloat creationRateVariation;	/*exposedField*/
	SFFloat emitAlpha;	/*exposedField*/
	SFColor emitColor;	/*exposedField*/
	SFColor emitColorVariation;	/*exposedField*/
	SFVec3f emitterPosition;	/*exposedField*/
	SFVec3f emitVelocity;	/*exposedField*/
	SFVec3f emitVelocityVariation;	/*exposedField*/
	SFBool enabled;	/*exposedField*/
	SFFloat fadeAlpha;	/*exposedField*/
	SFColor fadeColor;	/*exposedField*/
	SFFloat fadeRate;	/*exposedField*/
	SFVec3f force;	/*exposedField*/
	GF_ChildNodeItem *influences;	/*exposedField*/
	GF_Node *init;	/*exposedField*/
	SFTime maxLifeTime;	/*exposedField*/
	SFFloat maxLifeTimeVariation;	/*exposedField*/
	SFInt32 maxParticles;	/*exposedField*/
	SFFloat minRange;	/*exposedField*/
	SFFloat maxRange;	/*exposedField*/
	GF_Node *primitive;	/*exposedField*/
	SFInt32 primitiveType;	/*exposedField*/
	SFFloat particleRadius;	/*exposedField*/
	SFFloat particleRadiusRate;	/*exposedField*/
	SFFloat particleRadiusVariation;	/*exposedField*/
} M_XXParticles;


typedef struct _tagXXParticleInitBox
{
	BASE_NODE
	SFFloat falloff;	/*exposedField*/
	SFVec3f size;	/*exposedField*/
} M_XXParticleInitBox;


typedef struct _tagXXPlanarObstacle
{
	BASE_NODE
	SFVec3f distance;	/*exposedField*/
	SFVec3f normal;	/*exposedField*/
	SFFloat reflection;	/*exposedField*/
	SFFloat absorption;	/*exposedField*/
} M_XXPlanarObstacle;


typedef struct _tagXXPointAttractor
{
	BASE_NODE
	SFFloat innerRadius;	/*exposedField*/
	SFFloat outerRadius;	/*exposedField*/
	SFVec3f position;	/*exposedField*/
	SFFloat rate;	/*exposedField*/
} M_XXPointAttractor;


typedef struct _tagPointTexture
{
	BASE_NODE
	MFColor color;	/*field*/
	MFInt32 depth;	/*field*/
	SFInt32 depthNbBits;	/*field*/
	SFInt32 height;	/*field*/
	SFInt32 width;	/*field*/
} M_PointTexture;


typedef struct _tagPositionAnimator
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFVec2f fromTo;	/*exposedField*/
	MFFloat key;	/*exposedField*/
	MFRotation keyOrientation;	/*exposedField*/
	SFInt32 keyType;	/*exposedField*/
	MFVec2f keySpline;	/*exposedField*/
	MFVec3f keyValue;	/*exposedField*/
	SFInt32 keyValueType;	/*exposedField*/
	SFVec3f offset;	/*exposedField*/
	MFFloat weight;	/*exposedField*/
	SFVec3f endValue;	/*eventOut*/
	SFRotation rotation_changed;	/*eventOut*/
	SFVec3f value_changed;	/*eventOut*/
} M_PositionAnimator;


typedef struct _tagPositionAnimator2D
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFVec2f fromTo;	/*exposedField*/
	MFFloat key;	/*exposedField*/
	SFInt32 keyOrientation;	/*exposedField*/
	SFInt32 keyType;	/*exposedField*/
	MFVec2f keySpline;	/*exposedField*/
	MFVec2f keyValue;	/*exposedField*/
	SFInt32 keyValueType;	/*exposedField*/
	SFVec2f offset;	/*exposedField*/
	MFFloat weight;	/*exposedField*/
	SFVec2f endValue;	/*eventOut*/
	SFFloat rotation_changed;	/*eventOut*/
	SFVec2f value_changed;	/*eventOut*/
} M_PositionAnimator2D;


typedef struct _tagPositionInterpolator4D
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFFloat key;	/*exposedField*/
	MFVec4f keyValue;	/*exposedField*/
	SFVec4f value_changed;	/*eventOut*/
} M_PositionInterpolator4D;


typedef struct _tagProceduralTexture
{
	BASE_NODE
	SFBool aSmooth;	/*exposedField*/
	MFVec2f aWarpmap;	/*exposedField*/
	MFFloat aWeights;	/*exposedField*/
	SFBool bSmooth;	/*exposedField*/
	MFVec2f bWarpmap;	/*exposedField*/
	MFFloat bWeights;	/*exposedField*/
	SFInt32 cellWidth;	/*exposedField*/
	SFInt32 cellHeight;	/*exposedField*/
	MFColor color;	/*exposedField*/
	SFFloat distortion;	/*exposedField*/
	SFInt32 height;	/*exposedField*/
	SFInt32 roughness;	/*exposedField*/
	SFInt32 seed;	/*exposedField*/
	SFInt32 type;	/*exposedField*/
	SFBool xSmooth;	/*exposedField*/
	MFVec2f xWarpmap;	/*exposedField*/
	SFBool ySmooth;	/*exposedField*/
	MFVec2f yWarpmap;	/*exposedField*/
	SFInt32 width;	/*exposedField*/
	SFImage image_changed;	/*eventOut*/
} M_ProceduralTexture;


typedef struct _tagQuadric
{
	BASE_NODE
	SFVec3f bboxSize;	/*exposedField*/
	MFInt32 densities;	/*exposedField*/
	SFBool dual;	/*exposedField*/
	SFVec4f P0;	/*exposedField*/
	SFVec4f P1;	/*exposedField*/
	SFVec4f P2;	/*exposedField*/
	SFVec4f P3;	/*exposedField*/
	SFVec4f P4;	/*exposedField*/
	SFVec4f P5;	/*exposedField*/
	SFBool solid;	/*exposedField*/
} M_Quadric;


typedef struct _tagSBBone
{
	BASE_NODE
	VRML_CHILDREN
	SFInt32 boneID;	/*exposedField*/
	SFVec3f center;	/*exposedField*/
	SFVec3f endpoint;	/*exposedField*/
	SFInt32 falloff;	/*exposedField*/
	SFInt32 ikChainPosition;	/*exposedField*/
	MFFloat ikPitchLimit;	/*exposedField*/
	MFFloat ikRollLimit;	/*exposedField*/
	MFFloat ikTxLimit;	/*exposedField*/
	MFFloat ikTyLimit;	/*exposedField*/
	MFFloat ikTzLimit;	/*exposedField*/
	MFFloat ikYawLimit;	/*exposedField*/
	SFRotation rotation;	/*exposedField*/
	SFInt32 rotationOrder;	/*exposedField*/
	SFVec3f scale;	/*exposedField*/
	SFRotation scaleOrientation;	/*exposedField*/
	MFFloat sectionInner;	/*exposedField*/
	MFFloat sectionOuter;	/*exposedField*/
	MFFloat sectionPosition;	/*exposedField*/
	MFInt32 skinCoordIndex;	/*exposedField*/
	MFFloat skinCoordWeight;	/*exposedField*/
	SFVec3f translation;	/*exposedField*/
} M_SBBone;


typedef struct _tagSBMuscle
{
	BASE_NODE
	SFInt32 falloff;	/*exposedField*/
	GF_Node *muscleCurve;	/*exposedField*/
	SFInt32 muscleID;	/*exposedField*/
	SFInt32 radius;	/*exposedField*/
	MFInt32 skinCoordIndex;	/*exposedField*/
	MFFloat skinCoordWeight;	/*exposedField*/
} M_SBMuscle;


typedef struct _tagSBSegment
{
	BASE_NODE
	VRML_CHILDREN
	SFVec3f centerOfMass;	/*exposedField*/
	SFFloat mass;	/*exposedField*/
	MFVec3f momentsOfInertia;	/*exposedField*/
	SFString name;	/*exposedField*/
} M_SBSegment;


typedef struct _tagSBSite
{
	BASE_NODE
	VRML_CHILDREN
	SFVec3f center;	/*exposedField*/
	SFString name;	/*exposedField*/
	SFRotation rotation;	/*exposedField*/
	SFVec3f scale;	/*exposedField*/
	SFRotation scaleOrientation;	/*exposedField*/
	SFVec3f translation;	/*exposedField*/
} M_SBSite;


typedef struct _tagSBSkinnedModel
{
	BASE_NODE
	GF_ChildNodeItem *bones;	/*exposedField*/
	SFVec3f center;	/*exposedField*/
	GF_ChildNodeItem *muscles;	/*exposedField*/
	SFString name;	/*exposedField*/
	SFRotation rotation;	/*exposedField*/
	GF_ChildNodeItem *segments;	/*exposedField*/
	SFVec3f scale;	/*exposedField*/
	SFRotation scaleOrientation;	/*exposedField*/
	GF_ChildNodeItem *sites;	/*exposedField*/
	GF_ChildNodeItem *skeleton;	/*exposedField*/
	GF_ChildNodeItem *skin;	/*exposedField*/
	GF_Node *skinCoord;	/*exposedField*/
	GF_Node *skinNormal;	/*exposedField*/
	SFVec3f translation;	/*exposedField*/
	GF_Node *weighsComputationSkinCoord;	/*exposedField*/
} M_SBSkinnedModel;


typedef struct _tagSBVCAnimation
{
	BASE_NODE
	MFURL url;	/*exposedField*/
	GF_ChildNodeItem *virtualCharacters;	/*exposedField*/
} M_SBVCAnimation;


typedef struct _tagScalarAnimator
{
	BASE_NODE
	SFFloat set_fraction;	/*eventIn*/
	void (*on_set_fraction)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFVec2f fromTo;	/*exposedField*/
	MFFloat key;	/*exposedField*/
	SFInt32 keyType;	/*exposedField*/
	MFVec2f keySpline;	/*exposedField*/
	MFFloat keyValue;	/*exposedField*/
	SFInt32 keyValueType;	/*exposedField*/
	SFFloat offset;	/*exposedField*/
	MFFloat weight;	/*exposedField*/
	SFFloat endValue;	/*eventOut*/
	SFFloat value_changed;	/*eventOut*/
} M_ScalarAnimator;


typedef struct _tagSimpleTexture
{
	BASE_NODE
	GF_Node *depth;	/*field*/
	GF_Node *texture;	/*field*/
} M_SimpleTexture;


typedef struct _tagSolidRep
{
	BASE_NODE
	SFVec3f bboxSize;	/*exposedField*/
	MFInt32 densityList;	/*exposedField*/
	GF_Node *solidTree;	/*exposedField*/
} M_SolidRep;


typedef struct _tagSubdivisionSurface
{
	BASE_NODE
	MFInt32 set_colorIndex;	/*eventIn*/
	void (*on_set_colorIndex)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFInt32 set_coordIndex;	/*eventIn*/
	void (*on_set_coordIndex)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFInt32 set_cornerVertexIndex;	/*eventIn*/
	void (*on_set_cornerVertexIndex)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFInt32 set_creaseEdgeIndex;	/*eventIn*/
	void (*on_set_creaseEdgeIndex)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFInt32 set_creaseVertexIndex;	/*eventIn*/
	void (*on_set_creaseVertexIndex)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFInt32 set_dartVertexIndex;	/*eventIn*/
	void (*on_set_dartVertexIndex)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFInt32 set_texCoordIndex;	/*eventIn*/
	void (*on_set_texCoordIndex)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	GF_Node *color;	/*exposedField*/
	GF_Node *coord;	/*exposedField*/
	GF_Node *texCoord;	/*exposedField*/
	GF_ChildNodeItem *sectors;	/*exposedField*/
	SFInt32 subdivisionLevel;	/*exposedField*/
	SFInt32 subdivisionType;	/*exposedField*/
	SFInt32 subdivisionSubType;	/*exposedField*/
	MFInt32 invisibleEdgeIndex;	/*field*/
	SFBool ccw;	/*field*/
	MFInt32 colorIndex;	/*field*/
	SFBool colorPerVertex;	/*field*/
	SFBool convex;	/*field*/
	MFInt32 coordIndex;	/*field*/
	MFInt32 cornerVertexIndex;	/*field*/
	MFInt32 creaseEdgeIndex;	/*field*/
	MFInt32 creaseVertexIndex;	/*field*/
	MFInt32 dartVertexIndex;	/*field*/
	SFBool solid;	/*field*/
	MFInt32 texCoordIndex;	/*field*/
} M_SubdivisionSurface;


typedef struct _tagSubdivSurfaceSector
{
	BASE_NODE
	SFFloat flatness;	/*exposedField*/
	SFVec3f normal;	/*exposedField*/
	SFFloat normalTension;	/*exposedField*/
	SFInt32 _tag;	/*exposedField*/
	SFFloat theta;	/*exposedField*/
	SFInt32 faceIndex;	/*field*/
	SFInt32 vertexIndex;	/*field*/
} M_SubdivSurfaceSector;


typedef struct _tagWaveletSubdivisionSurface
{
	BASE_NODE
	GF_Node *baseMesh;	/*exposedField*/
	SFFloat fieldOfView;	/*exposedField*/
	SFFloat frequency;	/*exposedField*/
	SFInt32 quality;	/*exposedField*/
} M_WaveletSubdivisionSurface;


typedef struct _tagClipper2D
{
	BASE_NODE
	VRML_CHILDREN
	GF_Node *geometry;	/*exposedField*/
	SFBool inside;	/*exposedField*/
	GF_Node *transform;	/*exposedField*/
	SFBool XOR;	/*exposedField*/
} M_Clipper2D;


typedef struct _tagColorTransform
{
	BASE_NODE
	VRML_CHILDREN
	SFFloat mrr;	/*exposedField*/
	SFFloat mrg;	/*exposedField*/
	SFFloat mrb;	/*exposedField*/
	SFFloat mra;	/*exposedField*/
	SFFloat tr;	/*exposedField*/
	SFFloat mgr;	/*exposedField*/
	SFFloat mgg;	/*exposedField*/
	SFFloat mgb;	/*exposedField*/
	SFFloat mga;	/*exposedField*/
	SFFloat tg;	/*exposedField*/
	SFFloat mbr;	/*exposedField*/
	SFFloat mbg;	/*exposedField*/
	SFFloat mbb;	/*exposedField*/
	SFFloat mba;	/*exposedField*/
	SFFloat tb;	/*exposedField*/
	SFFloat mar;	/*exposedField*/
	SFFloat mag;	/*exposedField*/
	SFFloat mab;	/*exposedField*/
	SFFloat maa;	/*exposedField*/
	SFFloat ta;	/*exposedField*/
} M_ColorTransform;


typedef struct _tagEllipse
{
	BASE_NODE
	SFVec2f radius;	/*exposedField*/
} M_Ellipse;


typedef struct _tagLinearGradient
{
	BASE_NODE
	SFVec2f endPoint;	/*exposedField*/
	MFFloat key;	/*exposedField*/
	MFColor keyValue;	/*exposedField*/
	MFFloat opacity;	/*exposedField*/
	SFInt32 spreadMethod;	/*exposedField*/
	SFVec2f startPoint;	/*exposedField*/
	GF_Node *transform;	/*exposedField*/
} M_LinearGradient;


typedef struct _tagPathLayout
{
	BASE_NODE
	VRML_CHILDREN
	GF_Node *geometry;	/*exposedField*/
	MFInt32 alignment;	/*exposedField*/
	SFFloat pathOffset;	/*exposedField*/
	SFFloat spacing;	/*exposedField*/
	SFBool reverseLayout;	/*exposedField*/
	SFInt32 wrapMode;	/*exposedField*/
	SFBool splitText;	/*exposedField*/
} M_PathLayout;


typedef struct _tagRadialGradient
{
	BASE_NODE
	SFVec2f center;	/*exposedField*/
	SFVec2f focalPoint;	/*exposedField*/
	MFFloat key;	/*exposedField*/
	MFColor keyValue;	/*exposedField*/
	MFFloat opacity;	/*exposedField*/
	SFFloat radius;	/*exposedField*/
	SFInt32 spreadMethod;	/*exposedField*/
	GF_Node *transform;	/*exposedField*/
} M_RadialGradient;


typedef struct _tagSynthesizedTexture
{
	BASE_NODE
	MFVec3f translation;	/*exposedField*/
	MFRotation rotation;	/*exposedField*/
	SFInt32 pixelWidth;	/*exposedField*/
	SFInt32 pixelHeight;	/*exposedField*/
	SFBool loop;	/*exposedField*/
	SFFloat speed;	/*exposedField*/
	SFTime startTime;	/*exposedField*/
	SFTime stopTime;	/*exposedField*/
	MFURL url;	/*exposedField*/
	SFTime duration_changed;	/*eventOut*/
	SFBool isActive;	/*eventOut*/
} M_SynthesizedTexture;


typedef struct _tagTransformMatrix2D
{
	BASE_NODE
	VRML_CHILDREN
	SFFloat mxx;	/*exposedField*/
	SFFloat mxy;	/*exposedField*/
	SFFloat tx;	/*exposedField*/
	SFFloat myx;	/*exposedField*/
	SFFloat myy;	/*exposedField*/
	SFFloat ty;	/*exposedField*/
} M_TransformMatrix2D;


typedef struct _tagViewport
{
	BASE_NODE
	SFBool set_bind;	/*eventIn*/
	void (*on_set_bind)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFVec2f position;	/*exposedField*/
	SFVec2f size;	/*exposedField*/
	SFFloat orientation;	/*exposedField*/
	MFInt32 alignment;	/*exposedField*/
	SFInt32 fit;	/*exposedField*/
	SFString description;	/*field*/
	SFTime bindTime;	/*eventOut*/
	SFBool isBound;	/*eventOut*/
} M_Viewport;


typedef struct _tagXCurve2D
{
	BASE_NODE
	GF_Node *point;	/*exposedField*/
	SFFloat fineness;	/*exposedField*/
	MFInt32 type;	/*exposedField*/
} M_XCurve2D;


typedef struct _tagXFontStyle
{
	BASE_NODE
	MFString fontName;	/*exposedField*/
	SFBool horizontal;	/*exposedField*/
	MFString justify;	/*exposedField*/
	SFString language;	/*exposedField*/
	SFBool leftToRight;	/*exposedField*/
	SFFloat size;	/*exposedField*/
	SFString stretch;	/*exposedField*/
	SFFloat letterSpacing;	/*exposedField*/
	SFFloat wordSpacing;	/*exposedField*/
	SFInt32 weight;	/*exposedField*/
	SFBool fontKerning;	/*exposedField*/
	SFString style;	/*exposedField*/
	SFBool topToBottom;	/*exposedField*/
	MFString featureName;	/*exposedField*/
	MFInt32 featureStartOffset;	/*exposedField*/
	MFInt32 featureLength;	/*exposedField*/
	MFInt32 featureValue;	/*exposedField*/
} M_XFontStyle;


typedef struct _tagXLineProperties
{
	BASE_NODE
	SFColor lineColor;	/*exposedField*/
	SFInt32 lineStyle;	/*exposedField*/
	SFBool isCenterAligned;	/*exposedField*/
	SFBool isScalable;	/*exposedField*/
	SFInt32 lineCap;	/*exposedField*/
	SFInt32 lineJoin;	/*exposedField*/
	SFFloat miterLimit;	/*exposedField*/
	SFFloat transparency;	/*exposedField*/
	SFFloat width;	/*exposedField*/
	SFFloat dashOffset;	/*exposedField*/
	MFFloat dashes;	/*exposedField*/
	GF_Node *texture;	/*exposedField*/
	GF_Node *textureTransform;	/*exposedField*/
} M_XLineProperties;


typedef struct _tagAdvancedAudioBuffer
{
	BASE_NODE
	VRML_CHILDREN
	SFBool loop;	/*exposedField*/
	SFFloat pitch;	/*exposedField*/
	SFTime startTime;	/*exposedField*/
	SFTime stopTime;	/*exposedField*/
	SFTime startLoadTime;	/*exposedField*/
	SFTime stopLoadTime;	/*exposedField*/
	SFInt32 loadMode;	/*exposedField*/
	SFInt32 numAccumulatedBlocks;	/*exposedField*/
	SFInt32 deleteBlock;	/*exposedField*/
	SFInt32 playBlock;	/*exposedField*/
	SFFloat length;	/*exposedField*/
	SFInt32 numChan;	/*field*/
	MFInt32 phaseGroup;	/*field*/
	SFTime duration_changed;	/*eventOut*/
	SFBool isActive;	/*eventOut*/
} M_AdvancedAudioBuffer;


typedef struct _tagAudioChannelConfig
{
	BASE_NODE
	VRML_CHILDREN
	SFInt32 generalChannelFormat;	/*exposedField*/
	SFInt32 fixedPreset;	/*exposedField*/
	SFInt32 fixedPresetSubset;	/*exposedField*/
	SFInt32 fixedPresetAddInf;	/*exposedField*/
	MFInt32 channelCoordinateSystems;	/*exposedField*/
	MFFloat channelSoundLocation;	/*exposedField*/
	MFInt32 channelDirectionalPattern;	/*exposedField*/
	MFVec3f channelDirection;	/*exposedField*/
	SFInt32 ambResolution2D;	/*exposedField*/
	SFInt32 ambResolution3D;	/*exposedField*/
	SFInt32 ambEncodingConvention;	/*exposedField*/
	SFFloat ambNfcReferenceDistance;	/*exposedField*/
	SFFloat ambSoundSpeed;	/*exposedField*/
	SFInt32 ambArrangementRule;	/*exposedField*/
	SFInt32 ambRecombinationPreset;	/*exposedField*/
	MFInt32 ambComponentIndex;	/*exposedField*/
	MFFloat ambBackwardMatrix;	/*exposedField*/
	MFInt32 ambSoundfieldResolution;	/*exposedField*/
	SFInt32 numChannel;	/*field*/
} M_AudioChannelConfig;


typedef struct _tagDepthImageV2
{
	BASE_NODE
	GF_Node *diTexture;	/*field*/
	SFFloat farPlane;	/*field*/
	SFVec2f fieldOfView;	/*field*/
	SFFloat nearPlane;	/*field*/
	SFRotation orientation;	/*field*/
	SFBool orthographic;	/*field*/
	SFVec3f position;	/*field*/
	SFVec2f splatMinMax;	/*field*/
} M_DepthImageV2;


typedef struct _tagMorphShape
{
	BASE_NODE
	GF_Node *baseShape;	/*exposedField*/
	SFInt32 morphID;	/*exposedField*/
	GF_ChildNodeItem *targetShapes;	/*exposedField*/
	MFFloat weights;	/*exposedField*/
} M_MorphShape;


typedef struct _tagMultiTexture
{
	BASE_NODE
	SFFloat alpha;	/*exposedField*/
	SFColor color;	/*exposedField*/
	MFInt32 function;	/*exposedField*/
	MFInt32 mode;	/*exposedField*/
	MFInt32 source;	/*exposedField*/
	GF_ChildNodeItem *texture;	/*exposedField*/
	MFVec3f cameraVector;	/*exposedField*/
	SFBool transparent;	/*exposedField*/
} M_MultiTexture;


typedef struct _tagPointTextureV2
{
	BASE_NODE
	MFColor color;	/*field*/
	MFInt32 depth;	/*field*/
	SFInt32 depthNbBits;	/*field*/
	SFInt32 height;	/*field*/
	GF_Node *normal;	/*field*/
	MFVec3f splatU;	/*field*/
	MFVec3f splatV;	/*field*/
	SFInt32 width;	/*field*/
} M_PointTextureV2;


typedef struct _tagSBVCAnimationV2
{
	BASE_NODE
	MFInt32 activeUrlIndex;	/*exposedField*/
	SFBool loop;	/*exposedField*/
	SFFloat speed;	/*exposedField*/
	SFTime startTime;	/*exposedField*/
	SFTime stopTime;	/*exposedField*/
	SFFloat transitionTime;	/*exposedField*/
	MFURL url;	/*exposedField*/
	GF_ChildNodeItem *virtualCharacters;	/*exposedField*/
	SFTime duration_changed;	/*eventOut*/
	SFBool isActive;	/*eventOut*/
} M_SBVCAnimationV2;


typedef struct _tagSimpleTextureV2
{
	BASE_NODE
	GF_Node *depth;	/*field*/
	GF_Node *normal;	/*field*/
	GF_Node *splatU;	/*field*/
	GF_Node *splatV;	/*field*/
	GF_Node *texture;	/*field*/
} M_SimpleTextureV2;


typedef struct _tagSurroundingSound
{
	BASE_NODE
	GF_Node *source;	/*exposedField*/
	SFFloat intensity;	/*exposedField*/
	SFFloat distance;	/*exposedField*/
	SFVec3f location;	/*exposedField*/
	SFFloat distortionFactor;	/*exposedField*/
	SFRotation orientation;	/*exposedField*/
	SFBool isTransformable;	/*exposedField*/
} M_SurroundingSound;


typedef struct _tagTransform3DAudio
{
	BASE_NODE
	VRML_CHILDREN
	SFFloat thirdCenterCoordinate;	/*exposedField*/
	SFVec3f rotationVector;	/*exposedField*/
	SFFloat thirdScaleCoordinate;	/*exposedField*/
	SFVec3f scaleOrientationVector;	/*exposedField*/
	SFFloat thirdTranslationCoordinate;	/*exposedField*/
	SFRotation coordinateTransform;	/*exposedField*/
} M_Transform3DAudio;


typedef struct _tagWideSound
{
	BASE_NODE
	GF_Node *source;	/*exposedField*/
	SFFloat intensity;	/*exposedField*/
	SFVec3f location;	/*exposedField*/
	SFBool spatialize;	/*exposedField*/
	GF_Node *perceptualParameters;	/*exposedField*/
	SFBool roomEffect;	/*exposedField*/
	SFInt32 shape;	/*exposedField*/
	MFFloat size;	/*exposedField*/
	SFVec3f direction;	/*exposedField*/
	SFFloat density;	/*exposedField*/
	SFInt32 diffuseSelect;	/*exposedField*/
	SFFloat decorrStrength;	/*exposedField*/
	SFFloat speedOfSound;	/*field*/
	SFFloat distance;	/*field*/
	SFBool useAirabs;	/*field*/
} M_WideSound;


typedef struct _tagScoreShape
{
	BASE_NODE
	GF_Node *score;	/*exposedField*/
	GF_Node *geometry;	/*exposedField*/
} M_ScoreShape;


typedef struct _tagMusicScore
{
	BASE_NODE
	SFBool executeCommand;	/*eventIn*/
	void (*on_executeCommand)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFString gotoLabel;	/*eventIn*/
	void (*on_gotoLabel)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFInt32 gotoMeasure;	/*eventIn*/
	void (*on_gotoMeasure)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFTime highlightTimePosition;	/*eventIn*/
	void (*on_highlightTimePosition)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFVec3f mousePosition;	/*eventIn*/
	void (*on_mousePosition)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	MFString argumentsOnExecute;	/*exposedField*/
	SFString commandOnExecute;	/*exposedField*/
	SFInt32 firstVisibleMeasure;	/*exposedField*/
	SFBool hyperlinkEnable;	/*exposedField*/
	SFBool loop;	/*exposedField*/
	MFString partsLyrics;	/*exposedField*/
	MFInt32 partsShown;	/*exposedField*/
	SFTime scoreOffset;	/*exposedField*/
	SFVec2f size;	/*exposedField*/
	SFFloat speed;	/*exposedField*/
	SFTime startTime;	/*exposedField*/
	SFTime stopTime;	/*exposedField*/
	SFFloat transpose;	/*exposedField*/
	MFURL url;	/*exposedField*/
	MFURL urlSA;	/*exposedField*/
	SFString viewType;	/*exposedField*/
	SFString activatedLink;	/*eventOut*/
	MFString availableCommands;	/*eventOut*/
	MFString availableLabels;	/*eventOut*/
	MFString availableLyricLanguages;	/*eventOut*/
	MFString availableViewTypes;	/*eventOut*/
	SFBool isActive;	/*eventOut*/
	SFVec3f highlightPosition;	/*eventOut*/
	SFInt32 lastVisibleMeasure;	/*eventOut*/
	SFInt32 numMeasures;	/*eventOut*/
	MFString partNames;	/*eventOut*/
} M_MusicScore;


typedef struct _tagFootPrintSetNode
{
	BASE_NODE
	VRML_CHILDREN
} M_FootPrintSetNode;


typedef struct _tagFootPrintNode
{
	BASE_NODE
	SFInt32 index;	/*exposedField*/
	GF_Node *footprint;	/*exposedField*/
} M_FootPrintNode;


typedef struct _tagBuildingPartNode
{
	BASE_NODE
	SFInt32 index;	/*exposedField*/
	GF_Node *footprint;	/*exposedField*/
	SFInt32 buildingIndex;	/*exposedField*/
	SFFloat height;	/*exposedField*/
	SFFloat altitude;	/*exposedField*/
	GF_ChildNodeItem *alternativeGeometry;	/*exposedField*/
	GF_ChildNodeItem *roofs;	/*exposedField*/
	GF_ChildNodeItem *facades;	/*exposedField*/
} M_BuildingPartNode;


typedef struct _tagRoofNode
{
	BASE_NODE
	SFInt32 Type;	/*exposedField*/
	SFFloat Height;	/*exposedField*/
	MFFloat SlopeAngle;	/*exposedField*/
	SFFloat EaveProjection;	/*exposedField*/
	SFInt32 EdgeSupportIndex;	/*exposedField*/
	SFURL RoofTextureURL;	/*exposedField*/
	SFBool IsGenericTexture;	/*exposedField*/
	SFFloat TextureXScale;	/*exposedField*/
	SFFloat TextureYScale;	/*exposedField*/
	SFFloat TextureXPosition;	/*exposedField*/
	SFFloat TextureYPosition;	/*exposedField*/
	SFFloat TextureRotation;	/*exposedField*/
} M_RoofNode;


typedef struct _tagFacadeNode
{
	BASE_NODE
	SFFloat WidthRatio;	/*exposedField*/
	SFFloat XScale;	/*exposedField*/
	SFFloat YScale;	/*exposedField*/
	SFFloat XPosition;	/*exposedField*/
	SFFloat YPosition;	/*exposedField*/
	SFFloat XRepeatInterval;	/*exposedField*/
	SFFloat YRepeatInterval;	/*exposedField*/
	SFBool Repeat;	/*exposedField*/
	SFURL FacadePrimitive;	/*exposedField*/
	SFInt32 NbStories;	/*exposedField*/
	MFInt32 NbFacadeCellsByStorey;	/*exposedField*/
	MFFloat StoreyHeight;	/*exposedField*/
	GF_ChildNodeItem *FacadeCellsArray;	/*exposedField*/
} M_FacadeNode;


typedef struct _tagShadow
{
	BASE_NODE
	VRML_CHILDREN
	SFBool enabled;	/*exposedField*/
	SFBool cast;	/*exposedField*/
	SFBool receive;	/*exposedField*/
	SFFloat penumbra;	/*exposedField*/
} M_Shadow;


typedef struct _tagCacheTexture
{
	BASE_NODE
	SFInt32 objectTypeIndication;	/*field*/
	SFString decoderSpecificInfo;	/*field*/
	SFString image;	/*field*/
	SFString cacheURL;	/*field*/
	MFURL cacheOD;	/*field*/
	SFInt32 expirationDate;	/*field*/
	SFBool repeatS;	/*field*/
	SFBool repeatT;	/*field*/
} M_CacheTexture;


typedef struct _tagEnvironmentTest
{
	BASE_NODE
	SFBool evaluate;	/*eventIn*/
	void (*on_evaluate)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFBool enabled;	/*exposedField*/
	SFInt32 parameter;	/*exposedField*/
	SFString compareValue;	/*exposedField*/
	SFBool evaluateOnChange;	/*exposedField*/
	SFBool valueLarger;	/*eventOut*/
	SFBool valueEqual;	/*eventOut*/
	SFBool valueSmaller;	/*eventOut*/
	SFString parameterValue;	/*eventOut*/
} M_EnvironmentTest;


typedef struct _tagKeyNavigator
{
	BASE_NODE
	SFBool setFocus;	/*eventIn*/
	void (*on_setFocus)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	GF_Node *sensor;	/*exposedField*/
	GF_Node *left;	/*exposedField*/
	GF_Node *right;	/*exposedField*/
	GF_Node *up;	/*exposedField*/
	GF_Node *down;	/*exposedField*/
	GF_Node *select;	/*exposedField*/
	GF_Node *quit;	/*exposedField*/
	SFFloat step;	/*exposedField*/
	SFBool focusSet;	/*eventOut*/
} M_KeyNavigator;


typedef struct _tagSpacePartition
{
	BASE_NODE
	VRML_CHILDREN
	SFURL SPStream;	/*exposedField*/
} M_SpacePartition;


typedef struct _tagStorage
{
	BASE_NODE
	SFBool forceSave;	/*eventIn*/
	void (*on_forceSave)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFBool forceRestore;	/*eventIn*/
	void (*on_forceRestore)(GF_Node *pThis, struct _route *route);	/*eventInHandler*/
	SFBool _auto;	/*exposedField*/
	SFInt32 expireAfter;	/*field*/
	SFString name;	/*field*/
	MFAttrRef storageList;	/*field*/
} M_Storage;


/*NodeDataType tags*/
enum {
	NDT_SFWorldNode = 1,
	NDT_SF3DNode,
	NDT_SF2DNode,
	NDT_SFStreamingNode,
	NDT_SFAppearanceNode,
	NDT_SFAudioNode,
	NDT_SFBackground3DNode,
	NDT_SFBackground2DNode,
	NDT_SFGeometryNode,
	NDT_SFColorNode,
	NDT_SFTextureNode,
	NDT_SFCoordinateNode,
	NDT_SFCoordinate2DNode,
	NDT_SFExpressionNode,
	NDT_SFFaceDefMeshNode,
	NDT_SFFaceDefTablesNode,
	NDT_SFFaceDefTransformNode,
	NDT_SFFAPNode,
	NDT_SFFDPNode,
	NDT_SFFITNode,
	NDT_SFFogNode,
	NDT_SFFontStyleNode,
	NDT_SFTopNode,
	NDT_SFLinePropertiesNode,
	NDT_SFMaterialNode,
	NDT_SFNavigationInfoNode,
	NDT_SFNormalNode,
	NDT_SFTextureCoordinateNode,
	NDT_SFTextureTransformNode,
	NDT_SFViewpointNode,
	NDT_SFVisemeNode,
	NDT_SFViewportNode,
	NDT_SFBAPNode,
	NDT_SFBDPNode,
	NDT_SFBodyDefTableNode,
	NDT_SFBodySegmentConnectionHintNode,
	NDT_SFPerceptualParameterNode,
	NDT_SFTemporalNode,
	NDT_SFDepthImageNode,
	NDT_SFBlendListNode,
	NDT_SFFrameListNode,
	NDT_SFLightMapNode,
	NDT_SFSurfaceMapNode,
	NDT_SFViewMapNode,
	NDT_SFParticleInitializerNode,
	NDT_SFInfluenceNode,
	NDT_SFDepthTextureNode,
	NDT_SFSBBoneNode,
	NDT_SFSBMuscleNode,
	NDT_SFSBSegmentNode,
	NDT_SFSBSiteNode,
	NDT_SFBaseMeshNode,
	NDT_SFSubdivSurfaceSectorNode,
	NDT_SFMusicScoreNode
};

/*All BIFS versions handled*/
#define GF_BIFS_NUM_VERSION		10

enum {
	GF_BIFS_V1 = 1,
	GF_BIFS_V2,
	GF_BIFS_V3,
	GF_BIFS_V4,
	GF_BIFS_V5,
	GF_BIFS_V6,
	GF_BIFS_V7,
	GF_BIFS_V8,
	GF_BIFS_V9,
	GF_BIFS_V10,
	GF_BIFS_LAST_VERSION = GF_BIFS_V10
};



#endif /*GPAC_DISABLE_VRML*/

#ifdef __cplusplus
}
#endif



#endif		/*_nodes_mpeg4_H*/

