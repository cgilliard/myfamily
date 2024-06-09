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
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

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

typedef struct Unit {
} Unit;
static Unit UNIT;

typedef struct Copy {
	void *obj;
	u64 size;
	void (*copy)(void *dest, void *src);
} Copy;

void copy_f32(void *dst, void *src);
void copy_f64(void *dst, void *src);
void copy_u128(void *dst, void *src);
void copy_u64(void *dst, void *src);
void copy_u32(void *dst, void *src);
void copy_u16(void *dst, void *src);
void copy_u8(void *dst, void *src);
void copy_i128(void *dst, void *src);
void copy_i64(void *dst, void *src);
void copy_i32(void *dst, void *src);
void copy_i16(void *dst, void *src);
void copy_i8(void *dst, void *src);
void copy_string(void *dst, void *src);
void copy_bool(void *dst, void *src);
void copy_unit(void *dst, void *src);
void __copy_default(void *dst, void *src);

#define COPY_NAME(x)                                                           \
	_Generic((x),                                                          \
	    f32: copy_f32,                                                     \
	    f64: copy_f64,                                                     \
	    u128: copy_u128,                                                   \
	    u64: copy_u64,                                                     \
	    u32: copy_u32,                                                     \
	    u16: copy_u16,                                                     \
	    u8: copy_u8,                                                       \
	    i128: copy_i128,                                                   \
	    i64: copy_i64,                                                     \
	    i32: copy_i32,                                                     \
	    i16: copy_i16,                                                     \
	    i8: copy_i8,                                                       \
	    String: copy_string,                                               \
	    bool: copy_bool,                                                   \
	    Unit: copy_unit,                                                   \
	    default: __copy_default)

#define COPY(c, x) build_copy(&c, &x, sizeof(x), COPY_NAME(x))
#define COPY_CUSTOM(c, x, fn) build_copy(&c, &x, sizeof(x), fn)
Copy build_copy(Copy *c, void *ptr, size_t size,
		void (*copy)(void *dest, void *src));

typedef struct BacktraceEntry {
	char *function_name;
	char *bin_name;
	char *address;
	char *file_path;
} BacktraceEntry;

typedef struct BacktraceImpl {
	BacktraceEntry *rows;
	u64 count;
} BacktraceImpl;

void backtrace_free(BacktraceImpl *ptr);
#define Backtrace BacktraceImpl CLEANUP(backtrace_free)
#define EMPTY_BACKTRACE {NULL, 0}
int backtrace_generate(Backtrace *ptr, u64 max_depth);

// #define ERROR_PRINT_FLAG_NO_COLOR 0x1
void backtrace_print(Backtrace *ptr, int flags);

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

#define ERROR_BACKTRACE_MAX_DEPTH 512

typedef struct ErrorImpl {
	ErrorKind kind;
	char msg[MAX_ERROR_MESSAGE_LEN];
	Backtrace backtrace;
} ErrorImpl;
void error_free(ErrorImpl *err);
#define Error ErrorImpl CLEANUP(error_free)
Error error_build(Error *err, ErrorKind kind, char *format, ...);
Error verror_build(Error *err, ErrorKind kind, char *format, va_list args);
#define ERROR(err, kind, ...) error_build(err, kind, ##__VA_ARGS__)

#define ERROR_PRINT_FLAG_NO_COLOR 0x1
#define ERROR_PRINT_FLAG_NO_BACKTRACE 0x1 << 1

void error_print(Error *err, int flags);

bool error_equal(Error *e1, Error *e2);
char *error_to_string(char *s, Error *e);

typedef struct ResultImpl {
	bool (*is_ok)();
	Copy value;
	Error error;
} ResultImpl;

void result_free(ResultImpl *res);
#define Result ResultImpl CLEANUP(result_free)

bool result_is_ok_false();
bool result_is_ok_true();
void *result_unwrap(Result x);
Error result_unwrap_err(Result x);
Result result_build_ok(Result *r, Copy copy);
Result result_build_err(Result *r, Error e);

#define UNWRAP(x, y)                                                           \
	*({                                                                    \
		typeof(x) ret = result_unwrap(y);                              \
		ret;                                                           \
	})
#define Unwrap1(x) result_unwrap(x)
#define CALL(res, x)                                                           \
	({                                                                     \
		if (!x.is_ok()) {                                              \
			return Err(res, Unwrap_err(x));                        \
		};                                                             \
		result_unwrap(x);                                              \
	})
#define Unwrap_or(r, d) rr2.is_ok() ? *((typeof(d) *)Unwrap1(r)) : d
#define Unwrap_err(x) result_unwrap_err(x)
#define Ok(r, c)                                                               \
	({                                                                     \
		Copy cx = COPY(cx, c);                                         \
		result_build_ok(r, cx);                                        \
	})
#define OkC(r, c, fn)                                                          \
	({                                                                     \
		Copy cx = COPY_CUSTOM(cx, c, fn);                              \
		result_build_ok(r, cx);                                        \
	})
#define Err(r, e) result_build_err(r, e)

typedef struct StringPtr {
	char *ptr;
	u64 len;
} StringPtr;

void string_free(StringPtr *s);
#define String StringPtr CLEANUP(string_free)

Result string_build(Result *res, const char *ptr);
#define TO_STR(s) s.ptr
#define STRING(r, s) string_build(r, s)
#define STRINGP(s, schar)                                                      \
	({                                                                     \
		Result rs;                                                     \
		rs = STRING(&rs, schar);                                       \
		UNWRAP(s, rs);                                                 \
	})

i64 saddi64(i64 a, i64 b);
u64 saddu64(u64 a, u64 b);

#endif /* _TYPES_BASE__ */
