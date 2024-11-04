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
#include <base/macros.h>
#include <base/print_util.h>
#include <base/queue.h>

// Michael-Scott lock free queue implementation

unsigned long long queue_global_seqno;

typedef struct QueueNode {
	Ptr next;
	Ptr value;
	unsigned long long seqno;
} QueueNode;

typedef struct QueueImpl {
	Ptr head;
	Ptr tail;
} QueueImpl;

Queue queue_create() {
	Ptr ret = $alloc(sizeof(QueueImpl));
	println("ret.id=%u,%u\n", ptr_id(ret), ptr_len(ret));
	if (ret == NULL) return NULL;
	Ptr node = $alloc(sizeof(QueueNode));
	println("head=%p", $(node));
	if (node == NULL) {
		$release(ret);
		return NULL;
	}
	QueueNode *qnode = $(node);
	qnode->next = NULL;
	QueueImpl *impl = $(ret);
	impl->head = fam_ptr_copy(node);
	impl->tail = fam_ptr_copy(node);

	println("create q->head=%p,q->tail=%p,impl->head=%p", $(impl->head),
			$(impl->tail), impl->head);

	return ret;
}

bool queue_node_equal(Ptr n1, Ptr n2) {
	println("eq %p %p", n1, n2);
	if (n1 != n2) return false;
	if (nil(n1) && nil(n2)) return true;
	if (nil(n1) || nil(n2)) return false;
	QueueNode *qnode1 = $(n1);
	QueueNode *qnode2 = $(n2);
	return qnode1->seqno == qnode2->seqno;
}

// clang-format off
/*
enqueue(Q: pointer to queue t, value: data type)
E1: node = new node() # Allocate a new node from the free list
E2: node–>value = value # Copy enqueued value into node
E3: node–>next.ptr = NULL # Set next pointer of node to NULL
E4: loop # Keep trying until Enqueue is done
E5: tail = Q–>Tail # Read Tail.ptr and Tail.count together
E6: next = tail.ptr–>next # Read next ptr and count fields together
E7: if tail == Q–>Tail # Are tail and next consistent?
E8: if next.ptr == NULL # Was Tail pointing to the last node?
E9: if CAS(&tail.ptr–>next, next, <node, next.count+1>) # Try to link node at the end of the linked list
E10: break # Enqueue is done. Exit loop
E11: endif
E12: else # Tail was not pointing to the last node
E13: CAS(&Q–>Tail, tail, <next.ptr, tail.count+1>) # Try to swing Tail to the next node
E14: endif
E15: endif
E16: endloop
E17: CAS(&Q–>Tail, tail, <node, tail.count+1>)
*/
// clang-format on

int queue_enqueue(Queue queue, Ptr value) {
	if (nil(value)) return -1;
	QueueImpl *q = $(queue);

	println("pre enqueue q->head=%p,q->tail=%p,q->head=%p", $(q->head),
			$(q->tail), q->head);

	Ptr node = $alloc(sizeof(QueueNode));
	println("node=%p", node);
	if (node == NULL) return -1;
	QueueNode *qnode = $(node);
	qnode->value = value;
	qnode->next = NULL;
	qnode->seqno = __atomic_fetch_add(&queue_global_seqno, 1, __ATOMIC_RELAXED);

	Ptr tail;
	Ptr next;

	println("pre enqueue q->head=%p,q->tail=%p", $(q->head), $(q->tail));

	loop {
		tail = q->tail;
		next = ((QueueNode *)$(tail))->next;

		if ($(tail) == $(q->tail)) {
			println("equal");
		}

		if (next == NULL) {
			println("next null");
			if (CAS_SEQ(&((QueueNode *)$(tail))->next, &next, node)) {
				println("cas seq cond true");
				CAS_SEQ(&q->tail, &tail, node);
				println("break");
				break;
			} else {
				println("next not null");
				CAS_SEQ(&q->tail, &tail, next);
			}
		}
	}

	println("post enqueue q->head=%p,q->tail=%p", $(q->head), $(q->tail));

	return 0;
}

// clang-format off
/*
dequeue(Q: pointer to queue t, pvalue: pointer to data type): boolean
D1: loop # Keep trying until Dequeue is done
D2: head = Q–>Head # Read Head
D3: tail = Q–>Tail # Read Tail
D4: next = head–>next # Read Head.ptr–>next
D5: if head == Q–>Head # Are head, tail, and next consistent?
D6: if head.ptr == tail.ptr # Is queue empty or Tail falling behind?
D7: if next.ptr == NULL # Is queue empty?
D8: return FALSE # Queue is empty, couldn’t dequeue
D9: endif
D10: CAS(&Q–>Tail, tail, <next.ptr, tail.count+1>) # Tail is falling behind. Try to advance it
D11: else # No need to deal with Tail
# Read value before CAS, otherwise another dequeue might free the next node
D12: *pvalue = next.ptr–>value
D13: if CAS(&Q–>Head, head, <next.ptr, head.count+1>) # Try to swing Head to the next node
D14: break # Dequeue is done. Exit loop
D15: endif
D16: endif
D17: endif
D18: endloop
D19: free(head.ptr) # It is safe now to free the old dummy node
D20: return TRUE
*/
// clang-format on

Ptr queue_dequeue(Queue queue) {
	return NULL;
}

// Functions that require override of const
#pragma clang diagnostic ignored \
	"-Wincompatible-pointer-types-discards-qualifiers"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"

void Queue_cleanup(const Queue *queue) {
	println("q cleanup");
	if (!nil(*queue)) {
		QueueImpl *impl = $(*queue);
		//$release(impl->head);
		fam_release(*queue);
	}
}
