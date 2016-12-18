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


#ifndef _GF_SG_VRML_H_
#define _GF_SG_VRML_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/scenegraph.h>
#include <gpac/math.h>

/*
	All extensions for VRML/MPEG-4/X3D graph structure	
*/

/*reserved NDT for MPEG4 (match binary coding)*/
#define MPEG4_RESERVED_NDT		200

/*the NDTs used in X3D not defined in MPEG4*/
enum
{
	NDT_SFMetadataNode = MPEG4_RESERVED_NDT+1,
	NDT_SFFillPropertiesNode,
	NDT_SFX3DLinePropertiesNode,
	NDT_SFGeoOriginNode,
	NDT_SFHAnimNode,
	NDT_SFHAnimDisplacerNode,
	NDT_SFNurbsControlCurveNode,
	NDT_SFNurbsSurfaceNode,
	NDT_SFNurbsCurveNode
};

/*
	VRML / BIFS TYPES DEFINITION
*/

/*
				event types, as defined in the specs 
	this should not be needed by non binary codecs
*/
enum
{
	GF_SG_EVENT_FIELD		=	0,
	GF_SG_EVENT_EXPOSED_FIELD	=	1,
	GF_SG_EVENT_IN		=	2,
	GF_SG_EVENT_OUT		=	3,
	GF_SG_EVENT_UNKNOWN	=	4
};
const char *gf_sg_vrml_get_event_type_name(u32 EventType, Bool forX3D);

/*
				field coding mode

	BIFS defines the bitstream syntax contextually, and therefore sometimes refer to fields as indexed
  in the node ("all" mode) or just as a sub-set (in, out, def, dyn modes) of similar types
*/
enum
{
	/*all fields and events*/
	GF_SG_FIELD_CODING_ALL		=	0,
	/*defined fields (exposedField and Field)*/
	GF_SG_FIELD_CODING_DEF		=	1,
	/*input field (exposedField and eventIn)*/
	GF_SG_FIELD_CODING_IN		=	2,
	/*output field (exposedField and eventOut)*/
	GF_SG_FIELD_CODING_OUT		=	3,
	/*field that can be animated (subset of inFields) used in BIFS_Anim only*/
	GF_SG_FIELD_CODING_DYN		=	4
};

/*get the number of field in the given mode (BIFS specific)*/
u32 gf_node_get_num_fields_in_mode(GF_Node *Node, u8 IndexMode);

/*		SF Types	*/
typedef Bool SFBool;
typedef s32 SFInt32;
typedef s32 SFInt;
typedef Fixed SFFloat;
typedef Double SFDouble;

typedef struct
{
	char* buffer;
} SFString;

typedef Double SFTime;

typedef struct {
	Fixed	red;
	Fixed	green;
	Fixed	blue;
} SFColor;

typedef struct {
	Fixed	red;
	Fixed	green;
	Fixed	blue;
	Fixed	alpha;
} SFColorRGBA;

typedef struct {
	u32 OD_ID;
	char *url;
} SFURL;

typedef struct {
	Double	x;
	Double	y;
} SFVec2d;

typedef struct {
	Double	x;
	Double	y;
	Double	z;
} SFVec3d;

/*typedef's to main math tools*/
typedef struct __vec2f SFVec2f;
typedef struct __vec3f SFVec3f;
typedef struct __vec4f SFRotation;
typedef struct __vec4f SFVec4f;

typedef struct {
	u32 width;
	u32 height;
	u8 numComponents;
	unsigned char* pixels;
} SFImage;
typedef struct {
	u32 bufferSize;
	unsigned char* buffer;
	/*uncompressed command list*/
	GF_List *commandList;
} SFCommandBuffer;

/*Note on SFScript: the javascript or vrml script is handled in its decompressed (textual) format
since most JS interpreter work with text*/
typedef struct {
	unsigned char* script_text;
} SFScript;

typedef struct {
	GF_Node *node;
	u32 fieldIndex;
} SFAttrRef;

/*		MF Types	*/

/*generic MF field: all MF fields use the same syntax except MFNode which uses GF_List. You  can thus use
this structure to safely typecast MF field pointers*/
typedef struct {
	u32 count;
	char *array;
} GenMFField;

typedef struct {
	u32 count;
	s32* vals;
} MFInt32;
typedef struct {
	u32 count;
	s32* vals;
} MFInt;
typedef struct {
	u32 count;
	Fixed *vals;
} MFFloat;
typedef struct {
	u32 count;
	Double *vals;
} MFDouble;
typedef struct {
	u32 count;
	u32* vals;
} MFBool;
typedef struct {
	u32 count;
	SFColor* vals;
} MFColor;
typedef struct {
	u32 count;
	SFColorRGBA* vals;
} MFColorRGBA;
typedef struct {
	u32 count;
	SFRotation*	vals;
} MFRotation;
typedef struct {
	u32 count;
	Double* vals;
} MFTime;
typedef struct {
	u32 count;
	SFVec2f* vals;
} MFVec2f;
typedef struct {
	u32 count;
	SFVec2d* vals;
} MFVec2d;
typedef struct {
	u32 count;
	SFVec3f* vals;
} MFVec3f;
typedef struct {
	u32 count;
	SFVec3d* vals;
} MFVec3d;
typedef struct {
	u32 count;
	SFVec4f* vals;
} MFVec4f;

typedef struct {
	u32 count;
	SFURL* vals;
} MFURL;
typedef struct {
	u32 count;
	char** vals;
} MFString;
typedef struct {
	u32 count;
	SFScript *vals;
} MFScript;

typedef struct {
	u32 count;
	SFAttrRef* vals;
} MFAttrRef;


SFColorRGBA gf_sg_sfcolor_to_rgba(SFColor val);

/*field types, as defined in BIFS encoding (used for scripts and proto coding)*/
enum
{
	GF_SG_VRML_SFBOOL		=	0,
	GF_SG_VRML_SFFLOAT		=	1,
	GF_SG_VRML_SFTIME		=	2,
	GF_SG_VRML_SFINT32		=	3,
	GF_SG_VRML_SFSTRING		=	4,
	GF_SG_VRML_SFVEC3F		=	5,
	GF_SG_VRML_SFVEC2F		=	6,
	GF_SG_VRML_SFCOLOR		=	7,
	GF_SG_VRML_SFROTATION	=	8,
	GF_SG_VRML_SFIMAGE		=	9,
	GF_SG_VRML_SFNODE		=	10,
	/*TO CHECK*/
	GF_SG_VRML_SFVEC4F		=	11,

	/*used types in GPAC but not defined in the MPEG4 spec*/
	GF_SG_VRML_SFURL,
	GF_SG_VRML_SFSCRIPT,
	GF_SG_VRML_SFCOMMANDBUFFER,
	/*used types in X3D*/
	GF_SG_VRML_SFDOUBLE,
	GF_SG_VRML_SFCOLORRGBA,
	GF_SG_VRML_SFVEC2D,
	GF_SG_VRML_SFVEC3D,

	GF_SG_VRML_FIRST_MF		= 32,
	GF_SG_VRML_MFBOOL		= GF_SG_VRML_FIRST_MF,
	GF_SG_VRML_MFFLOAT,
	GF_SG_VRML_MFTIME,
	GF_SG_VRML_MFINT32,
	GF_SG_VRML_MFSTRING,
	GF_SG_VRML_MFVEC3F,
	GF_SG_VRML_MFVEC2F,
	GF_SG_VRML_MFCOLOR,
	GF_SG_VRML_MFROTATION,
	GF_SG_VRML_MFIMAGE,
	GF_SG_VRML_MFNODE,
	GF_SG_VRML_MFVEC4F,

	GF_SG_VRML_SFATTRREF	=	45,
	GF_SG_VRML_MFATTRREF	=	46,

	/*used types in GPAC but not defined in the MPEG4 spec*/
	GF_SG_VRML_MFURL,
	GF_SG_VRML_MFSCRIPT,
	GF_SG_VRML_MFCOMMANDBUFFER,

	/*used types in X3D*/
	GF_SG_VRML_MFDOUBLE,
	GF_SG_VRML_MFCOLORRGBA,
	GF_SG_VRML_MFVEC2D,
	GF_SG_VRML_MFVEC3D,

	/*special event only used in routes for binding eventOut/exposedFields to script functions. 
	 A route with ToField.FieldType set to this value holds a pointer to a function object. 
	*/
	GF_SG_VRML_SCRIPT_FUNCTION,


	GF_SG_VRML_UNKNOWN
};


Bool gf_sg_vrml_is_sf_field(u32 FieldType);

/*translates MF/SF to SF type*/
u32 gf_sg_vrml_get_sf_type(u32 FieldType);


/*Insert (+alloc) a slot in the MFField with a specified position for insertion and sets the ptr
to the newly created slot
@InsertAt is the 0-based index for the new slot
*/
GF_Err gf_sg_vrml_mf_insert(void *mf, u32 FieldType, void **new_ptr, u32 InsertAt);
/*remove all items of the MFField*/
GF_Err gf_sg_vrml_mf_reset(void *mf, u32 FieldType);

/*exported for URL handling in compositor*/
void gf_sg_mfurl_del(MFURL url);
void gf_sg_vrml_copy_mfurl(MFURL *dst, MFURL *src);
/*exported for 3D camera in compositor*/
SFRotation gf_sg_sfrotation_interpolate(SFRotation kv1, SFRotation kv2, Fixed fraction);

/*adds a new node to the "children" field
position is the 0-BASED index in the list of children, -1 means end of list (append)
DOES NOT CHECK CHILD/PARENT type compatibility
*/
GF_Err gf_node_insert_child(GF_Node *parent, GF_Node *new_child, s32 Position);
/*removes an existing node from the "children" field*/
GF_Err gf_node_remove_child(GF_Node *parent, GF_Node *toremove_child);
/*remove and replace given child by specified node. If node is NULL, only delete target node
position is the 0-BASED index in the list of children, -1 means end of list (append)
DOES NOT CHECK CHILD/PARENT type compatibility
*/
GF_Err gf_node_replace_child(GF_Node *node, GF_ChildNodeItem **container, s32 pos, GF_Node *newNode);

/*set proto loader - callback is the same as simulation time callback
	GetExternProtoLib is a pointer to the proto lib loader - this callback shall return the LPSCENEGRAPH
of the extern proto lib if found and loaded, NULL if not found and GF_SG_INTERNAL_PROTO for internal
hardcoded protos (extensions of MPEG-4 scene graph used for module deveopment)
*/
#define GF_SG_INTERNAL_PROTO	(GF_SceneGraph *) 0xFFFFFFFF


#ifndef GPAC_DISABLE_VRML




/*VRML grouping nodes macro - note we have inverted the children field to be 
compatible with the base GF_ParentNode node
All grouping nodes (with "children" field) implement the following: 

addChildren: chain containing nodes to add passed as eventIn - handled internally through ROUTE
void (*on_addChildren)(GF_Node *pNode): add eventIn signaler - this is handled internally by the scene_graph and SHALL 
NOT BE OVERRIDEN since it takes care of node(s) routing

removeChildren: chain containing nodes to remove passed as eventIn - handled internally through ROUTE

void (*on_removeChildren)(GF_Node *pNode): remove eventIn signaler - this is handled internally by the scene_graph and SHALL 
NOT BE OVERRIDEN since it takes care of node(s) routing

children: list of children SFNodes
*/

#define VRML_CHILDREN							\
	CHILDREN									\
	GF_ChildNodeItem *addChildren;							\
	void (*on_addChildren)(GF_Node *pNode, struct _route *route);		\
	GF_ChildNodeItem *removeChildren;						\
	void (*on_removeChildren)(GF_Node *pNode, struct _route *route);		\

typedef struct
{
	BASE_NODE
	VRML_CHILDREN
} GF_VRMLParent;

void gf_sg_vrml_parent_setup(GF_Node *pNode);
void gf_sg_vrml_parent_destroy(GF_Node *pNode);


Bool gf_node_in_table_by_tag(u32 tag, u32 NDTType);


const char *gf_sg_vrml_get_field_type_by_name(u32 FieldType);


/*
allocates a new field and gets it back. 
	NOTE:
			GF_SG_VRML_MFNODE will return a pointer to a GF_List structure (eg GF_List *)
			GF_SG_VRML_SFNODE will return NULL
*/
void *gf_sg_vrml_field_pointer_new(u32 FieldType);
/*deletes a field pointer (including SF an,d MF nodes)*/
void gf_sg_vrml_field_pointer_del(void *field, u32 FieldType);


/*adds at the end and gets the ptr*/
GF_Err gf_sg_vrml_mf_append(void *mf, u32 FieldType, void **new_ptr);
/*remove the desired item*/
GF_Err gf_sg_vrml_mf_remove(void *mf, u32 FieldType, u32 RemoveFrom);
/*alloc a fixed array*/
GF_Err gf_sg_vrml_mf_alloc(void *mf, u32 FieldType, u32 NbItems);
/*get the item in the array*/
GF_Err gf_sg_vrml_mf_get_item(void *mf, u32 FieldType, void **new_ptr, u32 ItemPos);

/*copies a field content EXCEPT SF/MFNode. Pointers to field shall be used
@dest, @orig: pointers to field
@FieldType: type of the field
*/
void gf_sg_vrml_field_copy(void *dest, void *orig, u32 FieldType);

/*clones a field content EXCEPT SF/MFNode. Pointers to field shall be used
@dest, @orig: pointers to field
@FieldType: type of the field
@inScene: target scene graph for SFCommandBuffers cloning
*/
void gf_sg_vrml_field_clone(void *dest, void *orig, u32 FieldType, GF_SceneGraph *inScene);

/*indicates whether 2 fields of same type EXCEPT SF/MFNode are equal
@dest, @orig: pointers to field
@FieldType: type of the field
*/
Bool gf_sg_vrml_field_equal(void *dest, void *orig, u32 FieldType);


/*GF_Route manip: routes are used to pass events between nodes. Event handling is managed by the scene graph
however only the nodes overloading the EventIn handler associated with the event will process the eventIn*/

/*creates a new route:
	@fromNode: @fromField: address of the eventOut field triggering the route
	@toNode: @toField: address of the destination eventIn field
NOTE: routes are automatically destroyed if either the target or origin node of the route is destroyed
*/
GF_Route *gf_sg_route_new(GF_SceneGraph *sg, GF_Node *fromNode, u32 fromField, GF_Node *toNode, u32 toField);

/*delete route*/
void gf_sg_route_del(GF_Route *route);
GF_Err gf_sg_route_del_by_id(GF_SceneGraph *sg,u32 routeID);

/*locate route by ID/name*/
GF_Route *gf_sg_route_find(GF_SceneGraph *sg, u32 RouteID);
GF_Route *gf_sg_route_find_by_name(GF_SceneGraph *sg, char *name);
/*assign route ID - fails if a route with same ID already exist*/
GF_Err gf_sg_route_set_id(GF_Route *route, u32 ID);
u32 gf_sg_route_get_id(GF_Route *route);
/*assign route name if desired*/
GF_Err gf_sg_route_set_name(GF_Route *route, char *name);
char *gf_sg_route_get_name(GF_Route *route);

/*retuns next available RouteID - Note this doesn't track inserted routes, that's the user responsability*/
u32 gf_sg_get_next_available_route_id(GF_SceneGraph *sg);
/*set max defined route ID used in the scene - used to handle RouteInsert commands
note that this must be called by the user to be effective,; otherwise the max route ID is computed
from the routes present in scene*/
void gf_sg_set_max_defined_route_id(GF_SceneGraph *sg, u32 ID);


/*activates all routes currently triggered - this follows the event cascade model of VRML/MPEG4:
	- routes are collected during eventOut generation
	- routes are activated. If eventOuts are generated during activation the cycle goes on.

  A route cannot be activated twice in the same simulation tick, hence this function shall be called 
  ONCE AND ONLY ONCE per simulation tick

Note that children scene graphs register their routes with the top-level graph, so only the main 
scene graph needs to be activated*/
void gf_sg_activate_routes(GF_SceneGraph *sg);


/*
				proto handling

	The lib allows you to construct prototype nodes as defined in VRML/MPEG4 by constructing 
	proto interfaces and instanciating them. An instanciated proto is handled as a single node for
	rendering, thus an application will never handle proto instances for rendering
*/

/*opaque handler for a proto object (declaration)*/
typedef struct _proto GF_Proto;
/*opaque handler for a proto field object (declaration)*/
typedef struct _protofield GF_ProtoFieldInterface;


/*retuns next available NodeID*/
u32 gf_sg_get_next_available_proto_id(GF_SceneGraph *sg);

/*proto constructor identified by ID/name in the given scene
2 protos in the same scene may not have the same ID/name

@unregistered: used for memory handling of scene graph only, the proto is not stored
in the graph main proto list but in an alternate list. Several protos with the same ID/Name can be stored unregistered
*/
GF_Proto *gf_sg_proto_new(GF_SceneGraph *inScene, u32 ProtoID, char *name, Bool unregistered);

/*destroy proto interface - can be used even if instances of the proto are still present*/
GF_Err gf_sg_proto_del(GF_Proto *proto);

/*used for memory handling of scene graph only. move proto from off-graph to in-graph or reverse*/
GF_Err gf_sg_proto_set_in_graph(GF_Proto *proto, GF_SceneGraph *inScene, Bool set_in);

/*returns graph associated with this proto. Such a graph cannot be used for rendering but is needed during
construction of proto dictionaries in case of nested protos*/
GF_SceneGraph *gf_sg_proto_get_graph(GF_Proto *proto);

/*get/set private data*/
void gf_sg_proto_set_private(GF_Proto *proto, void *ptr, void (*OnDelete)(void *ptr) );
void *gf_sg_proto_get_private(GF_Proto *proto);

/*add node code - a proto is build of several nodes, the first node is used for rendering
and the others are kept private. This set of nodes is refered to as the proto "node code"*/
GF_Err gf_sg_proto_add_node_code(GF_Proto *proto, GF_Node *pNode);

/*gets number of field in the proto interface*/
u32 gf_sg_proto_get_field_count(GF_Proto *proto);
/*locates field declaration by name*/
GF_ProtoFieldInterface *gf_sg_proto_field_find_by_name(GF_Proto *proto, char *fieldName);
/*locates field declaration by index (0-based)*/
GF_ProtoFieldInterface *gf_sg_proto_field_find(GF_Proto *proto, u32 fieldIndex);

/*creates a new field declaration in the proto. of given fieldtype and eventType
fieldName can be NULL, if so the name will be fieldN, N being the index of the created field*/
GF_ProtoFieldInterface *gf_sg_proto_field_new(GF_Proto *proto, u32 fieldType, u32 eventType, char *fieldName);

/*assign the node field to a field of the proto (the node field IS the proto field)
the node shall be a node of the proto scenegraph, and the fieldtype/eventType of both fields shall match
(except SF/MFString and MF/SFURL which are allowed) due to BIFS semantics*/
GF_Err gf_sg_proto_field_set_ised(GF_Proto *proto, u32 protoFieldIndex, GF_Node *node, u32 nodeFieldIndex);
/*set/get user private data for the proto field declaration*/
void gf_sg_proto_field_set_private(GF_ProtoFieldInterface *field, void *ptr, void (*OnDelete)(void *ptr) );
void *gf_sg_proto_field_get_private(GF_ProtoFieldInterface *field);
/*returns field info of the field - this is typically used to setup the default value of the field*/
GF_Err gf_sg_proto_field_get_field(GF_ProtoFieldInterface *field, GF_FieldInfo *info);

/*
	NOTE on proto instances:
		The proto instance is handled as an GF_Node outside the scenegraph lib, and is manipulated with the same functions 
		as an GF_Node 
		The proto instance may or may not be loaded. 
		An unloaded instance only contains the proto instance fields 
		A loaded instance contains the proto instance fields plus all the proto code (Nodes, routes) and 
		will load any scripts present in it. This allows keeping the memory usage of proto very low, especially
		when nested protos (protos used as building blocks of their parent proto) are used.
*/

/*creates the proto interface without the proto code.*/
GF_Node *gf_sg_proto_create_instance(GF_SceneGraph *sg, GF_Proto *proto);

/*lodes code in this instance - all subprotos are automatically created, thus you must only instanciate
top-level protos. VRML/BIFS doesn't allow for non top-level proto instanciation in the main graph
All nodes created in this proto will be forwarded to the app for initialization*/
GF_Err gf_sg_proto_load_code(GF_Node *proto_inst);

/*locate a prototype definition by ID or by name. when looking by name, ID is ignored*/
GF_Proto *gf_sg_find_proto(GF_SceneGraph *sg, u32 ProtoID, char *name);

/*deletes all protos in given scene - does NOT delete instances of protos, only the proto object is destroyed */
GF_Err gf_sg_delete_all_protos(GF_SceneGraph *scene);


/*tools for hardcoded proto*/
/*gets proto of this node - if the node is not a prototype instance, returns NULL*/
GF_Proto *gf_node_get_proto(GF_Node *node);
/*returns the ID of the proto*/
u32 gf_sg_proto_get_id(GF_Proto *proto);
/*returns proto name*/
const char *gf_sg_proto_get_class_name(GF_Proto *proto);

/*Returns 1 if the given field is ISed to a startTime/stopTime field (MPEG-4 specific for updates)*/
Bool gf_sg_proto_field_is_sftime_offset(GF_Node *node, GF_FieldInfo *field);

/*set an ISed field in a proto instance (not a proto) - this is needed with dynamic node creation inside a proto
instance (conditionals)*/
GF_Err gf_sg_proto_instance_set_ised(GF_Node *protoinst, u32 protoFieldIndex, GF_Node *node, u32 nodeFieldIndex);

/*returns root node (the one and only one being traversed) of this proto instance if any*/
GF_Node *gf_node_get_proto_root(GF_Node *node);

/*returns parent ProtoInstance node if this node is in a proto*/
GF_Node *gf_node_get_proto_parent(GF_Node *node);

/*indicates proto field has been parsed and its value is valid - this is needed for externProtos not specifying default
values*/
void gf_sg_proto_mark_field_loaded(GF_Node *proto_inst, GF_FieldInfo *info);

/*
			JavaScript tools
*/

/*script fields type don't have the same value as the bifs ones...*/
enum
{
	GF_SG_SCRIPT_TYPE_FIELD = 0,
	GF_SG_SCRIPT_TYPE_EVENT_IN,
	GF_SG_SCRIPT_TYPE_EVENT_OUT,
};

typedef struct _scriptfield GF_ScriptField;
/*creates new sript field - script fields are dynamically added to the node, and thus can be accessed through the
same functions as other GF_Node fields*/
GF_ScriptField *gf_sg_script_field_new(GF_Node *script, u32 eventType, u32 fieldType, const char *name);
/*retrieves field info, usefull to get the field index*/
GF_Err gf_sg_script_field_get_info(GF_ScriptField *field, GF_FieldInfo *info);

/*activate eventIn for script node - needed for BIFS field replace*/
void gf_sg_script_event_in(GF_Node *node, GF_FieldInfo *in_field);



/*set the scene proto loader function for externProto - callback is the same as the scene callback*/
void gf_sg_set_proto_loader(GF_SceneGraph *scene, GF_SceneGraph *(*GetExternProtoLib)(void *SceneCallback, MFURL *lib_url));

/*get a pointer to the MF URL field for externProto info - DO NOT TOUCH THIS FIELD*/
MFURL *gf_sg_proto_get_extern_url(GF_Proto *proto);

/*signals eventOut has been set. FieldIndex/eventName identify the eventOut field. Routes are automatically triggered
when the event is signaled*/
void gf_node_event_out(GF_Node *node, u32 FieldIndex);
void gf_node_event_out_str(GF_Node *node, const char *eventName);


/*exported for parsers*/
u32 gf_node_mpeg4_type_by_class_name(const char *node_name);

#ifndef GPAC_DISABLE_X3D
u32 gf_node_x3d_type_by_class_name(const char *node_name);
#endif


#endif /*GPAC_DISABLE_VRML*/


/*returns 1 if proto is a hardcoded proto acting as a grouping node*/
Bool gf_node_proto_is_grouping(GF_Node *node);

/*tags a hardcoded proto as being a grouping node*/
GF_Err gf_node_proto_set_grouping(GF_Node *node);


#ifdef __cplusplus
}
#endif



#endif /*_GF_SG_VRML_H_*/
