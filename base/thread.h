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

#ifndef _BASE_THREAD__
#define _BASE_THREAD__

#include <base/types.h>

typedef struct ThreadConfig {
	int guard_size;
	void *stack_address;
	u64 stack_size;
} ThreadConfig;

static const ThreadConfig THREAD_CONFIG_DEFAULT = {
	.guard_size = -1,			   // Default of -1 indicates PAGE_SIZE.
	.stack_size = 2 * 1024 * 1024  // 2mb
};

typedef struct Thread Thread;

typedef void (*ThreadSignalHandler)();
typedef void *(*ThreadFunction)(void *arg);

Thread *thread_init(ThreadConfig *config);
void thread_cleanup(Thread *th);
int thread_start(Thread *th, ThreadFunction func, void *arg);
int thread_register_handler(ThreadSignalHandler handler);
int thread_signal(Thread *th);
void *thread_join(Thread *th);
u64 thread_id();

#endif	// _BASE_THREAD__
