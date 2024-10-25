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

#include <base/fam_err.h>
#include <base/macro_util.h>
#include <base/os.h>
#include <base/print_util.h>
#include <base/string.h>

#define BUF_LEN 64

// get the va functionality (with GCC/Clang use the builtin version, otherwise use stdarg)
#if defined(__GNUC__) || defined(__clang__)
typedef __builtin_va_list va_list;
#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#else // __GNUC__ or __clang__
#include <stdarg.h>
#endif // __GNUC__ or __clang__

i32 write_loop(const Stream *strm, u8 *s, i32 *cur, i32 limit, const u8 *buf, u64 len) {
	if (s) {
		if (strm->handle == -1) {
			// length only
			*cur += len;
		} else if (*cur < limit) {
			u8 *res;
			if (*cur == 0) {
				res = mystrcpy(s, buf, limit);
				if (res == NULL)
					return -1;
			} else {
				res = mystrcat(s, buf, limit);
				if (res == NULL)
					return -1;
			}
			*cur = res - s;
		}
	} else
		while (len > 0) {
			i64 w = strm_write(strm, buf, len);
			if (w < 0) {
				SetErr(IO);
				return w;
			}

			len -= w;
		}
	return 0;
}

i32 print_impl(const Stream *strm, u8 *s, i32 capacity, bool nl, bool do_exit, i32 code,
			   const u8 *prefix, const u8 *fmt, ...) {
	int ret = 0;
	if (capacity < 0 && s) {
		SetErr(IllegalArgument);
		ret = -1;
	}
	va_list args;
	va_start(args, fmt);
	i32 max = capacity;
	capacity = 0;

	if (prefix && ret != -1) {
		if (write_loop(strm, s, &capacity, max, prefix, mystrlen(prefix)))
			ret = -1;
	}

	while (ret != -1) {
		bool is_hex = false;
		const u8 *next = mystrstr(fmt, "{}");
		const u8 *next_hex = mystrstr(fmt, "{hex}");
		if ((next_hex && !next) || (next_hex && next_hex < next)) {
			next = next_hex;
			is_hex = true;
		}

		if (next == NULL) {
			if (write_loop(strm, s, &capacity, max, fmt, mystrlen(fmt))) {
				ret = -1;
				break;
			}
			if (nl)
				if (write_loop(strm, s, &capacity, max, "\n", 1)) {
					ret = -1;
					break;
				}
			break;
		} else {
			u64 diff = next - fmt;
			u8 buf[1 + diff];
			mystrcpy(buf, fmt, 1 + diff);
			if (write_loop(strm, s, &capacity, max, buf, diff)) {
				ret = -1;
				break;
			}
		}

		PrintPair arg = va_arg(args, PrintPair);

		if (arg.type == PrintTypeTerm) {
			if (next) {
				if (write_loop(strm, s, &capacity, max, next, mystrlen(next))) {
					ret = -1;
					break;
				}
				if (nl)
					if (write_loop(strm, s, &capacity, max, "\n", 1)) {
						ret = -1;
						break;
					}
			}
			break;
		} else if (arg.type == PrintTypeI64) {
			i64 value;
			mymemcpy(&value, arg.buf, sizeof(i64));
			u8 buf[BUF_LEN];
			if (is_hex)
				citoai64(value, buf, 16);
			else
				citoai64(value, buf, 10);
			if (write_loop(strm, s, &capacity, max, buf, mystrlen(buf))) {
				ret = -1;
				break;
			}
		} else if (arg.type == PrintTypeU64) {
			u64 value;
			mymemcpy(&value, arg.buf, sizeof(u64));
			u8 buf[BUF_LEN];
			if (is_hex)
				citoau64(value, buf, 16);
			else
				citoau64(value, buf, 10);
			if (write_loop(strm, s, &capacity, max, buf, mystrlen(buf))) {
				ret = -1;
				break;
			}
		} else if (arg.type == PrintTypeI32) {
			i32 value;
			mymemcpy(&value, arg.buf, sizeof(i32));
			u8 buf[BUF_LEN];
			citoai64(value, buf, 10);
			if (write_loop(strm, s, &capacity, max, buf, mystrlen(buf))) {
				ret = -1;
				break;
			}
		} else if (arg.type == PrintTypeI16) {
			i16 value;
			mymemcpy(&value, arg.buf, sizeof(i16));
			u8 buf[BUF_LEN];
			citoai64(value, buf, 10);
			if (write_loop(strm, s, &capacity, max, buf, mystrlen(buf))) {
				ret = -1;
				break;
			}
		} else if (arg.type == PrintTypeString) {
			if (write_loop(strm, s, &capacity, max, arg.data, mystrlen(arg.data))) {
				ret = -1;
				break;
			}
		}
		if (is_hex)
			fmt = next + 5;
		else
			fmt = next + 2;
	}

	va_end(args);

	if (do_exit) {
		Backtrace bt;
		backtrace_generate(&bt);
		backtrace_print(&bt);

		exit(code);
	}

	if (ret == -1)
		return ret;
	return capacity;
}
