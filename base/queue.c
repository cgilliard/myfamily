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

#include <base/print_util.h>
#include <base/queue.h>

// Michael-Scott lock free queue implementation

typedef struct QueueNode {
	struct QueueNode *next;
	byte value[];
} QueueNode;

typedef struct QueueImpl {
	QueueNode *head;
	QueueNode *tail;
} QueueImpl;

Queue queue_create() {
	return NULL;
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

int queue_enqueue(Queue queue, void *value) {
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

void *queue_dequeue(Queue queue) {
	return NULL;
}

// Functions that require override of const
#pragma clang diagnostic ignored \
	"-Wincompatible-pointer-types-discards-qualifiers"
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"

void Queue_cleanup(const Queue *queue) {
}
