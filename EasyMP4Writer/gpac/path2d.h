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


#ifndef _GF_PATH2D_H_
#define _GF_PATH2D_H_

/*!
 *	\file <gpac/path2d.h>
 *	\brief 2D Vectorial Path functions.
 */



#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/math.h>
#include <gpac/constants.h>


/*!
 *\addtogroup path_grp path2d
 *\ingroup utils_grp
 *\brief Vectorial 2D Path manipulation functions
 *
 *This section documents the 2D path object used in the GPAC framework. 
 *	@{
 */

	
/*!\brief 2D Path Object
 *
 *The 2D path object is used to construct complex 2D shapes for later drawing
 * or outlining.
 */
typedef struct
{
	/*! number of contours in path*/
	u32 n_contours;
	/*! number of points in path and alloc size*/
	u32 n_points, n_alloc_points;
	/*! path points */
	GF_Point2D *points;
	/*! point tags (one per point)*/
	u8 *tags;
	/*! contour end points*/
	u32 *contours;
	/*! path bbox - NEVER USE WITHOUT FIRST CALLING \ref gf_path_get_bounds*/
	GF_Rect bbox;
	/*! path flags*/
	s32 flags;
	/*! fineness to use whenever flattening the path - default is \ref FIX_ONE*/
	Fixed fineness;
} GF_Path;


/*!
 *	\brief path constructor
 *
 *	Constructs an empty 2D path object
 *	\return new path object
 */
GF_Path *gf_path_new();
/*!
 *	\brief path destructor
 *
 *	Destructs a 2D path object
 *	\param gp the target path
 */
void gf_path_del(GF_Path *gp);
/*!
 *	\brief path reset
 *
 *	Resets the 2D path object
 *	\param gp the target path
 */
void gf_path_reset(GF_Path *gp);
/*!
 *	\brief path copy constuctor
 *
 *	Resets a copy of a 2D path object
 *	\param gp the target path
 *	\return new path copy
 */
GF_Path *gf_path_clone(GF_Path *gp);
/*!
 *	\brief path close
 *
 *	Closes current path contour
 *	\param gp the target path
 *	\return error code if any error, \ref GF_OK otherwise
 */
GF_Err gf_path_close(GF_Path *gp);
/*!
 *	\brief path moveTo
 *
 *	Starts a new contour from the specified point
 *	\param gp the target path
 *	\param x x-coordinate of the new point
 *	\param y y-coordinate of the new point
 *	\return error code if any error, \ref GF_OK otherwise
 */
GF_Err gf_path_add_move_to(GF_Path *gp, Fixed x, Fixed y);
/*!
 *	\brief starts new contour
 *
 *	Starts a new contour from the specified point
 *	\param gp the target path
 *	\param pt pointer to the new start point
 *	\return error code if any error, \ref GF_OK otherwise
 */
GF_Err gf_path_add_move_to_vec(GF_Path *gp, GF_Point2D *pt);
/*!
 *	\brief adds line to path
 *
 *	Adds a line from the current point in path to the specified point
 *	\param gp the target path
 *	\param x x-coordinate of the line end
 *	\param y y-coordinate of the line end
 *	\return error code if any error, \ref GF_OK otherwise
 */
GF_Err gf_path_add_line_to(GF_Path *gp, Fixed x, Fixed y);
/*!
 *	\brief adds line to path
 *
 *	Adds a line from the current point in path to the specified point
 *	\param gp the target path
 *	\param pt line end
 *	\return error code if any error, \ref GF_OK otherwise
 */
GF_Err gf_path_add_line_to_vec(GF_Path *gp, GF_Point2D *pt);
/*!
 *	\brief adds cubic to path
 *
 *	Adds a cubic bezier curve to the current contour, starting from the current path point
 *	\param gp the target path
 *	\param c1_x x-coordinate of the first control point of the cubic curve
 *	\param c1_y y-coordinate of the first control point of the cubic curve
 *	\param c2_x x-coordinate of the second control point of the cubic curve
 *	\param c2_y y-coordinate of the second control point of the cubic curve
 *	\param x x-coordinate of the end point of the cubic curve
 *	\param y y-coordinate of the end point of the cubic curve
 *	\return error code if any error, \ref GF_OK otherwise
 */
GF_Err gf_path_add_cubic_to(GF_Path *gp, Fixed c1_x, Fixed c1_y, Fixed c2_x, Fixed c2_y, Fixed x, Fixed y);
/*!
 *	\brief adds cubic to path
 *
 *	Adds a cubic bezier curve to the current contour, starting from the current path point
 *	\param gp the target path
 *	\param c1 first control point of the cubic curve
 *	\param c2 second control point of the cubic curve
 *	\param pt end point of the cubic curve
 *	\return error code if any error, \ref GF_OK otherwise
 */
GF_Err gf_path_add_cubic_to_vec(GF_Path *gp, GF_Point2D *c1, GF_Point2D *c2, GF_Point2D *pt);
/*!
 *	\brief adds quadratic to path
 *
 *	Adds a quadratic bezier curve to the current contour, starting from the current path point
 *	\param gp the target path
 *	\param c_x x-coordinate of the control point of the quadratic curve
 *	\param c_y y-coordinate of the control point of the quadratic curve
 *	\param x x-coordinate of the end point of the cubic quadratic
 *	\param y y-coordinate of the end point of the cubic quadratic
 *	\return error code if any error, \ref GF_OK otherwise
 */
GF_Err gf_path_add_quadratic_to(GF_Path *gp, Fixed c_x, Fixed c_y, Fixed x, Fixed y);
/*!
 *	\brief adds quadratic to path
 *
 *	Adds a quadratic bezier curve to the current contour, starting from the current path point
 *	\param gp the target path
 *	\param c control point of the quadratic curve
 *	\param pt end point of the cubic quadratic
 *	\return error code if any error, \ref GF_OK otherwise
 */
GF_Err gf_path_add_quadratic_to_vec(GF_Path *gp, GF_Point2D *c, GF_Point2D *pt);
/*!
 *	\brief adds rectangle to path
 *
 *	Adds a rectangle contour to the path
 *	\param gp the target path
 *	\param cx x-coordinate of the rectangle center
 *	\param cy y-coordinate of the rectangle center
 *	\param w width of the rectangle
 *	\param h height of the rectangle
 *	\return error code if any error, \ref GF_OK otherwise
 */
GF_Err gf_path_add_rect_center(GF_Path *gp, Fixed cx, Fixed cy, Fixed w, Fixed h);
/*!
 *	\brief adds rectangle to path
 *
 *	Adds a rectangle contour to the path
 *	\param gp the target path
 *	\param ox left-most coordinate of the rectangle 
 *	\param oy top-most coordinate of the rectangle
 *	\param w width of the rectangle
 *	\param h height of the rectangle
 *	\return error code if any error, \ref GF_OK otherwise
 */
GF_Err gf_path_add_rect(GF_Path *gp, Fixed ox, Fixed oy, Fixed w, Fixed h);
/*!
 *	\brief adds ellipse to path
 *
 *	Adds an ellipse contour to the path
 *	\param gp the target path
 *	\param cx x-coordinate of the ellipse center
 *	\param cy y-coordinate of the ellipse center
 *	\param a_axis length of the horizontal ellipse axis
 *	\param b_axis length of the vertical ellipse axis
 *	\return error code if any error, \ref GF_OK otherwise
 */
GF_Err gf_path_add_ellipse(GF_Path *gp, Fixed cx, Fixed cy, Fixed a_axis, Fixed b_axis);
/*!
 *	\brief adds N-bezier curve to path
 *
 *	Adds an N-degree bezier curve to the path, starting from the current point
 *	\param gp the target path
 *	\param pts points used to define the curve
 *	\param nb_pts number of points used to define the curve. The degree of the curve is therefore (nb_pts-1).
 *	\return error code if any error, \ref GF_OK otherwise
 *	\note the fineness of the path must be set before calling this function.
 */
GF_Err gf_path_add_bezier(GF_Path *gp, GF_Point2D *pts, u32 nb_pts);
/*!
 *	\brief adds arc as described in MPEG-4 BIFS to path
 *
 *	Adds an arc contour to the path from focal and end points.
 *	\param gp the target path
 *	\param end_x x-coordinate of the arc end point
 *	\param end_y y-coordinate of the arc end point
 *	\param fa_x x-coordinate of the arc first focal point
 *	\param fa_y y-coordinate of the arc first focal point
 *	\param fb_x x-coordinate of the arc second focal point
 *	\param fb_y y-coordinate of the arc second focal point
 *	\param cw if 1, the arc will be clockwise, otherwise counter-clockwise.
 *	\return error code if any error, \ref GF_OK otherwise
 */
GF_Err gf_path_add_arc_to(GF_Path *gp, Fixed end_x, Fixed end_y, Fixed fa_x, Fixed fa_y, Fixed fb_x, Fixed fb_y, Bool cw);
/*!
 *	\brief adds arc as described in SVG to path
 *
 *	Adds an arc contour to the path from end point, radii and 3 parameters.
 *	\param gp the target path
 *	\param end_x x-coordinate of the arc end point
 *	\param end_y y-coordinate of the arc end point
 *	\param r_x x-axis radius
 *	\param r_y y-axis radius 
 *	\param x_axis_rotation angle for the x-axis
 *	\param large_arc_flag large or short arc selection
 *	\param sweep_flag if 1, the arc will be clockwise, otherwise counter-clockwise.
 *	\return error code if any error, \ref GF_OK otherwise
 */
GF_Err gf_path_add_svg_arc_to(GF_Path *gp, Fixed end_x, Fixed end_y, Fixed r_x, Fixed r_y, Fixed x_axis_rotation, Bool large_arc_flag, Bool sweep_flag);
/*!
 *	\brief adds arc to path
 *
 *	Adds an arc contour to the path.
 *	\param gp the target path
 *	\param radius radius of the arc 
 *	\param start_angle start angle of the arc in radians
 *	\param end_angle end angle of the arc in radians
 *	\param close_type closing type: 0 for open arc, 1 for close arc, 2 for pie
 *	\return error code if any error, \ref GF_OK otherwise
 */
GF_Err gf_path_add_arc(GF_Path *gp, Fixed radius, Fixed start_angle, Fixed end_angle, u32 close_type);

/*!
 *	\brief concatenates path
 *
 *	Adds a sub-path to the path with a given transform.
 *	\param gp the target path
 *	\param subpath the path to add
 *	\param mat Matrix for subpath 
 *	\return error code if any error, \ref GF_OK otherwise
 */
GF_Err gf_path_add_subpath(GF_Path *gp, GF_Path *subpath, GF_Matrix2D *mx);
/*!
 *	\brief gets path control bounds
 *
 *	Gets the path control bounds, i.e. the rectangle covering all lineTo and bezier control points.
 *	\param gp the target path
 *	\param rc pointer to rectangle receiving the control rectangle
 *	\return error code if any error, \ref GF_OK otherwise
 */
GF_Err gf_path_get_control_bounds(GF_Path *gp, GF_Rect *rc);
/*!
 *	\brief gets path bounds
 *
 *	Gets the path bounds, i.e. the rectangle covering all points in path except bezier control points.
 *	\param gp the target path
 *	\param rc pointer to rectangle receiving the control rectangle
 *	\return error code if any error, \ref GF_OK otherwise
 */
GF_Err gf_path_get_bounds(GF_Path *gp, GF_Rect *rc);
/*!
 *	\brief flattens path 
 *
 *	Flattens the path, i.e. transform all bezier curves to lines according to the path flatness.
 *	\param gp the target path
 */
void gf_path_flatten(GF_Path *gp);
/*!
 *	\brief gets flatten copy of path 
 *
 *	Gets a flatten copy of the path.
 *	\param gp the target path
 *	\return the flatten path
 */
GF_Path *gf_path_get_flatten(GF_Path *gp);
/*!
 *	\brief point over path testing
 *
 *	Tests if a point is over a path or not, according to the path filling rule.
 *	\param gp the target path
 *	\param x x-coordinate of the point to check
 *	\param y y-coordinate of the point to check
 *	\return 1 if the point is over the path, 0 otherwise.
 */
Bool gf_path_point_over(GF_Path *gp, Fixed x, Fixed y);

/*!
 *	\brief path init testing
 *
 *	Tests if the path is empty or not.
 *	\param gp the target path
 *	\return 1 if the path is empty, 0 otherwise.
 */
Bool gf_path_is_empty(GF_Path *gp);

/*!
 *	\brief path iterator 
 *
 *	The path iterator object is used to compute the length of a given path as well
 * as transformation matrices along this path.
 */
typedef struct _path_iterator GF_PathIterator;

/*!
 *	\brief path iterator constructor
 *
 *	Creates a new path iterator from a given path
 *	\param gp the target path
 *	\return the path iterator object.
 */
GF_PathIterator *gf_path_iterator_new(GF_Path *gp);
/*!
 *	\brief path iterator destructor
 *
 *	Destructs the path iterator object
 *	\param it the target path iterator
 */
void gf_path_iterator_del(GF_PathIterator *it);

/*!
 *	\brief get path length
 *
 *	Gets a path length from its iterator
 *	\param it the target path iterator
 *	\return the length of the path
 */
Fixed gf_path_iterator_get_length(GF_PathIterator *it);
/*!
 *\brief gets transformation matrix at given point on path
 *
 * Gets the transformation of a given point on the path, given by offset from origin. 
 *The transform is so that a local system is translated to the given point, its x-axis tangent 
 *to the path and in the same direction. The path direction is from first point to last point
 *of the path.
 *	\param it the target path iterator
 *	\param offset length on the path in local system unit
 *	\param follow_tangent indicates if transformation shall be computed if offset indicates a point outside the path (<0 or >path_length). In which case the path shall be virtually extended by the tangent at origin (offset <0) or at end (offset>path_length). Otherwise the transformation is not computed and 0 is returned.
 *	\param mat matrix to be transformed (transformation shall be appended) - the matrix shall not be initialized
 *	\param smooth_edges indicates if discontinuities shall be smoothed. If not set, the rotation angle THETA is the slope (DX/DY) of the current segment found.
 *	\param length_after_point if set and smooth_edges is set, the amount of the object that lies on next segment shall be computed according to length_after_point. 
 \code
  Let:
	len_last: length of current checked segment
	len1: length of all previous segments so that len1 + len_last >= offset then if (offset + length_after_point > len1 + len_last) {
	ratio = (len1 + len_last - offset) / length_after_point;
	then THETA = ratio * slope(L1) + (1-ratio) * slope(L2)

  Of course care must be taken for PI/2 angles and similar situations 
 \endcode

 *	\return 1 if matrix has been updated, 0 otherwise, if failure or if point is out of path without tangent extension.
 */
Bool gf_path_iterator_get_transform(GF_PathIterator *it, Fixed offset, Bool follow_tangent, GF_Matrix2D *mat, Bool smooth_edges, Fixed length_after_point);



/*! brief gets convexity type for a 2D polygon
 *
 * Gets the convexity type of the given 2D polygon
 * \param pts the points of the polygon
 * \param nb_pts number of points in the polygon
 * \return the convexity type of the polygon
*/
u32 gf_polygone2d_get_convexity(GF_Point2D *pts, u32 nb_pts);


/* 2D Path constants */

/*!
 *2D Path point tags
 *	\hideinitializer
 */
enum
{
	/*/! Point is on curve (moveTo, lineTo, end of splines)*/
	GF_PATH_CURVE_ON = 1,
	/*! Point is a contour close*/
	GF_PATH_CLOSE	= 5,
	/*! Point is a quadratic control point*/
	GF_PATH_CURVE_CONIC = 0,
	/*! Point is a cubic control point*/
	GF_PATH_CURVE_CUBIC = 2,
};


/*!
 *2D Path flags
 *	\hideinitializer
 */
enum
{
	/*! Path is filled using the zero-nonzero rule. If not set, filling uses odd/even rule*/
	GF_PATH_FILL_ZERO_NONZERO = 1,
	/*! When set bbox must be recomputed. 
	\note Read only, used to avoid wasting time on bounds calculation*/
	GF_PATH_BBOX_DIRTY = 2,
	/*! Indicates the path is flattened flattened
	\note Read only, used to avoid wasting time on flattening*/
	GF_PATH_FLATTENED = 4,
};

/*!
 * 2D Polygon convexity type
 *	\hideinitializer
 */
enum
{
	/*! Polygon is either complex or unknown*/
	GF_POLYGON_COMPLEX,
	/*! Polygon is complex, starting in counter-clockwise order*/
	GF_POLYGON_COMPLEX_CCW,
	/*! Polygon is complex, starting in clockwise order*/
	GF_POLYGON_COMPLEX_CW,
	/*! Polygon is a counter-clockwise convex polygon*/
	GF_POLYGON_CONVEX_CCW,
	/*! Polygon is a clockwise convex polygon*/
	GF_POLYGON_CONVEX_CW,
	/*! Polygon is a convex line (degenerated path with all segments aligned)*/
	GF_POLYGON_CONVEX_LINE
};

/*!
 * Stencil alignment type for outlining
 *	\hideinitializer
 */
enum
{	
	/*! outline is centered on the path (default)*/
	GF_PATH_LINE_CENTER = 0,
	/*! outline is inside the path*/
	GF_PATH_LINE_INSIDE,
	/*! outline is outside the path*/
	GF_PATH_LINE_OUTSIDE,
};

/*!
 * Line cap type for outlining
 *	\hideinitializer
 */
enum
{
	/*! End of line is flat (default)*/
	GF_LINE_CAP_FLAT = 0,
	/*! End of line is round*/
	GF_LINE_CAP_ROUND,
	/*! End of line is square*/
	GF_LINE_CAP_SQUARE,
	/*! End of line is triangle*/
	GF_LINE_CAP_TRIANGLE,
};

/*!
 * Line join type for outlining
 *	\hideinitializer
 */
enum
{
	/*! Line join is a miter join (default)*/
	GF_LINE_JOIN_MITER = 0,
	/*! Line join is a round join*/
	GF_LINE_JOIN_ROUND,
	/*! Line join is a bevel join*/
	GF_LINE_JOIN_BEVEL,
	/*! Line join is a miter then bevel join*/
	GF_LINE_JOIN_MITER_SVG
};

/*!
 * Dash types for outlining
 *	\hideinitializer
 */
enum
{
	/*! No dashing is used (default)*/
	GF_DASH_STYLE_PLAIN = 0,
	/*! Predefined dash pattern is used*/
	GF_DASH_STYLE_DASH,
	/*! Predefined dot pattern is used*/
	GF_DASH_STYLE_DOT,
	/*! Predefined dash-dot pattern is used*/
	GF_DASH_STYLE_DASH_DOT,
	/*! Predefined dash-dash-dot pattern is used*/
	GF_DASH_STYLE_DASH_DASH_DOT,
	/*! Predefined dash-dot-dot pattern is used*/
	GF_DASH_STYLE_DASH_DOT_DOT,
	/*! Custom pattern is used. Dash lengths are given in percentage of the pen width*/
	GF_DASH_STYLE_CUSTOM,
	/*! SVG pattern is used. Dash lengths are given in the same unit as the pen width
	and dash offset follows SVG specs (offset in dash pattern)*/
	GF_DASH_STYLE_SVG,
};


/*!\brief Custom dash pattern
 *
 *The custom dash pattern object is used to specify custom dashes when outlining a path.
 */
typedef struct
{
	/*begining of the structure is casted in MFFloat in BIFS, DO NOT CHANGE ORDER*/

	/*! Number of dashes in the pattern*/
	u32 num_dash;
	/*! Value of the pattern dashes. Unit depends on the dash type*/
	Fixed *dashes;
    /*! SVG/CSS unit for the dashes */
    u8 *dash_units;
} GF_DashSettings;

/*!\brief Pen properties 
 *
 *The pen properties object is used to specify several parameters used when building
 *the vectorial outline of a path.
 */
typedef struct
{
	/*! The width of the outline*/
	Fixed width;
	/*! The style of the lines ends*/
	u8 cap;
	/*! The style of the lines joins*/
	u8 join;
	/*! The alignment of the outline with regard to the path*/
	u8 align;
	/*! The dash style of the line*/
	u8 dash;
	/*! The miter limit of the line joins*/
	Fixed miterLimit;
	/*! The initial dash offset in the outline. All points before this offset will be 
	* ignored when building the outline*/
	Fixed dash_offset;
	/*! The dash pattern used for curstom dashing*/
	GF_DashSettings *dash_set;
	/*! The author-specified path length. Ignored if <= 0*/
	Fixed path_length;
} GF_PenSettings;

/*! brief builds the vectorial outline of a path
 *
 * Builds the vectorial outline of a path for the given settings. The outline of a path is a path.
 * \param path the desired path to outline
 * \param pen the properties of the virtual pen used for outlining
 * \return the outline of the path 
*/
GF_Path *gf_path_get_outline(GF_Path *path, GF_PenSettings pen);


/*! @} */

#ifdef __cplusplus
}
#endif


#endif	/*_GF_PATH2D_H_*/

