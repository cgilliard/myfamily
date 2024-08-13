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

#include <core/std.h>
#include <pthread.h>

#define _FILE_OFFSET_BITS 64

ENUM(LogConfigOption,
     VARIANTS(SHOW_COLORS, SHOW_TERMINAL, SHOW_TIMESTAMP, SHOW_MILLIS,
	      SHOW_LOG_LEVEL, SHOW_LINENUM, AUTO_ROTATE, DELETE_ROTATION,
	      MAX_SIZE_BYTES, MAX_AGE_MILLIS, LOG_FILE_PATH, FILE_HEADER,
	      LOG_SYNC, FORMATTER_SIZE, LINENUM_MAX_LEN),
     TYPES("bool", "bool", "bool", "bool", "bool", "bool", "bool", "bool",
	   "u64", "u64", "String", "String", "bool", "u64", "u16"))
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
	bool show_terminal;
	bool show_timestamp;
	bool show_millis;
	bool show_log_level;
	bool show_linenum;
	bool auto_rotate;
	bool delete_rotation;
	u64 max_size_bytes;
	u64 max_age_millis;
	Option log_file_path;
	Option file_header;
	bool is_sync;
	u64 formatter_size;
	u16 line_num_max_len;
} LogConfig;

#define LOG_CORE(T)                                                            \
	TRAIT_REQUIRED(T, Result, log, T##Ptr *log, LogLevel level,            \
		       String line)                                            \
	TRAIT_REQUIRED(T, Formatter, formatter, T##Ptr *log)                   \
	TRAIT_REQUIRED(T, Result, build, int n, ...)                           \
	TRAIT_REQUIRED(T, Result, build_rc, int n, ...)                        \
	TRAIT_REQUIRED(T, Result, rotate, T##Ptr *log)                         \
	TRAIT_REQUIRED(T, bool, need_rotate, T##Ptr *log)

CLASS(Log,
      FIELD(LogConfig, config) FIELD(FILE *, fp) FIELD(Formatter, formatter)
	  FIELD(u64, cur_size) FIELD(u64, last_rotation) FIELD(Slab, lock))
IMPL(Log, LOG_CORE)
#define Log DEFINE_CLASS(Log)

static GETTER(Log, lock);

#define PROC_LOG_CONFIG(value) ({ value; })

#define ShowLogLevel(x)                                                        \
	({                                                                     \
		bool __v1_ = x;                                                \
		LogConfigOption __opt1_ =                                      \
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

#define ShowTerminal(x)                                                        \
	({                                                                     \
		bool __v6_ = x;                                                \
		LogConfigOptionPtr __opt6_ =                                   \
		    BUILD_ENUM(LogConfigOption, SHOW_TERMINAL, __v6_);         \
		RcPtr __rc6_ = HEAPIFY(__opt6_);                               \
		__rc6_;                                                        \
	})

#define ShowColors(x)                                                          \
	({                                                                     \
		bool __v7_ = x;                                                \
		LogConfigOptionPtr __opt7_ =                                   \
		    BUILD_ENUM(LogConfigOption, SHOW_COLORS, __v7_);           \
		RcPtr __rc7_ = HEAPIFY(__opt7_);                               \
		__rc7_;                                                        \
	})

#define AutoRotate(x)                                                          \
	({                                                                     \
		bool __v8_ = x;                                                \
		LogConfigOptionPtr __opt8_ =                                   \
		    BUILD_ENUM(LogConfigOption, AUTO_ROTATE, __v8_);           \
		RcPtr __rc8_ = HEAPIFY(__opt8_);                               \
		__rc8_;                                                        \
	})

#define DeleteRotation(x)                                                      \
	({                                                                     \
		bool __v9_ = x;                                                \
		LogConfigOptionPtr __opt9_ =                                   \
		    BUILD_ENUM(LogConfigOption, DELETE_ROTATION, __v9_);       \
		RcPtr __rc9_ = HEAPIFY(__opt9_);                               \
		__rc9_;                                                        \
	})

#define MaxAgeMillis(x)                                                        \
	({                                                                     \
		u64 __v10_ = x;                                                \
		LogConfigOptionPtr __opt10_ =                                  \
		    BUILD_ENUM(LogConfigOption, MAX_AGE_MILLIS, __v10_);       \
		RcPtr __rc10_ = HEAPIFY(__opt10_);                             \
		__rc10_;                                                       \
	})

#define MaxSizeBytes(x)                                                        \
	({                                                                     \
		u64 __v11_ = x;                                                \
		LogConfigOptionPtr __opt11_ =                                  \
		    BUILD_ENUM(LogConfigOption, MAX_SIZE_BYTES, __v11_);       \
		RcPtr __rc11_ = HEAPIFY(__opt11_);                             \
		__rc11_;                                                       \
	})

#define LogSync(x)                                                             \
	({                                                                     \
		bool __v12_ = x;                                               \
		LogConfigOptionPtr __opt12_ =                                  \
		    BUILD_ENUM(LogConfigOption, LOG_SYNC, __v12_);             \
		RcPtr __rc12_ = HEAPIFY(__opt12_);                             \
		__rc12_;                                                       \
	})

#define LogFilePath(x)                                                         \
	({                                                                     \
		StringPtr __p13_ = STRING(x);                                  \
		LogConfigOptionPtr __opt13_ =                                  \
		    BUILD_ENUM(LogConfigOption, LOG_FILE_PATH, __p13_);        \
		RcPtr __rc13_ = HEAPIFY(__opt13_);                             \
		__rc13_;                                                       \
	})

#define FileHeader(x)                                                          \
	({                                                                     \
		StringPtr __p14_ = STRING(x);                                  \
		LogConfigOptionPtr __opt14_ =                                  \
		    BUILD_ENUM(LogConfigOption, FILE_HEADER, __p14_);          \
		RcPtr __rc14_ = HEAPIFY(__opt14_);                             \
		__rc14_;                                                       \
	})

#define LineNumMaxLen(x)                                                       \
	({                                                                     \
		u16 __v15_ = x;                                                \
		LogConfigOptionPtr __opt15_ =                                  \
		    BUILD_ENUM(LogConfigOption, LINENUM_MAX_LEN, __v15_);      \
		RcPtr __rc15_ = HEAPIFY(__opt15_);                             \
		__rc15_;                                                       \
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
	return Ok(UNIT);
}

#define log_impl(l, local_level, level, fmt, ...)                              \
	({                                                                     \
		if (local_level <= level) {                                    \
			Slab lslab = GET(Log, l, lock);                        \
			pthread_mutex_t *lock = lslab.data;                    \
			if (lock) {                                            \
				if (pthread_mutex_lock(lock))                  \
					panic("Could not obtain lock");        \
			}                                                      \
			int __counter___ = 0;                                  \
			EXPAND(FOR_EACH(COUNT_ARGS, __VA_ARGS__));             \
			FormatterPtr formatter = Log_formatter(l);             \
			Formatter_reset(&formatter);                           \
			Result _r1__ =                                         \
			    FORMAT(&formatter, (char *)fmt, __VA_ARGS__);      \
			if (IS_ERR(_r1__) && lock)                             \
				if (pthread_mutex_unlock(lock))                \
					panic("Could not unlock "              \
					      "pthread_mutex_t");              \
			TRYU(_r1__);                                           \
			Result _r2__ = Formatter_to_string(&formatter);        \
			if (IS_ERR(_r2__) && lock)                             \
				if (pthread_mutex_unlock(lock))                \
					panic("Could not unlock "              \
					      "pthread_mutex_t");              \
			String _s__ = TRY(_r2__, _s__);                        \
			Result _r3__ = Log_log(l, level, _s__);                \
			if (IS_ERR(_r3__) && lock)                             \
				if (pthread_mutex_unlock(lock))                \
					panic("Could not unlock "              \
					      "pthread_mutex_t");              \
			TRYU(_r3__);                                           \
			if (lock)                                              \
				if (pthread_mutex_unlock(lock))                \
					panic("Could not unlock "              \
					      "pthread_mutex_t");              \
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
