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

#include <base/fam_alloc.h>
#include <base/queue.h>

typedef struct QueueNode {
	byte b;
	Ptr next;
	Ptr value;
} QueueNode;

typedef struct QueueImpl {
	Ptr head;
	Ptr tail;
} QueueImpl;

Queue queue_create() {
	Ptr ret = $alloc(sizeof(QueueImpl));
	if (ret == NULL) return NULL;
	Ptr node = $alloc(sizeof(QueueNode));
	if (node == NULL) {
		$release(ret);
		return NULL;
	}
	QueueNode *qnode = $(node);
	qnode->next = NULL;
	QueueImpl *impl = $(ret);
	impl->head = impl->tail = node;
	return ret;
}

int queue_enqueue(Queue queue, Ptr value) {
	QueueImpl *q = $(queue);
	Ptr node = $alloc(sizeof(QueueNode));
	if (node == NULL) return -1;
	QueueNode *qnode = $(node);
	qnode->value = value;
	qnode->next = NULL;

	Ptr tail;
	Ptr next;
	loop {
		tail = q->tail;
		next = ((QueueNode *)$(tail))->next;
		if (tail == q->tail) {
			if (((QueueNode *)$(next)) == NULL) {
				if (CAS(&((QueueNode *)$(tail))->next, &next, node)) {
					CAS(&q->tail, &tail, node);
					break;
				}
			} else {
				CAS(&q->tail, &tail, next);
			}
		}
	}

	return 0;
}

Ptr queue_dequeue(Queue q) {
	return NULL;
}

// Functions that require override of const
#pragma clang diagnostic ignored \
	"-Wincompatible-pointer-types-discards-qualifiers"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"

void Queue_cleanup(const Queue *queue) {
	if (!nil(*queue)) {
		QueueImpl *impl = $(*queue);
		$release(impl->head);
		fam_release(*queue);
	}
}