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

#include <core/slice.h>

GETTER(Slice, len)

void Slice_cleanup(Slice *rh) {}

u64 Slice_len(Slice *rh) {
	u64 ret = GET(Slice, rh, len);
	return ret;
}

Slice Slice_build(void *ptr, u64 size) {
	Slice ret = BUILD(Slice, ptr, size);
	return ret;
}

