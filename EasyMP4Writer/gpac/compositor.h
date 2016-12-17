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

#ifndef _GF_COMPOSITOR_H_
#define _GF_COMPOSITOR_H_

#ifdef __cplusplus
extern "C" {
#endif


/*include scene graph API*/
#include <gpac/scenegraph.h>
/*GF_User and GF_Terminal */
#include <gpac/user.h>
/*frame buffer definition*/
#include <gpac/color.h>

typedef struct __tag_compositor GF_Compositor;

/*creates default compositor 
if self_threaded, video compositor uses a dedicated thread, otherwise visual rendering is done by the user
audio compositor always runs in its own thread if enabled
term may be NULL, in which case InputSensors won't be enabled
*/
GF_Compositor *gf_sc_new(GF_User *user_interface, Bool self_threaded, GF_Terminal *term);
void gf_sc_del(GF_Compositor *sr);

/*sets simulation frame rate*/
void gf_sc_set_fps(GF_Compositor *sr, Double fps);

/*set the root scene graph of the compositor - if NULL remove current and reset simulation time*/
GF_Err gf_sc_set_scene(GF_Compositor *sr, GF_SceneGraph *scene_graph);

/*if the compositor doesn't use its own thread for visual, this will perform a render pass*/
Bool gf_sc_draw_frame(GF_Compositor *sr);

/*inits rendering info for the node - shall be called for all nodes the parent system doesn't handle*/
void gf_sc_on_node_init(GF_Compositor *sr, GF_Node *node);

/*notify the given node has been modified. The compositor filters object to decide whether the scene graph has to be 
traversed or not- if object is NULL, this means complete traversing of the graph is requested (use carefully since it
can be a time consuming operation)*/
void gf_sc_invalidate(GF_Compositor *sr, GF_Node *byObj);

/*return the compositor time - this is the time every time line syncs on*/
u32 gf_sc_get_clock(GF_Compositor *sr);


/*locks/unlocks the visual scene rendering - modification of the scene tree shall only happen when scene compositor is locked*/
void gf_sc_lock(GF_Compositor *sr, Bool doLock);
/*locks/unlocks the audio scene rendering - this is needed whenever an audio object changes config on the fly*/
void gf_sc_lock_audio(GF_Compositor *sr, Bool doLock);

/*notify user input - returns 0 if event hasn't been handled by the compositor*/
Bool gf_sc_user_event(GF_Compositor *sr, GF_Event *event);

/*maps screen coordinates to bifs 2D coordinates for the current zoom/pan settings
X and Y are point coordinates in the display expressed in BIFS-like fashion (0,0) at center of 
display and Y increasing from bottom to top*/
void gf_sc_map_point(GF_Compositor *sr, s32 X, s32 Y, Fixed *bifsX, Fixed *bifsY);

/*signal the size of the display area has been changed*/
GF_Err gf_sc_size_changed(GF_Compositor *sr, u32 NewWidth, u32 NewHeight);

/*set/get user options - options are as defined in user.h*/
GF_Err gf_sc_set_option(GF_Compositor *sr, u32 type, u32 value);
u32 gf_sc_get_option(GF_Compositor *sr, u32 type);

/*returns current FPS
if @absoluteFPS is set, the return value is the absolute framerate, eg NbFrameCount/NbTimeSpent regardless of
whether a frame has been drawn or not, which means the FPS returned can be much greater than the compositor FPS
if @absoluteFPS is not set, the return value is the FPS taking into account not drawn frames (eg, less than or equal to
compositor FPS)
*/
Double gf_sc_get_fps(GF_Compositor *sr, Bool absoluteFPS);

Bool gf_sc_has_text_selection(GF_Compositor *compositor);
const char *gf_sc_get_selected_text(GF_Compositor *compositor);

GF_Err gf_sc_paste_text(GF_Compositor *compositor, const char *text);

/*user-define management: this is used for instant visual rendering of the scene graph, 
for exporting or authoring tools preview. User is responsible for calling render when desired and shall also maintain
scene timing*/

/*force render tick*/
void gf_sc_render(GF_Compositor *sr);
/*gets screen buffer - this locks the scene graph too until released is called*/
GF_Err gf_sc_get_screen_buffer(GF_Compositor *sr, GF_VideoSurface *framebuffer, u32 depth_buffer_mode);
/*gets offscreen buffer - this locks the scene graph too until released is called*/
GF_Err gf_sc_get_offscreen_buffer(GF_Compositor *sr, GF_VideoSurface *framebuffer, u32 view_idx, u32 depth_buffer_mode);
/*releases screen buffer and unlocks graph*/
GF_Err gf_sc_release_screen_buffer(GF_Compositor *sr, GF_VideoSurface *framebuffer);

/*renders one frame*/
void gf_sc_simulation_tick(GF_Compositor *sr);

/*forces graphics cache recompute*/
void gf_sc_reset_graphics(GF_Compositor *sr);

/*picks a node (may return NULL) - coords are given in OS client system coordinate, as in UserInput*/
GF_Node *gf_sc_pick_node(GF_Compositor *sr, s32 X, s32 Y);

/*get viewpoints/viewports for main scene - idx is 1-based, and if greater than number of viewpoints return GF_EOS*/
GF_Err gf_sc_get_viewpoint(GF_Compositor *sr, u32 viewpoint_idx, const char **outName, Bool *is_bound);
/*set viewpoints/viewports for main scene given its name - idx is 1-based, or 0 to retrieve by viewpoint name
if only one viewpoint is present in the scene, this will bind/unbind it*/
GF_Err gf_sc_set_viewpoint(GF_Compositor *sr, u32 viewpoint_idx, const char *viewpoint_name);

/*render subscene root node. rs is the current traverse stack
this is needed to handle graph metrics changes between scenes...*/
void gf_sc_traverse_subscene(GF_Compositor *sr, GF_Node *inline_parent, GF_SceneGraph *subscene, void *rs);

/*set outupt size*/
GF_Err gf_sc_set_size(GF_Compositor *sr, u32 NewWidth, u32 NewHeight);
/*get outupt size*/
Bool gf_sc_get_size(GF_Compositor *sr, u32 *Width, u32 *Height);

/*returns total length of audio hardware buffer in ms, 0 if no audio*/
u32 gf_sc_get_audio_buffer_length(GF_Compositor *sr);

/*add/remove extra scene from compositor (extra scenes are OSDs or any other scene graphs not directly
usable by main scene, like 3GP text streams*/
void gf_sc_register_extra_graph(GF_Compositor *sr, GF_SceneGraph *extra_scene, Bool do_remove);

/*gets audio hardware delay*/
u32 gf_sc_get_audio_delay(GF_Compositor *sr);

/*returns total length of audio hardware buffer in ms, 0 if no audio*/
u32 gf_sc_get_audio_buffer_length(GF_Compositor *sr);

void *gf_sc_get_visual_compositor(GF_Compositor *sr);

GF_Compositor *gf_sc_get_compositor(GF_Node *node);

Bool gf_sc_script_action(GF_Compositor *sr, u32 type, GF_Node *n, GF_JSAPIParam *param);

void gf_sc_reload_audio_filters(GF_Compositor *compositor);

#ifdef __cplusplus
}
#endif

#endif	/*_GF_COMPOSITOR_H_*/

