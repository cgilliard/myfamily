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

#ifndef _BASE_PANIC__
#define _BASE_PANIC__

#include <base/types.h>

typedef void __attribute__((noreturn)) (*panic_handler_t)(const char *msg);

#define THREAD_PANIC 1

void panic(const char *format, ...);
void set_on_panic(panic_handler_t on_panic);

#endif // _BASE_PANIC__
