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

#include <base/fam_err.h>
#include <base/lock.h>
#include <base/memmap.h>
#include <base/mmap.h>
#include <base/print_util.h>
#include <base/util.h>

#define MEMMAP_ENTRY_PER_LEVEL 512
#define BITMAP_SIZE 64
#define IMAX 32

typedef struct MemMapImpl {
	byte ****data;
	unsigned int size;
	unsigned int memmap_id;
	Lock lock;
	byte padding[8];
} MemMapImpl;

typedef struct MemMapIndexCollection {
	unsigned int i;
	unsigned int j;
	unsigned int k;
	unsigned int l;
	unsigned int x;
} MemMapIndexCollection;

static unsigned int memmap_id = 0;
static _Thread_local MemMapIndexCollection last[MAX_MEMMAPS] = {};
bool _debug_capacity_exceeded = false;
static int _debug_memmap_data = -1;

Ptr memmap_index_to_ptr(unsigned int i, unsigned int j, unsigned int k,
						unsigned int l, unsigned int x) {
	unsigned int ret = ((i & 0x1FF) << 27) | ((j & 0x1FF) << 18) |
					   ((k & 0x1FF) << 9) | ((l << 6) & 0x1C0) | (x & 0x3F);
	return ret;
}

MemMapIndexCollection memmap_ptr_to_index(Ptr ptr) {
	MemMapIndexCollection ret = {
		.i = (ptr >> 27) & 0x1FF,
		.j = (ptr >> 18) & 0x1FF,
		.k = (ptr >> 9) & 0x1FF,
		.l = ((ptr >> 6) & 0x7),
		.x = (ptr & 0x3F),
	};
	return ret;
}

unsigned long long *memmap_itt_for(MemMapImpl *impl, int i, int j, int k,
								   int l) {
	lockw(&impl->lock);
	if (impl->data == NULL) {
		impl->data = mmap_allocate(MEMMAP_ENTRY_PER_LEVEL * sizeof(byte ***));
		if (!impl->data) {
			SetErr(AllocErr);
			unlock(&impl->lock);
			return NULL;
		}
		set_bytes((byte *)impl->data, '\0',
				  MEMMAP_ENTRY_PER_LEVEL * sizeof(byte ***));
	}
	if (impl->data[i] == NULL) {
		impl->data[i] = mmap_allocate(MEMMAP_ENTRY_PER_LEVEL * sizeof(byte **));
		if (!impl->data[i]) {
			SetErr(AllocErr);
			unlock(&impl->lock);
			return NULL;
		}
		set_bytes((byte *)impl->data[i], '\0',
				  MEMMAP_ENTRY_PER_LEVEL * sizeof(byte **));
	}
	if (impl->data[i][j] == NULL) {
		impl->data[i][j] =
			mmap_allocate(MEMMAP_ENTRY_PER_LEVEL * sizeof(byte *));
		if (!impl->data[i][j]) {
			SetErr(AllocErr);
			unlock(&impl->lock);
			return NULL;
		}
		set_bytes((byte *)impl->data[i][j], '\0',
				  MEMMAP_ENTRY_PER_LEVEL * sizeof(byte *));
	}
	if (impl->data[i][j][k] == NULL) {
		impl->data[i][j][k] = mmap_allocate(
			MEMMAP_ENTRY_PER_LEVEL * impl->size * sizeof(byte) + BITMAP_SIZE);
		if (!impl->data[i][j][k]) {
			SetErr(AllocErr);
			unlock(&impl->lock);
			return NULL;
		}
		set_bytes((byte *)impl->data[i][j][k], '\0', BITMAP_SIZE);
		if (i == 0 && j == 0 && k == 0) impl->data[i][j][k][0] = 0x3;
	}
	unlock(&impl->lock);

	unsigned long long *ret = ((unsigned long long *)impl->data[i][j][k]) + l;
	return ret;
}

void __attribute__((constructor)) __memmap_check_sizes() {
	if (sizeof(MemMapImpl) != sizeof(MemMap))
		panic("sizeof(MemMapImpl) (%i) != sizeof(MemMap) (%i)",
			  sizeof(MemMapImpl), sizeof(MemMap));
}

int memmap_init(MemMap *mm, unsigned int size) {
	MemMapImpl *impl = (MemMapImpl *)mm;
	impl->size = size;
	impl->data = NULL;
	impl->lock = INIT_LOCK;
	impl->memmap_id = AADD(&memmap_id, 1);
	if (impl->memmap_id > MAX_MEMMAPS) {
		SetErr(CapacityExceeded);
		return -1;
	}
	return 0;
}

void *memmap_data(const MemMap *mm, Ptr ptr) {
	if (_debug_memmap_data >= 0) {
		if (_debug_memmap_data == 0) {
			_debug_memmap_data--;
			return NULL;
		}
		_debug_memmap_data--;
	}
	const MemMapImpl *impl = (const MemMapImpl *)mm;
	MemMapIndexCollection index = memmap_ptr_to_index(ptr);
	byte *block = impl->data[index.i][index.j][index.k];
	return (byte *)(block +
					(((index.l << 6) | index.x) * impl->size + BITMAP_SIZE));
}

Ptr memmap_allocate(MemMap *mm) {
	Slab s = memmap_allocate_slab(mm);
	return s.ptr;
}
Slab memmap_allocate_slab(MemMap *mm) {
	Slab ret = {};
	MemMapImpl *impl = (MemMapImpl *)mm;
	int i = last[impl->memmap_id].i, j = last[impl->memmap_id].j,
		k = last[impl->memmap_id].k, l = last[impl->memmap_id].l;
	unsigned long long *current, desired, v;
	int x;

	do {
		loop {
			// load section of bitmap
			current = memmap_itt_for(impl, i, j, k, l);

			if (current == NULL) return ret;
			v = ALOAD(current);
			if (v != ((unsigned long long)0) - 1) {
				break;
			}
			if (_debug_capacity_exceeded || ++l >= 8) {
				l = 0;
				if (_debug_capacity_exceeded || ++k >= MEMMAP_ENTRY_PER_LEVEL) {
					k = 0;
					if (_debug_capacity_exceeded ||
						++j >= MEMMAP_ENTRY_PER_LEVEL) {
						j = 0;
						if (++i >= IMAX) {	// reduced range
							i = 0;
						}
						if (_debug_capacity_exceeded ||
							i == last[impl->memmap_id].i) {
							SetErr(CapacityExceeded);
							return ret;
						}
					}
				}
			}
		}

		// find open bit
		for (x = 0; (v & (0x1ULL << x)) != 0; x++);
		// set bit
		desired = v | (0x1ULL << x);
	} while (!CAS_SEQ(current, &v, desired));

	ret.ptr = memmap_index_to_ptr(i, j, k, l, x);
	ret.data = (byte *)impl->data[i][j][k] +
			   (((l << 6) | x) * impl->size + BITMAP_SIZE);

	last[impl->memmap_id].i = i;
	last[impl->memmap_id].j = j;
	last[impl->memmap_id].k = k;
	last[impl->memmap_id].l = l;

	return ret;
}

void memmap_free(MemMap *mm, Ptr ptr) {
	if (ptr == null) panic("attempt to free null!");
	if (ptr == ptr_reserved) panic("attempt to free a reserved ptr = 1");
	if (mm == NULL) panic("invalid (null) memmap");
	MemMapImpl *impl = (MemMapImpl *)mm;
	unsigned long long nv, *v, vo;
	int i, j, k, l, x;

	MemMapIndexCollection index = memmap_ptr_to_index(ptr);

	i = index.i;
	j = index.j;
	k = index.k;
	l = index.l;
	x = index.x;

	do {
		v = (unsigned long long *)impl->data[i][j][k] + l;
		vo = ALOAD((unsigned long long *)(v));
		if ((vo & (0x1ULL << x)) == 0) panic("double free attempt!");
		nv = vo & ~(0x1ULL << x);
	} while (!CAS_SEQ(&*v, &vo, nv));

	last[impl->memmap_id].i = i;
	last[impl->memmap_id].j = j;
	last[impl->memmap_id].k = k;
	last[impl->memmap_id].l = 0;
}
void memmap_cleanup(MemMap *mm) {
	MemMapImpl *impl = (MemMapImpl *)mm;
	if (impl->data == NULL) return;
	for (int i = 0; i < IMAX && impl->data[i]; i++) {
		for (int j = 0; j < MEMMAP_ENTRY_PER_LEVEL && impl->data[i][j]; j++) {
			for (int k = 0; k < MEMMAP_ENTRY_PER_LEVEL && impl->data[i][j][k];
				 k++) {
				mmap_free(impl->data[i][j][k],
						  MEMMAP_ENTRY_PER_LEVEL * impl->size * sizeof(byte) +
							  BITMAP_SIZE);
				impl->data[i][j][k] = NULL;
			}
			mmap_free(impl->data[i][j],
					  MEMMAP_ENTRY_PER_LEVEL * sizeof(byte *));
		}
		mmap_free(impl->data[i], MEMMAP_ENTRY_PER_LEVEL * sizeof(byte *));
	}
	mmap_free(impl->data, MEMMAP_ENTRY_PER_LEVEL * sizeof(byte *));
}

#ifdef TEST
void mmemap_force_cleanup(MemMap *mm) {
	MemMapImpl *impl = (MemMapImpl *)mm;
	if (impl->data == NULL) return;
	for (int i = 0; i < IMAX; i++) {
		if (impl->data[i]) {
			for (int j = 0; j < MEMMAP_ENTRY_PER_LEVEL; j++) {
				if (impl->data[i][j]) {
					for (int k = 0; k < MEMMAP_ENTRY_PER_LEVEL; k++) {
						if (impl->data[i][j][k]) {
							mmap_free(impl->data[i][j][k],
									  MEMMAP_ENTRY_PER_LEVEL * impl->size *
											  sizeof(byte) +
										  BITMAP_SIZE);
							impl->data[i][j][k] = NULL;
						}
					}
					mmap_free(impl->data[i][j],
							  MEMMAP_ENTRY_PER_LEVEL * sizeof(byte *));
				}
			}
			mmap_free(impl->data[i], MEMMAP_ENTRY_PER_LEVEL * sizeof(byte *));
		}
	}
	mmap_free(impl->data, MEMMAP_ENTRY_PER_LEVEL * sizeof(byte *));
}
void memmap_reset() {
	ASTORE(&memmap_id, 0);
	for (int i = 0; i < MAX_MEMMAPS; i++)
		last[i].i = last[i].j = last[i].k = last[i].l = 0;
}
void memmap_setijkl(int index, int i, int j, int k, int l) {
	last[index].i = i;
	last[index].j = j;
	last[index].k = k;
	last[index].l = l;
}
void set_debug_memmap_data(int value) {
	_debug_memmap_data = value;
}
#endif	// TEST
