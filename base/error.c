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
#include <string.h>

void ErrorKind_cleanup(ErrorKindPtr *obj) {}
size_t ErrorKind_size(ErrorKind *obj) { return sizeof(ErrorKind); }
bool ErrorKind_copy(ErrorKind *dst, ErrorKind *src) {
	void *dst_kind = ErrorKind_get_kind(dst);
	void *src_kind = ErrorKind_get_kind(src);
	memcpy(dst_kind, src_kind, MAX_ERROR_KIND_LEN);
	return true;
}
bool ErrorKind_equal(ErrorKind *obj1, ErrorKind *obj2) {
	return !strcmp((char *)ErrorKind_get_kind(obj1),
		       (char *)ErrorKind_get_kind(obj2));
}

void Error_cleanup(ErrorPtr *obj) {
	BacktracePtr *bt = Error_get_bt(obj);
	dispose(bt);
}

bool Error_copy(Error *dst, Error *src) {
	memcpy(dst->_kind._kind, src->_kind._kind, MAX_ERROR_KIND_LEN);
	memcpy(dst->_message, src->_message, MAX_ERROR_MESSAGE_LEN);
	return copy(&dst->_bt, &src->_bt);
}

size_t Error_size(Error *obj) { return sizeof(Error); }
void Error_print(Error *obj, u64 flags) {
	ErrorKindPtr *kind = Error_get_kind(obj);
	char *kind_str = (char *)ErrorKind_get_kind(kind);
	char *msg = (char *)Error_get_message(obj);

	if ((flags & ERROR_PRINT_FLAG_NO_COLOR) == 0)
		printf(ANSI_COLOR_BRIGHT_RED "%s" ANSI_COLOR_RESET
					     ": \"" ANSI_COLOR_GREEN
					     "%s" ANSI_COLOR_RESET "\"\n",
		       kind_str, msg);
	else
		printf("%s: \"%s\"\n", kind_str, msg);
	if ((flags & ERROR_PRINT_FLAG_NO_BACKTRACE) == 0)
		print(Error_get_bt(obj), flags);
}
bool Error_equal(Error *obj1, Error *obj2) {
	return equal(Error_get_kind(obj1), Error_get_kind(obj2));
}

Error error_build(ErrorKind kind, char *msg) {
	BacktracePtr bt = BUILD(Backtrace, NULL, 0);
	Backtrace_generate(&bt, 100);
	ErrorPtr ret = BUILD(Error, kind, {}, bt);
	char *err_msg = (char *)Error_get_message(&ret);
	int len = strlen(msg);
	if (len > MAX_ERROR_MESSAGE_LEN)
		len = MAX_ERROR_MESSAGE_LEN;
	memcpy(err_msg, msg, len);
	return ret;
}
