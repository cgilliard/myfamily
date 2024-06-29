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

#ifndef _BASE_TUPLE__
#define _BASE_TUPLE__

#include <base/class.h>
#include <base/macro_utils.h>
#include <base/prim.h>
#include <base/traits_base.h>

CLASS(Tuple, FIELD(void **, elements) FIELD(u64, count))
IMPL(Tuple, TRAIT_LEN)
IMPL(Tuple, TRAIT_DEBUG)
IMPL(Tuple, TRAIT_COPY)
IMPL(Tuple, TRAIT_EQUAL)
#define Tuple DEFINE_CLASS(Tuple)

void Tuple_element_at(Tuple *t, u64 index, void *dst);
void *Tuple_add_value(void *value);
void *Tuple_add_value_u128(void *value);
void *Tuple_add_value_u64(void *value);
void *Tuple_add_value_u32(void *value);
void *Tuple_add_value_u16(void *value);
void *Tuple_add_value_u8(void *value);
void *Tuple_add_value_i128(void *value);
void *Tuple_add_value_i64(void *value);
void *Tuple_add_value_i32(void *value);
void *Tuple_add_value_i16(void *value);
void *Tuple_add_value_i8(void *value);
void *Tuple_add_value_str(char **value);
void *Tuple_add_value_bool(void *value);

#define CREATE_GENERIC(value)                                                  \
	_Generic((value),                                                      \
	    i128: Tuple_add_value_i128,                                        \
	    i64: Tuple_add_value_i64,                                          \
	    i32: Tuple_add_value_i32,                                          \
	    i16: Tuple_add_value_i16,                                          \
	    i8: Tuple_add_value_i8,                                            \
	    u128: Tuple_add_value_u128,                                        \
	    u64: Tuple_add_value_u64,                                          \
	    u32: Tuple_add_value_u32,                                          \
	    u16: Tuple_add_value_u16,                                          \
	    u8: Tuple_add_value_u8,                                            \
	    char *: Tuple_add_value_str,                                       \
	    bool: Tuple_add_value_bool,                                        \
	    default: Tuple_add_value)(&value)

#define ELEMENT_AT(tuple, index, dst) Tuple_element_at(tuple, index, dst)

#define TUPLE(...)                                                             \
	({                                                                     \
		void *values[] = {                                             \
		    EXPAND(FOR_EACH(CREATE_GENERIC, __VA_ARGS__))};            \
		u64 count = sizeof(values) / sizeof(values[0]);                \
		void **elements = tlmalloc(count * sizeof(void *));            \
		for (u64 i = 0; i < count; ++i) {                              \
			elements[i] = values[i];                               \
		}                                                              \
		TuplePtr tuple = BUILD(Tuple, elements, count);                \
		tuple;                                                         \
	})

#endif // _BASE_TUPLE__
