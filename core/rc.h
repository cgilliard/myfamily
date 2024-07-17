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

#ifndef _CORE_RC__
#define _CORE_RC__

#include <core/class.h>
#include <core/traits_base.h>

#define RC_FLAGS_NO_UNWRAP 0x1

#define TRAIT_RC_BUILD(T) TRAIT_REQUIRED(T, T##Ptr, build, void *ref)

CLASS(Rc, FIELD(void *, ref) FIELD(u64 *, count) FIELD(u8, flags))
IMPL(Rc, TRAIT_COPY)
IMPL(Rc, TRAIT_RC_BUILD)
IMPL(Rc, TRAIT_UNWRAP)
#define Rc DEFINE_CLASS(Rc)

static GETTER(Rc, flags);
static SETTER(Rc, flags);

#define RC(x) Rc_build(x)

#define DEREF(rc, clz) ((clz *)unwrap(&rc))

#define HEAPIFY_DIRECT(value)                                                  \
	({                                                                     \
		void *ret = mymalloc(size(&value));                            \
		memcpy(ret, &value, size(&value));                             \
		ret;                                                           \
	})

#define HEAPIFY(value)                                                         \
	({                                                                     \
		void *ptr = HEAPIFY_DIRECT(value);                             \
		RcPtr ret = RC(ptr);                                           \
		ret;                                                           \
	})

#endif // _CORE_RC__
