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

#include <core/panic.h>
#include <core/traits.h>
#include <core/type.h>

/*
void drop(Object *obj) {
	void (*impl)(Object *obj) = find_fn(obj, "drop");
	if (!impl)
		panic("Drop not implemented");
	return impl(obj);
}
*/

/*
bool clone(Object *dst, Object *src) {
	// BoundsCheck b1 = {"dst", dst};
	// BoundsCheck b2 = {"src", src};
	// check_bounds("Clone", "clone", b1, b2);
	bool (*impl)(Object *dst, Object *src) = find_fn(src, "clone");

	if (!impl)
		panic("Not found");

	return impl(dst, src);
}
*/
