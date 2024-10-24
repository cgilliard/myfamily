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

#include <base/test.h>

MySuite(base);

MyTest(base, test_init) {
	u64 as = alloc_sum();
	u64 rs = release_sum();
	u64 diff = as - rs;
	u8 *name = alloc(10, false);
	name[0] = 'a';
	name[1] = 0;
	DirectoryEntry *x = alloc(sizeof(DirectoryEntry), false);
	x[0].name = name;
	Directory d = {.count = 1, .entries = x};
}
