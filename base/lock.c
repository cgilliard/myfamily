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
#include <base/lock.h>
#include <base/print_util.h>

_Thread_local bool _lock_is_write__ = false;

typedef struct LockImpl {
	// state defines our state. The upper 32 bits are a sequence number which
	// ensures proper ordering the lower 32 bits are: 1.) The higest bit
	// indicates 'write pending' - a writer would like to write the lower 31
	// bits are used as a counter for active readers.
	unsigned long long state;
} LockImpl;

Lock lock_create() {
	Ptr ret = ptr_direct_alloc(sizeof(LockImpl));
	if (nil(ret)) return NULL;
	LockImpl *impl = $(ret);
	impl->state = 0;
	return ret;
}
void lock_read(Lock lock) {
	_lock_is_write__ = false;
	LockImpl *impl = $(lock);

	unsigned long long state;
	unsigned long long state_update;
	do {
		// get current state set the write_pending bit to false
		state =
			__atomic_load_n(&impl->state, __ATOMIC_RELAXED) & ~0x80000000ULL;
		// increment the read counter and add 1 to the sequence number (upper 32
		// bits)
		state_update = (state + 0x100000000ULL) + 1ULL;
		// while our target state (no change including sequence number and
		// write_pending = false, we spin)
	} while (!__atomic_compare_exchange_n(&impl->state, &state, state_update,
										  false, __ATOMIC_RELAXED,
										  __ATOMIC_RELAXED));
}
void lock_write(Lock lock) {
	_lock_is_write__ = true;
	LockImpl *impl = $(lock);

	unsigned long long state;
	unsigned long long state_update;
	do {
		// get current state set the write_pending bit to false (we can't write
		// until it's false) also set the read counter to 0, we can't write
		// until both conditions are met
		state = __atomic_load_n(&impl->state, __ATOMIC_RELAXED) &
				0xFFFFFFFF00000000ULL;
		// set the updated value to set the write bit true and read count to 0,
		// incremenet the sequence number
		state_update = (state + 0x180000000ULL) & 0xFFFFFFFF80000000ULL;
	} while (!__atomic_compare_exchange_n(&impl->state, &state, state_update,
										  false, __ATOMIC_RELAXED,
										  __ATOMIC_RELAXED));
}
void lock_unlock(Lock lock) {
	LockImpl *impl = $(lock);
	unsigned long long state;
	unsigned long long state_update;
	// check thread local write variable
	if (_lock_is_write__) {
		// writer
		do {
			// get current state
			state = __atomic_load_n(&impl->state, __ATOMIC_RELAXED);
			// unset the write bit and increment the sequence number
			state_update = (state + 0x100000000ULL) & ~0x80000000ULL;
		} while (!__atomic_compare_exchange_n(
			&impl->state, &state, state_update, false, __ATOMIC_RELAXED,
			__ATOMIC_RELAXED));
	} else {
		// reader
		do {
			// get current state
			state = __atomic_load_n(&impl->state, __ATOMIC_RELAXED);
			if ((state & 0x7FFFFFFF) == 0) panic("underflow!");
			// subtract 1 from the read count and increment the sequence number
			state_update = (state + 0x100000000ULL) - 1ULL;
		} while (!__atomic_compare_exchange_n(
			&impl->state, &state, state_update, false, __ATOMIC_RELAXED,
			__ATOMIC_RELAXED));
	}
}

void Lock_cleanup(const Lock *ptr) {
	if (!nil(*ptr)) {
		ptr_direct_release(*ptr);
	}
}

typedef struct LockGuardImpl {
	Lock lock;
} LockGuardImpl;

LockGuard lock_guard_read(Lock l) {
	LockImpl *li = $(l);
	Ptr ret = fam_alloc(sizeof(LockGuardImpl));
	LockGuardImpl *lgi = $(ret);
	lgi->lock = l;
	lock_read(lgi->lock);
	return ret;
}
LockGuard lock_guard_write(Lock l) {
	LockImpl *li = $(l);
	Ptr ret = fam_alloc(sizeof(LockGuardImpl));
	LockGuardImpl *lgi = $(ret);
	lgi->lock = l;
	lock_write(lgi->lock);
	return ret;
}

void LockGuard_cleanup(const LockGuard *lg) {
	if (initialized(*lg)) {
		LockGuardImpl *lgi = $(*lg);
		lock_unlock(lgi->lock);
		fam_release(*lg);
	}
}

#ifdef TEST
unsigned long long lock_get_state(Lock lock) {
	LockImpl *impl = $(lock);
	return impl->state;
}
#endif	// TEST
