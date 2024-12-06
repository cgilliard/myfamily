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

#include <base/macro_util.h>
#include <base/orbtree.h>
#include <base/print_util.h>
#include <base/proc.h>

#define OFFSET_CPU_TIME (offsetof(Process, cpu_time))
#define OFFSET_LOTTERY (offsetof(Process, lottery))
#define PROC_TABLE_ARR_LEN 1024

typedef struct ProcTableImpl {
	Process *proc_arr;
	Process *proc_scheduler;
	Process *proc_scheduler_last;
	u64 proc_scheduler_capacity;
} ProcTableImpl;

void __attribute__((constructor)) __check_proc_table_sizes() {
	if (sizeof(ProcTableImpl) != sizeof(ProcTable))
		panic("sizeof(ProcTableImpl) ({}) != sizeof(ProcTable) ({})",
			  sizeof(ProcTableImpl), sizeof(ProcTable));
}

Object proctable_init(ProcTable *table) {
	u64 needed = sizeof(Process) * PROC_TABLE_ARR_LEN;
	u64 pages = 1 + (needed / PAGE_SIZE);
	ProcTableImpl *impl = (ProcTableImpl *)table;
	impl->proc_arr = map(pages);
	if (impl->proc_arr == NULL) return Err(AllocErr);
	return $(0);
}

Object proctable_add_process(ProcTable *table, Process *proc) {
	ProcTableImpl *impl = (ProcTableImpl *)table;
	u32 index = *(proc->id.value + 28) % PROC_TABLE_ARR_LEN;
	Process *slot = &impl->proc_arr[index];
	while (slot->task) slot = slot->hash_list_next;
	*slot = *proc;
	return $(0);
}

Process *proctable_remove_process(ProcTable *table, Channel *channel) {
	u32 index = *(channel->value + 28) % PROC_TABLE_ARR_LEN;
	ProcTableImpl *impl = (ProcTableImpl *)table;
	Process *slot = &impl->proc_arr[index];
	Process *prev = NULL;
	while (slot->task) {
		if (channel_equal(&slot->id, channel)) break;
		prev = slot;
		slot = slot->hash_list_next;
	}
	if (slot->task) {
		if (prev)
			prev->hash_list_next = slot->hash_list_next;
		else
			impl->proc_arr[index] = *(slot->hash_list_next);
		return slot;
	}
	return NULL;
}

Process *proctable_get_process(const ProcTable *table, Channel *channel) {
	u32 index = *(channel->value + 28) % PROC_TABLE_ARR_LEN;
	ProcTableImpl *impl = (ProcTableImpl *)table;
	Process *slot = &impl->proc_arr[index];
	while (slot->task) {
		if (channel_equal(&slot->id, channel)) return slot;
		slot = slot->hash_list_next;
	}
	return NULL;
}

Object proctable_reset_epoch(ProcTable *table) {
	return $(0);
}

void proctable_cleanup(ProcTable *table) {
	u64 needed = sizeof(Process) * PROC_TABLE_ARR_LEN;
	u64 pages = 1 + (needed / PAGE_SIZE);
	ProcTableImpl *impl = (ProcTableImpl *)table;
	if (impl->proc_arr) unmap(impl->proc_arr, pages);
}
