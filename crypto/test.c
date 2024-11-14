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

#include <base/test.h>
#include <crypto/lib.h>

Suite(crypto);

Test(test_rand) {
	int count = 10;
	int rand[count];
	for (int i = 0; i < count; i++) fam_assert(!rand_int(&rand[i]));
	bool diff = false;
	for (int i = 1; i < count; i++)
		if (rand[i] != rand[0]) diff = true;
	fam_assert(diff);

	byte randb[count];
	for (int i = 0; i < count; i++) fam_assert(!rand_byte(&randb[i]));
	diff = false;
	for (int i = 1; i < count; i++)
		if (randb[i] != randb[0]) diff = true;
	fam_assert(diff);

	int64 rand64[count];
	for (int i = 0; i < count; i++) fam_assert(!rand_int64(&rand64[i]));
	diff = false;
	for (int i = 1; i < count; i++)
		if (rand64[i] != rand64[0]) diff = true;
	fam_assert(diff);

	byte buf1[count], buf2[count];
	fam_assert(!rand_bytes(buf1, count));
	fam_assert(!rand_bytes(buf2, count));
	fam_assert(cstring_compare_n(buf1, buf2, count));
}

Test(test_set_iv) {
	byte iv[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	byte key[32] = {0,	1,	2,	3,	4,	5,	6,	7,	8,	9,	10,
					11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
					22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
	if (rand_bytes(key, 32)) {
		panic("Could not generate entropy for AES key generation");
	}
	if (rand_bytes(iv, 16)) {
		panic("Could not generate entropy for AES iv generation");
	}

	struct AES_ctx aes_ctx;
	AES_init_ctx_iv(&aes_ctx, key, iv);
	for (int i = 0; i < 16; i++) {
		fam_assert_eq(aes_ctx.Iv[i], iv[i]);
	}
	for (int i = 0; i < 32; i++) {
		fam_assert_eq(aes_ctx.RoundKey[i], key[i]);
	}
	byte iv2[16] = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
	AES_ctx_set_iv(&aes_ctx, iv2);
	for (int i = 0; i < 16; i++) {
		fam_assert_eq(aes_ctx.Iv[i], iv2[i]);
	}
}

Test(test_cpsrng) {
	// seed values for test
	byte iv[16] = {2};
	byte key[32] = {1};
	cpsrng_test_seed(iv, key);
	byte b = 0;
	cpsrng_rand_byte(&b);
	fam_assert_eq(b, 239);

	byte bytes[5] = {}, expected[5] = {83, 3, 132, 185, 144};
	cpsrng_rand_bytes(bytes, 5);

	for (int i = 0; i < 5; i++) fam_assert_eq(bytes[i], expected[i]);

	_debug_print_util_disable__ = true;
	_debug_getentropy_err = true;

	fam_err = NoErrors;
	cpsrng_reseed();
	fam_assert_eq(fam_err, IllegalState);

	_debug_getentropy_err = false;
	_debug_print_util_disable__ = false;
}
