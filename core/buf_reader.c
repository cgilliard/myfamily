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
	printf("br cleanup\n");
	FilePtr file = GET(BufReader, ptr, f);
	cleanup(&file);
}

Result BufReader_read_fixed_bytes(BufReader *ptr, char *buf, u64 len) {
	return Ok(_());
}

Result BufReader_open_impl(int n, va_list ptr, bool is_rc) {
	FilePtr file;
	for (int i = 0; i < n; i++) {
		printf("i=%i\n", i);
		BufReaderOptionPtr next;
		RcPtr rc;
		if (!is_rc) {
			printf("x\n");
			next = va_arg(ptr, BufReaderOption);
		} else {
			printf("ok\n");
			rc = va_arg(ptr, Rc);
			printf("next\n");
			void *vptr = unwrap(&rc);
			printf("1\n");
			memcpy(&next, vptr, size(vptr));
			printf("2\n");
		}

		MATCH(next, VARIANT(BUF_READER_FILE, {
			      printf("found a file\n");
			      FilePtr fptr = TRY_ENUM_VALUE(file, File, next);
			      memcpy(&file, &fptr, size(&fptr));
		      }));
	}
	va_end(ptr);
	printf("exit\n");

	BufReaderPtr ret = BUILD(BufReader, file);

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
