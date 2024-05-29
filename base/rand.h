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

#ifndef _RAND_BASE__
#define _RAND_BASE__

#include <stdint.h>
#include <stdlib.h>

int rand_i32(int32_t *v);
int rand_u32(uint32_t *v);
int rand_i64(int64_t *v);
int rand_u64(uint64_t *v);
int rand_i128(__int128_t *v);
int rand_u128(__uint128_t *v);
int rand_bytes(void *buf, size_t length);

#endif /* _RAND_BASE__ */
