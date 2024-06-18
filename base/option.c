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

#include <base/option.h>

GETTER(Option, ref)
SETTER(Option, ref)
GETTER(Option, no_cleanup)
SETTER(Option, no_cleanup)

void Option_cleanup(Option *option) {
	void *ref = *Option_get_ref(option);
	bool no_cleanup = *Option_get_no_cleanup(option);
	if (ref) {
		if (!no_cleanup)
			cleanup(ref);
		tlfree(ref);
		Option_set_ref(option, NULL);
	}
}
size_t Option_size(Option *option) { return sizeof(Option); }
bool Option_copy(Option *dst, Option *src) {
	dst->is_some = src->is_some;
	Option_set_ref(dst, *Option_get_ref(src));
	Option_set_no_cleanup(dst, *Option_get_no_cleanup(src));

	return true;
}
Option Option_build(void *ref) {
	void *ref_copy = tlmalloc(size(ref));
	if (!ref_copy) {
		panic("Could not allocate memory to copy option");
	}
	if (!copy(ref_copy, ref)) {
		panic("Error copying object");
	}
	OptionPtr ret = BUILD(Option, is_some_true, ref_copy, false);

	return ret;
}
void *Option_unwrap(Option *option) {
	if (!option->is_some()) {
		panic("attempt to unwrap an Option that is none");
	}
	void *ref = *Option_get_ref(option);

	// automatically unwrap the primitive types
	if (!strcmp(CLASS_NAME(ref), "Bool")) {
		ref = unwrap(ref);
	} else if (!strcmp(CLASS_NAME(ref), "U8")) {
		ref = unwrap(ref);
	} else if (!strcmp(CLASS_NAME(ref), "U16")) {
		ref = unwrap(ref);
	} else if (!strcmp(CLASS_NAME(ref), "U32")) {
		ref = unwrap(ref);
	} else if (!strcmp(CLASS_NAME(ref), "U64")) {
		ref = unwrap(ref);
	} else if (!strcmp(CLASS_NAME(ref), "U128")) {
		ref = unwrap(ref);
	} else if (!strcmp(CLASS_NAME(ref), "I128")) {
		ref = unwrap(ref);
	} else if (!strcmp(CLASS_NAME(ref), "I64")) {
		ref = unwrap(ref);
	} else if (!strcmp(CLASS_NAME(ref), "I32")) {
		ref = unwrap(ref);
	} else if (!strcmp(CLASS_NAME(ref), "I16")) {
		ref = unwrap(ref);
	} else if (!strcmp(CLASS_NAME(ref), "I8")) {
		ref = unwrap(ref);
	}

	// ownership is now transferred
	// we don't want to cleanup
	Option_set_no_cleanup(option, true);

	return ref;
}
