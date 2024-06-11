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

#ifndef _VTABLE_BASE__
#define _VTABLE_BASE__

#include <base/types.h>

typedef struct {
	char *name;
	void *fn_ptr;
} VtableEntry;

typedef struct {
	u64 len;
	u128 id;
	VtableEntry entries[];
} Vtable;

typedef struct {
	Vtable *vtable;
} Object;

void *find_fn(Object *obj, const char *trait);

Vtable init_vtable(u64 len, VtableEntry entries[]);

// common fns
bool equal(void *obj1, void *obj2);
void *unwrap(void *obj);
void *unwrap_err(void *obj);
size_t size(void *obj);
void copy(void *dest, void *src);
void cleanup(void *ptr);

#endif // _VTABLE_BASE__
