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

#include <base/result.h>

GETTER(Result, err)
GETTER(Result, ref)
SETTER(Result, err)
SETTER(Result, ref)

bool is_ok_impl_true() { return true; }

bool is_ok_impl_false() { return false; }

void Result_cleanup(Result *ptr) {
	void *ref = *Result_get_ref(ptr);
	void *err = *Result_get_err(ptr);
	if (ref) {
		cleanup(ref);
		tlfree(ref);
		Result_set_ref(ptr, NULL);
	}
	if (err) {
		cleanup(err);
		tlfree(err);
		Result_set_err(ptr, NULL);
	}
}

Result Result_build_err(Error *ref) {
	void *ref_copy = tlmalloc(size(ref));
	copy(ref_copy, ref);
	ResultPtr ret = BUILD(Result, is_ok_impl_false, ref_copy, NULL);
	return ret;
}

Result Result_build_ok(void *ref) {
	void *ref_copy = tlmalloc(size(ref));
	copy(ref_copy, ref);
	ResultPtr ret = BUILD(Result, is_ok_impl_true, NULL, ref_copy);
	return ret;
}

void *Result_unwrap(Result *result) {
	if (!result->is_ok())
		panic("attempt to unwrap a Result that is an error");

	void *ref = *Result_get_ref(result);
	return ref;
}

ErrorPtr *Result_unwrap_err(Result *result) {
	if (result->is_ok())
		panic("attempt to unwrap_err a Result that is not an error");
	void *ref = *Result_get_err(result);
	return ref;
}
