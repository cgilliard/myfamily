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

#include <base/chain_allocator.h>
#include <base/resources.h>
#include <errno.h>
#include <string.h>
#include <util/vec.h>

typedef struct VecImpl {
	void *data;
	u64 capacity;
	u64 cur;
	u64 size_of_entry;
} VecImpl;

void vec_cleanup(Vec *p)
{
	if (p->impl.data) {
		VecImpl *vi = p->impl.data;
		if (vi->data) {
			myfree(vi->data);
			vi->data = NULL;
		}
		chain_free(&p->impl);
	}
}

int vec_init(Vec *p, u64 initial_capacity, u64 size_of_entry)
{
	if (chain_malloc(&p->impl, sizeof(VecImpl))) {
		return -1;
	}

	VecImpl *vi = p->impl.data;
	if (initial_capacity) {
		vi->data = mymalloc(size_of_entry * initial_capacity);
		if (p->impl.data == NULL) {
			chain_free(&p->impl);
			return -1;
		}
	}

	vi->cur = 0;
	vi->capacity = initial_capacity;
	vi->size_of_entry = size_of_entry;

	return 0;
}
int vec_push(Vec *p, void *entry)
{
	VecImpl *vi = p->impl.data;
	if (vi->cur >= vi->capacity) {
		if (vec_resize(p, vi->capacity + 10)) {
			return -1;
		}
	}

	memcpy(vi->data + (vi->cur * vi->size_of_entry), entry, vi->size_of_entry);
	vi->cur++;
	return 0;
}
void *vec_element_at(Vec *p, u64 index)
{
	VecImpl *vi = p->impl.data;
	if (index >= vi->cur)
		return NULL;
	return vi->data + (index * vi->size_of_entry);
}
int vec_remove(Vec *p, u64 index)
{
	VecImpl *vi = p->impl.data;
	char *arr = (char *)vi->data;
	if (index >= vi->cur) {
		errno = EOVERFLOW;
		return -1;
	}
	char *start = arr + index * vi->size_of_entry;
	u64 bytes_to_move = (vi->cur - index - 1) * vi->size_of_entry;
	if (bytes_to_move > 0) {
		memmove(start, start + vi->size_of_entry, bytes_to_move);
	}
	vi->cur--;

	return 0;
}
void *vec_pop(Vec *p)
{
	VecImpl *vi = p->impl.data;
	if (vi->cur == 0)
		return NULL;
	void *ret = vec_element_at(p, vi->cur - 1);
	vi->cur--;
	return ret;
}
int vec_resize(Vec *p, u64 new_size)
{
	VecImpl *vi = p->impl.data;
	if (new_size < vi->cur)
		new_size = vi->cur;
	void *tmp = myrealloc(vi->data, new_size * vi->size_of_entry);
	if (tmp == NULL)
		return -1;
	vi->data = tmp;
	vi->capacity = new_size;

	return 0;
}
int vec_truncate(Vec *p, u64 new_size)
{
	VecImpl *vi = p->impl.data;
	if (new_size == 0) {
		myfree(vi->data);
	} else {
		void *tmp = myrealloc(vi->data, new_size * vi->size_of_entry);
		if (tmp == NULL)
			return -1;
		vi->data = tmp;
	}
	vi->capacity = new_size;
	return 0;
}
int vec_clear(Vec *p)
{
	return vec_truncate(p, 0);
}

u64 vec_size(Vec *p)
{
	VecImpl *vi = p->impl.data;
	return vi->cur;
}

u64 vec_capacity(Vec *p)
{
	VecImpl *vi = p->impl.data;
	return vi->capacity;
}
