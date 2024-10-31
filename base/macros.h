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

#ifndef _BASE_MACROS__
#define _BASE_MACROS__

// Type macros
#define Type(type)                                                                                 \
	typedef struct Type *type##Nc;                                                                 \
	void type##_cleanup(const type##Nc *ptr);
#define DefineType(type) type##Nc __attribute((warn_unused_result, cleanup(type##_cleanup)))

// Ptr macros
#define $(ptr) ptr_data(ptr)
#define $len(ptr) ptr_len(ptr)
#define nil(ptr) (ptr == NULL || ptr_len(ptr) == UINT32_MAX)

// Object macros
#define $int(obj) *(int *)object_value_of(obj)
#define move(src)                                                                                  \
	({                                                                                             \
		ObjectNc _ret__ = object_move(src);                                                        \
		src = null;                                                                                \
		_ret__;                                                                                    \
	})

// String macros

/*
#define INIT_STRING {.impl = NULL}
#define EMPTY_STRING                                                                               \
	({                                                                                             \
		stringNc _empty__ = INIT_STRING;                                                           \
		_empty__;                                                                                  \
	})
#define string stringNc __attribute__((warn_unused_result, cleanup(string_cleanup)))
#define String_(v)                                                                                 \
	({                                                                                             \
		stringNc _ret__ = INIT_STRING;                                                             \
		_Generic((v),                                                                              \
			char *: ({                                                                             \
					 char *val = _Generic((v), char *: v, default: NULL);                          \
					 string_create_cs(&_ret__, (char *)val);                                       \
				 }),                                                                               \
			string: ({                                                                             \
					 stringNc val = _Generic((v), string: v, default: EMPTY_STRING);               \
					 string_create_s(&_ret__, &val);                                               \
				 }),                                                                               \
			default: string_create(&_ret__));                                                      \
		_ret__;                                                                                    \
	})
#define String(...) String_(__VA_OPT__(__VA_ARGS__) __VA_OPT__(NONE)(0ULL))

#define nil_string(s) (s.impl == NULL)

#define string_append(s, v, ...)                                                                   \
	_Generic((v), string: ({ string_append_s(&s, (string *)&v); }), default: ({                    \
				 int64 _len__ = __VA_OPT__(__VA_ARGS__) __VA_OPT__(NONE)(cstring_len((byte *)&v)); \
				 string_append_ch(&s, (byte *)&v, _len__);                                         \
			 }))
#define append(...) string_append(__VA_ARGS__)

#define string_equal_(s1, s2)                                                                      \
	_Generic((s2), string: ({ string_equal(&s1, (string *)&s2); }), default: ({                    \
				 string _tmp__ = String(s2);                                                       \
				 string_equal(&s1, &_tmp__);                                                       \
			 }))

#define equal(s1, s2) string_equal_(s1, s2)

#define len(s) string_len(&s)

#define substring(src, begin, ...)                                                                 \
	({                                                                                             \
		stringNc _ret__ = INIT_STRING;                                                             \
		int64 _len__ = __VA_OPT__(__VA_ARGS__) __VA_OPT__(NONE)(string_len(&src));                 \
		string_substring_s(&_ret__, &src, begin, _len__);                                          \
		_ret__;                                                                                    \
	})

#define char_at(s, index) string_char_at(s, index)

#define index_of(s1, s2)                                                                           \
	_Generic((s2), string: ({ string_index_of(&s1, (string *)&s2); }), default: ({                 \
				 string _tmp__ = String(s2);                                                       \
				 string_index_of(&s1, (string *)&_tmp__);                                          \
			 }))

#define last_index_of(s1, s2)                                                                      \
	_Generic((s2), string: ({ string_last_index_of(&s1, (string *)&s2); }), default: ({            \
				 string _tmp__ = String(s2);                                                       \
				 string_last_index_of(&s1, (string *)&_tmp__);                                     \
			 }))

#define move(x, y) string_move(&x, &y)
*/

#endif // _BASE_MACROS__
