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
#include <core/unit.h>

GETTER(BufReader, f)

void BufReader_cleanup(BufReader *ptr) {
	Rc file = GET(BufReader, ptr, f);
	cleanup(&file);
}

Result BufReader_read_fixed_bytes(BufReader *ptr, char *buf, u64 len) {
	return Ok(_());
}

Result BufReader_open_impl(int n, va_list ptr, bool is_rc) {
	Rc frc;
	NO_CLEANUP(frc);
	for (int i = 0; i < n; i++) {
		BufReaderOptionPtr next;
		Rc rc;
		if (!is_rc) {
			next = va_arg(ptr, BufReaderOption);
		} else {
			rc = va_arg(ptr, Rc);
			void *vptr = unwrap(&rc);
			memcpy(&next, vptr, size(vptr));
		}

		MATCH(next,
		      VARIANT(BUF_READER_FILE, { myclone(&frc, next.value); }));
	}
	va_end(ptr);

	BufReaderPtr ret = BUILD(BufReader, frc);

	return Ok(ret);
}

Result BufReader_open(int n, ...) {
	va_list ptr;
	va_start(ptr, n);
	return BufReader_open_impl(n, ptr, false);
}

Result BufReader_open_rc(int n, ...) {
	va_list ptr;
	va_start(ptr, n);
	return BufReader_open_impl(n, ptr, true);
}