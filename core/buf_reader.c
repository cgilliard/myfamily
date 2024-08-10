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

void BufReader_cleanup(BufReader *ptr) {}
Result BufReader_read_fixed_bytes(BufReader *ptr, char *buffer, u64 len) {
	todo();
}
Result BufReader_open_impl(int n, va_list ptr) { todo(); }
void BufReader_consume_buf(BufReader *ptr, u64 amt) {}
Result BufReader_fill_buf(BufReader *ptr) { todo(); }
Result BufReader_read(BufReader *ptr, char *buffer, u64 len) { todo(); }

Result BufReader_open(int n, ...) { todo(); }

Result read_line_impl(Object *ptr, String *dst) { todo(); }

Result read_until_impl(Object *ptr, Slice dst, u8 b) { todo(); }

Result lines_impl(Object *ptr) { todo(); }

void BufReaderLineIterator_cleanup(BufReaderLineIterator *ptr) {}

Result BufReaderLineIterator_next(BufReaderLineIterator *ptr) { todo(); }

Rc BufReader_into_iter(BufReader *ptr) {
	/*
	BufReaderLineIteratorPtr *ret = mymalloc(sizeof(BufReaderLineIterator));
	BUILDPTR(ret, BufReaderLineIterator);
	SET(BufReaderLineIterator, ret, reader, (Object *)ptr);
	RcPtr rc = RC(ret);
	return rc;
	*/
	int x = 0;
	Rc h = HEAPIFY(x);
	return h;
}
