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

#include <base/object.h>
#include <base/print_util.h>
#include <base/sys.h>
#include <base/util.h>

#define va_end(...) __builtin_va_end(__VA_ARGS__)
#define va_start(...) __builtin_va_start(__VA_ARGS__)
#define va_arg(...) __builtin_va_arg(__VA_ARGS__)

int write_data(const char *s, long long len) {
	return write(1, s, len);
}

long long print_impl(Channel *channel, char *buffer, long long capacity,
					 int newline, int exit, const char *prefix, const char *fmt,
					 ...) {
	long long fmt_len = cstring_len(fmt);
	va_list args;
	va_start(args, fmt);
	Printable p;
	long long itt = 0;

	if (prefix) write_data(prefix, cstring_len(prefix));

	do {
		const char *s = cstring_strstr(fmt + itt, "{}");
		if (s) {
			long long diff = 2 + (s - fmt);
			write_data(fmt + itt, (s - fmt) - itt);

			itt = diff;
		} else {
			write_data(fmt + itt, fmt_len - itt);
			itt = fmt_len;
			break;
		}
		p = va_arg(args, Printable);
		if (p.type == PrintTypeInt) {
			unsigned long long len =
				cstring_itoai64(p.value.int_value, 0, 10, 0);
			if (len > 0) {
				char buf[len + 1];
				cstring_itoai64(p.value.int_value, buf, 10, len);
				write_data(buf, len);
			}
		} else if (p.type == PrintTypeUInt) {
			unsigned long long len =
				cstring_itoau64(p.value.int_value, 0, 10, 0);
			if (len > 0) {
				char buf[len + 1];
				cstring_itoau64(p.value.int_value, buf, 10, len);
				write_data(buf, len);
			}
		} else if (p.type == PrintTypeFloat) {
			unsigned long long len = snprintf(0, 0, "%f", p.value.float_value);
			if (len > 0) {
				char buf[len + 1];
				snprintf(buf, len + 1, "%f", p.value.float_value);
				write_data(buf, len);
			}
		} else if (p.type == PrintTypeString) {
			if (p.value.string_value != 0) {
				unsigned long long len = cstring_len(p.value.string_value);
				if (len > 0) write_data(p.value.string_value, len);
			}
		} else if (p.type == PrintTypeObject) {
			ObjectType type = object_type(&p.value.object_value);
			if (type == Int) {
				long long v = $int(p.value.object_value);
				unsigned long long len = cstring_itoai64(v, 0, 10, 0);
				if (len) {
					char buf[len + 1];
					cstring_itoai64(v, buf, 10, len);
					write_data(buf, len);
				}
			} else if (type == Float) {
				double v = $float(p.value.object_value);
				int len = snprintf(0, 0, "%f", v);
				if (len) {
					char buf[len + 1];
					snprintf(buf, len + 1, "%f", v);
					write_data(buf, len);
				}
			} else if (type == UInt) {
				unsigned long long v = $uint(p.value.object_value);
				unsigned long long len = cstring_itoau64(v, 0, 10, 0);
				if (len) {
					char buf[len + 1];
					cstring_itoau64(v, buf, 10, len);
					write_data(buf, len);
				}
			} else if (type == Err) {
				const char *err_text = $err(p.value.object_value);
				unsigned long long len = cstring_len(err_text);
				if (len > 0) write_data(err_text, len);
			} else {
			}  // TODO: handle other types
		}

	} while (p.type != PrintTypeTerm);
	va_end(args);

	if (itt < fmt_len) {
		write_data(fmt + itt, fmt_len - itt);
	}
	if (newline) write_data("\n", 1);
	if (exit) _exit(-1);
	return 0;
}
