/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005
 *					All rights reserved
 *
 *  This file is part of GPAC / Crypto Tools sub-project
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

/*
	The GPAC crypto lib is a simplified version of libmcrypt - not all algos are included.
	Doc here is man mcrypt
	Original libmcrypt license
*/

/*
 * Copyright (C) 1998,1999,2000 Nikos Mavroyanopoulos
 * 
 * This library is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU Library General Public License as published 
 * by the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _GF_CRYPT_H_
#define _GF_CRYPT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gpac/tools.h>

#ifndef GPAC_DISABLE_MCRYPT


/*max number of possible key sizes for all supported modes*/
#define MAX_KEY_SIZES	4

/*crypto lib handler*/
typedef struct _tag_crypt_stream GF_Crypt;

/*supported modes (case insensitive): "CBC", "CFB", "CTR", "ECB", "nCFB", "nOFB", "OFB", "STREAM"*/
/*supported algos (case insensitive): 
	"AES-128" == "Rijndael-128"
	"AES-192" == "Rijndael-192"
	"AES-256" == "Rijndael-256"
	"DES", "3DES"
*/


/*opens crypto context - algo and mode SHALL NOT be NULL*/
GF_Crypt *gf_crypt_open(const char *algorithm, const char *mode);
/*close crypto context*/
void gf_crypt_close(GF_Crypt *gfc);

/* sets the state of the algorithm. Can be used only with block algorithms and certain modes like CBC, CFB etc. 
It is usefully if you want to restart or start a different encryption quickly. 
*/
GF_Err gf_crypt_set_state(GF_Crypt *gfc, const void *iv, int size);
/*gets the state of the algorithm. Can be used only certain modes and algorithms. 
The size will hold the size of the state and the state must have enough bytes to hold it.
*/
GF_Err gf_crypt_get_state(GF_Crypt *gfc, void *iv, int *size);
/*Returns 1 if the algorithm is a block algorithm or 0 if it is a stream algorithm.*/
Bool gf_crypt_is_block_algorithm(GF_Crypt *gfc);
/*Returns 1 if the mode is for use with block algorithms, otherwise it returns 0.*/
Bool gf_crypt_is_block_algorithm_mode(GF_Crypt *gfc);
/*Returns 1 if the mode outputs blocks of bytes or 0 if it outputs bytes. (eg. 1 for cbc and ecb, and 0 for cfb and stream)*/
Bool gf_crypt_is_block_mode(GF_Crypt *gfc);
/*Returns the block size of the algorithm specified by the encryption descriptor in bytes.*/
u32 gf_crypt_get_block_size(GF_Crypt *gfc);
/*Returns the maximum supported key size of the algorithm specified by the encryption descriptor in bytes.*/
u32 gf_crypt_get_key_size(GF_Crypt *gfc);
/*Returns the number of supported key sizes.
@keys: array of at least MAX_KEY_SIZES size - will hold the supported sizes*/
u32 gf_crypt_get_supported_key_sizes(GF_Crypt *gfc, u32 *key_sizes);
/*Returns size (in bytes) of the IV of the algorithm specified for the context. 
If it is '0' then the IV is ignored in that algorithm. 
IV is used in CBC, CFB, OFB modes, and in some algorithms in STREAM mode.
*/
u32 gf_crypt_get_iv_size(GF_Crypt *gfc);
/*Returns 1 if the mode needs an IV, 0 otherwise. 
Some 'stream' algorithms may need an IV even if the mode itself does not need an IV.
*/
Bool gf_crypt_mode_has_iv(GF_Crypt *gfc);

/*guess what these do...*/
const char *gf_crypt_get_algorithm_name(GF_Crypt *gfc);
u32 gf_crypt_get_algorithm_version(GF_Crypt *gfc);
const char *gf_crypt_get_mode_name(GF_Crypt *gfc);
u32 gf_crypt_get_mode_version(GF_Crypt *gfc);


/*
This function initializes all buffers for the specified context
@Lenofkey: key size in BYTES - maximum value of lenofkey should be the one obtained by 
calling gf_crypt_get_key_size() and every value smaller than this is legal.
@IV: usually size of the algorithms block size - get it by calling gf_crypt_get_iv_size().
	IV is ignored in ECB. IV MUST exist in CFB, CBC, STREAM, nOFB and OFB modes.
	It needs to be random and unique (but not secret). The same IV must be used
	for encryption/decryption. 
After calling this function you can use the descriptor for encryption or decryption (not both). 
*/
GF_Err gf_crypt_init(GF_Crypt *gfc, void *key, u32 lenofkey, const void *IV);
/*releases context buffers - you may call gf_crypt_init after that, or gf_crypt_close*/
void gf_crypt_deinit(GF_Crypt *gfc);
/*changes key and IV*/
GF_Err gf_crypt_set_key(GF_Crypt *gfc, void *key, u32 keysize, const void *iv);

/*
main encryption function. 
@Plaintext, @len: plaintext to encrypt - len should be  k*algorithms_block_size if used in a mode
which operated in blocks (cbc, ecb, nofb), or whatever when used in cfb or ofb which operate in streams.
The plaintext is replaced by the ciphertext. 
*/
GF_Err gf_crypt_encrypt(GF_Crypt *gfc, void *plaintext, int len);
/*decryption function. It is almost the same with gf_crypt_generic.*/
GF_Err gf_crypt_decrypt(GF_Crypt *gfc, void *ciphertext, int len);

/*various queries on both modes and algo*/
u32 gf_crypt_str_get_algorithm_version(const char *algorithm);
u32 gf_crypt_str_get_mode_version(const char *mode);
Bool gf_crypt_str_is_block_algorithm(const char *algorithm);
Bool gf_crypt_str_is_block_algorithm_mode(const char *algorithm);
Bool gf_crypt_str_is_block_mode(const char *mode);
u32 gf_crypt_str_module_get_algo_block_size(const char *algorithm);
u32 gf_crypt_str_module_get_algo_key_size(const char *algorithm);
u32 gf_crypt_str_get_algo_supported_key_sizes(const char *algorithm, int *keys);

#endif /*GPAC_DISABLE_MCRYPT*/


/*SHA1 from Christophe Devine*/
typedef struct
{
    u32 total[2];
    u32 state[5];
    u8 buffer[64];
} GF_SHA1Context;

/*
 * Core SHA-1 functions
 */
void gf_sha1_starts(GF_SHA1Context *ctx );
void gf_sha1_update(GF_SHA1Context *ctx, u8 *input, u32 length);
void gf_sha1_finish(GF_SHA1Context *ctx, u8 digest[20] );

/*
 * Output SHA-1(file contents), returns 0 if successful.
 */
int gf_sha1_file(const char *filename, u8 digest[20]);

/*
 * Output SHA-1(buf)
 */
void gf_sha1_csum(u8 *buf, u32 buflen, u8 digest[20]);

/*
 * Output HMAC-SHA-1(key,buf)
 */
void gf_sha1_hmac(u8 *key, u32 keylen, u8 *buf, u32 buflen, u8 digest[20]);


#ifdef __cplusplus
}
#endif

#endif	/*_GF_CRYPT_H_*/

