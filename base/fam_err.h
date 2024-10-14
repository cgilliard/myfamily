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

#ifndef _BASE_FAM_ERR__
#define _BASE_FAM_ERR__

#include <base/backtrace.h>

#define ERR_LEN 1024

extern _Thread_local char fam_err_last[ERR_LEN];

typedef enum FamErr {
	NoErrors,
	IllegalArgument,
	AllocErr,
	InitErr,
	AlreadyInitialized,
	IndexOutOfBounds,
	IO,
	FileNotFound,
	NotADirectory,
	IllegalState,
	TooBig,
	ResourceNotAvailable,
	Permission,
	BackTraceErr,
	FamErrCount,
} FamErr;

extern char *FamErrText[FamErrCount];

extern int fam_err;
extern _Thread_local Backtrace thread_local_bt__;

void print_err(const char *text);
const char *get_err();
void do_backtrace_generate(Backtrace *bt);

#define SetErr(err)                                                                                \
	({                                                                                             \
		fam_err = err;                                                                             \
		do_backtrace_generate(&thread_local_bt__);                                                 \
	})

#endif // _BASE_FAM_ERR__
