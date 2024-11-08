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

// Most code originally from:
// From repo: https://github.com/kokke/tiny-AES-c/
// License: https://github.com/kokke/tiny-AES-c/blob/master/unlicense.txt:
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org/>

#include <base/types.h>

#ifndef _CRYPTO_AES__
#define _CRYPTO_AES__

// #define the macros below to 1/0 to enable/disable the mode of operation.
//
// CBC enables AES encryption in CBC-mode of operation.
// CTR enables encryption in counter-mode.
// ECB enables the basic ECB 16-byte block algorithm. All can be enabled
// simultaneously.

// The #ifndef-guard allows it to be configured before #include'ing or at
// compile time.
#ifndef CBC
#define CBC 1
#endif

#ifndef ECB
#define ECB 1
#endif

#ifndef CTR
#define CTR 1
#endif

// #define AES128 1
//  #define AES192 1
#define AES256 1

#define AES_BLOCKLEN 16	 // Block length in bytes - AES is 128b block only

#if defined(AES256) && (AES256 == 1)
#define AES_KEYLEN 32
#define AES_keyExpSize 240
#elif defined(AES192) && (AES192 == 1)
#define AES_KEYLEN 24
#define AES_keyExpSize 208
#else
#define AES_KEYLEN 16  // Key length in bytes
#define AES_keyExpSize 176
#endif

struct AES_ctx {
	byte RoundKey[AES_keyExpSize];
#if (defined(CBC) && (CBC == 1)) || (defined(CTR) && (CTR == 1))
	byte Iv[AES_BLOCKLEN];
#endif
};

void AES_init_ctx(struct AES_ctx *ctx, const byte *key);
#if (defined(CBC) && (CBC == 1)) || (defined(CTR) && (CTR == 1))
void AES_init_ctx_iv(struct AES_ctx *ctx, const byte *key, const byte *iv);
void AES_ctx_set_iv(struct AES_ctx *ctx, const byte *iv);
#endif

#if defined(ECB) && (ECB == 1)
// buffer size is exactly AES_BLOCKLEN bytes;
// you need only AES_init_ctx as IV is not used in ECB
// NB: ECB is considered insecure for most uses
void AES_ECB_encrypt(const struct AES_ctx *ctx, byte *buf);
void AES_ECB_decrypt(const struct AES_ctx *ctx, byte *buf);

#endif	// #if defined(ECB) && (ECB == !)

#if defined(CBC) && (CBC == 1)
// buffer size MUST be mutile of AES_BLOCKLEN;
// Suggest https://en.wikipedia.org/wiki/Padding_(cryptography)#PKCS7 for
// padding scheme NOTES: you need to set IV in ctx via AES_init_ctx_iv() or
// AES_ctx_set_iv()
//        no IV should ever be reused with the same key
void AES_CBC_encrypt_buffer(struct AES_ctx *ctx, byte *buf,
							unsigned long long length);
void AES_CBC_decrypt_buffer(struct AES_ctx *ctx, byte *buf,
							unsigned long long length);

#endif	// #if defined(CBC) && (CBC == 1)

#if defined(CTR) && (CTR == 1)

// Same function for encrypting as for decrypting.
// IV is incremented for every block, and used after encryption as
// XOR-compliment for output Suggesting
// https://en.wikipedia.org/wiki/Padding_(cryptography)#PKCS7 for padding scheme
// NOTES: you need to set IV in ctx with AES_init_ctx_iv() or AES_ctx_set_iv()
//        no IV should ever be reused with the same key
void AES_CTR_xcrypt_buffer(struct AES_ctx *ctx, byte *buf,
						   unsigned long long length);

#endif	// #if defined(CTR) && (CTR == 1)

#endif	// _CRYPTO_AES__
