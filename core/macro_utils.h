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

#include <core/macro_utils_inner.h>

#define UNIQUE_ID __COUNTER__
#define STRINGIFY(x) #x
#define EXPAND(x) x
#define EXPAND_ALL(...) __VA_ARGS__
#define CATI(x, y) x##y
#define CAT(x, y) CATI(x, y)

#define FIRST(x, y) x
#define SECOND(x, y) y
#define BOTH(x, y) x y

#define loop while (true)

#define GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, NAME, ...) NAME

#define FOR_EACH(action, arg, delim, ...)                                      \
	__VA_OPT__(EXPAND(GET_MACRO(                                           \
	    __VA_ARGS__, FOR_EACH_10, FOR_EACH_9, FOR_EACH_8, FOR_EACH_7,      \
	    FOR_EACH_6, FOR_EACH_5, FOR_EACH_4, FOR_EACH_3, FOR_EACH_2,        \
	    FOR_EACH_1)(action, arg, delim, __VA_ARGS__)))

#define FOR_EACH_1(action, arg, delim, v0) action(arg, v0)
#define FOR_EACH_2(action, arg, delim, v0, v1)                                 \
	action(arg, v0) EXPAND_ALL delim action(arg, v1)
#define FOR_EACH_3(action, arg, delim, v0, v1, v2)                             \
	action(arg, v0) EXPAND_ALL delim action(arg, v1)                       \
	EXPAND_ALL delim action(arg, v2)
#define FOR_EACH_4(action, arg, delim, v0, v1, v2, v3)                         \
	action(arg, v0) EXPAND_ALL delim action(arg, v1)                       \
	EXPAND_ALL delim action(arg, v2)                                       \
	EXPAND_ALL delim action(arg, v3)
#define FOR_EACH_5(action, arg, delim, v0, v1, v2, v3, v4)                     \
	action(arg, v0) EXPAND_ALL delim action(arg, v1)                       \
	EXPAND_ALL delim action(arg, v2)                                       \
	EXPAND_ALL delim action(arg, v3)                                       \
	EXPAND_ALL delim action(arg, v4)
#define FOR_EACH_6(action, arg, delim, v0, v1, v2, v3, v4, v5)                 \
	action(arg, v0) EXPAND_ALL delim action(arg, v1)                       \
	EXPAND_ALL delim action(arg, v2)                                       \
	EXPAND_ALL delim action(arg, v3)                                       \
	EXPAND_ALL delim action(arg, v4)                                       \
	EXPAND_ALL delim action(arg, v5)
#define FOR_EACH_7(action, arg, delim, v0, v1, v2, v3, v4, v5, v6)             \
	action(arg, v0) EXPAND_ALL delim action(arg, v1)                       \
	EXPAND_ALL delim action(arg, v2)                                       \
	EXPAND_ALL delim action(arg, v3)                                       \
	EXPAND_ALL delim action(arg, v4)                                       \
	EXPAND_ALL delim action(arg, v5)                                       \
	EXPAND_ALL delim action(arg, v6)
#define FOR_EACH_8(action, arg, delim, v0, v1, v2, v3, v4, v5, v6, v7)         \
	action(arg, v0) EXPAND_ALL delim action(arg, v1)                       \
	EXPAND_ALL delim action(arg, v2)                                       \
	EXPAND_ALL delim action(arg, v3)                                       \
	EXPAND_ALL delim action(arg, v4)                                       \
	EXPAND_ALL delim action(arg, v5)                                       \
	EXPAND_ALL delim action(arg, v6)                                       \
	EXPAND_ALL delim action(arg, v7)
#define FOR_EACH_9(action, arg, delim, v0, v1, v2, v3, v4, v5, v6, v7, v8)     \
	action(arg, v0) EXPAND_ALL delim action(arg, v1)                       \
	EXPAND_ALL delim action(arg, v2)                                       \
	EXPAND_ALL delim action(arg, v3)                                       \
	EXPAND_ALL delim action(arg, v4)                                       \
	EXPAND_ALL delim action(arg, v5)                                       \
	EXPAND_ALL delim action(arg, v6)                                       \
	EXPAND_ALL delim action(arg, v7)                                       \
	EXPAND_ALL delim action(arg, v8)
#define FOR_EACH_10(action, arg, delim, v0, v1, v2, v3, v4, v5, v6, v7, v8,    \
		    v9)                                                        \
	action(arg, v0) EXPAND_ALL delim action(arg, v1)                       \
	EXPAND_ALL delim action(arg, v2)                                       \
	EXPAND_ALL delim action(arg, v3)                                       \
	EXPAND_ALL delim action(arg, v4)                                       \
	EXPAND_ALL delim action(arg, v5)                                       \
	EXPAND_ALL delim action(arg, v6)                                       \
	EXPAND_ALL delim action(arg, v7)                                       \
	EXPAND_ALL delim action(arg, v8)                                       \
	EXPAND_ALL delim action(arg, v9)
