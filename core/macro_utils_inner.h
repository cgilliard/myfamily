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

#define EXPAND_INNER(x) x
#define EXPAND_INNER_ALL(...) __VA_ARGS__
#define GET_MACRO_INNER(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, NAME, ...) NAME

#define FOR_EACH_INNER(action_inner, arg, delim, ...)                          \
	EXPAND_INNER(GET_MACRO_INNER(                                          \
	    __VA_ARGS__, FOR_EACH_INNER_10, FOR_EACH_INNER_9,                  \
	    FOR_EACH_INNER_8, FOR_EACH_INNER_7, FOR_EACH_INNER_6,              \
	    FOR_EACH_INNER_5, FOR_EACH_INNER_4, FOR_EACH_INNER_3,              \
	    FOR_EACH_INNER_2,                                                  \
	    FOR_EACH_INNER_1)(action_inner, arg, delim, __VA_ARGS__))

#define FOR_EACH_INNER_1(action_inner, arg, delim, v0) action_inner(arg, v0)
#define FOR_EACH_INNER_2(action_inner, arg, delim, v0, v1)                     \
	action_inner(arg, v0) EXPAND_INNER_ALL delim action_inner(arg, v1)
#define FOR_EACH_INNER_3(action_inner, arg, delim, v0, v1, v2)                 \
	action_inner(arg, v0) EXPAND_INNER_ALL delim action_inner(arg, v1)     \
	EXPAND_INNER_ALL delim action_inner(arg, v2)
#define FOR_EACH_INNER_4(action_inner, arg, delim, v0, v1, v2, v3)             \
	action_inner(arg, v0) EXPAND_INNER_ALL delim action_inner(arg, v1)     \
	EXPAND_INNER_ALL delim action_inner(arg, v2)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v3)
#define FOR_EACH_INNER_5(action_inner, arg, delim, v0, v1, v2, v3, v4)         \
	action_inner(arg, v0) EXPAND_INNER_ALL delim action_inner(arg, v1)     \
	EXPAND_INNER_ALL delim action_inner(arg, v2)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v3)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v4)
#define FOR_EACH_INNER_6(action_inner, arg, delim, v0, v1, v2, v3, v4, v5)     \
	action_inner(arg, v0) EXPAND_INNER_ALL delim action_inner(arg, v1)     \
	EXPAND_INNER_ALL delim action_inner(arg, v2)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v3)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v4)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v5)
#define FOR_EACH_INNER_7(action_inner, arg, delim, v0, v1, v2, v3, v4, v5, v6) \
	action_inner(arg, v0) EXPAND_INNER_ALL delim action_inner(arg, v1)     \
	EXPAND_INNER_ALL delim action_inner(arg, v2)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v3)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v4)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v5)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v6)
#define FOR_EACH_INNER_8(action_inner, arg, delim, v0, v1, v2, v3, v4, v5, v6, \
			 v7)                                                   \
	action_inner(arg, v0) EXPAND_INNER_ALL delim action_inner(arg, v1)     \
	EXPAND_INNER_ALL delim action_inner(arg, v2)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v3)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v4)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v5)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v6)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v7)
#define FOR_EACH_INNER_9(action_inner, arg, delim, v0, v1, v2, v3, v4, v5, v6, \
			 v7, v8)                                               \
	action_inner(arg, v0) EXPAND_INNER_ALL delim action_inner(arg, v1)     \
	EXPAND_INNER_ALL delim action_inner(arg, v2)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v3)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v4)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v5)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v6)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v7)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v8)
#define FOR_EACH_INNER_10(action_inner, arg, delim, v0, v1, v2, v3, v4, v5,    \
			  v6, v7, v8, v9)                                      \
	action_inner(arg, v0) EXPAND_INNER_ALL delim action_inner(arg, v1)     \
	EXPAND_INNER_ALL delim action_inner(arg, v2)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v3)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v4)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v5)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v6)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v7)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v8)                           \
	EXPAND_INNER_ALL delim action_inner(arg, v9)
