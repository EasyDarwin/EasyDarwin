/*
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 1999-2008 Apple Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 *
 */
 /*
	 File:       md5digest.cpp

	 Contains:   Implements the function declared in md5digest.h
 */

#include "md5.h"
#include "md5digest.h"
#include "StrPtrLen.h"
#include <string.h>

static StrPtrLen sColon(":", 1);
static StrPtrLen sMD5Sess("md5-sess", 8);
static StrPtrLen sQopAuth("auth", 4);
static StrPtrLen sQopAuthInt("auth-int", 8);

// allocates memory for hashStr->Ptr
void HashToString(unsigned char aHash[kHashLen], StrPtrLen* hashStr) {
	UInt16 i;
	UInt8 hexDigit;
	// Allocating memory
	char* str = new char[kHashHexLen + 1];
	str[kHashHexLen] = 0;

	for (i = 0; i < kHashLen; i++) {
		hexDigit = (aHash[i] >> 4) & 0xF;
		str[i * 2] = (hexDigit <= 9) ? (hexDigit + '0') : (hexDigit + 'a' - 10);
		hexDigit = aHash[i] & 0xF;
		str[i * 2 + 1] = (hexDigit <= 9) ? (hexDigit + '0') : (hexDigit + 'a' - 10);
	}

	hashStr->Ptr = str;
	hashStr->Len = kHashHexLen;
}

// allocates memory for hashA1Hex16Bit->Ptr
void CalcMD5HA1(StrPtrLen* userName,
	StrPtrLen* realm,
	StrPtrLen* userPassword,
	StrPtrLen* hashA1Hex16Bit
)
{
	// parameters must be valid pointers 
	// It is ok if parameter->Ptr is NULL as long as parameter->Len is 0
	Assert(userName);
	Assert(realm);
	Assert(userPassword);
	Assert(hashA1Hex16Bit);
	Assert(hashA1Hex16Bit->Ptr == NULL); //This is the result. A Ptr here will be replaced. Value should be NULL.

	MD5_CTX context;
	unsigned char* aHash = new unsigned char[kHashLen];

	// Calculate H(A1) for MD5
	// where A1 for algorithm = "md5" or if nothing is specified is
	//              A1  = userName:realm:userPassword
	MD5_Init(&context);
	MD5_Update(&context, (unsigned char *)userName->Ptr, userName->Len);
	MD5_Update(&context, (unsigned char *)sColon.Ptr, sColon.Len);
	MD5_Update(&context, (unsigned char *)realm->Ptr, realm->Len);
	MD5_Update(&context, (unsigned char *)sColon.Ptr, sColon.Len);
	MD5_Update(&context, (unsigned char *)userPassword->Ptr, userPassword->Len);
	MD5_Final(aHash, &context);
	hashA1Hex16Bit->Ptr = (char *)aHash;
	hashA1Hex16Bit->Len = kHashLen;
}

// allocates memory to hA1->Ptr
void CalcHA1(StrPtrLen* algorithm,
	StrPtrLen* userName,
	StrPtrLen* realm,
	StrPtrLen* userPassword,
	StrPtrLen* nonce,
	StrPtrLen* cNonce,
	StrPtrLen* hA1
)
{
	// parameters must be valid pointers 
	// It is ok if parameter->Ptr is NULL as long as parameter->Len is 0
	Assert(algorithm);
	Assert(userName);
	Assert(realm);
	Assert(userPassword);
	Assert(nonce);
	Assert(cNonce);
	Assert(hA1);
	Assert(hA1->Ptr == NULL); //This is the result. A Ptr here will be replaced. Value should be NULL.

	MD5_CTX context;
	unsigned char aHash[kHashLen];

	// Calculate H(A1)
	// where A1 for algorithm = "md5" or if nothing is specified is
	//              A1  = userName:realm:userPassword
	// and for algorithm = "md5-sess" is
	//              A1  = H(userName:realm:userPassword):nonce:cnonce
	MD5_Init(&context);
	MD5_Update(&context, (unsigned char *)userName->Ptr, userName->Len);
	MD5_Update(&context, (unsigned char *)sColon.Ptr, sColon.Len);
	MD5_Update(&context, (unsigned char *)realm->Ptr, realm->Len);
	MD5_Update(&context, (unsigned char *)sColon.Ptr, sColon.Len);
	MD5_Update(&context, (unsigned char *)userPassword->Ptr, userPassword->Len);
	MD5_Final(aHash, &context);
	if (algorithm->Equal(sMD5Sess)) {
		MD5_Init(&context);
		MD5_Update(&context, aHash, kHashLen);
		MD5_Update(&context, (unsigned char *)sColon.Ptr, sColon.Len);
		MD5_Update(&context, (unsigned char *)nonce->Ptr, nonce->Len);
		MD5_Update(&context, (unsigned char *)sColon.Ptr, sColon.Len);
		MD5_Update(&context, (unsigned char *)cNonce->Ptr, cNonce->Len);
		MD5_Final(aHash, &context);
	}
	HashToString(aHash, hA1);
}

// allocates memory to hA1->Ptr
void CalcHA1Md5Sess(StrPtrLen* hashA1Hex16Bit, StrPtrLen* nonce, StrPtrLen* cNonce, StrPtrLen* hA1)
{
	// parameters must be valid pointers 
	// It is ok if parameter->Ptr is NULL as long as parameter->Len is 0
	Assert(hashA1Hex16Bit);
	Assert(hashA1Hex16Bit->Len == kHashLen);
	Assert(nonce);
	Assert(cNonce);
	Assert(hA1);
	Assert(hA1->Ptr == NULL); //This is the result. A Ptr here will be replaced. Value should be NULL.

	MD5_CTX context;
	unsigned char aHash[kHashLen];

	MD5_Init(&context);
	MD5_Update(&context, (unsigned char *)hashA1Hex16Bit->Ptr, kHashLen);
	MD5_Update(&context, (unsigned char *)sColon.Ptr, sColon.Len);
	MD5_Update(&context, (unsigned char *)nonce->Ptr, nonce->Len);
	MD5_Update(&context, (unsigned char *)sColon.Ptr, sColon.Len);
	MD5_Update(&context, (unsigned char *)cNonce->Ptr, cNonce->Len);
	MD5_Final(aHash, &context);

	// allocates memory to hA1->Ptr
	HashToString(aHash, hA1);
}

// allocates memory for requestDigest->Ptr              
void CalcRequestDigest(StrPtrLen* hA1,
	StrPtrLen* nonce,
	StrPtrLen* nonceCount,
	StrPtrLen* cNonce,
	StrPtrLen* qop,
	StrPtrLen* method,
	StrPtrLen* digestUri,
	StrPtrLen* hEntity,
	StrPtrLen* requestDigest
)
{
	// parameters must be valid pointers 
	// It is ok if parameter->Ptr is NULL as long as parameter->Len is 0
	Assert(hA1);
	Assert(nonce);
	Assert(nonceCount);
	Assert(cNonce);
	Assert(qop);
	Assert(method);
	Assert(digestUri);
	Assert(hEntity);
	Assert(requestDigest);
	Assert(requestDigest->Ptr == NULL); //This is the result. A Ptr here will be replaced. Value should be NULL.

	unsigned char aHash[kHashLen], requestHash[kHashLen];
	StrPtrLen hA2;
	MD5_CTX context;


	// H(data) = MD5(data)
	// and KD(secret, data) = H(concat(secret, ":", data))

	// Calculate H(A2)
	// where A2 for qop="auth" or no qop is 
	//              A2  = method:digestUri
	// and for qop = "auth-int" is
	//              A2 = method:digestUri:H(entity-body)
	MD5_Init(&context);
	MD5_Update(&context, (unsigned char *)method->Ptr, method->Len);
	MD5_Update(&context, (unsigned char *)sColon.Ptr, sColon.Len);
	MD5_Update(&context, (unsigned char *)digestUri->Ptr, digestUri->Len);
	if (qop->Equal(sQopAuthInt)) {
		MD5_Update(&context, (unsigned char *)sColon.Ptr, sColon.Len);
		MD5_Update(&context, (unsigned char *)hEntity->Ptr, hEntity->Len);
	}
	MD5_Final(aHash, &context);

	// HashToString allocates memory for hA2...delete it after request-digest is created
	HashToString(aHash, &hA2);
	// Calculate request-digest
	// where request-digest for qop="auth" or qop="auth-int" is
	//          request-digest  = KD( H(A1), nonce:nonceCount:cNonce:qop:H(A2) )
	// and if qop directive isn't present is
	//          request-digest = KD( H(A1), nonce:H(A2) )
	MD5_Init(&context);
	MD5_Update(&context, (unsigned char *)hA1->Ptr, hA1->Len);
	MD5_Update(&context, (unsigned char *)sColon.Ptr, sColon.Len);
	MD5_Update(&context, (unsigned char *)nonce->Ptr, nonce->Len);
	MD5_Update(&context, (unsigned char *)sColon.Ptr, sColon.Len);
	if (qop->Ptr != NULL) {
		MD5_Update(&context, (unsigned char *)nonceCount->Ptr, nonceCount->Len);
		MD5_Update(&context, (unsigned char *)sColon.Ptr, sColon.Len);
		MD5_Update(&context, (unsigned char *)cNonce->Ptr, cNonce->Len);
		MD5_Update(&context, (unsigned char *)sColon.Ptr, sColon.Len);
		MD5_Update(&context, (unsigned char *)qop->Ptr, qop->Len);
		MD5_Update(&context, (unsigned char *)sColon.Ptr, sColon.Len);
	}
	MD5_Update(&context, (unsigned char *)hA2.Ptr, hA2.Len);
	MD5_Final(requestHash, &context);
	HashToString(requestHash, requestDigest);

	// Deleting memory allocated for hA2
	delete[] hA2.Ptr;
}



/* From local_passwd.c (C) Regents of Univ. of California blah blah */
static unsigned char itoa64[] = /* 0 ... 63 => ascii - 64 */
"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void to64(register char* s, register SInt32 v, register int n)
{
	while (--n >= 0) {
		*s++ = itoa64[v & 0x3f];
		v >>= 6;
	}
}

/*
 * Define the Magic String prefix that identifies a password as being
 * hashed using our algorithm.
 */
static char *dufr_id = "$dufr$";

// Doesn't allocate any memory. The size of the result buffer should be nbytes
void MD5Encode(char* pw, char* salt, char* result, int nbytes)
{
	/*
	 * Minimum size is 8 bytes for salt, plus 1 for the trailing NUL,
	 * plus 4 for the '$' separators, plus the password hash itself.
	 * Let's leave a goodly amount of leeway.
	 */

	char passwd[120], *p;
	char *sp, *ep;
	unsigned char final[kHashLen];
	int sl, pl, i;
	MD5_CTX ctx, ctx1;
	UInt32 l;

	/*
	 * Refine the salt first.  It's possible we were given an already-hashed
	 * string as the salt argument, so extract the actual salt value from it
	 * if so.  Otherwise just use the string up to the first '$' as the salt.
	 */
	sp = salt;

	//If it starts with the magic string, then skip that.
	if (!strncmp(sp, dufr_id, strlen(dufr_id)))
	{
		sp += strlen(dufr_id);
	}

	//It stops at the first '$' or 8 chars, whichever comes first
	for (ep = sp; (*ep != '\0') && (*ep != '$') && (ep < (sp + 8)); ep++)
	{
		continue;
	}

	//Get the length of the true salt
	sl = ep - sp;

	//'Time to make the doughnuts..'
	MD5_Init(&ctx);

	//The password first, since that is what is most unknown
	MD5_Update(&ctx, (unsigned char *)pw, strlen(pw));

	//Then our magic string
	MD5_Update(&ctx, (unsigned char *)dufr_id, strlen(dufr_id));

	//Then the raw salt
	MD5_Update(&ctx, (unsigned char *)sp, sl);

	//Then just as many characters of the MD5(pw, salt, pw)
	MD5_Init(&ctx1);
	MD5_Update(&ctx1, (unsigned char *)pw, strlen(pw));
	MD5_Update(&ctx1, (unsigned char *)sp, sl);
	MD5_Update(&ctx1, (unsigned char *)pw, strlen(pw));
	MD5_Final(final, &ctx1);
	for (pl = strlen(pw); pl > 0; pl -= kHashLen)
	{
		MD5_Update(&ctx, (unsigned char *)final, (pl > kHashLen) ? kHashLen : pl);
	}

	//Don't leave anything around in vm they could use.
	memset(final, 0, sizeof(final));

	//Then something really weird...
	for (i = strlen(pw); i != 0; i >>= 1)
	{
		if (i & 1) {
			MD5_Update(&ctx, (unsigned char *)final, 1);
		}
		else {
			MD5_Update(&ctx, (unsigned char *)pw, 1);
		}
	}

	/*
	 * Now make the output string.  We know our limitations, so we
	 * can use the string routines without bounds checking.
	 */
	strcpy(passwd, dufr_id);
	strncat(passwd, sp, sl);
	strcat(passwd, "$");

	MD5_Final(final, &ctx);

	/*
	 * And now, just to make sure things don't run too fast..
	 * On a 60 Mhz Pentium this takes 34 msec, so you would
	 * need 30 seconds to build a 1000 entry dictionary...
	 */
	for (i = 0; i < 1000; i++)
	{
		MD5_Init(&ctx1);
		if (i & 1) {
			MD5_Update(&ctx1, (unsigned char *)pw, strlen(pw));
		}
		else {
			MD5_Update(&ctx1, final, kHashLen);
		}
		if (i % 3) {
			MD5_Update(&ctx1, (unsigned char *)sp, sl);
		}

		if (i % 7) {
			MD5_Update(&ctx1, (unsigned char *)pw, strlen(pw));
		}

		if (i & 1) {
			MD5_Update(&ctx1, (unsigned char *)final, kHashLen);
		}
		else {
			MD5_Update(&ctx1, (unsigned char *)pw, strlen(pw));
		}
		MD5_Final(final, &ctx1);
	}

	p = passwd + strlen(passwd);

	l = (final[0] << 16) | (final[6] << 8) | final[12]; to64(p, l, 4); p += 4;
	l = (final[1] << 16) | (final[7] << 8) | final[13]; to64(p, l, 4); p += 4;
	l = (final[2] << 16) | (final[8] << 8) | final[14]; to64(p, l, 4); p += 4;
	l = (final[3] << 16) | (final[9] << 8) | final[15]; to64(p, l, 4); p += 4;
	l = (final[4] << 16) | (final[10] << 8) | final[5]; to64(p, l, 4); p += 4;
	l = final[11]; to64(p, l, 2); p += 2;
	*p = '\0';

	//Don't leave anything around in vm they could use.
	memset(final, 0, sizeof(final));

	strncpy(result, passwd, nbytes - 1);
}
