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
	byte *block = impl->data[ptr >> 19][ptr >> 6];
	return (byte *)(block + ((ptr & 0x3F) * impl->size));
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
	if (impl->bitmap[i] == NULL) {
		impl->bitmap[i] = malloc(MEM_MAP_CHUNK_SIZE);
		if (impl->bitmap[i] == NULL) {
			SetErr(AllocErr);
			return -1;
		}
		memset(impl->bitmap[i], '\0', MEM_MAP_CHUNK_SIZE);
		// set null to allocated so we don't assign it
		if (i == 0) impl->bitmap[i][0] = 0x1;
	}
	return 0;
}

int memmap_check_data(MemMapImpl *impl, int i, int j, int k) {
	byte **data;
	bool mallocked = false;
	byte **nulldata = NULL;

	do {
		if (mallocked) free(data);
		data = ALOAD(&impl->data[i]);
		if (data == NULL) {
			int page_size = getpagesize();
			int slots_per_page = page_size / impl->size;
			if (slots_per_page == 0) slots_per_page = 1;
			int alloc_size = sizeof(byte *) * (UINT32_MAX / (impl->size * 8)) /
							 slots_per_page;
			data = malloc(alloc_size);
			if (data == NULL) {
				SetErr(AllocErr);
				return -1;
			}
			memset(data, '\0', alloc_size);
			mallocked = true;
		} else
			break;
	} while (!CAS(&impl->data[i], &nulldata, data));

	// now check our block
	byte *block;
	byte *nullblock = NULL;
	mallocked = false;
	do {
		if (mallocked) {
			size_t page_size = getpagesize();
			size_t aligned_size =
				((size_t)impl->size + page_size - 1) & ~(page_size - 1);
			munmap(block, aligned_size);
		}
		block = ALOAD(impl->data[i] + j);
		if (block == NULL) {
			size_t page_size = getpagesize();
			int slots_per_page = page_size / impl->size;
			if (slots_per_page == 0) slots_per_page = 1;
			int alloc_size = sizeof(byte *) * (UINT32_MAX / (impl->size * 8)) /
							 slots_per_page;

			size_t aligned_size =
				((size_t)impl->size + page_size - 1) & ~(page_size - 1);
			block = mmap(NULL, aligned_size, PROT_READ | PROT_WRITE,
						 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			if (block == NULL) {
				SetErr(AllocErr);
				return -1;
			}
			mallocked = true;
		} else
			break;
	} while (!CAS(impl->data[i] + j, &nullblock, block));

	return 0;
}

Ptr memmap_allocate(MemMap *mm) {
	int i, j, k;
	unsigned long long *itt, nitt, v;
	MemMapImpl *impl;

	i = j = k = 0;
	impl = (MemMapImpl *)mm;

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
		if (memmap_check_data(impl, i, j, k)) return null;
	} while (!CAS(itt, &v, nitt));
	return ((i << 19) | (j << 6) | k);
}

void memmap_free(MemMap *mm, Ptr ptr) {
	if (ptr == null) panic("attempt to free null!");
	MemMapImpl *impl = (MemMapImpl *)mm;
	unsigned long long nv, *v, vo;
	do {
		v = impl->bitmap[ptr >> 19] + (ptr >> 6);
		vo = ALOAD((unsigned long long *)(v));
		if ((vo & (0x1ULL << (ptr & 0x3F))) == 0) panic("double free attempt!");
		nv = vo & ~(0x1ULL << (ptr & 0x3F));
	} while (!CAS(&*v, &vo, nv));
}
