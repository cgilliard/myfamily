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

#include <base/cleanup.h>
#include <base/types.h>

#define BACKTRACE_MAX_DEPTH 100

typedef struct BacktraceEntry {
    char* function_name;
    char* bin_name;
    char* address;
    char* file_path;
} BacktraceEntry;

typedef struct BacktraceImpl {
    BacktraceEntry* rows;
    u64 count;
} BacktraceImpl;

void backtrace_free(BacktraceImpl* ptr);
#define Backtrace BacktraceImpl CLEANUP(backtrace_free)
#define EMPTY_BACKTRACE { NULL, 0 }
int backtrace_generate(Backtrace* ptr, u64 max_depth);
void backtrace_print(Backtrace* ptr);

String backtrace_to_string(String* s);

#endif // _BACKTRACE_BASE__
