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

GETTER(BufReader, f)
GETTER(BufReader, buf)
GETTER(BufReader, offset)
SETTER(BufReader, offset)
GETTER(BufReaderLineIterator, reader)
SETTER(BufReaderLineIterator, reader)

Result lines(void *obj) {
	ResultPtr (*do_lines)(Object *obj) = find_fn((Object *)obj, "lines");
	if (do_lines == NULL)
		panic("lines not implemented for this type [%s]",
		      CLASS_NAME(obj));
	return do_lines(obj);
}

Result read_line(void *obj, String *s) {
	ResultPtr (*do_read_line)(Object *obj, String *s) =
	    find_fn((Object *)obj, "read_line");
	if (do_read_line == NULL)
		panic("read_line not implemented for this type [%s]",
		      CLASS_NAME(obj));
	return do_read_line(obj, s);
}

Result read_until(void *obj, Slice dst, u8 b) {
	ResultPtr (*do_read_until)(Object *obj, Slice dst, u8 b) =
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

	Slab slab = GET(BufReader, ptr, buf);
	if (slab.data) {
		myfree(&slab);
		slab.data = NULL;
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
	return Ok(UNIT);
}
Result BufReader_open_impl(int n, va_list ptr) {
	Rc frc;
	NO_CLEANUP(frc);
	u64 capacity = 8196;
	bool found_readable = false;
	for (int i = 0; i < n; i++) {
		BufReaderOption next;
		Rc rc = va_arg(ptr, Rc);
		void *vptr = unwrap(&rc);
		memcpy(&next, vptr, mysize(vptr));

		MATCH(next, VARIANT(BUF_READER_FILE, {
			      found_readable = true;
			      Rc readable = ENUM_VALUE(readable, Rc, next);
			      myclone(&frc, &readable);
		      }) VARIANT(BUF_READER_CAPACITY, {
			      capacity = ENUM_VALUE(capacity, u64, next);
		      }));
	}
	va_end(ptr);

	if (!found_readable) {
		Error e = ERR(ILLEGAL_ARGUMENT,
			      "BufReaderReadable must be specified");
		return Err(e);
	}

	Slab buf = ALLOCATE_SLAB(sizeof(char) * capacity);
	BufReader ret = BUILD(BufReader, frc, buf, 0);

	return Ok(ret);
}
void BufReader_consume_buf(BufReader *ptr, u64 amt) {
	u64 offset = GET(BufReader, ptr, offset);
	Slab buf = GET(BufReader, ptr, buf);

	if (amt > offset)
		amt = offset;

	if (amt > 0) {
		memmove(buf.data, buf.data + amt, offset - amt);
		SET(BufReader, ptr, offset, offset - amt);
	}
}
Result BufReader_fill_buf(BufReader *ptr) {
	Slab buf = GET(BufReader, ptr, buf);
	u64 offset = GET(BufReader, ptr, offset);
	u64 capacity = buf.len;

	u64 rem = capacity - offset;

	Rc readablerc = GET(BufReader, ptr, f);
	Object *readable = unwrap(&readablerc);

	if (offset == 0) {
		Result r = myread(readable, buf.data + offset, rem);
		u64 rlen = TRY(r, rlen);
		offset += rlen;
		SET(BufReader, ptr, offset, offset);
	}

	Slice ret = SLICE(buf.data, offset);
	return Ok(ret);
}
Result BufReader_read(BufReader *ptr, char *buffer, u64 len) {
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

	return Ok(UNIT);
}

Result BufReader_open(int n, ...) {
	va_list ptr;
	va_start(ptr, n);
	return BufReader_open_impl(n, ptr);
}

Result read_line_impl(Object *ptr, String *dst) {
	char buf[1024];
	Slice s = SLICE(buf, 1024);
	bool nlfound = false;

	while (true) {
		Result r = read_until_impl(ptr, s, '\n');
		u64 rlen = TRY(r, rlen);

		if (rlen > 0) {
			char *slice_data = GET(Slice, &s, ref);
			if (slice_data[rlen - 1] == '\n') {
				slice_data[rlen - 1] = 0;
				nlfound = true;
			}
			Result r2 = String_from_slice(&s, rlen);
			String nstr = TRY(r2, nstr);
			Result r3 = append(dst, &nstr);
			TRYU(r3);
		}

		if (rlen < 1024 || nlfound)
			break;
	}

	return Ok(UNIT);
}

Result read_until_impl(Object *ptr, Slice dst, u8 b) {
	int counter = 0;
	u64 rlen = 0;
	u64 dst_len = len(&dst);
	while (true) {
		u64 offset = rlen;
		Result r = fill_buf(ptr);
		Slice slice = TRY(r, slice);
		char *slice_data = GET(Slice, &slice, ref);
		u64 slice_len = len(&slice);
		if (slice_len == 0)
			break;

		u64 to_consume = slice_len;
		bool found_mark = false;

		for (u64 i = 0; i < slice_len; i++) {
			if (b == slice_data[i]) {
				to_consume = i + 1;
				found_mark = true;
				break;
			}
		}

		if (to_consume + rlen > dst_len) {
			to_consume = dst_len - rlen;
			rlen = dst_len;
		} else {
			rlen += to_consume;
		}

		memcpy(GET(Slice, &dst, ref) + offset, slice_data, to_consume);
		consume_buf(ptr, to_consume);

		if (rlen == dst_len || found_mark)
			break;

		if (++counter > 30)
			break;
	}

	return Ok(rlen);
}

Result lines_impl(Object *ptr) {
	BufReaderLineIterator ret = BUILD(BufReaderLineIterator, ptr);
	return Ok(ret);
}

void BufReaderLineIterator_cleanup(BufReaderLineIterator *ptr) {}

Result BufReaderLineIterator_next(BufReaderLineIterator *ptr) {
	char buf[1024];
	Slice slice = SLICE(buf, 1024);
	Object *obj = GET(BufReaderLineIterator, ptr, reader);
	Result r = read_until(obj, slice, '\n');
	u64 rlen = TRY(r, rlen);

	char *slice_chars = GET(Slice, &slice, ref);
	if (rlen > 0)
		slice_chars[rlen - 1] = 0;

	String ret = STRING("");
	Result slice_str_res = String_from_slice(&slice, rlen);
	String slice_str = TRY(slice_str_res, slice_str);
	Result ar = append(&ret, &slice_str);
	TRYU(ar);
	if (len(&ret) > 0) {
		Option retopt = Some(ret);
		return Ok(retopt);
	} else
		return Ok(None);
}

Rc BufReader_into_iter(BufReader *ptr) {
	Slab slab = ALLOCATE_SLABP(sizeof(BufReaderLineIterator));
	Object *obj = slab.data;
	BUILDPTR(obj, BufReaderLineIterator);
	SET(BufReaderLineIterator, slab.data, reader, (Object *)ptr);
	RcPtr rc = RC(slab);
	return rc;
}
