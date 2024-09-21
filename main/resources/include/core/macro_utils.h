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

#define UNIQUE_ID __COUNTER__
#define STRINGIFY(x) #x
#define EXPAND(x) x
#define EXPAND_ALL(...) __VA_ARGS__
#define EXPAND_ALL_INNER(...) __VA_ARGS__
#define CATI(x, y) x##y
#define CAT(x, y) CATI(x, y)
#define PAREN (
#define PAREN_END )

#define NONE(...)
#define FIRST(x, y) x
#define FIRST_STRINGIFY(x, y) #x
#define SECOND(x, y) y
#define BOTH(x, y) x y

#define loop while (true)
#define π(...)                                                                                     \
	while (true)                                                                                   \
	sleep(UINT_MAX)

#define MULTI_PARAM(action, ...) action(__VA_ARGS__)
#define SINGLE_PARAM(action, x) action(x)
#define MULTI_SWITCH_(action1, action2, x, ...)                                                    \
	EXPAND_ALL EXPAND(__VA_OPT__(NONE)(SINGLE_PARAM(action1, x))                                   \
			__VA_OPT__((MULTI_PARAM(action2, x, __VA_ARGS__))))
#define MULTI_SWITCH(action1, action2, ...) EXPAND(MULTI_SWITCH_(action1, action2, __VA_ARGS__))

#define DOUBLE_PARAM(action, x, y) action(x, y)
#define MULTI_SWITCH2_(action1, action2, arg, x, ...)                                              \
	EXPAND_ALL EXPAND(__VA_OPT__(NONE)(DOUBLE_PARAM(action1, arg, x))                              \
			__VA_OPT__((MULTI_PARAM(action2, arg, x, __VA_ARGS__))))
#define MULTI_SWITCH2(action1, action2, arg, ...)                                                  \
	EXPAND(MULTI_SWITCH2_(action1, action2, arg, __VA_ARGS__))

#define MULTI_PARAM_INNER(action, ...) action(__VA_ARGS__)
#define DOUBLE_PARAM_INNER(action, x, y) action(x, y)
#define MULTI_SWITCH2_INNER_(action1, action2, arg, x, ...)                                        \
	EXPAND_ALL EXPAND(__VA_OPT__(NONE)(DOUBLE_PARAM_INNER(action1, arg, x))                        \
			__VA_OPT__((MULTI_PARAM_INNER(action2, arg, x, __VA_ARGS__))))
#define MULTI_SWITCH2_INNER(action1, action2, arg, ...)                                            \
	EXPAND(MULTI_SWITCH2_INNER_(action1, action2, arg, __VA_ARGS__))

#define EMPTY()
#define DEFER1(m) m EMPTY()
#define UNWRAP(...) __VA_ARGS__

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

#define MAP(m, arg, delim, first, ...)                                                             \
	m(arg, first) __VA_OPT__(UNWRAP delim DEFER1(_MAP)()(m, arg, delim, __VA_ARGS__))
#define _MAP() MAP

#define EVAL_INNER(...) EVAL1024_INNER(__VA_ARGS__)
#define EVAL1024_INNER(...) EVAL512_INNER(EVAL512_INNER(__VA_ARGS__))
#define EVAL512_INNER(...) EVAL256_INNER(EVAL256_INNER(__VA_ARGS__))
#define EVAL256_INNER(...) EVAL128_INNER(EVAL128_INNER(__VA_ARGS__))
#define EVAL128_INNER(...) EVAL64_INNER(EVAL64_INNER(__VA_ARGS__))
#define EVAL64_INNER(...) EVAL32_INNER(EVAL32_INNER(__VA_ARGS__))
#define EVAL32_INNER(...) EVAL16_INNER(EVAL16_INNER(__VA_ARGS__))
#define EVAL16_INNER(...) EVAL8_INNER(EVAL8_INNER(__VA_ARGS__))
#define EVAL8_INNER(...) EVAL4_INNER(EVAL4_INNER(__VA_ARGS__))
#define EVAL4_INNER(...) EVAL2_INNER(EVAL2_INNER(__VA_ARGS__))
#define EVAL2_INNER(...) EVAL1_INNER(EVAL1_INNER(__VA_ARGS__))
#define EVAL1_INNER(...) __VA_ARGS__

#define MAP_INNER(m, arg, delim, first, ...)                                                       \
	m(arg, first) __VA_OPT__(UNWRAP delim DEFER1(_MAP_INNER)()(m, arg, delim, __VA_ARGS__))
#define _MAP_INNER() MAP_INNER

#define FOR_EACH(m, arg, delim, ...) EVAL(__VA_OPT__(MAP(m, arg, delim, __VA_ARGS__)))

#define FOR_EACH_INNER(m, arg, delim, ...) __VA_OPT__(MAP_INNER(m, arg, delim, __VA_ARGS__))
