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

#include <core/prim.h>

#define DEFINE_PRIM(lower, upper, bits)                             \
	Type(upper##bits, Field(lower##bits, value));               \
	void upper##bits##_build(const upper##bits##Config* config) \
	{                                                           \
		$Var(value) = config->value;                        \
	}                                                           \
	void upper##bits##_value_of(void* buf)                      \
	{                                                           \
		*(lower##bits*)buf = $(value);                      \
	}

#define IMPL U8
DEFINE_PRIM(u, U, 8);
#undef IMPL

#define IMPL U16
DEFINE_PRIM(u, U, 16);
#undef IMPL

#define IMPL U32
DEFINE_PRIM(u, U, 32);
#undef IMPL

#define IMPL U64
DEFINE_PRIM(u, U, 64);
#undef IMPL

#define IMPL U128
DEFINE_PRIM(u, U, 128);
#undef IMPL

#define IMPL I8
DEFINE_PRIM(i, I, 8);
#undef IMPL

#define IMPL I16
DEFINE_PRIM(i, I, 16);
#undef IMPL

#define IMPL I32
DEFINE_PRIM(i, I, 32);
#undef IMPL

#define IMPL I64
DEFINE_PRIM(i, I, 64);
#undef IMPL

#define IMPL I128
DEFINE_PRIM(i, I, 128);
#undef IMPL

#define IMPL F32
DEFINE_PRIM(f, F, 32);
#undef IMPL

#define IMPL F64
DEFINE_PRIM(f, F, 64);
#undef IMPL

#define IMPL Bool
Type(Bool, Field(bool, value));
void Bool_build(const BoolConfig* config)
{
	$Var(value) = config->value;
}
void Bool_value_of(void* buf)
{
	*(bool*)buf = $(value);
}
#undef IMPL
