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

#include <base/os.h>

void *alloc(u64 size, bool zeroed) {
	return NULL;
}
void *resize(void *, u64 size) {
	return NULL;
}

void release(void *) {
}

u64 alloc_sum() {
	return 0;
}
u64 resize_sum() {
	return 0;
}
u64 release_sum() {
	return 0;
}

// Persistence
void persistent_set_root(const u8 *path) {
}
void *persistent_alloc(const u8 *name, u64 size, bool zeroed) {
	return NULL;
}
void *persistent_resize(const u8 *name, u64 size) {
	return NULL;
}
void persistent_sync(void *ptr) {
}
void persistent_delete(const u8 *name) {
}

u8 *env(const u8 *name) {
	return NULL;
}
