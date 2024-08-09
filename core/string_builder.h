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

#ifndef _CORE_STRING_BUILDER__
#define _CORE_STRING_BUILDER__

#include <core/class.h>
#include <core/result.h>
#include <core/traits.h>
#include <core/unit.h>

#define TRAIT_STRING_BUILDER_CORE(T)                                           \
	TRAIT_REQUIRED(T, Result, build, char *s)                              \
	TRAIT_REQUIRED(T, Result, to_string, T##Ptr *sb)

CLASS(StringBuilder, FIELD(Slab, slab) FIELD(u64, capacity))
IMPL(StringBuilder, TRAIT_STRING_BUILDER_CORE)
IMPL(StringBuilder, TRAIT_LEN)
IMPL(StringBuilder, TRAIT_APPEND)
#define StringBuilder DEFINE_CLASS(StringBuilder)

static GETTER(StringBuilder, capacity);
static GETTER(StringBuilder, slab);

#define STRING_BUILDER(x, b)                                                   \
	({                                                                     \
		Result __r__ = StringBuilder_build(x);                         \
		TRY(__r__, b);                                                 \
	})

#endif // _CORE_STRING_BUILDER__
