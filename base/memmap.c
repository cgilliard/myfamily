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

static unsigned int memmap_id = 0;
_Thread_local int last_i[MAX_MEMMAPS] = {};
_Thread_local int last_j[MAX_MEMMAPS] = {};
_Thread_local int last_k[MAX_MEMMAPS] = {};
_Thread_local int last_l[MAX_MEMMAPS] = {};

typedef struct MemMapImpl {
	byte ****data;
	unsigned int size;
	unsigned int memmap_id;
} MemMapImpl;

void __attribute__((constructor)) __memmap_check_sizes() {
	if (sizeof(MemMapImpl) != sizeof(MemMap))
		panic("sizeof(MemMapImpl) (%i) != sizeof(MemMap) (%i)",
			  sizeof(MemMapImpl), sizeof(MemMap));
}

typedef struct MemMapIndexCollection {
	unsigned int i;
	unsigned int j;
	unsigned int k;
	unsigned int l;
	unsigned int x;
} MemMapIndexCollection;

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
	bool mmapped = false;
	byte ****nullvalue1 = NULL;
	byte ****data1 = NULL;
	// load data
	do {
		if (mmapped) {
			mmap_free(data1, MEMMAP_ENTRY_PER_LEVEL * sizeof(byte ***));
		}
		data1 = ALOAD(&impl->data);
		if (data1 == NULL) {
			data1 = mmap_allocate(MEMMAP_ENTRY_PER_LEVEL * sizeof(byte ***));
			if (data1 == NULL) {
				SetErr(AllocErr);
				return NULL;
			}
			mmapped = true;
			set_bytes((byte *)data1, '\0',
					  MEMMAP_ENTRY_PER_LEVEL * sizeof(byte ***));
		} else
			break;
	} while (!CAS(&impl->data, &nullvalue1, data1));

	// load second level
	byte ***nullvalue2 = NULL;
	byte ***data2;
	mmapped = false;
	do {
		if (mmapped) {
			mmap_free(data2, MEMMAP_ENTRY_PER_LEVEL * sizeof(byte **));
		}
		data2 = ALOAD(&impl->data[i]);
		if (data2 == NULL) {
			data2 = mmap_allocate(MEMMAP_ENTRY_PER_LEVEL * sizeof(byte **));
			if (data2 == NULL) {
				SetErr(AllocErr);
				return NULL;
			}
			mmapped = true;
			set_bytes((byte *)data2, '\0',
					  MEMMAP_ENTRY_PER_LEVEL * sizeof(byte **));
		} else
			break;
	} while (!CAS(&impl->data[i], &nullvalue2, data2));
	// load third level
	byte **nullvalue3 = NULL;
	byte **data3;
	mmapped = false;
	do {
		if (mmapped) {
			mmap_free(data3, MEMMAP_ENTRY_PER_LEVEL * sizeof(byte *));
		}

		data3 = ALOAD(&impl->data[i][j]);
		if (data3 == NULL) {
			data3 = mmap_allocate(MEMMAP_ENTRY_PER_LEVEL * sizeof(byte *));
			if (data3 == NULL) {
				SetErr(AllocErr);
				return NULL;
			}
			mmapped = true;
			set_bytes((byte *)data3, '\0',
					  MEMMAP_ENTRY_PER_LEVEL * sizeof(byte *));
		} else
			break;
	} while (!CAS(&impl->data[i][j], &nullvalue3, data3));
	// load fourth and final level
	byte *nullvalue4 = NULL;
	byte *data4;
	mmapped = false;
	// add 32 bytes for the bitmap
	do {
		if (mmapped) {
			mmap_free(data4,
					  MEMMAP_ENTRY_PER_LEVEL * impl->size * sizeof(byte) +
						  BITMAP_SIZE);
		}

		data4 = ALOAD(&impl->data[i][j][k]);
		if (data4 == NULL) {
			data4 = mmap_allocate(MEMMAP_ENTRY_PER_LEVEL * impl->size *
									  sizeof(byte) +
								  BITMAP_SIZE);
			if (data4 == NULL) {
				SetErr(AllocErr);
				return NULL;
			}
			mmapped = true;
			set_bytes((byte *)data4, '\0', BITMAP_SIZE);
			// set Ptr=0 to allocated so we never return null / also reserve the
			// first value for other purposes
			if (i == 0 && j == 0 && k == 0) data4[0] = 0x3;
		} else
			break;
	} while (!CAS(&impl->data[i][j][k], &nullvalue4, data4));

	// return the lth item at the begining of the data array (32 bytes reserved)
	// l is between 0-3.
	unsigned long long *ret = ((unsigned long long *)impl->data[i][j][k]) + l;
	return ret;
}

void *memmap_data(const MemMap *mm, Ptr ptr) {
	const MemMapImpl *impl = (const MemMapImpl *)mm;
	MemMapIndexCollection index = memmap_ptr_to_index(ptr);
	byte *block = impl->data[index.i][index.j][index.k];
	return (byte *)(block +
					(((index.l << 6) | index.x) * impl->size + BITMAP_SIZE));
}
int memmap_init(MemMap *mm, unsigned int size) {
	MemMapImpl *impl = (MemMapImpl *)mm;
	impl->size = size;
	impl->data = NULL;
	impl->memmap_id = AADD(&memmap_id, 1);
	if (impl->memmap_id > MAX_MEMMAPS) {
		SetErr(CapacityExceeded);
		return -1;
	}
	return 0;
}

Ptr memmap_allocate(MemMap *mm) {
	Ptr ret = null;
	MemMapImpl *impl = (MemMapImpl *)mm;
	int i = last_i[impl->memmap_id], j = last_j[impl->memmap_id],
		k = last_k[impl->memmap_id], l = last_l[impl->memmap_id];
	unsigned long long *current, desired, v;

	do {
		loop {
			// load section of bitmap
			current = memmap_itt_for(impl, i, j, k, l);

			if (current == NULL) return null;
			v = ALOAD(current);
			if (v != ((unsigned long long)0) - 1) {
				break;
			}
			// l is 4 because 64 bits are used in the atomic load
			// the other 4 combinations cover all 256 entries
			if (++l >= 8) {
				l = 0;
				if (++k >= MEMMAP_ENTRY_PER_LEVEL) {
					k = 0;
					if (++j >= MEMMAP_ENTRY_PER_LEVEL) {
						j = 0;
						if (++i >= IMAX) {	// reduced range
							i = 0;
						}
						if (i == last_i[impl->memmap_id]) {
							SetErr(CapacityExceeded);
							return null;
						}
					}
				}
			}
		}

		// find open bit
		int x;
		for (x = 0; (v & (0x1ULL << x)) != 0; x++);
		ret = memmap_index_to_ptr(i, j, k, l, x);
		// set bit
		desired = v | (0x1ULL << x);
	} while (!CAS(current, &v, desired));

	last_i[impl->memmap_id] = i;
	last_j[impl->memmap_id] = j;
	last_k[impl->memmap_id] = k;
	last_l[impl->memmap_id] = l;

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
	} while (!CAS(&*v, &vo, nv));

	last_i[impl->memmap_id] = i;
	last_j[impl->memmap_id] = j;
	last_k[impl->memmap_id] = k;
	last_l[impl->memmap_id] = 0;
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
void memmap_reset() {
	ASTORE(&memmap_id, 0);
	for (int i = 0; i < MAX_MEMMAPS; i++)
		last_i[i] = last_j[i] = last_k[i] = last_l[i] = 0;
}

void memmap_setijkl(int index, int i, int j, int k, int l) {
	last_i[index] = i;
	last_j[index] = j;
	last_k[index] = k;
	last_l[index] = l;
}
#endif	// TEST
