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
#include <core/rc.h>
#include <core/slabs.h>
#include <core/traits_base.h>

CLASS(Tuple, FIELD(Slab, elements) FIELD(u64, count))
IMPL(Tuple, TRAIT_LEN)
IMPL(Tuple, TRAIT_COPY)
#define Tuple DEFINE_CLASS(Tuple)

void *Tuple_element_at(Tuple *t, u64 index, void *dst);
Slab Tuple_add_value(Rc value);
Slab Tuple_add_value_u128(void *value);
Slab Tuple_add_value_u64(void *value);
Slab Tuple_add_value_u32(void *value);
Slab Tuple_add_value_u16(void *value);
Slab Tuple_add_value_u8(void *value);
Slab Tuple_add_value_i128(void *value);
Slab Tuple_add_value_i64(void *value);
Slab Tuple_add_value_i32(void *value);
Slab Tuple_add_value_i16(void *value);
Slab Tuple_add_value_i8(void *value);
Slab Tuple_add_value_f64(void *value);
Slab Tuple_add_value_f32(void *value);
Slab Tuple_add_value_bool(void *value);

#define CREATE_GENERIC_TYPE(sign, bits, value)                                 \
	({                                                                     \
		Slab _ptr__ = Tuple_add_value_##sign##bits(&value);            \
		if (!_ptr__.data) {                                            \
			return STATIC_ALLOC_ERROR_RESULT;                      \
		}                                                              \
		_ptr__;                                                        \
	})

#define CREATE_GENERIC(value)                                                  \
	_Generic((value),                                                      \
	    u128: CREATE_GENERIC_TYPE(u, 128, value),                          \
	    u64: CREATE_GENERIC_TYPE(u, 64, value),                            \
	    u32: CREATE_GENERIC_TYPE(u, 32, value),                            \
	    u16: CREATE_GENERIC_TYPE(u, 16, value),                            \
	    u8: CREATE_GENERIC_TYPE(u, 8, value),                              \
	    i128: CREATE_GENERIC_TYPE(i, 128, value),                          \
	    i64: CREATE_GENERIC_TYPE(i, 64, value),                            \
	    i32: CREATE_GENERIC_TYPE(i, 32, value),                            \
	    i16: CREATE_GENERIC_TYPE(i, 16, value),                            \
	    i8: CREATE_GENERIC_TYPE(i, 8, value),                              \
	    f64: CREATE_GENERIC_TYPE(f, 64, value),                            \
	    f32: CREATE_GENERIC_TYPE(f, 32, value),                            \
	    bool: CREATE_GENERIC_TYPE(bo, ol, value),                          \
	    default: ({                                                        \
			 RcPtr _rc__ = HEAPIFY(value);                         \
			 Slab _ptr__ = Tuple_add_value(_rc__);                 \
			 if (!_ptr__.data) {                                   \
				 return STATIC_ALLOC_ERROR_RESULT;             \
			 }                                                     \
			 _ptr__;                                               \
		 }))

#define ELEMENT_AT(tuple, index, dst)                                          \
	({                                                                     \
		Tuple_element_at(&tuple, index, &dst);                         \
		dst;                                                           \
	})

#define TUPLE(...)                                                             \
	({                                                                     \
		Slab _values__tuple_impl__[] = {                               \
		    EXPAND(FOR_EACH(CREATE_GENERIC, __VA_ARGS__))};            \
		u64 _count_tuple_impl__ = sizeof(_values__tuple_impl__) /      \
					  sizeof(_values__tuple_impl__[0]);    \
		Slab _elements_tuple_impl__;                                   \
		if (mymalloc(&_elements_tuple_impl__,                          \
			     _count_tuple_impl__ * sizeof(Slab *))) {          \
			return STATIC_ALLOC_ERROR_RESULT;                      \
		}                                                              \
		Slab *_element_arr__tuple_impl__ =                             \
		    _elements_tuple_impl__.data;                               \
		for (u64 i = 0; i < _count_tuple_impl__; ++i) {                \
			_element_arr__tuple_impl__[i] =                        \
			    _values__tuple_impl__[i];                          \
		}                                                              \
		TuplePtr _tuple_return_tuple_impl__ =                          \
		    BUILD(Tuple, _elements_tuple_impl__, _count_tuple_impl__); \
		_tuple_return_tuple_impl__;                                    \
	})

#endif // _CORE_TUPLE__
