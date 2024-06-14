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

#define EMPTY_BACKTRACE_ENTRY BUILD(BacktraceEntry, NULL, NULL, NULL, NULL)
#define EMPTY_BACKTRACE BUILD(Backtrace, NULL, 0)

CLASS(BacktraceEntry, FIELD(char *, name) FIELD(char *, bin_name)
			  FIELD(char *, address) FIELD(char *, file_path))
IMPL(BacktraceEntry, TRAIT_COPY)
IMPL(BacktraceEntry, TRAIT_SIZE)
IMPL(BacktraceEntry, TRAIT_SET_BACKTRACE_ENTRY)

#define BacktraceEntry DEFINE_CLASS(BacktraceEntry)
GETTER(BacktraceEntry, name);
SETTER(BacktraceEntry, name);
GETTER(BacktraceEntry, bin_name);
SETTER(BacktraceEntry, bin_name);
GETTER(BacktraceEntry, address);
SETTER(BacktraceEntry, address);
GETTER(BacktraceEntry, file_path);
SETTER(BacktraceEntry, file_path);

CLASS(Backtrace, FIELD(BacktraceEntryPtr *, rows) FIELD(u64, count))
IMPL(Backtrace, TRAIT_COPY)
IMPL(Backtrace, TRAIT_SIZE)
IMPL(Backtrace, TRAIT_GENERATE_BACKTRACE)
IMPL(Backtrace, TRAIT_TO_STR)
IMPL(Backtrace, TRAIT_PRINT)
#define Backtrace DEFINE_CLASS(Backtrace)
GETTER(Backtrace, rows);
SETTER(Backtrace, rows);
GETTER(Backtrace, count);
SETTER(Backtrace, count);

#endif // _UNIT_BASE__
