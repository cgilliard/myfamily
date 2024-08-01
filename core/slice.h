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

#ifndef _CORE_SLICE__
#define _CORE_SLICE__

#include <core/class.h>
#include <core/traits.h>

CLASS(Slice, FIELD(void *, ref) FIELD(u64, len))
IMPL(Slice, TRAIT_LEN)
#define Slice DEFINE_CLASS(Slice)

static GETTER(Slice, ref);
static SETTER(Slice, ref);

Slice Slice_build(void *ref, u64 size);

#define SLICE(ref, size) Slice_build(ref, size)

#endif // _CORE_SLICE__
