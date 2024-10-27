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

#ifndef _BASE_TYPES__
#define _BASE_TYPES__

#include <base/limits.h>

// primitives
#define number long long
#define byte unsigned char
#define dec double

// booleans
#define bool _Bool
#define true (_Bool)1
#define false (_Bool)0

// atomics
typedef _Atomic long long anum;
typedef _Atomic _Bool abool;

// NULL
#define NULL ((void *)0)

#endif // _BASE_TYPES__
