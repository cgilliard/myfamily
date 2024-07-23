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
	    u128: ({                                                           \
			 U128Ptr _vu128__ = BUILD(U128);                       \
			 memcpy(&(_vu128__._value), &(value), sizeof(u128));   \
			 _vu128__;                                             \
		 }),                                                           \
	    u64: ({                                                            \
			 U64Ptr _vu64__ = BUILD(U64);                          \
			 memcpy(&(_vu64__._value), &(value), sizeof(u64));     \
			 _vu64__;                                              \
		 }),                                                           \
	    u32: ({                                                            \
			 U32Ptr _vu32__ = BUILD(U32);                          \
			 memcpy(&(_vu32__._value), &(value), sizeof(u32));     \
			 _vu32__;                                              \
		 }),                                                           \
	    u16: ({                                                            \
			 U16Ptr _vu16__ = BUILD(U16);                          \
			 memcpy(&(_vu16__._value), &(value), sizeof(u16));     \
			 _vu16__;                                              \
		 }),                                                           \
	    u8: ({                                                             \
			 U8Ptr _vu8__ = BUILD(U8);                             \
			 memcpy(&(_vu8__._value), &(value), sizeof(u8));       \
			 _vu8__;                                               \
		 }),                                                           \
	    i128: ({                                                           \
			 U128Ptr _vi128__ = BUILD(I128);                       \
			 memcpy(&(_vi128__._value), &(value), sizeof(i128));   \
			 _vi128__;                                             \
		 }),                                                           \
	    i64: ({                                                            \
			 U64Ptr _vi64__ = BUILD(I64);                          \
			 memcpy(&(_vi64__._value), &(value), sizeof(i64));     \
			 _vi64__;                                              \
		 }),                                                           \
	    i32: ({                                                            \
			 U32Ptr _vi32__ = BUILD(I32);                          \
			 memcpy(&(_vi32__._value), &(value), sizeof(i32));     \
			 _vi32__;                                              \
		 }),                                                           \
	    i16: ({                                                            \
			 U16Ptr _vi16__ = BUILD(I16);                          \
			 memcpy(&(_vi16__._value), &(value), sizeof(i16));     \
			 _vi16__;                                              \
		 }),                                                           \
	    i8: ({                                                             \
			 I8Ptr _vi8__ = BUILD(I8);                             \
			 memcpy(&(_vi8__._value), &(value), sizeof(i8));       \
			 _vi8__;                                               \
		 }),                                                           \
	    f64: ({                                                            \
			 U64Ptr _vf64__ = BUILD(F64);                          \
			 memcpy(&(_vf64__._value), &(value), sizeof(f64));     \
			 _vf64__;                                              \
		 }),                                                           \
	    f32: ({                                                            \
			 U32Ptr _vf32__ = BUILD(F32);                          \
			 memcpy(&(_vf32__._value), &(value), sizeof(f32));     \
			 _vf32__;                                              \
		 }),                                                           \
	    usize: ({                                                          \
			 USizePtr _vusize__ = BUILD(USize);                    \
			 memcpy(&(_vusize__._value), &(value), sizeof(usize)); \
			 _vusize__;                                            \
		 }),                                                           \
	    isize: ({                                                          \
			 ISizePtr _visize__ = BUILD(ISize);                    \
			 memcpy(&(_visize__._value), &(value), sizeof(isize)); \
			 _visize__;                                            \
		 }),                                                           \
	    bool: ({                                                           \
			 BoolPtr _bool__ = BUILD(Bool);                        \
			 memcpy(&(_bool__._value), &(value), sizeof(bool));    \
			 _bool__;                                              \
		 }),                                                           \
	    default: ({ value; }))

#define FORMAT_NO_ARGS(f, fmt) format(f, fmt)
#define FORMAT_ARGS(f, fmt, ...)                                               \
	format(f, fmt, EXPAND(FOR_EACH(PROC_ARG, __VA_ARGS__)))
/*
#define FORMAT(f, fmt, ...)                                                    \
	IF_ELSE(HAS_ARGS(__VA_ARGS__))                                         \
	(FORMAT_ARGS(f, fmt, __VA_ARGS__)), (FORMAT_NO_ARGS(f, fmt))
	*/

#define FORMAT(f, fmt, ...)                                                    \
	format(f, fmt __VA_OPT__(, ) EXPAND(FOR_EACH(PROC_ARG, __VA_ARGS__)))

#endif // _CORE_FORMAT__
