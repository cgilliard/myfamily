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

#ifndef _CORE_UNIT__
#define _CORE_UNIT__

#include <core/class.h>
#include <core/traits_base.h>

CLASS(Unit, FIELD(bool, _dummy))
// CLASS(Unit)
IMPL(Unit, TRAIT_COPY)
IMPL(Unit, TRAIT_EQUAL)
#define Unit DEFINE_CLASS(Unit)
static Unit UNIT = BUILD(Unit);

#endif // _CORE_UNIT__
