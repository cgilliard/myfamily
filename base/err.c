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

#include <base/err.h>
#include <base/print_util.h>
#include <base/util.h>

_Thread_local char err_last[ERR_LEN + 1] = {""};

_Thread_local long long err = NoErrors;

const unsigned char *get_err() {
	int len = cstring_len(FamErrText[err]);
	if (len > ERR_LEN) len = ERR_LEN;
	copy_bytes(err_last, FamErrText[err], len);
	return err_last;
}

void print_err(const char *text) {
	// println("%s: %s", FamErrText[err], text);
}
