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

#include <base/class.h>

/*
static char *TraitNames[3] = {"copy", "size", "cleanup"};

Trait Traits[3] = {{"copy", "bool copy(Object *dst, Object *src);"},
		   {"size", "size_t size(Object *obj);"},
		   {"cleanup", "void cleanup(Object *obj);"}};

#define DEFINE_RESULT_BUILD(type, type_name, i)                                \
	static int result2_build_ok_##type_name(void *ref) {                   \
		int ret = 0;                                                   \
		return ret;                                                    \
	}

DEFINE_RESULT_BUILD(u64, u64, 0)
*/

// CLASS(MyClass)
// IMPL(MyClass, TRAIT_COPY)
