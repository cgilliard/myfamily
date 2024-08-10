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

#include <core/tuple.h>

GETTER(Tuple, count)
SETTER(Tuple, count)
GETTER(Tuple, elements)
SETTER(Tuple, elements)

void Tuple_cleanup(Tuple *tuple) {
	u64 count = *Tuple_get_count(tuple);

	if (count) {
		Slab slab = GET(Tuple, tuple, elements);
		Slab *elements = slab.data;
		for (u64 i = 0; i < count; i++) {
			cleanup(elements[i].data);
			myfree(&elements[i]);
		}

		myfree(&slab);
		SET(Tuple, tuple, count, 0);
	}
}

bool Tuple_myclone(Tuple *dst, Tuple *src) {
	u64 count_src = GET(Tuple, src, count);
	Slab src_slab = GET(Tuple, src, elements);
	Slab *src_elements = src_slab.data;
	Slab dst_slab;
	if (mymalloc(&dst_slab, sizeof(Slab) * count_src)) {
		Tuple_set_count(dst, 0);
		return false;
	}
	Slab *element_arr = dst_slab.data;
	for (u64 i = 0; i < count_src; i++) {
		mymalloc(&element_arr[i], mysize(src_elements[i].data));
		if (element_arr[i].data == NULL) {
			for (u64 j = i - 1; j >= 0; j--) {
				myfree(&element_arr[j]);
			}
			myfree(&dst_slab);
			Tuple_set_count(dst, 0);
			return false;
		}
		if (implements(src_elements[i].data, "deep_copy")) {
			deep_copy(element_arr[i].data, src_elements[i].data);
		} else {
			copy(element_arr[i].data, src_elements[i].data);
		}
	}

	SET(Tuple, dst, elements, dst_slab);
	SET(Tuple, dst, count, count_src);
	return true;
}

u64 Tuple_len(Tuple *tuple) {
	u64 count = *Tuple_get_count(tuple);
	return count;
}

void *Tuple_element_at(Tuple *tuple, u64 index, void *dst) {
	u64 count = *Tuple_get_count(tuple);
	if (index >= count)
		panic("Attempt to access element index %llu on a tuple of size "
		      "%llu",
		      index, count);
	Slab elements = GET(Tuple, tuple, elements);
	Slab *elements_ptr = elements.data;
	Slab ref = elements_ptr[index];

	char *cn = CLASS_NAME(ref.data);
	if (!strcmp(cn, "U64")) {
		void *dst_ref = unwrap(ref.data);
		memcpy(dst, dst_ref, sizeof(u64));
	} else {
		// it's an object, we'll call copy which must be implemented
		if (!strcmp(cn, "Rc")) {
			void *tmp = unwrap(ref.data);
			memcpy(dst, tmp, mysize(tmp));
		} else {
			if (!copy(dst, ref.data))
				panic("Could not copy object");
		}
	}

	return dst;
}

Slab Tuple_add_value(Rc rc) {
	Slab slab;
	if (mymalloc(&slab, mysize(&rc))) {
		slab.data = NULL;
		return slab;
	}
	memcpy(slab.data, &rc, sizeof(Rc));

	return slab;
}

Slab Tuple_add_value_u64(void *value) {
	Slab slab;
	if (mymalloc(&slab, sizeof(U64))) {
		slab.data = NULL;
		return slab;
	}
	Object *obj = slab.data;
	BUILDPTR(obj, U64);
	((U64Ptr *)(slab.data))->_value = *(u64 *)value;
	return slab;
}

