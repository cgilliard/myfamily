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

#define Drop(T) Required(T, void, drop, Param(Object *, self))
TraitImpl(void, drop, Param(Object *, self));

// # desired syntax:
// #define Drop(T) Required(T, void, drop, Param(Object *, self))
// Trait(Drop);
// #define Clone(T) Required(T, Object, clone, Param(Object *, self))
// Trait(Clone);
// #define SuperClone(T) Required(T, Object, super_clone, Param(Object *, self))
// Where(SuperClone, TraitBound(self, Super), FunctionBound(clone, self, Clone))
// Trait(SuperClone);

#define Clone(T)                                                               \
	Required(T, bool, myclone, Param(Object *, dst), Param(Object *, self))
TraitImpl(bool, myclone, Param(Object *, dst), Param(Object *, self));

// is this possible?
// Bound(Clone, clone, (dst, Copy), (src, Drop))
// Then in .c file:
// TraitImpl(Clone) generates:
// void clone(Object *dst, Object *src) { // check bounds from static table and
// panic if not implemented. Then execute the trait. }

#endif // _CORE_TRAITS__
