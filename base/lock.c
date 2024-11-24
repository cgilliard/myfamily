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

#include <base/lock.h>
#include <base/macros.h>
#include <base/print_util.h>

#define MAX_LOCK_LEVEL 128
_Thread_local bool _lock_is_write__[MAX_LOCK_LEVEL + 1] = {};
_Thread_local int stack_level = 0;

Lock lock_create() {
	return 0;
}
void lock_read(Lock *lock) {
	if (stack_level >= MAX_LOCK_LEVEL)
		panic("too many lock levels: MAX=%i", MAX_LOCK_LEVEL);
	_lock_is_write__[stack_level++] = false;
	uint64 state;
	uint64 state_update;
	do {
		// get current state set the write_pending bit to false
		state = ALOAD(lock) & ~0x80000000ULL;
		// increment the read counter and add 1 to the sequence number (upper 32
		// bits)
		state_update = (state + 0x100000000ULL) + 1ULL;
		// while our target state (no change including sequence number and
		// write_pending != false, we spin)
	} while (!CAS_ACQUIRE(lock, &state, state_update));
}
void lock_write(Lock *lock) {
	if (stack_level >= MAX_LOCK_LEVEL)
		panic("too many lock levels: MAX=%i", MAX_LOCK_LEVEL);
	_lock_is_write__[stack_level++] = true;
	uint64 state;
	uint64 state_update;

	// first step, set write bit true indicating a writer is waiting
	// this lock should be obtained soon after the previous writer
	// is complete indicating to readers our desire to write
	// this avoids write starvation
	do {
		state = ALOAD(lock) & ~0x80000000ULL;
		state_update = (state + 0x100000000ULL) | 0x80000000ULL;
	} while (!CAS_SEQ(lock, &state, state_update));

	// second step, obtain total lock before proceeding
	do {
		// get current state. We will wait for the read count to go to 0.
		state = ALOAD(lock) & 0xFFFFFFFF80000000ULL;
		// set the updated value to set the write bit true and read count to 0,
		// increment the sequence number
		state_update = (state + 0x100000000ULL) & 0xFFFFFFFF80000000ULL;
	} while (!CAS_ACQUIRE(lock, &state, state_update));
}

void lock_unlock(Lock *lock) {
	uint64 state;
	uint64 state_update;
	// check thread local write variable
	if (_lock_is_write__[--stack_level]) {
		// writer
		do {
			// get current state
			state = ALOAD(lock);
			// unset the write bit and increment the sequence number
			state_update = (state + 0x100000000ULL) & ~0x80000000ULL;
		} while (!CAS_RELEASE(lock, &state, state_update));
	} else {
		// reader
		do {
			// get current state
			state = ALOAD(lock);
			if ((state & 0x7FFFFFFF) == 0) panic("underflow!");
			// subtract 1 from the read count and increment the sequence number
			state_update = (state + 0x100000000ULL) - 1ULL;
		} while (!CAS_RELEASE(lock, &state, state_update));
	}
}

void lock_guard_cleanup(LockGuard *l) {
	lock_unlock(l);
}

LockGuard lock_guard_read(Lock l) {
	lock_read(&l);
	return l;
}
LockGuard lock_guard_write(Lock l) {
	lock_write(&l);
	return l;
}
