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
#include <base/types.h>
#include <sys/random.h>

int rand_i8(i8 *v) {
	return getentropy(v, sizeof(i8));
}

int rand_u8(u8 *v) {
	return getentropy(v, sizeof(u8));
}

int rand_i16(i16 *v) {
	return getentropy(v, sizeof(i16));
}

int rand_u16(u16 *v) {
	return getentropy(v, sizeof(u16));
}

int rand_i32(i32 *v) {
	return getentropy(v, sizeof(i32));
}

int rand_u32(u32 *v) {
	return getentropy(v, sizeof(u32));
}

int rand_i64(i64 *v) {
	return getentropy(v, sizeof(i64));
}

int rand_u64(u64 *v) {
	return getentropy(v, sizeof(u64));
}

int rand_i128(i128 *v) {
	return getentropy(v, sizeof(i128));
}

int rand_u128(u128 *v) {
	return getentropy(v, sizeof(u128));
}

int rand_bytes(void *buf, u64 length) {
	return getentropy(buf, length);
}
