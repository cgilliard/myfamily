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

#include <core/ekinds.h>
#include <core/error.h>
#include <core/formatter.h>
#include <core/string.h>
#include <core/unit.h>
#include <log/log.h>

GETTER(Log, formatter)
GETTER(Log, config)

void Log_cleanup(Log *log) {
	FormatterPtr f = GET(Log, log, formatter);
	cleanup(&f);
}

Result Log_log(Log *log, LogLevel level, String line) {
	String full_line = STRING("");
	LogConfig config = GET(Log, log, config);
	if (config.show_log_level) {
		String level_str;
		if (level == TRACE)
			level_str = STRING("TRACE: ");
		else if (level == DEBUG)
			level_str = STRING("DEBUG: ");
		else if (level == INFO)
			level_str = STRING("INFO: ");
		else if (level == WARN)
			level_str = STRING("WARN: ");
		else if (level == ERROR)
			level_str = STRING("ERROR: ");
		else if (level == FATAL)
			level_str = STRING("FATAL: ");
		else {
			Error err = ERR(ILLEGAL_STATE, "unexpected log level");
			return Err(err);
		}
		Result r1 = append(&full_line, &level_str);
		TRYU(r1);
	}
	if (config.show_timestamp) {
		String ts_string = STRING("[Tue Jul 23 14:59:52 PDT 2024]: ");
		Result r1 = append(&full_line, &ts_string);
		TRYU(r1);
	}
	Result r2 = append(&full_line, &line);
	TRYU(r2);
	printf("%s\n", unwrap(&full_line));

	return Ok(_());
}

Formatter Log_formatter(Log *log) { return GET(Log, log, formatter); }

Result Log_build_impl(int n, va_list ptr, bool is_rc) {
	LogConfig lc;
	lc.show_log_level = true;
	lc.show_timestamp = true;
	lc.formatter_size = 10000;

	for (int i = 0; i < n; i++) {
		LogConfigOptionPtr next;
		Rc rc;
		if (!is_rc)
			next = va_arg(ptr, LogConfigOption);
		else {
			rc = va_arg(ptr, Rc);
			void *ptr = unwrap(&rc);
			memcpy(&next, ptr, size(ptr));
		}

		MATCH(next, VARIANT(SHOW_TIMESTAMP, {
			      lc.show_timestamp =
				  ENUM_VALUE(lc.show_timestamp, bool, next);
		      }) VARIANT(SHOW_LOG_LEVEL, {
			      lc.show_log_level =
				  ENUM_VALUE(lc.show_log_level, bool, next);
		      }) VARIANT(FORMATTER_SIZE, {
			      lc.formatter_size =
				  ENUM_VALUE(lc.formatter_size, u64, next);
		      }) VARIANT(SHOW_TERMINAL, {}));
	}
	va_end(ptr);

	FormatterPtr f = FORMATTER(lc.formatter_size);
	LogPtr ret = BUILD(Log, lc, NULL, f);
	return Ok(ret);
}

Result Log_build_rc(int n, ...) {
	va_list ptr;
	va_start(ptr, n);
	return Log_build_impl(n, ptr, true);
}

Result Log_build(int n, ...) {
	va_list ptr;
	va_start(ptr, n);
	return Log_build_impl(n, ptr, false);
}
