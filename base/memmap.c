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
	byte **data;
	unsigned long long *bitmap;
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
	return (void *)(impl->data[(ptr >> 6)] + ((ptr & 0x3F) * impl->size));
}
int memmap_init(MemMap *mm, unsigned int size) {
	MemMapImpl *impl = (MemMapImpl *)mm;
	impl->size = size;
	impl->chunks = 0;
	impl->chunk_size = 128;
	impl->bitmap = NULL;
	impl->data = NULL;
	impl->lock = INIT_LOCK;

	impl->bitmap = malloc(UINT32_MAX / (size * 8));
	memset(impl->bitmap, '\0', UINT32_MAX / (size * 8));
	// set first bit as allocated to ensure null is never returned.
	((byte *)impl->bitmap)[0] = 0x1;
	return null;
}
Ptr memmap_allocate(MemMap *mm) {
	unsigned int i, j;
	unsigned long long *itt, nitt, v;
	MemMapImpl *impl;

	impl = (MemMapImpl *)mm;
	i = 0;

	do {
		// create our 64 bit iterator for the bitmap
		itt = impl->bitmap;
		// initialize j to 0, i remains at the previous value before the CAS
		// failure to avoid looping through allocated blocks again.
		j = 0;

		// iterate through the bitmap until we find any 0 bits
		loop {
			// This condition checks if there are any 0 bits in our bitmap
			// if so we break, otherwise we keep iterating
			if ((v = ALOAD(itt)) != ((unsigned long long)0) - 1) break;
			// inrement iterator because we did not find a 0 bit
			itt++;
			// increment i and do a bounds check. If we've gone through the list
			// and still not found anything we have to return null '0' (which is
			// reserved in the previous function so that it is a unique value.
			if (++i >= UINT32_MAX / (impl->size * 8)) return null;
		}

		// we know at least one of these bits is not 0 so we loop through until
		// we find it
		loop {
			// check for 0 bit and exit loop
			if ((v & (0x1ULL << j)) == 0) break;
			if (++j >= 8 * sizeof(unsigned long long))
				continue;  // a modification must have occurred
		}

		// Set the value of 'nitt' (new itt) to it's current value with the jth
		// bit set.
		nitt = v | (0x1ULL << j);
		// CAS if the value of itt has not changed (meaning this bit is still
		// not allocated), we update the value to 'nitt' which has the bit we
		// are reserving set.

		// before we commit, make sure the memory is allocated
		byte **data;
		bool mallocked = false;
		byte **nulldata = NULL;

		do {
			// if the CAS fails, deallocate memory
			if (mallocked) free(data);
			data = ALOAD(&impl->data);
			// if data is null we need to allocate it
			if (data == NULL) {
				int page_size = getpagesize();
				int slots_per_page = page_size / impl->size;
				if (slots_per_page == 0) slots_per_page = 1;
				int alloc_size = sizeof(byte *) *
								 (UINT32_MAX / (impl->size * 8)) /
								 slots_per_page;
				data = malloc(alloc_size);
				if (data == NULL) {
					SetErr(AllocErr);
					return null;
				}
				memset(data, '\0', alloc_size);
				mallocked = true;
			} else
				break;
		} while (!CAS(&impl->data, &nulldata, data));

		// now check our block
		byte *block;
		byte *nullblock = NULL;
		do {
			block = ALOAD(&impl->data[i]);
			if (block == NULL) {
				size_t page_size = getpagesize();
				size_t aligned_size =
					((size_t)impl->size + page_size - 1) & ~(page_size - 1);
				block = mmap(NULL, aligned_size, PROT_READ | PROT_WRITE,
							 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			}
		} while (!CAS(&impl->data[i], &nullblock, block));

	} while (!CAS(itt, &v, nitt));

	// return the bitwise shifted value of i << 6 | j. This is the bit which is
	// assigned to us based on the 64 bit shift of 6 bytes and the value of j
	return ((i << 6) | j);
}

void memmap_free(MemMap *mm, Ptr ptr) {
	if (ptr == null) panic("attempt to free null!");
	MemMapImpl *impl = (MemMapImpl *)mm;
	unsigned long long nv, *v, vo;
	do {
		// point to the appropriate unsigned long long location in the bitmap
		v = impl->bitmap + (ptr >> 6);
		// atomically load it
		vo = ALOAD((unsigned long long *)(v));

		// check for double free (unallocated free)
		if ((vo & (0x1ULL << (ptr & 0x3F))) == 0) panic("double free attempt!");

		// update our value unsetting the appropriate bit
		nv = vo & ~(0x1ULL << (ptr & 0x3F));
		// repeate our cas loop if *v changes while we're writing
	} while (!CAS(&*v, &vo, nv));
}
