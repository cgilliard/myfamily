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

#include <base/limits.h>

#define UNIQUE_ID __COUNTER__
#define STRINGIFY(x) #x
#define EXPAND(x) x
#define EXPAND_ALL(...) __VA_ARGS__
#define CATI(x, y) x##y
#define CAT(x, y) CATI(x, y)
#define PAREN (
#define PAREN_END )

#define NONE(...)
#define FIRST(x, y) x
#define SECOND(x, y) y
#define FIRST_STRINGIFY(x, y) #x
#define SECOND_STRINGIFY(x, y) #y
#define BOTH(x, y) x y

#define container_of(ptr, type, member)                    \
	({                                                     \
		const typeof(((type *)0)->member) *__mptr = (ptr); \
		(type *)((char *)__mptr - offsetof(type, member)); \
	})

#define loop while (true)
#define π(...) \
	while (true) sleep(INT_MAX)

#define EMPTY()
#define DEFER1(m) m EMPTY()

#define EVAL(...) EVAL1024(__VA_ARGS__)
#define EVAL1024(...) EVAL512(EVAL512(__VA_ARGS__))
#define EVAL512(...) EVAL256(EVAL256(__VA_ARGS__))
#define EVAL256(...) EVAL128(EVAL128(__VA_ARGS__))
#define EVAL128(...) EVAL64(EVAL64(__VA_ARGS__))
#define EVAL64(...) EVAL32(EVAL32(__VA_ARGS__))
#define EVAL32(...) EVAL16(EVAL16(__VA_ARGS__))
#define EVAL16(...) EVAL8(EVAL8(__VA_ARGS__))
#define EVAL8(...) EVAL4(EVAL4(__VA_ARGS__))
#define EVAL4(...) EVAL2(EVAL2(__VA_ARGS__))
#define EVAL2(...) EVAL1(EVAL1(__VA_ARGS__))
#define EVAL1(...) __VA_ARGS__

#define MAP(m, arg, delim, first, ...) \
	m(arg, first) __VA_OPT__(          \
		EXPAND_ALL delim DEFER1(_MAP)()(m, arg, delim, __VA_ARGS__))
#define _MAP() MAP

#define FOR_EACH(m, arg, delim, ...) \
	EVAL(__VA_OPT__(MAP(m, arg, delim, __VA_ARGS__)))
