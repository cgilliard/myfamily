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

#ifndef _BACKTRACE_BASE__
#define _BAKCTRACE_BASE__

#include <base/class.h>
#include <base/traits.h>

#define TRAIT_GENERATE_BACKTRACE(T)                                            \
	TRAIT_REQUIRED(T, bool, generate, T##Ptr *bt, u64 max_depth)

#define TRAIT_SET_BACKTRACE_ENTRY(T)                                           \
	TRAIT_REQUIRED(T, bool, set_backtrace_entry_values, T##Ptr *bt,        \
		       const char *name, const char *bin_name,                 \
		       const char *address, const char *file_path)

#define TRAIT_GET_BACKTRACE(T)                                                 \
	TRAIT_REQUIRED(T, bool, fn_name, T##Ptr *bt, char *buffer, size_t len, \
		       u64 index)                                              \
	TRAIT_REQUIRED(T, bool, bin_name, T##Ptr *bt, char *buffer,            \
		       size_t len, u64 index)                                  \
	TRAIT_REQUIRED(T, bool, address, T##Ptr *bt, char *buffer, size_t len, \
		       u64 index)                                              \
	TRAIT_REQUIRED(T, bool, file_path, T##Ptr *bt, char *buffer,           \
		       size_t len, u64 index)

#define EMPTY_BACKTRACE_ENTRY BUILD(BacktraceEntry, NULL, NULL, NULL, NULL)
#define EMPTY_BACKTRACE BUILD(Backtrace, NULL, 0)

CLASS(BacktraceEntry, FIELD(char *, name) FIELD(char *, bin_name)
			  FIELD(char *, address) FIELD(char *, file_path))
IMPL(BacktraceEntry, TRAIT_COPY)
IMPL(BacktraceEntry, TRAIT_SIZE)
IMPL(BacktraceEntry, TRAIT_SET_BACKTRACE_ENTRY)

#define BacktraceEntry DEFINE_CLASS(BacktraceEntry)

CLASS(Backtrace, FIELD(BacktraceEntryPtr *, rows) FIELD(u64, count))
IMPL(Backtrace, TRAIT_COPY)
IMPL(Backtrace, TRAIT_SIZE)
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

#endif // _BACKTRACE_BASE__
