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

#ifndef _BASE_TASK__
#define _BASE_TASK__

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif
#include <base/channel.h>
#include <base/object.h>
#include <base/sys.h>
#include <base/types.h>
#include <ucontext.h>

#define INIT_TASK_TABLE                                           \
	({                                                            \
		TaskTable _ret__ = {};                                    \
		let _res__ = tasktable_init(&_ret__);                     \
		if ($is_err(_res__))                                      \
			panic("Could not initialize task table: {}", _res__); \
		_ret__;                                                   \
	})

typedef enum TaskState {
	Initializable,
	Running,
	Runnable,
	Sleeping,
} TaskState;

typedef struct Task {
	Channel id;
	void (*task)(void *channel);
	u32 state;
	size_t task_scheduler_index;
	void *stack_base;
	size_t stack_size;
	Channel wait_channel;
	ucontext_t uctx;
	struct Task *hash_list_next;
	struct Task *group;
} Task;

#define TASK_TABLE_SIZE 40
typedef struct TaskTable {
	byte impl[TASK_TABLE_SIZE];
} TaskTable;

Object tasktable_init(TaskTable *table);
Object tasktable_add_task(TaskTable *table, Task *task);
Task *tasktable_get_task(const TaskTable *table, Channel *channel);
Task *tasktable_remove_task(TaskTable *table, Channel *channel);
Task *tasktable_pick_random_runnable(TaskTable *table);
void tasktable_cleanup(TaskTable *table);

#endif	// _BASE_TASK__
