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
#include <base/tlmalloc.h>
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

Error result_unwrap_err(Error *e, Result x) {
	if (x.is_ok()) {
		panic("attempt to unwrap_err on an Ok");
	} else {
		error_copy(e, x.error);
		x.error = NULL;
		return *e;
	}
}

Result result_build_ok(Result *res, Copy copy) {
	printf("result build ok\n");
	res->is_ok = result_is_ok_true;
	res->value.obj = tlmalloc(copy.size);
	res->error = NULL;
	copy.copy(res->value.obj, copy.obj);
	printf("result build ok compl\n");

	return *res;
}

Result result_build_err(Result *res, Error e) {
	va_list args;
	printf("build err\n");

	res->is_ok = result_is_ok_false;
	res->value.obj = NULL;
	res->error = tlmalloc(sizeof(Error));
	*(res->error) = verror_build(res->error, e.kind, e.msg, args);
	printf("berr complete\n");

	return *res;
}

void result_free(ResultImpl *ptr) {
	if (ptr->value.obj) {
		tlfree(ptr->value.obj);
		ptr->value.obj = NULL;
	}
	if (ptr->error) {
		printf("bt free %i\n", ptr);
		backtrace_free(&ptr->error->backtrace);
		tlfree(ptr->error);
		ptr->error = NULL;
	}
	// if (!ptr->is_ok()) {
	// printf("bt free %i\n", ptr);
	//  backtrace_free(&ptr->error.backtrace);
	// printf("ok\n");
	//}
}
