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

#define NIL                                                                                        \
	(const ObjectNc) {                                                                             \
		.impl = rc_null, .flags = 0                                                                \
	}

#define UNIT                                                                                       \
	(const ObjectNc) {                                                                             \
		.impl = 0, .flags = OBJECT_FLAG_NO_CLEANUP                                                 \
	}

#define $objnil(v) fat_ptr_is_nil(&(&((ObjectNc *)&(v))->impl)->impl)
#define obj_nil(v)                                                                                 \
	_Generic((v),                                                                                  \
		Object: $objnil(v),                                                                        \
		FatPtr: fat_ptr_is_nil((FatPtr *)&v),                                                      \
		Rc: fat_ptr_is_nil(&((Rc *)&v)->impl),                                                     \
		default: ({}))
#undef nil
#define nil(v) obj_nil(v)
