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
#include <inttypes.h>

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
#include <stdio.h>

i32 write_loop(const Stream *strm, u8 *s, i32 *cur, i32 limit, const u8 *buf, u64 len) {
	if (s) {
		if (strm->handle == -1) {
			// length only
			*cur += len;
		} else if (*cur < limit) {
			u8 *res;
			if (*cur == 0) {
				res = strncpy(s, buf, len);
				if (res == NULL)
					return -1;
			} else {
				res = strncat(s, buf, len);
				if (res == NULL)
					return -1;
			}

			i32 l = strlen(buf);
			*cur = l;
		}
	} else {
		while (len > 0) {
			i64 w = strm_write(strm, buf, len);
			if (w < 0) {
				SetErr(IO);
				return w;
			}

			len -= w;
		}
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
	if (s) {
		memset(s, '\0', capacity);
	}
	va_list args;
	va_start(args, fmt);
	i32 max = capacity;
	capacity = 0;

	if (prefix && ret != -1) {
		if (write_loop(strm, s, &capacity, max, prefix, strlen(prefix)))
			ret = -1;
	}

	while (ret != -1) {
		bool is_hex = false;
		const u8 *next = strstr(fmt, "{}");
		const u8 *next_hex = strstr(fmt, "{hex}");
		if ((next_hex && !next) || (next_hex && next_hex < next)) {
			next = next_hex;
			is_hex = true;
		}

		if (next == NULL) {
			if (write_loop(strm, s, &capacity, max, fmt, strlen(fmt))) {
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
			memset(buf, '\0', 1 + diff);
			strncpy(buf, fmt, diff);
			if (write_loop(strm, s, &capacity, max, buf, diff)) {
				ret = -1;
				break;
			}
		}

		PrintPair arg = va_arg(args, PrintPair);

		if (arg.type == PrintTypeTerm) {
			if (next) {
				if (write_loop(strm, s, &capacity, max, next, strlen(next))) {
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
		} else if (arg.type == PrintTypeBool) {
			bool value;
			memcpy(&value, arg.buf, sizeof(bool));
			u8 buf[BUF_LEN];
			if (value)
				strncpy(buf, "true", 5);
			else
				strncpy(buf, "false", 6);
			if (write_loop(strm, s, &capacity, max, buf, strlen(buf))) {
				ret = -1;
				break;
			}
		} else if (arg.type == PrintTypeF64) {
			f64 value;
			memcpy(&value, arg.buf, sizeof(f64));
			u8 buf[BUF_LEN];
			strncpy(buf, "", 1);
			snprintf(buf, BUF_LEN - 1, "%lf", value);
			if (write_loop(strm, s, &capacity, max, buf, strlen(buf))) {
				ret = -1;
				break;
			}
		} else if (arg.type == PrintTypeF32) {
			f32 value;
			memcpy(&value, arg.buf, sizeof(f32));
			u8 buf[BUF_LEN];
			strncpy(buf, "", 1);
			snprintf(buf, BUF_LEN - 1, "%f", value);
			if (write_loop(strm, s, &capacity, max, buf, strlen(buf))) {
				ret = -1;
				break;
			}
		} else if (arg.type == PrintTypeI64) {
			i64 value;
			memcpy(&value, arg.buf, sizeof(i64));
			u8 buf[BUF_LEN];
			strncpy(buf, "", 1);
			if (is_hex)
				snprintf(buf, BUF_LEN - 1, "%llX", value);
			else
				snprintf(buf, BUF_LEN - 1, "%lli", value);
			if (write_loop(strm, s, &capacity, max, buf, strlen(buf))) {
				ret = -1;
				break;
			}
		} else if (arg.type == PrintTypeU64) {
			u64 value;
			memcpy(&value, arg.buf, sizeof(u64));
			u8 buf[BUF_LEN];
			strncpy(buf, "", 1);
			if (is_hex)
				snprintf(buf, BUF_LEN - 1, "%llx", value);
			else
				snprintf(buf, BUF_LEN - 1, "%llu", value);

			if (write_loop(strm, s, &capacity, max, buf, strlen(buf))) {
				ret = -1;
				break;
			}
		} else if (arg.type == PrintTypeI32) {
			i32 value;
			memcpy(&value, arg.buf, sizeof(i32));
			u8 buf[BUF_LEN];
			strncpy(buf, "", 1);
			snprintf(buf, BUF_LEN - 1, "%i", value);
			if (write_loop(strm, s, &capacity, max, buf, strlen(buf))) {
				ret = -1;
				break;
			}
		} else if (arg.type == PrintTypeU32) {
			u32 value;
			memcpy(&value, arg.buf, sizeof(u32));
			u8 buf[BUF_LEN];
			strncpy(buf, "", 1);
			snprintf(buf, BUF_LEN - 1, "%u", value);
			if (write_loop(strm, s, &capacity, max, buf, strlen(buf))) {
				ret = -1;
				break;
			}
		} else if (arg.type == PrintTypeI16) {
			i16 value;
			memcpy(&value, arg.buf, sizeof(i16));
			u8 buf[BUF_LEN];
			strncpy(buf, "", 1);
			snprintf(buf, BUF_LEN - 1, "%i", value);
			if (write_loop(strm, s, &capacity, max, buf, strlen(buf))) {
				ret = -1;
				break;
			}
		} else if (arg.type == PrintTypeU16) {
			u16 value;
			memcpy(&value, arg.buf, sizeof(u16));
			u8 buf[BUF_LEN];
			strncpy(buf, "", 1);
			snprintf(buf, BUF_LEN - 1, "%u", value);
			if (write_loop(strm, s, &capacity, max, buf, strlen(buf))) {
				ret = -1;
				break;
			}
		} else if (arg.type == PrintTypeI8) {
			i8 value;
			memcpy(&value, arg.buf, sizeof(i8));
			u8 buf[BUF_LEN];
			strncpy(buf, "", 1);
			snprintf(buf, BUF_LEN - 1, "%i", value);
			if (write_loop(strm, s, &capacity, max, buf, strlen(buf))) {
				ret = -1;
				break;
			}
		} else if (arg.type == PrintTypeU8) {
			u8 value;
			memcpy(&value, arg.buf, sizeof(u8));
			u8 buf[BUF_LEN];
			strncpy(buf, "", 1);
			snprintf(buf, BUF_LEN - 1, "%u", value);
			if (write_loop(strm, s, &capacity, max, buf, strlen(buf))) {
				ret = -1;
				break;
			}
		} else if (arg.type == PrintTypeString) {
			if (write_loop(strm, s, &capacity, max, arg.data, strlen(arg.data))) {
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
