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
#include <base/io.h>
#include <base/unit.h>

#define RTS_BUF_SIZE 1024

Result myread(void *obj, char *buf, u64 limit) {
	ResultPtr (*do_read)(Object *obj, char *buf, u64 limit) =
	    find_fn((Object *)obj, "read");
	if (do_read == NULL)
		panic("read not implemented for this type [%s]",
		      CLASS_NAME(obj));
	return do_read(obj, buf, limit);
}

Result read_to_string(void *obj) {
	ResultPtr (*do_read_to_string)(Object *obj) =
	    find_fn((Object *)obj, "read_to_string");
	if (do_read_to_string == NULL)
		panic("read_to_string not implemented for this type [%s]",
		      CLASS_NAME(obj));
	return do_read_to_string(obj);
}

Result read_exact(void *obj, char *buf, u64 limit) {
	ResultPtr (*do_read)(Object *obj, char *buf, u64 limit) =
	    find_fn((Object *)obj, "read_exact");
	if (do_read == NULL)
		panic("read_exact not implemented for this type [%s]",
		      CLASS_NAME(obj));
	return do_read(obj, buf, limit);
}

Result myseek(void *obj, u64 pos) {
	ResultPtr (*do_seek)(Object *obj, u64 pos) =
	    find_fn((Object *)obj, "seek");
	if (do_seek == NULL)
		panic("seek not implemented for this type [%s]",
		      CLASS_NAME(obj));
	return do_seek(obj, pos);
}

Result read_to_string_impl(Object *self) {
	char buf[RTS_BUF_SIZE + 1];
	Result r0 = STRING("");
	StringRef ret = Try(StringRef, r0);
	while (true) {
		Result r = myread(self, buf, RTS_BUF_SIZE);
		u64 rlen = *(u64 *)unwrap(&r);
		buf[rlen] = 0;

		if (rlen > 0) {
			Result r1 = STRING(buf);
			StringRef next = Try(StringRef, r1);
			Result r2 = append(&ret, &next);
			Try(Unit, r2);
		}

		if (rlen < RTS_BUF_SIZE)
			break;
	}

	return Ok(ret);
}

Result read_exact_impl(Object *self, char *buf, u64 len) {
	Result r = myread(self, buf, len);
	u64 rlen = *(u64 *)unwrap(&r);
	if (rlen != len) {
		Error err = ERROR(
		    UNEXPECTED_EOF,
		    "failed to fill the whole buffer (%llu of %llu avaialable)",
		    rlen, len);
		return Err(err);
	}

	return Ok(UNIT);
}

Result write_all_impl(Object *self, char *buf, u64 len){todo()}

Result write_fmt_impl(Object *self, char *fmt, ...) {
	todo()
}
