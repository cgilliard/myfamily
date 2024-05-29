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

#include <criterion/criterion.h>
#include <stdio.h>
#include <base/rand.h>
#include <inttypes.h>

Test(base, rand) {
	int32_t v32;
	rand_i32(&v32);
	printf("v=%d\n", v32);
	int64_t v64;
	rand_i64(&v64);
	printf("v=%" PRId64 "\n", v64);
	__uint128_t ru128;
       	int v = rand_u128(&ru128);
	uint64_t high = (ru128 >> 64);
	uint64_t low = ru128 & 0xFFFFFFFFFFFFFFFF;
	printf("x=%" PRIu64, high);
	printf("%" PRIu64"\n", low);
	__int128_t r128;
        v = rand_i128(&r128);
        uint64_t ihigh = (r128 >> 64);
        uint64_t ilow = r128 & 0xFFFFFFFFFFFFFFFF;
        printf("d=%" PRId64, ihigh);
        printf("%" PRIu64"\n", ilow);

	char buf[4];
	rand_bytes(&buf, 4);
	printf("%i %i %i %i\n", buf[0], buf[1], buf[2], buf[3]);
}
