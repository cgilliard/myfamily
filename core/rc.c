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

#include <core/rc.h>

GETTER(Rc, count)
SETTER(Rc, count)
GETTER(Rc, ref)
SETTER(Rc, ref)

void Rc_cleanup(Rc *ptr) {
	Slab count = GET(Rc, ptr, count);
	Slab ref = GET(Rc, ptr, ref);
	u8 flags = GET(Rc, ptr, flags);
	if (count.data) {
		*(u64 *)count.data -= 1;

		if (*(u64 *)count.data == 0) {
			if ((flags & RC_FLAGS_NO_CLEANUP) == 0) {
				cleanup(ref.data);
			}
			myfree(&ref);
			myfree(&count);
		}
	}
}

bool Rc_myclone(Rc *dst, Rc *src) {
	Slab count = GET(Rc, src, count);
	u8 flags = GET(Rc, src, flags);
	if ((flags & RC_FLAGS_NO_COUNTER) == 0) {
		*(u64 *)count.data += 1;
	}
	SET(Rc, dst, ref, GET(Rc, src, ref));
	SET(Rc, dst, flags, GET(Rc, src, flags));
	SET(Rc, dst, count, count);

	return true;
}

Rc Rc_build(Slab slab) {
	Slab count;
	count.data = NULL;
	if (slab.data) {
		mymalloc(&count, sizeof(u64));
		u64 initial = 1;
		*(u64 *)count.data = initial;
	}
	RcPtr ret = BUILD(Rc, slab, count, 0);
	return ret;
}

void *Rc_unwrap(Rc *ptr) {
	Slab ref = GET(Rc, ptr, ref);
	u8 flags = GET(Rc, ptr, flags);
	// set no cleanup after unwrap is called
	SET(Rc, ptr, flags, flags | RC_FLAGS_NO_CLEANUP);
	if ((flags & RC_FLAGS_PRIM) != 0) {
		return unwrap(ref.data);
	}
	return ref.data;
}
