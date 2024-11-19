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

#include <base/block.h>

int _gfd = -1;

void __attribute__((constructor)) __init_blocks() {
}

int64 root_block() {
	return 0;
}

int64 allocate_block() {
	return 0;
}
void free_block(int64 id) {
}

int mark_block(int64 id) {
	return 0;
}
int flush(bool sync) {
	return 0;
}

void *fmap(int64 id) {
	return NULL;
}

void *fview(int64 id) {
	return NULL;
}
