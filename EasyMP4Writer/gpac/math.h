/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005 
 *					All rights reserved
 *
 *  This file is part of GPAC / common tools sub-project
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

#ifndef _GF_MATH_H_
#define _GF_MATH_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *	\file <gpac/math.h>
 *	\brief math and trigo functions.
 */

#include <gpac/setup.h>
	
/*NOTE: there is a conflict on Win32 VC6 with C++ and gpac headers when including <math.h>*/
#if !defined(__cplusplus) || defined(__SYMBIAN32__)
#include <math.h>
#endif


/*!
 *\addtogroup math_grp math
 *\ingroup utils_grp
 *\brief Mathematics and Trigonometric functions
 *
 *This section documents the math and trigo functions used in the GPAC framework. GPAC can be compiled with
 *fixed-point support, representing float values on a 16.16 signed integer, which implies a developer 
 *must take care of float computations when using GPAC.\n
 *A developper should not need to know in which mode the framework has been compiled as long as he uses
 *the math functions of GPAC which work in both float and fixed-point mode.\n
 *Using fixed-point version is decided at compilation time and cannot be changed. The feature is signaled
 *through the following macros:
 *- GPAC_FIXED_POINT: when defined, GPAC has been compiled in fixed-point mode
 *- GPAC_NO_FIXED_POINT: when defined, GPAC has been compiled in regular (float) mode
 *	@{
 */


/*****************************************************************************************
			FIXED-POINT SUPPORT - HARDCODED FOR 16.16 representation
	the software rasterizer also use a 16.16 representation even in non-fixed version
******************************************************************************************/

#ifdef GPAC_FIXED_POINT

/*!
 *Fixed 16.16 number
 *\hideinitializer
 \note This documentation has been generated for a fixed-point version of the GPAC framework.
 */
typedef s32 Fixed;
#define FIX_ONE			0x10000L
#define INT2FIX(v)		((Fixed)( ((s32) (v) ) << 16))
#define FLT2FIX(v)		((Fixed) ((v) * FIX_ONE))
#define FIX2INT(v)		((s32)(((v)+((FIX_ONE>>1)))>>16))
#define FIX2FLT(v)		((Float)( ((Float)(v)) / ((Float) FIX_ONE)))
#define FIX_EPSILON		2
#define FIX_MAX			0x7FFFFFFF
#define FIX_MIN			-FIX_MAX
#define GF_PI2		102944
#define GF_PI		205887
#define GF_2PI		411774

/*!\return 1/a, expressed as fixed number*/
Fixed gf_invfix(Fixed a);
/*!\return a*b, expressed as fixed number*/
Fixed gf_mulfix(Fixed a, Fixed b);
/*!\return a*b/c, expressed as fixed number*/
Fixed gf_muldiv(Fixed a, Fixed b, Fixed c);
/*!\return a/b, expressed as fixed number*/
Fixed gf_divfix(Fixed a, Fixed b);
/*!\return sqrt(a), expressed as fixed number*/
Fixed gf_sqrt(Fixed x);
/*!\return ceil(a), expressed as fixed number*/
Fixed gf_ceil(Fixed a);
/*!\return floor(a), expressed as fixed number*/
Fixed gf_floor(Fixed a);
/*!\return cos(a), expressed as fixed number*/
Fixed gf_cos(Fixed angle);
/*!\return sin(a), expressed as fixed number*/
Fixed gf_sin(Fixed angle);
/*!\return tan(a), expressed as fixed number*/
Fixed gf_tan(Fixed angle);
/*!\return acos(a), expressed as fixed number*/
Fixed gf_acos(Fixed angle);
/*!\return asin(a), expressed as fixed number*/
Fixed gf_asin(Fixed angle);
/*!\return atan(y, x), expressed as fixed number*/
Fixed gf_atan2(Fixed y, Fixed x);

#else


/*!Fixed is 32bit float number
 \note This documentation has been generated for a float version of the GPAC framework.
*/
typedef Float Fixed;
#define FIX_ONE			1.0f
#define INT2FIX(v)		((Float) (v))
#define FLT2FIX(v)		((Float) (v))
#define FIX2INT(v)		((s32)(v))
#define FIX2FLT(v)		((Float) (v))
#define FIX_EPSILON		GF_EPSILON_FLOAT
#define FIX_MAX			GF_MAX_FLOAT
#define FIX_MIN			-GF_MAX_FLOAT
#define GF_PI2		1.5707963267949f
#define GF_PI		3.1415926535898f
#define GF_2PI		6.2831853071796f

/*!\hideinitializer 1/_a, expressed as fixed number*/
#define gf_invfix(_a)	(FIX_ONE/(_a))
/*!\hideinitializer _a*_b, expressed as fixed number*/
#define gf_mulfix(_a, _b)		((_a)*(_b))
/*!\hideinitializer _a*_b/_c, expressed as fixed number*/
#define gf_muldiv(_a, _b, _c)	((_c) ? (_a)*(_b)/(_c) : GF_MAX_FLOAT)
/*!\hideinitializer _a/_b, expressed as fixed number*/
#define gf_divfix(_a, _b)		((_b) ? (_a)/(_b) : GF_MAX_FLOAT)
/*!\hideinitializer sqrt(_a), expressed as fixed number*/
#define gf_sqrt(_a) ((Float) sqrt(_a))
/*!\hideinitializer ceil(_a), expressed as fixed number*/
#define gf_ceil(_a) ((Float) ceil(_a))
/*!\hideinitializer floor(_a), expressed as fixed number*/
#define gf_floor(_a) ((Float) floor(_a))
/*!\hideinitializer cos(_a), expressed as fixed number*/
#define gf_cos(_a) ((Float) cos(_a))
/*!\hideinitializer sin(_a), expressed as fixed number*/
#define gf_sin(_a) ((Float) sin(_a))
/*!\hideinitializer tan(_a), expressed as fixed number*/
#define gf_tan(_a) ((Float) tan(_a))
/*!\hideinitializer atan2(_y,_x), expressed as fixed number*/
#define gf_atan2(_y, _x) ((Float) atan2(_y, _x))
/*!\hideinitializer acos(_a), expressed as fixed number*/
#define gf_acos(_a) ((Float) acos(_a))
/*!\hideinitializer asin(_a), expressed as fixed number*/
#define gf_asin(_a) ((Float) asin(_a))

#endif

/*!\def FIX_ONE
 \hideinitializer
 Fixed unit value
*/
/*!\def INT2FIX(v)
 \hideinitializer
 Conversion from integer to fixed
*/
/*!\def FLT2FIX(v)
 \hideinitializer
 Conversion from float to fixed
*/
/*!\def FIX2INT(v)
 \hideinitializer
 Conversion from fixed to integer
*/
/*!\def FIX2FLT(v)
 \hideinitializer
 Conversion from fixed to float
*/
/*!\def FIX_EPSILON
 \hideinitializer
 Epsilon Fixed (positive value closest to 0)
*/
/*!\def FIX_MAX
 \hideinitializer
 Maximum Fixed (maximum representable fixed value)
*/
/*!\def FIX_MIN
 \hideinitializer
 Minimum Fixed (minimum representable fixed value)
*/
/*!\def GF_PI2
 \hideinitializer
 PI/2 expressed as Fixed
*/
/*!\def GF_PI
 \hideinitializer
 PI expressed as Fixed
*/
/*!\def GF_2PI
 \hideinitializer
 2*PI expressed as Fixed
*/

Fixed gf_angle_diff(Fixed a, Fixed b);

/*!
 *	\brief Field bit-size 
 *
 *	Gets the number of bits needed to represent the value.
 *	\param MaxVal Maximum value to be represented.
 *	\return number of bits required to represent the value.
 */
u32 gf_get_bit_size(u32 MaxVal);

/*!
 *	\brief Get power of 2
 *
 *	Gets the closest power of 2 greater or equal to the value.
 *	\param val value to be used.
 *	\return requested power of 2.
 */
u32 gf_get_next_pow2(u32 val);

/*!
 *\addtogroup math2d_grp math2d
 *\ingroup math_grp
 *\brief 2D Mathematics functions
 *
 *This section documents mathematic tools for 2D geometry and color matrices operations
 *	@{
 */

/*!\brief 2D point
 *
 *The 2D point object is used in all the GPAC framework for both point and vector representation.
*/
typedef struct __vec2f
{
	Fixed x;
	Fixed y;
} GF_Point2D;
/*!
 *\brief get 2D vector length
 *
 *Gets the length of a 2D vector
 *\return length of the vector
 */
Fixed gf_v2d_len(GF_Point2D *vec);
/*!
 *\brief 2D vector from polar coordinates
 *
 *Constructs a 2D vector from its polar coordinates
 *\param length the length of the vector
 *\param angle the angle of the vector in radians
 *\return the 2D vector
 */
GF_Point2D gf_v2d_from_polar(Fixed length, Fixed angle);

/*!\brief rectangle 2D
 *
 *The 2D rectangle used in the GPAC project.
 */
typedef struct
{
	/*!the left coordinate of the rectangle*/
	Fixed x;
	/*!the top coordinate of the rectangle, regardless of the canvas orientation. In other words, y is always the 
	greatest coordinate value, 	even if the rectangle is presented bottom-up. This insures proper rectangles testing*/
	Fixed y;
	/*!the width of the rectangle. Width must be greater than or equal to 0*/
	Fixed width;
	/*!the height of the rectangle. Height must be greater than or equal to 0*/
	Fixed height;
} GF_Rect;

/*!
 \brief rectangle union
 *
 *Gets the union of two rectangles.
 *\param rc1 first rectangle of the union. Upon return, this rectangle will contain the result of the union
 *\param rc2 second rectangle of the union
*/
void gf_rect_union(GF_Rect *rc1, GF_Rect *rc2);
/*!
 \brief centers a rectangle
 *
 *Builds a rectangle centered on the origin
 *\param w width of the rectangle
 *\param h height of the rectangle
 *\return centered rectangle object
*/
GF_Rect gf_rect_center(Fixed w, Fixed h);
/*!
 \brief rectangle overlap test
 *
 *Tests if two rectangles overlap.
 *\param rc1 first rectangle to test
 *\param rc2 second rectangle to test
 *\return 1 if rectangles overlap, 0 otherwise
*/
Bool gf_rect_overlaps(GF_Rect rc1, GF_Rect rc2);
/*!
 \brief rectangle identity test
 *
 *Tests if two rectangles are identical.
 *\param rc1 first rectangle to test
 *\param rc2 second rectangle to test
 *\return 1 if rectangles are identical, 0 otherwise
*/
Bool gf_rect_equal(GF_Rect rc1, GF_Rect rc2);

/*!
 *\brief pixel-aligned rectangle
 *
 *Pixel-aligned rectangle used in the GPAC framework. This is usually needed for 2D drawing algorithms.
 */
typedef struct
{
	/*!the left coordinate of the rectangle*/
	s32 x;
	/*!the top coordinate of the rectangle, regardless of the canvas orientation. In other words, y is always the 
	greatest coordinate value, even if the rectangle is presented bottom-up. This insures proper rectangles operations*/
	s32 y;
	/*!the width of the rectangle. Width must be greater than or equal to 0*/
	s32 width;
	/*!the height of the rectangle. Height must be greater than or equal to 0*/
	s32 height;
} GF_IRect;
/*!
 *\brief gets the pixelized version of a rectangle
 *
 *Returns the smallest pixel-aligned rectangle completely containing a rectangle
 *\param r the rectangle to transform
 *\return the pixel-aligned transformed rectangle
*/
GF_IRect gf_rect_pixelize(GF_Rect *r);


/*!
 *\brief 2D matrix
 *
 *The 2D affine matrix object usied in GPAC. The transformation of P(x,y) in P'(X, Y) is:
 \code
	X = m[0]*x + m[1]*y + m[2];
	Y = m[3]*x + m[4]*y + m[5];
 \endcode
*/
typedef struct
{
	Fixed m[6];
} GF_Matrix2D;

/*!\brief matrix initialization
 *\hideinitializer
 *
 *Inits the matrix to the identity matrix
*/
#define gf_mx2d_init(_obj) { memset((_obj).m, 0, sizeof(Fixed)*6); (_obj).m[0] = (_obj).m[4] = FIX_ONE; }
/*!\brief matrix copy
 *\hideinitializer
 *
 *Copies the matrix _from to the matrix _obj
*/
#define gf_mx2d_copy(_obj, from) memcpy((_obj).m, (from).m, sizeof(Fixed)*6)
/*!\brief matrix identity testing
 *\hideinitializer
 *
 *This macro evaluates to 1 if the matrix _obj is the identity matrix, 0 otherwise
*/
#define gf_mx2d_is_identity(_obj) ((!(_obj).m[1] && !(_obj).m[2] && !(_obj).m[3] && !(_obj).m[5] && ((_obj).m[0]==FIX_ONE) && ((_obj).m[4]==FIX_ONE)) ? 1 : 0)

/*!\brief 2D matrix multiplication
 *
 *Multiplies two 2D matrices from*_this
 *\param _this matrix being transformed. Once the function is called, _this contains the result matrix
 *\param from transformation matrix to add
*/
void gf_mx2d_add_matrix(GF_Matrix2D *_this, GF_Matrix2D *from);

/*!\brief 2D matrix pre-multiplication
 *
 *Multiplies two 2D matrices _this*from
 *\param _this matrix being transformed. Once the function is called, _this contains the result matrix
 *\param from transformation matrix to add
*/
void gf_mx2d_pre_multiply(GF_Matrix2D *_this, GF_Matrix2D *from);

/*!\brief matrix translating
 *
 *Translates a 2D matrix
 *\param _this matrix being transformed. Once the function is called, _this contains the result matrix
 *\param cx horizontal translation
 *\param cy vertical translation
*/
void gf_mx2d_add_translation(GF_Matrix2D *_this, Fixed cx, Fixed cy);
/*!\brief matrix rotating
 *
 *Rotates a 2D matrix
 *\param _this matrix being transformed. Once the function is called, _this contains the result matrix
 *\param cx horizontal rotation center coordinate
 *\param cy vertical rotation center coordinate
 *\param angle rotation angle in radians
*/
void gf_mx2d_add_rotation(GF_Matrix2D *_this, Fixed cx, Fixed cy, Fixed angle);
/*!\brief matrix scaling
 *
 *Scales a 2D matrix
 *\param _this matrix being transformed. Once the function is called, _this contains the result matrix
 *\param scale_x horizontal scaling factor
 *\param scale_y vertical scaling factor
*/
void gf_mx2d_add_scale(GF_Matrix2D *_this, Fixed scale_x, Fixed scale_y);
/*!\brief matrix uncentered scaling
 *
 *Scales a 2D matrix with a non-centered scale
 *\param _this matrix being transformed. Once the function is called, _this contains the result matrix
 *\param scale_x horizontal scaling factor
 *\param scale_y vertical scaling factor
 *\param cx horizontal scaling center coordinate
 *\param cy vertical scaling center coordinate
 *\param angle scale orienttion angle in radians
*/
void gf_mx2d_add_scale_at(GF_Matrix2D *_this, Fixed scale_x, Fixed scale_y, Fixed cx, Fixed cy, Fixed angle);
/*!\brief matrix skewing
 *
 *Skews a 2D matrix
 *\param _this matrix being transformed. Once the function is called, _this contains the result matrix
 *\param skew_x horizontal skew factor
 *\param skew_y vertical skew factor
*/
void gf_mx2d_add_skew(GF_Matrix2D *_this, Fixed skew_x, Fixed skew_y);
/*!\brief matrix horizontal skewing
 *
 *Skews a 2D matrix horizontally by a given angle
 *\param _this matrix being transformed. Once the function is called, _this contains the result matrix
 *\param angle horizontal skew angle in radians
*/
void gf_mx2d_add_skew_x(GF_Matrix2D *_this, Fixed angle);
/*!\brief matrix vertical skewing
 *
 *Skews a 2D matrix vertically by a given angle
 *\param _this matrix being transformed. Once the function is called, _this contains the result matrix
 *\param angle vertical skew angle in radians
*/
void gf_mx2d_add_skew_y(GF_Matrix2D *_this, Fixed angle);
/*!\brief matrix inversing
 *
 *Inverses a 2D matrix 
 *\param _this matrix being transformed. Once the function is called, _this contains the result matrix
*/
void gf_mx2d_inverse(GF_Matrix2D *_this);
/*!\brief matrix coordinate transformation
 *
 *Applies a 2D matrix transformation to coordinates
 *\param _this transformation matrix
 *\param x pointer to horizontal coordinate. Once the function is called, x contains the transformed horizontal coordinate
 *\param y pointer to vertical coordinate. Once the function is called, y contains the transformed vertical coordinate
*/
void gf_mx2d_apply_coords(GF_Matrix2D *_this, Fixed *x, Fixed *y);
/*!\brief matrix point transformation
 *
 *Applies a 2D matrix transformation to a 2D point
 *\param _this transformation matrix
 *\param pt pointer to 2D point. Once the function is called, pt contains the transformed point
*/
void gf_mx2d_apply_point(GF_Matrix2D *_this, GF_Point2D *pt);
/*!\brief matrix rectangle transformation
 *
 *Applies a 2D matrix transformation to a rectangle, giving the enclosing rectangle of the transformed one
 *\param _this transformation matrix
 *\param rc pointer to rectangle. Once the function is called, rc contains the transformed rectangle
*/
void gf_mx2d_apply_rect(GF_Matrix2D *_this, GF_Rect *rc);

/*!\brief matrix decomposition
 *
 *Decomposes a 2D matrix M as M=Scale x Rotation x Translation if possible
 *\param _this transformation matrix
 *\param scale resulting scale part
 *\param rotate resulting rotation part
 *\param translate resulting translation part
 *\return 0 if matrix cannot be decomposed, 1 otherwise
*/
Bool gf_mx2d_decompose(GF_Matrix2D *_this, GF_Point2D *scale, Fixed *rotate, GF_Point2D *translate);

/*! @} */


/*!
 *\addtogroup math3d_grp math3d
 *\ingroup math_grp
 *\brief 3D Mathematics functions
 *
 *This section documents mathematic tools for 3D geometry operations
 *	@{
 */

/*!\brief 3D point or vector
 *
 *The 3D point object is used in all the GPAC framework for both point and vector representation.
*/
typedef struct __vec3f
{
	Fixed x;
	Fixed y;
	Fixed z;
} GF_Vec;

/*base vector operations are MACROs for faster access*/
/*!\hideinitializer macro evaluating to 1 if vectors are equal, 0 otherwise*/
#define gf_vec_equal(v1, v2) (((v1).x == (v2).x) && ((v1).y == (v2).y) && ((v1).z == (v2).z))
/*!\hideinitializer macro reversing a vector v = v*/
#define gf_vec_rev(v) { (v).x = -(v).x; (v).y = -(v).y; (v).z = -(v).z; }
/*!\hideinitializer macro performing the minus operation res = v1 - v2*/
#define gf_vec_diff(res, v1, v2) { (res).x = (v1).x - (v2).x; (res).y = (v1).y - (v2).y; (res).z = (v1).z - (v2).z; }
/*!\hideinitializer macro performing the add operation res = v1 + v2*/
#define gf_vec_add(res, v1, v2) { (res).x = (v1).x + (v2).x; (res).y = (v1).y + (v2).y; (res).z = (v1).z + (v2).z; }

/*!
 *\brief get 3D vector length
 *
 *Gets the length of a 3D vector
 *\return length of the vector
 */
Fixed gf_vec_len(GF_Vec v);
/*!
 *\brief get 3D vector square length
 *
 *Gets the square length of a 3D vector
 *\return square length of the vector
 */
Fixed gf_vec_lensq(GF_Vec v);
/*!
 *\brief get 3D vector dot product
 *
 *Gets the dot product of two vectors
 *\return dot product of the vectors
 */
Fixed gf_vec_dot(GF_Vec v1, GF_Vec v2);
/*!
 *\brief vector normalization
 *
 *Norms the vector, eg make its length equal to \ref FIX_ONE
 *\param v vector to normalize
 */
void gf_vec_norm(GF_Vec *v);
/*!
 *\brief vector scaling
 *
 *Scales a vector by a given amount
 *\param v vector to scale
 *\param f scale factor
 *\return scaled vector
 */
GF_Vec gf_vec_scale(GF_Vec v, Fixed f);
/*!
 *\brief vector cross product
 *
 *Gets the cross product of two vectors
 *\param v1 first vector
 *\param v2 second vector
 *\return cross-product vector
 */
GF_Vec gf_vec_cross(GF_Vec v1, GF_Vec v2);

/*!\brief 4D vector
 *
 *The 4D vector object is used in all the GPAC framework for 4 dimension vectors, VRML Rotations and quaternions representation.
*/
typedef struct __vec4f
{
	Fixed x;
	Fixed y;
	Fixed z;
	Fixed q;
} GF_Vec4;


/*!\brief 3D matrix
 *
 *The 3D matrix object used in GPAC. The matrix is oriented like OpenGL matrices (column-major ordering), with 
 the translation part at the end of the coefficients list.
 \note Unless specified otherwise, the matrix object is always expected to represent an affine transformation.
 */
typedef struct
{
	Fixed m[16];
} GF_Matrix;


/*!\hideinitializer gets the len of a quaternion*/
#define gf_quat_len(v) gf_sqrt(gf_mulfix((v).q,(v).q) + gf_mulfix((v).x,(v).x) + gf_mulfix((v).y,(v).y) + gf_mulfix((v).z,(v).z))
/*!\hideinitializer normalizes a quaternion*/
#define gf_quat_norm(v) { \
	Fixed __mag = gf_quat_len(v);	\
	(v).x = gf_divfix((v).x, __mag); (v).y = gf_divfix((v).y, __mag); (v).z = gf_divfix((v).z, __mag); (v).q = gf_divfix((v).q, __mag);	\
	}	\

/*!\brief quaternion to rotation
 *
 *Transforms a quaternion to a Rotation, expressed as a 4 dimension vector with x,y,z for axis and q for rotation angle
 *\param quat the quaternion to transform
 *\return the rotation value
 */
GF_Vec4 gf_quat_to_rotation(GF_Vec4 *quat);
/*!\brief quaternion from rotation
 *
 *Transforms a Rotation to a quaternion
 *\param rot the rotation to transform
 *\return the quaternion value
 */
GF_Vec4 gf_quat_from_rotation(GF_Vec4 rot);
/*!inverses a quaternion*/
GF_Vec4 gf_quat_get_inv(GF_Vec4 *quat);
/*!\brief quaternion multiplication
 *
 *Multiplies two quaternions
 *\param q1 the first quaternion
 *\param q2 the second quaternion
 *\return the resulting quaternion
 */
GF_Vec4 gf_quat_multiply(GF_Vec4 *q1, GF_Vec4 *q2);
/*!\brief quaternion vector rotating
 *
 *Rotates a vector with a quaternion 
 *\param quat the quaternion modelizing the rotation
 *\param vec the vector to rotate
 *\return the resulting vector
 */
GF_Vec gf_quat_rotate(GF_Vec4 *quat, GF_Vec *vec);
/*!\brief quaternion from axis and cos
 *
 *Constructs a quaternion from an axis and a cosinus value (shortcut to \ref gf_quat_from_rotation)
 *\param axis the rotation axis
 *\param cos_a the rotation cosinus value
 *\return the resulting quaternion
 */
GF_Vec4 gf_quat_from_axis_cos(GF_Vec axis, Fixed cos_a);
/*!\brief quaternion interpolation
 *
 *Interpolates two quaternions using spherical linear interpolation
 *\param q1 the first quaternion
 *\param q2 the second quaternion
 *\param frac the fraction of the interpolation, between 0 and \ref FIX_ONE
 *\return the interpolated quaternion
 */
GF_Vec4 gf_quat_slerp(GF_Vec4 q1, GF_Vec4 q2, Fixed frac);

/*!\brief 3D Bounding Box
 *
 *The 3D Bounding Box is a 3D Axis-Aligned Bounding Box used to in various tools of the GPAC framework for bounds 
 estimation of a 3D object. It features an axis-aligned box and a sphere bounding volume for fast intersection tests.
 */
typedef struct
{
	/*!minimum x, y, and z of the object*/
	GF_Vec min_edge;
	/*!maximum x, y, and z of the object*/
	GF_Vec max_edge;

	/*!center of the bounding box.\note this is computed from min_edge and max_edge*/
	GF_Vec center;
	/*!radius of the bounding sphere for this box.\note this is computed from min_edge and max_edge*/
	Fixed radius;
	/*!the bbox center and radius are valid*/
	Bool is_set;
} GF_BBox;
/*!updates information of the bounding box based on the edge information*/
void gf_bbox_refresh(GF_BBox *b);
/*!builds a bounding box from a 2D rectangle*/
void gf_bbox_from_rect(GF_BBox *box, GF_Rect *rc);
/*!builds a rectangle from a 3D bounding box.\note The z dimension is lost and no projection is performed*/
void gf_rect_from_bbox(GF_Rect *rc, GF_BBox *box);
/*!\brief bounding box expansion
 *
 *Checks if a point is inside a bounding box and updates the bounding box to include it if not the case
 *\param box the bounding box object
 *\param pt the 3D point to check
*/
void gf_bbox_grow_point(GF_BBox *box, GF_Vec pt);
/*!performs the union of two bounding boxes*/
void gf_bbox_union(GF_BBox *b1, GF_BBox *b2);
/*!checks if two bounding boxes are equal or not*/
Bool gf_bbox_equal(GF_BBox *b1, GF_BBox *b2);
/*!checks if a point is inside a bounding box or not*/
Bool gf_bbox_point_inside(GF_BBox *box, GF_Vec *p);
/*!\brief get box vertices
 *
 *Returns the 8 bounding box vertices given the minimum and maximum edge. Vertices are ordered to respect 
 "p-vertex indexes", (vertex from a box closest to plane) and so that n-vertex (vertex from a box farthest from plane) 
 is 7-p_vx_idx
 *\param bmin minimum edge of the box
 *\param bmax maximum edge of the box
 *\param vecs list of 8 3D points used to store the vertices.
*/
void gf_bbox_get_vertices(GF_Vec bmin, GF_Vec bmax, GF_Vec *vecs);


/*!\brief matrix initialization
 *\hideinitializer
 *
 *Inits the matrix to the identity matrix
*/
#define gf_mx_init(_obj) { memset((_obj).m, 0, sizeof(Fixed)*16); (_obj).m[0] = (_obj).m[5] = (_obj).m[10] = (_obj).m[15] = FIX_ONE; }
/*!\brief matrix copy
 *\hideinitializer
 *
 *Copies the matrix _from to the matrix _obj
*/
#define gf_mx_copy(_obj, from) memcpy(&(_obj), &(from), sizeof(GF_Matrix));
/*!\brief matrix constructor from 2D
 *
 *Initializes a 3D matrix from a 2D matrix.\note all z-related coefficients will be set to default.
*/
void gf_mx_from_mx2d(GF_Matrix *mx, GF_Matrix2D *mat2D);
/*!\brief matrix identity testing
 *
 *Tests if two matrices are equal or not.
 \return 1 if matrices are same, 0 otherwise
*/
Bool gf_mx_equal(GF_Matrix *mx1, GF_Matrix *mx2);
/*!\brief matrix translation
 *
 *Translates a matrix 
 *\param mx the matrix being transformed. Once the function is called, contains the result matrix
 *\param tx horizontal translation
 *\param ty vertical translation
 *\param tz depth translation
*/
void gf_mx_add_translation(GF_Matrix *mx, Fixed tx, Fixed ty, Fixed tz);
/*!\brief matrix scaling
 *
 *Scales a matrix 
 *\param mx the matrix being transformed. Once the function is called, contains the result matrix
 *\param sx horizontal translation scaling
 *\param sy vertical translation scaling
 *\param sz depth translation scaling
*/
void gf_mx_add_scale(GF_Matrix *mx, Fixed sx, Fixed sy, Fixed sz);
/*!\brief matrix rotating
 *
 *Rotates a matrix 
 *\param mx the matrix being transformed. Once the function is called, contains the result matrix
 *\param angle rotation angle in radians
 *\param x horizontal coordinate of rotation axis
 *\param y vertical coordinate of rotation axis
 *\param z depth coordinate of rotation axis
*/
void gf_mx_add_rotation(GF_Matrix *mx, Fixed angle, Fixed x, Fixed y, Fixed z);
/*!\brief matrices multiplication 
 *
 *Multiplies a matrix with another one mx = mx*mul
 *\param mx the matrix being transformed. Once the function is called, contains the result matrix
 *\param mul the matrix to add
*/
void gf_mx_add_matrix(GF_Matrix *mx, GF_Matrix *mul);
/*!\brief 2D matrix multiplication
 *
 *Adds a 2D affine matrix to a matrix
 *\param mx the matrix 
 *\param mat2D the matrix to premultiply
 */
void gf_mx_add_matrix_2d(GF_Matrix *mx, GF_Matrix2D *mat2D);

/*!\brief affine matrix inversion
 *
 *Inverses an affine matrix.\warning Results are undefined if the matrix is not an affine one
 *\param mx the matrix to inverse
 */
void gf_mx_inverse(GF_Matrix *mx);
/*!\brief matrix point transformation
 *
 *Applies a 3D matrix transformation to a 3D point
 *\param mx transformation matrix
 *\param pt pointer to 3D point. Once the function is called, pt contains the transformed point
*/
void gf_mx_apply_vec(GF_Matrix *mx, GF_Vec *pt);
/*!\brief matrix rectangle transformation
 *
 *Applies a 3D matrix transformation to a rectangle, giving the enclosing rectangle of the transformed one.\note all depth information are discarded.
 *\param _this transformation matrix
 *\param rc pointer to rectangle. Once the function is called, rc contains the transformed rectangle
*/
void gf_mx_apply_rect(GF_Matrix *_this, GF_Rect *rc);
/*!\brief ortho matrix construction
 *
 *Creates an orthogonal projection matrix
 *\param mx matrix to initialize
 *\param left min horizontal coordinate of viewport
 *\param right max horizontal coordinate of viewport
 *\param bottom min vertical coordinate of viewport
 *\param top max vertical coordinate of viewport
 *\param z_near min depth coordinate of viewport
 *\param z_far max depth coordinate of viewport
*/
void gf_mx_ortho(GF_Matrix *mx, Fixed left, Fixed right, Fixed bottom, Fixed top, Fixed z_near, Fixed z_far);
/*!\brief perspective matrix construction
 *
 *Creates a perspective projection matrix
 *\param mx matrix to initialize
 *\param foc camera field of view angle in radian
 *\param aspect_ratio viewport aspect ratio
 *\param z_near min depth coordinate of viewport
 *\param z_far max depth coordinate of viewport
*/
void gf_mx_perspective(GF_Matrix *mx, Fixed foc, Fixed aspect_ratio, Fixed z_near, Fixed z_far);
/*!\brief creates look matrix
 *
 *Creates a transformation matrix looking at a given direction from a given point (camera matrix).
 *\param mx matrix to initialize
 *\param position position
 *\param target look direction
 *\param up_vector vector describing the up direction
*/
void gf_mx_lookat(GF_Matrix *mx, GF_Vec position, GF_Vec target, GF_Vec up_vector);
/*!\brief matrix box transformation
 *
 *Applies a 3D matrix transformation to a bounding box, giving the enclosing box of the transformed one
 *\param mx transformation matrix
 *\param b pointer to bounding box. Once the function is called, contains the transformed bounding box
*/
void gf_mx_apply_bbox(GF_Matrix *mx, GF_BBox *b);
/*!\brief matrix box sphere transformation
 *
 *Applies a 3D matrix transformation to a bounding box, computing only the enclosing sphere of the transformed one.
 *\param mx transformation matrix
 *\param b pointer to bounding box. Once the function is called, contains the transformed bounding sphere
*/
void gf_mx_apply_bbox_sphere(GF_Matrix *mx, GF_BBox *box);
/*!\brief non-affine matrix multiplication
 *
 *Multiplies two non-affine matrices mx = mx*mul
*/
void gf_mx_add_matrix_4x4(GF_Matrix *mat, GF_Matrix *mul);
/*!\brief non-affine matrix inversion
 *
 *Inverses a non-affine matrices
 *\return 1 if inversion was done, 0 if inversion not possible.
*/
Bool gf_mx_inverse_4x4(GF_Matrix *mx);
/*!\brief matrix 4D vector transformation
 *
 *Applies a 3D non-affine matrix transformation to a 4 dimension vector
 *\param mx transformation matrix
 *\param vec pointer to the vector. Once the function is called, contains the transformed vector
*/
void gf_mx_apply_vec_4x4(GF_Matrix *mx, GF_Vec4 *vec);
/*!\brief matrix decomposition
 *
 *Decomposes a matrix into translation, scale, shear and rotate
 *\param mx the matrix to decompose
 *\param translate the decomposed translation part
 *\param scale the decomposed scaling part
 *\param rotate the decomposed rotation part, expressed as a Rotataion (axis + angle)
 *\param shear the decomposed shear part
 */
void gf_mx_decompose(GF_Matrix *mx, GF_Vec *translate, GF_Vec *scale, GF_Vec4 *rotate, GF_Vec *shear);
/*!\brief matrix vector rotation
 *
 *Rotates a vector with a given matrix, ignoring any translation.
 *\param mx transformation matrix
 *\param pt pointer to 3D vector. Once the function is called, pt contains the transformed vector
 */
void gf_mx_rotate_vector(GF_Matrix *mx, GF_Vec *pt);
/*!\brief matrix initialization from vectors
 *
 *Inits a matrix to rotate the local axis in the given vectors
 \param mx matrix to initialize
 \param x_axis target normalized X axis
 \param y_axis target normalized Y axis
 \param z_axis target normalized Z axis
*/
void gf_mx_rotation_matrix_from_vectors(GF_Matrix *mx, GF_Vec x_axis, GF_Vec y_axis, GF_Vec z_axis);
/*!\brief matrix to 2D matrix 
 *
 *Inits a 2D matrix by removing all depth info from a 3D matrix
 *\param mx2d 2D matrix to initialize
 *\param mx 3D matrix to use
*/
void gf_mx2d_from_mx(GF_Matrix2D *mx2d, GF_Matrix *mx);

/*!\brief Plane object*/
typedef struct
{
	/*!normal vector to the plane*/
	GF_Vec normal;
	/*!distance from origin of the plane*/
	Fixed d;
} GF_Plane;
/*!\brief matrix plane transformation
 *
 *Transorms a plane by a given matrix
 *\param mx the matrix to use
 *\param plane pointer to 3D plane. Once the function is called, plane contains the transformed plane
 */
void gf_mx_apply_plane(GF_Matrix *mx, GF_Plane *plane);
/*!\brief point to plane distance
 *
 *Gets the distance between a point and a plne
 *\param plane the plane to use
 *\param p pointer to ^point to check
 *\return the distance between the place and the point
 */
Fixed gf_plane_get_distance(GF_Plane *plane, GF_Vec *p);
/*!\brief closest point on a line
 *
 *Gets the closest point on a line from a given point in space
 *\param line_pt a point of the line to test
 *\param line_vec the normalized direction vector of the line
 *\param pt the point to check
 *\return the closest point on the line to the desired point
 */
GF_Vec gf_closest_point_to_line(GF_Vec line_pt, GF_Vec line_vec, GF_Vec pt);
/*!\brief box p-vertex index
 *
 *Gets the p-vertex index for a given plane. The p-vertex index is the index of the closest vertex of a bounding box to the plane. The vertices of a box are always 
 *ordered in GPAC? cf \ref gf_bbox_get_vertices
 *\param p the plane to check
 *\return the p-vertex index value, ranging from 0 to 7
*/
u32 gf_plane_get_p_vertex_idx(GF_Plane *p);
/*!\brief plane line intersection
 *
 *Checks for the intersection of a plane and a line
 *\param plane plane to test
 *\param linepoint a point on the line to test
 *\param linevec normalized direction vector of the line to test
 *\param outPoint optional pointer to retrieve the intersection point, NULL otherwise
 *\return 1 if line and plane intersect, 0 otherwise
*/
Bool gf_plane_intersect_line(GF_Plane *plane, GF_Vec *linepoint, GF_Vec *linevec, GF_Vec *outPoint);

/*!Classification types for box/plane position used in \ref gf_bbox_plane_relation*/
enum 
{	
	/*!box is in front of the plane*/
	GF_BBOX_FRONT,
	/*!box intersects the plane*/
	GF_BBOX_INTER,
	/*!box is back of the plane*/
	GF_BBOX_BACK
};
/*!\brief box-plane relation
 *
 *Gets the spatial relation between a box and a plane
 *\param box the box to check
 *\param p the plane to check
 *\return the relation type
 */
u32 gf_bbox_plane_relation(GF_BBox *box, GF_Plane *p);

/*!\brief 3D Ray
 *
 *The 3D ray object is used in GPAC for all collision and mouse interaction tests
*/
typedef struct
{
	/*!origin point of the ray*/
	GF_Vec orig;
	/*!normalized direction vector of the ray*/
	GF_Vec dir;
} GF_Ray;

/*!\brief ray constructor
 *
 *Constructs a ray object
 *\param start starting point of the ray
 *\param end end point of the ray, or any point on the ray
 *\return the ray object
*/
GF_Ray gf_ray(GF_Vec start, GF_Vec end);
/*!\brief matrix ray transformation
 *
 *Transforms a ray by a given transformation matrix
 *\param mx the matrix to use
 *\param r pointer to the ray. Once the function is called, contains the transformed ray
*/
void gf_mx_apply_ray(GF_Matrix *mx, GF_Ray *r);
/*!\brief ray box intersection test
 *
 *Checks if a ray intersects a box or not
 *\param ray the ray to check
 *\param min_edge the minimum edge of the box to check
 *\param max_edge the maximum edge of the box to check
 *\param out_point optional location of a 3D point to store the intersection, NULL otherwise.
 *\return retuns 1 if the ray intersects the box, 0 otherwise
*/
Bool gf_ray_hit_box(GF_Ray *ray, GF_Vec min_edge, GF_Vec max_edge, GF_Vec *out_point);
/*!\brief ray sphere intersection test
 *
 *Checks if a ray intersects a box or not
 *\param ray the ray to check
 *\param center the center of the sphere to check. If NULL, the origin (0,0,0)is used
 *\param radius the radius of the sphere to check
 *\param out_point optional location of a 3D point to store the intersection, NULL otherwise
 *\return retuns 1 if the ray intersects the sphere, 0 otherwise
*/
Bool gf_ray_hit_sphere(GF_Ray *ray, GF_Vec *center, Fixed radius, GF_Vec *out_point);
/*!\brief ray triangle intersection test
 *
 *Checks if a ray intersects a triangle or not
 *\param ray the ray to check
 *\param v0 first vertex of the triangle
 *\param v1 second vertex of the triangle
 *\param v2 third vertex of the triangle
 *\param dist optional location of a fixed number to store the intersection distance from ray origin if any, NULL otherwise
 *\return retuns 1 if the ray intersects the triangle, 0 otherwise
*/
Bool gf_ray_hit_triangle(GF_Ray *ray, GF_Vec *v0, GF_Vec *v1, GF_Vec *v2, Fixed *dist);
/*same as above and performs backface cull (solid meshes)*/
/*!\brief ray triangle intersection test
 *
 *Checks if a ray intersects a triangle or not, performing backface culling. For parameters details, look at \ref gf_ray_hit_triangle_backcull
 */
Bool gf_ray_hit_triangle_backcull(GF_Ray *ray, GF_Vec *v0, GF_Vec *v1, GF_Vec *v2, Fixed *dist);

/*! @} */

/*! @} */

#ifdef __cplusplus
}
#endif


#endif		/*_GF_MATH_H_*/

