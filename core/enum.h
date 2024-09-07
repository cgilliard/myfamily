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

#ifndef _CORE_ENUM__
#define _CORE_ENUM__

#include <core/traits.h>
#include <core/type.h>

#define Enum(name, ...)                                                                                           \
	TypeImpl(name, Where(T), Field(u32, variant_id), Generic(T, obj));                                        \
	Impl(name, EnumProperties);                                                                               \
	u64 name##_sizeof_variant(u32 variant_id)                                                                 \
	{                                                                                                         \
		/* TODO: This is just hard coded. Need to modify this to set appropriate size per variant. */     \
		/* Basic approach FOR_EACH go through variants passed in. For each one either call size if it */  \
		/* doesn't implement EnumProperties or sizeof_max_variant for those that do implement.*/          \
		return 128 + sizeof(u32);                                                                         \
	}                                                                                                         \
	u32 name##_variant_id() { return $Context($(), name, variant_id); }                                       \
	void __attribute__((constructor)) __add_enum_##name()                                                     \
	{                                                                                                         \
		vtable_add_trait(&name##_Vtable__, "EnumProperties");                                             \
	}                                                                                                         \
	u64 name##_size()                                                                                         \
	{                                                                                                         \
		if (!Implements((*$()), "EnumProperties"))                                                        \
			panic("required trait [EnumProperties] not implemented in type [%s].", TypeName((*$()))); \
		u32 id = variant_id($());                                                                         \
		u64 size = sizeof_variant($(), id);                                                               \
		return size;                                                                                      \
	}
#define EnumBuilder(name)
#define match(e)
#define _()

#endif // _CORE_ENUM__
