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

#include <base/rand.h>
#include <sys/random.h>

int rand_i32(int32_t *v) {
	char buf[4];
	int ret = getentropy(buf, 4);
	*v = ((int32_t)buf[0] << 24) + ((int32_t)buf[1] << 16) + ((int32_t)buf[2] << 8) + (int32_t)buf[3];
	return ret;
}

int rand_u32(uint32_t *v) {
        char buf[4];
        int ret = getentropy(buf, 4);
        *v = ((uint32_t)buf[0] << 24) + ((uint32_t)buf[1] << 16) + ((uint32_t)buf[2] << 8) + (uint32_t)buf[3];
        return ret;
}

int rand_i64(int64_t *v) {
	char buf[8];
        int ret = getentropy(buf, 8);
	*v =
		((int64_t)buf[0] << 56) + ((int64_t)buf[1] << 48) +
		((int64_t)buf[2] << 40) + ((int64_t)buf[3] << 32) +
		((int64_t)buf[4] << 24) + ((int64_t)buf[5] << 16) +
		((int64_t)buf[6] << 8) + (int64_t)buf[7];
	return ret;
}

int rand_u64(uint64_t *v) {
	char buf[8];
        int ret = getentropy(buf, 8);
        *v = 
                ((uint64_t)buf[0] << 56) + ((uint64_t)buf[1] << 48) +
                ((uint64_t)buf[2] << 40) + ((uint64_t)buf[3] << 32) +
                ((uint64_t)buf[4] << 24) + ((uint64_t)buf[5] << 16) +
                ((uint64_t)buf[6] << 8) + (uint64_t)buf[7];
        return ret;
}

int rand_i128(i128 *v) {
	char buf[16];
	int ret = getentropy(buf, 16);
 	*v = 
                ((i128)buf[0] << 120) + ((i128)buf[1] << 112) +
                ((i128)buf[2] << 104) + ((i128)buf[3] << 96) +
                ((i128)buf[4] << 88) + ((i128)buf[5] << 80) +
                ((i128)buf[6] << 72) + ((i128)buf[7] << 64) +
                ((i128)buf[8] << 56) + ((i128)buf[9] << 48) +
                ((i128)buf[10] << 40) + ((i128)buf[11] << 32) +
                ((i128)buf[12] << 24) + ((i128)buf[13] << 16) +
                ((i128)buf[14] << 8) + (i128)buf[15];
	return ret;
}


int rand_u128(u128 *v) {
	char buf[16];
	int ret = getentropy(buf, 16);
	*v =
		((u128)buf[0] << 120) + ((u128)buf[1] << 112) +
		((u128)buf[2] << 104) + ((u128)buf[3] << 96) +
		((u128)buf[4] << 88) + ((u128)buf[5] << 80) +
		((u128)buf[6] << 72) + ((u128)buf[7] << 64) +
                ((u128)buf[8] << 56) + ((u128)buf[9] << 48) +
                ((u128)buf[10] << 40) + ((u128)buf[11] << 32) +
                ((u128)buf[12] << 24) + ((u128)buf[13] << 16) +
                ((u128)buf[14] << 8) + (u128)buf[15];
	return ret;
}

int rand_bytes(void *buf, size_t length) {
	return getentropy(buf, length);
}
