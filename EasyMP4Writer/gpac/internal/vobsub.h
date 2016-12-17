/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) by  Falco (Ivan Vecera) 2006
 *					All rights reserved
 *
 *  This file is part of GPAC / Media Tools sub-project
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


#ifndef _GF_VOBSUB_H_
#define _GF_VOBSUB_H_

#include <gpac/tools.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VOBSUBIDXVER 7

typedef struct _tag_vobsub_pos
{
	u64  filepos;
	u64  start;
	u64  stop;
} vobsub_pos;

typedef struct _tag_vobsub_lang
{
	u32      id;
	char    *name;
	GF_List *subpos;
} vobsub_lang;

typedef struct _tag_vobsub_file
{
	u32         width;
	u32         height;
	u8          palette[16][4];
	u32         num_langs;
	vobsub_lang langs[32];
} vobsub_file;

GFINLINE static void vobsub_trim_ext(char *filename)
{
	char *pos = strrchr(filename, '.');

	if (pos != NULL) {
		if (!stricmp(pos, ".idx") || !stricmp(pos, ".sub")) {
			*pos = '\0';
		}
	}
}

s32    vobsub_lang_name(u16 id);
char  *vobsub_lang_id(char *name);
GF_Err vobsub_read_idx(FILE *file, vobsub_file *vobsub, int *version);
void   vobsub_free(vobsub_file *vobsub);
GF_Err vobsub_get_subpic_duration(char *data, u32 psize, u32 dsize, u32 *duration);
GF_Err vobsub_packetize_subpicture(FILE *fsub, u64 pts, char *data, u32 dataSize);

#ifdef __cplusplus
}
#endif

#endif /* _GF_VOBSUB_H_ */
