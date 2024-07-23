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

#ifndef _LOG_LOG__
#define _LOG_LOG__

#include <core/class.h>
#include <core/colors.h>
#include <core/enum.h>
#include <core/formatter.h>
#include <core/macro_utils.h>
#include <core/string.h>

#define _FILE_OFFSET_BITS 64

ENUM(LogConfigOption,
     VARIANTS(SHOW_COLORS, SHOW_TERMINAL, SHOW_TIMESTAMP, SHOW_MILLIS,
	      SHOW_LOG_LEVEL, SHOW_LINENUM, AUTO_ROTATE, DELETE_ROTATION,
	      MAX_SIZE_BYTES, MAX_AGE_MILLIS, LOG_FILE_PATH, FILE_HEADER),
     TYPES("bool", "bool", "bool", "bool", "bool", "bool", "bool", "bool",
	   "u64", "u64", "String", "String"))
#define LogConfigOption DEFINE_ENUM(LogConfigOption)

typedef enum LogLevel {
	TRACE = 0,
	DEBUG = 1,
	INFO = 2,
	WARN = 3,
	ERROR = 4,
	FATAL = 5,
} LogLevel;

typedef struct LogConfig {
	bool show_colors;
	bool show_stdout;
	bool show_timestamp;
	bool show_millis;
	bool show_log_level;
	bool show_linenum;
	bool auto_rotate;
	bool delete_rotation;
	u64 max_size_bytes;
	u64 max_age_millis;
	String log_file_path;
	String file_header;
} LogConfig;

#define LOG_CORE(T)                                                            \
	TRAIT_REQUIRED(T, Result, log, T##Ptr *log, LogLevel level,            \
		       String line)                                            \
	TRAIT_REQUIRED(T, Formatter, formatter, T##Ptr *log)                   \
	TRAIT_REQUIRED(T, Result, build, int n, ...)                           \
	TRAIT_REQUIRED(T, Result, build_rc, int n, ...)

CLASS(Log,
      FIELD(LogConfig, config) FIELD(FILE *, fp) FIELD(Formatter, formatter))
IMPL(Log, LOG_CORE)
#define Log DEFINE_CLASS(Log)

#define COUNT_LOG_CONFIG(value) ({ __counter___ += 1; })

#define PROC_LOG_CONFIG(value) ({ value; })

#define ShowLogLevel(x)                                                        \
	({                                                                     \
		bool v = x;                                                    \
		LogConfigOptionPtr opt =                                       \
		    BUILD_ENUM(LogConfigOption, SHOW_LOG_LEVEL, v);            \
		RcPtr rc = HEAPIFY(opt);                                       \
		rc;                                                            \
	})

#define ShowTimestamp(x)                                                       \
	({                                                                     \
		bool v = x;                                                    \
		LogConfigOptionPtr opt2 =                                      \
		    BUILD_ENUM(LogConfigOption, SHOW_TIMESTAMP, v);            \
		RcPtr rc2 = HEAPIFY(opt2);                                     \
		rc2;                                                           \
	})

#define LOG(...)                                                               \
	({                                                                     \
		int __counter___ = 0;                                          \
		EXPAND(FOR_EACH(COUNT_LOG_CONFIG, __VA_ARGS__));               \
		Result __rr___ =                                               \
		    Log_build_rc(__counter___ __VA_OPT__(, ) __VA_OPT__(       \
			EXPAND(FOR_EACH(PROC_LOG_CONFIG, __VA_ARGS__))));      \
		LogPtr __log___ = TRY(__rr___, __log___);                      \
		__log___;                                                      \
	})

#define debug(l, fmt, ...)                                                     \
	({                                                                     \
		FormatterPtr formatter = Log_formatter(l);                     \
		Formatter_reset(&formatter);                                   \
		Result _r1__ = FORMAT(&formatter, fmt, __VA_ARGS__);           \
		TRYU(_r1__);                                                   \
		Result _r2__ = Formatter_to_string(&formatter);                \
		String _s__ = TRY(_r2__, _s__);                                \
		Result _r2debug__ = Log_log(l, DEBUG, _s__);                   \
		TRYU(_r2debug__);                                              \
	})

#define info(l, fmt, ...)                                                      \
	({                                                                     \
		FormatterPtr formatter = Log_formatter(l);                     \
		Formatter_reset(&formatter);                                   \
		Result _r1__ = FORMAT(&formatter, fmt, __VA_ARGS__);           \
		TRYU(_r1__);                                                   \
		Result _r2__ = Formatter_to_string(&formatter);                \
		String _s__ = TRY(_r2__, _s__);                                \
		Result _r2debug__ = Log_log(l, INFO, _s__);                    \
		TRYU(_r2debug__);                                              \
	})

#endif // _LOG_LOG__
