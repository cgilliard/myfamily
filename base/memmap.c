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
#include <base/memmap.h>
#include <base/osdef.h>
#include <base/print_util.h>

#define MEMMAP_ENTRY_PER_LEVEL 256
#define BITMAP_SIZE 32
#define MEMMAP_SHIFT 8

_Thread_local int last_i = 0, last_j = 0, last_k = 0, last_l = 0;

typedef struct MemMapImpl {
	byte ****data;
	unsigned int size;
} MemMapImpl;

void __attribute__((constructor)) __memmap_check_sizes() {
	if (sizeof(MemMapImpl) != sizeof(MemMap))
		panic("sizeof(MemMapImpl) (%i) != sizeof(MemMap) (%i)",
			  sizeof(MemMapImpl), sizeof(MemMap));
}

unsigned long long *memmap_itt_for(MemMapImpl *impl, int i, int j, int k,
								   int l) {
	bool mmapped = false;
	byte ****nullvalue1 = NULL;
	byte ****data1 = NULL;
	// load data
	do {
		if (mmapped) {
			MUNMAP(data1, MEMMAP_ENTRY_PER_LEVEL * sizeof(byte ***));
		}
		data1 = ALOAD(&impl->data);
		if (data1 == NULL) {
			data1 = MMAP(MEMMAP_ENTRY_PER_LEVEL * sizeof(byte ***));
			if (data1 == NULL) {
				SetErr(AllocErr);
				return NULL;
			}
			mmapped = true;
			memset(data1, '\0', MEMMAP_ENTRY_PER_LEVEL * sizeof(byte ***));
		} else
			break;
	} while (!CAS(&impl->data, &nullvalue1, data1));

	// load second level
	byte ***nullvalue2 = NULL;
	byte ***data2;
	mmapped = false;
	do {
		if (mmapped) {
			MUNMAP(data2, MEMMAP_ENTRY_PER_LEVEL * sizeof(byte **));
		}
		data2 = ALOAD(&impl->data[i]);
		if (data2 == NULL) {
			data2 = MMAP(MEMMAP_ENTRY_PER_LEVEL * sizeof(byte **));
			if (data2 == NULL) {
				SetErr(AllocErr);
				return NULL;
			}
			mmapped = true;
			memset(data2, '\0', MEMMAP_ENTRY_PER_LEVEL * sizeof(byte **));
		} else
			break;
	} while (!CAS(&impl->data[i], &nullvalue2, data2));

	// load third level
	byte **nullvalue3 = NULL;
	byte **data3;
	mmapped = false;
	do {
		if (mmapped) {
			MUNMAP(data3, MEMMAP_ENTRY_PER_LEVEL * sizeof(byte *));
		}
		data3 = ALOAD(&impl->data[i][j]);
		if (data3 == NULL) {
			data3 = MMAP(MEMMAP_ENTRY_PER_LEVEL * sizeof(byte *));
			if (data3 == NULL) {
				SetErr(AllocErr);
				return NULL;
			}
			mmapped = true;
			memset(data3, '\0', MEMMAP_ENTRY_PER_LEVEL * sizeof(byte *));
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
			MUNMAP(data4, MEMMAP_ENTRY_PER_LEVEL * impl->size * sizeof(byte) +
							  BITMAP_SIZE);
		}
		data4 = ALOAD(&impl->data[i][j][k]);
		if (data4 == NULL) {
			data4 = MMAP(MEMMAP_ENTRY_PER_LEVEL * impl->size * sizeof(byte) +
						 BITMAP_SIZE);
			if (data4 == NULL) {
				SetErr(AllocErr);
				return NULL;
			}
			mmapped = true;
			memset(data4, '\0',
				   MEMMAP_ENTRY_PER_LEVEL * impl->size * sizeof(byte) +
					   BITMAP_SIZE);
			// set Ptr=0 to allocated so we never return null
			if (i == 0 && j == 0 && k == 0) data4[0] = 0x1;
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
	byte *block = impl->data[ptr >> (MEMMAP_SHIFT * 3)]
							[(ptr >> (MEMMAP_SHIFT * 2)) & 0xFF]
							[(ptr >> (MEMMAP_SHIFT)) & 0xFF];
	return (byte *)(block + ((ptr & 0xFF) * impl->size + 32));
}
int memmap_init(MemMap *mm, unsigned int size) {
	MemMapImpl *impl = (MemMapImpl *)mm;
	impl->size = size;
	impl->data = NULL;
	return 0;
}

Ptr memmap_allocate(MemMap *mm) {
	Ptr ret = null;
	MemMapImpl *impl = (MemMapImpl *)mm;
	int i = last_i, j = last_j, k = last_k, l = last_l;
	unsigned long long *current, desired, v;

	do {
		loop {
			// load section of bitmap
			current = memmap_itt_for(impl, i, j, k, l);
			if (current == NULL) return null;
			v = ALOAD(current);
			if (v != ((unsigned long long)0) - 1) break;
			// l is 4 because 64 bits are used in the atomic load
			// the other 4 combinations cover all 256 entries
			if (++l >= 4) {
				l = 0;
				if (++k >= MEMMAP_ENTRY_PER_LEVEL) {
					k = 0;
					if (++j >= MEMMAP_ENTRY_PER_LEVEL) {
						j = 0;
						i++;
						if (i == last_i) {
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
		ret = (i << (MEMMAP_SHIFT * 3)) | (j << (2 * MEMMAP_SHIFT)) |
			  (k << MEMMAP_SHIFT) | (l << 6) | x;
		// set bit
		desired = v | (0x1ULL << x);
	} while (!CAS(current, &v, desired));

	last_i = i;
	last_j = j;
	last_k = k;
	last_l = l;

	return ret;
}

void memmap_free(MemMap *mm, Ptr ptr) {
	if (ptr == null) panic("attempt to free null!");
	if (mm == NULL) panic("invalid (null) memmap");
	MemMapImpl *impl = (MemMapImpl *)mm;
	unsigned long long nv, *v, vo;
	int i, j, k, l;

	i = (ptr >> (MEMMAP_SHIFT * 3)) & 0xFF;
	j = (ptr >> (MEMMAP_SHIFT * 2)) & 0xFF;
	k = (ptr >> (MEMMAP_SHIFT)) & 0xFF;
	l = (ptr >> 6) & 0x3;

	do {
		v = (unsigned long long *)impl->data[i][j][k] + l;
		vo = ALOAD((unsigned long long *)(v));
		if ((vo & (0x1ULL << (ptr & 0x3F))) == 0) panic("double free attempt!");
		nv = vo & ~(0x1ULL << (ptr & 0x3F));
	} while (!CAS(&*v, &vo, nv));

	last_i = i;
	last_j = j;
	last_k = k;
	last_l = 0;
}

void memmap_cleanup(MemMap *mm) {
	MemMapImpl *impl = (MemMapImpl *)mm;
	if (impl->data == NULL) return;
	for (int i = 0; impl->data[i] && i < MEMMAP_ENTRY_PER_LEVEL; i++) {
		for (int j = 0; impl->data[i][j] && j < MEMMAP_ENTRY_PER_LEVEL; j++) {
			for (int k = 0; impl->data[i][j][k] && k < MEMMAP_ENTRY_PER_LEVEL;
				 k++) {
				MUNMAP(impl->data[i][j][k],
					   MEMMAP_ENTRY_PER_LEVEL * impl->size * sizeof(byte) +
						   BITMAP_SIZE);
				impl->data[i][j][k] = NULL;
			}
			MUNMAP(impl->data[i][j], MEMMAP_ENTRY_PER_LEVEL * sizeof(byte *));
		}
		MUNMAP(impl->data[i], MEMMAP_ENTRY_PER_LEVEL * sizeof(byte *));
	}
	MUNMAP(impl->data, MEMMAP_ENTRY_PER_LEVEL * sizeof(byte *));
}
