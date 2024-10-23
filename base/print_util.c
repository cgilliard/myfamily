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

#include <base/deps.h>
#include <base/fam_err.h>
#include <base/macro_util.h>
#include <base/misc.h>
#include <base/print_util.h>
#include <stdarg.h>

#define BUF_LEN 64

void exit(int status);

i32 write_loop(const Stream *strm, const u8 *buf, u64 len) {
	while (len > 0) {
		i64 w = write(strm->handle, buf, len);
		if (w < 0) {
			SetErr(IO);
			return w;
		}

		len -= w;
	}
	return 0;
}

i32 print_impl(const Stream *strm, u8 *s, i32 capacity, bool nl, bool do_exit, i32 code,
			   const u8 *fmt, ...) {
	i32 ret = 0;
	va_list args;
	va_start(args, fmt);

	loop {
		const u8 *next = mystrstr(fmt, "{}");

		if (next == NULL) {
			if (write_loop(strm, fmt, mystrlen(fmt))) {
				ret = -1;
				break;
			}
			if (nl)
				if (write_loop(strm, "\n", 1)) {
					ret = -1;
					break;
				}
			break;
		} else {
			u64 diff = next - fmt;
			u8 buf[1 + diff];
			mystrcpy(buf, fmt, 1 + diff);
			if (write_loop(strm, buf, diff)) {
				ret = -1;
				break;
			}
		}

		PrintPair arg = va_arg(args, PrintPair);

		if (arg.type == PrintTypeTerm) {
			if (next) {
				if (write_loop(strm, next, mystrlen(next))) {
					ret = -1;
					break;
				}
				if (nl)
					if (write_loop(strm, "\n", 1)) {
						ret = -1;
						break;
					}
			}
			break;
		} else if (arg.type == PrintTypeU64) {
			u64 value;
			mymemcpy(&value, arg.buf, sizeof(u64));
			u8 buf[BUF_LEN];
			citoau64(value, buf, 10);
			if (write_loop(strm, buf, mystrlen(buf))) {
				ret = -1;
				break;
			}
		} else if (arg.type == PrintTypeI32) {
			i32 value;
			mymemcpy(&value, arg.buf, sizeof(i32));
			u8 buf[BUF_LEN];
			citoai64(value, buf, 10);
			if (write_loop(strm, buf, mystrlen(buf))) {
				ret = -1;
				break;
			}
		} else if (arg.type == PrintTypeI16) {
			i16 value;
			mymemcpy(&value, arg.buf, sizeof(i16));
			u8 buf[BUF_LEN];
			citoai64(value, buf, 10);
			if (write_loop(strm, buf, mystrlen(buf))) {
				ret = -1;
				break;
			}
		} else if (arg.type == PrintTypeString) {
			if (write_loop(strm, arg.data, mystrlen(arg.data))) {
				ret = -1;
				break;
			}
		}
		fmt = next + 2;
	}

	va_end(args);

	if (do_exit) {
		exit(code);
	}

	return ret;
}
