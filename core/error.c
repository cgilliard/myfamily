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

#include <core/colors.h>
#include <core/error.h>
#include <string.h>

GETTER(Error, flags)
SETTER(Error, flags)

void ErrorKind_cleanup(ErrorKindPtr *obj) {}
bool ErrorKind_clone(ErrorKind *dst, ErrorKind *src) {
	void *dst_kind = ErrorKind_get_kind(dst);
	void *src_kind = ErrorKind_get_kind(src);
	memcpy(dst_kind, src_kind, MAX_ERROR_KIND_LEN);
	return true;
}
bool ErrorKind_equal(ErrorKind *obj1, ErrorKind *obj2) {
	return !strcmp((char *)ErrorKind_get_kind(obj1),
		       (char *)ErrorKind_get_kind(obj2));
}

#include <core/result.h>

void Error_cleanup(ErrorPtr *obj) {
	u64 flags = *Error_get_flags(obj);

	if (!(flags && ERROR_NO_CLEANUP)) {
		BacktracePtr *bt = Error_get_bt(obj);
		cleanup(bt);
	}
}

bool Error_clone(Error *dst, Error *src) {
	ErrorKindPtr *dst_kind = Error_get_kind(dst);
	ErrorKindPtr *src_kind = Error_get_kind(src);
	if (!copy(dst_kind, src_kind))
		return false;

	char *src_msg = (char *)Error_get_message(src);
	char *dst_msg = (char *)Error_get_message(dst);
	memcpy(dst_msg, src_msg, MAX_ERROR_MESSAGE_LEN);

	BacktracePtr *bt = Error_get_bt(dst);
	BacktracePtr *src_bt = Error_get_bt(src);

	Error_set_flags(dst, *Error_get_flags(src));
	return copy(bt, src_bt);
}

void Error_print(Error *obj) {
	ErrorKindPtr *kind = Error_get_kind(obj);
	char *kind_str = (char *)ErrorKind_get_kind(kind);
	char *msg = (char *)Error_get_message(obj);

	printf("%s%s%s: \"%s%s%s\"\n", RED, kind_str, RESET, GREEN, msg, RESET);
	u64 flags = *Error_get_flags(obj);

	if ((flags & ERROR_PRINT_FLAG_NO_BACKTRACE) == 0) {
		void *bt = Error_get_bt(obj);
		print(bt);
	}
}
bool Error_equal(Error *obj1, Error *obj2) {
	return equal(Error_get_kind(obj1), Error_get_kind(obj2));
}

Error error_build(ErrorKind kind, char *fmt, ...) {
	BacktracePtr bt = BUILD(Backtrace, NULL, 0);
	Backtrace_generate(&bt, 100);
	ErrorPtr ret = BUILD(Error, kind, {}, bt);
	char *err_msg = (char *)Error_get_message(&ret);
	va_list args;
	va_start(args, fmt);
	u64 len = vsnprintf(err_msg, MAX_ERROR_MESSAGE_LEN, fmt, args);
	va_end(args);
	return ret;
}
