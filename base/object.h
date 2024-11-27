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

#ifndef _BASE_OBJECT__
#define _BASE_OBJECT__

typedef __int128_t Object;

typedef enum ObjectType {
	Int,
	UInt,
	Float,
	Box,
	Function,
	Err,
} ObjectType;

#define $(v)                                                                   \
	_Generic((v),                                                              \
		double: object_float(_Generic((v), double: v, default: 0)),            \
		float: object_float(_Generic((v), float: v, default: 0)),              \
		long long: object_int(_Generic((v), long long: v, default: 0)),        \
		int: object_int(_Generic((v), int: v, default: 0)),                    \
		unsigned long long: object_uint(                                       \
				 _Generic((v), unsigned long long: v, default: 0)),            \
		unsigned int: object_uint(_Generic((v), unsigned int: v, default: 0)), \
		default: ({                                                            \
				 object_function(_Generic((v),                                 \
					 double: 0,                                                \
					 int: 0,                                                   \
					 long long: 0,                                             \
					 unsigned long long: 0,                                    \
					 default: v));                                             \
			 }))

#define $fn(v) value_of(v)
#define $int(obj) (*(long long *)value_of(&obj))
#define $float(obj) (*(double *)value_of(&obj))
#define $uint(obj) (*(unsigned long long *)value_of(&obj))
#define let const Object
#define var Object

Object object_int(long long value);
Object object_uint(unsigned long long value);
Object object_float(double value);
Object object_function(void *fn);
Object object_err(int code);
Object box(long long size);
const void *value_of(const Object *obj);
const void *value_of_fn(const Object *obj);
ObjectType object_type(const Object *obj);

#endif	// _BASE_OBJECT__
