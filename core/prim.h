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

#ifndef _CORE_PRIM__
#define _CORE_PRIM__

#include <core/class.h>
#include <core/traits_base.h>

#define DEFINE_PRIM_CLASS(prim_type, type)                                     \
	CLASS(type, FIELD(prim_type, value))                                   \
	IMPL(type, TRAIT_COPY)                                                 \
	IMPL(type, TRAIT_TO_STR_BUF)                                           \
	IMPL(type, TRAIT_UNWRAP)

DEFINE_PRIM_CLASS(bool, Bool)
#define Bool DEFINE_CLASS(Bool)
static GETTER(Bool, value);

DEFINE_PRIM_CLASS(i128, I128)
#define I128 DEFINE_CLASS(I128)
static GETTER(I128, value);

DEFINE_PRIM_CLASS(i64, I64)
#define I64 DEFINE_CLASS(I64)
static GETTER(I64, value);

DEFINE_PRIM_CLASS(i32, I32)
#define I32 DEFINE_CLASS(I32)
static GETTER(I32, value);

DEFINE_PRIM_CLASS(i16, I16)
#define I16 DEFINE_CLASS(I16)
static GETTER(I16, value);

DEFINE_PRIM_CLASS(i8, I8)
#define I8 DEFINE_CLASS(I8)
static GETTER(I8, value);

DEFINE_PRIM_CLASS(u128, U128)
#define U128 DEFINE_CLASS(U128)
static GETTER(U128, value);

DEFINE_PRIM_CLASS(u64, U64)
#define U64 DEFINE_CLASS(U64)
static GETTER(U64, value);

DEFINE_PRIM_CLASS(u32, U32)
#define U32 DEFINE_CLASS(U32)
static GETTER(U32, value);

DEFINE_PRIM_CLASS(u16, U16)
#define U16 DEFINE_CLASS(U16)
static GETTER(U16, value);

DEFINE_PRIM_CLASS(u8, U8)
#define U8 DEFINE_CLASS(U8)
static GETTER(U8, value);

DEFINE_PRIM_CLASS(usize, USize)
#define USize DEFINE_CLASS(USize)
static GETTER(USize, value);

DEFINE_PRIM_CLASS(f32, F32)
#define F32 DEFINE_CLASS(F32)
static GETTER(F32, value);

DEFINE_PRIM_CLASS(f64, F64)
#define F64 DEFINE_CLASS(F64)
static GETTER(F64, value);

#endif // _CORE_PRIM__
