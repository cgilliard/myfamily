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

#include <base/colors.h>
#include <base/error.h>
#include <stdlib.h>
#include <string.h>

bool errorkind_equal(ErrorKind *kind1, ErrorKind *kind2) {
	return strcmp(kind1->type_str, kind2->type_str) == 0;
}

ErrorKind errorkind_build(char *type_str) {
	ErrorKind ret;
	ret.vtable = &ErrorKindVtable;
	int len = strlen(type_str);
	if (len < MAX_ERROR_KIND_LEN) {
		strcpy(ret.type_str, type_str);
	} else {
		memcpy(ret.type_str, type_str, MAX_ERROR_KIND_LEN - 1);
		ret.type_str[MAX_ERROR_KIND_LEN - 1] = 0;
	}
	return ret;
}

void error_free(ErrorPtr *err) {
	if (err->msg) {
		// don't use tlfree because this is allocated by vasprintf which
		// uses malloc
		free(err->msg);
		err->msg = NULL;
	}
	backtrace_free(&err->backtrace);
}

Error error_build(ErrorKind kind, char *format, ...) {
	va_list args;
	va_start(args, format);
	ErrorPtr ret = verror_build(kind, format, args);
	va_end(args);
	return ret;
}
Error verror_build(ErrorKind kind, char *format, va_list args) {
	ErrorPtr err;
	err.vtable = &ErrorVtable;

	// set error message to formatted message
	if (vasprintf(&err.msg, format, args) < 0) {
		// couldn't allocate memory
		err.msg = NULL;
		// continue because we can copy the error kind over and
		// attempte to get the backtrace
	}

	// copy kind over
	err.kind = errorkind_build(kind.type_str);

	// generate a backtrace
	err.backtrace = backtrace_generate(ERROR_BACKTRACE_MAX_DEPTH);

	return err;
}

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

bool error_equal(Error *e1, Error *e2) {
	// only compare kinds of errors, not message
	return errorkind_equal(&e1->kind, &e2->kind);
}

size_t error_size(Error *e) { return sizeof(Error); }
bool error_copy(Error *dst, Error *src) {
	*dst = error_build(src->kind, "%s", src->msg);
	return true;
}
