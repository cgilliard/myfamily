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

#include <core/buf_reader.h>
#include <core/slice.h>
#include <core/unit.h>

GETTER(BufReader, f)
GETTER(BufReader, buf)
SETTER(BufReader, buf)
GETTER(BufReader, offset)
SETTER(BufReader, offset)
GETTER(BufReader, capacity)
SETTER(BufReader, capacity)

Result read_until(void *obj, String *dst, u8 b) {
	ResultPtr (*do_read_until)(Object *obj, String *dst, u8 b) =
	    find_fn((Object *)obj, "read_until");
	if (do_read_until == NULL)
		panic("read_until not implemented for this type [%s]",
		      CLASS_NAME(obj));
	return do_read_until(obj, dst, b);
}

void consume_buf(void *obj, u64 amt) {
	void (*do_consume_buf)(Object *obj, u64 amt) =
	    find_fn((Object *)obj, "consume_buf");
	if (do_consume_buf == NULL)
		panic("consume_buf not implemented for this type [%s]",
		      CLASS_NAME(obj));

	return do_consume_buf(obj, amt);
}

Result fill_buf(void *obj) {
	ResultPtr (*do_fill_buf)(Object *obj) =
	    find_fn((Object *)obj, "fill_buf");
	if (do_fill_buf == NULL)
		panic("fill_buf not implemented for this type [%s]",
		      CLASS_NAME(obj));

	return do_fill_buf(obj);
}

void BufReader_cleanup(BufReader *ptr) {
	RcPtr readable = GET(BufReader, ptr, f);
	cleanup(&readable);

	char *buf = GET(BufReader, ptr, buf);
	if (buf) {
		myfree(buf);
		SET(BufReader, ptr, buf, NULL);
	}
}

Result BufReader_read_fixed_bytes(BufReader *ptr, char *buffer, u64 len) {
	u64 wlen_sum = 0;
	Rc readablerc = GET(BufReader, ptr, f);
	Object *readable = unwrap(&readablerc);
	u64 rem = len;
	u64 offset = 0;

	while (rem > 0) {
		Result r = myread(readable, buffer + offset, rem);
		u64 wlen = TRY(r, wlen);

		if (wlen == 0) {
			Error e = ERR(IO_ERROR,
				      "Could not fill whole buffer. Found "
				      "%llu. Needed %llu",
				      wlen_sum, len);
			return Err(e);
		}

		wlen_sum += wlen;
		rem -= wlen;
	}
	return Ok(_());
}

Result BufReader_open_impl(int n, va_list ptr) {
	Rc frc;
	NO_CLEANUP(frc);
	u64 capacity = 8196;
	bool found_readable = false;
	for (int i = 0; i < n; i++) {
		BufReaderOptionPtr next;
		Rc rc = va_arg(ptr, Rc);
		void *vptr = unwrap(&rc);
		memcpy(&next, vptr, size(vptr));

		MATCH(next, VARIANT(BUF_READER_FILE, {
			      found_readable = true;
			      myclone(&frc, next.value);
		      }) VARIANT(BUF_READER_CAPACITY, {
			      capacity = TRY_ENUM_VALUE(capacity, u64, next);
		      }));
	}
	va_end(ptr);

	if (!found_readable) {
		Error e = ERR(ILLEGAL_ARGUMENT,
			      "BufReaderReadable must be specified");
		return Err(e);
	}

	char *buf = mymalloc(sizeof(char) * capacity);

	BufReader ret = BUILD(BufReader, frc, capacity, buf, 0);
	return Ok(ret);
}

void BufReader_consume_buf(BufReader *ptr, u64 amt) {
	u64 offset = GET(BufReader, ptr, offset);
	char *buf = GET(BufReader, ptr, buf);

	if (amt > offset)
		amt = offset;

	if (amt > 0) {
		memmove(buf, buf + amt, amt);
		SET(BufReader, ptr, offset, offset - amt);
	}
}

Result BufReader_fill_buf(BufReader *ptr) {
	char *buf = GET(BufReader, ptr, buf);
	u64 offset = GET(BufReader, ptr, offset);
	u64 capacity = GET(BufReader, ptr, capacity);

	u64 rem = capacity - offset;

	Rc readablerc = GET(BufReader, ptr, f);
	Object *readable = unwrap(&readablerc);

	if (offset == 0) {
		Result r = myread(readable, buf + offset, rem);
		u64 rlen = TRY(r, rlen);
		offset += rlen;
		SET(BufReader, ptr, offset, offset);
	}

	Slice ret = SLICE(buf, offset);
	return Ok(ret);
}

Result BufReader_open(int n, ...) {
	va_list ptr;
	va_start(ptr, n);
	return BufReader_open_impl(n, ptr);
}

Result read_line_impl(Object *ptr, String *dst) { todo(); }
Result read_until_impl(Object *ptr, String *dst, u8 b) {
	int count = 0;
	while (true) {
		Result r = fill_buf(ptr);
		Slice slice = TRY(r, slice);
		u64 slice_len = len(&slice);
		printf("slice_len=%llu\n", slice_len);
		if (slice_len == 0)
			break;

		char buffer[slice_len + 1];
		// memcpy(buffer, slice_len);
		buffer[slice_len] = 0;
		consume_buf(ptr, slice_len);
	}

	return Ok(_());
}
Object *lines_impl(Object *ptr) { return NULL; }
