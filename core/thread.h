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

#ifndef _CORE_THREAD__
#define _CORE_THREAD__

#include <core/types.h>

typedef struct JoinResult
{
	bool is_panic;
	bool is_error;
} JoinResult;

typedef struct ThreadImpl ThreadImpl;

typedef struct ThreadPtr
{
	FatPtr impl;
} ThreadPtr;

void Thread_cleanup(ThreadPtr *ptr);

#define Thread \
	ThreadPtr __attribute__((warn_unused_result, cleanup(Thread_cleanup)))

int Thread_start(Thread *ptr, void (*start_routine)(void *), void *args);
u64 Thread_id(Thread *ptr);
JoinResult Thread_join(Thread *ptr);

bool JoinResult_is_panic(JoinResult *jr);
bool JoinResult_is_error(JoinResult *jr);

#endif // _CORE_THREAD__
