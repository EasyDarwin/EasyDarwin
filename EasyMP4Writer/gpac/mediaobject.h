/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005 
 *					All rights reserved
 *
 *  This file is part of GPAC / Stream Management sub-project
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



#ifndef _GF_MEDIA_OBJECT_H_
#define _GF_MEDIA_OBJECT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/scenegraph_vrml.h>


/*
		Media Object

  opaque handler for all natural media objects (audio, video, image) so that compositor and systems engine
are not too tied up. 
	NOTE: the media object location relies on the node parent graph (this is to deal with namespaces in OD framework)
therefore it is the task of the media management app to setup clear links between the scene graph and its ressources
(but this is not mandatory, cf URLs in VRML )

	TODO - add interface for shape coding positioning in mediaObject and in the decoder API
*/

typedef struct _mediaobj GF_MediaObject;

/*locate media object related to the given node - url designes the object to find - returns NULL if
URL cannot be handled - note that until the mediaObject.isInit member is true, the media object is not valid
(and could actually never be) */
GF_MediaObject *gf_mo_register(GF_Node *node, MFURL *url, Bool lock_timelines, Bool force_new_res);
/*unregister the node from the media object*/
void gf_mo_unregister(GF_Node *node, GF_MediaObject *mo);

/*opens media object*/
void gf_mo_play(GF_MediaObject *mo, Double clipBegin, Double clipEnd, Bool can_loop);
/*stops media object - video memory is not reset, last frame is kept*/
Bool gf_mo_stop(GF_MediaObject *mo);
/*restarts media object - shall be used for all looping media instead of stop/play for mediaControl
to restart appropriated objects*/
void gf_mo_restart(GF_MediaObject *mo);

void gf_mo_pause(GF_MediaObject *mo);
void gf_mo_resume(GF_MediaObject *mo);

/*
	Note on mediaControl: mediaControl is the media management app responsability, therefore
is hidden from the rendering app. Since MediaControl overrides default settings of the node (speed and loop)
you must use the gf_mo_get_speed and gf_mo_get_loop in order to know whether the related field applies or not
*/

/*set speed of media - speed is not always applied, depending on media control settings.
NOTE: audio pitching is the responsability of the rendering app*/
void gf_mo_set_speed(GF_MediaObject *mo, Fixed speed);
/*returns current speed of media - in_speed is the speed of the media as set in the node (MovieTexture, 
AudioClip and AudioSource) - the return value is the real speed of the media as overloaded by mediaControl if any*/
Fixed gf_mo_get_speed(GF_MediaObject *mo, Fixed in_speed);
/*returns looping flag of media - in_loop is the looping flag of the media as set in the node (MovieTexture, 
AudioClip) - the return value is the real loop flag of the media as overloaded by mediaControl if any*/
Bool gf_mo_get_loop(GF_MediaObject *mo, Bool in_loop);
/*returns media object duration*/
Double gf_mo_get_duration(GF_MediaObject *mo);
/*returns whether the object should be deactivated (stop) or not - this checks object status as well as 
mediaControl status */
Bool gf_mo_should_deactivate(GF_MediaObject *mo);
/*checks whether the target object is changed - you MUST use this in order to detect url changes*/
Bool gf_mo_url_changed(GF_MediaObject *mo, MFURL *url);


/*fetch media data 

*/
char *gf_mo_fetch_data(GF_MediaObject *mo, Bool resync, Bool *eos, u32 *timestamp, u32 *size);

/*release given amount of media data - nb_bytes is used for audio - if forceDrop is set, the unlocked frame will be 
droped if all bytes are consumed, otherwise it will be droped based on object time - typically, video fetches with the resync
flag set and release without forceDrop, while audio fetches without resync but forces buffer drop. If forceDrop is set to 2, 
the frame will be stated as a discraded frame*/
void gf_mo_release_data(GF_MediaObject *mo, u32 nb_bytes, s32 forceDrop);
/*get media time*/
void gf_mo_get_media_time(GF_MediaObject *mo, u32 *media_time, u32 *media_dur);
/*get object clock*/
void gf_mo_get_object_time(GF_MediaObject *mo, u32 *obj_time);
/*returns mute flag of media - if muted the media shouldn't be displayed*/
Bool gf_mo_is_muted(GF_MediaObject *mo);
/*returns end of stream state*/
Bool gf_mo_is_done(GF_MediaObject *mo);
/*resyncs clock - only audio objects are allowed to use this*/
void gf_mo_adjust_clock(GF_MediaObject *mo, s32 ms_drift);

u32 gf_mo_get_last_frame_time(GF_MediaObject *mo);

Bool gf_mo_get_visual_info(GF_MediaObject *mo, u32 *width, u32 *height, u32 *stride, u32 *pixel_ar, u32 *pixelFormat);

Bool gf_mo_get_audio_info(GF_MediaObject *mo, u32 *sample_rate, u32 *bits_per_sample, u32 *num_channels, u32 *channel_config);

Fixed gf_mo_get_current_speed(GF_MediaObject *mo);

/*checks if the service associated withthis object has an audio stream*/
Bool gf_mo_has_audio(GF_MediaObject *mo);

/*checks if the service associated withthis object has an audio stream*/
Bool gf_mo_is_private_media(GF_MediaObject *mo);

/*set destination window for harware codecs directly outputing to video out - returns 1 if video size has changed*/
Bool gf_mo_set_position(GF_MediaObject *mo, GF_Window *src, GF_Window *dst);

enum
{
	/*this is set to 0 by the OD manager whenever a change occur in the media (w/h change, SR change, etc) 
	as a hint for the compositor*/
	GF_MO_IS_INIT = (1<<1),
	/*used by animation stream to remove TEXT from display upon delete and URL change*/
	GF_MO_DISPLAY_REMOVE = (1<<2),
};

u32 gf_mo_get_flags(GF_MediaObject *mo);
void gf_mo_set_flag(GF_MediaObject *mo, u32 flag, Bool set_on);

/*loads a new resource as indicated in the xlink:href attribute of the node. If this points to a fragment
of the current document, returns NULL. This will automatically trigger a play request on the resource*/
GF_MediaObject *gf_mo_load_xlink_resource(GF_Node *node, Bool primary_resource, Double clipBegin, Double clipEnd);
void gf_mo_unload_xlink_resource(GF_Node *node, GF_MediaObject *mo);
/*returns scene graph associated with a scene/document object, or NULL if wrong type or not loaded*/
GF_SceneGraph *gf_mo_get_scenegraph(GF_MediaObject *mo);

#ifdef __cplusplus
}
#endif


#endif	/*_GF_MEDIA_OBJECT_H_*/


