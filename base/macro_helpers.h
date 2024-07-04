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

#define EXPAND(x) x
#define CATI(x, y) x##y
#define CAT(x, y) CATI(x, y)

#define SIZE___int128_t 16
#define SIZE_int64_t 8
#define SIZE_int32_t 4
#define SIZE_int16_t 2
#define SIZE_int8_t 1
#define SIZE___uint128_t 16
#define SIZE_uint64_t 8
#define SIZE_uint32_t 4
#define SIZE_uint16_t 2
#define SIZE_uint8_t 1
#define SIZE__Bool 1
#define SIZE_size_t 8

#define SIZE_OF(type) SIZE_##type
