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

#include <core/formatter.h>
#include <core/string.h>
#include <core/unit.h>
#include <log/log.h>

void Log_cleanup(Log *log) {}

Result Log_log(Log *log, LogLevel level, String fmt, ...) {
	Formatter formatter = FORMATTER(10000);
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
	va_start(ptr, n);

	itt = 0;
	String fmt_str = STRING("");
	for (int i = 0; i < n; i++) {
		Result r = String_index_of_s(&fmt, "{}", itt);
		i64 v = TRY(r, v);
		if (v < 0)
			break;
		String sub = SUBSTRING(&fmt, itt, v);
		Result r2 = append(&fmt_str, &sub);
		Result r3 = WRITE(&formatter, unwrap(&sub));
		Object *next = va_arg(ptr, void *);
		if (CLASS_NAME(next) == NULL) {
			printf("null\n");
		}
		Result r4 = to_string(next);
		String s4 = TRY(r4, s4);
		Result r5 = WRITE(&formatter, unwrap(&s4));

		itt = v + 2;
	}
	String sub = SUBSTRING(&fmt, itt, len(&fmt));
	Result r5 = WRITE(&formatter, unwrap(&sub));

	va_end(ptr);
	Result r6 = Formatter_to_string(&formatter);
	String s6 = UNWRAP(r6, String);
	printf("ERROR: '%s'\n", unwrap(&s6));

	return Ok(UNIT);
}
