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

#ifndef _BASE_RAND__
#define _BASE_RAND__

#include <base/types.h>
#include <stdlib.h>

/**
 * Returns a random i8 value using the operating systems' getentropy function.
 * @param v A pointer to the location to store the value in.
 * @return the value returned by the getentropy function.
 * @see [rand_u32]
 * @see [rand_bytes]
 */
int rand_i8(i8 *v);

/**
 * Returns a random u8 value using the operating systems' getentropy function.
 * @param v A pointer to the location to store the value in.
 * @return the value returned by the getentropy function.
 * @see [rand_u32]
 * @see [rand_bytes]
 */
int rand_u8(u8 *v);

/**
 * Returns a random i16 value using the operating systems' getentropy function.
 * @param v A pointer to the location to store the value in.
 * @return the value returned by the getentropy function.
 * @see [rand_u32]
 * @see [rand_bytes]
 */
int rand_i16(i16 *v);

/**
 * Returns a random u16 value using the operating systems' getentropy function.
 * @param v A pointer to the location to store the value in.
 * @return the value returned by the getentropy function.
 * @see [rand_u32]
 * @see [rand_bytes]
 */
int rand_u16(u16 *v);

/**
 * Returns a random i32 value using the operating systems' getentropy function.
 * @param v A pointer to the location to store the value in.
 * @return the value returned by the getentropy function.
 * @see [rand_u32]
 * @see [rand_bytes]
 */
int rand_i32(i32 *v);

/**
 * Returns a random u32 value using the operating systems' getentropy function.
 * @param v A pointer to the location to store the value in.
 * @return the value returned by the getentropy function.
 * @see [rand_i32]
 * @see [rand_bytes]
 */
int rand_u32(u32 *v);

/**
 * Returns a random i64 value using the operating systems' getentropy function.
 * @param v A pointer to the location to store the value in.
 * @return the value returned by the getentropy function.
 * @see [rand_u64]
 * @see [rand_bytes]
 */
int rand_i64(i64 *v);

/**
 * Returns a random u64 value using the operating systems' getentropy function.
 * @param v A pointer to the location to store the value in.
 * @return the value returned by the getentropy function.
 * @see [rand_i64]
 * @see [rand_bytes]
 */
int rand_u64(u64 *v);

/**
 * Returns a random i128 value using the operating systems' getentropy function.
 * @param v A pointer to the location to store the value in.
 * @return the value returned by the getentropy function.
 * @see [rand_u128]
 * @see [rand_bytes]
 */
int rand_i128(i128 *v);

/**
 * Returns a random u128 value using the operating systems' getentropy function.
 * @param v A pointer to the location to store the value in.
 * @return the value returned by the getentropy function.
 * @see [rand_i128]
 * @see [rand_bytes]
 */
int rand_u128(u128 *v);

/**
 * Returns random bytes of the specified length using the operating systems'
 * getentropy function.
 * @param buf A pointer to the location to store the value in.
 * @param length the number of bytes to write to the buffer.
 * @return the value returned by the getentropy function.
 * @see [rand_i64]
 * @see [rand_u128]
 */
int rand_bytes(void *buf, usize length);

#endif // _BASE_RAND__
