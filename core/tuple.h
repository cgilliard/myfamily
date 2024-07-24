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

#ifndef _CORE_TUPLE__
#define _CORE_TUPLE__

#include <core/class.h>
#include <core/macro_utils.h>
#include <core/prim.h>
#include <core/traits_base.h>

CLASS(Tuple, FIELD(void **, elements) FIELD(u64, count))
IMPL(Tuple, TRAIT_LEN)
IMPL(Tuple, TRAIT_COPY)
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
void *Tuple_add_value_f64(void *value);
void *Tuple_add_value_f32(void *value);
void *Tuple_add_value_bool(void *value);

#define CREATE_GENERIC(value)                                                  \
	_Generic((value),                                                      \
	    __int128_t: ({                                                     \
			 void *_ptr__ = Tuple_add_value_i128(&value);          \
			 if (!_ptr__) {                                        \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 _ptr__;                                               \
		 }),                                                           \
	    int64_t: ({                                                        \
			 void *_ptr__ = Tuple_add_value_i64(&value);           \
			 if (!_ptr__) {                                        \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 _ptr__;                                               \
		 }),                                                           \
	    int32_t: ({                                                        \
			 void *_ptr__ = Tuple_add_value_i32(&value);           \
			 if (!_ptr__) {                                        \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 _ptr__;                                               \
		 }),                                                           \
	    int16_t: ({                                                        \
			 void *_ptr__ = Tuple_add_value_i16(&value);           \
			 if (!_ptr__) {                                        \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 _ptr__;                                               \
		 }),                                                           \
	    int8_t: ({                                                         \
			 void *_ptr__ = Tuple_add_value_i8(&value);            \
			 if (!_ptr__) {                                        \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 _ptr__;                                               \
		 }),                                                           \
	    __uint128_t: ({                                                    \
			 void *_ptr__ = Tuple_add_value_u128(&value);          \
			 if (!_ptr__) {                                        \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 _ptr__;                                               \
		 }),                                                           \
	    uint64_t: ({                                                       \
			 void *_ptr__ = Tuple_add_value_u64(&value);           \
			 if (!_ptr__) {                                        \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 _ptr__;                                               \
		 }),                                                           \
	    uint32_t: ({                                                       \
			 void *_ptr__ = Tuple_add_value_u32(&value);           \
			 if (!_ptr__) {                                        \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 _ptr__;                                               \
		 }),                                                           \
	    uint16_t: ({                                                       \
			 void *_ptr__ = Tuple_add_value_u16(&value);           \
			 if (!_ptr__) {                                        \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 _ptr__;                                               \
		 }),                                                           \
	    uint8_t: ({                                                        \
			 void *_ptr__ = Tuple_add_value_u8(&value);            \
			 if (!_ptr__) {                                        \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 _ptr__;                                               \
		 }),                                                           \
	    double: ({                                                         \
			 void *_ptr__ = Tuple_add_value_f64(&value);           \
			 if (!_ptr__) {                                        \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 _ptr__;                                               \
		 }),                                                           \
	    float: ({                                                          \
			 void *_ptr__ = Tuple_add_value_f32(&value);           \
			 if (!_ptr__) {                                        \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 _ptr__;                                               \
		 }),                                                           \
	    bool: ({                                                           \
			 void *_ptr__ = Tuple_add_value_bool(&value);          \
			 if (!_ptr__) {                                        \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 _ptr__;                                               \
		 }),                                                           \
	    default: ({                                                        \
			 void *_ptr__ = Tuple_add_value(&value);               \
			 if (!_ptr__) {                                        \
				 return STATIC_ALLOC_RESULT;                   \
			 }                                                     \
			 _ptr__;                                               \
		 }))

#define CREATE_GENERICP(value)                                                 \
	_Generic((value),                                                      \
	    __int128_t: ({                                                     \
			 void *_ptr__ = Tuple_add_value_i128(&value);          \
			 if (!_ptr__)                                          \
				 panic("Could not allocate "                   \
				       "sufficient memory");                   \
			 _ptr__;                                               \
		 }),                                                           \
	    int64_t: ({                                                        \
			 void *_ptr__ = Tuple_add_value_i64(&value);           \
			 if (!_ptr__)                                          \
				 panic("Could not allocate "                   \
				       "sufficient memory");                   \
			 _ptr__;                                               \
		 }),                                                           \
	    int32_t: ({                                                        \
			 void *_ptr__ = Tuple_add_value_i32(&value);           \
			 if (!_ptr__)                                          \
				 panic("Could not allocate "                   \
				       "sufficient memory");                   \
			 _ptr__;                                               \
		 }),                                                           \
	    int16_t: ({                                                        \
			 void *_ptr__ = Tuple_add_value_i16(&value);           \
			 if (!_ptr__)                                          \
				 panic("Could not allocate "                   \
				       "sufficient memory");                   \
			 _ptr__;                                               \
		 }),                                                           \
	    int8_t: ({                                                         \
			 void *_ptr__ = Tuple_add_value_i8(&value);            \
			 if (!_ptr__)                                          \
				 panic("Could not allocate "                   \
				       "sufficient memory");                   \
			 _ptr__;                                               \
		 }),                                                           \
	    __uint128_t: ({                                                    \
			 void *_ptr__ = Tuple_add_value_u128(&value);          \
			 if (!_ptr__)                                          \
				 panic("Could not allocate "                   \
				       "sufficient memory");                   \
			 _ptr__;                                               \
		 }),                                                           \
	    uint64_t: ({                                                       \
			 void *_ptr__ = Tuple_add_value_u64(&value);           \
			 if (!_ptr__)                                          \
				 panic("Could not allocate "                   \
				       "sufficient memory");                   \
			 _ptr__;                                               \
		 }),                                                           \
	    uint32_t: ({                                                       \
			 void *_ptr__ = Tuple_add_value_u32(&value);           \
			 if (!_ptr__)                                          \
				 panic("Could not allocate "                   \
				       "sufficient memory");                   \
			 _ptr__;                                               \
		 }),                                                           \
	    uint16_t: ({                                                       \
			 void *_ptr__ = Tuple_add_value_u16(&value);           \
			 if (!_ptr__)                                          \
				 panic("Could not allocate "                   \
				       "sufficient memory");                   \
			 _ptr__;                                               \
		 }),                                                           \
	    uint8_t: ({                                                        \
			 void *_ptr__ = Tuple_add_value_u8(&value);            \
			 if (!_ptr__)                                          \
				 panic("Could not allocate "                   \
				       "sufficient memory");                   \
			 _ptr__;                                               \
		 }),                                                           \
	    double: ({                                                         \
			 void *_ptr__ = Tuple_add_value_f64(&value);           \
			 if (!_ptr__)                                          \
				 panic("Could not allocate "                   \
				       "sufficient memory");                   \
			 _ptr__;                                               \
		 }),                                                           \
	    float: ({                                                          \
			 void *_ptr__ = Tuple_add_value_f32(&value);           \
			 if (!_ptr__)                                          \
				 panic("Could not allocate "                   \
				       "sufficient memory");                   \
			 _ptr__;                                               \
		 }),                                                           \
	    bool: ({                                                           \
			 void *_ptr__ = Tuple_add_value_bool(&value);          \
			 if (!_ptr__)                                          \
				 panic("Could not allocate "                   \
				       "sufficient memory");                   \
			 _ptr__;                                               \
		 }),                                                           \
	    default: ({                                                        \
			 void *_ptr__ = Tuple_add_value(&value);               \
			 if (!_ptr__)                                          \
				 panic("Could not allocate "                   \
				       "sufficient memory");                   \
			 _ptr__;                                               \
		 }))

#define ELEMENT_AT(tuple, index, dst) Tuple_element_at(tuple, index, dst)

#define TUPLE(...)                                                             \
	({                                                                     \
		void *values[] = {                                             \
		    EXPAND(FOR_EACH(CREATE_GENERIC, __VA_ARGS__))};            \
		u64 count = sizeof(values) / sizeof(values[0]);                \
		void **elements = mymalloc(count * sizeof(void *));            \
		if (!elements) {                                               \
			return STATIC_ALLOC_RESULT;                            \
		}                                                              \
		for (u64 i = 0; i < count; ++i) {                              \
			elements[i] = values[i];                               \
		}                                                              \
		TuplePtr tuple = BUILD(Tuple, elements, count);                \
		tuple;                                                         \
	})

#define TUPLEP(...)                                                            \
	({                                                                     \
		void *values[] = {                                             \
		    EXPAND(FOR_EACH(CREATE_GENERICP, __VA_ARGS__))};           \
		u64 count = sizeof(values) / sizeof(values[0]);                \
		void **elements = mymalloc(count * sizeof(void *));            \
		if (!elements)                                                 \
			panic("Could not allocate sufficient "                 \
			      "memory");                                       \
		for (u64 i = 0; i < count; ++i) {                              \
			elements[i] = values[i];                               \
		}                                                              \
		TuplePtr tuple = BUILD(Tuple, elements, count);                \
		tuple;                                                         \
	})

#endif // _CORE_TUPLE__
