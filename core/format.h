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

#ifndef _CORE_FORMAT__
#define _CORE_FORMAT__

#include <core/formatter.h>
#include <core/result.h>
#include <core/string.h>

Result format(FormatterPtr *formatter, char *fmt, ...);

#define PROC_TYPE(sign_upper, sign_lower, bits, value)                         \
	__arg_arr__[__itt___] = mymalloc(sizeof(sign_upper##bits));            \
	sign_upper##bits##Ptr *__ptr___ = __arg_arr__[__itt___];               \
	BUILDPTR(__ptr___, sign_upper##bits);                                  \
	memcpy(&__ptr___->_value, &value, sizeof(sign_lower##bits));           \
	__itt___++;                                                            \
	__arg_arr__[__itt___ - 1];

#define PROC_TYPE_ISIZE(value)                                                 \
	__arg_arr__[__itt___] = mymalloc(sizeof(isize));                       \
	ISizePtr *__ptr___ = __arg_arr__[__itt___];                            \
	BUILDPTR(__ptr___, ISize);                                             \
	memcpy(&__ptr___->_value, &value, sizeof(isize));                      \
	__itt___++;                                                            \
	__arg_arr__[__itt___ - 1];

#define PROC_TYPE_USIZE(value)                                                 \
	__arg_arr__[__itt___] = mymalloc(sizeof(usize));                       \
	USizePtr *__ptr___ = __arg_arr__[__itt___];                            \
	BUILDPTR(__ptr___, USize);                                             \
	memcpy(&__ptr___->_value, &value, sizeof(usize));                      \
	__itt___++;                                                            \
	__arg_arr__[__itt___ - 1];

#define PROC_ARG(value)                                                        \
	_Generic((value),                                                      \
	    i8: ({PROC_TYPE(I, i, 8, value)}),                                 \
	    i16: ({PROC_TYPE(I, i, 16, value)}),                               \
	    i32: ({PROC_TYPE(I, i, 32, value)}),                               \
	    i64: ({PROC_TYPE(I, i, 64, value)}),                               \
	    i128: ({PROC_TYPE(I, i, 128, value)}),                             \
	    u8: ({PROC_TYPE(U, u, 8, value)}),                                 \
	    u16: ({PROC_TYPE(U, u, 16, value)}),                               \
	    u32: ({PROC_TYPE(U, u, 32, value)}),                               \
	    u64: ({PROC_TYPE(U, u, 64, value)}),                               \
	    u128: ({PROC_TYPE(U, u, 128, value)}),                             \
	    f32: ({PROC_TYPE(F, f, 32, value)}),                               \
	    f64: ({PROC_TYPE(F, f, 64, value)}),                               \
	    isize: ({PROC_TYPE_ISIZE(value)}),                                 \
	    usize: ({PROC_TYPE_USIZE(value)}),                                 \
	    bool: ({                                                           \
			 __arg_arr__[__itt___] = mymalloc(sizeof(Bool));       \
			 BoolPtr *__ptr___ = __arg_arr__[__itt___];            \
			 BUILDPTR(__ptr___, Bool);                             \
			 memcpy(&__ptr___->_value, &value, sizeof(bool));      \
			 __itt___++;                                           \
			 __arg_arr__[__itt___ - 1];                            \
		 }),                                                           \
	    default: ({ &value; }))

#define COUNT_FORMAT(value) ({ __counter___ += 1; })

#define FORMAT(f, fmt, ...)                                                    \
	({                                                                     \
		u64 __counter___ = 0;                                          \
		EXPAND(FOR_EACH(COUNT_FORMAT, __VA_ARGS__));                   \
		void **__arg_arr__ = mymalloc(sizeof(void *) * __counter___);  \
		for (u64 __i___ = 0; __i___ < __counter___; __i___++)          \
			__arg_arr__[__i___] = NULL;                            \
		u64 __itt___ = 0;                                              \
		ResultPtr __ret__ =                                            \
		    format(f, fmt __VA_OPT__(, )                               \
				  EXPAND(FOR_EACH(PROC_ARG, __VA_ARGS__)));    \
		for (u64 __i___ = 0; __i___ < __counter___; __i___++) {        \
			if (__arg_arr__[__i___])                               \
				myfree(__arg_arr__[__i___]);                   \
		}                                                              \
		myfree(__arg_arr__);                                           \
		__ret__;                                                       \
	})

#endif // _CORE_FORMAT__
