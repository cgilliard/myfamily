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

#include <base/types.h>

i64 saddi64(i64 a, i64 b) {
	if (a > 0) {
		if (b > INT64_MAX - a) {
			return INT64_MAX;
		}
	} else if (b < INT64_MIN - a) {
		return INT64_MIN;
	}

	return a + b;
}

u64 saddu64(u64 a, u64 b) {
	return (a > 0xFFFFFFFFFFFFFFFF - b) ? 0xFFFFFFFFFFFFFFFF : a + b;
}
