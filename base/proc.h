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

#ifndef _BASE_PROC__
#define _BASE_PROC__

#include <base/object.h>

typedef struct Process {
	Channel channel;
	Object (*task)(Channel channel);
	unsigned int process_priority;
	unsigned int state;
	unsigned long long epoch_cpu_time;
	ucontext_t context;
	void *stack_base;
	size_t stack_size;
	char name[32];
	struct Process *wait_process;
	OrbTreeNode cpu_time;
	OrbTreeNode lottery;
	struct Process *next_link;	// singly linked list for hashtable.
} Process;

#define PROC_TABLE_SIZE 32
typedef struct ProcTable {
	unsigned char impl[PROC_TABLE_SIZE];
} ProcTable;

Object proctable_add_process(ProcTable *table, Process *proc);

#endif	// _BASE_PROC__
