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

#include <base/panic.h>
#include <base/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool result_is_ok_false() { return false; }

bool result_is_ok_true() { return true; }

void *result_unwrap(Result x) {
	if (!x.is_ok()) {
		panic("attempt to unwrap on an Err");
	} else {
		return x.value.obj;
	}
}

Error result_unwrap_err(Result x) {
	if (x.is_ok()) {
		panic("attempt to unwrap_err on an Ok");
	} else {
		return x.error;
	}
}

Result result_build_ok(Result *res, Copy copy) {
	res->is_ok = result_is_ok_true;
	res->value.obj = malloc(copy.size);
	copy.copy(res->value.obj, copy.obj);

	return *res;
}

Result result_build_err(Result *res, Error e) {
	va_list args;

	res->is_ok = result_is_ok_false;
	res->value.obj = NULL;
	res->error = verror_build(&res->error, e.kind, e.msg, args);

	return *res;
}

void result_free(ResultImpl *ptr) {
	if (ptr->value.obj) {
		free(ptr->value.obj);
		ptr->value.obj = NULL;
	}
}
