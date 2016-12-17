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


#ifndef _GF_OD_PARSE_COMMON_H_
#define _GF_OD_PARSE_COMMON_H_
#include <gpac/setup.h>

#define GET_U8(field) { u32 d; if (strstr(val, "0x")) { ret += sscanf(val, "%x", &d); if (ret) field = (u8) d; } else { ret += sscanf(val, "%u", &d); if (ret) field = (u8) d; }	}	
#define GET_U16(field) { u16 d; if (strstr(val, "0x")) { ret += sscanf(val, "%hx", &d); if (ret) field = d; } else { ret += sscanf(val, "%hu", &d); if (ret) field = d; }	}
#define GET_S16(field) { s16 d; if (strstr(val, "0x")) { ret += sscanf(val, "%hx", (u16*)&d); if (ret) field = d; } else { ret += sscanf(val, "%hd", &d); if (ret) field = d; }	}	
#define GET_U32(field) { u32 d; if (strstr(val, "0x")) { ret += sscanf(val, "%x", &d); if (ret) field = d; } else { ret += sscanf(val, "%ud", &d); if (ret) field = d; }	}	
#define GET_S32(field) { s32 d; if (strstr(val, "0x")) { ret += sscanf(val, "%x", (u32*)&d); if (ret) field = d; } else { ret += sscanf(val, "%d", &d); if (ret) field = d; }	}	
#define GET_BOOL(field) { ret = 1; field = (!stricmp(val, "true") || !stricmp(val, "1")) ? 1 : 0; }
#define GET_U64(field) { u64 d; if (strstr(val, "0x")) { ret += sscanf(val, LLX, &d); if (ret) field = d; } else { ret += sscanf(val, LLU, &d); if (ret) field = d; }	}

#define GET_DOUBLE(field) { Float v; ret = 1; sscanf(val, "%f", &v); field = (Double) v;}
#define GET_STRING(field) { ret = 1; field = gf_strdup(val); if (val[0] == '"') strcpy(field, val+1); if (field[strlen(field)-1] == '"') field[strlen(field)-1] = 0; }


#endif	/* _GF_OD_PARSE_COMMON_H_  */

