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

#include <base/macro_utils.h>

#define let const Object
#define var Object

#define NIL {.impl = rc_null, .flags = null}

#define $objnil(v) fat_ptr_is_nil(&(&((ObjectNc *)&(v))->impl)->impl)
#define obj_nil(v)                                                                                 \
	_Generic((v),                                                                                  \
		Object: $objnil(v),                                                                        \
		FatPtr: fat_ptr_is_nil((FatPtr *)&v),                                                      \
		Rc: fat_ptr_is_nil(&((Rc *)&v)->impl),                                                     \
		default: ({}))
#undef nil
#define nil(v) obj_nil(v)

#define $u64(obj)                                                                                  \
	({                                                                                             \
		u64 _ret__;                                                                                \
		object_as_u64(&obj, &_ret__);                                                              \
		_ret__;                                                                                    \
	})

#define $string(obj) object_as_string(&obj)

#define $create_empty()                                                                            \
	({                                                                                             \
		ObjectNc _ret__ = NIL;                                                                     \
		object_create(&_ret__, false, ObjectTypeObject, NULL);                                     \
		_ret__;                                                                                    \
	})

#define $set_property(obj, k, v)                                                                   \
	_Generic((v),                                                                                  \
		u64: ({                                                                                    \
				 ObjectNc _v__ = NIL;                                                              \
				 object_create(&_v__, false, ObjectTypeU64, &v);                                   \
				 if (!nil(_v__))                                                                   \
					 object_set_property(&obj, k, &_v__);                                          \
			 }),                                                                                   \
		Object: ({                                                                                 \
				 ObjectNc _v__ = NIL;                                                              \
				 object_create(&_v__, false, ObjectTypeObject, &v);                                \
				 if (!nil(_v__))                                                                   \
					 object_set_property(&obj, k, &_v__);                                          \
			 }),                                                                                   \
		default: ({                                                                                \
				 ObjectNc _v__ = NIL;                                                              \
				 const char *_vin__ = _Generic((v), char *: v, default: NULL);                     \
				 object_create(&_v__, false, ObjectTypeString, _vin__);                            \
				 if (!nil(_v__))                                                                   \
					 object_set_property(&obj, k, &_v__);                                          \
			 }))

#define $get_property_(obj, k)                                                                     \
	({                                                                                             \
		ObjectNc _ret__ = object_get_property(&obj, k);                                            \
		_ret__;                                                                                    \
	})

#define $get_property(obj, ...)                                                                    \
	__VA_OPT__($get_property_(obj, __VA_ARGS__)) __VA_OPT__(NONE)($create_empty())

#define $object_impl_(obj, k, ...)                                                                 \
	__VA_OPT__($set_property(obj, k, __VA_ARGS__)) __VA_OPT__(NONE)($get_property(obj, k))

#define $object_impl(v, ...)                                                                       \
	_Generic((v), Object: $object_impl_(v, __VA_ARGS__), default: $rc_impl(v))
#undef $
#define $(...) __VA_OPT__($object_impl(__VA_ARGS__)) __VA_OPT__(NONE)($create_empty())
