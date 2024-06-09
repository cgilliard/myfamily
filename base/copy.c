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

#include <base/tlmalloc.h>
#include <base/types.h>
#include <stdlib.h>
#include <string.h>

void copy_unit(void *dst, void *src) {}
void copy_u128(void *dest, void *src) { memcpy(dest, src, sizeof(u128)); }
void copy_u64(void *dest, void *src) { memcpy(dest, src, sizeof(u64)); }
void copy_u32(void *dest, void *src) { memcpy(dest, src, sizeof(u32)); }
void copy_u16(void *dest, void *src) { memcpy(dest, src, sizeof(u16)); }
void copy_u8(void *dest, void *src) { memcpy(dest, src, sizeof(u8)); }

void copy_i128(void *dest, void *src) { memcpy(dest, src, sizeof(i128)); }
void copy_i64(void *dest, void *src) { memcpy(dest, src, sizeof(i64)); }
void copy_i32(void *dest, void *src) { memcpy(dest, src, sizeof(i32)); }
void copy_i16(void *dest, void *src) { memcpy(dest, src, sizeof(i16)); }
void copy_i8(void *dest, void *src) { memcpy(dest, src, sizeof(i8)); }

void copy_f64(void *dest, void *src) { memcpy(dest, src, sizeof(f64)); }
void copy_f32(void *dest, void *src) { memcpy(dest, src, sizeof(f32)); }

void copy_bool(void *dest, void *src) { memcpy(dest, src, sizeof(bool)); }
void copy_string(void *dest, void *src) {
	((String *)dest)->ptr =
	    tlmalloc(sizeof(char) * (((String *)src)->len + 1));
	((String *)dest)->len = ((String *)src)->len;
	strcpy(((String *)dest)->ptr, ((String *)src)->ptr);
}

Copy build_copy(Copy *c, void *obj, size_t size,
		void (*copy)(void *dest, void *src)) {
	c->size = size;
	c->obj = obj;
	c->copy = copy;
	return *c;
}
