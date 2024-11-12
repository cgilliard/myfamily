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
