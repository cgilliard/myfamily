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
#include <crypto/aes.h>
#include <crypto/psrng.h>

MySuite(crypto);

MyTest(crypto, test_rand) {
	u8 v1 = 0;
	psrng_rand_u8(&v1);
	printf("v1=%u\n", v1);
	psrng_rand_u8(&v1);
	printf("v1=%u\n", v1);
	psrng_rand_u8(&v1);
	printf("v1=%u\n", v1);

	u64 v2 = 0;
	psrng_rand_u64(&v2);
	printf("v2=%llu\n", v2);
	psrng_rand_u64(&v2);
	printf("v2=%llu\n", v2);
	psrng_rand_u64(&v2);
	printf("v2=%llu\n", v2);

	u8 key[32] = {1, 4, 3, 4, 5};
	u8 iv[16] = {1, 2, 3, 4};
	psrng_test_seed(iv, key);
	v1 = 0;
	psrng_rand_u8(&v1);
	printf("v1=%u\n", v1);
	psrng_rand_u8(&v1);
	printf("v1=%u\n", v1);
	psrng_rand_u8(&v1);
	printf("v1=%u\n", v1);

	v2 = 0;
	psrng_rand_u64(&v2);
	printf("v2=%llu\n", v2);
	psrng_rand_u64(&v2);
	printf("v2=%llu\n", v2);
	psrng_rand_u64(&v2);
	printf("v2=%llu\n", v2);
}
