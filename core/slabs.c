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

#include <core/result.h>
#include <core/slabs.h>
#include <stdlib.h>

GETTER(SlabData, data)
SETTER(SlabData, data)

GETTER(SlabData, sdp)
SETTER(SlabData, sdp)

GETTER(SlabAllocator, slab_data_arr);
SETTER(SlabAllocator, slab_data_arr);
GETTER(SlabAllocator, slab_data_arr_size);

GETTER(SlabSizeCount, slab_size);
GETTER(SlabSizeCount, slab_count);
GETTER(SlabSizeCount, max_slabs);

void SlabSizeCount_cleanup(SlabSizeCount *ptr) {}

bool SlabSizeCount_myclone(SlabSizeCount *dst, SlabSizeCount *src) {
	dst->_slab_size = src->_slab_size;
	dst->_slab_count = src->_slab_count;
	dst->_max_slabs = src->_max_slabs;
	return true;
}

void slab_write_ptr(char *bytes, u64 n) {
	for (int i = 7; i >= 0; i--) {
		bytes[i] = n & 0xFF;
		n = n >> 8;
	}
}

void slab_set_max(char *bytes) {
	for (int i = 0; i < 8; i++)
		bytes[i] = 0xFF;
}

Result slab_init_free_list(SlabData *sd, u64 slab_count, u64 slab_offset) {
	SlabDataParams p = GET(SlabData, sd, sdp);
	u64 slab_size = p.slab_size;
	for (u64 i = slab_offset; i < slab_offset + slab_count; i++) {
		char next_bytes[8];
		if (i < (slab_offset + slab_count) - 1) {
			slab_write_ptr(next_bytes, i + 1);
		} else {
			slab_set_max(next_bytes);
		}
		u64 offset_next = i * (8 + slab_size);
		Slice slice = SLICE(next_bytes, 8);
		Result r = SlabData_write(sd, &slice, offset_next);
		TRYU(r);
	}
	return Ok(_());
}

void SlabData_cleanup(SlabData *ptr) {
	void *d = GET(SlabData, ptr, data);
	if (d) {
		myfree(d);
		SET(SlabData, ptr, data, NULL);
	}
}

OrdOptions SlabData_cmp(const void *a, const void *b) {
	const SlabDataPtr *sa = a;
	const SlabDataPtr *sb = b;
	SlabDataParams params1 = sa->_sdp;
	SlabDataParams params2 = sb->_sdp;
	if (params1.slab_size < params2.slab_size)
		return LessThan;
	if (params1.slab_size > params2.slab_size)
		return GreaterThan;
	return EqualTo;
}

Result SlabData_init(SlabData *ptr, u64 initial_slabs, u64 slab_size,
		     u64 max_slabs) {
	void *d = mymalloc(slab_size * initial_slabs);
	SET(SlabData, ptr, data, d);

	SlabDataParams p;
	p.slab_size = slab_size;
	p.slab_count = initial_slabs;
	p.max_slabs = max_slabs;
	p.free_list_head = 0;

	SET(SlabData, ptr, sdp, p);

	Result r = slab_init_free_list(ptr, initial_slabs, 0);
	TRYU(r);

	return Ok(_());
}

Result SlabData_read(SlabData *ptr, Slice *slice, u64 offset) {
	SlabDataParams p = GET(SlabData, ptr, sdp);
	u64 slab_size = p.slab_size;
	memcpy(GET(Slice, slice, ref),
	       GET(SlabData, ptr, data) + (slab_size + 8) * offset, len(slice));
	return Ok(_());
}

Result SlabData_write(SlabData *ptr, Slice *slice, u64 offset) {
	SlabDataParams p = GET(SlabData, ptr, sdp);
	u64 slab_size = p.slab_size;
	memcpy(GET(SlabData, ptr, data) + (slab_size + 8) * offset,
	       GET(Slice, slice, ref), len(slice));
	return Ok(_());
}

Result SlabData_reference(SlabData *ptr, Slice *slice, u64 offset) {
	SET(Slice, slice, ref, GET(SlabData, ptr, data) + offset);
	return Ok(_());
}

Result SlabData_resize(SlabData *ptr, u64 slabs) {
	void *d = GET(SlabData, ptr, data);
	SlabDataParams p = GET(SlabData, ptr, sdp);
	u64 cur_slabs = p.slab_count;
	u64 slab_size = p.slab_size;

	void *nd = myrealloc(d, slabs * (slab_size + 8));
	if (nd == NULL) {
		return STATIC_ALLOC_RESULT;
	}
	SET(SlabData, ptr, data, nd);

	if (slabs > cur_slabs) {
		Result r =
		    slab_init_free_list(ptr, slabs - cur_slabs, cur_slabs);
		TRYU(r);
	}

	return Ok(_());
}

Result SlabAllocator_build_impl(int num, va_list ptr) {
	u64 count = 0;
	SlabDataPtr *slab_data_arr = NULL;
	u64 slabs_per_resize = 10;
	bool zeroed = false;
	for (int i = 0; i < num; i++) {
		SlabAllocatorOptionPtr next;
		Rc rc = va_arg(ptr, Rc);
		void *vptr = unwrap(&rc);
		memcpy(&next, vptr, size(vptr));
		// clang-format off
		MATCH(next,
			VARIANT(SLAB_OPTION_SIZE_COUNT, {
				SlabSizeCount sac = TRY_ENUM_VALUE(
					sac,
					SlabSizeCount,
					next
				);

				if(slab_data_arr) {
					void *tmp = myrealloc(slab_data_arr, sizeof(SlabDataPtr) * (count + 1));
					if(!tmp)
						return STATIC_ALLOC_RESULT;
					slab_data_arr = tmp;
				} else {
					slab_data_arr = mymalloc(sizeof(SlabDataPtr));
					if(!slab_data_arr)
						return STATIC_ALLOC_RESULT;
				}

				u64 slab_size = GET(SlabSizeCount, &sac, slab_size);
				u64 slab_count = GET(SlabSizeCount, &sac, slab_count);
				u64 max_slabs = GET(SlabSizeCount, &sac, max_slabs);
				SlabDataPtr *ptr = &slab_data_arr[count];
				BUILDPTR(ptr, SlabData);

				Result r = SlabData_init(
					&slab_data_arr[count],
					slab_size,
					slab_count,
					max_slabs
				);

				TRYU(r);
				count += 1;
			})
			VARIANT(SLAB_OPTION_ZEROED, {
				zeroed = TRY_ENUM_VALUE(zeroed, bool, next);
			})
			VARIANT(SLAB_OPTION_SLABS_PER_RESIZE, {
				slabs_per_resize = TRY_ENUM_VALUE(slabs_per_resize, u64, next);
			})
		);
		// clang-format on
	}

	if (count > 1) {
		myqsort(slab_data_arr, count);
	}

	for (int i = 0; i < count; i++) {
		printf("sz[%i] = %i\n", i, slab_data_arr[i]._sdp.slab_size);
	}

	SlabAllocator ret = BUILD(SlabAllocator, slab_data_arr, count, zeroed,
				  slabs_per_resize);
	return Ok(ret);
}

Result SlabAllocator_build(int n, ...) {
	va_list ptr;
	va_start(ptr, n);
	return SlabAllocator_build_impl(n, ptr);
}

Result SlabAllocator_allocate(SlabAllocator *ptr, Slice *slice,
			      u64 size){todo()}

Result SlabAllocator_get(SlabAllocator *ptr, Slice *slice, u64 id){todo()}

Result SlabAllocator_free(SlabAllocator *ptr, u64 id) {
	todo()
}

void SlabAllocator_cleanup(SlabAllocator *ptr) {
	SlabDataPtr *slab_data_arr = GET(SlabAllocator, ptr, slab_data_arr);
	u64 count = GET(SlabAllocator, ptr, slab_data_arr_size);
	if (slab_data_arr) {
		for (u64 i = 0; i < count; i++) {
			cleanup(&slab_data_arr[i]);
		}
		myfree(slab_data_arr);
		SET(SlabAllocator, ptr, slab_data_arr, NULL);
	}
}
