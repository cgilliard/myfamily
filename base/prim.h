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

#ifndef _BASE_PRIM__
#define _BASE_PRIM__

#include <base/class.h>
#include <base/traits.h>

CLASS(U64, FIELD(u64, value))
IMPL(U64, TRAIT_COPY)
IMPL(U64, TRAIT_UNWRAP)
#define U64 DEFINE_CLASS(U64)
static GETTER(U64, value);

CLASS(I32, FIELD(i32, value))
IMPL(I32, TRAIT_COPY)
IMPL(I32, TRAIT_UNWRAP)
#define I32 DEFINE_CLASS(I32)
static GETTER(I32, value);

#define VALUE(x) _Generic((x), U64 *: U64_get_value, default: I32_get_value)(&x)

#endif // _BASE_PRIM__
