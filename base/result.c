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

#include <base/ekinds.h>
#include <base/panic.h>
#include <base/result.h>
#include <base/tlmalloc.h>

bool is_ok_false() { return false; }
bool is_ok_true() { return true; }

void result_free(ResultPtr *result) {
	if (result->err) {
		cleanup(result->err);
		tlfree(result->err);
		result->err = NULL;
	}
	if (result->ref) {
		if (!result->is_prim) {
			cleanup(result->ref);
		}
		tlfree(result->ref);
		result->ref = NULL;
	}
}
void *result_unwrap(Result *result) {
	if (result->is_ok()) {
		return result->ref;
	} else {
		panic("Attempt to unwrap on an Err");
	}
}

Error *result_unwrap_err(Result *result) {
	if (!result->is_ok()) {
		return result->err;
	} else {
		panic("Attempt to unwrap_err on an Ok");
	}
}
Result result_build_ok(void *ref) {
	printf("1\n");
	ResultPtr ret;
	ret.vtable = &ResultVtable;
	ret.is_ok = is_ok_true;
	ret.err = NULL;
	ret.ref = tlmalloc(size(ref));
	((Object *)ret.ref)->vtable = ((Object *)ref)->vtable;
	ret.is_prim = false;
	if (copy(ret.ref, ref)) {
		return ret;
	} else {
		// error copying return err
		Error err = ERROR(COPYING_ERROR, "could not copy to result");
		return Err(err);
	}
}

void result_init_prim_generic(ResultPtr *ptr, size_t size, void *ref) {
	ptr->vtable = &ResultVtable;
	ptr->prim_size = size;
	ptr->is_ok = is_ok_true;
	ptr->err = NULL;
	ptr->is_prim = true;
	ptr->ref = tlmalloc(size);
	memcpy(ptr->ref, ref, size);
}

Result result_build_err(Error err) {
	ResultPtr ret;
	ret.vtable = &ResultVtable;
	ret.is_ok = is_ok_false;
	ret.ref = NULL;
	ret.err = tlmalloc(size(&err));
	((Object *)ret.err)->vtable = err.vtable;

	// error copy always succeeds
	copy(ret.err, &err);

	return ret;
}

size_t result_size(Result *result) { return sizeof(Result); }
bool result_copy(Result *dst, Result *src) {
	bool ret = true;
	dst->vtable = &ResultVtable;
	dst->is_ok = src->is_ok;
	if (src->ref) {
		if (src->is_prim) {
			dst->ref = tlmalloc(src->prim_size);
			memcpy(dst->ref, src->ref, src->prim_size);
			dst->is_prim = true;
			dst->prim_size = src->prim_size;
		} else {
			dst->ref = tlmalloc(size(src->ref));
			dst->is_prim = false;
			((Object *)dst->ref)->vtable =
			    ((Object *)src->ref)->vtable;
			ret = copy(dst->ref, src->ref);
		}
	} else {
		dst->ref = NULL;
	}
	return ret;
}
