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


#ifndef _GF_SCENEGRAPH_DEV_H_
#define _GF_SCENEGRAPH_DEV_H_

/*defined this macro to enable cyclic render*/
#define GF_CYCLIC_TRAVERSE_ON

/*defined this macro to enable scene replacement from inside (through conditional)*/
//#define GF_SELF_REPLACE_ENABLE

/*for vrml base types, ROUTEs and PROTOs*/
#include <gpac/scenegraph_vrml.h>

#include <gpac/scenegraph_svg.h>

#include <gpac/thread.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifdef GPAC_HAS_SPIDERMONKEY

/*WIN32 and WinCE config (no configure script)*/
#if defined(WIN32) || defined(_WIN32_WCE) || defined(__SYMBIAN32__)
#ifndef XP_PC
#define XP_PC
#endif
#ifndef XP_WIN
#define XP_WIN
#endif
/*WINCE specific config*/
#if defined(_WIN32_WCE)
#include <windows.h>
#define XP_WINCE
#endif
#endif

/*other platforms should be setup through configure*/

#endif


void gf_node_setup(GF_Node *p, u32 tag);

typedef struct _parent_list
{
	struct _parent_list *next;
	GF_Node *node;
} GF_ParentList;


/*internal flag reserved for NodeID*/
#define GF_NODE_IS_DEF			0x80000000	// 1<<31
/*internal flag reserved for activate/deactivate*/
#define GF_NODE_IS_DEACTIVATED	0x40000000	// 1<<30
/*internal flag reserved for node with scripting bindings*/
#define GF_NODE_HAS_BINDING		0x20000000	// 1<<29

#ifdef GF_CYCLIC_TRAVERSE_ON
#define GF_NODE_IN_TRAVERSE		0x10000000	//	1<<28
#define GF_NODE_INTERNAL_FLAGS	0xF0000000
#else
#define GF_NODE_INTERNAL_FLAGS	0xE0000000
#endif

struct _node_interactive_ext
{
	/*routes on eventOut, ISed routes, ... for VRML-based scene graphs
	THIS IS DYNAMICALLY CREATED*/
	GF_List *routes;

#ifdef GPAC_HAS_SPIDERMONKEY
	/*JS bindings if any - THIS IS DYNAMICALLY CREATED
	This speeds up field modif notification (script bindings are listed here)*/
	struct _node_js_binding *js_binding;
#endif

#ifndef GPAC_DISABLE_SVG
	/*event listeners - THIS IS DYNAMICALLY CREATED*/
	GF_DOMEventTarget *dom_evt;

	/* SVG animations are registered in the target node  - THIS IS DYNAMICALLY CREATED*/
	GF_List *animations;
#endif

};

typedef struct _nodepriv
{
	/*node type*/
	u16 tag;
	/*number of instances of this node in the graph*/
	u16 num_instances;
	/*node flags*/
	u32 flags;
	/*scenegraph holding the node*/
	struct __tag_scene_graph *scenegraph;

	/*user defined callback function */
	void (*UserCallback)(struct _base_node *node, void *render_stack, Bool node_destroy);
	/*user defined stack*/
	void *UserPrivate;

	/*list of all parent nodes (needed to invalidate parent tree)*/
	GF_ParentList *parents;
	
	/*holder for all interactive stuff - THIS IS DYNAMICALLY CREATED*/
	struct _node_interactive_ext *interact;
} NodePriv;


typedef struct __tag_node_id
{
	struct __tag_node_id *next;
	GF_Node *node;

	/*node ID*/
	u32 NodeID;
	/*node def name*/
	char *NodeName;
} NodeIDedItem;

typedef struct
{
	char *name;
	char *qname;
	u32 xmlns_id;
} GF_XMLNS;

struct __tag_scene_graph 
{
	/*used to discriminate between node and scenegraph*/
	u64 __reserved_null;

	/*all DEF nodes (explicit)*/
	NodeIDedItem *id_node, *id_node_last;

	/*pointer to the root node*/
	GF_Node *RootNode;

	/*nodes exported from this scene graph*/
	GF_List *exported_nodes;


	/*user private data*/
	void *userpriv;

	/*callback routines*/
	/*node callback*/
	void (*NodeCallback)(void *user_priv, u32 type, GF_Node *node, void *ctxdata);
	/*real scene time callback*/
	Double (*GetSceneTime)(void *userpriv);



	/*parent scene if any*/
	struct __tag_scene_graph *parent_scene;

	/*size info and pixel metrics - this is not used internally, however it helps when rendering
	and decoding modules don't know each-other (as in MPEG4)*/
	u32 width, height;
	Bool usePixelMetrics;

	Bool modified;

	/*application interface for javascript*/
	gf_sg_script_action script_action;
	void *script_action_cbck;

	/*script loader*/
	void (*script_load)(GF_Node *node);
	/*callback to JS upon node modif*/
	void (*on_node_modified)(struct __tag_scene_graph *sg, GF_Node *node, GF_FieldInfo *info, GF_Node *script);

#ifdef GF_SELF_REPLACE_ENABLE
	/*to detect replace scene from within conditionals*/
	Bool graph_has_been_reset;
#endif


	/*namespaces list. This list is used while parsing/dumping the tree to store the hierarchy of xmlns attributes in subtrees.
	It is a stack of GF_XMLNS structures pushed/popped at each element*/
	GF_List *ns;

	/*temp storage for name conversions*/
	char szNameBuffer[100];

#ifndef GPAC_DISABLE_VRML

	/*all routes available*/
	GF_List *Routes;

	/*when a proto is instanciated it creates its own scene graph. BIFS/VRML specify that the namespace is the same 
	(eg cannot reuse a NodeID or route name/ID), but this could be done differently by some other stds
	if NULL this is the main scenegraph*/
	struct _proto_instance *pOwningProto;

	/*all first-level protos of the graph (the only ones that can be instanciated in this graph)*/
	GF_List *protos;
	/*all first-level protos of the graph not currently registered - memory handling of graph only*/
	GF_List *unregistered_protos;

	/*routes to be activated (cascade model). This is used at the top-level graph only (eg
	proto routes use that too, ecept ISed fields). It is the app responsability to 
	correctly connect or browse scene graphs connected through Inline*/
	GF_List *routes_to_activate;

	/*since events may trigger deletion of objects we use a 2 step delete*/
	GF_List *routes_to_destroy;

	u32 simulation_tick;

	GF_SceneGraph *(*GetExternProtoLib)(void *userpriv, MFURL *lib_url);

	u32 max_defined_route_id;

	/*global qp used in BIFS coding*/
	GF_Node *global_qp;
#endif


#ifndef GPAC_DISABLE_SVG
	/*use stack as used in the dom_fire_event - this is only valid during an event fire, and may be NULL*/
	GF_List *use_stack;
	Bool abort_bubbling;


	GF_Mutex *dom_evt_mx;
	GF_DOMEventTarget dom_evt;
	u32 nb_evts_focus;
	u32 nb_evts_mouse;
	u32 nb_evts_key;
	u32 nb_evts_ui;
	u32 nb_evts_text;
	u32 nb_evts_smil;
	u32 nb_evts_mutation;
	u32 nb_evts_laser;
	u32 nb_evts_mae;
	u32 nb_evts_svg;
	u32 dom_evt_filter;

	GF_List *xlink_hrefs;
	GF_List *smil_timed_elements;
	GF_List *modified_smil_timed_elements;
	Bool update_smil_timing;

	/*listeners to add*/
	GF_List *listeners_to_add;

#ifdef GPAC_HAS_SPIDERMONKEY
	struct __tag_svg_script_ctx *svg_js;
#endif

#endif

#ifdef GPAC_HAS_SPIDERMONKEY
	GF_List *scripts;
	/*
			Note about reference counter

	  A DOM document (<=> scenegraph) may be created through javascript, and the JS object having created the 
	  document may be destroyed while the document is still in use. Moreover with XMLHttpRequest, the
	  "associated" doc is re-created at each request, but the script may still refer to the original document.
	  Since the document doesn't have a fixed owner, a reference counter is use and the scenegraph is kept alive 
	  until the last object using it is destroyed.

	If this counter is set to 0 when creating DOM Elements/..., this means the scenegraph is hold by an external
	entity (typically the player), and cannot be destroyed from the scripting engine
	*/
	u32 reference_count;
	/*DOM nodes*/
	GF_List *objects;
	/*DOM document*/
	struct JSObject *document;

	Bool dcci_doc;
#endif
};

void gf_sg_parent_setup(GF_Node *pNode);
void gf_sg_parent_reset(GF_Node *pNode);

void *gf_node_get_name_address(GF_Node*node);

void gf_node_changed_internal(GF_Node *node, GF_FieldInfo *field, Bool notify_scripts);

void gf_node_dirty_parent_graph(GF_Node *node);


/*BASE node (GF_Node) destructor*/
void gf_node_free(GF_Node *node);

/*node destructor dispatcher: redirects destruction for each graph type: VRML/MPEG4, X3D, SVG...)*/
void gf_node_del(GF_Node *node);

/*creates an undefined GF_Node - for parsing only*/
GF_Node *gf_sg_new_base_node();

#ifndef GPAC_DISABLE_VRML

struct _route
{
	u8 is_setup;
	/*set to true for proto IS fields*/
	u8 IS_route;

	u32 ID;
	char *name;

	/*scope of this route*/
	GF_SceneGraph *graph;
	u32 lastActivateTime;

	GF_Node *FromNode;
	GF_FieldInfo FromField;

	GF_Node *ToNode;
	GF_FieldInfo ToField;
};

void gf_sg_route_unqueue(GF_SceneGraph *sg, GF_Route *r);
/*returns TRUE if route modified destination node*/
Bool gf_sg_route_activate(GF_Route *r);
void gf_sg_route_queue(GF_SceneGraph *pSG, GF_Route *r);
void gf_sg_destroy_routes(GF_SceneGraph *sg);
void gf_sg_route_setup(GF_Route *r);


/*MPEG4 def*/
GF_Node *gf_sg_mpeg4_node_new(u32 NodeTag);
u32 gf_sg_mpeg4_node_get_child_ndt(GF_Node *node);
GF_Err gf_sg_mpeg4_node_get_field_index(GF_Node *node, u32 inField, u8 code_mode, u32 *fieldIndex);
GF_Err gf_sg_mpeg4_node_get_field(GF_Node *node, GF_FieldInfo *field);
u32 gf_sg_mpeg4_node_get_field_count(GF_Node *node, u8 code_mode);
void gf_sg_mpeg4_node_del(GF_Node *node);
const char *gf_sg_mpeg4_node_get_class_name(u32 NodeTag);
Bool gf_sg_mpeg4_node_get_aq_info(GF_Node *node, u32 FieldIndex, u8 *QType, u8 *AType, Fixed *b_min, Fixed *b_max, u32 *QT13_bits);
s32 gf_sg_mpeg4_node_get_field_index_by_name(GF_Node *node, char *name);

#ifndef GPAC_DISABLE_X3D
/*X3D def*/
GF_Node *gf_sg_x3d_node_new(u32 NodeTag);
GF_Err gf_sg_x3d_node_get_field(GF_Node *node, GF_FieldInfo *field);
u32 gf_sg_x3d_node_get_field_count(GF_Node *node);
void gf_sg_x3d_node_del(GF_Node *node);
const char *gf_sg_x3d_node_get_class_name(u32 NodeTag);
s32 gf_sg_x3d_node_get_field_index_by_name(GF_Node *node, char *name);
Bool gf_x3d_get_node_type(u32 NDT_Tag, u32 NodeTag);
#endif


/*VRML/X3D types*/
void gf_sg_mfint32_del(MFInt32 par);
void gf_sg_mffloat_del(MFFloat par);
void gf_sg_mfdouble_del(MFDouble par);
void gf_sg_mfbool_del(MFBool par);
void gf_sg_mfcolor_del(MFColor par);
void gf_sg_mfcolor_rgba_del(MFColorRGBA par);
void gf_sg_mfrotation_del(MFRotation par);
void gf_sg_mfstring_del(MFString par);
void gf_sg_mftime_del(MFTime par);
void gf_sg_mfvec2f_del(MFVec2f par);
void gf_sg_mfvec3f_del(MFVec3f par);
void gf_sg_mfvec4f_del(MFVec4f par);
void gf_sg_mfvec2d_del(MFVec2d par);
void gf_sg_mfvec3d_del(MFVec3d par);
void gf_sg_sfimage_del(SFImage im);
void gf_sg_sfstring_del(SFString par);
void gf_sg_mfscript_del(MFScript sc);
void gf_sg_sfcommand_del(SFCommandBuffer cb);
void gf_sg_sfurl_del(SFURL url);

Bool gf_sg_vrml_node_init(GF_Node *node);
Bool gf_sg_vrml_node_changed(GF_Node *node, GF_FieldInfo *field);

char *gf_node_vrml_dump_attribute(GF_Node *n, GF_FieldInfo *info);



//
//		MF Fields tools
//	WARNING: MF / SF Nodes CANNOT USE THESE FUNCTIONS
//

//return the size (in bytes) of fixed fields (buffers are handled as a char ptr , 1 byte)
u32 gf_sg_vrml_get_sf_size(u32 FieldType);

/*returns field type from its name*/
u32 gf_sg_field_type_by_name(char *fieldType);

/*clones the command in another graph - needed for uncompressed conditional in protos
if force_clone is not set and the target graph is the same as the command graph, nodes are just registered
with the new commands rather than cloned*/
GF_Command *gf_sg_vrml_command_clone(GF_Command *com, GF_SceneGraph *inGraph, Bool force_clone);


/*
			Proto node

*/

/*field interface to codec. This is used to do the node decoding, index translation
and all QP/BIFS Anim parsing. */
struct _protofield
{
	u8 EventType;
	u8 FieldType;
	/*if UseName, otherwise fieldN*/
	char *FieldName;

	/*default field value*/
	void *def_value;
	
	GF_Node *def_sfnode_value;
	GF_ChildNodeItem *def_mfnode_value;

	/*coding indexes*/
	u32 IN_index, OUT_index, DEF_index, ALL_index;

	/*Quantization*/
	u32 QP_Type, hasMinMax;
	void *qp_min_value, *qp_max_value;
	/*this is for QP=13 only*/
	u32 NumBits;

	/*Animation*/
	u32 Anim_Type;

	void *userpriv;
	void (*OnDelete)(void *ptr);
};

GF_ProtoFieldInterface *gf_sg_proto_new_field_interface(u32 FieldType);

/*set QP and anim info for a proto field (BIFS allows for that in proto coding)*/
GF_Err gf_bifs_proto_field_set_aq_info(GF_ProtoFieldInterface *field, u32 QP_Type, u32 hasMinMax, u32 QPSFType, void *qp_min_value, void *qp_max_value, u32 QP13_NumBits);

/*proto field instance. since it is useless to duplicate all coding info, names and the like
we seperate proto declaration and proto instanciation*/
typedef struct 
{
	u8 EventType;
	u8 FieldType;
	u8 has_been_accessed;
	void *field_pointer;
} GF_ProtoField;


struct _proto
{
	/*1 - Prototype interface*/
	u32 ID;
	char *Name;
	GF_List *proto_fields;

	/*pointer to parent scene graph*/
	struct __tag_scene_graph *parent_graph;
	/*pointer to proto scene graph*/
	struct __tag_scene_graph *sub_graph;

	/*2 - proto implementation as declared in the bitstream*/
	GF_List *node_code;

	/*num fields*/
	u32 NumIn, NumOut, NumDef, NumDyn;

	void *userpriv;
	void (*OnDelete)(void *ptr);

	/*URL of extern proto lib (if none, URL is empty)*/
	MFURL ExternProto;

	/*list of instances*/
	GF_List *instances;
};

/*proto field API*/
u32 gf_sg_proto_get_num_fields(GF_Node *node, u8 code_mode);
GF_Err gf_sg_proto_get_field(GF_Proto *proto, GF_Node *node, GF_FieldInfo *field);

enum
{
	GF_SG_PROTO_LOADED	=	1,
	GF_SG_PROTO_IS_GROUPING =	2,
};

typedef struct _proto_instance
{
	/*this is a node*/
	BASE_NODE

	/*Prototype interface for coding and field addressing*/
	GF_Proto *proto_interface;

	/*proto implementation at run-time (aka the state of the nodes may differ accross
	different instances of the proto)*/
	GF_List *fields;

	/*a proto doesn't have one root SFnode but a collection of nodes for implementation*/
	GF_List *node_code;

	/*node for proto rendering, first of all declared nodes*/
	GF_Node *RenderingNode;

	/*in case the PROTO is destroyed*/
	char *proto_name;

	/*scripts are loaded once all IS routes are activated and node code is loaded*/
	GF_List *scripts_to_load;

	u32 flags;
} GF_ProtoInstance;

/*destroy proto*/
void gf_sg_proto_del_instance(GF_ProtoInstance *inst);
GF_Err gf_sg_proto_get_field_index(GF_ProtoInstance *proto, u32 index, u32 code_mode, u32 *all_index);
Bool gf_sg_proto_get_aq_info(GF_Node *Node, u32 FieldIndex, u8 *QType, u8 *AType, Fixed *b_min, Fixed *b_max, u32 *QT13_bits);
GF_Err gf_sg_proto_get_field_ind_static(GF_Node *Node, u32 inField, u8 IndexMode, u32 *allField);
GF_Node *gf_sg_proto_create_node(GF_SceneGraph *scene, GF_Proto *proto, GF_ProtoInstance *from_inst);
void gf_sg_proto_instanciate(GF_ProtoInstance *proto_node);

/*get tag of first node in proto code - used for validation only*/
u32 gf_sg_proto_get_root_tag(GF_Proto *proto);


/*to call when a proto field has been modified (at creation or through commands, modifications through events 
are handled internally).
node can be the proto instance or a node from the proto code
this will call NodeChanged if needed, forward to proto/node or trigger any route if needed*/
void gf_sg_proto_propagate_event(GF_Node *node, u32 fieldIndex, GF_Node *from_node);

s32 gf_sg_proto_get_field_index_by_name(GF_Proto *proto, GF_Node *node, char *name);

GF_Node *gf_vrml_node_clone(GF_SceneGraph *inScene, GF_Node *orig, GF_Node *cloned_parent, char *inst_id_suffix);

#endif /*GPAC_DISABLE_VRML*/


/*specialized node unregister for Memory Commands - checks if the node(s) used in the command have been destroyed
during the reset. If so don't attempt to unregister the node*/
GF_Err gf_node_try_destroy(GF_SceneGraph *sg, GF_Node *pNode, GF_Node *parentNode);

#ifndef GPAC_DISABLE_SVG


/* reset functions for SVG types */
void gf_svg_reset_path(SVG_PathData path);
void gf_svg_reset_iri(GF_SceneGraph *sg, XMLRI*iri);
/* delete functions for SVG types */
void gf_svg_delete_paint		(GF_SceneGraph *sg, SVG_Paint *paint);
void gf_smil_delete_times		(GF_List *l);
void gf_svg_delete_points		(GF_List *l);
void gf_svg_delete_coordinates	(GF_List *l);
/*for keyTimes, keyPoints and keySplines*/
void gf_smil_delete_key_types	(GF_List *l);

u32 gf_node_get_attribute_count(GF_Node *node);
GF_Err gf_node_get_attribute_info(GF_Node *node, GF_FieldInfo *info) ;


/*SMIL anim tools*/

typedef struct __xlink_attrip_ptrs {
	XMLRI *href;
	SVG_ContentType *type;
	SVG_String *title;
	XMLRI *arcrole; 
	XMLRI *role;
	SVG_String *show;
	SVG_String *actuate;
} XLinkAttributesPointers;

typedef struct __smil_time_attrip_ptrs {
	SMIL_Times *begin, *end;
	SVG_Clock *clipBegin, *clipEnd;
	SMIL_Duration *dur;
	SMIL_RepeatCount *repeatCount;
	SMIL_Duration *repeatDur;
	SMIL_Restart *restart;
	SMIL_Fill *fill;
	SMIL_Duration *max;
	SMIL_Duration *min;
	struct _smil_timing_rti *runtime; /* contains values for runtime handling of the SMIL timing */
} SMILTimingAttributesPointers;

typedef struct __smil_sync_attrip_ptrs {
	SMIL_SyncBehavior *syncBehavior, *syncBehaviorDefault;
	SMIL_SyncTolerance *syncTolerance, *syncToleranceDefault;
	SVG_Boolean *syncMaster;
	XMLRI *syncReference;
} SMILSyncAttributesPointers;

typedef struct __smil_anim_attrip_ptrs {
	SMIL_AttributeName *attributeName; 
	SMIL_AttributeType *attributeType;
	SMIL_AnimateValue *to, *by, *from;
	SMIL_AnimateValues *values;
	SMIL_CalcMode *calcMode;
	SMIL_Accumulate *accumulate;
	SMIL_Additive *additive;
	SMIL_KeySplines *keySplines;
	SMIL_KeyTimes *keyTimes;
	SVG_TransformType *type;
	SVG_Boolean *lsr_enabled;

	SMIL_KeyPoints *keyPoints;
	SVG_String *origin;
	SVG_Rotate *rotate;
	SVG_PathData *path;
} SMILAnimationAttributesPointers;


typedef struct {
	GF_DOM_BASE_NODE

	/*shortcuts for xlink, anim, timing attributes*/
	XLinkAttributesPointers *xlinkp;
	SMILAnimationAttributesPointers *animp;
	SMILTimingAttributesPointers *timingp;
} SVGTimedAnimBaseElement;

GF_Err gf_node_animation_add(GF_Node *node, void *animation);
GF_Err gf_node_animation_del(GF_Node *node);
u32 gf_node_animation_count(GF_Node *node);
void *gf_node_animation_get(GF_Node *node, u32 i);
Bool gf_svg_is_inherit(GF_FieldInfo *a);
Bool gf_svg_is_current_color(GF_FieldInfo *a);

void gf_svg_reset_animate_values(SMIL_AnimateValues anim_values, GF_SceneGraph *sg);
void gf_svg_reset_animate_value(SMIL_AnimateValue anim_value, GF_SceneGraph *sg);

Bool gf_svg_is_timing_tag(u32 tag);
Bool gf_svg_is_animation_tag(u32 tag);
u32 gf_svg_get_modification_flags(SVG_Element *n, GF_FieldInfo *info);

Bool gf_svg_resolve_smil_times(GF_Node *anim, void *event_base_element, GF_List *smil_times, Bool is_end, const char *node_name);


/* SMIL Timing structures */
/* status of an SMIL timed element */ 
enum {
	SMIL_STATUS_WAITING_TO_BEGIN = 0,
	SMIL_STATUS_ACTIVE,
	SMIL_STATUS_POST_ACTIVE,
	SMIL_STATUS_FROZEN,
	SMIL_STATUS_DONE
};

typedef struct {
	u32 activation_cycle;
	u32 nb_iterations;

	/* for the case where min > simple duration*/
	Bool min_active;

	/* negative values mean indefinite */
	Double begin, 
		   end,
		   simple_duration, 
		   active_duration,
		   repeat_duration;

} SMIL_Interval;

struct _smil_timing_rti
{
	GF_Node *timed_elt;
	SMILTimingAttributesPointers *timingp;

	Double scene_time;
	Fixed normalized_simple_time;
	Bool force_reevaluation;

	/* SMIL element life-cycle status */
	u8 status;

	SMIL_Interval *current_interval;
	SMIL_Interval *next_interval;

	/* Evaluation of animations is postponed untill tree traversal, so that inherit values can be computed
	Other timed elements (audio, video, animation) are evaluated directly and do not require
	scene tree traversal.*/
	Bool postpone;

	void (*evaluate)(struct _smil_timing_rti *rti, Fixed normalized_simple_time, u32 state);
	u32 evaluate_status;

#if 0
	/* is called only when the timed element is active */
	void (*activation)(struct _smil_timing_rti *rti, Fixed normalized_simple_time);

	/* is called (possibly many times) when the timed element is frozen */
	void (*freeze)(struct _smil_timing_rti *rti, Fixed normalized_simple_time);

	/* is called (only once) when the timed element is restored */
	void (*restore)(struct _smil_timing_rti *rti, Fixed normalized_simple_time);

	/* is called only when the timed element is inactive and receives a fraction event, the second parameter is ignored */
	void (*fraction_activation)(struct _smil_timing_rti *rti, Fixed normalized_simple_time);
#endif
	/* simulated normalized simple time */
	Fixed fraction;

	Bool paused;
	Double media_duration;

	/* shortcut when this rti corresponds to an animation */
	struct _smil_anim_rti *rai;
};

void gf_smil_timing_init_runtime_info(GF_Node *timed_elt);
void gf_smil_timing_delete_runtime_info(GF_Node *timed_elt, SMIL_Timing_RTI *rti);
Fixed gf_smil_timing_get_normalized_simple_time(SMIL_Timing_RTI *rti, Double scene_time, Bool *force_end);
/*returns 1 if an animation changed a value in the rendering tree */
s32 gf_smil_timing_notify_time(SMIL_Timing_RTI *rti, Double scene_time);


/* SMIL Animation Structures */
/* This structure is used per animated attribute,
   it contains:
    - all the animations applying to the same attribute,
    - the specified value before any inheritance has been applied nor any animation started 
	    (as specified in the SVG document),
    - the presentation value passed from one animation to the next one, at the same level in the tree
	- a boolean indicating if the animated attribute is in fact a property
   
   and if the attribute is a property:
	- a pointer to presentation value passed from the previous level in the tree
	- a pointer to the value of the color property (for handling of 'currentColor'), from previous level in the tree
	- the location of the attribute in the elt structure when it was created 
	  (used for fast comparison of SVG properties when animating from/to/by/values/... inherited values)
*/
typedef struct {
	GF_List *anims;
	GF_FieldInfo specified_value;
	GF_FieldInfo presentation_value;
	Bool is_property;
	GF_FieldInfo parent_presentation_value;
	GF_FieldInfo current_color_value;
	void *orig_dom_ptr;
	/* flag set by any animation to inform other animations that there base value has changed */
	Bool presentation_value_changed;
	/* flag used for rendering */
	u32 dirty_flags;
	Bool dirty_parents;
} SMIL_AttributeAnimations;

/* This structure is per animation element, 
   it holds the result of the animation and 
   some info to make animation computation faster */
typedef struct _smil_anim_rti {
	SMIL_AttributeAnimations *owner;

	Bool is_first_anim;

	/* animation element */
	GF_Node *anim_elt;
	SMILAnimationAttributesPointers *animp;
	SMILTimingAttributesPointers *timingp;
	XLinkAttributesPointers *xlinkp;

	/* in case of animateTransform without from or to, the underlying value is the identity transform */
	GF_Matrix2D identity;
	GF_FieldInfo default_transform_value;

	/* result of the animation */
	GF_FieldInfo interpolated_value;

	/* last value of the animation, used in accumulation phase */
	/* normally the far pointer in the last specified value is a pointer to a real attribute value,
	   and there's no need to allocate a new value. Except if the last specified value is the last 
	   point in a path (animateMotion) in which case we allocate a matrix as last spec value,
	   which we need to delete (see animate-elem-202-t.svg). This is signaled if rai->path is not NULL*/
	GF_FieldInfo last_specified_value;

	/* temporary value needed when the type of 
	   the key values is different from the target attribute type */
	GF_FieldInfo tmp_value;

	/* the number of values in animations should be constant (unless updated with LASeR commands) 
	   we can store them to avoid computing them at each cycle */
	u32 values_count;
	u32 key_times_count;
	u32 key_points_count;
	u32 key_splines_count;


	/* In change detection mode, we test previous animation parameters to determine 
	   if a new evaluation of the animation will produce a different result. 
	   The result of these test is stored in interpolated_value_changed */
	Bool	change_detection_mode;
	Bool	interpolated_value_changed;
	s32		previous_key_index;
	u32		previous_keytime_index;
	Fixed	previous_coef;
	s32		previous_iteration;
	Bool	anim_done;

	GF_Path *path;
	u8 rotate;
	GF_PathIterator *path_iterator;
	Fixed length;

} SMIL_Anim_RTI;

void gf_smil_anim_init_node(GF_Node *node);
void gf_smil_anim_init_discard(GF_Node *node);
void gf_smil_anim_init_runtime_info(GF_Node *node);
void gf_smil_anim_delete_runtime_info(SMIL_Anim_RTI *rai);
void gf_smil_anim_delete_animations(GF_Node *e);
void gf_smil_anim_remove_from_target(GF_Node *anim, GF_Node *target);

void gf_sg_handle_dom_event(GF_Node *hdl, GF_DOM_Event *event, GF_Node *observer);
void gf_smil_setup_events(GF_Node *node);

void gf_smil_anim_reset_variables(SMIL_Anim_RTI *rai);
void gf_smil_anim_set_anim_runtime_in_timing(GF_Node *n);

void gf_smil_timing_pause(GF_Node *node);
void gf_smil_timing_resume(GF_Node *node);

#endif



/*
		Script node
*/

typedef struct 
{
	//extra script fields
	GF_List *fields;

	//BIFS coding stuff
	u32 numIn, numDef, numOut;

#ifdef GPAC_HAS_SPIDERMONKEY
	struct JSContext *js_ctx;
	struct JSObject *js_obj;
	struct JSObject *js_browser;
	/*all attached objects (eg, not created by the script) are stored here so that we don't
	allocate them again and again when getting properties. Garbage collection is performed (if needed)
	on these objects after each eventIn execution*/
	GF_List *js_cache;
	struct JSObject *event;
#endif

	void (*JS_PreDestroy)(GF_Node *node);
	void (*JS_EventIn)(GF_Node *node, GF_FieldInfo *in_field);

	Bool is_loaded;

} GF_ScriptPriv;

/*setup script stack*/
void gf_sg_script_init(GF_Node *node);
/*get script field*/
GF_Err gf_sg_script_get_field(GF_Node *node, GF_FieldInfo *info);
/*get effective field count per event mode*/
u32 gf_sg_script_get_num_fields(GF_Node *node, u8 IndexMode);
/*translate field index from inMode to ALL mode*/
GF_Err gf_sg_script_get_field_index(GF_Node *Node, u32 inField, u8 IndexMode, u32 *allField);
/*create dynamic fields in the clone*/
GF_Err gf_sg_script_prepare_clone(GF_Node *dest, GF_Node *orig);

struct _scriptfield
{
	u32 eventType;
	u32 fieldType;
	char *name;

	s32 IN_index, OUT_index, DEF_index;
	u32 ALL_index;

	//real field
	void *pField;

	Double last_route_time;
	Bool activate_event_out;
};


#ifdef GPAC_HAS_SPIDERMONKEY


#include <gpac/download.h>
#include <gpac/network.h>


struct JSContext *gf_sg_ecmascript_new(GF_SceneGraph *sg);
void gf_sg_ecmascript_del(struct JSContext *);

GF_Node *gf_sg_js_get_node(struct JSContext *c, struct JSObject *obj);

void gf_sg_script_init_sm_api(GF_ScriptPriv *sc, GF_Node *script);

/*GC thing type: 0 for jsval, 1 for jsstring, 2 for jsobject*/
enum
{
	GF_JSGC_VAL=0,
	GF_JSGC_STRING,
	GF_JSGC_OBJECT,
};

Bool gf_js_add_root(struct JSContext *cx, void *rp, u32 type);
Bool gf_js_add_named_root(struct JSContext *cx, void *rp, u32 type, const char *name);
Bool gf_js_remove_root(struct JSContext *cx, void *rp, u32 type);
void gf_js_vrml_flush_event_out(GF_Node *node, GF_ScriptPriv *priv);

#ifdef GPAC_HAS_SPIDERMONKEY
void gf_sg_lock_javascript(struct JSContext *c, Bool LockIt);
Bool gf_sg_try_lock_javascript(struct JSContext *c);
void gf_sg_js_call_gc(struct JSContext *c);
#endif /* GPAC_HAS_SPIDERMONKEY */


typedef struct 
{
	GF_FieldInfo field;
	GF_Node *owner;
	struct JSObject *obj;

	/*JS list for MFFields or NULL*/
	struct JSObject *js_list;

	/*pointer to the SFNode if this is an SFNode or MFNode[i] field */
	GF_Node *node;
	/*when creating MFnode from inside the script, the node list is stored here untill attached to an object*/
	GF_ChildNodeItem *temp_list;
	/*when not owned by a node*/
	void *field_ptr;
	
	/*cpontext in which the field was created*/
	struct JSContext *js_ctx;
	Bool is_rooted;
} GF_JSField;


struct _node_js_binding
{
	void *node;	/*GF_JSField for VRML, JSObject otherwise*/
	GF_List *fields;
};


#ifndef GPAC_DISABLE_SVG

typedef struct __tag_svg_script_ctx 
{
	Bool (*script_execute)(struct __tag_scene_graph *sg, char *utf8_script, GF_DOM_Event *event);
	Bool (*handler_execute)(GF_Node *n, GF_DOM_Event *event, GF_Node *observer, char *utf8_script);
	u32 nb_scripts;
	/*global script context for the scene*/
	struct JSContext *js_ctx;
	/*global object*/
	struct JSObject *global;
	/*global event object - used to update the associated DOMEvent (JS private stack) when dispatching events*/
	struct JSObject *event;

	Bool in_script;
	Bool force_gc;
} GF_SVGJS;

#endif	/*GPAC_DISABLE_SVG*/

/*initialize DOM Core (subset) + xmlHTTPRequest API. The global object MUST have private data storage
and its private data MUST be a scenegraph. This scenegraph is only used to create new documents
and setup the callback pointers*/
void dom_js_load(GF_SceneGraph *scene, struct JSContext *c, struct JSObject *global);
/*unloads the DOM core support (to be called upon destruction only, once the JSContext has been destroyed
to releases all resources used by DOM JS)*/
void dom_js_unload();
/*unloads DOM core before the JSContext is being destroyed */
void dom_js_pre_destroy(struct JSContext *c, GF_SceneGraph *sg, GF_Node *script_or_handler_node);

/*defines a new global object "document" of type Document*/
void dom_js_define_document(struct JSContext *c, struct JSObject *global, GF_SceneGraph *doc);
/*defines a new global object "evt" of type Event*/
struct JSObject *dom_js_define_event(struct JSContext *c, struct JSObject *global);

struct JSObject *gf_dom_new_event(struct JSContext *c);

struct JSObject *dom_js_get_node_proto(struct JSContext *c);
struct JSObject *dom_js_get_element_proto(struct JSContext *c);
struct JSObject *dom_js_get_document_proto(struct JSContext *c);
struct JSObject *dom_js_get_event_proto(struct JSContext *c);

void dom_set_class_selector(struct JSContext *c, /*struct JSClass*/void *(*get_element_class)(GF_Node *n), /*struct JSClass*/void *(*get_document_class)(GF_SceneGraph *n) );

enum
{
	GF_DOM_EXC_INDEX_SIZE_ERR = 1,
	GF_DOM_EXC_DOMSTRING_SIZE_ERR = 2,
	GF_DOM_EXC_HIERARCHY_REQUEST_ERR = 3,
	GF_DOM_EXC_WRONG_DOCUMENT_ERR = 4,
	GF_DOM_EXC_INVALID_CHARACTER_ERR = 5,
	GF_DOM_EXC_NO_DATA_ALLOWED_ERR = 6,
	GF_DOM_EXC_NO_MODIFICATION_ALLOWED_ERR = 7,
	GF_DOM_EXC_NOT_FOUND_ERR = 8,
	GF_DOM_EXC_NOT_SUPPORTED_ERR = 9,
	GF_DOM_EXC_INUSE_ATTRIBUTE_ERR = 10,
	GF_DOM_EXC_INVALID_STATE_ERR = 11,
	GF_DOM_EXC_SYNTAX_ERR = 12,
	GF_DOM_EXC_INVALID_MODIFICATION_ERR = 13,
	GF_DOM_EXC_NAMESPACE_ERR = 14,
	GF_DOM_EXC_INVALID_ACCESS_ERR = 15,
	GF_DOM_EXC_VALIDATION_ERR = 16,
	GF_DOM_EXC_TYPE_MISMATCH_ERR = 17,
};

int dom_throw_exception(struct JSContext *c, u32 code);

void gf_sg_handle_dom_event_for_vrml(GF_Node *hdl, GF_DOM_Event *event, GF_Node *observer);

void gf_sg_load_script_extensions(GF_SceneGraph *sg, struct JSContext *c, struct JSObject *obj, Bool unload);

Bool gf_sg_javascript_initialized();
#endif	/*GPAC_HAS_SPIDERMONKEY*/

GF_Err gf_sg_reload_xml_doc(const char *src, GF_SceneGraph *scene);

SVG_Element *gf_svg_create_node(u32 tag);
Bool gf_svg_node_init(GF_Node *node);
void gf_svg_node_del(GF_Node *node);
Bool gf_svg_node_changed(GF_Node *node, GF_FieldInfo *field);
const char *gf_xml_get_element_name(GF_Node *node);

SVGAttribute *gf_node_create_attribute_from_datatype(u32 data_type, u32 attribute_tag);

GF_Err gf_node_get_attribute_by_name(GF_Node *node, char *name, u32 xmlns_code, Bool create_if_not_found, Bool set_default, GF_FieldInfo *field);
void *gf_svg_get_property_pointer_from_tag(SVGPropertiesPointers *output_property_context, u32 prop_tag);
void *gf_svg_get_property_pointer(SVG_Element *elt, void *input_attribute,
								   SVGPropertiesPointers *output_property_context);

Bool gf_svg_is_property(GF_Node *node, GF_FieldInfo *target_attribute);

/*exported for LASeR paring*/
u32 svg_parse_point(SVG_Point *p, char *value_string);

/*activates node. This is used by LASeR:activate and whenever a node is inserted in the scene
through DOM*/
GF_Err gf_node_activate(GF_Node *node);
/*deactivates node. This is used by LASeR:deactivate and whenever a node is removed from the scene
through DOM*/
GF_Err gf_node_deactivate(GF_Node *node);

/*post a listener to be added - this is only used by LASeR:activate and DOM.addEventListener. This 
is to ensure that when a node is processing an event creating a new listener on this node, this listener
will not be triggered*/
void gf_sg_listener_post_add(GF_Node *obs, GF_Node *listener);
/*process all pending add_listener request*/
void gf_dom_listener_process_add(GF_SceneGraph *sg);
void gf_dom_listener_reset_defered(GF_SceneGraph *sg);


void gf_node_delete_attributes(GF_Node *node);


typedef GF_DOMNode XBL_Element;
const char *gf_xbl_get_element_name(u32 tag);
u32 gf_xbl_get_element_tag(const char *element_name);
XBL_Element *gf_xbl_create_node(u32 ElementTag);
u32 gf_xbl_get_attribute_tag(u32 element_tag, const char *attribute_name);
GF_DOMAttribute *gf_xbl_create_attribute(GF_DOMNode *elt, u32 tag);


GF_Node *gf_xml_node_clone(GF_SceneGraph *inScene, GF_Node *orig, GF_Node *cloned_parent, char *inst_id, Bool deep);

GF_Err gf_dom_listener_del(GF_Node *listener, GF_DOMEventTarget *target);

GF_DOMHandler *gf_dom_listener_build_ex(GF_Node *node, u32 event_type, u32 event_parameter, GF_Node *handler, GF_Node **out_listener);



#ifdef __cplusplus
}
#endif

#endif	/*_GF_SCENEGRAPH_DEV_H_*/

