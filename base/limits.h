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

#ifndef _BASE_LIMITS__
#define _BASE_LIMITS__

#include <base/macro_util.h>

#define SHIFT(ignore, v) (1ULL << v)
#define BIT_SHIFT_OR(...) FOR_EACH(SHIFT, ignore, (|), __VA_ARGS__)

#define BYTE_MAX_IMPL (BIT_SHIFT_OR(0, 1, 2, 3, 4, 5, 6, 7))
#ifndef BYTE_MAX
#define BYTE_MAX BYTE_MAX_IMPL
#endif // BYTE_MAX

#define INT_MAX_IMPL                                                                               \
	(BIT_SHIFT_OR(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,    \
				  22, 23, 24, 25, 26, 27, 28, 29, 30))
#ifndef INT_MAX
#define INT_MAX INT_MAX_IMPL
#endif // INT_MAX

#define UINT32_MAX_IMPL                                                                            \
	(BIT_SHIFT_OR(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,    \
				  22, 23, 24, 25, 26, 27, 28, 29, 30, 31))
#ifndef UINT32_MAX
#define UINT32_MAX UINT32_MAX_IMPL
#endif // UINT32_MAX

#define INT64_MAX_IMPL                                                                             \
	(BIT_SHIFT_OR(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,    \
				  22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41,  \
				  42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,  \
				  62))
#ifndef INT64_MAX
#define INT64_MAX INT64_MAX_IMPL
#endif // INT64_MAX

#define INT_MIN_IMPL (-INT_MAX_IMPL - 1)
#ifndef INT_MIN
#define INT_MIN INT_MIN_IMPL
#endif // INT_MIN

#endif // _BASE_LIMITS__
