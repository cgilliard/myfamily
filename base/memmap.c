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
#include <base/osdef.h>
#include <base/print_util.h>

#define ALLOCS_PER_BLOCK 2048

typedef struct MemMapImpl {
	byte **data[MEM_MAP_NUM_CHUNKS];
	unsigned long long *bitmap[MEM_MAP_NUM_CHUNKS];
	unsigned int chunks;
	unsigned int size;
	unsigned int chunk_size;
	Lock lock;
} MemMapImpl;

void __attribute__((constructor)) __memmap_check_sizes() {
	if (sizeof(MemMapImpl) != sizeof(MemMap))
		panic("sizeof(MemMapImpl) (%i) != sizeof(MemMap) (%i)",
			  sizeof(MemMapImpl), sizeof(MemMap));
}

void *memmap_data(const MemMap *mm, Ptr ptr) {
	const MemMapImpl *impl = (const MemMapImpl *)mm;
	byte *block = impl->data[ptr >> 22][(ptr >> 11) & 0x7FF];
	return (byte *)(block + ((ptr & 0x7FF) * impl->size));
}
int memmap_init(MemMap *mm, unsigned int size) {
	MemMapImpl *impl = (MemMapImpl *)mm;
	impl->size = size;
	impl->chunks = 0;
	impl->chunk_size = 128;
	impl->lock = INIT_LOCK;

	memset(impl->bitmap, '\0',
		   MEM_MAP_NUM_CHUNKS * sizeof(unsigned long long *));
	memset(impl->data, '\0', MEM_MAP_NUM_CHUNKS * sizeof(byte **));
	return 0;
}

int memmap_allocate_bitmap(MemMapImpl *impl, int i) {
	bool mmapped = false;
	unsigned long long *nulldata = NULL;
	unsigned long long *bitmap;
	do {
		if (mmapped) {
			MUNMAP(bitmap, MEM_MAP_CHUNK_SIZE);
		}
		bitmap = ALOAD(&impl->bitmap[i]);
		if (bitmap == NULL) {
			bitmap = MMAP(MEM_MAP_CHUNK_SIZE);
			mmapped = true;
			if (bitmap == NULL) {
				SetErr(AllocErr);
				return -1;
			}
			memset(bitmap, '\0', MEM_MAP_CHUNK_SIZE);
			if (i == 0) bitmap[0] = 0x1;
			impl->bitmap[i] = bitmap;
		} else
			break;
	} while (CAS(&impl->bitmap[i], &nulldata, bitmap));
	return 0;
}

int memmap_check_data(MemMapImpl *impl, Ptr ptr) {
	byte **data;
	bool mmapped = false;
	byte **nulldata = NULL;

	int alloc_size = ALLOCS_PER_BLOCK * 8;

	do {
		if (mmapped) MUNMAP(data, alloc_size);
		data = ALOAD(&impl->data[ptr >> 22]);
		if (data == NULL) {
			data = MMAP(alloc_size);
			if (data == NULL) {
				SetErr(AllocErr);
				return -1;
			}
			memset(data, '\0', alloc_size);
			mmapped = true;
		} else
			break;
	} while (!CAS(&impl->data[ptr >> 22], &nulldata, data));

	// now check our block
	byte *block;
	byte *nullblock = NULL;
	mmapped = false;
	do {
		if (mmapped) {
			MUNMAP(block, impl->size * ALLOCS_PER_BLOCK);
		}
		block = ALOAD(impl->data[ptr >> 22] + ((ptr >> 11) & 0x7FF));
		if (block == NULL) {
			block = MMAP(impl->size * ALLOCS_PER_BLOCK);
			if (block == NULL) {
				SetErr(AllocErr);
				return -1;
			}
			mmapped = true;
		} else
			break;
	} while (
		!CAS(impl->data[ptr >> 22] + ((ptr >> 11) & 0x7FF), &nullblock, block));

	return 0;
}

_Thread_local int last_i = 0, last_j = 0;

Ptr memmap_allocate(MemMap *mm) {
	Ptr ret;
	int i, j, k;
	unsigned long long *itt, nitt, v;
	MemMapImpl *impl;

	i = j = k = 0;
	impl = (MemMapImpl *)mm;

	i = last_i;

	do {
		loop {
			if (memmap_allocate_bitmap(impl, i)) return null;
			itt = (impl->bitmap[i] + j);
			if ((v = ALOAD(itt)) != ((unsigned long long)0) - 1) break;
			if (++j >= MEM_MAP_NUM_CHUNKS) {
				j = 0;
				i++;
			}
		}
		k = 0;
		while ((v & (0x1ULL << k)) != 0) k++;
		nitt = v | (0x1ULL << k);
		ret = ((i << 19) | (j << 6) | k);
		if (memmap_check_data(impl, ret)) return null;
	} while (!CAS(itt, &v, nitt));
	last_i = i;
	last_j = j;
	return ret;
}

void memmap_free(MemMap *mm, Ptr ptr) {
	if (ptr == null) panic("attempt to free null!");
	MemMapImpl *impl = (MemMapImpl *)mm;
	unsigned long long nv, *v, vo;
	do {
		v = impl->bitmap[ptr >> 19] + ((ptr >> 6) & 0x3F);
		vo = ALOAD((unsigned long long *)(v));
		if ((vo & (0x1ULL << (ptr & 0x3F))) == 0) panic("double free attempt!");
		nv = vo & ~(0x1ULL << (ptr & 0x3F));
	} while (!CAS(&*v, &vo, nv));

	last_i = ptr >> 19;
	last_j = ptr >> 6;
}

void memmap_cleanup(MemMap *mm) {
	MemMapImpl *impl = (MemMapImpl *)mm;
	for (int i = 0; i < MEM_MAP_NUM_CHUNKS; i++) {
		if (impl->bitmap[i]) MUNMAP(impl->bitmap[i], MEM_MAP_NUM_CHUNKS);
		impl->bitmap[i] = NULL;

		if (impl->data[i]) {
			for (int j = 0; j < ALLOCS_PER_BLOCK; j++) {
				if (impl->data[i][j]) {
					MUNMAP(impl->data[i][j], impl->size * ALLOCS_PER_BLOCK);
					impl->data[i][j] = NULL;
				}
			}
			MUNMAP(impl->data[i], MEM_MAP_NUM_CHUNKS);
		}
		impl->data[i] = NULL;
	}
}
