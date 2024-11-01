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

ucontext_t uctx_main, uctx_return;

int context_execution_count = 0;
int context_next = 0;
void (*context_fn_array[10])();

#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

void init(void (*main)()) {
	char main_stack[16384];

	println("init");
	if (getcontext(&uctx_main)) {
		panic("getcontext");
	}

	uctx_main.uc_stack.ss_sp = main_stack;
	uctx_main.uc_stack.ss_size = sizeof(main_stack);
	uctx_main.uc_link = &uctx_return;
	makecontext(&uctx_main, main, 0);

	if (swapcontext(&uctx_return, &uctx_main)) {
		panic("swapcontext");
	}
	println("continue");

	while (context_next < context_execution_count) {
		if (getcontext(&uctx_main)) {
			panic("getcontext");
		}
		uctx_main.uc_stack.ss_sp = main_stack;
		uctx_main.uc_stack.ss_size = sizeof(main_stack);
		uctx_main.uc_link = &uctx_return;
		makecontext(&uctx_main, context_fn_array[context_next], 0);

		println("exe");
		if (swapcontext(&uctx_return, &uctx_main)) {
			panic("swapcontext");
		}
		println("exe complete");
		context_next++;
	}
}

ucontext_t uctx_spawn, uctx_spawn_return, uctx_spawn_finalize;

void spawn(void (*fn)()) {
	context_fn_array[context_execution_count] = fn;
	context_execution_count++;
}
