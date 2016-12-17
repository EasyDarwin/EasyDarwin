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

#ifndef _GF_RINGBUFFER_H
#define _GF_RINGBUFFER_H

#include <gpac/tools.h>
#include <gpac/thread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  u8           *buf;
  volatile u32 write_ptr;
  volatile u32 read_ptr;
  u32          size;
  u32          size_mask;
  GF_Mutex *   mx;
}
GF_Ringbuffer ;


/*!
 * Creates a new ringbuffer with specified size. The caller has the
 * reponsability to free the ringbuffer using gf_ringbuffer_del()
 *
 * \param sz the ringbuffer size in bytes
 *
 * \return a pointer to a new ringbuffer if successful, NULL otherwise.
 */
GF_Ringbuffer * gf_ringbuffer_new(u32 sz);
 
/*!
 * Frees a previously allocated ringbuffer
 * \param ringbuffer The ringbuffer to free
 */
void gf_ringbuffer_del(GF_Ringbuffer * ringbuffer);

/*!
 * Reads bytes from ringbuffer
 * \param rb The ringbuffer to read from
 * \param dest The destination
 * \param szDest Size of destination
 * \return the number of bytes readen
 */
u32 gf_ringbuffer_read(GF_Ringbuffer *rb, u8 *dest, u32 szDest);

/*!
 * Return the number of bytes available for reading.  This is the
 * number of bytes in front of the read pointer and behind the write
 * pointer.
 * \param rb The ringbuffer
 * \return the number of bytes available for reading
 */
u32 gf_ringbuffer_available_for_read (GF_Ringbuffer * rb);

/*!
 * Copy at most sz bytes to rb from src.
 * \param rb The ringbuffer to write to
 * \param src The source buffer
 * \param sz the size of source
 * \return Returns the actual number of bytes copied, may be lower than sz if ringbuffer is already full
 */
u32 gf_ringbuffer_write (GF_Ringbuffer * rb, const u8 * src, u32 sz);
 
#ifdef __cplusplus
}
#endif
#endif /* _GF_RINGBUFFER_H */
