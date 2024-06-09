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

#include <base/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool errorkind_equal(ErrorKind *kind1, ErrorKind *kind2) {
	return strcmp(kind1->type_str, kind2->type_str) == 0;
}

void error_free(ErrorImpl *err) { backtrace_free(&err->backtrace); }

bool error_equal(Error *e1, Error *e2) {
	// only compare kinds of errors, not message
	return errorkind_equal(&e1->kind, &e2->kind);
}

char *error_to_string(char *s, Error *e) {
	strcpy(s, e->kind.type_str);
	strcat(s, ": ");
	strcat(s, e->msg);
	return s;
}

Error verror_build(Error *err, ErrorKind kind, char *format, va_list args) {

	// copy the message over in formatted form
	strcpy(err->msg, "");
	vsprintf(err->msg, format, args);

	// either take the length of the type_str or the full array if an error
	// occurs
	int len = strlen(kind.type_str);
	if (len < 0 || len >= MAX_ERROR_KIND_LEN)
		len = MAX_ERROR_KIND_LEN - 1;

	memcpy(err->kind.type_str, kind.type_str, len);
	err->kind.type_str[len] = 0;

	// generate a backtrace
	err->backtrace.rows = NULL;
	err->backtrace.count = 0;
	backtrace_generate(&err->backtrace, ERROR_BACKTRACE_MAX_DEPTH);

	return *err;
}

Error error_build(Error *err, ErrorKind kind, char *format, ...) {
	va_list args;
	va_start(args, format);
	verror_build(err, kind, format, args);
	va_end(args);
	return *err;
}

#define ANSI_COLOR_DIMMED "\x1b[2m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_BRIGHT_RED "\x1b[91m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

void error_print(Error *err, int flags) {
	if ((flags & ERROR_PRINT_FLAG_NO_COLOR) == 0)
		printf(ANSI_COLOR_BRIGHT_RED "%s" ANSI_COLOR_RESET
					     ": \"" ANSI_COLOR_GREEN
					     "%s" ANSI_COLOR_RESET "\"\n",
		       err->kind.type_str, err->msg);
	else
		printf("%s: \"%s\"\n", err->kind.type_str, err->msg);
	if ((flags & ERROR_PRINT_FLAG_NO_BACKTRACE) == 0)
		backtrace_print(&err->backtrace, flags);
}
