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
	OrbTree lottery_tree;
	OrbTree cpu_time_tree;
	Process *proc_arr;
	byte padding[8];
} ProcTableImpl;

void __attribute__((constructor)) __check_proc_table_sizes() {
	if (sizeof(ProcTableImpl) != sizeof(ProcTable))
		panic("sizeof(ProcTableImpl) ({}) != sizeof(ProcTable) ({})",
			  sizeof(ProcTableImpl), sizeof(ProcTable));
}

int proctable_search_cpu_time(OrbTreeNode *cur, const OrbTreeNode *value,
							  OrbTreeNodePair *retval) {
	while (cur) {
		u64 v1 = ((Process *)((byte *)cur - OFFSET_CPU_TIME))->epoch_cpu_time;
		u64 v2 = ((Process *)((byte *)value - OFFSET_CPU_TIME))->epoch_cpu_time;

		if (v1 == v2) {
			retval->self = cur;
			break;
		} else if (v1 < v2) {
			retval->parent = cur;
			retval->is_right = 1;
			cur = cur->right;
		} else {
			retval->parent = cur;
			retval->is_right = 0;
			cur = cur->left;
		}
		retval->self = cur;
	}
	return 0;
}

int proctable_search_lottery(OrbTreeNode *cur, const OrbTreeNode *value,
							 OrbTreeNodePair *retval) {
	while (cur) {
		u64 v1 = ((Process *)((byte *)cur - OFFSET_LOTTERY))->ticket_start;
		u64 v2 = ((Process *)((byte *)value - OFFSET_LOTTERY))->ticket_start;

		if (v1 == v2) {
			retval->self = cur;
			break;
		} else if (v1 < v2) {
			retval->parent = cur;
			retval->is_right = 1;
			cur = cur->right;
		} else {
			retval->parent = cur;
			retval->is_right = 0;
			cur = cur->left;
		}
		retval->self = cur;
	}
	return 0;
}

Object proctable_init(ProcTable *table) {
	return $(0);
}

Object proctable_add_process(ProcTable *table, Process *proc) {
	ProcTableImpl *impl = (ProcTableImpl *)table;
	orbtree_put(&impl->cpu_time_tree, &proc->cpu_time,
				proctable_search_cpu_time);
	orbtree_put(&impl->lottery_tree, &proc->lottery, proctable_search_lottery);

	u32 index = *(proc->channel.value + 28) % PROC_TABLE_ARR_LEN;

	return $(0);
}

Object proctable_remove_process(ProcTable *table, Channel channel) {
	return $(0);
}

Process *proctable_get_process(const ProcTable *table, Channel channel) {
	return NULL;
}

Object proctable_reset_epoch(ProcTable *table) {
	return $(0);
}
