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

#include <stddef.h>
#include <stdio.h>

#define STRINGIZE(arg) STRINGIZE1(arg)
#define STRINGIZE1(arg) STRINGIZE2(arg)
#define STRINGIZE2(arg) #arg

#define CONCATENATE(arg1, arg2) CONCATENATE1(arg1, arg2)
#define CONCATENATE1(arg1, arg2) CONCATENATE2(arg1, arg2)
#define CONCATENATE2(arg1, arg2) arg1##arg2

#define FOR_EACH_1(what, x, ...) what(x)
#define FOR_EACH_2(what, x, ...)                                               \
	what(x);                                                               \
	FOR_EACH_1(what, __VA_ARGS__);
#define FOR_EACH_3(what, x, ...)                                               \
	what(x);                                                               \
	FOR_EACH_2(what, __VA_ARGS__);
#define FOR_EACH_4(what, x, ...)                                               \
	what(x);                                                               \
	FOR_EACH_3(what, __VA_ARGS__);
#define FOR_EACH_5(what, x, ...)                                               \
	what(x);                                                               \
	FOR_EACH_4(what, __VA_ARGS__);
#define FOR_EACH_6(what, x, ...)                                               \
	what(x);                                                               \
	FOR_EACH_5(what, __VA_ARGS__);
#define FOR_EACH_7(what, x, ...)                                               \
	what(x);                                                               \
	FOR_EACH_6(what, __VA_ARGS__);
#define FOR_EACH_8(what, x, ...)                                               \
	what(x);                                                               \
	FOR_EACH_7(what, __VA_ARGS__);

#define FOR_EACH_NARG(...) FOR_EACH_NARG_(__VA_ARGS__, FOR_EACH_RSEQ_N())
#define FOR_EACH_NARG_(...) FOR_EACH_ARG_N(__VA_ARGS__)
#define FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define FOR_EACH_RSEQ_N() 8, 7, 6, 5, 4, 3, 2, 1, 0

#define FOR_EACH_(N, what, x, ...)                                             \
	CONCATENATE(FOR_EACH_, N)(what, x, __VA_ARGS__)
#define FOR_EACH(what, x, ...)                                                 \
	FOR_EACH_(FOR_EACH_NARG(x, __VA_ARGS__), what, x, __VA_ARGS__)

#define SPLIT_0(str) SPLIT_1(str)
#define SPLIT_1(str) SPLIT_2(str)
#define SPLIT_2(str) SPLIT_3(str)
#define SPLIT_3(str) SPLIT_4(str)
#define SPLIT_4(str) SPLIT_5(str)
#define SPLIT_5(str) SPLIT_6(str)
#define SPLIT_6(str) SPLIT_7(str)
#define SPLIT_7(str) SPLIT_8(str)
#define SPLIT_8(str) SPLIT_9(str)
#define SPLIT_9(str) str

#define GET_FIRST(str) GET_FIRST_I(str, ;, )
#define GET_FIRST_I(str, delim, ...) GET_FIRST_II(str, delim, __VA_ARGS__)
#define GET_FIRST_II(first, ...) first

#define GET_REST(str) GET_REST_I(str, ;, )
#define GET_REST_I(str, delim, ...) GET_REST_II(__VA_ARGS__)
#define GET_REST_II(first, ...) __VA_ARGS__

#define SPLIT(str) SPLIT_0(str)
