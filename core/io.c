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

#include <core/ekinds.h>
#include <core/io.h>
#include <core/string.h>
#include <core/string_builder.h>
#include <core/unit.h>

#define RTS_BUF_SIZE 1024

Result write_all(void *obj, char *buf, u64 limit) {
	ResultPtr (*do_write_all)(Object *obj, char *buf, u64 limit) =
	    find_fn((Object *)obj, "write_all");
	if (do_write_all == NULL)
		panic("write_all not implemented for this type [%s]",
		      CLASS_NAME(obj));
	return do_write_all(obj, buf, limit);
}

Result myflush(void *obj) {
	ResultPtr (*do_flush)(Object *obj) = find_fn((Object *)obj, "flush");
	if (do_flush == NULL)
		panic("flush not implemented for this type [%s]",
		      CLASS_NAME(obj));
	return do_flush(obj);
}

Result myread(void *obj, char *buf, u64 limit) {
	ResultPtr (*do_read)(Object *obj, char *buf, u64 limit) =
	    find_fn((Object *)obj, "read");
	if (do_read == NULL)
		panic("read not implemented for this type [%s]",
		      CLASS_NAME(obj));
	return do_read(obj, buf, limit);
}

Result mywrite(void *obj, char *buf, u64 limit) {
	ResultPtr (*do_write)(Object *obj, char *buf, u64 limit) =
	    find_fn((Object *)obj, "write");
	if (do_write == NULL)
		panic("write not implemented for this type [%s]",
		      CLASS_NAME(obj));
	return do_write(obj, buf, limit);
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
	StringBuilder ret = STRING_BUILDER("", ret);
	while (true) {
		Result r = myread(self, buf, RTS_BUF_SIZE);
		u64 rlen = TRY(r, rlen);
		buf[rlen] = 0;

		if (rlen > 0) {
			String next = STRING(buf);
			Result r2 = append(&ret, &next);
			TRYU(r2);
		}

		if (rlen < RTS_BUF_SIZE)
			break;
	}

	return StringBuilder_to_string(&ret);
}

Result read_exact_impl(Object *self, char *buf, u64 len) {
	Result r = myread(self, buf, len);
	u64 rlen = *(u64 *)unwrap(&r);
	if (rlen != len) {
		Error err = ERR(
		    UNEXPECTED_EOF,
		    "failed to fill the whole buffer (%llu of %llu avaialable)",
		    rlen, len);
		return Err(err);
	}

	return Ok(_());
}

Result write_all_impl(Object *self, char *buf, u64 len) {
	u64 wlen_sum = 0;
	u64 rem = len;
	while (true) {
		if (wlen_sum == len)
			break;
		Result r = mywrite(self, buf + wlen_sum, rem);
		u64 wlen = TRY(r, wlen);
		wlen_sum += wlen;
		rem -= wlen;
	}
	return Ok(_());
}

Result write_fmt_impl(Object *self, char *fmt, ...) { return Ok(_()); }

Result read_fixed_bytes(void *obj, char *buf, u64 limit) {
	ResultPtr (*fn_ptr)(Object *obj, char *buf, u64 limit) =
	    find_fn((Object *)obj, "read_fixed_bytes");
	if (fn_ptr == NULL)
		panic("read_fixed_bytes not implemented for this type [%s]",
		      CLASS_NAME(obj));
	return fn_ptr(obj, buf, limit);
}

Result read_u8_impl(void *obj) { todo(); }
Result read_u16_impl(void *obj) { todo(); }
Result read_u32_impl(void *obj) { todo(); }
Result read_u64_impl(void *obj) { todo(); }
Result read_u128_impl(void *obj) { todo(); }
Result read_i8_impl(void *obj) { todo(); }
Result read_i16_impl(void *obj) { todo(); }
Result read_i32_impl(void *obj) { todo(); }
Result read_i64_impl(void *obj) { todo(); }
Result read_i128_impl(void *obj) { todo(); }
Result read_bool_impl(void *obj) { todo(); }
Result expect_u8_impl(void *obj, u8 value) { todo(); }
Result read_empty_bytes_impl(void *obj, u64 count) { todo(); }
