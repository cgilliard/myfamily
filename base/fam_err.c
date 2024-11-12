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
#include <base/print_util.h>
#include <base/util.h>

_Thread_local byte fam_err_last[ERR_LEN + 1] = {""};

_Thread_local int64 fam_err = NoErrors;

const byte *get_err() {
	copy_bytes(fam_err_last, FamErrText[fam_err], ERR_LEN);
	return fam_err_last;
}

void print_err(const byte *text) {
	println("%s: %s", FamErrText[fam_err], text);
}
