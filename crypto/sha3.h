// Copyright (c) 2024, The MyFamily Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SHA3_H
#define SHA3_H

#include <base/misc.h>
#include <base/resources.h>
#include <base/types.h>

// Added macro to hash a string
#define SHA3_HASH(in, out)                                                                         \
	({                                                                                             \
		sha3_context _c__;                                                                         \
		const u8 *_hash__;                                                                         \
		sha3_Init256(&_c__);                                                                       \
		sha3_Update(&_c__, in, mystrlen(in));                                                      \
		_hash__ = sha3_Finalize(&_c__);                                                            \
		for (int i = 0; i < 32; i++) {                                                             \
			char s[23];                                                                            \
			byte_to_hex(_hash__[i], s);                                                            \
			out[i * 2] = s[0];                                                                     \
			out[(i * 2) + 1] = s[1];                                                               \
		}                                                                                          \
		out[64] = 0;                                                                               \
	})

static void byte_to_hex(u8 b, char s[23]) {
	unsigned i = 1;
	s[0] = s[1] = '0';
	s[2] = '\0';
	while (b) {
		unsigned t = b & 0x0f;
		if (t < 10) {
			s[i] = '0' + t;
		} else {
			s[i] = 'a' + t - 10;
		}
		i--;
		b >>= 4;
	}
}

// 'hash' points to a buffer inside 'c'
// with the value of SHA3-256

/* -------------------------------------------------------------------------
 * Works when compiled for either 32-bit or 64-bit targets, optimized for
 * 64 bit.
 *
 * Canonical implementation of Init/Update/Finalize for SHA-3 byte input.
 *
 * SHA3-256, SHA3-384, SHA-512 are implemented. SHA-224 can easily be added.
 *
 * Based on code from http://keccak.noekeon.org/ .
 *
 * I place the code that I wrote into public domain, free to use.
 *
 * I would appreciate if you give credits to this work if you used it to
 * write or test * your code.
 *
 * Aug 2015. Andrey Jivsov. crypto@brainhub.org
 * ---------------------------------------------------------------------- */

/* 'Words' here refers to u64 */
#define SHA3_KECCAK_SPONGE_WORDS (((1600) / 8 /*bits to byte*/) / sizeof(u64))
typedef struct sha3_context_ {
	u64 saved; /* the portion of the input message that we
				* didn't consume yet */
	union {	   /* Keccak's state */
		u64 s[SHA3_KECCAK_SPONGE_WORDS];
		u8 sb[SHA3_KECCAK_SPONGE_WORDS * 8];
	} u;
	unsigned byteIndex;		/* 0..7--the next byte after the set one
							 * (starts from 0; 0--none are buffered) */
	unsigned wordIndex;		/* 0..24--the next word to integrate input
							 * (starts from 0) */
	unsigned capacityWords; /* the double size of the hash output in
							 * words (e.g. 16 for Keccak 512) */
} sha3_context;

enum SHA3_FLAGS { SHA3_FLAGS_NONE = 0, SHA3_FLAGS_KECCAK = 1 };

enum SHA3_RETURN { SHA3_RETURN_OK = 0, SHA3_RETURN_BAD_PARAMS = 1 };
typedef enum SHA3_RETURN sha3_return_t;

/* For Init or Reset call these: */
sha3_return_t sha3_Init(void *priv, unsigned bitSize);

void sha3_Init256(void *priv);
void sha3_Init384(void *priv);
void sha3_Init512(void *priv);

enum SHA3_FLAGS sha3_SetFlags(void *priv, enum SHA3_FLAGS);

void sha3_Update(void *priv, void const *bufIn, u64 len);

void const *sha3_Finalize(void *priv);

/* Single-call hashing */
sha3_return_t sha3_HashBuffer(unsigned bitSize,		 /* 256, 384, 512 */
							  enum SHA3_FLAGS flags, /* SHA3_FLAGS_NONE or SHA3_FLAGS_KECCAK */
							  const void *in, unsigned inBytes, void *out,
							  unsigned outBytes); /* up to bitSize/8; truncation OK */

#endif
