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

#ifndef _CORE_TEST_ENC__
#define _CORE_TEST_ENC__

#include <core/type.h>

#define HiddenApi DefineTrait(HiddenApi, Required(Const, u64, get_value), Required(Var, void, set_value, Param(u64)), Required(Const, u64, get_capacity_impl))
TraitImpl(HiddenApi);

TypeDef(Hidden, Config(u64, capacity));

Impl(Hidden, HiddenApi);
Impl(Hidden, Drop);
Impl(Hidden, Build);

#endif // _CORE_TEST_ENC__
