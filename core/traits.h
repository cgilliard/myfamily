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

#ifndef _CORE_TRAITS__
#define _CORE_TRAITS__

#include <core/option.h>
#include <core/result.h>
#include <core/traits_base.h>

typedef enum OrdOptions {
	GreaterThan = 1,
	LessThan = -1,
	EqualTo = 0,
} OrdOptions;

#define append(dst, src)                                                       \
	_Generic((src),                                                        \
	    char *: ({ append_impl(dst, src); }),                              \
	    void *: ({ append_impl(dst, src); }),                              \
	    default: ({ append_s(dst, (Object *)src); }))

Result append_impl(void *dst, void *src);
Result append_s(void *dst, Object *src);
Rc into_iter_impl(void *obj);
void myqsort(void *arr, u64 len);
Result binsearch(void *arr, u64 len, const Object *value);

#define INTO_ITER(x)                                                           \
	({                                                                     \
		RcPtr _into_iter_rc___ = into_iter_impl(x);                    \
		_into_iter_rc___;                                              \
	})

#define TRAIT_INTO_ITER(T) TRAIT_REQUIRED(T, Rc, into_iter, T##Ptr *obj)

#define TRAIT_APPEND(T)                                                        \
	TRAIT_REQUIRED(T, Result, append, T##Ptr *dst, char *src)              \
	TRAIT_REQUIRED(T, Result, append_s, T##Ptr *dst, Object *s)

#define TRAIT_TO_STRING(T) TRAIT_REQUIRED(T, Result, to_string, T##Ptr *obj)

#define TRAIT_FORMATTER(T)                                                     \
	TRAIT_REQUIRED(T, Result, write, T##Ptr *f, char *fmt, ...)

#define TRAIT_DISPLAY(T)                                                       \
	TRAIT_REQUIRED(T, Result, fmt, T##Ptr *self, Formatter *formatter)

#define TRAIT_DEBUG(T)                                                         \
	TRAIT_REQUIRED(T, Result, dbg, T##Ptr *self, Formatter *formatter)

#define TRAIT_CMP(T)                                                           \
	TRAIT_REQUIRED(T, OrdOptions, cmp, const void *a, const void *b)

#endif // _CORE_TRAITS__
