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

#ifndef _BASE_MYSTDINT__
#define _BASE_MYSTDINT__

// Include standard int if specified
#ifdef USE_STDINT
#include <stdint.h>
#endif // USE_STDINT

#ifndef UINT8_T
typedef unsigned char u8; // 8 bits
#endif

#ifndef UINT16_T
typedef unsigned short u16; // 16 bits
#endif

#ifndef UINT32_T
typedef unsigned int u32; // 32 bits
#endif

#ifndef UINT64_T
typedef unsigned long long u64; // 64 bits
#endif

#ifndef INT8_T
typedef signed char i8; // 8 bits
#endif

#ifndef INT16_T
typedef signed short i16; // 16 bits
#endif

#ifndef INT32_T
typedef signed int i32; // 32 bits
#endif

#ifndef INT64_T
typedef signed long long i64; // 64 bits
#endif

#endif // _BASE_MYSTDINT__
