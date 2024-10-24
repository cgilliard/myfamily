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

#include <base/fam_err.h>
#include <base/macro_util.h>
#include <base/os.h>
#include <base/print_util.h>
#include <base/string.h>

_Thread_local u8 fam_err_last[ERR_LEN + 1] = {""};

_Thread_local i32 fam_err = NoErrors;
_Thread_local Backtrace thread_local_bt__;

const u8 *get_err() {
	// return mystrcpy(fam_err_last, FamErrText[fam_err], ERR_LEN);
	return NULL;
}

void print_err(const u8 *text) {
	// println("%s: %s", FamErrText[fam_err], text);
	// if (getenv("CBACKTRACE") != NULL) {
	//  backtrace_print(&thread_local_bt__);
	//} else {
	//	println("Backtrace currently disabled set env variable CBACKTRACE to enable");
	//}
}

void do_backtrace_generate(Backtrace *bt) {
	// if (getenv("CBACKTRACE") != NULL) {
	//  backtrace_generate(bt);
	//}
}
