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

#ifndef _BASE_BACKTRACE__
#define _BASE_BACKTRACE__

#include <base/types.h>

#define MAX_ENTRIES 128
#define MAX_ENTRY_SIZE 1024

typedef struct BacktraceEntry {
	u16 start_bin;
	u16 start_addr;
	u16 start_file_path;
	u8 data[MAX_ENTRY_SIZE + 1];
} BacktraceEntry;

typedef struct Backtrace {
	u8 cur_entries;
	BacktraceEntry entries[MAX_ENTRIES];
} Backtrace;

i32 backtrace_generate(Backtrace *ptr);
void backtrace_print(const Backtrace *ptr);

#endif // _BASE_BACKTRACE__
