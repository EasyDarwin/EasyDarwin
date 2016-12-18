/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005 
 *					All rights reserved
 *
 *  This file is part of GPAC / Authoring Tools sub-project
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

#ifndef _GF_ISMACRYP_H_
#define _GF_ISMACRYP_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/isomedia.h>

/*loads key and salt from a LOCAL gpac-DRM file (cf MP4Box doc)*/
GF_Err gf_ismacryp_gpac_get_info(u32 stream_id, char *drm_file, char *key, char *salt);

/*loads key and salt for MPEG4IP protected files*/
Bool gf_ismacryp_mpeg4ip_get_info(char *kms_uri, char *key, char *salt);

/*computes file hash. If file is ISO-based, computre hash according to OMA (P)DCF (without MutableDRMInformation box)*/
GF_Err gf_media_get_file_hash(const char *file, u8 hash[20]);
	
enum
{
	/*no selective encryption*/
	GF_ISMACRYP_SELENC_NONE = 0,
	/*only encrypts RAP samples*/
	GF_ISMACRYP_SELENC_RAP = 1,
	/*only encrypts non-RAP samples*/
	GF_ISMACRYP_SELENC_NON_RAP = 2,
	/*selective encryption of random samples*/
	GF_ISMACRYP_SELENC_RAND = 3,
	/*selective encryption of a random sample in given range*/
	GF_ISMACRYP_SELENC_RAND_RANGE = 4,
	/*selective encryption of first sample in given range*/
	GF_ISMACRYP_SELENC_RANGE = 5,
	/*encryption of all samples but the preview range*/
	GF_ISMACRYP_SELENC_PREVIEW = 6,
};

typedef struct
{
	/*0: ISMACryp - 1: OMA DRM*/
	u32 enc_type;
	u32 trackID;
	unsigned char key[16];
	unsigned char salt[16];

	/*the rest is only used for encryption*/
	char KMS_URI[5000];
	char Scheme_URI[5000];
	/*selecive encryption type*/
	u32 sel_enc_type;
	u32 sel_enc_range;
	/*IPMP signaling: 0: none, 1: IPMP, 2: IPMPX
	when IPMP signaling is enabled, the OD stream will be updated with
	IPMP Update commands*/
	u32 ipmp_type;
	/*if not set and IPMP enabled, defaults to TrackID*/
	u32 ipmp_desc_id;

	/*OMA extensions*/
	/*0: none, 1: AES CBC, 2: AES CTR*/
	u8 encryption;
	char TextualHeaders[5000];
	u32 TextualHeadersLen;
	char TransactionID[17];

} GF_TrackCryptInfo;

#if !defined(GPAC_DISABLE_MCRYPT) && !defined(GPAC_DISABLE_ISOM_WRITE)

/*encrypts track - logs, progress: info callbacks, NULL for stdout*/
GF_Err gf_ismacryp_encrypt_track(GF_ISOFile *mp4, GF_TrackCryptInfo *tci, void (*progress)(void *cbk, u64 done, u64 total), void *cbk);

/*decrypts track - logs, progress: info callbacks, NULL for stdout*/
GF_Err gf_ismacryp_decrypt_track(GF_ISOFile *mp4, GF_TrackCryptInfo *tci, void (*progress)(void *cbk, u64 done, u64 total), void *cbk);

/*decrypt a file 
@drm_file: location of DRM data (cf MP4Box doc).
@LogMsg: redirection for message or NULL for stdout
*/
GF_Err gf_ismacryp_decrypt_file(GF_ISOFile *mp4file, const char *drm_file);

/*Crypt a the file 
@drm_file: location of DRM data.
@LogMsg: redirection for message or NULL for stdout
*/
GF_Err gf_ismacryp_crypt_file(GF_ISOFile *mp4file, const char *drm_file);

#endif /*!defined(GPAC_DISABLE_MCRYPT) && !defined(GPAC_DISABLE_ISOM_WRITE)*/


#ifdef __cplusplus
}
#endif


#endif	/*_GF_ISMACRYP_H_*/

