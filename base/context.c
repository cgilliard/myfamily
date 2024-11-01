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

#define _XOPEN_SOURCE

#include <base/context.h>
#include <base/osdef.h>
#include <base/print_util.h>
#include <ucontext.h>

ucontext_t uctx_func1;

#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

void init() {
	printf("init\n");
	if (getcontext(&uctx_func1)) {
		panic("getcontext");
	}
}
