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

#include <base/ekinds.h>
#include <base/formatter.h>
#include <base/unit.h>
#include <errno.h>

GETTER(Formatter, buf)
GETTER(Formatter, pos)
SETTER(Formatter, pos)
GETTER(Formatter, len)

void Formatter_cleanup(Formatter *ptr) {}

Result Formatter_write(Formatter *ptr, char *fmt, ...) {
	char *buf = *Formatter_get_buf(ptr);
	u64 len = *Formatter_get_len(ptr);
	u64 pos = *Formatter_get_pos(ptr);
	u64 rem;
	if (pos > len)
		rem = 0;
	else
		rem = len - pos;

	va_list args;
	va_list args2;
	va_copy(args2, args);
	va_start(args, fmt);
	i32 r = vsnprintf(buf + pos, rem, fmt, args);
	if (r < 0) {
		char *msg = strerror(errno);
		Error err = ERROR(WRITE_ERROR, "%s", msg);
		return Err(err);
	}
	va_end(args);

	if (r < rem) {
		Formatter_set_pos(ptr, pos + r);
		return Ok(UNIT);
	} else if (r >= rem) {
		va_start(args2, fmt);
		i32 r2 = vsnprintf(NULL, 0, fmt, args2);
		if (r2 > rem) {
			Error err =
			    ERROR(OVERFLOW,
				  "formatted string would have been %i "
				  "bytes. Only %llu bytes were available",
				  r2, rem);
			return Err(err);
		} else {
			Formatter_set_pos(ptr, pos + r);
			return Ok(UNIT);
		}
	}
	return Ok(UNIT);
}

Result Formatter_to_str_ref(Formatter *ptr) {
	char *buf = *Formatter_get_buf(ptr);
	u64 pos = *Formatter_get_pos(ptr);
	buf[pos] = 0;
	return STRING(buf);
}