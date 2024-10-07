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

#ifndef _CRYPTO_PSRNG__
#define _CRYPTO_PSRNG__

#include <base/types.h>

void psrng_reseed();
void psrng_rand_u8(u8 *v);
void psrng_rand_u64(u64 *v);
void psrng_rand_u128(u128 *v);
void psrng_rand_bytes(void *v, u64 size);

#ifdef TEST
void psrng_test_seed(u8 iv[16], u8 key[32]);
#endif // TEST

#endif // _CRYPTO_PSRNG__
