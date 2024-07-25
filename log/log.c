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

#include <core/colors.h>
#include <core/ekinds.h>
#include <core/error.h>
#include <core/formatter.h>
#include <core/string.h>
#include <core/unit.h>
#include <log/log.h>
#include <sys/time.h>
#include <time.h>

GETTER(Log, formatter)
GETTER(Log, config)

void Log_cleanup(Log *log) {
	FormatterPtr f = GET(Log, log, formatter);
	cleanup(&f);
	LogConfig config = GET(Log, log, config);
	cleanup(&config.log_file_path);
	cleanup(&config.file_header);
}

Result Log_log(Log *log, LogLevel level, String line) {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	char buf[100];
	char milli_buf[20];
	String full_line = STRING("");
	LogConfig config = GET(Log, log, config);

	if (config.show_millis) {
		struct timeval time;
		gettimeofday(&time, NULL);
		int millis = time.tv_usec / 1000;
		snprintf(milli_buf, 20, ".%03d", millis);
	} else
		strcpy(milli_buf, "");

	if (config.show_timestamp) {
		if (config.show_colors) {
			snprintf(
			    buf, 100,
			    "[%s%d-%02d-%02d %02d:%02d:%02d%s%s]: ", DIMMED,
			    tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			    tm.tm_hour, tm.tm_min, tm.tm_sec, milli_buf, RESET);
		} else
			snprintf(buf, 100, "[%d-%02d-%02d %02d:%02d:%02d%s]: ",
				 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
				 tm.tm_hour, tm.tm_min, tm.tm_sec, milli_buf);
		String ts_string = STRING(buf);
		Result r1 = append(&full_line, &ts_string);
		TRYU(r1);
	}
	if (config.show_log_level) {
		String level_str;
		if (level == TRACE) {
			if (config.show_colors)
				snprintf(buf, 100, "(%sTRACE%s): ", BLUE,
					 RESET);
			else
				snprintf(buf, 100, "(TRACE): ");
			level_str = STRING(buf);
		} else if (level == DEBUG) {
			if (config.show_colors)
				snprintf(buf, 100, "(%sDEBUG%s): ", CYAN,
					 RESET);
			else
				snprintf(buf, 100, "(DEBUG): ");
			level_str = STRING(buf);
		} else if (level == INFO) {
			if (config.show_colors)
				snprintf(buf, 100, "(%sINFO%s) : ", GREEN,
					 RESET);
			else
				snprintf(buf, 100, "(INFO) : ");
			level_str = STRING(buf);
		} else if (level == WARN) {
			if (config.show_colors)
				snprintf(buf, 100, "(%sWARN%s) : ", YELLOW,
					 RESET);
			else
				snprintf(buf, 100, "(WARN) : ");
			level_str = STRING(buf);
		} else if (level == ERROR) {
			if (config.show_colors)
				snprintf(buf, 100, "(%sERROR%s): ", MAGENTA,
					 RESET);
			else
				snprintf(buf, 100, "(ERROR): ");
			level_str = STRING(buf);
		} else if (level == FATAL) {
			if (config.show_colors)
				snprintf(buf, 100, "(%sFATAL%s): ", BRIGHT_RED,
					 RESET);
			else
				snprintf(buf, 100, "(FATAL): ");
			level_str = STRING(buf);
		} else {
			Error err = ERR(ILLEGAL_STATE, "unexpected log level");
			return Err(err);
		}
		Result r1 = append(&full_line, &level_str);
		TRYU(r1);
	}
	if (config.show_linenum) {
		char bt_buf[2000];
		Backtrace bt = BUILD(Backtrace, NULL, 0);
		Backtrace_generate(&bt, 100);

		u64 count = GET(Backtrace, &bt, count);
		bool found_log_c = false;
		bool found_path = false;
		for (u64 i = 0; i < count; i++) {
			Backtrace_file_path(&bt, bt_buf, 2000, i);
			if (strstr(bt_buf, "log/log.c:"))
				found_log_c = true;
			else if (found_log_c) {
				found_path = true;
				break;
			}
		}

		if (!found_path)
			strcpy(bt_buf, "UNKNOWN");
		// take out space (linux)
		for (u64 i = 0; i < strlen(bt_buf); i++)
			if (bt_buf[i] == ' ')
				bt_buf[i] = 0;

		String line_num_str;
		if (config.show_colors) {
			snprintf(buf, 100, "[%s%s%s]: ", YELLOW, bt_buf, RESET);
			line_num_str = STRING(buf);
		} else {
			snprintf(buf, 100, "[%s]: ", bt_buf);
			line_num_str = STRING(buf);
		}
		Result r1 = append(&full_line, &line_num_str);
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
	lc.show_linenum = true;
	lc.show_colors = true;
	lc.show_millis = true;
	lc.log_file_path = None;
	lc.file_header = None;

	for (int i = 0; i < n; i++) {
		LogConfigOptionPtr next;
		Rc rc;
		if (!is_rc)
			next = va_arg(ptr, LogConfigOption);
		else {
			rc = va_arg(ptr, Rc);
			void *vptr = unwrap(&rc);
			memcpy(&next, vptr, size(vptr));
		}

		// clang-format off
		MATCH(next,
			VARIANT(SHOW_TIMESTAMP, {
				lc.show_timestamp = ENUM_VALUE(lc.show_timestamp, bool, next);
			}) VARIANT(SHOW_LOG_LEVEL, {
				lc.show_log_level = ENUM_VALUE(lc.show_log_level, bool, next);
			}) VARIANT(FORMATTER_SIZE, {
				lc.formatter_size = ENUM_VALUE(lc.formatter_size, u64, next);
			}) VARIANT(SHOW_MILLIS, {
				lc.show_millis = ENUM_VALUE(lc.show_millis, bool, next);
			}) VARIANT(LOG_FILE_PATH, {
				StringPtr s = ENUM_VALUE(s, String, next);
				StringPtr sclone;
				clone(&sclone, &s);
				lc.log_file_path = Some(sclone);
			}) VARIANT(FILE_HEADER, {
				StringPtr s = ENUM_VALUE(s, String, next);
				StringPtr sclone;
				clone(&sclone, &s);
				lc.file_header = Some(sclone);
			}) VARIANT(SHOW_COLORS, {
				lc.show_colors = ENUM_VALUE(lc.show_colors, bool, next);
			}) VARIANT(LOG_SYNC, {
                                lc.is_sync = ENUM_VALUE(lc.is_sync, bool, next);
			}) VARIANT(SHOW_TERMINAL, {
				lc.show_terminal = ENUM_VALUE(lc.show_terminal, bool, next);
			}) VARIANT(AUTO_ROTATE, {
				lc.auto_rotate = ENUM_VALUE(lc.auto_rotate, bool, next);
			}) VARIANT(DELETE_ROTATION, {
				lc.delete_rotation = ENUM_VALUE(lc.delete_rotation, bool, next);
			}) VARIANT(MAX_AGE_MILLIS, {
                                lc.max_age_millis = ENUM_VALUE(lc.max_age_millis, u64, next);
			}) VARIANT(MAX_SIZE_BYTES, {
                                lc.max_size_bytes= ENUM_VALUE(lc.max_size_bytes, u64, next);
			}) VARIANT(SHOW_LINENUM, {
				lc.show_linenum = ENUM_VALUE(lc.show_linenum, bool, next);
			}));
		// clang-format on
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
