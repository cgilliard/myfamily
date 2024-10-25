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

#include <base/path.h>

i32 path_for(Path *dst, const u8 *path) {
	return 0;
}
i32 path_canonicalize(Path *p) {
	return 0;
}
i32 path_push(Path *p, const u8 *next) {
	return 0;
}
i32 path_pop(Path *p) {
	return 0;
}
u8 *path_to_string(const Path *p) {
	return NULL;
}
bool path_exists(const Path *p) {
	return false;
}
bool path_is_dir(const Path *p) {
	return false;
}
bool path_mkdir(Path *p, bool parent) {
	return false;
}
const u8 *path_file_name(const Path *p) {
	return NULL;
}
i32 path_file_stem(const Path *p, u8 *buf, u64 limit) {
	return 0;
}
i32 path_copy(Path *dst, const Path *src) {
	return 0;
}
u64 path_file_size(Path *p) {
	return 0;
}
