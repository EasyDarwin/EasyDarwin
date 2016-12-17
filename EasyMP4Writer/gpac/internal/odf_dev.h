/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005 
 *					All rights reserved
 *
 *  This file is part of GPAC / MPEG-4 ObjectDescriptor sub-project
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


#ifndef _GF_OD_DEV_H_
#define _GF_OD_DEV_H_

#include <gpac/mpeg4_odf.h>

/*read-write OD formatted strings*/
GF_Err gf_odf_read_url_string(GF_BitStream *bs, char **string, u32 *readBytes);
GF_Err gf_odf_write_url_string(GF_BitStream *bs, char *string);
u32 gf_odf_size_url_string(char *string);

/*descriptors base functions*/
GF_Descriptor *gf_odf_create_descriptor(u8 tag);
GF_Err gf_odf_delete_descriptor(GF_Descriptor *desc);
GF_Err gf_odf_parse_descriptor(GF_BitStream *bs, GF_Descriptor **desc, u32 *size);
GF_Err gf_odf_read_descriptor(GF_BitStream *bs, GF_Descriptor *desc, u32 DescSize);
GF_Err gf_odf_write_base_descriptor(GF_BitStream *bs, u8 tag, u32 size);
GF_Err gf_odf_write_descriptor(GF_BitStream *bs, GF_Descriptor *desc);
GF_Err gf_odf_size_descriptor(GF_Descriptor *desc, u32 *outSize);
GF_Err gf_odf_delete_descriptor_list(GF_List *descList);
GF_Err gf_odf_write_descriptor_list(GF_BitStream *bs, GF_List *descList);
GF_Err gf_odf_write_descriptor_list_filter(GF_BitStream *bs, GF_List *descList, u8 tag_only);
GF_Err gf_odf_size_descriptor_list(GF_List *descList, u32 *outSize);

/*handle lazy bitstreams where SizeOfInstance is always encoded on 4 bytes*/
s32 gf_odf_size_field_size(u32 size_desc);

GF_Err DumpDescList(GF_List *list, FILE *trace, u32 indent, const char *ListName, Bool XMTDump, Bool no_skip_empty);

/*IPMPX tools*/
u32 gf_ipmpx_array_size(GF_BitStream *bs, u32 *array_size);
void gf_ipmpx_write_array(GF_BitStream *bs, char *data, u32 data_len);

/*QoS qualifiers base functions*/
GF_Err gf_odf_parse_qos_qual(GF_BitStream *bs, GF_QoS_Default **qos_qual, u32 *qos_size);
void gf_odf_delete_qos_qual(GF_QoS_Default *qos);
GF_Err gf_odf_size_qos_qual(GF_QoS_Default *qos);
GF_Err gf_odf_write_qos_qual(GF_BitStream *bs, GF_QoS_Default *qos);

GF_Descriptor *gf_odf_new_iod();
GF_Descriptor *gf_odf_new_esd();
GF_Descriptor *gf_odf_new_dcd();
GF_Descriptor *gf_odf_new_slc(u8 predef);
GF_Descriptor *gf_odf_new_cc();
GF_Descriptor *gf_odf_new_cc_date();
GF_Descriptor *gf_odf_new_cc_name();
GF_Descriptor *gf_odf_new_ci();
GF_Descriptor *gf_odf_new_default();
GF_Descriptor *gf_odf_new_esd_inc();
GF_Descriptor *gf_odf_new_esd_ref();
GF_Descriptor *gf_odf_new_exp_text();
GF_Descriptor *gf_odf_new_pl_ext();
GF_Descriptor *gf_odf_new_ipi_ptr();
GF_Descriptor *gf_odf_new_ipmp();
GF_Descriptor *gf_odf_new_ipmp_ptr();
GF_Descriptor *gf_odf_new_kw();
GF_Descriptor *gf_odf_new_lang();
GF_Descriptor *gf_odf_new_isom_iod();
GF_Descriptor *gf_odf_new_isom_od();
GF_Descriptor *gf_odf_new_od();
GF_Descriptor *gf_odf_new_oci_date();
GF_Descriptor *gf_odf_new_oci_name();
GF_Descriptor *gf_odf_new_pl_idx();
GF_Descriptor *gf_odf_new_qos();
GF_Descriptor *gf_odf_new_rating();
GF_Descriptor *gf_odf_new_reg();
GF_Descriptor *gf_odf_new_short_text();
GF_Descriptor *gf_odf_new_smpte_camera();
GF_Descriptor *gf_odf_new_sup_cid();
GF_Descriptor *gf_odf_new_segment();
GF_Descriptor *gf_odf_new_mediatime();
GF_Descriptor *gf_odf_new_ipmp_tool_list();
GF_Descriptor *gf_odf_new_ipmp_tool();
GF_Descriptor *gf_odf_new_muxinfo();
GF_Descriptor *gf_odf_New_ElemMask();
GF_Descriptor *gf_odf_new_bifs_cfg();
GF_Descriptor *gf_odf_new_ui_cfg();
GF_Descriptor *gf_odf_new_laser_cfg();
GF_Descriptor *gf_odf_new_auxvid();


GF_Err gf_odf_del_iod(GF_InitialObjectDescriptor *iod);
GF_Err gf_odf_del_esd(GF_ESD *esd);
GF_Err gf_odf_del_dcd(GF_DecoderConfig *dcd);
GF_Err gf_odf_del_slc(GF_SLConfig *sl);
GF_Err gf_odf_del_cc(GF_CCDescriptor *ccd);
GF_Err gf_odf_del_cc_date(GF_CC_Date *cdd);
GF_Err gf_odf_del_cc_name(GF_CC_Name *cnd);
GF_Err gf_odf_del_ci(GF_CIDesc *cid);
GF_Err gf_odf_del_default(GF_DefaultDescriptor *dd);
GF_Err gf_odf_del_esd_inc(GF_ES_ID_Inc *esd_inc);
GF_Err gf_odf_del_esd_ref(GF_ES_ID_Ref *esd_ref);
GF_Err gf_odf_del_exp_text(GF_ExpandedTextual *etd);
GF_Err gf_odf_del_pl_ext(GF_PLExt *pld);
GF_Err gf_odf_del_ipi_ptr(GF_IPIPtr *ipid);
GF_Err gf_odf_del_ipmp(GF_IPMP_Descriptor *ipmp);
GF_Err gf_odf_del_ipmp_ptr(GF_IPMPPtr *ipmpd);
GF_Err gf_odf_del_kw(GF_KeyWord *kwd);
GF_Err gf_odf_del_lang(GF_Language *ld);
GF_Err gf_odf_del_isom_iod(GF_IsomInitialObjectDescriptor *iod);
GF_Err gf_odf_del_isom_od(GF_IsomObjectDescriptor *od);
GF_Err gf_odf_del_od(GF_ObjectDescriptor *od);
GF_Err gf_odf_del_oci_date(GF_OCI_Data *ocd);
GF_Err gf_odf_del_oci_name(GF_OCICreators *ocn);
GF_Err gf_odf_del_pl_idx(GF_PL_IDX *plid);
GF_Err gf_odf_del_qos(GF_QoS_Descriptor *qos);
GF_Err gf_odf_del_rating(GF_Rating *rd);
GF_Err gf_odf_del_reg(GF_Registration *reg);
GF_Err gf_odf_del_short_text(GF_ShortTextual *std);
GF_Err gf_odf_del_smpte_camera(GF_SMPTECamera *cpd);
GF_Err gf_odf_del_sup_cid(GF_SCIDesc *scid);
GF_Err gf_odf_del_segment(GF_Segment *sd);
GF_Err gf_odf_del_mediatime(GF_MediaTime *mt);
GF_Err gf_odf_del_ipmp_tool_list(GF_IPMP_ToolList *ipmptl);
GF_Err gf_odf_del_ipmp_tool(GF_IPMP_Tool *ipmp);
GF_Err gf_odf_del_muxinfo(GF_MuxInfo *mi);
GF_Err gf_odf_del_bifs_cfg(GF_BIFSConfig *desc);
GF_Err gf_odf_del_ui_cfg(GF_UIConfig *desc);
GF_Err gf_odf_del_laser_cfg(GF_LASERConfig *desc);
GF_Err gf_odf_del_auxvid(GF_AuxVideoDescriptor *ld);

GF_Err gf_odf_read_iod(GF_BitStream *bs, GF_InitialObjectDescriptor *iod, u32 DescSize);
GF_Err gf_odf_read_esd(GF_BitStream *bs, GF_ESD *esd, u32 DescSize);
GF_Err gf_odf_read_dcd(GF_BitStream *bs, GF_DecoderConfig *dcd, u32 DescSize);
GF_Err gf_odf_read_slc(GF_BitStream *bs, GF_SLConfig *sl, u32 DescSize);
GF_Err gf_odf_read_cc(GF_BitStream *bs, GF_CCDescriptor *ccd, u32 DescSize);
GF_Err gf_odf_read_cc_date(GF_BitStream *bs, GF_CC_Date *cdd, u32 DescSize);
GF_Err gf_odf_read_cc_name(GF_BitStream *bs, GF_CC_Name *cnd, u32 DescSize);
GF_Err gf_odf_read_ci(GF_BitStream *bs, GF_CIDesc *cid, u32 DescSize);
GF_Err gf_odf_read_default(GF_BitStream *bs, GF_DefaultDescriptor *dd, u32 DescSize);
GF_Err gf_odf_read_esd_inc(GF_BitStream *bs, GF_ES_ID_Inc *esd_inc, u32 DescSize);
GF_Err gf_odf_read_esd_ref(GF_BitStream *bs, GF_ES_ID_Ref *esd_ref, u32 DescSize);
GF_Err gf_odf_read_exp_text(GF_BitStream *bs, GF_ExpandedTextual *etd, u32 DescSize);
GF_Err gf_odf_read_pl_ext(GF_BitStream *bs, GF_PLExt *pld, u32 DescSize);
GF_Err gf_odf_read_ipi_ptr(GF_BitStream *bs, GF_IPIPtr *ipid, u32 DescSize);
GF_Err gf_odf_read_ipmp(GF_BitStream *bs, GF_IPMP_Descriptor *ipmp, u32 DescSize);
GF_Err gf_odf_read_ipmp_ptr(GF_BitStream *bs, GF_IPMPPtr *ipmpd, u32 DescSize);
GF_Err gf_odf_read_kw(GF_BitStream *bs, GF_KeyWord *kwd, u32 DescSize);
GF_Err gf_odf_read_lang(GF_BitStream *bs, GF_Language *ld, u32 DescSize);
GF_Err gf_odf_read_isom_iod(GF_BitStream *bs, GF_IsomInitialObjectDescriptor *iod, u32 DescSize);
GF_Err gf_odf_read_isom_od(GF_BitStream *bs, GF_IsomObjectDescriptor *od, u32 DescSize);
GF_Err gf_odf_read_od(GF_BitStream *bs, GF_ObjectDescriptor *od, u32 DescSize);
GF_Err gf_odf_read_oci_date(GF_BitStream *bs, GF_OCI_Data *ocd, u32 DescSize);
GF_Err gf_odf_read_oci_name(GF_BitStream *bs, GF_OCICreators *ocn, u32 DescSize);
GF_Err gf_odf_read_pl_idx(GF_BitStream *bs, GF_PL_IDX *plid, u32 DescSize);
GF_Err gf_odf_read_qos(GF_BitStream *bs, GF_QoS_Descriptor *qos, u32 DescSize);
GF_Err gf_odf_read_rating(GF_BitStream *bs, GF_Rating *rd, u32 DescSize);
GF_Err gf_odf_read_reg(GF_BitStream *bs, GF_Registration *reg, u32 DescSize);
GF_Err gf_odf_read_short_text(GF_BitStream *bs, GF_ShortTextual *std, u32 DescSize);
GF_Err gf_odf_read_smpte_camera(GF_BitStream *bs, GF_SMPTECamera *cpd, u32 DescSize);
GF_Err gf_odf_read_sup_cid(GF_BitStream *bs, GF_SCIDesc *scid, u32 DescSize);
GF_Err gf_odf_read_segment(GF_BitStream *bs, GF_Segment *sd, u32 DescSize);
GF_Err gf_odf_read_mediatime(GF_BitStream *bs, GF_MediaTime *mt, u32 DescSize);
GF_Err gf_odf_read_muxinfo(GF_BitStream *bs, GF_MuxInfo *mi, u32 DescSize);
GF_Err gf_odf_read_ipmp_tool_list(GF_BitStream *bs, GF_IPMP_ToolList *ipmptl, u32 DescSize);
GF_Err gf_odf_read_ipmp_tool(GF_BitStream *bs, GF_IPMP_Tool *ipmp, u32 DescSize);
GF_Err gf_odf_read_auxvid(GF_BitStream *bs, GF_AuxVideoDescriptor *ld, u32 DescSize);

GF_Err gf_odf_size_iod(GF_InitialObjectDescriptor *iod, u32 *outSize);
GF_Err gf_odf_size_esd(GF_ESD *esd, u32 *outSize);
GF_Err gf_odf_size_dcd(GF_DecoderConfig *dcd, u32 *outSize);
GF_Err gf_odf_size_slc(GF_SLConfig *sl, u32 *outSize);
GF_Err gf_odf_size_cc(GF_CCDescriptor *ccd, u32 *outSize);
GF_Err gf_odf_size_cc_date(GF_CC_Date *cdd, u32 *outSize);
GF_Err gf_odf_size_cc_name(GF_CC_Name *cnd, u32 *outSize);
GF_Err gf_odf_size_ci(GF_CIDesc *cid, u32 *outSize);
GF_Err gf_odf_size_default(GF_DefaultDescriptor *dd, u32 *outSize);
GF_Err gf_odf_size_esd_inc(GF_ES_ID_Inc *esd_inc, u32 *outSize);
GF_Err gf_odf_size_esd_ref(GF_ES_ID_Ref *esd_ref, u32 *outSize);
GF_Err gf_odf_size_exp_text(GF_ExpandedTextual *etd, u32 *outSize);
GF_Err gf_odf_size_pl_ext(GF_PLExt *pld, u32 *outSize);
GF_Err gf_odf_size_ipi_ptr(GF_IPIPtr *ipid, u32 *outSize);
GF_Err gf_odf_size_ipmp(GF_IPMP_Descriptor *ipmp, u32 *outSize);
GF_Err gf_odf_size_ipmp_ptr(GF_IPMPPtr *ipmpd, u32 *outSize);
GF_Err gf_odf_size_kw(GF_KeyWord *kwd, u32 *outSize);
GF_Err gf_odf_size_lang(GF_Language *ld, u32 *outSize);
GF_Err gf_odf_size_isom_iod(GF_IsomInitialObjectDescriptor *iod, u32 *outSize);
GF_Err gf_odf_size_isom_od(GF_IsomObjectDescriptor *od, u32 *outSize);
GF_Err gf_odf_size_od(GF_ObjectDescriptor *od, u32 *outSize);
GF_Err gf_odf_size_oci_date(GF_OCI_Data *ocd, u32 *outSize);
GF_Err gf_odf_size_oci_name(GF_OCICreators *ocn, u32 *outSize);
GF_Err gf_odf_size_pl_idx(GF_PL_IDX *plid, u32 *outSize);
GF_Err gf_odf_size_qos(GF_QoS_Descriptor *qos, u32 *outSize);
GF_Err gf_odf_size_rating(GF_Rating *rd, u32 *outSize);
GF_Err gf_odf_size_reg(GF_Registration *reg, u32 *outSize);
GF_Err gf_odf_size_short_text(GF_ShortTextual *std, u32 *outSize);
GF_Err gf_odf_size_smpte_camera(GF_SMPTECamera *cpd, u32 *outSize);
GF_Err gf_odf_size_sup_cid(GF_SCIDesc *scid, u32 *outSize);
GF_Err gf_odf_size_segment(GF_Segment *sd, u32 *outSize);
GF_Err gf_odf_size_mediatime(GF_MediaTime *mt, u32 *outSize);
GF_Err gf_odf_size_muxinfo(GF_MuxInfo *mi, u32 *outSize);
GF_Err gf_odf_size_ipmp_tool_list(GF_IPMP_ToolList *ipmptl, u32 *outSize);
GF_Err gf_odf_size_ipmp_tool(GF_IPMP_Tool *ipmp, u32 *outSize);
GF_Err gf_odf_size_auxvid(GF_AuxVideoDescriptor *ld, u32 *outSize);

GF_Err gf_odf_write_iod(GF_BitStream *bs, GF_InitialObjectDescriptor *iod);
GF_Err gf_odf_write_esd(GF_BitStream *bs, GF_ESD *esd);
GF_Err gf_odf_write_dcd(GF_BitStream *bs, GF_DecoderConfig *dcd);
GF_Err gf_odf_write_slc(GF_BitStream *bs, GF_SLConfig *sl);
GF_Err gf_odf_write_cc(GF_BitStream *bs, GF_CCDescriptor *ccd);
GF_Err gf_odf_write_cc_date(GF_BitStream *bs, GF_CC_Date *cdd);
GF_Err gf_odf_write_cc_name(GF_BitStream *bs, GF_CC_Name *cnd);
GF_Err gf_odf_write_ci(GF_BitStream *bs, GF_CIDesc *cid);
GF_Err gf_odf_write_default(GF_BitStream *bs, GF_DefaultDescriptor *dd);
GF_Err gf_odf_write_esd_inc(GF_BitStream *bs, GF_ES_ID_Inc *esd_inc);
GF_Err gf_odf_write_esd_ref(GF_BitStream *bs, GF_ES_ID_Ref *esd_ref);
GF_Err gf_odf_write_exp_text(GF_BitStream *bs, GF_ExpandedTextual *etd);
GF_Err gf_odf_write_pl_ext(GF_BitStream *bs, GF_PLExt *pld);
GF_Err gf_odf_write_ipi_ptr(GF_BitStream *bs, GF_IPIPtr *ipid);
GF_Err gf_odf_write_ipmp(GF_BitStream *bs, GF_IPMP_Descriptor *ipmp);
GF_Err gf_odf_write_ipmp_ptr(GF_BitStream *bs, GF_IPMPPtr *ipmpd);
GF_Err gf_odf_write_kw(GF_BitStream *bs, GF_KeyWord *kwd);
GF_Err gf_odf_write_lang(GF_BitStream *bs, GF_Language *ld);
GF_Err gf_odf_write_isom_iod(GF_BitStream *bs, GF_IsomInitialObjectDescriptor *iod);
GF_Err gf_odf_write_isom_od(GF_BitStream *bs, GF_IsomObjectDescriptor *od);
GF_Err gf_odf_write_od(GF_BitStream *bs, GF_ObjectDescriptor *od);
GF_Err gf_odf_write_oci_date(GF_BitStream *bs, GF_OCI_Data *ocd);
GF_Err gf_odf_write_oci_name(GF_BitStream *bs, GF_OCICreators *ocn);
GF_Err gf_odf_write_pl_idx(GF_BitStream *bs, GF_PL_IDX *plid);
GF_Err gf_odf_write_qos(GF_BitStream *bs, GF_QoS_Descriptor *qos);
GF_Err gf_odf_write_rating(GF_BitStream *bs, GF_Rating *rd);
GF_Err gf_odf_write_reg(GF_BitStream *bs, GF_Registration *reg);
GF_Err gf_odf_write_short_text(GF_BitStream *bs, GF_ShortTextual *std);
GF_Err gf_odf_write_smpte_camera(GF_BitStream *bs, GF_SMPTECamera *cpd);
GF_Err gf_odf_write_sup_cid(GF_BitStream *bs, GF_SCIDesc *scid);
GF_Err gf_odf_write_segment(GF_BitStream *bs, GF_Segment *sd);
GF_Err gf_odf_write_mediatime(GF_BitStream *bs, GF_MediaTime *mt);
GF_Err gf_odf_write_muxinfo(GF_BitStream *bs, GF_MuxInfo *mi);
GF_Err gf_odf_write_ipmp_tool_list(GF_BitStream *bs, GF_IPMP_ToolList *ipmptl);
GF_Err gf_odf_write_ipmp_tool(GF_BitStream *bs, GF_IPMP_Tool *ipmp);
GF_Err gf_odf_write_auxvid(GF_BitStream *bs, GF_AuxVideoDescriptor *ld);

GF_Descriptor *gf_odf_new_text_cfg();
GF_Err gf_odf_del_text_cfg(GF_TextConfig *desc);
GF_Descriptor *gf_odf_new_tx3g();
GF_Err gf_odf_del_tx3g(GF_TextSampleDescriptor *sd);

/*our commands base functions*/
GF_ODCom *gf_odf_create_command(u8 tag);
GF_Err gf_odf_delete_command(GF_ODCom *com);
GF_Err gf_odf_parse_command(GF_BitStream *bs, GF_ODCom **com, u32 *com_size);
GF_Err gf_odf_read_command(GF_BitStream *bs, GF_ODCom *com, u32 gf_odf_size_command);
GF_Err gf_odf_size_command(GF_ODCom *com, u32 *outSize);
GF_Err gf_odf_write_command(GF_BitStream *bs, GF_ODCom *com);

GF_ODCom *gf_odf_new_od_remove();
GF_ODCom *gf_odf_new_od_update();
GF_ODCom *gf_odf_new_esd_update();
GF_ODCom *gf_odf_new_esd_remove();
GF_ODCom *gf_odf_new_ipmp_update();
GF_ODCom *gf_odf_new_ipmp_remove();
GF_ODCom *gf_odf_new_base_command();

GF_Err gf_odf_del_od_remove(GF_ODRemove *ODRemove);
GF_Err gf_odf_del_od_update(GF_ODUpdate *ODUpdate);
GF_Err gf_odf_del_esd_update(GF_ESDUpdate *ESDUpdate);
GF_Err gf_odf_del_esd_remove(GF_ESDRemove *ESDRemove);
GF_Err gf_odf_del_ipmp_update(GF_IPMPUpdate *IPMPDUpdate);
GF_Err gf_odf_del_ipmp_remove(GF_IPMPRemove *IPMPDRemove);
GF_Err gf_odf_del_base_command(GF_BaseODCom *bcRemove);

GF_Err gf_odf_read_od_remove(GF_BitStream *bs, GF_ODRemove *odRem, u32 gf_odf_size_command);
GF_Err gf_odf_read_od_update(GF_BitStream *bs, GF_ODUpdate *odUp, u32 gf_odf_size_command);
GF_Err gf_odf_read_esd_update(GF_BitStream *bs, GF_ESDUpdate *esdUp, u32 gf_odf_size_command);
GF_Err gf_odf_read_esd_remove(GF_BitStream *bs, GF_ESDRemove *esdRem, u32 gf_odf_size_command);
GF_Err gf_odf_read_ipmp_update(GF_BitStream *bs, GF_IPMPUpdate *ipmpUp, u32 gf_odf_size_command);
GF_Err gf_odf_read_ipmp_remove(GF_BitStream *bs, GF_IPMPRemove *ipmpRem, u32 gf_odf_size_command);
GF_Err gf_odf_read_base_command(GF_BitStream *bs, GF_BaseODCom *bcRem, u32 gf_odf_size_command);

GF_Err gf_odf_size_od_remove(GF_ODRemove *odRem, u32 *outSize);
GF_Err gf_odf_size_od_update(GF_ODUpdate *odUp, u32 *outSize);
GF_Err gf_odf_size_esd_update(GF_ESDUpdate *esdUp, u32 *outSize);
GF_Err gf_odf_size_esd_remove(GF_ESDRemove *esdRem, u32 *outSize);
GF_Err gf_odf_size_ipmp_update(GF_IPMPUpdate *ipmpUp, u32 *outSize);
GF_Err gf_odf_size_ipmp_remove(GF_IPMPRemove *ipmpRem, u32 *outSize);
GF_Err gf_odf_size_base_command(GF_BaseODCom *bcRem, u32 *outSize);

GF_Err gf_odf_write_od_remove(GF_BitStream *bs, GF_ODRemove *odRem);
GF_Err gf_odf_write_od_update(GF_BitStream *bs, GF_ODUpdate *odUp);
GF_Err gf_odf_write_esd_update(GF_BitStream *bs, GF_ESDUpdate *esdUp);
GF_Err gf_odf_write_esd_remove(GF_BitStream *bs, GF_ESDRemove *esdRem);
GF_Err gf_odf_write_ipmp_update(GF_BitStream *bs, GF_IPMPUpdate *ipmpUp);
GF_Err gf_odf_write_ipmp_remove(GF_BitStream *bs, GF_IPMPRemove *ipmpRem);
GF_Err gf_odf_write_base_command(GF_BitStream *bs, GF_BaseODCom *bcRem);



/*dumping*/
GF_Err gf_odf_dump_iod(GF_InitialObjectDescriptor *iod, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_esd(GF_ESD *esd, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_dcd(GF_DecoderConfig *dcd, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_slc(GF_SLConfig *sl, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_cc(GF_CCDescriptor *ccd, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_cc_date(GF_CC_Date *cdd, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_cc_name(GF_CC_Name *cnd, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_ci(GF_CIDesc *cid, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_default(GF_DefaultDescriptor *dd, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_esd_inc(GF_ES_ID_Inc *esd_inc, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_esd_ref(GF_ES_ID_Ref *esd_ref, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_exp_text(GF_ExpandedTextual *etd, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_pl_ext(GF_PLExt *pld, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_ipi_ptr(GF_IPIPtr *ipid, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_ipmp(GF_IPMP_Descriptor *ipmp, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_ipmp_ptr(GF_IPMPPtr *ipmpd, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_kw(GF_KeyWord *kwd, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_lang(GF_Language *ld, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_isom_iod(GF_IsomInitialObjectDescriptor *iod, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_isom_od(GF_IsomObjectDescriptor *od, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_od(GF_ObjectDescriptor *od, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_oci_date(GF_OCI_Data *ocd, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_oci_name(GF_OCICreators *ocn, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_pl_idx(GF_PL_IDX *plid, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_qos(GF_QoS_Descriptor *qos, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_rating(GF_Rating *rd, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_reg(GF_Registration *reg, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_short_text(GF_ShortTextual *std, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_smpte_camera(GF_SMPTECamera *cpd, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_sup_cid(GF_SCIDesc *scid, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_segment(GF_Segment *sd, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_mediatime(GF_MediaTime *mt, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_muxinfo(GF_MuxInfo *mi, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_bifs_cfg(GF_BIFSConfig *dsi, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_laser_cfg(GF_LASERConfig *dsi, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_ui_cfg(GF_UIConfig *dsi, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_txtcfg(GF_TextConfig *desc, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_ipmp_tool_list(GF_IPMP_ToolList *tl, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_ipmp_tool(GF_IPMP_Tool*t, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_aux_vid(GF_AuxVideoDescriptor *ld, FILE *trace, u32 indent, Bool XMTDump);


GF_Err gf_odf_dump_od_update(GF_ODUpdate *com, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_od_remove(GF_ODRemove *com, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_esd_update(GF_ESDUpdate *com, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_esd_remove(GF_ESDRemove *com, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_ipmp_update(GF_IPMPUpdate *com, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_ipmp_remove(GF_IPMPRemove *com, FILE *trace, u32 indent, Bool XMTDump);
GF_Err gf_odf_dump_base_command(GF_BaseODCom *com, FILE *trace, u32 indent, Bool XMTDump);

#endif	/*_GF_OD_DEV_H_*/

