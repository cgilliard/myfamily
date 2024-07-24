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

#ifndef _CORE_FORMAT__
#define _CORE_FORMAT__

#include <core/formatter.h>
#include <core/result.h>
#include <core/string.h>

Result format(FormatterPtr *formatter, char *fmt, ...);

#define PROC_ARG(value)                                                        \
	_Generic((value),                                                      \
	    __uint128_t: ({                                                    \
			 U128Ptr _vu128__ = BUILD(U128);                       \
			 memcpy(&(_vu128__._value), &(value), sizeof(u128));   \
			 _vu128__;                                             \
		 }),                                                           \
	    uint64_t: ({                                                       \
			 U64Ptr _vu64__ = BUILD(U64);                          \
			 memcpy(&(_vu64__._value), &(value), sizeof(u64));     \
			 _vu64__;                                              \
		 }),                                                           \
	    uint32_t: ({                                                       \
			 U32Ptr _vu32__ = BUILD(U32);                          \
			 memcpy(&(_vu32__._value), &(value), sizeof(u32));     \
			 _vu32__;                                              \
		 }),                                                           \
	    uint16_t: ({                                                       \
			 U16Ptr _vu16__ = BUILD(U16);                          \
			 memcpy(&(_vu16__._value), &(value), sizeof(u16));     \
			 _vu16__;                                              \
		 }),                                                           \
	    uint8_t: ({                                                        \
			 U8Ptr _vu8__ = BUILD(U8);                             \
			 memcpy(&(_vu8__._value), &(value), sizeof(u8));       \
			 _vu8__;                                               \
		 }),                                                           \
	    __int128_t: ({                                                     \
			 U128Ptr _vi128__ = BUILD(I128);                       \
			 memcpy(&(_vi128__._value), &(value), sizeof(i128));   \
			 _vi128__;                                             \
		 }),                                                           \
	    int64_t: ({                                                        \
			 U64Ptr _vi64__ = BUILD(I64);                          \
			 memcpy(&(_vi64__._value), &(value), sizeof(i64));     \
			 _vi64__;                                              \
		 }),                                                           \
	    int32_t: ({                                                        \
			 U32Ptr _vi32__ = BUILD(I32);                          \
			 memcpy(&(_vi32__._value), &(value), sizeof(i32));     \
			 _vi32__;                                              \
		 }),                                                           \
	    int16_t: ({                                                        \
			 U16Ptr _vi16__ = BUILD(I16);                          \
			 memcpy(&(_vi16__._value), &(value), sizeof(i16));     \
			 _vi16__;                                              \
		 }),                                                           \
	    int8_t: ({                                                         \
			 I8Ptr _vi8__ = BUILD(I8);                             \
			 memcpy(&(_vi8__._value), &(value), sizeof(i8));       \
			 _vi8__;                                               \
		 }),                                                           \
	    double: ({                                                         \
			 U64Ptr _vf64__ = BUILD(F64);                          \
			 memcpy(&(_vf64__._value), &(value), sizeof(f64));     \
			 _vf64__;                                              \
		 }),                                                           \
	    float: ({                                                          \
			 U32Ptr _vf32__ = BUILD(F32);                          \
			 memcpy(&(_vf32__._value), &(value), sizeof(f32));     \
			 _vf32__;                                              \
		 }),                                                           \
	    size_t: ({                                                         \
			 USizePtr _vu64__ = BUILD(USize);                      \
			 memcpy(&(_vu64__._value), &(value), sizeof(u64));     \
			 _vu64__;                                              \
		 }),                                                           \
	    intptr_t: ({                                                       \
			 ISizePtr _vu64__ = BUILD(ISize);                      \
			 memcpy(&(_vu64__._value), &(value), sizeof(u64));     \
			 _vu64__;                                              \
		 }),                                                           \
	    bool: ({                                                           \
			 BoolPtr _bool__ = BUILD(Bool);                        \
			 memcpy(&(_bool__._value), &(value), sizeof(bool));    \
			 _bool__;                                              \
		 }),                                                           \
	    default: ({ value; }))

#define FORMAT(f, fmt, ...)                                                    \
	format(f, fmt __VA_OPT__(, ) EXPAND(FOR_EACH(PROC_ARG, __VA_ARGS__)))

#endif // _CORE_FORMAT__
