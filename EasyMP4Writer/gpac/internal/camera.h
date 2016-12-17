/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005
 *					All rights reserved
 *
 *  This file is part of GPAC / Scene Compositor sub-project
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

#ifndef _CAMERA_H_
#define _CAMERA_H_

//#include <gpac/internal/compositor_dev.h>
#include <gpac/scenegraph_vrml.h>

/*camera flags*/
enum
{
	/*if set frustum needs to be recomputed
	we avoid computing it at each frame/interaction since that's a lot of matrix maths*/
	CAM_IS_DIRTY = 1,
	/*if set when ortho, indicates the viewport matrix shall be used when computing modelview (2D only)*/
	CAM_HAS_VIEWPORT = 1<<2,
	/*if set when ortho to disable LookAt mode*/
	CAM_NO_LOOKAT = 1<<3,
};

enum
{
	/*only valid at root node*/
	CULL_NOT_SET = 0,
	/*subtree completely outside view vol*/
	CULL_OUTSIDE,
	/*subtree completely inside view vol*/
	CULL_INSIDE,
	/*subtree overlaps view vol - FIXME: would be nice to keep track of intersecting planes*/
	CULL_INTERSECTS
};

/*navigation info flags - non-VRML ones are simply blaxxun contact ones */
enum
{
	/*headlight is on*/
	NAV_HEADLIGHT = 1,
	/*any navigation (eg, user-interface navigation control allowed)*/
	NAV_ANY = 1<<1
};

/*frustum object*/
enum
{
	FRUS_NEAR_PLANE = 0,
	FRUS_FAR_PLANE,
	FRUS_LEFT_PLANE,
	FRUS_RIGHT_PLANE,
	FRUS_BOTTOM_PLANE,
	FRUS_TOP_PLANE
};



enum
{
	/*nothing detected*/
	CF_NONE = 0,
	/*collision detected*/
	CF_COLLISION = 1,
	/*gravity detecion enabled*/
	CF_DO_GRAVITY = (1<<1),
	/*gravity detected*/
	CF_GRAVITY = (1<<2),
	/*viewpoint is stored at end of animation*/
	CF_STORE_VP = (1<<3),
};

typedef struct _camera
{
	/*this flag MUST be set by the owner of the camera*/
	Bool is_3D;

	u32 flags;

	/*viewport info*/
	GF_Rect vp;
	/*not always same as VP due to aspect ratio*/
	Fixed width, height;
	Fixed z_near, z_far;

	/*current vectors*/
	Fixed fieldOfView;
	SFVec3f up, position, target;

	/*initial vp for reset*/
	SFVec3f vp_position;
	SFRotation vp_orientation;
	Fixed vp_fov, vp_dist;

	/*animation path*/
	SFVec3f start_pos, end_pos;
	SFRotation start_ori, end_ori;
	Fixed start_fov, end_fov;
	/*for 2D cameras we never animate except for vp reset*/
	Fixed start_zoom, end_zoom;
	SFVec2f start_trans, start_rot;

	/*center of examine movement*/
	SFVec3f examine_center;

	/*anim*/
	u32 anim_len, anim_start;
	Bool jumping;
	Fixed dheight;

	/*navigation info - overwridden by any bindable NavigationInfo node*/
	u32 navigation_flags, navigate_mode;
	SFVec3f avatar_size;
	Fixed visibility, speed;
	Bool had_viewpoint, had_nav_info;

	/*last camera position before collision& gravity detection*/
	SFVec3f last_pos;
	u32 collide_flags;
	/*collision point in world coord*/
	SFVec3f collide_point;
	/*collide dist in world coord, used to check if we have a closer collision*/
	Fixed collide_dist;
	/*ground in world coord*/
	SFVec3f ground_point;
	/*ground dist in world coord, used to check if we have a closer ground*/
	Fixed ground_dist;
	/*for obstacle detection*/
	Bool last_had_ground;
	Bool last_had_col;

	/*projection & modelview matrices*/
	GF_Matrix projection, modelview;
	/*unprojection matrix = INV(P*M) used for screen->world compute*/
	GF_Matrix unprojection;
	/*viewport matrix*/
	GF_Matrix viewport;
	/*frustum planes*/
	GF_Plane planes[6];
	/*p vertex idx per plane (for bbox-frustum intersection checks)*/
	u32 p_idx[6];
	/*frustrum bounding sphere (for sphere-sphere frustum intersection checks)*/
	SFVec3f center;
	Fixed radius;

	GF_BBox world_bbox;
} GF_Camera;

/*invalidate camera to force recompute of all params*/
void camera_invalidate(GF_Camera *cam);
/*updates camera. user transform is only used in 2D to set global user zoom/pan/translate*/
void camera_update(GF_Camera *cam, GF_Matrix2D *user_transform, Bool center_coords, Fixed horizontal_shift, Fixed viewing_distance, Fixed view_distance_offset, u32 camera_layout);
/*reset to last viewport*/
void camera_reset_viewpoint(GF_Camera *cam, Bool animate);
/*move camera to given vp*/
void camera_move_to(GF_Camera *cam, SFVec3f pos, SFVec3f target, SFVec3f up);
Bool camera_animate(GF_Camera *cam);
void camera_stop_anim(GF_Camera *cam);
/*start jump mode*/
void camera_jump(GF_Camera *cam);

void camera_set_vectors(GF_Camera *cam, SFVec3f pos, SFRotation ori, Fixed fov);

SFRotation camera_get_orientation(SFVec3f pos, SFVec3f target, SFVec3f up);
SFVec3f camera_get_pos_dir(GF_Camera *cam);
SFVec3f camera_get_target_dir(GF_Camera *cam);
SFVec3f camera_get_right_dir(GF_Camera *cam);
void camera_set_2d(GF_Camera *cam);

#endif

