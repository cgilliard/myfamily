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
#include <base/print_util.h>
#include <base/queue.h>

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
	if (ret == NULL) return NULL;
	Ptr node = $alloc(sizeof(QueueNode));
	println("head=%p", node);
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
	Ptr node = $alloc(sizeof(QueueNode));
	println("node=%p", node);
	if (node == NULL) return -1;
	QueueNode *qnode = $(node);
	qnode->value = value;
	qnode->next = NULL;
	qnode->seqno = __atomic_fetch_add(&queue_global_seqno, 1, __ATOMIC_RELAXED);

	Ptr tail;
	Ptr next;
	println("begin cas");
	loop {
		tail = q->tail;
		next = ((QueueNode *)$(tail))->next;
		if ($(tail) == $(q->tail)) {
			println("equal");
			if (next == NULL) {
				println("next null");
				if (CAS_SEQ(&((QueueNode *)$(tail))->next, &next, node)) {
					println("break");
					CAS_SEQ(&q->tail, &tail, node);
					break;
				}
			} else {
				println("next not null");
				CAS_SEQ(&q->tail, &tail, next);
			}
		}
	}

	println("q->head=%p,q->tail=%p", q->head, q->tail);

	return 0;
}

Ptr queue_dequeue(Queue queue) {
	println("dequeue");
	Ptr ret = NULL;
	QueueImpl *q = $(queue);
	Ptr head;
	Ptr tail;
	Ptr next;

	loop {
		head = q->head;
		tail = q->tail;

		if (head == q->head) {
			if (head == tail && ((QueueNode *)$(head))->next == NULL) {
				println("ret null");
				println("q->head=%p,q->tail=%p", q->head, q->tail);
				return NULL;
			}

			if (head != tail) {
				println("head != tail");
				if (CAS_SEQ(
						&q->head, &head,
						((QueueNode *)$(head))->next)) {  // Update head first
					next = ((QueueNode *)$(head))
							   ->next;	// Now get the next pointer
					ret = next ? ((QueueNode *)$(next))->value : NULL;
					// TODO: deallocation logic
					break;
				}
			} else {
				println("else");
				CAS_SEQ(&q->tail, &tail, ((QueueNode *)$(tail))->next);
			}
		}
	}

	println("q->head=%p,q->tail=%p", q->head, q->tail);
	println("ret val");
	return ret;
}

/*
Ptr queue_dequeue(Queue queue) {
	Ptr ret = NULL;
	QueueImpl *q = $(queue);
	Ptr head;
	Ptr tail;
	Ptr next;

	loop {
		head = q->head;
		tail = q->tail;
		next = ((QueueNode *)$(head))->next;
		// if (queue_node_equal(head, q->head)) {
		if (head == q->head) {
			println("empty check %p %p %p", head, tail, next);
			if (head == tail && next == head) {
				println("ret null");
				println("q->head=%p,q->tail=%p", q->head, q->tail);
				return NULL;
			}

		//				if (queue_node_equal(head, tail) &&
		//	   queue_node_equal(next, head)) { println("ret null"); return NULL;
		//				}
			println("empty check complete");
			// if (!queue_node_equal(head, tail)) {
			if (head != tail) {
				ret = next ? ((QueueNode *)$(next))->value : NULL;

				if (CAS_SEQ(&q->head, &head, next)) {
					println("checking empty queue");
					if (next == NULL) {
						println("queue empty set head to point to itself");
						((QueueNode *)$(head))->next = head;
					}
					// TODO: deallocation logic
					break;
				}
			} else {
				CAS_SEQ(&q->tail, &tail, next);
			}
		}
	}

	println("q->head=%p,q->tail=%p", q->head, q->tail);
	println("ret val");
	return ret;
}
*/

// Functions that require override of const
#pragma clang diagnostic ignored \
	"-Wincompatible-pointer-types-discards-qualifiers"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"

void Queue_cleanup(const Queue *queue) {
	if (!nil(*queue)) {
		QueueImpl *impl = $(*queue);
		//$release(impl->head);
		// fam_release(*queue);
	}
}
