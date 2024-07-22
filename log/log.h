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
		       String fmt, ...)

CLASS(Log, FIELD(FILE *, fp) FIELD(LogConfig, config))
IMPL(Log, LOG_CORE)
#define Log DEFINE_CLASS(Log)

#define PROC_ARG(value)                                                        \
	_Generic((value),                                                      \
	    u128: ({                                                           \
			 U128Ptr _vu128__ = BUILD(U128);                       \
			 memcpy(&(_vu128__._value), &value, sizeof(u128));     \
			 _vu128__;                                             \
		 }),                                                           \
	    u64: ({                                                            \
			 U64Ptr _vu64__ = BUILD(U64);                          \
			 memcpy(&(_vu64__._value), &value, sizeof(u64));       \
			 _vu64__;                                              \
		 }),                                                           \
	    u32: ({                                                            \
			 U32Ptr _vu32__ = BUILD(U32);                          \
			 memcpy(&(_vu32__._value), &value, sizeof(u32));       \
			 _vu32__;                                              \
		 }),                                                           \
	    u16: ({                                                            \
			 U16Ptr _vu16__ = BUILD(U16);                          \
			 memcpy(&(_vu16__._value), &value, sizeof(u16));       \
			 _vu16__;                                              \
		 }),                                                           \
	    u8: ({                                                             \
			 U8Ptr _vu8__ = BUILD(U8);                             \
			 memcpy(&(_vu8__._value), &value, sizeof(u8));         \
			 _vu8__;                                               \
		 }),                                                           \
	    i128: ({                                                           \
			 U128Ptr _vi128__ = BUILD(I128);                       \
			 memcpy(&(_vi128__._value), &value, sizeof(i128));     \
			 _vi128__;                                             \
		 }),                                                           \
	    i64: ({                                                            \
			 U64Ptr _vi64__ = BUILD(I64);                          \
			 memcpy(&(_vi64__._value), &value, sizeof(i64));       \
			 _vi64__;                                              \
		 }),                                                           \
	    i32: ({                                                            \
			 U32Ptr _vi32__ = BUILD(I32);                          \
			 memcpy(&(_vi32__._value), &value, sizeof(i32));       \
			 _vi32__;                                              \
		 }),                                                           \
	    i16: ({                                                            \
			 U16Ptr _vi16__ = BUILD(I16);                          \
			 memcpy(&(_vi16__._value), &value, sizeof(i16));       \
			 _vi16__;                                              \
		 }),                                                           \
	    i8: ({                                                             \
			 I8Ptr _vi8__ = BUILD(I8);                             \
			 memcpy(&(_vi8__._value), &value, sizeof(i8));         \
			 _vi8__;                                               \
		 }),                                                           \
	    f64: ({                                                            \
			 U64Ptr _vf64__ = BUILD(F64);                          \
			 memcpy(&(_vf64__._value), &value, sizeof(f64));       \
			 _vf64__;                                              \
		 }),                                                           \
	    f32: ({                                                            \
			 U32Ptr _vf32__ = BUILD(F32);                          \
			 memcpy(&(_vf32__._value), &value, sizeof(f32));       \
			 _vf32__;                                              \
		 }),                                                           \
	    usize: ({                                                          \
			 USizePtr _vusize__ = BUILD(USize);                    \
			 memcpy(&(_vusize__._value), &value, sizeof(usize));   \
			 _vusize__;                                            \
		 }),                                                           \
	    isize: ({                                                          \
			 ISizePtr _visize__ = BUILD(ISize);                    \
			 memcpy(&(_visize__._value), &value, sizeof(isize));   \
			 _visize__;                                            \
		 }),                                                           \
	    bool: ({                                                           \
			 BoolPtr _bool__ = BUILD(Bool);                        \
			 memcpy(&(_bool__._value), &value, sizeof(bool));      \
			 _bool__;                                              \
		 }),                                                           \
	    default: ({ value; }))
#define debug(l, fmt, ...)                                                     \
	({ log(l, DEBUG, fmt, EXPAND(FOR_EACH(PROC_ARG, __VA_ARGS__))); })

#define log(l, level, fmt, ...)                                                \
	({                                                                     \
		String _s__ = STRING(fmt);                                     \
		Result _r__ = Log_log(l, level, _s__, __VA_ARGS__);            \
		TRYU(_r__);                                                    \
	})

#endif // _LOG_LOG__
