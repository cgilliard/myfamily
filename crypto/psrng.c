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

#include <crypto/aes.h>
#include <crypto/psrng.h>
#include <crypto/rand.h>
#include <stdio.h>
#include <string.h>

static struct AES_ctx ctx;

void psrng_reseed() {
	u8 iv[16];
	u8 key[32];
	printf("reseed with key[0]=%u\n", key[0]);
	if (rand_bytes(key, 32)) {
		perror("Could not generate entropy for AES key generation");
		exit(-1);
	}
	if (rand_bytes(iv, 16)) {
		perror("Could not generate entropy for AES iv generation");
		exit(-1);
	}

	AES_init_ctx_iv(&ctx, key, iv);
}

// __attribute__ ((constructor)) guaranteed to be called before main.
// This will either succeed or exit before main is called.
void __attribute__((constructor)) __init_psrng() {
	psrng_reseed();
}

// note: not thread safe as user must ensure thread safety. This allows for flexible usage
// in a single thread, no locking is needed. In multi-threaded environments, locking may be used.
void psrng_rand_u8(u8 *v) {
	AES_CTR_xcrypt_buffer(&ctx, v, sizeof(u8));
}

// note: not thread safe as user must ensure thread safety. This allows for flexible usage
// in a single thread, no locking is needed. In multi-threaded environments, locking may be used.
void psrng_rand_u64(u64 *v) {
	AES_CTR_xcrypt_buffer(&ctx, (u8 *)v, sizeof(u64));
}

// note: not thread safe as user must ensure thread safety. This allows for flexible usage
// in a single thread, no locking is needed. In multi-threaded environments, locking may be used.
void psrng_rand_u128(u128 *v) {
	AES_CTR_xcrypt_buffer(&ctx, (u8 *)v, sizeof(u128));
}

// note: not thread safe as user must ensure thread safety. This allows for flexible usage
// in a single thread, no locking is needed. In multi-threaded environments, locking may be used.
void psrng_rand_bytes(void *v, u64 size) {
	AES_CTR_xcrypt_buffer(&ctx, (u8 *)v, size);
}

// only available in test mode for tests. Not used in production environments.
#ifdef TEST
void psrng_test_seed(u8 iv[16], u8 key[32]) {
	AES_init_ctx_iv(&ctx, key, iv);
	u64 v0 = 0;
	psrng_rand_u64(&v0);
}
#endif // TEST
