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
#include <base/rc.h>
#include <base/result.h>

GETTER(Result, err)
GETTER(Result, ref)
SETTER(Result, err)
SETTER(Result, ref)
GETTER(Result, no_cleanup)
SETTER(Result, no_cleanup)

bool is_ok_impl_true() { return true; }

bool is_ok_impl_false() { return false; }

usize Result_size(Result *ptr) { return sizeof(Result); }

bool Result_clone(Result *dst, Result *src) {
	Object *ref;
	void *src_ref = *Result_get_ref(src);
	if (src_ref) {
		ref = tlmalloc(size(src_ref));
		if (!ref)
			return false;
		if (!copy(ref, src_ref)) {
			tlfree(ref);
			return false;
		}

	} else {
		ref = NULL;
	}
	Result_set_ref(dst, ref);

	ErrorPtr *err;
	ErrorPtr *src_err = *Result_get_err(src);
	if (src_err) {
		err = tlmalloc(size(src_err));
		if (!err)
			return false;
		// copy error always returns true
		copy(err, src_err);
	} else {
		err = NULL;
	}
	Result_set_err(dst, err);

	Result_set_no_cleanup(dst, *Result_get_no_cleanup(src));
	dst->is_ok = src->is_ok;

	return true;
}

void Result_cleanup(Result *ptr) {
	void *ref = *Result_get_ref(ptr);
	void *err = *Result_get_err(ptr);
	bool no_cleanup = *Result_get_no_cleanup(ptr);
	if (ref) {
		if (!no_cleanup) {
			cleanup(ref);
		}
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
	if (!ref_copy) {
		Error e = ERROR(ALLOC_ERROR,
				"Could not allocate memory to copy result");
		return Err(e);
	}
	copy(ref_copy, ref);
	ResultPtr ret = BUILD(Result, is_ok_impl_false, ref_copy, NULL, false);
	return ret;
}

Result Result_build_ok(void *ref) {
	if (!implements(ref, "copy")) {
		Rc rc = RC(ref);
		return Result_build_ok(&rc);
	}

	void *ref_copy = tlmalloc(size(ref));
	if (!ref_copy) {
		Error e = ERROR(ALLOC_ERROR,
				"Could not allocate memory to copy result");
		return Err(e);
	}
	if (!copy(ref_copy, ref)) {
		Error e = ERROR(COPY_ERROR, "Error copying object");
		return Err(e);
	}
	ResultPtr ret = BUILD(Result, is_ok_impl_true, NULL, ref_copy, false);
	return ret;
}

void *Result_unwrap(Result *result) {
	if (!result->is_ok())
		panic("attempt to unwrap a Result that is an error");

	void *ref = *Result_get_ref(result);
	bool not_rc = true;

	char *cn = CLASS_NAME(ref);

	// automatically unwrap the primitive types
	if (!strcmp(cn, "Bool")) {
		ref = unwrap(ref);
	} else if (!strcmp(cn, "U8")) {
		ref = unwrap(ref);
	} else if (!strcmp(cn, "U16")) {
		ref = unwrap(ref);
	} else if (!strcmp(cn, "U32")) {
		ref = unwrap(ref);
	} else if (!strcmp(cn, "U64")) {
		ref = unwrap(ref);
	} else if (!strcmp(cn, "U128")) {
		ref = unwrap(ref);
	} else if (!strcmp(cn, "I128")) {
		ref = unwrap(ref);
	} else if (!strcmp(cn, "I64")) {
		ref = unwrap(ref);
	} else if (!strcmp(cn, "I32")) {
		ref = unwrap(ref);
	} else if (!strcmp(cn, "I16")) {
		ref = unwrap(ref);
	} else if (!strcmp(cn, "I8")) {
		ref = unwrap(ref);
	} else if (!strcmp(cn, "Rc")) {
		not_rc = false;
		u8 flags = *Rc_get_flags(ref);
		if ((flags & RC_FLAGS_NO_UNWRAP) == 0) {
			ref = unwrap(ref);
		}
	}

	// ownership is now transferred
	// we don't want to cleanup
	if (not_rc)
		Result_set_no_cleanup(result, true);

	return ref;
}

Error Result_unwrap_err(Result *result) {
	ErrorPtr ret;
	copy(&ret, *Result_get_err(result));
	return ret;
}
