/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005
 *					All rights reserved
 *
 *  This file is part of GPAC / BIFS codec sub-project
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

#ifndef _NDT_H
#define _NDT_H

#include <gpac/nodes_mpeg4.h>



#ifndef GPAC_DISABLE_BIFS


u32 ALL_GetNodeType(const u32 *table, const u32 count, u32 NodeTag, u32 Version);



/* NDT BIFS Version 1 */

#define SFWorldNode_V1_NUMBITS		7
#define SFWorldNode_V1_Count	100

static const u32 SFWorldNode_V1_TypeToTag[100] = {
 TAG_MPEG4_Anchor, TAG_MPEG4_AnimationStream, TAG_MPEG4_Appearance, TAG_MPEG4_AudioBuffer, TAG_MPEG4_AudioClip, TAG_MPEG4_AudioDelay, TAG_MPEG4_AudioFX, TAG_MPEG4_AudioMix, TAG_MPEG4_AudioSource, TAG_MPEG4_AudioSwitch, TAG_MPEG4_Background, TAG_MPEG4_Background2D, TAG_MPEG4_Billboard, TAG_MPEG4_Bitmap, TAG_MPEG4_Box, TAG_MPEG4_Circle, TAG_MPEG4_Collision, TAG_MPEG4_Color, TAG_MPEG4_ColorInterpolator, TAG_MPEG4_CompositeTexture2D, TAG_MPEG4_CompositeTexture3D, TAG_MPEG4_Conditional, TAG_MPEG4_Cone, TAG_MPEG4_Coordinate, TAG_MPEG4_Coordinate2D, TAG_MPEG4_CoordinateInterpolator, TAG_MPEG4_CoordinateInterpolator2D, TAG_MPEG4_Curve2D, TAG_MPEG4_Cylinder, TAG_MPEG4_CylinderSensor, TAG_MPEG4_DirectionalLight, TAG_MPEG4_DiscSensor, TAG_MPEG4_ElevationGrid, TAG_MPEG4_Expression, TAG_MPEG4_Extrusion, TAG_MPEG4_Face, TAG_MPEG4_FaceDefMesh, TAG_MPEG4_FaceDefTables, TAG_MPEG4_FaceDefTransform, TAG_MPEG4_FAP, TAG_MPEG4_FDP, TAG_MPEG4_FIT, TAG_MPEG4_Fog, TAG_MPEG4_FontStyle, TAG_MPEG4_Form, TAG_MPEG4_Group, TAG_MPEG4_ImageTexture, TAG_MPEG4_IndexedFaceSet, TAG_MPEG4_IndexedFaceSet2D, TAG_MPEG4_IndexedLineSet, TAG_MPEG4_IndexedLineSet2D, TAG_MPEG4_Inline, TAG_MPEG4_LOD, TAG_MPEG4_Layer2D, TAG_MPEG4_Layer3D, TAG_MPEG4_Layout, TAG_MPEG4_LineProperties, TAG_MPEG4_ListeningPoint, TAG_MPEG4_Material, TAG_MPEG4_Material2D, TAG_MPEG4_MovieTexture, TAG_MPEG4_NavigationInfo, TAG_MPEG4_Normal, TAG_MPEG4_NormalInterpolator, TAG_MPEG4_OrderedGroup, TAG_MPEG4_OrientationInterpolator, TAG_MPEG4_PixelTexture, TAG_MPEG4_PlaneSensor, TAG_MPEG4_PlaneSensor2D, TAG_MPEG4_PointLight, TAG_MPEG4_PointSet, TAG_MPEG4_PointSet2D, TAG_MPEG4_PositionInterpolator, TAG_MPEG4_PositionInterpolator2D, TAG_MPEG4_ProximitySensor2D, TAG_MPEG4_ProximitySensor, TAG_MPEG4_QuantizationParameter, TAG_MPEG4_Rectangle, TAG_MPEG4_ScalarInterpolator, TAG_MPEG4_Script, TAG_MPEG4_Shape, TAG_MPEG4_Sound, TAG_MPEG4_Sound2D, TAG_MPEG4_Sphere, TAG_MPEG4_SphereSensor, TAG_MPEG4_SpotLight, TAG_MPEG4_Switch, TAG_MPEG4_TermCap, TAG_MPEG4_Text, TAG_MPEG4_TextureCoordinate, TAG_MPEG4_TextureTransform, TAG_MPEG4_TimeSensor, TAG_MPEG4_TouchSensor, TAG_MPEG4_Transform, TAG_MPEG4_Transform2D, TAG_MPEG4_Valuator, TAG_MPEG4_Viewpoint, TAG_MPEG4_VisibilitySensor, TAG_MPEG4_Viseme, TAG_MPEG4_WorldInfo
};

#define SF3DNode_V1_NUMBITS		6
#define SF3DNode_V1_Count	52

static const u32 SF3DNode_V1_TypeToTag[52] = {
 TAG_MPEG4_Anchor, TAG_MPEG4_AnimationStream, TAG_MPEG4_Background, TAG_MPEG4_Background2D, TAG_MPEG4_Billboard, TAG_MPEG4_Collision, TAG_MPEG4_ColorInterpolator, TAG_MPEG4_Conditional, TAG_MPEG4_CoordinateInterpolator, TAG_MPEG4_CoordinateInterpolator2D, TAG_MPEG4_CylinderSensor, TAG_MPEG4_DirectionalLight, TAG_MPEG4_DiscSensor, TAG_MPEG4_Face, TAG_MPEG4_Fog, TAG_MPEG4_Form, TAG_MPEG4_Group, TAG_MPEG4_Inline, TAG_MPEG4_LOD, TAG_MPEG4_Layer2D, TAG_MPEG4_Layer3D, TAG_MPEG4_Layout, TAG_MPEG4_ListeningPoint, TAG_MPEG4_NavigationInfo, TAG_MPEG4_NormalInterpolator, TAG_MPEG4_OrderedGroup, TAG_MPEG4_OrientationInterpolator, TAG_MPEG4_PlaneSensor, TAG_MPEG4_PlaneSensor2D, TAG_MPEG4_PointLight, TAG_MPEG4_PositionInterpolator, TAG_MPEG4_PositionInterpolator2D, TAG_MPEG4_ProximitySensor2D, TAG_MPEG4_ProximitySensor, TAG_MPEG4_QuantizationParameter, TAG_MPEG4_ScalarInterpolator, TAG_MPEG4_Script, TAG_MPEG4_Shape, TAG_MPEG4_Sound, TAG_MPEG4_Sound2D, TAG_MPEG4_SphereSensor, TAG_MPEG4_SpotLight, TAG_MPEG4_Switch, TAG_MPEG4_TermCap, TAG_MPEG4_TimeSensor, TAG_MPEG4_TouchSensor, TAG_MPEG4_Transform, TAG_MPEG4_Transform2D, TAG_MPEG4_Valuator, TAG_MPEG4_Viewpoint, TAG_MPEG4_VisibilitySensor, TAG_MPEG4_WorldInfo
};

#define SF2DNode_V1_NUMBITS		5
#define SF2DNode_V1_Count	31

static const u32 SF2DNode_V1_TypeToTag[31] = {
 TAG_MPEG4_Anchor, TAG_MPEG4_AnimationStream, TAG_MPEG4_Background2D, TAG_MPEG4_ColorInterpolator, TAG_MPEG4_Conditional, TAG_MPEG4_CoordinateInterpolator2D, TAG_MPEG4_DiscSensor, TAG_MPEG4_Face, TAG_MPEG4_Form, TAG_MPEG4_Group, TAG_MPEG4_Inline, TAG_MPEG4_LOD, TAG_MPEG4_Layer2D, TAG_MPEG4_Layer3D, TAG_MPEG4_Layout, TAG_MPEG4_OrderedGroup, TAG_MPEG4_PlaneSensor2D, TAG_MPEG4_PositionInterpolator2D, TAG_MPEG4_ProximitySensor2D, TAG_MPEG4_QuantizationParameter, TAG_MPEG4_ScalarInterpolator, TAG_MPEG4_Script, TAG_MPEG4_Shape, TAG_MPEG4_Sound2D, TAG_MPEG4_Switch, TAG_MPEG4_TermCap, TAG_MPEG4_TimeSensor, TAG_MPEG4_TouchSensor, TAG_MPEG4_Transform2D, TAG_MPEG4_Valuator, TAG_MPEG4_WorldInfo
};

#define SFStreamingNode_V1_NUMBITS		3
#define SFStreamingNode_V1_Count	5

static const u32 SFStreamingNode_V1_TypeToTag[5] = {
 TAG_MPEG4_AnimationStream, TAG_MPEG4_AudioClip, TAG_MPEG4_AudioSource, TAG_MPEG4_Inline, TAG_MPEG4_MovieTexture
};

#define SFAppearanceNode_V1_NUMBITS		1
#define SFAppearanceNode_V1_Count	1

static const u32 SFAppearanceNode_V1_TypeToTag[1] = {
 TAG_MPEG4_Appearance
};

#define SFAudioNode_V1_NUMBITS		3
#define SFAudioNode_V1_Count	7

static const u32 SFAudioNode_V1_TypeToTag[7] = {
 TAG_MPEG4_AudioBuffer, TAG_MPEG4_AudioClip, TAG_MPEG4_AudioDelay, TAG_MPEG4_AudioFX, TAG_MPEG4_AudioMix, TAG_MPEG4_AudioSource, TAG_MPEG4_AudioSwitch
};

#define SFBackground3DNode_V1_NUMBITS		1
#define SFBackground3DNode_V1_Count	1

static const u32 SFBackground3DNode_V1_TypeToTag[1] = {
 TAG_MPEG4_Background
};

#define SFBackground2DNode_V1_NUMBITS		1
#define SFBackground2DNode_V1_Count	1

static const u32 SFBackground2DNode_V1_TypeToTag[1] = {
 TAG_MPEG4_Background2D
};

#define SFGeometryNode_V1_NUMBITS		5
#define SFGeometryNode_V1_Count	17

static const u32 SFGeometryNode_V1_TypeToTag[17] = {
 TAG_MPEG4_Bitmap, TAG_MPEG4_Box, TAG_MPEG4_Circle, TAG_MPEG4_Cone, TAG_MPEG4_Curve2D, TAG_MPEG4_Cylinder, TAG_MPEG4_ElevationGrid, TAG_MPEG4_Extrusion, TAG_MPEG4_IndexedFaceSet, TAG_MPEG4_IndexedFaceSet2D, TAG_MPEG4_IndexedLineSet, TAG_MPEG4_IndexedLineSet2D, TAG_MPEG4_PointSet, TAG_MPEG4_PointSet2D, TAG_MPEG4_Rectangle, TAG_MPEG4_Sphere, TAG_MPEG4_Text
};

#define SFColorNode_V1_NUMBITS		1
#define SFColorNode_V1_Count	1

static const u32 SFColorNode_V1_TypeToTag[1] = {
 TAG_MPEG4_Color
};

#define SFTextureNode_V1_NUMBITS		3
#define SFTextureNode_V1_Count	5

static const u32 SFTextureNode_V1_TypeToTag[5] = {
 TAG_MPEG4_CompositeTexture2D, TAG_MPEG4_CompositeTexture3D, TAG_MPEG4_ImageTexture, TAG_MPEG4_MovieTexture, TAG_MPEG4_PixelTexture
};

#define SFCoordinateNode_V1_NUMBITS		1
#define SFCoordinateNode_V1_Count	1

static const u32 SFCoordinateNode_V1_TypeToTag[1] = {
 TAG_MPEG4_Coordinate
};

#define SFCoordinate2DNode_V1_NUMBITS		1
#define SFCoordinate2DNode_V1_Count	1

static const u32 SFCoordinate2DNode_V1_TypeToTag[1] = {
 TAG_MPEG4_Coordinate2D
};

#define SFExpressionNode_V1_NUMBITS		1
#define SFExpressionNode_V1_Count	1

static const u32 SFExpressionNode_V1_TypeToTag[1] = {
 TAG_MPEG4_Expression
};

#define SFFaceDefMeshNode_V1_NUMBITS		1
#define SFFaceDefMeshNode_V1_Count	1

static const u32 SFFaceDefMeshNode_V1_TypeToTag[1] = {
 TAG_MPEG4_FaceDefMesh
};

#define SFFaceDefTablesNode_V1_NUMBITS		1
#define SFFaceDefTablesNode_V1_Count	1

static const u32 SFFaceDefTablesNode_V1_TypeToTag[1] = {
 TAG_MPEG4_FaceDefTables
};

#define SFFaceDefTransformNode_V1_NUMBITS		1
#define SFFaceDefTransformNode_V1_Count	1

static const u32 SFFaceDefTransformNode_V1_TypeToTag[1] = {
 TAG_MPEG4_FaceDefTransform
};

#define SFFAPNode_V1_NUMBITS		1
#define SFFAPNode_V1_Count	1

static const u32 SFFAPNode_V1_TypeToTag[1] = {
 TAG_MPEG4_FAP
};

#define SFFDPNode_V1_NUMBITS		1
#define SFFDPNode_V1_Count	1

static const u32 SFFDPNode_V1_TypeToTag[1] = {
 TAG_MPEG4_FDP
};

#define SFFITNode_V1_NUMBITS		1
#define SFFITNode_V1_Count	1

static const u32 SFFITNode_V1_TypeToTag[1] = {
 TAG_MPEG4_FIT
};

#define SFFogNode_V1_NUMBITS		1
#define SFFogNode_V1_Count	1

static const u32 SFFogNode_V1_TypeToTag[1] = {
 TAG_MPEG4_Fog
};

#define SFFontStyleNode_V1_NUMBITS		1
#define SFFontStyleNode_V1_Count	1

static const u32 SFFontStyleNode_V1_TypeToTag[1] = {
 TAG_MPEG4_FontStyle
};

#define SFTopNode_V1_NUMBITS		3
#define SFTopNode_V1_Count	4

static const u32 SFTopNode_V1_TypeToTag[4] = {
 TAG_MPEG4_Group, TAG_MPEG4_Layer2D, TAG_MPEG4_Layer3D, TAG_MPEG4_OrderedGroup
};

#define SFLinePropertiesNode_V1_NUMBITS		1
#define SFLinePropertiesNode_V1_Count	1

static const u32 SFLinePropertiesNode_V1_TypeToTag[1] = {
 TAG_MPEG4_LineProperties
};

#define SFMaterialNode_V1_NUMBITS		2
#define SFMaterialNode_V1_Count	2

static const u32 SFMaterialNode_V1_TypeToTag[2] = {
 TAG_MPEG4_Material, TAG_MPEG4_Material2D
};

#define SFNavigationInfoNode_V1_NUMBITS		1
#define SFNavigationInfoNode_V1_Count	1

static const u32 SFNavigationInfoNode_V1_TypeToTag[1] = {
 TAG_MPEG4_NavigationInfo
};

#define SFNormalNode_V1_NUMBITS		1
#define SFNormalNode_V1_Count	1

static const u32 SFNormalNode_V1_TypeToTag[1] = {
 TAG_MPEG4_Normal
};

#define SFTextureCoordinateNode_V1_NUMBITS		1
#define SFTextureCoordinateNode_V1_Count	1

static const u32 SFTextureCoordinateNode_V1_TypeToTag[1] = {
 TAG_MPEG4_TextureCoordinate
};

#define SFTextureTransformNode_V1_NUMBITS		1
#define SFTextureTransformNode_V1_Count	1

static const u32 SFTextureTransformNode_V1_TypeToTag[1] = {
 TAG_MPEG4_TextureTransform
};

#define SFViewpointNode_V1_NUMBITS		1
#define SFViewpointNode_V1_Count	1

static const u32 SFViewpointNode_V1_TypeToTag[1] = {
 TAG_MPEG4_Viewpoint
};

#define SFVisemeNode_V1_NUMBITS		1
#define SFVisemeNode_V1_Count	1

static const u32 SFVisemeNode_V1_TypeToTag[1] = {
 TAG_MPEG4_Viseme
};


u32 NDT_V1_GetNumBits(u32 NDT_Tag);
u32 NDT_V1_GetNodeTag(u32 Context_NDT_Tag, u32 NodeType);
u32 NDT_V1_GetNodeType(u32 NDT_Tag, u32 NodeTag);




/* NDT BIFS Version 2 */

#define SFWorldNode_V2_NUMBITS		4
#define SFWorldNode_V2_Count	12

static const u32 SFWorldNode_V2_TypeToTag[12] = {
 TAG_MPEG4_AcousticMaterial, TAG_MPEG4_AcousticScene, TAG_MPEG4_ApplicationWindow, TAG_MPEG4_BAP, TAG_MPEG4_BDP, TAG_MPEG4_Body, TAG_MPEG4_BodyDefTable, TAG_MPEG4_BodySegmentConnectionHint, TAG_MPEG4_DirectiveSound, TAG_MPEG4_Hierarchical3DMesh, TAG_MPEG4_MaterialKey, TAG_MPEG4_PerceptualParameters
};

#define SF3DNode_V2_NUMBITS		3
#define SF3DNode_V2_Count	3

static const u32 SF3DNode_V2_TypeToTag[3] = {
 TAG_MPEG4_AcousticScene, TAG_MPEG4_Body, TAG_MPEG4_DirectiveSound
};

#define SF2DNode_V2_NUMBITS		2
#define SF2DNode_V2_Count	2

static const u32 SF2DNode_V2_TypeToTag[2] = {
 TAG_MPEG4_ApplicationWindow, TAG_MPEG4_Body
};

#define SFGeometryNode_V2_NUMBITS		2
#define SFGeometryNode_V2_Count	1

static const u32 SFGeometryNode_V2_TypeToTag[1] = {
 TAG_MPEG4_Hierarchical3DMesh
};

#define SFMaterialNode_V2_NUMBITS		2
#define SFMaterialNode_V2_Count	2

static const u32 SFMaterialNode_V2_TypeToTag[2] = {
 TAG_MPEG4_AcousticMaterial, TAG_MPEG4_MaterialKey
};

#define SFBAPNode_V2_NUMBITS		2
#define SFBAPNode_V2_Count	1

static const u32 SFBAPNode_V2_TypeToTag[1] = {
 TAG_MPEG4_BAP
};

#define SFBDPNode_V2_NUMBITS		2
#define SFBDPNode_V2_Count	1

static const u32 SFBDPNode_V2_TypeToTag[1] = {
 TAG_MPEG4_BDP
};

#define SFBodyDefTableNode_V2_NUMBITS		2
#define SFBodyDefTableNode_V2_Count	1

static const u32 SFBodyDefTableNode_V2_TypeToTag[1] = {
 TAG_MPEG4_BodyDefTable
};

#define SFBodySegmentConnectionHintNode_V2_NUMBITS		2
#define SFBodySegmentConnectionHintNode_V2_Count	1

static const u32 SFBodySegmentConnectionHintNode_V2_TypeToTag[1] = {
 TAG_MPEG4_BodySegmentConnectionHint
};

#define SFPerceptualParameterNode_V2_NUMBITS		2
#define SFPerceptualParameterNode_V2_Count	1

static const u32 SFPerceptualParameterNode_V2_TypeToTag[1] = {
 TAG_MPEG4_PerceptualParameters
};


u32 NDT_V2_GetNumBits(u32 NDT_Tag);
u32 NDT_V2_GetNodeTag(u32 Context_NDT_Tag, u32 NodeType);
u32 NDT_V2_GetNodeType(u32 NDT_Tag, u32 NodeTag);




/* NDT BIFS Version 3 */

#define SFWorldNode_V3_NUMBITS		2
#define SFWorldNode_V3_Count	3

static const u32 SFWorldNode_V3_TypeToTag[3] = {
 TAG_MPEG4_TemporalTransform, TAG_MPEG4_TemporalGroup, TAG_MPEG4_ServerCommand
};

#define SF3DNode_V3_NUMBITS		2
#define SF3DNode_V3_Count	3

static const u32 SF3DNode_V3_TypeToTag[3] = {
 TAG_MPEG4_TemporalTransform, TAG_MPEG4_TemporalGroup, TAG_MPEG4_ServerCommand
};

#define SF2DNode_V3_NUMBITS		2
#define SF2DNode_V3_Count	3

static const u32 SF2DNode_V3_TypeToTag[3] = {
 TAG_MPEG4_TemporalTransform, TAG_MPEG4_TemporalGroup, TAG_MPEG4_ServerCommand
};

#define SFTemporalNode_V3_NUMBITS		2
#define SFTemporalNode_V3_Count	2

static const u32 SFTemporalNode_V3_TypeToTag[2] = {
 TAG_MPEG4_TemporalTransform, TAG_MPEG4_TemporalGroup
};


u32 NDT_V3_GetNumBits(u32 NDT_Tag);
u32 NDT_V3_GetNodeTag(u32 Context_NDT_Tag, u32 NodeType);
u32 NDT_V3_GetNodeType(u32 NDT_Tag, u32 NodeTag);




/* NDT BIFS Version 4 */

#define SFWorldNode_V4_NUMBITS		3
#define SFWorldNode_V4_Count	5

static const u32 SFWorldNode_V4_TypeToTag[5] = {
 TAG_MPEG4_InputSensor, TAG_MPEG4_MatteTexture, TAG_MPEG4_MediaBuffer, TAG_MPEG4_MediaControl, TAG_MPEG4_MediaSensor
};

#define SF3DNode_V4_NUMBITS		3
#define SF3DNode_V4_Count	5

static const u32 SF3DNode_V4_TypeToTag[5] = {
 TAG_MPEG4_InputSensor, TAG_MPEG4_MatteTexture, TAG_MPEG4_MediaBuffer, TAG_MPEG4_MediaControl, TAG_MPEG4_MediaSensor
};

#define SF2DNode_V4_NUMBITS		3
#define SF2DNode_V4_Count	5

static const u32 SF2DNode_V4_TypeToTag[5] = {
 TAG_MPEG4_InputSensor, TAG_MPEG4_MatteTexture, TAG_MPEG4_MediaBuffer, TAG_MPEG4_MediaControl, TAG_MPEG4_MediaSensor
};

#define SFTextureNode_V4_NUMBITS		1
#define SFTextureNode_V4_Count	1

static const u32 SFTextureNode_V4_TypeToTag[1] = {
 TAG_MPEG4_MatteTexture
};


u32 NDT_V4_GetNumBits(u32 NDT_Tag);
u32 NDT_V4_GetNodeTag(u32 Context_NDT_Tag, u32 NodeType);
u32 NDT_V4_GetNodeType(u32 NDT_Tag, u32 NodeTag);




/* NDT BIFS Version 5 */

#define SFWorldNode_V5_NUMBITS		6
#define SFWorldNode_V5_Count	39

static const u32 SFWorldNode_V5_TypeToTag[39] = {
 TAG_MPEG4_BitWrapper, TAG_MPEG4_CoordinateInterpolator4D, TAG_MPEG4_DepthImage, TAG_MPEG4_FFD, TAG_MPEG4_Implicit, TAG_MPEG4_XXLFM_Appearance, TAG_MPEG4_XXLFM_BlendList, TAG_MPEG4_XXLFM_FrameList, TAG_MPEG4_XXLFM_LightMap, TAG_MPEG4_XXLFM_SurfaceMapList, TAG_MPEG4_XXLFM_ViewMapList, TAG_MPEG4_MeshGrid, TAG_MPEG4_NonLinearDeformer, TAG_MPEG4_NurbsCurve, TAG_MPEG4_NurbsCurve2D, TAG_MPEG4_NurbsSurface, TAG_MPEG4_OctreeImage, TAG_MPEG4_XXParticles, TAG_MPEG4_XXParticleInitBox, TAG_MPEG4_XXPlanarObstacle, TAG_MPEG4_XXPointAttractor, TAG_MPEG4_PointTexture, TAG_MPEG4_PositionAnimator, TAG_MPEG4_PositionAnimator2D, TAG_MPEG4_PositionInterpolator4D, TAG_MPEG4_ProceduralTexture, TAG_MPEG4_Quadric, TAG_MPEG4_SBBone, TAG_MPEG4_SBMuscle, TAG_MPEG4_SBSegment, TAG_MPEG4_SBSite, TAG_MPEG4_SBSkinnedModel, TAG_MPEG4_SBVCAnimation, TAG_MPEG4_ScalarAnimator, TAG_MPEG4_SimpleTexture, TAG_MPEG4_SolidRep, TAG_MPEG4_SubdivisionSurface, TAG_MPEG4_SubdivSurfaceSector, TAG_MPEG4_WaveletSubdivisionSurface
};

#define SF3DNode_V5_NUMBITS		5
#define SF3DNode_V5_Count	17

static const u32 SF3DNode_V5_TypeToTag[17] = {
 TAG_MPEG4_BitWrapper, TAG_MPEG4_CoordinateInterpolator4D, TAG_MPEG4_DepthImage, TAG_MPEG4_FFD, TAG_MPEG4_OctreeImage, TAG_MPEG4_XXParticles, TAG_MPEG4_PositionAnimator, TAG_MPEG4_PositionAnimator2D, TAG_MPEG4_PositionInterpolator4D, TAG_MPEG4_SBBone, TAG_MPEG4_SBMuscle, TAG_MPEG4_SBSegment, TAG_MPEG4_SBSite, TAG_MPEG4_SBSkinnedModel, TAG_MPEG4_SBVCAnimation, TAG_MPEG4_ScalarAnimator, TAG_MPEG4_WaveletSubdivisionSurface
};

#define SF2DNode_V5_NUMBITS		4
#define SF2DNode_V5_Count	9

static const u32 SF2DNode_V5_TypeToTag[9] = {
 TAG_MPEG4_BitWrapper, TAG_MPEG4_PositionAnimator2D, TAG_MPEG4_SBBone, TAG_MPEG4_SBMuscle, TAG_MPEG4_SBSegment, TAG_MPEG4_SBSite, TAG_MPEG4_SBSkinnedModel, TAG_MPEG4_SBVCAnimation, TAG_MPEG4_ScalarAnimator
};

#define SFAppearanceNode_V5_NUMBITS		1
#define SFAppearanceNode_V5_Count	1

static const u32 SFAppearanceNode_V5_TypeToTag[1] = {
 TAG_MPEG4_XXLFM_Appearance
};

#define SFGeometryNode_V5_NUMBITS		4
#define SFGeometryNode_V5_Count	10

static const u32 SFGeometryNode_V5_TypeToTag[10] = {
 TAG_MPEG4_BitWrapper, TAG_MPEG4_Implicit, TAG_MPEG4_MeshGrid, TAG_MPEG4_NonLinearDeformer, TAG_MPEG4_NurbsCurve, TAG_MPEG4_NurbsCurve2D, TAG_MPEG4_NurbsSurface, TAG_MPEG4_Quadric, TAG_MPEG4_SolidRep, TAG_MPEG4_SubdivisionSurface
};

#define SFTextureNode_V5_NUMBITS		1
#define SFTextureNode_V5_Count	1

static const u32 SFTextureNode_V5_TypeToTag[1] = {
 TAG_MPEG4_ProceduralTexture
};

#define SFDepthImageNode_V5_NUMBITS		1
#define SFDepthImageNode_V5_Count	1

static const u32 SFDepthImageNode_V5_TypeToTag[1] = {
 TAG_MPEG4_DepthImage
};

#define SFBlendListNode_V5_NUMBITS		1
#define SFBlendListNode_V5_Count	1

static const u32 SFBlendListNode_V5_TypeToTag[1] = {
 TAG_MPEG4_XXLFM_BlendList
};

#define SFFrameListNode_V5_NUMBITS		1
#define SFFrameListNode_V5_Count	1

static const u32 SFFrameListNode_V5_TypeToTag[1] = {
 TAG_MPEG4_XXLFM_FrameList
};

#define SFLightMapNode_V5_NUMBITS		1
#define SFLightMapNode_V5_Count	1

static const u32 SFLightMapNode_V5_TypeToTag[1] = {
 TAG_MPEG4_XXLFM_LightMap
};

#define SFSurfaceMapNode_V5_NUMBITS		1
#define SFSurfaceMapNode_V5_Count	1

static const u32 SFSurfaceMapNode_V5_TypeToTag[1] = {
 TAG_MPEG4_XXLFM_SurfaceMapList
};

#define SFViewMapNode_V5_NUMBITS		1
#define SFViewMapNode_V5_Count	1

static const u32 SFViewMapNode_V5_TypeToTag[1] = {
 TAG_MPEG4_XXLFM_ViewMapList
};

#define SFParticleInitializerNode_V5_NUMBITS		1
#define SFParticleInitializerNode_V5_Count	1

static const u32 SFParticleInitializerNode_V5_TypeToTag[1] = {
 TAG_MPEG4_XXParticleInitBox
};

#define SFInfluenceNode_V5_NUMBITS		2
#define SFInfluenceNode_V5_Count	2

static const u32 SFInfluenceNode_V5_TypeToTag[2] = {
 TAG_MPEG4_XXPlanarObstacle, TAG_MPEG4_XXPointAttractor
};

#define SFDepthTextureNode_V5_NUMBITS		2
#define SFDepthTextureNode_V5_Count	2

static const u32 SFDepthTextureNode_V5_TypeToTag[2] = {
 TAG_MPEG4_PointTexture, TAG_MPEG4_SimpleTexture
};

#define SFSBBoneNode_V5_NUMBITS		1
#define SFSBBoneNode_V5_Count	1

static const u32 SFSBBoneNode_V5_TypeToTag[1] = {
 TAG_MPEG4_SBBone
};

#define SFSBMuscleNode_V5_NUMBITS		1
#define SFSBMuscleNode_V5_Count	1

static const u32 SFSBMuscleNode_V5_TypeToTag[1] = {
 TAG_MPEG4_SBMuscle
};

#define SFSBSegmentNode_V5_NUMBITS		1
#define SFSBSegmentNode_V5_Count	1

static const u32 SFSBSegmentNode_V5_TypeToTag[1] = {
 TAG_MPEG4_SBSegment
};

#define SFSBSiteNode_V5_NUMBITS		1
#define SFSBSiteNode_V5_Count	1

static const u32 SFSBSiteNode_V5_TypeToTag[1] = {
 TAG_MPEG4_SBSite
};

#define SFBaseMeshNode_V5_NUMBITS		1
#define SFBaseMeshNode_V5_Count	1

static const u32 SFBaseMeshNode_V5_TypeToTag[1] = {
 TAG_MPEG4_SubdivisionSurface
};

#define SFSubdivSurfaceSectorNode_V5_NUMBITS		1
#define SFSubdivSurfaceSectorNode_V5_Count	1

static const u32 SFSubdivSurfaceSectorNode_V5_TypeToTag[1] = {
 TAG_MPEG4_SubdivSurfaceSector
};


u32 NDT_V5_GetNumBits(u32 NDT_Tag);
u32 NDT_V5_GetNodeTag(u32 Context_NDT_Tag, u32 NodeType);
u32 NDT_V5_GetNodeType(u32 NDT_Tag, u32 NodeTag);




/* NDT BIFS Version 6 */

#define SFWorldNode_V6_NUMBITS		4
#define SFWorldNode_V6_Count	12

static const u32 SFWorldNode_V6_TypeToTag[12] = {
 TAG_MPEG4_Clipper2D, TAG_MPEG4_ColorTransform, TAG_MPEG4_Ellipse, TAG_MPEG4_LinearGradient, TAG_MPEG4_PathLayout, TAG_MPEG4_RadialGradient, TAG_MPEG4_SynthesizedTexture, TAG_MPEG4_TransformMatrix2D, TAG_MPEG4_Viewport, TAG_MPEG4_XCurve2D, TAG_MPEG4_XFontStyle, TAG_MPEG4_XLineProperties
};

#define SF3DNode_V6_NUMBITS		3
#define SF3DNode_V6_Count	5

static const u32 SF3DNode_V6_TypeToTag[5] = {
 TAG_MPEG4_Clipper2D, TAG_MPEG4_ColorTransform, TAG_MPEG4_PathLayout, TAG_MPEG4_TransformMatrix2D, TAG_MPEG4_Viewport
};

#define SF2DNode_V6_NUMBITS		3
#define SF2DNode_V6_Count	5

static const u32 SF2DNode_V6_TypeToTag[5] = {
 TAG_MPEG4_Clipper2D, TAG_MPEG4_ColorTransform, TAG_MPEG4_PathLayout, TAG_MPEG4_TransformMatrix2D, TAG_MPEG4_Viewport
};

#define SFGeometryNode_V6_NUMBITS		2
#define SFGeometryNode_V6_Count	2

static const u32 SFGeometryNode_V6_TypeToTag[2] = {
 TAG_MPEG4_Ellipse, TAG_MPEG4_XCurve2D
};

#define SFTextureNode_V6_NUMBITS		2
#define SFTextureNode_V6_Count	3

static const u32 SFTextureNode_V6_TypeToTag[3] = {
 TAG_MPEG4_LinearGradient, TAG_MPEG4_RadialGradient, TAG_MPEG4_SynthesizedTexture
};

#define SFFontStyleNode_V6_NUMBITS		1
#define SFFontStyleNode_V6_Count	1

static const u32 SFFontStyleNode_V6_TypeToTag[1] = {
 TAG_MPEG4_XFontStyle
};

#define SFLinePropertiesNode_V6_NUMBITS		1
#define SFLinePropertiesNode_V6_Count	1

static const u32 SFLinePropertiesNode_V6_TypeToTag[1] = {
 TAG_MPEG4_XLineProperties
};

#define SFTextureTransformNode_V6_NUMBITS		1
#define SFTextureTransformNode_V6_Count	1

static const u32 SFTextureTransformNode_V6_TypeToTag[1] = {
 TAG_MPEG4_TransformMatrix2D
};

#define SFViewportNode_V6_NUMBITS		1
#define SFViewportNode_V6_Count	1

static const u32 SFViewportNode_V6_TypeToTag[1] = {
 TAG_MPEG4_Viewport
};


u32 NDT_V6_GetNumBits(u32 NDT_Tag);
u32 NDT_V6_GetNodeTag(u32 Context_NDT_Tag, u32 NodeType);
u32 NDT_V6_GetNodeType(u32 NDT_Tag, u32 NodeTag);




/* NDT BIFS Version 7 */

#define SFWorldNode_V7_NUMBITS		4
#define SFWorldNode_V7_Count	11

static const u32 SFWorldNode_V7_TypeToTag[11] = {
 TAG_MPEG4_AdvancedAudioBuffer, TAG_MPEG4_AudioChannelConfig, TAG_MPEG4_DepthImageV2, TAG_MPEG4_MorphShape, TAG_MPEG4_MultiTexture, TAG_MPEG4_PointTextureV2, TAG_MPEG4_SBVCAnimationV2, TAG_MPEG4_SimpleTextureV2, TAG_MPEG4_SurroundingSound, TAG_MPEG4_Transform3DAudio, TAG_MPEG4_WideSound
};

#define SF3DNode_V7_NUMBITS		3
#define SF3DNode_V7_Count	6

static const u32 SF3DNode_V7_TypeToTag[6] = {
 TAG_MPEG4_DepthImageV2, TAG_MPEG4_MorphShape, TAG_MPEG4_SBVCAnimationV2, TAG_MPEG4_SurroundingSound, TAG_MPEG4_Transform3DAudio, TAG_MPEG4_WideSound
};

#define SF2DNode_V7_NUMBITS		2
#define SF2DNode_V7_Count	3

static const u32 SF2DNode_V7_TypeToTag[3] = {
 TAG_MPEG4_MorphShape, TAG_MPEG4_SBVCAnimationV2, TAG_MPEG4_Transform3DAudio
};

#define SFAudioNode_V7_NUMBITS		2
#define SFAudioNode_V7_Count	2

static const u32 SFAudioNode_V7_TypeToTag[2] = {
 TAG_MPEG4_AdvancedAudioBuffer, TAG_MPEG4_AudioChannelConfig
};

#define SFTextureNode_V7_NUMBITS		1
#define SFTextureNode_V7_Count	1

static const u32 SFTextureNode_V7_TypeToTag[1] = {
 TAG_MPEG4_MultiTexture
};

#define SFDepthImageNode_V7_NUMBITS		1
#define SFDepthImageNode_V7_Count	1

static const u32 SFDepthImageNode_V7_TypeToTag[1] = {
 TAG_MPEG4_DepthImageV2
};

#define SFDepthTextureNode_V7_NUMBITS		2
#define SFDepthTextureNode_V7_Count	2

static const u32 SFDepthTextureNode_V7_TypeToTag[2] = {
 TAG_MPEG4_PointTextureV2, TAG_MPEG4_SimpleTextureV2
};


u32 NDT_V7_GetNumBits(u32 NDT_Tag);
u32 NDT_V7_GetNodeTag(u32 Context_NDT_Tag, u32 NodeType);
u32 NDT_V7_GetNodeType(u32 NDT_Tag, u32 NodeTag);




/* NDT BIFS Version 8 */

#define SFWorldNode_V8_NUMBITS		2
#define SFWorldNode_V8_Count	2

static const u32 SFWorldNode_V8_TypeToTag[2] = {
 TAG_MPEG4_ScoreShape, TAG_MPEG4_MusicScore
};

#define SF3DNode_V8_NUMBITS		1
#define SF3DNode_V8_Count	1

static const u32 SF3DNode_V8_TypeToTag[1] = {
 TAG_MPEG4_ScoreShape
};

#define SF2DNode_V8_NUMBITS		1
#define SF2DNode_V8_Count	1

static const u32 SF2DNode_V8_TypeToTag[1] = {
 TAG_MPEG4_ScoreShape
};

#define SFMusicScoreNode_V8_NUMBITS		1
#define SFMusicScoreNode_V8_Count	1

static const u32 SFMusicScoreNode_V8_TypeToTag[1] = {
 TAG_MPEG4_MusicScore
};


u32 NDT_V8_GetNumBits(u32 NDT_Tag);
u32 NDT_V8_GetNodeTag(u32 Context_NDT_Tag, u32 NodeType);
u32 NDT_V8_GetNodeType(u32 NDT_Tag, u32 NodeTag);




/* NDT BIFS Version 9 */

#define SFWorldNode_V9_NUMBITS		3
#define SFWorldNode_V9_Count	6

static const u32 SFWorldNode_V9_TypeToTag[6] = {
 TAG_MPEG4_FootPrintSetNode, TAG_MPEG4_FootPrintNode, TAG_MPEG4_BuildingPartNode, TAG_MPEG4_RoofNode, TAG_MPEG4_FacadeNode, TAG_MPEG4_Shadow
};

#define SF3DNode_V9_NUMBITS		3
#define SF3DNode_V9_Count	6

static const u32 SF3DNode_V9_TypeToTag[6] = {
 TAG_MPEG4_FootPrintSetNode, TAG_MPEG4_FootPrintNode, TAG_MPEG4_BuildingPartNode, TAG_MPEG4_RoofNode, TAG_MPEG4_FacadeNode, TAG_MPEG4_Shadow
};

#define SFGeometryNode_V9_NUMBITS		3
#define SFGeometryNode_V9_Count	6

static const u32 SFGeometryNode_V9_TypeToTag[6] = {
 TAG_MPEG4_FootPrintSetNode, TAG_MPEG4_FootPrintNode, TAG_MPEG4_BuildingPartNode, TAG_MPEG4_RoofNode, TAG_MPEG4_FacadeNode, TAG_MPEG4_Shadow
};


u32 NDT_V9_GetNumBits(u32 NDT_Tag);
u32 NDT_V9_GetNodeTag(u32 Context_NDT_Tag, u32 NodeType);
u32 NDT_V9_GetNodeType(u32 NDT_Tag, u32 NodeTag);




/* NDT BIFS Version 10 */

#define SFWorldNode_V10_NUMBITS		3
#define SFWorldNode_V10_Count	5

static const u32 SFWorldNode_V10_TypeToTag[5] = {
 TAG_MPEG4_CacheTexture, TAG_MPEG4_EnvironmentTest, TAG_MPEG4_KeyNavigator, TAG_MPEG4_SpacePartition, TAG_MPEG4_Storage
};

#define SF3DNode_V10_NUMBITS		3
#define SF3DNode_V10_Count	5

static const u32 SF3DNode_V10_TypeToTag[5] = {
 TAG_MPEG4_CacheTexture, TAG_MPEG4_EnvironmentTest, TAG_MPEG4_KeyNavigator, TAG_MPEG4_SpacePartition, TAG_MPEG4_Storage
};

#define SF2DNode_V10_NUMBITS		3
#define SF2DNode_V10_Count	4

static const u32 SF2DNode_V10_TypeToTag[4] = {
 TAG_MPEG4_CacheTexture, TAG_MPEG4_EnvironmentTest, TAG_MPEG4_KeyNavigator, TAG_MPEG4_Storage
};

#define SFTextureNode_V10_NUMBITS		1
#define SFTextureNode_V10_Count	1

static const u32 SFTextureNode_V10_TypeToTag[1] = {
 TAG_MPEG4_CacheTexture
};


u32 NDT_V10_GetNumBits(u32 NDT_Tag);
u32 NDT_V10_GetNodeTag(u32 Context_NDT_Tag, u32 NodeType);
u32 NDT_V10_GetNodeType(u32 NDT_Tag, u32 NodeTag);



u32 NDT_GetChildTable(u32 NodeTag);




#endif /*GPAC_DISABLE_BIFS*/



#endif		/*_NDT_H*/

