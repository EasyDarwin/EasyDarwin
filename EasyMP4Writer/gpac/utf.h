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

#ifndef _GF_UTF_H_
#define _GF_UTF_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *	\file <gpac/utf.h>
 *	\brief UTF functions.
 */

/*!
 *	\addtogroup utf_grp UTF
 *	\ingroup utils_grp
 *	\brief UTF encoding functions
 *
 *This section documents the UTF functions of the GPAC framework.\n
 *The wide characters in GPAC are unsignad shorts, in other words GPAC only supports UTF8 and UTF16 coding styles.
 *\note these functions are just ports of libutf8 library tools into GPAC.
 *	@{
 */

#include <gpac/tools.h>

/*!
 *\brief wide-char to multibyte conversion
 *
 *Converts a wide-char string to a multibyte string
 *\param dst multibyte destination buffer
 *\param dst_len multibyte destination buffer size
 *\param srcp address of the wide-char string. This will be set to the next char to be converted in the input buffer if not enough space in the destination, or NULL if conversion was completed.
 *\return length (in byte) of the multibyte string or -1 if error.
 */
size_t gf_utf8_wcstombs(char* dst, size_t dst_len, const unsigned short** srcp);
/*converts UTF8 string to wide char string - returns (-1) if error. set @srcp to next char to be
converted if not enough space*/
/*!
 *\brief multibyte to wide-char conversion
 *
 *Converts a multibyte string to a wide-char string 
 *\param dst wide-char destination buffer
 *\param dst_len wide-char destination buffer size
 *\param srcp address of the multibyte character buffer. This will be set to the next char to be converted in the input buffer if not enough space in the destination, or NULL if conversion was completed.
 *\return length (in unsigned short) of the wide-char string or -1 if error.
 */
size_t gf_utf8_mbstowcs(unsigned short* dst, size_t dst_len, const char** srcp);
/*!
 *\brief wide-char string length
 *
 *Returns the length in character of a wide-char string
 *\param s the wide-char string
 *\return the wide-char string length
 */
size_t gf_utf8_wcslen(const unsigned short *s);

/*!
 *\brief string bidi reordering
 *
 *Performs a simple reordering of words in the string based on each word direction, so that glyphs are sorted in display order.
 *\param utf_string the wide-char string
 *\param len the len of the wide-char string
 *\return 1 if the main direction is right-to-left, 0 otherwise
 */
Bool gf_utf8_reorder_bidi(u16 *utf_string, u32 len);

/*! @} */

#ifdef __cplusplus
}
#endif


#endif		/*_GF_UTF_H_*/

