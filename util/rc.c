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

#include <base/chain_alloc.h>
#include <base/fam_err.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/rc.h>

typedef struct RcMetaAtomic {
	atomic_ullong counts;	 // combined weak/strong counts (strong lower bits, weak upper bits)
	bool atomic;			 // whether or not this is an atomic reference counter
	void (*cleanup)(void *); // cleanup function to call when last Rc goes out of scope
	u64 size;				 // size of data
} RcMetaAtomic;

// meta data associated with the Rc.
typedef struct RcMeta {
	u32 weak_count;			 // number of weak references
	u32 strong_count;		 // number of strong references
	bool atomic;			 // whether or not this is an atomic reference counter
	void (*cleanup)(void *); // cleanup function to call when last Rc goes out of scope
	u64 size;				 // size of data
} RcMeta;

void __attribute__((constructor)) __check_atomic_size__() {
	if (sizeof(RcMetaAtomic) != sizeof(RcMeta)) {
		fprintf(stderr, "Error: sizes of RcMetaAtomic and RcMeta are not equal on this platform. "
						"Cannnot continue\n");
		exit(EXIT_FAILURE);
	}
}

typedef struct RcImpl {
	RcMeta meta; // meta data for this Rc
	u8 data[];	 // data held in the Rc
} RcImpl;

// cleanup function automatically called when the Rc goes out of scope
void rc_cleanup(RcNc *ptr) {
	if (nil(ptr->impl))
		return;
	RcImpl *impl = $Ref(&ptr->impl);

	u32 strong_count, weak_count;

	if (impl->meta.atomic) {
		RcMetaAtomic *atomic = ((RcMetaAtomic *)&impl->meta);

		// Decrement strong count
		u64 combined_counts = atomic_fetch_sub(&atomic->counts, 1);
		weak_count = ((combined_counts & 0xFFFFFFFF00000000ULL) >> 32);
		strong_count = (u32)(combined_counts & 0xFFFFFFFFULL);
		strong_count--;
		printf("rc cleanup with counts = %u, %u\n", weak_count, strong_count);
	} else {
		strong_count = --impl->meta.strong_count;
		weak_count = impl->meta.weak_count;
	}

	// if strong count is 0, do cleanup tasks
	if (strong_count == 0) {
		// first get chain guard
		ChainGuard _ = ChainSend(impl->meta.atomic);
		// call user defined cleanup function
		impl->meta.cleanup(impl->data);

		// if there's no weak references left, also free pointer
		if (weak_count == 0) {
			chain_free(&ptr->impl);
		}
	}
	ptr->impl = null;
}

// cleanup function for the weak ref
void weak_cleanup(WeakNc *ptr) {
	if (nil(ptr->impl))
		return;
	RcImpl *impl = $Ref(&ptr->impl);

	u32 strong_count, weak_count;

	if (impl->meta.atomic) {
		RcMetaAtomic *atomic = ((RcMetaAtomic *)&impl->meta);

		// Decrement weak count
		u64 combined_counts = atomic_fetch_sub(&atomic->counts, 1ULL << 32);
		weak_count = ((combined_counts & 0xFFFFFFFF00000000ULL) >> 32);
		strong_count = (u32)(combined_counts & 0xFFFFFFFFULL);
		weak_count--;
		printf("weak cleanup with counts = %u, %u\n", weak_count, strong_count);
	} else {
		strong_count = impl->meta.strong_count;
		weak_count = --impl->meta.weak_count;
	}

	// if both weak and strong counts are 0, we free the data here.
	if (strong_count == 0 && weak_count == 0) {
		ChainGuard _ = ChainSend(impl->meta.atomic);
		chain_free(&ptr->impl);
	}
	ptr->impl = null;
}

// build an rc
int rc_build(Rc *ptr, void *data, u64 size, bool atomic, void (*cleanup)(void *)) {
	if (ptr == NULL || size == 0) {
		fam_err = IllegalArgument;
		return -1;
	}

	{
		// allocate for both meta data and user data
		ChainGuard _ = ChainSend(atomic);
		if (chain_malloc(&ptr->impl, size + sizeof(RcMeta))) {
			fam_err = AllocErr;
			return -1;
		}
	}

	// set initial values
	RcImpl *impl = $Ref(&ptr->impl);
	impl->meta.atomic = atomic;

	if (atomic) {
		RcMetaAtomic *atomic = ((RcMetaAtomic *)&impl->meta);
		atomic_store(&atomic->counts, 1ULL);
	} else {
		impl->meta.weak_count = 0;
		impl->meta.strong_count = 1;
	}
	impl->meta.cleanup = cleanup;
	impl->meta.size = size;

	// copy data
	memcpy(impl->data, data, size);

	return 0;
}

// clone the reference counter incrementing the count
int rc_clone(Rc *dst, const Rc *src) {
	if (dst == NULL || src == NULL) {
		fam_err = IllegalArgument;
		return -1;
	}

	RcImpl *impl = $Ref(&src->impl);
	if (impl->meta.atomic) {
		RcMetaAtomic *atomic = ((RcMetaAtomic *)&impl->meta);
		atomic_fetch_add(&atomic->counts, 1ULL);
	} else {
		impl->meta.strong_count++;
	}
	dst->impl = src->impl;

	return 0;
}

void *rc_access(Rc *ptr) {
	if (ptr == NULL)
		return NULL;
	RcImpl *impl = $Ref(&ptr->impl);
	return impl->data;
}

int rc_weak(Weak *dst, const Rc *src) {
	if (dst == NULL || src == NULL) {
		fam_err = IllegalArgument;
		return -1;
	}
	RcImpl *impl = $Ref(&src->impl);
	if (impl->meta.atomic) {
		RcMetaAtomic *atomic = ((RcMetaAtomic *)&impl->meta);
		atomic_fetch_add(&atomic->counts, 1ULL << 32);

	} else {
		impl->meta.weak_count++;
	}
	dst->impl = src->impl;

	return 0;
}

int rc_upgrade(Rc *upgraded, Weak *ptr) {
	RcImpl *impl = $Ref(&ptr->impl);

	if (impl->meta.atomic) {
		RcMetaAtomic *atomic = ((RcMetaAtomic *)&impl->meta);

		u64 combined_counts, new_combined_counts;
		u32 strong_count;

		do {
			combined_counts = atomic_load(&atomic->counts);
			strong_count = (u32)(combined_counts & 0xFFFFFFFFULL);
			if (strong_count == 0) {
				return -1; // Object already deallocated
			}

			// Calculate the new combined count after the upgrade
			new_combined_counts = combined_counts - (1ULL << 32) + 1;

			// Try to atomically update the counts using compare-and-swap
		} while (
			!atomic_compare_exchange_weak(&atomic->counts, &combined_counts, new_combined_counts));

	} else {
		if (impl->meta.strong_count == 0)
			return -1;
		impl->meta.strong_count++;
		impl->meta.weak_count--;
	}

	upgraded->impl = ptr->impl;
	ptr->impl = null;
	return 0;
}
