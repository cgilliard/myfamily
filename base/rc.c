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

#include <base/rc.h>

GETTER(Rc, count)
SETTER(Rc, count)
GETTER(Rc, ref)
SETTER(Rc, ref)

void Rc_cleanup(Rc *obj) {
	u64 *count = *Rc_get_count(obj);
	if (count) {
		*count -= 1;

		if (*count == 0) {
			void *ref = *Rc_get_ref(obj);
			cleanup(ref);
			tlfree(ref);
			tlfree(count);
			count = NULL;
		}
	}
}

usize Rc_size(Rc *obj) { return sizeof(Rc); }

bool Rc_clone(Rc *dst, Rc *src) {
	u64 *count = *Rc_get_count(src);
	void *ref = *Rc_get_ref(src);
	u8 flags = *Rc_get_flags(src);
	*count += 1;
	Rc_set_count(dst, count);
	Rc_set_ref(dst, ref);
	Rc_set_flags(dst, flags);

	return true;
}

Rc Rc_build(void *obj) {
	u64 *count = tlmalloc(sizeof(u64));
	*count = 1;
	RcPtr ret = BUILD(Rc, obj, count, 0);
	return ret;
}

void *Rc_unwrap(Rc *obj) {
	void *ref = *Rc_get_ref(obj);
	return ref;
}
