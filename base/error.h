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

#ifndef _ERROR_BASE__
#define _ERROR_BASE__
#define ERROR_PRINT_FLAG_NO_COLOR 0x1
#define ERROR_PRINT_FLAG_NO_BACKTRACE 0x1 << 1

#include <base/vtable.h>
#include <string.h>

#define MAX_ERROR_KIND_LEN 128
#define MAX_ERROR_MSG_LEN 512
#define ERROR_BACKTRACE_MAX_DEPTH 512

#include <base/backtrace.h>
#include <base/types.h>

// ErrorKind

// typedef
typedef struct {
	Vtable *vtable;
	char type_str[MAX_ERROR_KIND_LEN];
} ErrorKind;

// vtable impls
bool errorkind_equal(ErrorKind *kind1, ErrorKind *kind2);

// vtable
static VtableEntry ErrorKindVtableEntries[] = {{"equal", errorkind_equal}};
DEFINE_VTABLE(ErrorKindVtable, ErrorKindVtableEntries)

// builder
ErrorKind errorkind_build(char *type_str);

// macro
#define EKind(s) {&ErrorKindVtable, s}

// Error

// typedef
typedef struct {
	Vtable *vtable;
	ErrorKind kind;
	char *msg;
	Backtrace backtrace;
} ErrorPtr;

// cleanup
void error_free(ErrorPtr *err);
#define Error ErrorPtr Cleanup(error_free)

// vtable impls
bool error_equal(Error *e1, Error *e2);
size_t error_size(Error *e);
void error_copy(Error *dst, Error *src);

// vtable
static VtableEntry ErrorVtableEntries[] = {{"copy", error_copy},
					   {"size", error_size},
					   {"equal", error_equal},
					   {"cleanup", error_free}};
DEFINE_VTABLE(ErrorVtable, ErrorVtableEntries)

// builders
Error error_build(ErrorKind kind, char *format, ...);
Error verror_build(ErrorKind kind, char *format, va_list args);

// macro
#define ERROR(kind, ...) error_build(kind, ##__VA_ARGS__)

// other functions
void error_print(Error *err, int flags);
char *error_to_string(char *s, Error *e);

#endif // _ERROR_BASE__
