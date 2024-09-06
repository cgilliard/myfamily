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

#ifndef _CORE_TRAITS__
#define _CORE_TRAITS__

#include <core/type.h>

#define Drop DefineTrait(Drop, Required(Var, void, drop))
// Drop has a special TraitImpl because we need to prevent it from being called
// twice so it's not implemented using the standard TraitImpl.
// TraitImpl(Drop);
void drop(Obj* self);

#define Build DefineTrait(Build, Required(Var, void, build, SelfConfig()))
TraitImpl(Build);

#define Equal DefineTrait(Equal, Required(Const, bool, equal, Self()))
TraitImpl(Equal);

// Clone trait. We use 'klone' to prevent name resolution conflicts on Linux.
#define Clone DefineTrait(       \
    Clone,                       \
    Required(Const, Obj, klone), \
    RequiredWithDefault(clone_from_impl, Var, void, clone_from, Self()))
TraitImpl(Clone);

// Define a basic iterator trait. Currently there are no 'RequiredWithDefault' functions
// specified, we can add those as needed.
#define Iterator DefineTrait( \
    Iterator,                 \
    Required(Var, Obj, next))
TraitImpl(Iterator);

#endif // _CORE_TRAITS__
