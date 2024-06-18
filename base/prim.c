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

#include <base/prim.h>

void U64_cleanup(U64 *ptr) {}
size_t U64_size(U64 *ptr) { return sizeof(U64); }
bool U64_copy(U64 *dst, U64 *src) {
	memcpy(&dst->_value, &src->_value, sizeof(u64));
	return true;
}
void *U64_unwrap(U64 *ptr) { return &ptr->_value; }

void I32_cleanup(I32 *ptr) {}
size_t I32_size(I32 *ptr) { return sizeof(I32); }
bool I32_copy(I32 *dst, I32 *src) {
	memcpy(&dst->_value, &src->_value, sizeof(i32));
	return true;
}
void *I32_unwrap(I32 *ptr) { return &ptr->_value; }
