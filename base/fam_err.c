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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __linux__
#define getenv(x) secure_getenv(x)
#endif // __linux__

char *FamErrText[FamErrCount] = {"NoErrors",
								 "IllegalArgument",
								 "AllocError",
								 "InitErr",
								 "AlreadyInitialized",
								 "IndexOutOfBounds",
								 "IllegalState",
								 "TooBig",
								 "ResourceNotAvailable",
								 "Permission",
								 "BackTraceErr"};

int fam_err = NoErrors;
_Thread_local char fam_err_last[ERR_LEN] = {""};
_Thread_local Backtrace thread_local_bt__;

const char *get_err() {
	strcpy(fam_err_last, FamErrText[fam_err]);
	return fam_err_last;
}

void print_err(const char *text) {
	fprintf(stderr, "%s: %s\n", FamErrText[fam_err], text);
	if (getenv("CBACKTRACE") != NULL) {
		backtrace_print(&thread_local_bt__);
	} else {
		printf("Backtrace currently disabled set env variable CBACKTRACE to enable\n");
	}
}
