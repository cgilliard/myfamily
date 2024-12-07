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

#include <base/cpsrng.h>
#include <base/lock.h>
#include <base/macro_util.h>
#include <base/orbtree.h>
#include <base/print_util.h>
#include <base/task.h>

#define OFFSET_CPU_TIME (offsetof(Task, cpu_time))
#define OFFSET_LOTTERY (offsetof(Task, lottery))
#define TASK_TABLE_ARR_LEN 1024

typedef struct TaskTableImpl {
	Task *task_arr;
	Task **task_scheduler;
	Task **task_scheduler_last;
	u64 task_scheduler_capacity;
	Lock lock;
} TaskTableImpl;

void __attribute__((constructor)) __check_task_table_sizes() {
	if (sizeof(TaskTableImpl) != sizeof(TaskTable))
		panic("sizeof(TaskTableImpl) ({}) != sizeof(TaskTable) ({})",
			  sizeof(TaskTableImpl), sizeof(TaskTable));
}

Object tasktable_init(TaskTable *table) {
	u64 needed = sizeof(Task) * TASK_TABLE_ARR_LEN;
	u64 pages = 1 + (needed / PAGE_SIZE);
	TaskTableImpl *impl = (TaskTableImpl *)table;
	impl->task_arr = map(pages);
	if (impl->task_arr == NULL) return Err(AllocErr);

	needed = sizeof(Task *) * TASK_TABLE_ARR_LEN;
	pages = 1 + (needed / PAGE_SIZE);
	impl->task_scheduler = map(pages);

	if (impl->task_scheduler == NULL) {
		needed = sizeof(Task) * TASK_TABLE_ARR_LEN;
		pages = 1 + (needed / PAGE_SIZE);
		unmap(impl->task_arr, pages);
		return Err(AllocErr);
	}

	impl->task_scheduler_capacity = TASK_TABLE_ARR_LEN;
	impl->task_scheduler_last = &impl->task_scheduler[0];
	impl->lock = INIT_LOCK;
	return $(0);
}

Object tasktable_add_task(TaskTable *table, Task *task) {
	TaskTableImpl *impl = (TaskTableImpl *)table;
	u32 index = *(task->id.value + 28) % TASK_TABLE_ARR_LEN;
	Task *slot = &impl->task_arr[index];
	while (slot->task) slot = slot->hash_list_next;
	*slot = *task;
	*impl->task_scheduler_last = slot;
	task->task_scheduler_index =
		impl->task_scheduler_last - impl->task_scheduler;
	impl->task_scheduler_last++;

	return $(0);
}

Task *tasktable_remove_task(TaskTable *table, Channel *channel) {
	u32 index = *(channel->value + 28) % TASK_TABLE_ARR_LEN;
	TaskTableImpl *impl = (TaskTableImpl *)table;
	Task *slot = &impl->task_arr[index];
	Task *prev = NULL;

	while (slot->task) {
		if (channel_equal(&slot->id, channel)) break;
		prev = slot;
		slot = slot->hash_list_next;
	}
	if (slot->task) {
		if (prev)
			prev->hash_list_next = slot->hash_list_next;
		else
			impl->task_arr[index] = *(slot->hash_list_next);
		return slot;
	}

	Task **ptr = (impl->task_scheduler + slot->task_scheduler_index);
	*ptr = *(--impl->task_scheduler_last);
	*(impl->task_scheduler_last + 1) = NULL;

	return slot;
}

Task *tasktable_get_task(const TaskTable *table, Channel *channel) {
	u32 index = *(channel->value + 28) % TASK_TABLE_ARR_LEN;
	TaskTableImpl *impl = (TaskTableImpl *)table;
	Task *slot = &impl->task_arr[index];
	while (slot->task) {
		if (channel_equal(&slot->id, channel)) return slot;
		slot = slot->hash_list_next;
	}
	return NULL;
}

Task *tasktable_pick_random_runnable(TaskTable *table) {
	TaskTableImpl *impl = (TaskTableImpl *)table;
	u64 task_count = impl->task_scheduler_last - impl->task_scheduler;
	u64 v;
	cpsrng_rand_i64(&v);
	if (task_count > 1)
		v = 1 + (v % (task_count - 1));
	else
		v = 0;
	Task *r = impl->task_scheduler[v];
	return r;
}

void tasktable_cleanup(TaskTable *table) {
	u64 needed = sizeof(Task) * TASK_TABLE_ARR_LEN;
	u64 pages = 1 + (needed / PAGE_SIZE);
	TaskTableImpl *impl = (TaskTableImpl *)table;
	if (impl->task_arr) unmap(impl->task_arr, pages);

	needed = sizeof(Task *) * TASK_TABLE_ARR_LEN;
	pages = 1 + (needed / PAGE_SIZE);
	if (impl->task_scheduler) unmap(impl->task_scheduler, pages);
}
