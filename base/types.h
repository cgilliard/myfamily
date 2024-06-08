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

#ifndef _TYPES_BASE__
#define _TYPES_BASE__

#include <base/cleanup.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define u8 uint8_t
#define i8 int8_t
#define u16 uint16_t
#define i16 int16_t
#define u32 uint32_t
#define i32 int32_t
#define u64 uint64_t
#define i64 int64_t
#define u128 __uint128_t
#define i128 __int128_t
#define f64 double
#define f32 float

bool option_is_some_false();
bool option_is_some_true();

typedef struct OptionImpl {
	bool (*is_some)();
	void *ref;
} OptionImpl;
void option_free(OptionImpl *ptr);
#define Option OptionImpl CLEANUP(option_free)

Option option_build(Option *opt, void *x, size_t size);
void *option_unwrap(Option x);

#define None {option_is_some_false, NULL}
#define Some(opt, x) option_build(opt, &x, sizeof(x))
#define Unwrap(x) option_unwrap(x)

#define MAX_ERROR_KIND_LEN 128
#define MAX_ERROR_MESSAGE_LEN 512

typedef struct ErrorKind {
	char type_str[MAX_ERROR_KIND_LEN];
} ErrorKind;

bool errorkind_equal(ErrorKind *kind1, ErrorKind *kind2);

typedef struct ErrorImpl {
	ErrorKind kind;
	char msg[MAX_ERROR_MESSAGE_LEN];
} ErrorImpl;
void error_free(ErrorImpl *err);
#define Error ErrorImpl CLEANUP(error_free)

bool error_equal(Error *e1, Error *e2);
char *error_to_string(char *s, Error *e);

typedef struct StringImpl {
	char *ptr;
	u64 len;
} StringImpl;

void string_free(StringImpl *s);
#define String StringImpl CLEANUP(string_free)

int string_set(StringImpl *s, const char *ptr);
#define STRING_INIT(s, value) string_set(&s, value)

i64 saddi64(i64 a, i64 b);
u64 saddu64(u64 a, u64 b);

#endif /* _TYPES_BASE__ */
