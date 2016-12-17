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

#ifndef _GF_BITSTREAM_H_
#define _GF_BITSTREAM_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *	\file <gpac/bitstream.h>
 *	\brief bitstream functions.
 */

/*!
 *	\addtogroup bs_grp bitstream
 *	\ingroup utils_grp
 *	\brief BitStream object
 *
 *	This section documents the bitstream object of the GPAC framework.
 *	\note Unless specified, all functions assume Big-Endian ordering of data in the bitstream.
 *	@{
 */

#include <gpac/tools.h>


enum
{
	GF_BITSTREAM_READ = 0,
	GF_BITSTREAM_WRITE
};

typedef struct __tag_bitstream GF_BitStream;

/*!
 *	\brief bitstream constructor
 *
 *	Constructs a bitstream from a buffer (read or write mode)
 *	\param buffer buffer to read or write. In WRITE mode, this can be NULL to let the bitstream object dynamically allocate memory, in which case the size param is ignored.
 *	\param size size of the buffer given. 
 *	\param mode operation mode for this bitstream: GF_BITSTREAM_READ for read, GF_BITSTREAM_WRITE for write.
 *	\return new bitstream object
 *	\note In write mode on an existing data buffer, data overflow is never signaled but simply ignored, it is the caller responsability to ensure it 
 *	does not write more than possible.
 */
GF_BitStream *gf_bs_new(const char *buffer, u64 size, u32 mode);
/*!
 *	\brief bitstream constructor from file handle
 *
 * Creates a bitstream from a file handle. 
 * \param f handle of the file to use. This handle must be created with binary mode.
 *	\param mode operation mode for this bitstream: GF_BITSTREAM_READ for read, GF_BITSTREAM_WRITE for write.
 *	\return new bitstream object
 *	\note - You have to open your file in the appropriated mode:\n
 *	- GF_BITSTREAM_READ: bitstream is constructed for reading\n
 *	- GF_BITSTREAM_WRITE: bitstream is constructed for writing\n
 *	\note - you may use any of these modes for a file with read/write access.
 *	\warning RESULTS ARE UNEXPECTED IF YOU TOUCH THE FILE WHILE USING THE BITSTREAM.
 */
GF_BitStream *gf_bs_from_file(FILE *f, u32 mode);
/*!
 *	\brief bitstream constructor from file handle
 *
 * Deletes the bitstream object. If the buffer was created by the bitstream, it is deleted if still present.
 */
void gf_bs_del(GF_BitStream *bs);

/*!
 *	\brief integer reading
 *
 *	Reads an integer coded on a number of bit.
 *	\param bs the target bitstream 
 *	\param nBits the number of bits to read
 *	\return the integer value read.
 */
u32 gf_bs_read_int(GF_BitStream *bs, u32 nBits);
/*!
 *	\brief large integer reading
 *
 *	Reads a large integer coded on a number of bit bigger than 32.
 *	\param bs the target bitstream 
 *	\param nBits the number of bits to read
 *	\return the large integer value read.
 */
u64 gf_bs_read_long_int(GF_BitStream *bs, u32 nBits);
/*!
 *	\brief float reading
 *
 *	Reads a float coded as IEEE 32 bit format.
 *	\param bs the target bitstream 
 *	\return the float value read.
 */
Float gf_bs_read_float(GF_BitStream *bs);
/*!
 *	\brief double reading
 *
 *	Reads a double coded as IEEE 64 bit format.
 *	\param bs the target bitstream 
 *	\return the double value read.
 */
Double gf_bs_read_double(GF_BitStream *bs);
/*!
 *	\brief data reading
 *
 *	Reads a data buffer
 *	\param bs the target bitstream 
 *	\param data the data buffer to be filled
 *	\param nbBytes the amount of bytes to read
 *	\return the number of bytes actually read.
 *	\warning the data buffer passed must be large enough to hold the desired amount of bytes.
 */
u32 gf_bs_read_data(GF_BitStream *bs, char *data, u32 nbBytes);

/*!
 *	\brief align char reading
 *
 *	Reads an integer coded on 8 bits starting at a byte boundary in the bitstream.
 *	\warning you must not use this function if the bitstream is not aligned
 *	\param bs the target bitstream 
 *	\return the char value read.
 */
u32 gf_bs_read_u8(GF_BitStream *bs);
/*!
 *	\brief align short reading
 *
 *	Reads an integer coded on 16 bits starting at a byte boundary in the bitstream.
 *	\warning you must not use this function if the bitstream is not aligned
 *	\param bs the target bitstream 
 *	\return the short value read.
 */
u32 gf_bs_read_u16(GF_BitStream *bs);
/*!
 *	\brief align 24-bit integer reading
 *
 *	Reads an integer coded on 24 bits starting at a byte boundary in the bitstream.
 *	\warning you must not use this function if the bitstream is not aligned
 *	\param bs the target bitstream 
 *	\return the integer value read.
 */
u32 gf_bs_read_u24(GF_BitStream *bs);
/*!
 *	\brief align integer reading
 *
 *	Reads an integer coded on 32 bits starting at a byte boundary in the bitstream.
 *	\warning you must not use this function if the bitstream is not aligned
 *	\param bs the target bitstream 
 *	\return the integer value read.
 */
u32 gf_bs_read_u32(GF_BitStream *bs);
/*!
 *	\brief align large integer reading
 *
 *	Reads an integer coded on 64 bits starting at a byte boundary in the bitstream.
 *	\warning you must not use this function if the bitstream is not aligned
 *	\param bs the target bitstream 
 *	\return the large integer value read.
 */
u64 gf_bs_read_u64(GF_BitStream *bs);
/*!
 *	\brief little endian integer reading
 *
 *	Reads an integer coded on 32 bits in little-endian order.
 *	\param bs the target bitstream 
 *	\return the integer value read.
 */
u32 gf_bs_read_u32_le(GF_BitStream *bs);
/*!
 *	\brief little endian integer reading
 *
 *	Reads an integer coded on 16 bits in little-endian order.
 *	\param bs the target bitstream 
 *	\return the integer value read.
 */
u16 gf_bs_read_u16_le(GF_BitStream *bs);


/*!
 *	\brief variable length integer reading
 *
 *	Reads an integer coded on a variable number of 4-bits chunks. The number of chunks is given by the number of non-0 bits at the begining.
 *	\param bs the target bitstream 
 *	\return the integer value read.
 */
u32 gf_bs_read_vluimsbf5(GF_BitStream *bs);

/*!
 *	\brief bit position
 *
 *	Returns current bit position in the bitstream - only works in memory mode.
 *	\param bs the target bitstream 
 *	\return the integer value read.
 */
u32 gf_bs_get_bit_offset(GF_BitStream *bs);

/*!
 *	\brief current bit position
 *
 *	Returns bit position in the current byte of the bitstream - only works in memory mode.
 *	\param bs the target bitstream 
 *	\return the integer value read.
 */
u32 gf_bs_get_bit_position(GF_BitStream *bs);


/*!
 *	\brief integer writing
 *
 *	Writes an integer on a given number of bits.
 *	\param bs the target bitstream 
 *	\param value the integer to write
 *	\param nBits number of bits used to code the integer
 */
void gf_bs_write_int(GF_BitStream *bs, s32 value, s32 nBits);
/*!
 *	\brief large integer writing
 *
 *	Writes an integer on a given number of bits greater than 32.
 *	\param bs the target bitstream 
 *	\param value the large integer to write
 *	\param nBits number of bits used to code the integer
 */
void gf_bs_write_long_int(GF_BitStream *bs, s64 value, s32 nBits);
/*!
 *	\brief float writing
 *
 *	Writes a float in IEEE 32 bits format.
 *	\param bs the target bitstream 
 *	\param value the float to write
 */
void gf_bs_write_float(GF_BitStream *bs, Float value);
/*!
 *	\brief double writing
 *
 *	Writes a double in IEEE 64 bits format.
 *	\param bs the target bitstream 
 *	\param value the double to write
 */
void gf_bs_write_double(GF_BitStream *bs, Double value);
/*!
 *	\brief data writing
 *
 *	Writes a data buffer.
 *	\param bs the target bitstream 
 *	\param data the data to write
 *	\param nbBytes number of data bytes to write
 */
u32 gf_bs_write_data(GF_BitStream *bs, const char *data, u32 nbBytes);

/*!
 *	\brief align char writing
 *
 *	Writes an integer on 8 bits starting at a byte boundary in the bitstream.
 *	\warning you must not use this function if the bitstream is not aligned
 *	\param bs the target bitstream 
 *	\param value the char value to write
 */
void gf_bs_write_u8(GF_BitStream *bs, u32 value);
/*!
 *	\brief align short writing
 *
 *	Writes an integer on 16 bits starting at a byte boundary in the bitstream.
 *	\warning you must not use this function if the bitstream is not aligned
 *	\param bs the target bitstream 
 *	\param value the short value to write
 */
void gf_bs_write_u16(GF_BitStream *bs, u32 value);
/*!
 *	\brief align 24-bits integer writing
 *
 *	Writes an integer on 24 bits starting at a byte boundary in the bitstream.
 *	\warning you must not use this function if the bitstream is not aligned
 *	\param bs the target bitstream 
 *	\param value the integer value to write
 */
void gf_bs_write_u24(GF_BitStream *bs, u32 value);
/*!
 *	\brief align integer writing
 *
 *	Writes an integer on 32 bits starting at a byte boundary in the bitstream.
 *	\warning you must not use this function if the bitstream is not aligned
 *	\param bs the target bitstream 
 *	\param value the integer value to write
 */
void gf_bs_write_u32(GF_BitStream *bs, u32 value);
/*!
 *	\brief align large integer writing
 *
 *	Writes an integer on 64 bits starting at a byte boundary in the bitstream.
 *	\warning you must not use this function if the bitstream is not aligned
 *	\param bs the target bitstream 
 *	\param value the large integer value to write
 */
void gf_bs_write_u64(GF_BitStream *bs, u64 value);
/*!
 *	\brief little endian integer writing
 *
 *	Writes an integer on 32 bits in little-endian order.
 *	\param bs the target bitstream
 *	\param value the integer value to write
 */
void gf_bs_write_u32_le(GF_BitStream *bs, u32 value);
/*!
 *	\brief little endian short writing
 *
 *	Writes an integer on 16 bits in little-endian order.
 *	\param bs the target bitstream
 *	\param value the short value to write
 */
void gf_bs_write_u16_le(GF_BitStream *bs, u32 value);

/*!
 *	\brief end of bitstream management
 *
 *	Assigns a notification callback function for end of stream signaling in read mode
 *	\param bs the target bitstream
 *	\param EndOfStream the notification function to use
 *	\param par opaque user data passed to the bitstream
 */
void gf_bs_set_eos_callback(GF_BitStream *bs, void (*EndOfStream)(void *par), void *par);

/*!
 *	\brief bitstream alignment
 *
 *	Aligns bitstream to next byte boundary. In write mode, this will write 0 bit values until alignment.
 *	\param bs the target bitstream
 *	\return the number of bits read/written until alignment
 */
u8 gf_bs_align(GF_BitStream *bs);
/*!
 *	\brief capacity query
 *
 *	Returns the number of bytes still available in the bitstream in read mode.
 *	\param bs the target bitstream
 *	\return the number of bytes still available in read mode, -1 in write modes.
 */
u64 gf_bs_available(GF_BitStream *bs);
/*!
 *	\brief buffer fetching
 *
 *	Fetches the internal bitstream buffer in write mode. If a buffer was given at the bitstream construction, or if the bitstream is in read mode, this does nothing.
 *	\param bs the target bitstream
 *	\param output address of a memory block to be allocated for bitstream data.
 *	\param outSize set to the size of the allocated memory block.
 *	\note 
	* It is the user responsability to destroy the allocated buffer
	* Once this function has been called, the internal bitstream buffer is reseted.
 */
void gf_bs_get_content(GF_BitStream *bs, char **output, u32 *outSize);
/*!
 *	\brief byte skipping
 *
 *	Skips bytes in the bitstream. In Write mode, this will write the 0 integer value for memory-based bitstreams or seek the stream
 for file-based bitstream.
 *	\param bs the target bitstream
 *	\param nbBytes the number of bytes to skip
 */
void gf_bs_skip_bytes(GF_BitStream *bs, u64 nbBytes);

/*!
 *\brief bitstream seeking
 *
 *Seeks the bitstream to a given offset after the begining of the stream. This will perform alignment of the bitstream in all modes.
 *\warning Results are unpredictable if seeking beyond the bitstream end is performed.
 *\param bs the target bitstream
 *\param offset buffer/file offset to seek to
 */
GF_Err gf_bs_seek(GF_BitStream *bs, u64 offset);

/*!
 *\brief bitstream truncation
 *
 *Truncates the bitstream at the current position
 *\param bs the target bitstream
 */
void gf_bs_truncate(GF_BitStream *bs);

/*!
 *\brief bit peeking 
 *
 *Peeks a given number of bits (read without moving the position indicator) for read modes only.
 *\param bs the target bitstream
 *\param numBits the number of bits to peek
 *\param byte_offset
	* if set, bitstream is aligned and moved from byte_offset before peeking (byte-aligned picking)
	* otherwise, bitstream is not aligned and bits are peeked from current state
 *\return the integer value read
*/
u32 gf_bs_peek_bits(GF_BitStream *bs, u32 numBits, u32 byte_offset);

/*!
 *\brief bit reservoir query
 *
 * Queries the number of bits available in read mode.
 *\param bs the target bitstream
 *\return number of available bits if position is in the last byte of the buffer/stream, 8 otherwise
 */
u8 gf_bs_bits_available(GF_BitStream *bs);
/*!
 *\brief position query
 *
 *Returns the reading/writting position in the buffer/file.
 *\param bs the target bitstream
 *\return the read/write position of the bitstream
 */
u64 gf_bs_get_position(GF_BitStream *bs);
/*!
 *\brief size query
 *
 *Returns the size of the associated buffer/file.
 *\param bs the target bitstream
 *\return the size of the bitstream
 */
u64 gf_bs_get_size(GF_BitStream *bs);
/*!
 *\brief file-based size query
 *
 *Returns the size of a file-based bitstream and force a seek to end of file. This is used in case the file handle
 *describes a file being constructed on disk while being read?
 *
 *\param bs the target bitstream
 *\return the disk size of the associated file
 */
u64 gf_bs_get_refreshed_size(GF_BitStream *bs);



/*! @} */

#ifdef __cplusplus
}
#endif


#endif		/*_GF_BITSTREAM_H_*/

