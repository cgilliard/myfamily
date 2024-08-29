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

#include <core/traits.h>

void drop(Object *self) {
	if (self->flags & OBJECT_FLAGS_CONSUMED)
		panic("Runtime error: Object [%s@%" PRIu64
		      "] has already been consumed!",
		      self->vtable->name, self->id);
	void (*impl)() = find_fn(self, "drop");
	if (!impl)
		panic("Runtime error: Trait bound violation! "
		      "Type "
		      "'%s' does "
		      "not implement the "
		      "required function [%s]",
		      TypeName((*self)), "drop");
	SelfCleanup sc = {__thread_local_self_Const, __thread_local_self_Var};
	__thread_local_self_Const = self;
	__thread_local_self_Var = self;

	// custom code here (set no cleanup since we're already done and consume
	// the object):
	self->flags |= OBJECT_FLAGS_NO_CLEANUP | OBJECT_FLAGS_CONSUMED;
	// free the pointer here as well since cleanup will not be called
	if (fat_ptr_data(&self->ptr)) {
		chain_free(&self->ptr);
	}
	return impl();
}
