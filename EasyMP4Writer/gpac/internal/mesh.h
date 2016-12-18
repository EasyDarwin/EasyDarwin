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


#ifndef _GF_MESH_H_
#define _GF_MESH_H_

#include <gpac/scenegraph_vrml.h>
#include <gpac/path2d.h>

/*by default we store each color on 32 bit rather than 4 floats (128 bits)*/

//#define MESH_USE_SFCOLOR

#ifdef MESH_USE_SFCOLOR
#define MESH_MAKE_COL(_argb) _argb
#define MESH_GET_COLOR(_argb, _vertex) _argb = (_vertex).color;
#else
#define MESH_MAKE_COL(_argb) GF_COL_ARGB(FIX2INT(255*(_argb.alpha)), FIX2INT(255*(_argb.blue)), FIX2INT(255*(_argb.green)), FIX2INT(255*(_argb.red)))
#define MESH_GET_COLOR(_argb, _vertex) { _argb.alpha = INT2FIX(GF_COL_A((_vertex).color))/255; _argb.red = INT2FIX(GF_COL_R((_vertex).color))/255; _argb.green = INT2FIX(GF_COL_G((_vertex).color))/255; _argb.blue = INT2FIX(GF_COL_B((_vertex).color))/255; }
#endif

/*by default we store normals as signed bytes rather than floats*/

//#define MESH_USE_FIXED_NORMAL

#ifdef MESH_USE_FIXED_NORMAL
#define MESH_SET_NORMAL(_vertex, _nor) _vertex.normal = _nor;
#define MESH_GET_NORMAL(_nor, _vertex) _nor = _vertex.normal;
#define MESH_NORMAL_UNIT	FIX_ONE
#else

typedef struct
{
	s8 x, y, z;
	s8 __dummy;
} SFVec3f_bytes;

#define MESH_NORMAL_UNIT	1

#ifdef GPAC_FIXED_POINT
#define MESH_SET_NORMAL(_vertex, _nor) { SFVec3f_bytes __nor; __nor.x = (s8) FIX2INT(_nor.x*100); __nor.y = (s8) FIX2INT(_nor.y*100); __nor.z = (s8) FIX2INT(_nor.z*100); __nor.__dummy=0; _vertex.normal = __nor; }
#define MESH_GET_NORMAL(_nor, _vertex) { (_nor).x = INT2FIX(_vertex.normal.x); (_nor).y = INT2FIX(_vertex.normal.y); (_nor).z = INT2FIX(_vertex.normal.z); gf_vec_norm(&(_nor)); }
#else
#define MESH_SET_NORMAL(_vertex, _nor) { SFVec3f_bytes __nor; __nor.x = (s8) (_nor.x*100); __nor.y = (s8) (_nor.y*100); __nor.z = (s8) (_nor.z*100); __nor.__dummy=0; _vertex.normal = __nor; }
#define MESH_GET_NORMAL(_nor, _vertex) { (_nor).x = _vertex.normal.x; (_nor).y = _vertex.normal.y; (_nor).z = _vertex.normal.z; gf_vec_norm(&(_nor)); }
#endif

#endif

typedef struct
{
	/*position*/
	SFVec3f pos;	
	/*texture coordinates*/
	SFVec2f texcoords;
	/*normal*/
#ifdef MESH_USE_FIXED_NORMAL
	SFVec3f normal;
#else
	SFVec3f_bytes normal;
#endif
	/*color if used by mesh object*/
#ifdef MESH_USE_SFCOLOR
	SFColorRGBA color;
#else
	u32 color;
#endif
} GF_Vertex;

/*memory offset in bytes from start of vertex to texcoords = 3 * 4bytes*/
#define MESH_TEX_OFFSET	12
/*memory offset in bytes from start of vertex to normal = 5 * 4bytes*/
#define MESH_NORMAL_OFFSET	20
/*memory offset in bytes from start of vertex to color - platform dependent*/
#ifdef MESH_USE_FIXED_NORMAL
/*3+2+3 * 4*/
#define MESH_COLOR_OFFSET	32
#else
/*3+2 * 4 + 4 (3 + 1 byte alignment)*/
#define MESH_COLOR_OFFSET	24
#endif

/*mesh type used*/
enum
{
	/*default: triangles described by indices (nb triangles = nb indices / 3) */
	MESH_TRIANGLES = 0,
	/*point set: indices is meaningless*/
	MESH_POINTSET,
	/*line set: lines described by indices (nb lines = nb indices / 2) */
	MESH_LINESET,
};

/*mesh flags*/
enum
{
	/*vertex.color is used*/
	MESH_HAS_COLOR = 1, 
	/*mesh is 2D: normal should be ignored and a global normal set to 0 0 1*/
	MESH_IS_2D = 1<<1, 
	/*mesh has no texture coords - disable texturing*/
	MESH_NO_TEXTURE = 1<<2, 
	/*mesh faces are clockwise*/
	MESH_IS_CW = 1<<3, 
	/*mesh is solid (back face culling + 2 side lighting)*/
	MESH_IS_SOLID = 1<<4, 
	/*mesh has smoothed normals*/
	MESH_IS_SMOOTHED = 1<<5, 
	/*vertex.color is used with alpha channel*/
	MESH_HAS_ALPHA = 1<<6, 
};

/*indexes as used in glDrawElements - note that integer type is not allowed with oglES*/
#ifdef GPAC_USE_OGL_ES
#define IDX_TYPE	u16
#else
#define IDX_TYPE	u32
#endif

/*mesh object used by all 2D/3D primitives. */
typedef struct __gf_mesh
{
	/*vertex list*/
	u32 v_count, v_alloc;
	GF_Vertex *vertices;
	/*triangle indexes*/
	u32 i_count, i_alloc;
	IDX_TYPE *indices;

	/*one of the above type*/
	u32 mesh_type;

	/*one of the above flags*/
	u32 flags;

	/*bounds info: bounding box and bounding sphere radius*/
	GF_BBox bounds;

	/*aabb tree of the mesh if any*/
	struct __AABBNode *aabb_root;
	/*triangle indexes used in AABB tree - order may be different than the one in mesh->indices*/
	IDX_TYPE *aabb_indices;
//	u32 aabb_nb_index;

	u32 vbo;
	Bool vbo_dirty, vbo_dynamic;
} GF_Mesh;

GF_Mesh *new_mesh();
void mesh_free(GF_Mesh *mesh);
/*reset mesh*/
void mesh_reset(GF_Mesh *mesh);
/*recompute mesh bounds*/
void mesh_update_bounds(GF_Mesh *mesh);
/*adds new vertex*/
void mesh_set_vertex_vx(GF_Mesh *mesh, GF_Vertex *vx);
/*adds new vertex (exported for tesselator only)*/
void mesh_set_vertex(GF_Mesh *mesh, Fixed x, Fixed y, Fixed z, Fixed nx, Fixed ny, Fixed nz, Fixed u, Fixed v);
/*adds an index (exported for tesselator only)*/
void mesh_set_index(GF_Mesh *mesh, u32 idx);
/*adds an point & associated color, normal set to NULL*/
void mesh_set_point(GF_Mesh *mesh, Fixed x, Fixed y, Fixed z, SFColorRGBA col);
/*adds an index (exported for tesselator only)*/
void mesh_set_triangle(GF_Mesh *mesh, u32 id1, u32 id2, u32 id3);
/*make dest mesh the clone of orig*/
void mesh_clone(GF_Mesh *dest, GF_Mesh *orig);
/*recompute all normals*/
void mesh_recompute_normals(GF_Mesh *mesh);
/*generate texture coordinate - ONLY LOCAL MODES SUPPORTED FOR NOW*/
void mesh_generate_tex_coords(GF_Mesh *mesh, GF_Node *__texCoords);

/*inserts a box (lines only) of size 1.0 1.0 1.0*/
void mesh_new_unit_bbox(GF_Mesh *mesh);

/*insert base primitives - low res indicates less subdivision steps for circles (cone, cylinder, ellipse, sphere)*/
void mesh_new_rectangle(GF_Mesh *mesh, SFVec2f size, SFVec2f *orig, Bool flip);
void mesh_new_ellipse(GF_Mesh *mesh, Fixed a_dia, Fixed b_dia, Bool low_res);
void mesh_new_box(GF_Mesh *mesh, SFVec3f size);
void mesh_new_cylinder(GF_Mesh *mesh, Fixed height, Fixed radius, Bool bottom, Bool side, Bool top, Bool low_res);
void mesh_new_cone(GF_Mesh *mesh, Fixed height, Fixed radius, Bool bottom, Bool side, Bool low_res);
void mesh_new_sphere(GF_Mesh *mesh, Fixed radius, Bool low_res);
/*inserts ILS/ILS2D and IFS2D outline when not filled*/
void mesh_new_ils(GF_Mesh *mesh, GF_Node *__coord, MFInt32 *coordIndex, GF_Node *__color, MFInt32 *colorIndex, Bool colorPerVertex, Bool do_close);
/*inserts IFS2D*/
void mesh_new_ifs2d(GF_Mesh *mesh, GF_Node *ifs2d);
/*inserts IFS*/
void mesh_new_ifs(GF_Mesh *mesh, GF_Node *ifs);
/*inserts PS/PS2D*/
void mesh_new_ps(GF_Mesh *mesh, GF_Node *__coord, GF_Node *__color);
/*inserts ElevationGrid*/
void mesh_new_elevation_grid(GF_Mesh *mesh, GF_Node *eg);
/*inserts Extrusion*/
void mesh_new_extrusion(GF_Mesh *mesh, GF_Node *ext);
/*builds mesh from path, performing tesselation if desired*/
void mesh_from_path(GF_Mesh *mesh, GF_Path *path);
/*builds mesh for outline of the given path*/
void mesh_get_outline(GF_Mesh *mesh, GF_Path *path);
/*constructs an extrusion from given path - mesh is reseted, txcoords computed from path bounds
@thespine: spine line
@creaseAngle: creaseAngle for normal smoothing, 0 for no smoothing
begin_cap, end_cap: indicates whether start/end faces shall be added
@spine_ori: orientation at spine points
@spine_scale: scale at spine points
@tx_along_spine: if set, texture coords are generated so that the texture is mapped on the side, 
otherwise the same txcoords are used all along the extrusion spine
*/
void mesh_extrude_path(GF_Mesh *mesh, GF_Path *path, MFVec3f *thespine, Fixed creaseAngle, Bool begin_cap, Bool end_cap, MFRotation *spine_ori, MFVec2f *spine_scale, Bool tx_along_spine);
/*special extension of the above: APPENDS an extrusion from given path - mesh is NOT reseted, txcoords are computed based on min_cx, min_cy, width_cx, width_cy*/
void mesh_extrude_path_ext(GF_Mesh *mesh, GF_Path *path, MFVec3f *thespine, Fixed creaseAngle, Fixed min_cx, Fixed min_cy, Fixed width_cx, Fixed width_cy, Bool begin_cap, Bool end_cap, MFRotation *spine_ori, MFVec2f *spine_scale, Bool tx_along_spine);

/*returns 1 if intersection and set outPoint to closest intersection, 0 otherwise*/
Bool gf_mesh_intersect_ray(GF_Mesh *mesh, GF_Ray *r, SFVec3f *outPoint, SFVec3f *outNormal, SFVec2f *outTexCoords);
/*returns 1 if any face is less than min_dist from pos, with collision point on closest face (towards pos)*/
Bool gf_mesh_closest_face(GF_Mesh *mesh, SFVec3f pos, Fixed min_dist, SFVec3f *outPoint);




/*AABB tree node (exported for bounds drawing)*/
typedef struct __AABBNode
{
	/*bbox*/
	SFVec3f min, max;
	/*sorted indices in mesh indices list*/
	IDX_TYPE *indices;
	/*nb triangles*/
	u32 nb_idx;
	/*children nodes, NULL if leaf*/
	struct __AABBNode *pos, *neg;
} AABBNode;

/*tree construction modes*/
enum
{
	/*AABB tree is not used*/
	AABB_NONE, 
	/*longest box axis is used to divide an AABB node*/
	AABB_LONGEST, 
	/*keep tree well-balanced*/
	AABB_BALANCED,
	/*best axis is use: test largest, then middle, then smallest axis*/
	AABB_BEST_AXIS, 
	/*use variance to pick axis*/
	AABB_SPLATTER,
	/*fifty/fifty point split*/
	AABB_FIFTY,
};

void gf_mesh_build_aabbtree(GF_Mesh *mesh);


/*
 *		tesselation functions
 */

/*appends given face (and tesselate if needed) to the mesh. Only vertices are used in the face
indices are ignored. 
partially implemented on ogl-ES*/
void TesselateFaceMesh(GF_Mesh *mesh, GF_Mesh *face);

#ifdef GPAC_HAS_GLU
/*converts 2D path into a polygon - these are only partially implemented when using oglES
for_outline:
	 0, regular odd/even windining rule with texCoords
	 1, zero-non-zero windining rule without texCoords
	 2, zero-non-zero windining rule with texCoords
*/
void gf_mesh_tesselate_path(GF_Mesh *mesh, GF_Path *path, u32 outline_style);

/*appends given face (and tesselate if needed) to the mesh. Only vertices are used in the face
indices are ignored. 
Same as TesselateFaceMesh + faces info to determine where are the polygons in the face - used by extruder only
*/
void TesselateFaceMeshComplex(GF_Mesh *dest, GF_Mesh *orig, u32 nbFaces, u32 *ptsPerFaces);

#endif

#endif		/*_GF_MESH_H_*/

