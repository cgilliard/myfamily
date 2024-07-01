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
#include <base/rc.h>
#include <base/unit.h>

GETTER(Option, ref)
SETTER(Option, ref)
GETTER(Option, no_cleanup)
SETTER(Option, no_cleanup)

Result Option_dbg(Option *option, Formatter *f) {
	void *ref = *Option_get_ref(option);

	if (!implements(ref, "dbg")) {
		Result r = WRITE(f, "Option<>");
		Try(Unit, r);
	} else {
		Result r0 = to_debug(ref);
		StringRef s0 = Try(StringRef, r0);
		Result r = WRITE(f, "Option[%s]", to_str(&s0));
		Try(Unit, r);
	}

	return Ok(UNIT);
}

void Option_cleanup(Option *option) {
	void *ref = *Option_get_ref(option);
	bool no_cleanup = *Option_get_no_cleanup(option);
	if (ref) {
		if (!no_cleanup) {
			cleanup(ref);
		}
		tlfree(ref);
		Option_set_ref(option, NULL);
	}
}
usize Option_size(Option *option) { return sizeof(Option); }
bool Option_clone(Option *dst, Option *src) {
	if (src->is_some()) {
		Object *ref = tlmalloc(size(src));
		if (!copy(ref, *Option_get_ref(src))) {
			tlfree(ref);
			return false;
		}
		Option_set_no_cleanup(dst, *Option_get_no_cleanup(src));
		Option_set_ref(dst, ref);
	} else {
		Option_set_ref(dst, NULL);
	}
	dst->is_some = src->is_some;

	return true;
}
Option Option_build(void *ref) {
	if (!implements(ref, "copy")) {
		panic("Copy must be implemented to build an option");
	}
	void *ref_copy = tlmalloc(size(ref));
	if (!ref_copy) {
		panic("Could not allocate memory to copy option");
	}
	if (!copy(ref_copy, ref)) {
		tlfree(ref_copy);
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
		ref = unwrap(ref);
	}

	// ownership is now transferred
	// we don't want to cleanup
	if (not_rc)
		Option_set_no_cleanup(option, true);

	return ref;
}

void *Option_unwrap_as(char *name, Option *option) {
	void *ret = unwrap(option);
	if (strcmp(CLASS_NAME(ret), name))
		panic("Expected class [%s], Found class [%s]", name,
		      CLASS_NAME(ret));
	return ret;
}
