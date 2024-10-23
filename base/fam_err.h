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
#include <base/macro_util.h>

#define ERR_LEN 64

#define ENUM_TEXT(none, x) x
#define SECOND_STRINGIFY(x, y) #y

#define DEFINE_FAMERR(e)                                                                           \
	typedef enum FamErr { FOR_EACH(ENUM_TEXT, none, (, ), e) } FamErr;                             \
	static const u8 *FamErrText[] = {FOR_EACH(SECOND_STRINGIFY, none, (, ), e)};

// Define FamErr enum values
// clang-format off
#define FAMERR_VALUES                                                                              \
	NoErrors, \
	IllegalArgument, \
	Overflow, \
	llocErr, \
	InitErr, \
	AlreadyInitialized, \
	IndexOutOfBounds, \
	IO, \
	FileNotFound, \
	NotADirectory, \
	IllegalState, \
	TooBig, \
	ResourceNotAvailable, \
	Permission, \
	BackTraceErr, \
	ExpectedTypeMismatch
// clang-format on

DEFINE_FAMERR(FAMERR_VALUES);

extern _Thread_local u8 fam_err_last[ERR_LEN + 1];
extern _Thread_local i32 fam_err;
extern _Thread_local Backtrace thread_local_bt__;

void print_err(const u8 *text);
const u8 *get_err();
void do_backtrace_generate(Backtrace *bt);

#define SetErr(err)                                                                                \
	({                                                                                             \
		fam_err = err;                                                                             \
		do_backtrace_generate(&thread_local_bt__);                                                 \
	})

#endif // _BASE_FAM_ERR__
