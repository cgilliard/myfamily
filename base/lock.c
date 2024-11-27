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

#include <base/atomic.h>
#include <base/lock.h>
#include <base/print_util.h>
#include <base/sys.h>

#define MAX_LOCK_LEVEL 128
_Thread_local char _lock_is_write__[MAX_LOCK_LEVEL + 1] = {};
_Thread_local int stack_level = 0;

// highest of the first 32 bits
#define WRITE_FLAG 0x80000000ULL
// second highest of the first 32 bits (allowing over 1 billion readers)
#define UPGRADE_FLAG 0x40000000ULL
#define SEQNO_INCR 0x100000000ULL
#define READER_INCR 0x1
#define READ_COMPLETE_MASK 0xFFFFFFFFA0000000ULL
#define MAX_READERS (0x1 << 29)

Lock lock_create() {
	return 0;
}
void lock_read(Lock *lock) {
	if (stack_level >= MAX_LOCK_LEVEL)
		panic("too many lock levels: MAX=%i", MAX_LOCK_LEVEL);
	_lock_is_write__[stack_level++] = 0;
	unsigned long long state;
	unsigned long long state_update;
	int itt = 0;
	// obtain read lock
	do {
		if (itt++) sched_yield();
		state = ALOAD(lock) & ~(WRITE_FLAG | UPGRADE_FLAG);
		if ((state & ~READ_COMPLETE_MASK) > MAX_READERS)
			panic("too many readers!");
		state_update = (state + SEQNO_INCR) + READER_INCR;
	} while (!CAS_RELEASE(lock, &state, state_update));
}
void lock_write(Lock *lock) {
	if (stack_level >= MAX_LOCK_LEVEL)
		panic("too many lock levels: MAX=%i", MAX_LOCK_LEVEL);
	_lock_is_write__[stack_level++] = 1;
	unsigned long long state;
	unsigned long long state_update;
	int itt = 0;
	// obtain write lock
	do {
		if (itt++) sched_yield();
		state = ALOAD(lock) & ~(WRITE_FLAG | UPGRADE_FLAG);
		state_update = (state + SEQNO_INCR) | WRITE_FLAG;
	} while (!CAS_SEQ(lock, &state, state_update));

	itt = 0;
	// now ensure all readers are complete
	do {
		if (itt++) sched_yield();
		state = ALOAD(lock) & READ_COMPLETE_MASK;
		state_update = (state + SEQNO_INCR);
	} while (!CAS_SEQ(lock, &state, state_update));
}

void lock_unlock(Lock *lock) {
	unsigned long long state;
	unsigned long long state_update;
	int itt = 0;
	do {
		if (itt++) sched_yield();
		if (stack_level == 0) panic("too many unlocks!");
		// determine type of lock
		if (_lock_is_write__[--stack_level]) {
			state = ALOAD(lock);
			state_update = (state + SEQNO_INCR) & ~WRITE_FLAG;
		} else {
			state = ALOAD(lock);
			if ((state & 0x3FFFFFFF) == 0) panic("underflow!");
			state_update = (state + SEQNO_INCR) - READER_INCR;
		}
	} while (!CAS_RELEASE(lock, &state, state_update));
}

void lock_upgrade(Lock *lock) {
	_lock_is_write__[stack_level] = 1;
	unsigned long long state;
	unsigned long long state_update;
	int itt = 0;
	// obtain upgrade lock
	do {
		if (itt++) sched_yield();
		state = ALOAD(lock) & ~(WRITE_FLAG | UPGRADE_FLAG);
		state_update = (state + SEQNO_INCR) | UPGRADE_FLAG;
	} while (!CAS_SEQ(lock, &state, state_update));

	itt = 0;
	// obtain write lock and also remove our read and upgrade locks
	do {
		if (itt++) sched_yield();
		state = ALOAD(lock);
		state_update =
			(((state + SEQNO_INCR) | WRITE_FLAG) & ~UPGRADE_FLAG) - READER_INCR;
	} while (!CAS_SEQ(lock, &state, state_update));
}

void lock_downgrade(Lock *lock) {
	_lock_is_write__[stack_level] = 0;
	unsigned long long state;
	unsigned long long state_update;
	int itt = 0;

	// remove our write lock and add read lock at the same time
	// we also ensure no one else is upgrading right now
	do {
		if (itt++) sched_yield();
		state = ALOAD(lock) & ~UPGRADE_FLAG;
		state_update = ((state + SEQNO_INCR) & ~WRITE_FLAG) + READER_INCR;
	} while (!CAS_RELEASE(lock, &state, state_update));
}
