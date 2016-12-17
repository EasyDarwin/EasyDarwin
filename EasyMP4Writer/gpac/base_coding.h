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

#ifndef _GF_BASE_CODING_H_
#define _GF_BASE_CODING_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *	\file <gpac/base_coding.h>
 *	\brief Base coding functions.
 */

/*!
 *	\addtogroup bascod_grp base coding
 *	\ingroup utils_grp
 *	\brief Base Coding functions
 *
 *	This section documents the base encoding and decoding functions of the GPAC framework.
 *	@{
 */

#include <gpac/tools.h>

/*!
 *\brief base64 encoder
 *
 *Encodes a data buffer to Base64
 *\param in_buffer input data buffer
 *\param in_buffer_size input data buffer size
 *\param out_buffer output Base64 buffer location
 *\param out_buffer_size output Base64 buffer allocated size
 *\return size of the encoded Base64 buffer
 *\note the encoded data buffer is not NULL-terminated.
 */
u32 gf_base64_encode(char *in_buffer, u32 in_buffer_size, char *out_buffer, u32 out_buffer_size);
/*!
 *\brief base64 decoder
 *
 *Decodes a Base64 buffer to data
 *\param in_buffer input Base64 buffer
 *\param in_buffer_size input Base64 buffer size
 *\param out_buffer output data buffer location
 *\param out_buffer_size output data buffer allocated size
 *\return size of the decoded buffer
 */
u32 gf_base64_decode(char *in_buffer, u32 in_buffer_size, char *out_buffer, u32 out_buffer_size);

/*!
 *\brief base16 encoder
 *
 *Encodes a data buffer to Base16
 *\param in_buffer input data buffer
 *\param in_buffer_size input data buffer size
 *\param out_buffer output Base16 buffer location
 *\param out_buffer_size output Base16 buffer allocated size
 *\return size of the encoded Base16 buffer
 *\note the encoded data buffer is not NULL-terminated.
 */
u32 gf_base16_encode(char *in_buffer, u32 in_buffer_size, char *out_buffer, u32 out_buffer_size);

/*!
 *\brief base16 decoder
 *
 *Decodes a Base16 buffer to data
 *\param in_buffer input Base16 buffer
 *\param in_buffer_size input Base16 buffer size
 *\param out_buffer output data buffer location
 *\param out_buffer_size output data buffer allocated size
 *\return size of the decoded buffer
 */
u32 gf_base16_decode(char *in_buffer, u32 in_buffer_size, char *out_buffer, u32 out_buffer_size);

/*! @} */

#ifdef __cplusplus
}
#endif


#endif		/*_GF_BASE_CODING_H_*/

