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
#include <base/panic.h>
#include <base/tlmalloc.h>
#include <string.h>

void option_free(OptionPtr *ptr) {
	if (ptr->ref != NULL) {
		if (!ptr->is_prim)
			cleanup(ptr->ref);
		tlfree(ptr->ref);
		ptr->ref = NULL;
	}
}
void *option_unwrap(Option *option) {
	if (!option->is_some()) {
		panic("attempt to unwrap on a None");
	}
	return option->ref;
}
Option option_build(void *ref) {
	void *ref_copy = tlmalloc(size(ref));
	OptionPtr ret;
	ret.vtable = &OptionVtable;
	ret.is_some = option_is_some_true;
	ret.ref = ref_copy;
	ret.is_prim = false;
	((Object *)ref_copy)->vtable = ((Object *)ref)->vtable;
	copy(ref_copy, ref);
	return ret;
}
size_t option_size(Option *option) { return sizeof(Option); }
bool option_copy(Option *dst, Option *src) {
	bool ret = true;
	dst->vtable = &OptionVtable;
	dst->is_some = src->is_some;
	if (src->ref) {
		size_t sz;
		if (src->is_prim)
			sz = src->prim_size;
		else
			sz = size(src->ref);
		dst->ref = tlmalloc(sz);
		if (!src->is_prim) {
			((Object *)dst->ref)->vtable =
			    ((Object *)src->ref)->vtable;
			ret = copy(dst->ref, src->ref);
		} else {
			memcpy(dst->ref, src->ref, sz);
			dst->is_prim = true;
		}
	} else {
		dst->ref = NULL;
	}
	return ret;
}

void option_init_prim_generic(OptionPtr *ptr, size_t size, void *ref) {
	ptr->vtable = &OptionVtable;
	ptr->is_some = option_is_some_true;
	ptr->is_prim = true;
	ptr->prim_size = size;
	ptr->ref = tlmalloc(size);
	memcpy(ptr->ref, ref, size);
}
