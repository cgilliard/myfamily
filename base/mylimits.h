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

// Custom limits for fixed-width integers
#ifndef UINT8_MAX
#define UINT8_MAX 255
#endif // UINT8_MAX

#ifndef UINT16_MAX
#define UINT16_MAX 65535
#endif // UINT16_MAX

#ifndef UINT32_MAX
#define UINT32_MAX 4294967295U
#endif // UINT32_MAX

#ifndef UINT64_MAX
#define UINT64_MAX 18446744073709551615ULL
#endif // UINT64_MAX

#ifndef INT8_MAX
#define INT8_MAX 127
#endif // INT8_MAX

#ifndef INT16_MAX
#define INT16_MAX 32767
#endif // INT16_MAX

#ifndef INT32_MAX
#define INT32_MAX 2147483647
#endif // INT32_MAX

#ifndef INT64_MAX
#define INT64_MAX 9223372036854775807LL
#endif // INT64_MAX

#ifndef INT8_MIN
#define INT8_MIN (-INT8_MAX - 1)
#endif // INT8_MIN

#ifndef INT16_MIN
#define INT16_MIN (-INT16_MAX - 1)
#endif // INT16_MIN

#ifndef INT32_MIN
#define INT32_MIN (-INT32_MAX - 1)
#endif // INT32_MIN

#ifndef INT64_MIN
#define INT64_MIN (-INT64_MAX - 1)
#endif // INT64_MIN

typedef __int128 myint128_t;
typedef unsigned __int128 myuint128_t;

// Define limits for these types (assuming they match 128-bit values)
#ifndef UINT128_MAX
#define UINT128_MAX ((myuint128_t)~0ULL >> 1) // 2^128 - 1
#endif										  // UINT128_MAX

#ifndef INT128_MAX
#define INT128_MAX ((myint128_t)(UINT128_MAX >> 1)) // 2^127 - 1
#endif												// INT128_MAX

#ifndef INT128_MIN
#define INT128_MIN ((myint128_t)(UINT128_MAX >> 1) + 1) // -2^127
#endif													// INT128_MIN
