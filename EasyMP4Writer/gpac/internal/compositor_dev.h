/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005 
 *					All rights reserved
 *
 *  This file is part of GPAC / Scene Rendering sub-project
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

#ifndef _COMPOSITOR_DEV_H_
#define _COMPOSITOR_DEV_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/compositor.h>
/*include scene graph API*/
#include <gpac/thread.h>
/*bridge between the rendering engine and the systems media engine*/
#include <gpac/mediaobject.h>

/*raster2D API*/
#include <gpac/modules/raster2d.h>
/*font engine API*/
#include <gpac/modules/font.h>
/*AV hardware API*/
#include <gpac/modules/video_out.h>
#include <gpac/modules/audio_out.h>

/*SVG properties*/
#ifndef GPAC_DISABLE_SVG
#include <gpac/scenegraph_svg.h>
#endif


/*if defined, events are queued before being processed, otherwise they are handled whenever triggered*/
//#define GF_SR_EVENT_QUEUE	


/*use 2D caching for groups*/
//#define GF_SR_USE_VIDEO_CACHE

//#define GPAC_USE_TINYGL

/*depth-enabled version for autostereoscopic displays */
#define GF_SR_USE_DEPTH

/*FPS computed on this number of frame*/
#define GF_SR_FPS_COMPUTE_SIZE	30



enum
{
	GF_SR_CFG_OVERRIDE_SIZE = 1,
	GF_SR_CFG_SET_SIZE = 1<<1,
	GF_SR_CFG_AR = 1<<2,
	GF_SR_CFG_FULLSCREEN = 1<<3,
	/*flag is set whenever we're reconfiguring visual. This will discard all UI
	messages during this phase in order to avoid any deadlocks*/
	GF_SR_IN_RECONFIG = 1<<4,
	/*special flag indicating the set size is actually due to a notif by the plugin*/
	GF_SR_CFG_WINDOWSIZE_NOTIF = 1<<10,
};



/*forward definition of the visual manager*/
typedef struct _visual_manager GF_VisualManager;
typedef struct _draw_aspect_2d DrawAspect2D;
typedef struct _traversing_state GF_TraverseState;
typedef struct _gf_ft_mgr GF_FontManager;

#ifndef GPAC_DISABLE_3D
#include <gpac/internal/camera.h>
#include <gpac/internal/mesh.h>

typedef struct 
{
	Bool multisample;
	Bool bgra_texture;
	Bool abgr_texture;
	Bool npot_texture;
	Bool rect_texture;
	Bool point_sprite;
	Bool vbo;
	u32 yuv_texture;
} GLCaps;

#endif

#if !defined(GPAC_DISABLE_3D) && !defined(GPAC_USE_TINYGL)
# define OPENGL_RASTER
#else
# ifdef OPENGL_RASTER
# undef OPENGL_RASTER
# endif
#endif

#define DOUBLECLICK_TIME_MS		250


enum
{
	/*no text selection*/
	GF_SC_TSEL_NONE = 0,
	/*text selection in progress*/
	GF_SC_TSEL_ACTIVE,
	/*text selection frozen*/
	GF_SC_TSEL_FROZEN,
	/*text selection has just been released*/
	GF_SC_TSEL_RELEASED,
};

enum
{
	GF_SC_DRAW_NONE,
	GF_SC_DRAW_FRAME,
	GF_SC_DRAW_FLUSH,
};

enum
{
	GF_SC_DEPTH_GL_NONE=0,
	GF_SC_DEPTH_GL_POINTS,
	GF_SC_DEPTH_GL_STRIPS,
	GF_SC_DEPTH_GL_VBO,
};

struct __tag_compositor
{
	/*the main user*/
	GF_User *user;
	/*terminal - only used for InputSensor*/
	GF_Terminal *term;

	/*audio renderer*/
	struct _audio_render *audio_renderer;
	/*video out*/
	GF_VideoOutput *video_out;
	/*2D rasterizer*/
	GF_Raster2D *rasterizer;

	/*all textures (texture handlers)*/
	GF_List *video_listeners;

	/*visual rendering thread if used*/
	GF_Thread *VisualThread;
	/*0: not init, 1: running, 2: exit requested, 3: done*/
	u32 video_th_state;

	/*compositor exclusive access to the scene and display*/
	GF_Mutex *mx;

	/*the main scene graph*/
	GF_SceneGraph *scene;
	/*extra scene graphs (OSD, etc), always registered in draw order. That's the module responsability
	to draw them*/
	GF_List *extra_scenes;

	u32 inherit_type_3d;
	
	/*all time nodes registered*/
	GF_List *time_nodes;
	/*all textures (texture handlers)*/
	GF_List *textures;

#ifdef GF_SR_EVENT_QUEUE
	/*event queue*/
	GF_List *events;
	GF_Mutex *ev_mx;
#endif

	/*simulation frame rate*/
	Double frame_rate;
	Bool no_regulation;
	u32 frame_duration;
	u32 frame_time[GF_SR_FPS_COMPUTE_SIZE];
	u32 current_frame;
	u32 last_frame_time, caret_next_draw_time;
	Bool show_caret;
	Bool text_edit_changed;

	u32 last_click_time;

	/*display size*/
	u32 display_width, display_height;

	/*visual output location on window (we may draw background color outside of it) 
		vp_x & vp_y: horizontal & vertical offset of the drawing area in the video output
		vp_width & vp_height: width & height of the drawing area
			* in scalable mode, this is the display size
			* in not scalable mode, this is the final drawing area size (dst_w & dst_h of the blit)
	*/
	u32 vp_x, vp_y, vp_width, vp_height;
	/*backbuffer size - in scalable mode, matches display size, otherwise matches scene size*/
	u32 output_width, output_height;

	/*scene size if any*/
	u32 scene_width, scene_height;
	Bool has_size_info;
	Bool fullscreen;
	/*!! paused will not stop display (this enables pausing a VRML world and still examining it)*/
	Bool paused, step_mode;
	u32 frame_draw_type;
	u32 force_next_frame_redraw;
	/*freeze_display prevents any screen updates - needed when output driver uses direct video memory access*/
	Bool is_hidden, freeze_display;
	Bool debug_defer;

	Bool disable_composite_blit, disable_hardware_blit, rebuild_offscreen_textures;

	/*current frame number*/
	u32 frame_number;
	/*count number of initialized sensors*/
	u32 interaction_sensors;

	/*set whenever 3D HW ctx changes (need to rebuild dlists/textures if any used)*/
	Bool reset_graphics;

	/*font engine*/
	GF_FontManager *font_manager;
	/*set whenever a new font has been received*/
	Bool reset_fonts;
	s32 fonts_pending;

	/*options*/
	u32 aspect_ratio, antiAlias, texture_text_mode;
	Bool high_speed, stress_mode;
	Bool force_opengl_2d;
#ifdef OPENGL_RASTER
	Bool opengl_raster;
#endif

	/*key modif*/
	u32 key_states;
	u32 interaction_level;

	/*size override when no size info is present
		flags:	1: size override is requested (cfg)
				2: size override has been applied
	*/
	u32 override_size_flags;

	/*any of the above flags - reseted at each simulation tick*/
	u32 msg_type;
	/*for size*/
	u32 new_width, new_height;

	/*current background color*/
	u32 back_color;

	/*bounding box draw type: none, unit box/rect and sphere (3D only)*/
	u32 draw_bvol;

	/*list of system colors*/
	u32 sys_colors[28];

	/*all visual managers created*/
	GF_List *visuals;
	/*all outlines cached*/
	GF_List *strike_bank;

	/*main visual manager - the one managing the primary video output*/
	GF_VisualManager *visual;
	/*set to false whenever a new scene is attached to compositor*/
	Bool root_visual_setup;
	
	/*indicates whether the aspect ratio shall be recomputed:
		1: AR changed
		2: AR changed and root visual type changed between 2D and 3D
	*/
	u32 recompute_ar;

	Bool zoom_changed;

	/*traversing context*/
	struct _traversing_state *traverse_state;

	/*current picked node if any*/
	GF_Node *grab_node;
	/*current picked node's parent use if any*/
	GF_Node *grab_use;
	/*current focus node if any*/
	GF_Node *focus_node;
	/*parent use node of the current focus node if any*/
	GF_Node *focus_used;
	/*current parent focus node if any - needed to navigate within PROTOs*/
	GF_List *focus_ancestors;
	GF_List *focus_use_stack;
	/*focus node uses dom events*/
	Bool focus_uses_dom_events;
	/*current sensor type*/
	u32 sensor_type;
	/*list of VRML sensors active before the picking phase (eg active at the previous pass)*/
	GF_List *previous_sensors;
	/*list of VRML sensors active after the picking phase*/
	GF_List *sensors;
	/*indicates a sensor is currently active*/
	u32 grabbed_sensor;

	/*current keynav node if any*/
	GF_Node *keynav_node;

	/*current keynav node if any*/
	GF_List *env_tests;

	/*hardware handle for 2D screen access - currently only used with win32 (HDC) */
	void *hw_context;
	/*indicates whether HW is locked*/
	Bool hw_locked;
	/*screen buffer for direct access*/
	GF_VideoSurface hw_surface;
	/*output buffer is configured in video memory*/
	Bool video_memory, request_video_memory;
	/*indicate if overlays were prezsent in the previous frame*/
	Bool last_had_overlays;
	GF_RasterCallback raster_callbacks;

	/*options*/
	Bool scalable_zoom;
	Bool enable_yuv_hw;
	/*disables partial hardware blit (eg during dirty rect) to avoid artefacts*/
	Bool disable_partial_hw_blit;

	/*user navigation mode*/
	u32 navigate_mode;
	/*set if content doesn't allow navigation*/
	Bool navigation_disabled;

	u32 rotate_mode;

	/*user mouse navigation state:
	 0: not active
	 1: pre-active phase: mouse has been clicked and waiting for mouse move to confirm. This allows
		for clicking on objects in the navigation mode
	 2: navigation is grabbed
	*/
	u32 navigation_state;
	/*navigation x & y grab point in scene coord system*/
	Fixed grab_x, grab_y;
	/*aspect ratio scale factor*/
	Fixed scale_x, scale_y;
	/*user zoom level*/
	Fixed zoom;
	/*user pan*/
	Fixed trans_x, trans_y;
	/*user rotation angle - ALWAYS CENTERED*/
	Fixed rotation;

	Bool skip_flush;
#ifndef GPAC_DISABLE_SVG
	u32 num_clicks;
#endif

	/*a dedicated drawable for focus highlight */
	struct _drawable *focus_highlight;
	/*highlight fill and stroke colors (ARGB)*/
	u32 highlight_fill, highlight_stroke;
	Fixed highlight_stroke_width;

	/*picking info*/

	/*picked node*/
	GF_Node *hit_node;
	/*appearance at hit point - used for composite texture*/
	GF_Node *hit_appear, *prev_hit_appear;
	/*parent use stack - SVG only*/
	GF_List *hit_use_stack, *prev_hit_use_stack;
	/*picked node uses DOM event or VRML events ?*/
	Bool hit_use_dom_events;

	/*world->local and local->world transform at hit point*/
	GF_Matrix hit_world_to_local, hit_local_to_world;
	/*hit point in local coord & world coord*/
	SFVec3f hit_local_point, hit_world_point;
	/*tex coords at hit point*/
	SFVec2f hit_texcoords;
	/*picking ray in world coord system*/
	GF_Ray hit_world_ray;
	/*normal at hit point, local coord system*/
	SFVec3f hit_normal;
	/*distance from ray origin used to discards further hits - FIXME: may not properly work with transparent layer3D*/
	Fixed hit_square_dist;

	/*text selection and edition*/

	/*the active parent text node under selection*/
	GF_Node *text_selection;
	/*text selection start/end in world coord system*/
	SFVec2f start_sel, end_sel;	
	/*text selection state*/ 
	u32 store_text_state;
	/*parent text node when a text is hit (to handle tspan selection)*/
	GF_Node *hit_text;
	u32 sel_buffer_len, sel_buffer_alloc;
	u16 *sel_buffer;
	u8 *selected_text;
	/*text selection color - reverse video not yet supported*/
	u32 text_sel_color;
	s32 picked_glyph_idx, picked_span_idx;

	/*set whenever the focus node is a text node*/
	u32 focus_text_type;
	Bool edit_is_tspan;
	/*pointer to edited text*/
	char **edited_text;
	u32 caret_pos, dom_text_pos;

#ifndef GPAC_DISABLE_3D
	/*options*/
	/*emulate power-of-2 for video texturing by using a po2 texture and texture scaling. If any textureTransform
	is applied to this texture, black stripes will appear on the borders.
	If not set video is written through glDrawPixels with bitmap (slow scaling), or converted to
	po2 texture*/
	Bool emul_pow2;
	/*use openGL for outline rather than vectorial ones*/
	Bool raster_outlines;
	/*disable RECT extensions (except for Bitmap...)*/
	Bool disable_rect_ext;
	/*disable RECT extensions (except for Bitmap...)*/
	Bool bitmap_use_pixels;
	/*disable RECT extensions (except for Bitmap...)*/
	u32 draw_normals;
	/*backface cull type: 0 off, 1: on, 2: on with transparency*/
	u32 backcull;
	/*polygon atialiasing*/
	Bool poly_aa;
	/*disable gluScaleImage*/
	Bool disable_glu_scale;
	/*wireframe/solid mode*/
	u32 wiremode;
	/*collision detection mode*/
	u32 collide_mode;
	/*gravity enabled*/
	Bool gravity_on;
	/*AABB tree-based culling is disabled*/
	Bool disable_gl_cull;
	/*YUV textures in OpenGL are disabled (soft YUV->RGB )*/
	Bool disable_yuvgl;

	u32 default_navigation_mode;

	/*unit box (1.0 size) and unit sphere (1.0 radius)*/
	GF_Mesh *unit_bbox;

	/*active layer3D for layer navigation - may be NULL*/
	GF_Node *active_layer;

	GLCaps gl_caps;

	u32 offscreen_width, offscreen_height;

#ifdef GPAC_USE_TINYGL
	void *tgl_ctx;
#endif

	Fixed depth_gl_scale, depth_gl_strips_filter;
	u32 depth_gl_type;
	/*increase/decrease the standard interoccular offset by the specified distance in cm*/
	Fixed interoccular_offset;
	/*increase/decrease the view distance by the specified distance in cm*/
	Fixed view_distance_offset;

#endif
	
	u32 networks_time;
	u32 decoders_time;

	u32 visual_config_time;
	u32 traverse_setup_time;
	u32 traverse_and_direct_draw_time;
	u32 indirect_draw_time;

#ifdef GF_SR_USE_VIDEO_CACHE
	/*video cache size / max size in kbytes*/
	u32 video_cache_current_size, video_cache_max_size;
	u32 cache_scale, cache_tolerance;
	/*sorted list (by cache priority) of cached groups - permanent for the lifetime of the scene/cache object*/
	GF_List *cached_groups;
	/*list of groups being cached in one frame */
	GF_List *cached_groups_queue;
#endif

#ifdef GF_SR_USE_DEPTH
	Bool auto_calibration;
	/*display depth in pixels - if -1, it is the height of the display area*/
	s32 display_depth;
#endif
};


/*base stack for timed nodes (nodes that activate themselves at given times)
	@UpdateTimeNode: shall be setup by the node handler and is called once per simulation frame
	@is_registerd: all handlers indicate store their state if wanted (provided for conveniency but not inspected by the compositor)
	@needs_unregister: all handlers indicate they can be removed from the list of active time nodes
in order to save time. THIS IS INSPECTED by the compositor at each simulation tick after calling UpdateTimeNode
and if set, the node is removed right away from the list
*/
typedef struct _time_node
{
	void (*UpdateTimeNode)(struct _time_node *);
	Bool is_registered, needs_unregister;
	/*user data*/
	void *udta;
} GF_TimeNode;

void gf_sc_register_time_node(GF_Compositor *sr, GF_TimeNode *tn);
void gf_sc_unregister_time_node(GF_Compositor *sr, GF_TimeNode *tn);

enum
{
	/*texture repeat along s*/
	GF_SR_TEXTURE_REPEAT_S = (1<<0),
	/*texture repeat along t*/
	GF_SR_TEXTURE_REPEAT_T = (1<<1),
	/*texture is a matte texture*/
	GF_SR_TEXTURE_MATTE = (1<<2),
	/*texture doesn't need vertical flip for OpenGL*/
	GF_SR_TEXTURE_NO_GL_FLIP = (1<<3),
	/*Set durin a composition cycle. If not set at the end of the cycle, 
	the hardware binding is released*/
	GF_SR_TEXTURE_USED = (1<<4),

	/*texture is SVG (needs special treatment in OpenGL)*/
	GF_SR_TEXTURE_SVG = (1<<5),

	/*special flag indicating the underlying media directly handled by the hardware (decoding and composition)*/
	GF_SR_TEXTURE_PRIVATE_MEDIA = (1<<6),
};

typedef struct _gf_sc_texture_handler
{
	GF_Node *owner;
	GF_Compositor *compositor;
	/*low-level texture object for internal rasterizer and OpenGL - this is not exposed out of libgpac*/
	struct __texture_wrapper *tx_io;
	/*media stream*/
	GF_MediaObject *stream;
	/*texture is open (for DEF/USE)*/
	Bool is_open;
	/*this is needed in case the Url is changed*/
//	MFURL current_url;
	/*to override by each texture node*/
	void (*update_texture_fcnt)(struct _gf_sc_texture_handler *txh);
	/*needs_release if a visual frame is grabbed (not used by modules)*/
	Bool needs_release;
	/*stream_finished: indicates stream is over (not used by modules)*/
	Bool stream_finished;
	/*needs_refresh: indicates texture content has been changed - needed by modules performing tile drawing*/
	Bool needs_refresh;
	/*needed to discard same frame fetch*/
	u32 last_frame_time;
	/*active display in the texture (0, 0 == top, left)*/
	//GF_Rect active_window;
	/*texture is transparent*/		
	Bool transparent;
	/*flags for user - the repeatS and repeatT are set upon creation, the rest is NEVER touched by compositor*/
	u32 flags;
	/*gradients are relative to the object bounds, therefore a gradient is not the same if used on 2 different
	objects - since we don't want to build an offscreen texture for the gradient, gradients have to be updated 
	at each draw - the matrix shall be updated to the gradient transformation in the local system
	MUST be set for gradient textures*/
	void (*compute_gradient_matrix)(struct _gf_sc_texture_handler *txh, GF_Rect *bounds, GF_Matrix2D *mat, Bool for_3d);

	/*image data for natural media*/
	char *data;
	u32 width, height, stride, pixelformat, pixel_ar;

#ifndef GPAC_DISABLE_VRML
	/*if set texture has been transformed by MatteTexture -> disable blit*/
	Bool has_cmat;

	/*matteTexture parent if any*/
	GF_Node *matteTexture;
#endif

	/*user data for video output module, if needed*/
	void *vout_udta;
} GF_TextureHandler;

/*setup texturing object*/
void gf_sc_texture_setup(GF_TextureHandler *hdl, GF_Compositor *sr, GF_Node *owner);
/*destroy texturing object*/
void gf_sc_texture_destroy(GF_TextureHandler *txh);

/*return texture handle for built-in textures (movieTexture, ImageTexture and PixelTexture)*/
GF_TextureHandler *gf_sc_texture_get_handler(GF_Node *n);

/*these ones are needed by modules only for Background(2D) handling*/

/*returns 1 if url changed from current one*/
Bool gf_sc_texture_check_url_change(GF_TextureHandler *txh, MFURL *url);
/*starts associated object*/
GF_Err gf_sc_texture_play(GF_TextureHandler *txh, MFURL *url);
GF_Err gf_sc_texture_play_from_to(GF_TextureHandler *txh, MFURL *url, Double start_offset, Double end_offset, Bool can_loop, Bool lock_scene_timeline);
/*stops associated object*/
void gf_sc_texture_stop(GF_TextureHandler *txh);
/*restarts associated object - DO NOT CALL stop/start*/
void gf_sc_texture_restart(GF_TextureHandler *txh);
/*common routine for all video texture: fetches a frame and update the 2D texture object */
void gf_sc_texture_update_frame(GF_TextureHandler *txh, Bool disable_resync);
/*release video memory if needed*/
void gf_sc_texture_release_stream(GF_TextureHandler *txh);



/*sensor node handler - this is not defined as a stack because Anchor is both a grouping node and a 
sensor node, and we DO need the groupingnode stack...*/
typedef struct _sensor_handler
{
	/*sensor enabled or not ?*/
	Bool (*IsEnabled)(GF_Node *node);
	/*user input on sensor:
	is_over: pointing device is over a shape the sensor is attached to
	is_cancel: the sensor state has been canceled due to another sensor. This typically happens following "click" events in SVG
				which do not consume the mousedown but consumes the mouseup
	evt_type: mouse event type
	compositor: pointer to compositor - hit info is stored at compositor level
	return: was the event consumed ?
	*/
	Bool (*OnUserEvent)(struct _sensor_handler *sh, Bool is_over, Bool is_cancel, GF_Event *ev, GF_Compositor *compositor);
	Bool grabbed;
	/*pointer to the sensor node*/
	GF_Node *sensor;
} GF_SensorHandler;

/*returns TRUE if the node is a pointing device sensor node that can be stacked during traversing (all sensor except anchor)*/
Bool compositor_mpeg4_is_sensor_node(GF_Node *node);
/*returns associated sensor handler from traversable stack (the node handler is always responsible for creation/deletion)
returns NULL if not a sensor or sensor is not activated*/
GF_SensorHandler *compositor_mpeg4_get_sensor_handler(GF_Node *n);

/*rendering modes*/
enum
{
	/*regular traversing mode for z-sorting:
		- 2D mode: builds the display list (may draw directly if requested)
		- 3D mode: sort & queue transparent objects
	*/
	TRAVERSE_SORT = 0,
	/*explicit draw routine used when flushing 2D display list*/
	TRAVERSE_DRAW_2D,
	/*pick routine*/
	TRAVERSE_PICK,
	/*get bounds routine: returns bounds in local coord system (including node transform if any)*/
	TRAVERSE_GET_BOUNDS,
	/*set to signal bindable render - only called on bindable stack top if present.
	for background (drawing request), viewports/viewpoints fog and navigation (setup)
	all other nodes SHALL NOT RESPOND TO THIS CALL
	*/
	TRAVERSE_BINDABLE,

	/*writes the text selection into the compositor buffer - we need a traversing mode for this operation
	to handle properly text and tspans*/
	TRAVERSE_GET_TEXT,

#ifndef GPAC_DISABLE_3D
	/*explicit draw routine used when flushing 3D display list*/
	TRAVERSE_DRAW_3D,
	/*set global lights on. Since the model_matrix is not pushed to the target in this 
	pass, global lights shall not forget to do it (cf lighting.c)*/
	TRAVERSE_LIGHTING,
	/*collision routine*/
	TRAVERSE_COLLIDE,
#endif
};


typedef struct _group_cache_candidate GF_CacheCandidate;



#define MAX_USER_CLIP_PLANES		4


/*the traversing context: set_up at top-level and passed through SFNode_Render. Each node is responsible for 
restoring the context state before returning*/
struct _traversing_state
{
	struct _audio_group *audio_parent;
	struct _soundinterface *sound_holder;

#ifndef GPAC_DISABLE_SVG
	SVGPropertiesPointers *svg_props;
	u32 svg_flags;
#endif

	/*current traversing mode*/
	u32 traversing_mode;
	/*for 2D drawing, indicates objects are to be drawn as soon as traversed, at each frame*/
	Bool immediate_draw;
	/*current subtree is part of a switched-off subtree (needed for audio)*/
	Bool switched_off;
	/*set by the traversed subtree to indicate no cull shall be performed*/
	Bool disable_cull;

	/*indicates if we are in a layer or not*/
	Bool is_layer;
	/*current graph traversed is in pixel metrics*/
	Bool pixel_metrics;
	/*minimal half-dimension (w/2, h/2)*/
	Fixed min_hsize;

	/*indicates if the current subtree is fliped compared to the target visual*/
	Bool fliped_coords;

	/*current size of viewport being traverse (root scene, layers)*/
	SFVec2f vp_size;

	/*the one and only visual manager currently being traversed*/
	GF_VisualManager *visual;
	
#ifndef GPAC_DISABLE_VRML
	/*current background and viewport stacks*/
	GF_List *backgrounds;
	GF_List *viewpoints;
#endif


	/*current transformation from top-level*/
	GF_Matrix2D transform;
	/*current color transformation from top-level*/
	GF_ColorMatrix color_mat;
	/* Contains the viewbox transform, used for svg ref() transform */
	GF_Matrix2D vb_transform;

	/*if set all nodes shall be redrawn - set only at specific places in the tree*/
	Bool invalidate_all;

	/*text splitting: 0: no splitting, 1: word by word, 2:letter by letter*/
	u32 text_split_mode;
	/*1-based idx of text element drawn*/
	u32 text_split_idx;

	/*all VRML sensors for the current level*/
	GF_List *vrml_sensors;

	/*current appearance when traversing geometry nodes*/
	GF_Node *appear;
	/*parent group for composition: can be Form, Layout or Layer2D*/
	struct _parent_node_2d *parent;

	/*group/object bounds in local coordinate system*/
	GF_Rect bounds;

	/*node for which bounds should be fetched - SVG only*/
	GF_Node *for_node;
	Bool abort_bounds_traverse;
	GF_Matrix2D mx_at_node;
	Bool ignore_strike;
	
	GF_List *use_stack;

	/* Styling Property and others for SVG context */
#ifndef GPAC_DISABLE_SVG
	SVG_Number *parent_use_opacity;
	SVGAllAttributes *parent_anim_atts;

	/*SVG text rendering state*/
	Bool in_svg_text;
	Bool in_svg_text_area;

	/* current chunk & position of last placed text chunk*/
	u32 chunk_index;
	Fixed text_end_x, text_end_y;

	/* text & tspan state*/
	GF_List *x_anchors;
	SVG_Coordinates *text_x, *text_y, *text_rotate;
	u32 count_x, count_y, count_rotate, idx_rotate;

	/* textArea state*/
	Fixed max_length, max_height;
	Fixed base_x, base_y;
	Fixed line_spacing;
	Fixed base_shift;
	/*quick and dirty hack to try to solve xml:space across text and tspans without 
	flattening the DOMText nodes
	0: first block of text
	1: previous block of text ended with a space
	2: previous block of text did NOT end with a space
	*/
	u32 last_char_type;
	/*in textArea, indicates that the children bounds must be refreshed due to a baseline adjustment*/
	u32 refresh_children_bounds;
#endif
	GF_Node *text_parent;

	/*current context to be drawn - only set when drawing in 2D mode or 3D for SVG*/
	struct _drawable_context *ctx;

	/*world ray for picking - in 2D, orig is 2D mouse pos and direction is -z*/
	GF_Ray ray;
	s32 pick_x, pick_y;

	/*we have 2 clippers, one for regular clipping (layout, form if clipping) which is maintained in world coord system
	and one for layer2D which is maintained in parent coord system (cf layer rendering). The layer clipper
	is used only when cascading layers - layer3D doesn't use clippers*/
	Bool has_clip, has_layer_clip;
	/*active clipper in world coord system */
	GF_Rect clipper, layer_clipper;

	
	/*set when traversing a cached group during offscreen bitmap construction.*/
	Bool in_group_cache;

	Bool in_svg_filter;

	u32 subscene_not_over;

#ifndef GPAC_DISABLE_3D
	/*the current camera*/
	GF_Camera *camera;

	/*current object (model) transformation from top-level, view is NOT included*/
	GF_Matrix model_matrix;

#ifndef GPAC_DISABLE_VRML
	/*fog bind stack*/
	GF_List *fogs; 
	/*navigation bind stack*/
	GF_List *navigations;
#endif

	/*when drawing, signals the mesh is transparent (enables blending)*/
	Bool mesh_is_transparent;
	/*when drawing, signals the number of textures used by the mesh*/
	u32 mesh_num_textures;

	/*bounds for TRAVERSE_GET_BOUNDS and background rendering*/
	GF_BBox bbox;

	/*cull flag (used to skip culling of children when parent bbox is completely inside/outside frustum)*/
	u32 cull_flag;

	/*toggle local lights on/off - field is ONLY valid in TRAVERSE_RENDER mode, and local lights
	are always set off in reverse order as when set on*/
	Bool local_light_on;
	/*current directional ligths contexts - only valid in TRAVERSE_RENDER*/
	GF_List *local_lights;

	/*clip planes in world coords*/
	GF_Plane clip_planes[MAX_USER_CLIP_PLANES];
	u32 num_clip_planes;


	/*layer traversal state:
		set to the first traversed layer3D when picking
		set to the current layer3D traversed when rendering 3D to an offscreen bitmap. This alows other 
			nodes (typically bindables) seting the layer dirty flags to force a redraw 
	*/
	GF_Node *layer3d;
#endif

	
#ifdef GF_SR_USE_DEPTH
    Fixed depth_gain, depth_offset;
#endif


#ifdef GF_SR_USE_VIDEO_CACHE
	/*set to 1 if cache evaluation can be skipped - this is only set when there is not enough memory 
	to cache a sub-group, in which case the group cannot be cached (we're caching in display coordinates)*/
	Bool cache_too_small;
#endif
};

/*
	Audio mixer - MAX 6 CHANNELS SUPPORTED
*/

/*the audio object as used by the mixer. All audio nodes need to implement this interface*/
typedef struct _audiointerface
{
	/*fetch audio data for a given audio delay (~soundcard drift) - if delay is 0 sync should not be performed 
	(eg intermediate mix) */
	char *(*FetchFrame) (void *callback, u32 *size, u32 audio_delay_ms);
	/*release a number of bytes in the indicated frame (ts)*/
	void (*ReleaseFrame) (void *callback, u32 nb_bytes);
	/*get media speed*/
	Fixed (*GetSpeed)(void *callback);
	/*gets volume for each channel - vol = Fixed[6]. returns 1 if volume shall be changed (!= 1.0)*/
	Bool (*GetChannelVolume)(void *callback, Fixed *vol);
	/*returns 1 if muted*/
	Bool (*IsMuted)(void *callback);
	/*user callback*/
	void *callback;
	/*returns 0 if config is not known yet or changed, 
	otherwise AND IF @for_reconf is set, updates member var below and return TRUE
	You may return 0 to force parent user invalidation*/
	Bool (*GetConfig)(struct _audiointerface *ai, Bool for_reconf);
	/*updated cfg, or 0 otherwise*/
	u32 chan, bps, samplerate, ch_cfg;
} GF_AudioInterface;

typedef struct __audiomix GF_AudioMixer;

/*create mixer - ar is NULL for any sub-mixers, or points to the main audio renderer (mixer outputs to sound driver)*/
GF_AudioMixer *gf_mixer_new(struct _audio_render *ar);
void gf_mixer_del(GF_AudioMixer *am);
void gf_mixer_remove_all(GF_AudioMixer *am);
void gf_mixer_add_input(GF_AudioMixer *am, GF_AudioInterface *src);
void gf_mixer_remove_input(GF_AudioMixer *am, GF_AudioInterface *src);
void gf_mixer_lock(GF_AudioMixer *am, Bool lockIt);
/*mix inputs in buffer, return number of bytes written to output*/
u32 gf_mixer_get_output(GF_AudioMixer *am, void *buffer, u32 buffer_size, u32 delay_ms);
/*reconfig all sources if needed - returns TRUE if main audio config changed
NOTE: this is called at each gf_mixer_get_output by the mixer. To call externally for audio hardware
reconfiguration only*/
Bool gf_mixer_reconfig(GF_AudioMixer *am);
/*retrieves mixer cfg*/
void gf_mixer_get_config(GF_AudioMixer *am, u32 *outSR, u32 *outCH, u32 *outBPS, u32 *outChCfg);
/*called by audio renderer in case the hardware used a different setup than requested*/
void gf_mixer_set_config(GF_AudioMixer *am, u32 outSR, u32 outCH, u32 outBPS, u32 ch_cfg);
Bool gf_mixer_is_src_present(GF_AudioMixer *am, GF_AudioInterface *ifce);
u32 gf_mixer_get_src_count(GF_AudioMixer *am);
void gf_mixer_force_chanel_out(GF_AudioMixer *am, u32 num_channels);
u32 gf_mixer_get_block_align(GF_AudioMixer *am);
Bool gf_mixer_must_reconfig(GF_AudioMixer *am);
Bool gf_mixer_empty(GF_AudioMixer *am);


struct _audiofilterentry
{
	struct _audiofilterentry *next;
	u32 in_block_size;
	char *in_block;
	u32 nb_bytes;
	u32 delay_ms;
	Bool enable, in_place;
	GF_AudioFilter *filter;
};

typedef struct
{
	Bool enable_filters;
	struct _audiofilterentry *filters;
	/*filter processing takes place in a temp buffer since we don't know how many
	samples a filter will output, and may ned to cache the output between 2 fill_buffer calls*/
	char *tmp_block1, *tmp_block2;
	u32 min_block_size, max_block_size, delay_ms;

} GF_AudioFilterChain;

GF_Err gf_afc_load(GF_AudioFilterChain *afc, GF_User *user, char *filterstring);
GF_Err gf_afc_setup(GF_AudioFilterChain *afc, u32 bps, u32 sr, u32 chan, u32 ch_cfg, u32 *ch_out, u32 *ch_cfg_out);
u32 gf_afc_process(GF_AudioFilterChain *afc, u32 nb_bytes);
void gf_afc_unload(GF_AudioFilterChain *afc);
void gf_afc_reset(GF_AudioFilterChain *afc);

/*the audio renderer*/
typedef struct _audio_render
{
	GF_AudioOutput *audio_out;

	Bool disable_resync;
	Bool disable_multichannel;

	/*startup time (the audio renderer is used when present as the system clock)*/
	u32 startTime;
	/*frozen time counter if set*/
	Bool Frozen;
	u32 FreezeTime;
	
	/*final output*/
	GF_AudioMixer *mixer;
	Bool need_reconfig;
	/*client*/
	GF_User *user;

	GF_List *audio_listeners;
	/*audio thread if output not self-threaded*/
	GF_Thread *th;
	/*thread state: 0: not init, 1: running, 2: waiting for stop, 3: done*/
	u32 audio_th_state;

	u32 audio_delay, volume, pan, mute;

	GF_AudioFilterChain filter_chain;
	u32 nb_filled, nb_used; 
} GF_AudioRenderer;

/*creates audio renderer*/
GF_AudioRenderer *gf_sc_ar_load(GF_User *user);
/*deletes audio renderer*/
void gf_sc_ar_del(GF_AudioRenderer *ar);
/*control audio renderer - CtrlType:
	0: pause
	1: resume
	2: clean HW buffer and play
*/
void gf_sc_ar_control(GF_AudioRenderer *ar, u32 CtrlType);
/*set volume and pan*/
void gf_sc_ar_set_volume(GF_AudioRenderer *ar, u32 Volume);
void gf_sc_ar_set_pan(GF_AudioRenderer *ar, u32 Balance);
/*mute/unmute audio*/
void gf_sc_ar_mute(GF_AudioRenderer *ar, Bool mute);

/*set audio priority*/
void gf_sc_ar_set_priority(GF_AudioRenderer *ar, u32 priority);
/*gets time in msec - this is the only clock used by the whole ESM system - depends on the audio driver*/
u32 gf_sc_ar_get_clock(GF_AudioRenderer *ar);
/*reset all input nodes*/
void gf_sc_ar_reset(GF_AudioRenderer *ar);
/*add audio node*/
void gf_sc_ar_add_src(GF_AudioRenderer *ar, GF_AudioInterface *source);
/*remove audio node*/
void gf_sc_ar_remove_src(GF_AudioRenderer *ar, GF_AudioInterface *source);
/*reconfig audio hardware if needed*/
void gf_sc_ar_reconfig(GF_AudioRenderer *ar);
u32 gf_sc_ar_get_delay(GF_AudioRenderer *ar);

/*the sound node interface for intensity & spatialization*/
typedef struct _soundinterface
{
	/*gets volume for each channel - vol = Fixed[6]. returns 1 if volume shall be changed (!= 1.0)
	if NULL channels are always at full intensity*/
	Bool (*GetChannelVolume)(GF_Node *owner, Fixed *vol);
	/*get sound priority (0: min, 255: max) - used by mixer to determine*/
	u8 (*GetPriority) (GF_Node *owner);
	/*node owning the structure*/
	GF_Node *owner;
} GF_SoundInterface;

typedef struct __audiofilteritem GF_AudioFilterItem;

/*audio common to AudioClip and AudioSource*/
typedef struct
{
	GF_Node *owner;
	GF_Compositor *compositor;
	GF_AudioInterface input_ifce;
	/*can be NULL if the audio node generates its output from other input*/
	GF_MediaObject *stream;
	/*object speed and intensity*/
	Fixed speed, intensity;
	Bool stream_finished;
	Bool need_release;
	u32 is_open;
	Bool is_muted;
	Bool register_with_renderer, register_with_parent;

	GF_SoundInterface *snd;
	GF_AudioFilterItem *filter;
} GF_AudioInput;
/*setup interface with audio renderer - overwrite any functions needed after setup EXCEPT callback object*/
void gf_sc_audio_setup(GF_AudioInput *ai, GF_Compositor *sr, GF_Node *node);
/*unregister interface from renderer/mixer and stops source - deleteing the interface is the caller responsability*/
void gf_sc_audio_predestroy(GF_AudioInput *ai);
/*open audio object*/
GF_Err gf_sc_audio_open(GF_AudioInput *ai, MFURL *url, Double clipBegin, Double clipEnd);
/*closes audio object*/
void gf_sc_audio_stop(GF_AudioInput *ai);
/*restarts audio object (cf note in MediaObj)*/
void gf_sc_audio_restart(GF_AudioInput *ai);

Bool gf_sc_audio_check_url(GF_AudioInput *ai, MFURL *url);

/*base grouping audio node (nodes with several audio sources as children)*/
#define AUDIO_GROUP_NODE	\
	GF_AudioInput output;		\
	void (*add_source)(struct _audio_group *_this, GF_AudioInput *src);	\

typedef struct _audio_group
{
	AUDIO_GROUP_NODE
} GF_AudioGroup;


/*register audio node with parent audio renderer (mixer or main renderer)*/
void gf_sc_audio_register(GF_AudioInput *ai, GF_TraverseState *tr_state);
void gf_sc_audio_unregister(GF_AudioInput *ai);


#ifndef GPAC_DISABLE_SVG
GF_Err gf_term_get_mfurl_from_xlink(GF_Node *node, MFURL *mfurl);
Fixed gf_sc_svg_convert_length_to_display(GF_Compositor *sr, SVG_Length *length);

char *gf_term_resolve_xlink(GF_Node *node, char *the_url);
#endif

GF_Err compositor_2d_set_aspect_ratio(GF_Compositor *sr);
void compositor_2d_set_user_transform(GF_Compositor *sr, Fixed zoom, Fixed tx, Fixed ty, Bool is_resize) ;
GF_Err compositor_2d_get_video_access(GF_VisualManager *surf);
void compositor_2d_release_video_access(GF_VisualManager *surf);
void compositor_2d_init_callbacks(GF_Compositor *compositor);
GF_Rect compositor_2d_update_clipper(GF_TraverseState *tr_state, GF_Rect this_clip, Bool *need_restore, GF_Rect *original, Bool for_layer);

Bool compositor_texture_rectangles(GF_VisualManager *visual, GF_TextureHandler *txh, GF_IRect *clip, GF_Rect *unclip, GF_Window *src, GF_Window *dst, Bool *disable_blit, Bool *has_scale);

Bool compositor_get_2d_plane_intersection(GF_Ray *ray, SFVec3f *res);

void compositor_send_resize_event(GF_Compositor *compositor, GF_SceneGraph *subscene, Fixed old_z, Fixed old_tx, Fixed old_ty, Bool is_resize);

void compositor_set_cache_memory(GF_Compositor *compositor, u32 memory);
/*checks whether the background node is transparent or not*/
Bool compositor_background_transparent(GF_Node *node);

#ifndef GPAC_DISABLE_3D

GF_Err compositor_3d_set_aspect_ratio(GF_Compositor *sr);
GF_Camera *compositor_3d_get_camera(GF_Compositor *sr);
void compositor_3d_reset_camera(GF_Compositor *sr);
GF_Camera *compositor_layer3d_get_camera(GF_Node *node);
void compositor_layer3d_bind_camera(GF_Node *node, Bool do_bind, u32 nav_value);
void compositor_3d_draw_bitmap(struct _drawable *stack, DrawAspect2D *asp, GF_TraverseState *tr_state, Fixed width, Fixed height, Fixed bmp_scale_x, Fixed bmp_scale_y);

GF_Err compositor_3d_get_screen_buffer(GF_Compositor *sr, GF_VideoSurface *fb, u32 depth_buffer_mode);
GF_Err compositor_3d_get_offscreen_buffer(GF_Compositor *sr, GF_VideoSurface *fb, u32 view_idx, u32 depth_buffer_mode);
GF_Err compositor_3d_release_screen_buffer(GF_Compositor *sr, GF_VideoSurface *framebuffer);

void gf_sc_load_opengl_extensions(GF_Compositor *sr, Bool has_gl_context);

Bool gf_sc_fit_world_to_screen(GF_Compositor *compositor);

#endif

Bool gf_sc_exec_event(GF_Compositor *sr, GF_Event *evt);
void gf_sc_get_nodes_bounds(GF_Node *self, GF_ChildNodeItem *children, GF_TraverseState *tr_state, s32 *child_idx);

Bool gf_sc_exec_event_vrml(GF_Compositor *compositor, GF_Event *ev);

void gf_sc_visual_register(GF_Compositor *sr, GF_VisualManager *surf);
void gf_sc_visual_unregister(GF_Compositor *sr, GF_VisualManager *surf);
Bool gf_sc_visual_is_registered(GF_Compositor *sr, GF_VisualManager *surf);

Bool gf_sc_pick_in_clipper(GF_TraverseState *tr_state, GF_Rect *clip);

void compositor_gradient_update(GF_TextureHandler *txh);
void compositor_set_ar_scale(GF_Compositor *sr, Fixed scaleX, Fixed scaleY);

/*reset focus if node being deleted has the focus - must be called for each focusable node (internally called for 2D & 3D drawable nodes)*/
void gf_sc_check_focus_upon_destroy(GF_Node *n);

void gf_sc_key_navigator_del(GF_Compositor *sr, GF_Node *n);
void gf_sc_change_key_navigator(GF_Compositor *sr, GF_Node *n);
GF_Node *gf_scene_get_keynav(GF_SceneGraph *sg, GF_Node *sensor);
const char *gf_scene_get_service_url(GF_SceneGraph *sg);


Bool gf_scene_is_over(GF_SceneGraph *sg);

#ifndef GPAC_DISABLE_SVG

void compositor_svg_build_gradient_texture(GF_TextureHandler *txh);

/*base routine fo all svg elements:
	- check for conditional processing (requiredXXX, ...)
	- apply animation and inheritance

	returns 0 if the node shall not be traversed due to conditional processing
*/
Bool compositor_svg_traverse_base(GF_Node *node, SVGAllAttributes *all_atts, GF_TraverseState *tr_state, SVGPropertiesPointers *backup_props, u32 *backup_flags);
Bool compositor_svg_is_display_off(SVGPropertiesPointers *props);
void compositor_svg_apply_local_transformation(GF_TraverseState *tr_state, SVGAllAttributes *atts, GF_Matrix2D *backup_matrix_2d, GF_Matrix *backup_matrix);
void compositor_svg_restore_parent_transformation(GF_TraverseState *tr_state, GF_Matrix2D *backup_matrix_2d, GF_Matrix *backup_matrix);

void compositor_svg_traverse_children(GF_ChildNodeItem *children, GF_TraverseState *tr_state);

Bool compositor_svg_evaluate_conditional(GF_Compositor *compositor, SVGAllAttributes *all_atts);

/*returns the node associated with the given xlink - this is not always the target node of the xlink structure due
to async restart of animation nodes*/
GF_Node *compositor_svg_get_xlink_resource_node(GF_Node *node, XMLRI *xlink);

GF_SceneGraph *gf_sc_animation_get_scenegraph(GF_Node *node);

#endif

/*Text handling*/

/*we identify the edit caret in a text string as this value*/
#define GF_CARET_CHAR 0x1

typedef struct _gf_font GF_Font;

struct _gf_font 
{
	/*fonts are linked within the font manager*/
	GF_Font *next;
	/*list of glyphs in the font*/
	GF_Glyph *glyph;

	char *name;
	u32 em_size;
	u32 styles;
	/*font uits in em size*/
	s32 ascent, descent, underline, line_spacing, max_advance_h, max_advance_v;
	s32 baseline;

	/*only set for embedded font engines (SVG fonts)*/
	GF_Font *(*get_alias)(void *udta);
	GF_Err (*get_glyphs)(void *udta, const char *utf_string, u32 *glyph_ids_buffer, u32 *io_glyph_ids_buffer_size, const char *xml_lang, Bool *is_rtl);
	GF_Glyph *(*load_glyph)(void *udta, u32 glyph_name);
	void *udta;

	Bool not_loaded;

	struct _gf_ft_mgr *ft_mgr;
	/*list of spans currently using the font - this is needed to allow for dynamic discard of the font*/
	GF_List *spans;
};

enum 
{
	/*span direction is horizontal*/
	GF_TEXT_SPAN_HORIZONTAL = 1,
	/*span is underlined*/
	GF_TEXT_SPAN_UNDERLINE = 1<<1,
	/*span is fliped (coord systems with Y-axis pointing downwards like SVG)*/
	GF_TEXT_SPAN_FLIP = 1<<2,
	/*span is in the current text selection*/
	GF_TEXT_SPAN_RIGHT_TO_LEFT = 1<<3,
	/*span is in the current text selection*/
	GF_TEXT_SPAN_SELECTED = 1<<4
};

typedef struct __text_span
{
	GF_Font *font;
	
	GF_Glyph **glyphs;
	u32 nb_glyphs;

	u32 flags;

	Fixed font_size;

	/*scale to apply to get to requested font size*/
	Fixed font_scale;
	GF_Rect bounds;

	/*MPEG-4 span scaling (length & maxExtend)*/
	Fixed x_scale, y_scale;
	/*x (resp. y) offset in local coord system. Ignored if per-glyph dx (resp dy) are specified*/
	Fixed off_x, off_y;

	/*per-glyph positioning - when allocated, this is the same number as the glyphs*/
	Fixed *dx, *dy, *rot;

	/*span language*/
//	const char *lang;

	/*span texturing and 3D tools*/
	struct _span_internal *ext;

	/*SVG stuff :(*/
	GF_Node *anchor;
	GF_Node *user;
} GF_TextSpan;

GF_FontManager *gf_font_manager_new(GF_User *user);
void gf_font_manager_del(GF_FontManager *fm);

GF_Font *gf_font_manager_set_font(GF_FontManager *fm, char **alt_fonts, u32 nb_fonts, u32 styles);
GF_Font *gf_font_manager_set_font_ex(GF_FontManager *fm, char **alt_fonts, u32 nb_fonts, u32 styles, Bool check_only);

GF_TextSpan *gf_font_manager_create_span(GF_FontManager *fm, GF_Font *font, char *span, Fixed font_size, Bool needs_x_offset, Bool needs_y_offset, Bool needs_rotate, const char *lang, Bool fliped_text, u32 styles, GF_Node *user);
void gf_font_manager_delete_span(GF_FontManager *fm, GF_TextSpan *tspan);

GF_Err gf_font_manager_register_font(GF_FontManager *fm, GF_Font *font);
GF_Err gf_font_manager_unregister_font(GF_FontManager *fm, GF_Font *font);

void gf_font_manager_refresh_span_bounds(GF_TextSpan *span);
GF_Path *gf_font_span_create_path(GF_TextSpan *span);


void gf_font_spans_draw_2d(GF_List *spans, GF_TraverseState *tr_state, u32 hl_color, Bool force_texture_text, GF_Rect *bounds);
void gf_font_spans_draw_3d(GF_List *spans, GF_TraverseState *tr_state, DrawAspect2D *asp, u32 text_hl, Bool force_texturing);
void gf_font_spans_pick(GF_Node *node, GF_List *spans, GF_TraverseState *tr_state, GF_Rect *node_bounds, Bool use_dom_events, struct _drawable *drawable);
void gf_font_spans_get_selection(GF_Node *node, GF_List *spans, GF_TraverseState *tr_state);

GF_Font *gf_compositor_svg_set_font(GF_FontManager *fm, char *a_font, u32 styles, Bool check_only);

/*switches focus node:
@compositor: compositor
@move_prev: finds previous focus rather than next
@focus: new focus if force_focus_type is set
@force_focus_type: 0: focus not forced, 1: focus forced to focus, 2: focus forced to prev/next focusable child of focus node*/
u32 gf_sc_focus_switch_ring(GF_Compositor *compositor, Bool move_prev, GF_Node *focus, u32 force_focus_type);

Bool compositor_handle_navigation(GF_Compositor *compositor, GF_Event *ev);

void gf_sc_next_frame_state(GF_Compositor *compositor, u32 state);


#ifdef GPAC_USE_TINYGL
void gf_get_tinygl_depth(GF_TextureHandler *txh);
#endif



typedef struct
{
	void *udta;
	/*called when new video frame is ready to be flushed on screen. time is the terminal global clock in ms*/
	void (*on_video_frame)(void *udta, u32 time);
	/*called when video output has been resized*/
	void (*on_video_reconfig)(void *udta, u32 width, u32 height);
} GF_VideoListener;

GF_Err gf_sc_add_video_listener(GF_Compositor *compositor, GF_VideoListener *vl);
GF_Err gf_sc_remove_video_listener(GF_Compositor *compositor, GF_VideoListener *vl);

typedef struct
{
	void *udta;
	/*called when audio frame is ready to be sent to the sound card. 
		@buffer, @buffer_size: audio buffer
		@time: the terminal global clock in ms
		@delay: Due to sound card latencies, audio is sent to the sound card delay milliseconds earlier than 
		its associated video.
	*/
	void (*on_audio_frame)(void *udta, char *buffer, u32 buffer_size, u32 time, u32 delay);
	/*called when audio output has been reconfigured*/
	void (*on_audio_reconfig)(void *udta, u32 samplerate, u32 bits_per_sample, u32 nb_channel, u32 channel_cfg);
} GF_AudioListener;

GF_Err gf_sc_add_audio_listener(GF_Compositor *compositor, GF_AudioListener *al);
GF_Err gf_sc_remove_audio_listener(GF_Compositor *compositor, GF_AudioListener *al);


GF_Err gf_sc_set_scene_size(GF_Compositor *compositor, u32 Width, u32 Height, Bool force_size);

#ifdef __cplusplus
}
#endif
#endif	/*_COMPOSITOR_DEV_H_*/

