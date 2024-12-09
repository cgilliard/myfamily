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

int write_data(const char *s, i64 len) {
	return write(1, s, len);
}

i64 sprint_int_impl(char *buffer, i64 capacity, i64 itt, i64 buf_itt,
					i64 value) {
	return cstring_itoai64(value, 0, 10, 0);
}

i64 sprint_uint_impl(char *buffer, i64 capacity, i64 itt, i64 buf_itt,
					 i64 value) {
	return cstring_itoau64(value, 0, 10, 0);
}

i64 sprint_float_impl(char *buffer, i64 capacity, i64 itt, i64 buf_itt,
					  double value) {
	return snprintf(NULL, 0, "%f", value);
}

i64 sprint_string_impl(char *buffer, i64 capacity, i64 itt, i64 buf_itt,
					   char *value) {
	return cstring_len(value);
}

i64 sprint_object_impl(char *buffer, i64 capacity, i64 itt, i64 buf_itt,
					   Object value) {
	return match(
		value,
		(Int, sprint_int_impl(buffer, capacity, itt, buf_itt, $int(value))),
		(UInt, sprint_uint_impl(buffer, capacity, itt, buf_itt, $uint(value))),
		(Float,
		 sprint_float_impl(buffer, capacity, itt, buf_itt, $float(value))),
		(-1));
}

i64 sprint_impl(char *buffer, i64 capacity, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	i64 itt = 0, buf_itt = 0, fmt_len = cstring_len(fmt), vlen;
	Printable p;
	while (itt < fmt_len) {
		const char *s = cstring_strstr(fmt + itt, "{}");
		if (s) {
			i64 diff = 2 + (s - fmt);
			buf_itt += (diff - 2) - itt;
			itt = diff;
		} else {
			buf_itt += fmt_len - itt;
			itt = fmt_len;
			break;
		}
		p = va_arg(args, Printable);
		if (p.type == PrintTypeTerm) break;
		switch (p.type) {
			case PrintTypeInt:
				vlen = sprint_int_impl(buffer, capacity, itt, buf_itt,
									   p.value.int_value);
				break;
			case PrintTypeUInt:
				vlen = sprint_uint_impl(buffer, capacity, itt, buf_itt,
										p.value.int_value);
				break;
			case PrintTypeFloat:
				vlen = sprint_float_impl(buffer, capacity, itt, buf_itt,
										 p.value.float_value);
				break;
			case PrintTypeString:
				vlen = sprint_string_impl(buffer, capacity, itt, buf_itt,
										  p.value.string_value);
				break;
			case PrintTypeObject:
				vlen = sprint_object_impl(buffer, capacity, itt, buf_itt,
										  p.value.object_value);
				break;
			default:
				vlen = -1;
				break;
		}
		if (vlen < 0) break;
		buf_itt += vlen;
	}
	va_end(args);
	return buf_itt;
}

i64 print_impl(Channel *channel, char *buffer, i64 capacity, int newline,
			   int exit, const char *prefix, const char *fmt, ...) {
	i64 fmt_len = cstring_len(fmt);
	va_list args;
	va_start(args, fmt);
	Printable p;
	i64 itt = 0;

	if (prefix) write_data(prefix, cstring_len(prefix));

	do {
		const char *s = cstring_strstr(fmt + itt, "{}");
		if (s) {
			i64 diff = 2 + (s - fmt);
			write_data(fmt + itt, (s - fmt) - itt);

			itt = diff;
		} else {
			write_data(fmt + itt, fmt_len - itt);
			itt = fmt_len;
			break;
		}
		p = va_arg(args, Printable);
		if (p.type == PrintTypeInt) {
			u64 len = cstring_itoai64(p.value.int_value, 0, 10, 0);
			if (len > 0) {
				char buf[len + 1];
				cstring_itoai64(p.value.int_value, buf, 10, len);
				write_data(buf, len);
			}
		} else if (p.type == PrintTypeUInt) {
			u64 len = cstring_itoau64(p.value.int_value, 0, 10, 0);
			if (len > 0) {
				char buf[len + 1];
				cstring_itoau64(p.value.int_value, buf, 10, len);
				write_data(buf, len);
			}
		} else if (p.type == PrintTypeFloat) {
			u64 len = snprintf(0, 0, "%f", p.value.float_value);
			if (len > 0) {
				char buf[len + 1];
				snprintf(buf, len + 1, "%f", p.value.float_value);
				write_data(buf, len);
			}
		} else if (p.type == PrintTypeString) {
			if (p.value.string_value != 0) {
				u64 len = cstring_len(p.value.string_value);
				if (len > 0) write_data(p.value.string_value, len);
			}
		} else if (p.type == PrintTypeObject) {
			ObjectType type = object_type(&p.value.object_value);
			if (type == Int) {
				i64 v = $int(p.value.object_value);
				u64 len = cstring_itoai64(v, 0, 10, 0);
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
				u64 v = $uint(p.value.object_value);
				u64 len = cstring_itoau64(v, 0, 10, 0);
				if (len) {
					char buf[len + 1];
					cstring_itoau64(v, buf, 10, len);
					write_data(buf, len);
				}
			} else if (type == Err) {
				const char *err_text = $err(p.value.object_value);
				const char *bt = $backtrace(p.value.object_value);
				u64 len = cstring_len(err_text);
				if (len) write_data(err_text, len);
				u64 bt_len = cstring_len(bt);
				if (len && bt_len) write_data("\n", 1);
				if (bt_len) write_data(bt, bt_len);
			} else {
			}  // TODO: handle other types
		}

	} while (p.type != PrintTypeTerm);
	va_end(args);

	if (itt < fmt_len) {
		write_data(fmt + itt, fmt_len - itt);
	}
	if (newline) write_data("\n", 1);
	if (exit) {
		const char *bt = backtrace_full();
		write_data(bt, cstring_len(bt));
		_exit(-1);
	}
	return 0;
}
