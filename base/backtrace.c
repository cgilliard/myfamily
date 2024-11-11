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

#include <base/backtrace.h>

#define MAX_ENTRIES 128
#define MAX_ENTRY_SIZE 1024

typedef struct BacktraceEntry {
	unsigned int start_bin;
	unsigned int start_addr;
	unsigned int start_file_path;
	unsigned int start_line_num;
	byte data[MAX_ENTRY_SIZE + 1];
} BacktraceEntry;

#define BACKTRACE_IMPL_SIZE 1024
typedef struct BacktraceImpl {
	int count;
} BacktraceImpl;
