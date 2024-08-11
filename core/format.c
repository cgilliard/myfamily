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

#include <core/format.h>
#include <core/result.h>
#include <core/string_builder.h>

Result format(Formatter *formatter, char *format_s, ...) {
	String fmt = STRING(format_s);
	int n = 0;

	u64 itt = 0;
	while (true) {
		Result r = String_index_of_s(&fmt, "{}", itt);
		i64 v = TRY(r, v);
		if (v < 0)
			break;
		itt = v + 2;
		n += 1;
	}

	va_list ptr;
	va_start(ptr, format_s);

	itt = 0;
	StringBuilder fmt_str = STRING_BUILDER("", fmt_str);
	for (int i = 0; i < n; i++) {
		Result r = String_index_of_s(&fmt, "{}", itt);
		i64 v = TRY(r, v);
		if (v < 0)
			break;
		String sub = SUBSTRING(&fmt, itt, v);
		char *substr = unwrap(&sub);
		Result r2 = append(&fmt_str, substr);
		TRYU(r2);
		Object *next = va_arg(ptr, Object *);
		Result r4 = to_string(next);
		String s4 = TRY(r4, s4);
		char *s4str = unwrap(&s4);
		Result r6 = append(&fmt_str, s4str);
		TRYU(r6);

		itt = v + 2;
	}

	String sub = SUBSTRING(&fmt, itt, len(&fmt));
	char *substr = unwrap(&sub);
	Result r2 = append(&fmt_str, substr);
	TRYU(r2);
	Result fmt_str_res = to_string(&fmt_str);
	String fmt_str_s = TRY(fmt_str_res, fmt_str_s);
	Result r5 = WRITE(formatter, unwrap(&fmt_str_s));
	TRYU(r5);
	va_end(ptr);
	return Ok(UNIT);
}
