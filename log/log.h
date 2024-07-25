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
#include <core/error.h>
#include <core/format.h>
#include <core/formatter.h>
#include <core/macro_utils.h>
#include <core/result.h>
#include <core/string.h>

#define _FILE_OFFSET_BITS 64

ENUM(LogConfigOption,
     VARIANTS(SHOW_COLORS, SHOW_TERMINAL, SHOW_TIMESTAMP, SHOW_MILLIS,
	      SHOW_LOG_LEVEL, SHOW_LINENUM, AUTO_ROTATE, DELETE_ROTATION,
	      MAX_SIZE_BYTES, MAX_AGE_MILLIS, LOG_FILE_PATH, FILE_HEADER,
	      LOG_SYNC, FORMATTER_SIZE),
     TYPES("bool", "bool", "bool", "bool", "bool", "bool", "bool", "bool",
	   "u64", "u64", "String", "String", "bool", "u64"))
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
	bool is_sync;
	u64 formatter_size;
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

#define COUNT_ARGS(value) ({ __counter___ += 1; })

#define PROC_LOG_CONFIG(value) ({ value; })

#define ShowLogLevel(x)                                                        \
	({                                                                     \
		bool __v1_ = x;                                                \
		LogConfigOptionPtr __opt1_ =                                   \
		    BUILD_ENUM(LogConfigOption, SHOW_LOG_LEVEL, __v1_);        \
		RcPtr __rc1_ = HEAPIFY(__opt1_);                               \
		__rc1_;                                                        \
	})

#define ShowTimestamp(x)                                                       \
	({                                                                     \
		bool __v2_ = x;                                                \
		LogConfigOptionPtr __opt2_ =                                   \
		    BUILD_ENUM(LogConfigOption, SHOW_TIMESTAMP, __v2_);        \
		RcPtr __rc2_ = HEAPIFY(__opt2_);                               \
		__rc2_;                                                        \
	})

#define FormatterSize(x)                                                       \
	({                                                                     \
		u64 __v3_ = x;                                                 \
		LogConfigOptionPtr _opt3__ =                                   \
		    BUILD_ENUM(LogConfigOption, FORMATTER_SIZE, __v3_);        \
		RcPtr __rc3_ = HEAPIFY(_opt3__);                               \
		__rc3_;                                                        \
	})

#define ShowLineNum(x)                                                         \
	({                                                                     \
		bool __v4_ = x;                                                \
		LogConfigOptionPtr __opt4_ =                                   \
		    BUILD_ENUM(LogConfigOption, SHOW_LINENUM, __v4_);          \
		RcPtr __rc4_ = HEAPIFY(__opt4_);                               \
		__rc4_;                                                        \
	})

#define ShowMillis(x)                                                          \
	({                                                                     \
		bool __v5_ = x;                                                \
		LogConfigOptionPtr __opt5_ =                                   \
		    BUILD_ENUM(LogConfigOption, SHOW_MILLIS, __v5_);           \
		RcPtr __rc5_ = HEAPIFY(__opt5_);                               \
		__rc5_;                                                        \
	})

#define LOG(...)                                                               \
	({                                                                     \
		int __counter___ = 0;                                          \
		EXPAND(FOR_EACH(COUNT_ARGS, __VA_ARGS__));                     \
		Result __rr___ =                                               \
		    Log_build_rc(__counter___ __VA_OPT__(, ) __VA_OPT__(       \
			EXPAND(FOR_EACH(PROC_LOG_CONFIG, __VA_ARGS__))));      \
		LogPtr __log___ = TRY(__rr___, __log___);                      \
		__log___;                                                      \
	})

static LogPtr *GLOBAL_LOGGER = NULL;
static Result init_global_log() {
	if (!GLOBAL_LOGGER) {
		GLOBAL_LOGGER = malloc(sizeof(LogPtr));
		*GLOBAL_LOGGER = LOG();
	}
	return Ok(_());
}

#define log_impl(l, local_level, level, fmt, ...)                              \
	({                                                                     \
		if (local_level <= level) {                                    \
			int __counter___ = 0;                                  \
			EXPAND(FOR_EACH(COUNT_ARGS, __VA_ARGS__));             \
			FormatterPtr formatter = Log_formatter(l);             \
			Formatter_reset(&formatter);                           \
			Result _r1__ =                                         \
			    FORMAT(&formatter, (char *)fmt, __VA_ARGS__);      \
			TRYU(_r1__);                                           \
			Result _r2__ = Formatter_to_string(&formatter);        \
			String _s__ = TRY(_r2__, _s__);                        \
			Result _r3__ = Log_log(l, level, _s__);                \
			TRYU(_r3__);                                           \
		}                                                              \
	})

#define itrace(l, fmt, ...) log_impl(l, LOG_LEVEL, TRACE, fmt, __VA_ARGS__)
#define trace(fmt, ...)                                                        \
	log_impl(GLOBAL_LOGGER, LOG_LEVEL, TRACE, fmt, __VA_ARGS__)

#define idebug(l, fmt, ...) log_impl(l, LOG_LEVEL, DEBUG, fmt, __VA_ARGS__)
#define debug(fmt, ...)                                                        \
	log_impl(GLOBAL_LOGGER, LOG_LEVEL, DEBUG, fmt, __VA_ARGS__)

#define iinfo(l, fmt, ...) log_impl(l, LOG_LEVEL, INFO, fmt, __VA_ARGS__)
#define info(fmt, ...)                                                         \
	log_impl(GLOBAL_LOGGER, LOG_LEVEL, INFO, fmt, __VA_ARGS__)

#define iwarn(l, fmt, ...) log_impl(l, LOG_LEVEL, WARN, fmt, __VA_ARGS__)
#define warn(fmt, ...)                                                         \
	log_impl(GLOBAL_LOGGER, LOG_LEVEL, WARN, fmt, __VA_ARGS__)

#define ierror(l, fmt, ...) log_impl(l, LOG_LEVEL, ERROR, fmt, __VA_ARGS__)
#define error(fmt, ...)                                                        \
	log_impl(GLOBAL_LOGGER, LOG_LEVEL, ERROR, fmt, __VA_ARGS__)

#define ifatal(l, fmt, ...) log_impl(l, LOG_LEVEL, FATAL, fmt, __VA_ARGS__)
#define fatal(fmt, ...)                                                        \
	log_impl(GLOBAL_LOGGER, LOG_LEVEL, FATAL, fmt, __VA_ARGS__)

#endif // _LOG_LOG__
