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

#include <base/aes.h>
#include <base/cpsrng.h>
#include <base/print_util.h>
#include <base/rand.h>

static struct AES_ctx aes_ctx;

void cpsrng_reseed() {
	byte iv[16];
	byte key[32];
	if (rand_bytes(key, 32)) {
		panic("Could not generate entropy for AES key generation");
	}
	if (rand_bytes(iv, 16)) {
		panic("Could not generate entropy for AES iv generation");
	}

	AES_init_ctx_iv(&aes_ctx, key, iv);
}

// __attribute__ ((constructor)) guaranteed to be called before main.
// This will either succeed or exit before main is called.
void __attribute__((constructor)) __init_cpsrng() {
	cpsrng_reseed();
}

// note: not thread safe as user must ensure thread safety. This allows for
// flexible usage in a single thread, no locking is needed. In multi-threaded
// environments, locking may be used.
void cpsrng_rand_byte(byte *v) {
	AES_CTR_xcrypt_buffer(&aes_ctx, (byte *)v, sizeof(byte));
}

// note: not thread safe as user must ensure thread safety. This allows for
// flexible usage in a single thread, no locking is needed. In multi-threaded
// environments, locking may be used.
void cpsrng_rand_i64(i64 *v) {
	AES_CTR_xcrypt_buffer(&aes_ctx, (byte *)v, sizeof(i64));
}

// note: not thread safe as user must ensure thread safety. This allows for
// flexible usage in a single thread, no locking is needed. In multi-threaded
// environments, locking may be used.
void cpsrng_rand_int(int *v) {
	AES_CTR_xcrypt_buffer(&aes_ctx, (byte *)v, sizeof(int));
}

// note: not thread safe as user must ensure thread safety. This allows for
// flexible usage in a single thread, no locking is needed. In multi-threaded
// environments, locking may be used.
void cpsrng_rand_bytes(void *v, unsigned long long size) {
	AES_CTR_xcrypt_buffer(&aes_ctx, (byte *)v, size);
}

// only available in test mode for tests. Not used in production environments.
#ifdef TEST
void cpsrng_test_seed(byte iv[16], byte key[32]) {
	AES_init_ctx_iv(&aes_ctx, key, iv);
	i64 v0 = 0;
	cpsrng_rand_i64(&v0);
}
#endif	// TEST
