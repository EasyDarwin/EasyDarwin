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

#ifndef _GF_UNICODE_H_
#define _GF_UNICODE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *	\file <gpac/math.h>
 *	\brief math and trigo functions.
 */

#include <gpac/setup.h>

/*!
 * \brief Unicode conversion from UTF-8 to UCS-4
 * \param ucs4_buf The UCS-4 buffer to fill
 * \param utf8_len The length of the UTF-8 buffer
 * \param utf8_buf The buffer containing the UTF-8 data
 * \return the length of the ucs4_buf. Note that the ucs4_buf should be allocated by parent and should be at least utf8_len * 4
 */
u32 utf8_to_ucs4 (u32 *ucs4_buf, u32 utf8_len, unsigned char *utf8_buf);
	
#ifdef __cplusplus
}
#endif

#endif		/*_GF_UNICODE_H_*/

