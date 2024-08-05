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

#ifndef _CORE_BACKTRACE__
#define _CORE_BACKTRACE__

#include <core/class.h>
#include <core/traits_base.h>

#define TRAIT_GENERATE_BACKTRACE(T)                                            \
	TRAIT_REQUIRED(T, bool, generate, T##Ptr *bt, u64 max_depth)           \
	TRAIT_REQUIRED(T, T##Ptr, generate_bt, u64 max_depth)

#define TRAIT_SET_BACKTRACE_ENTRY(T)                                           \
	TRAIT_REQUIRED(T, void, set_backtrace_entry_values, T##Ptr *bt,        \
		       const char *name, const char *bin_name,                 \
		       const char *address, const char *file_path)

#define TRAIT_GET_BACKTRACE(T)                                                 \
	TRAIT_REQUIRED(T, bool, fn_name, T##Ptr *bt, char *buffer, u64 len,    \
		       u64 index)                                              \
	TRAIT_REQUIRED(T, bool, bin_name, T##Ptr *bt, char *buffer, u64 len,   \
		       u64 index)                                              \
	TRAIT_REQUIRED(T, bool, address, T##Ptr *bt, char *buffer, u64 len,    \
		       u64 index)                                              \
	TRAIT_REQUIRED(T, bool, file_path, T##Ptr *bt, char *buffer, u64 len,  \
		       u64 index)

#define EMPTY_BACKTRACE_ENTRY BUILD(BacktraceEntry, NULL, NULL, NULL, NULL)
#define EMPTY_BACKTRACE BUILD(Backtrace, NULL, 0)

#define BACKTRACE_CHAR_ARRAY_LEN 100
#define MAX_BACKTRACE_ROWS 100

CLASS(BacktraceEntry,
      FIELD(char, name[BACKTRACE_CHAR_ARRAY_LEN])
	  FIELD(char, bin_name[BACKTRACE_CHAR_ARRAY_LEN])
	      FIELD(char, address[BACKTRACE_CHAR_ARRAY_LEN])
		  FIELD(char, file_path[BACKTRACE_CHAR_ARRAY_LEN]))
IMPL(BacktraceEntry, TRAIT_COPY)
IMPL(BacktraceEntry, TRAIT_SET_BACKTRACE_ENTRY)

#define BacktraceEntry DEFINE_CLASS(BacktraceEntry)

CLASS(Backtrace,
      FIELD(BacktraceEntryPtr, rows[MAX_BACKTRACE_ROWS]) FIELD(u64, count))
IMPL(Backtrace, TRAIT_COPY)
IMPL(Backtrace, TRAIT_GENERATE_BACKTRACE)
IMPL(Backtrace, TRAIT_PRINT)
IMPL(Backtrace, TRAIT_GET_BACKTRACE)
#define Backtrace DEFINE_CLASS(Backtrace)

GETTER_PROTO(Backtrace, count)
GETTER_PROTO(Backtrace, rows)
GETTER_PROTO(BacktraceEntry, name)
GETTER_PROTO(BacktraceEntry, bin_name)
GETTER_PROTO(BacktraceEntry, address)
GETTER_PROTO(BacktraceEntry, file_path)

#define INIT_BACKTRACE BUILD(Backtrace)
#define BACKTRACE(bt) ({ Backtrace_generate(bt, 100); })
#define GENERATE_BACKTRACE Backtrace_generate_bt(100)

#endif // _CORE_BACKTRACE__
