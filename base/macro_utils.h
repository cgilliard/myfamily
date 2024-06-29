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

#define EXPAND(x) x
#define CATI(x, y) x##y
#define CAT(x, y) CATI(x, y)

#define GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, NAME,     \
		  ...)                                                         \
	NAME
#define FOR_EACH(action, ...)                                                  \
	EXPAND(GET_MACRO(__VA_ARGS__, FOR_EACH_12, FOR_EACH_11, FOR_EACH_10,   \
			 FOR_EACH_9, FOR_EACH_8, FOR_EACH_7, FOR_EACH_6,       \
			 FOR_EACH_5, FOR_EACH_4, FOR_EACH_3, FOR_EACH_2,       \
			 FOR_EACH_1)(action, __VA_ARGS__))

#define FOR_EACH_1(action, a) action(a)
#define FOR_EACH_2(action, a, b) action(a), action(b)
#define FOR_EACH_3(action, a, b, c) action(a), action(b), action(c)
#define FOR_EACH_4(action, a, b, c, d)                                         \
	action(a), action(b), action(c), action(d)
#define FOR_EACH_5(action, a, b, c, d, e)                                      \
	action(a), action(b), action(c), action(d), action(e)
#define FOR_EACH_6(action, a, b, c, d, e, f)                                   \
	action(a), action(b), action(c), action(d), action(e), action(f)
#define FOR_EACH_7(action, a, b, c, d, e, f, g)                                \
	action(a), action(b), action(c), action(d), action(e), action(f),      \
	    action(g)
#define FOR_EACH_8(action, a, b, c, d, e, f, g, h)                             \
	action(a), action(b), action(c), action(d), action(e), action(f),      \
	    action(g), action(h)
#define FOR_EACH_9(action, a, b, c, d, e, f, g, h, i)                          \
	action(a), action(b), action(c), action(d), action(e), action(f),      \
	    action(g), action(h), action(i)
#define FOR_EACH_10(action, a, b, c, d, e, f, g, h, i, j)                      \
	action(a), action(b), action(c), action(d), action(e), action(f),      \
	    action(g), action(h), action(i), action(j)
#define FOR_EACH_11(action, a, b, c, d, e, f, g, h, i, j, k)                   \
	action(a), action(b), action(c), action(d), action(e), action(f),      \
	    action(g), action(h), action(i), action(j), action(k)
#define FOR_EACH_12(action, a, b, c, d, e, f, g, h, i, j, k, l)                \
	action(a), action(b), action(c), action(d), action(e), action(f),      \
	    action(g), action(h), action(i), action(j), action(k), action(l)

