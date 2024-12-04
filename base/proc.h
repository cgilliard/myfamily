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

#include <base/channel.h>
#include <base/object.h>
#include <base/sys.h>
#include <base/types.h>

#define INIT_PROC_TABLE                                           \
	({                                                            \
		ProcTable _ret__ = {};                                    \
		let _res__ = proctable_init(&_ret__);                     \
		if ($is_err(_res__))                                      \
			panic("Could not initialize proc table: {}", _res__); \
		_ret__;                                                   \
	})

typedef struct Process {
	Channel channel;
	Object (*task)(Channel channel);
	u32 process_priority;
	u32 state;
	u64 epoch_cpu_time;
	void *stack_base;
	size_t stack_size;
	u64 ticket_start;
	u64 ticket_len;
	OrbTreeNode cpu_time;
	OrbTreeNode lottery;
	struct Process *hash_list_next;
} Process;

#define PROC_TABLE_SIZE 32
typedef struct ProcTable {
	byte impl[PROC_TABLE_SIZE];
} ProcTable;

Object proctable_init(ProcTable *table);
Object proctable_add_process(ProcTable *table, Process *proc);
Process *proctable_get_process(const ProcTable *table, Channel channel);
Object proctable_remove_process(ProcTable *table, Channel channel);
Object proctable_reset_epoch(ProcTable *table);

#endif	// _BASE_PROC__
