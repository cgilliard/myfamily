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

#include <core/traits.h>
#include <core/type.h>

#define DEFINE_PRIM_HEADER(lower, upper, bits)            \
	Builder(upper##bits, Config(lower##bits, value)); \
	Impl(upper##bits, Build);                         \
	Impl(upper##bits, ValueOf);                       \
	Impl(upper##bits, Equal);

DEFINE_PRIM_HEADER(u, U, 8);
DEFINE_PRIM_HEADER(u, U, 16);
DEFINE_PRIM_HEADER(u, U, 32);
DEFINE_PRIM_HEADER(u, U, 64);
DEFINE_PRIM_HEADER(u, U, 128);
DEFINE_PRIM_HEADER(i, I, 8);
DEFINE_PRIM_HEADER(i, I, 16);
DEFINE_PRIM_HEADER(i, I, 32);
DEFINE_PRIM_HEADER(i, I, 64);
DEFINE_PRIM_HEADER(i, I, 128);
DEFINE_PRIM_HEADER(f, F, 32);
DEFINE_PRIM_HEADER(f, F, 64);

Builder(Bool, Config(bool, value));
Impl(Bool, Build);
Impl(Bool, ValueOf);
Impl(Bool, Equal);

#define DISABLE_WARNING _Pragma("GCC diagnostic push")                 \
    _Pragma("GCC diagnostic ignored \"-Wincompatible-pointer-types\"") \
	_Pragma("GCC diagnostic ignored \"-Wpointer-sign\"")
#define ENABLE_WARNING _Pragma("GCC diagnostic pop")

#define Box2(v) _Generic((v), \
    u8: ({ DISABLE_WARNING; u8 * vptr = &v; const Obj _ret____ = new (U8, With(value, *vptr)); ENABLE_WARNING; _ret____; }),                \
    u16: ({ DISABLE_WARNING; u16 * vptr = &v; const Obj _ret___ = new (U16, With(value, *vptr)); ENABLE_WARNING; _ret___; }),               \
    u32: ({ DISABLE_WARNING; u32 * vptr = &v; const Obj _ret___ = new (U32, With(value, *vptr)); ENABLE_WARNING; _ret___; }),               \
    u64: ({ DISABLE_WARNING; u64 * vptr = &v; const Obj _ret___ = new (U64, With(value, *vptr)); ENABLE_WARNING; _ret___; }),               \
    u128: ({ DISABLE_WARNING; u128 * vptr = &v; const Obj _ret__ = new (U128, With(value, *vptr)); ENABLE_WARNING; _ret__; }),              \
    i8: ({ DISABLE_WARNING; i8 * vptr = &v; const Obj _ret____ = new (I8, With(value, *vptr)); ENABLE_WARNING; _ret____; }),                \
    i16: ({ DISABLE_WARNING; i16 * vptr = &v; const Obj _ret___ = new (I16, With(value, *vptr)); ENABLE_WARNING; _ret___; }),               \
    i32: ({ DISABLE_WARNING; i32 * vptr = &v; const Obj _ret___ = new (I32, With(value, *vptr)); ENABLE_WARNING; _ret___; }),               \
    i64: ({ DISABLE_WARNING; i64 * vptr = &v; const Obj _ret___ = new (I64, With(value, *vptr)); ENABLE_WARNING; _ret___; }),               \
    i128: ({ DISABLE_WARNING; i128 * vptr = &v; const Obj _ret__ = new (I128, With(value, *vptr)); ENABLE_WARNING; _ret__; }),              \
    f32: ({ DISABLE_WARNING; f32 * vptr = &v; const Obj _ret___ = new (F32, With(value, *vptr)); ENABLE_WARNING; _ret___; }),               \
    f64: ({ DISABLE_WARNING; f64 * vptr = &v; const Obj _ret___ = new (F64, With(value, *vptr)); ENABLE_WARNING; _ret___; }),               \
    bool: ({ DISABLE_WARNING; bool * vptr = &v; const Obj _ret__ = new (Bool, With(value, *vptr)); ENABLE_WARNING; _ret__; }),              \
    Obj: ({ v; }),            \
    default: ({ panic("Attempt to box an unknown type!"); }))

#define Box(v) _Generic((v),          \
    u8: new (U8, With(value, v)),     \
    u16: new (U16, With(value, v)),   \
    u32: new (U32, With(value, v)),   \
    u64: new (U64, With(value, v)),   \
    u128: new (U128, With(value, v)), \
    i8: new (I8, With(value, v)),     \
    i16: new (I16, With(value, v)),   \
    i32: new (I32, With(value, v)),   \
    i64: new (I64, With(value, v)),   \
    i128: new (I128, With(value, v)), \
    f32: new (F32, With(value, v)),   \
    f64: new (F64, With(value, v)),   \
    bool: new (Bool, With(value, v)), \
    default: panic("Attempt to box an unknown type!"))

#define Unbox(v, to) value_of(&v, &to)

#endif // _CORE_PRIM__