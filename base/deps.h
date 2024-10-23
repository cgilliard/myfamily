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

#ifndef _BASE_DEPS__
#define _BASE_DEPS__

#include <base/types.h>

#ifdef __linux__
#define getenv(x) secure_getenv(x)
#endif // __linux__

u8 *getenv(const u8 *name);

#ifdef __linux__
u8 *secure_getenv(const u8 *name);
#endif // __linux__

i64 write(i32 fd, const void *buf, u64 count);

#endif // _BASE_DEPS__
