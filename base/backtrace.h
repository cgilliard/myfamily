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
#define _BACKTRACE_BASE__

#include <base/types.h>

typedef struct BacktraceEntry {
	char *function_name;
	char *bin_name;
	char *address;
	char *file_path;
} BacktraceEntry;

typedef struct BacktracePtr {
	BacktraceEntry *rows;
	u64 count;
} BacktracePtr;

void backtrace_free(BacktracePtr *ptr);
#define Backtrace BacktracePtr CLEANUP(backtrace_free)
#define EMPTY_BACKTRACE {NULL, 0}
Backtrace backtrace_generate(u64 max_depth);

// #define ERROR_PRINT_FLAG_NO_COLOR 0x1
void backtrace_print(Backtrace *ptr, int flags);
void backtrace_copy(Backtrace *dst, Backtrace *src);

#endif // _BACKTRACE_BASE__

