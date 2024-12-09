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

Suite(base);

Test(print) {
	char buf[100];
	/*
		assert_eq(sprint(buf, 100, "abc={}", 1), snprintf(buf, 100, "abc=%i",
	   1)); assert_eq(sprint(buf, 100, "abc={}", (u64)134), snprintf(buf, 100,
	   "abc=%llu", (u64)134));
	*/
	let x = $(1);
	assert_eq(sprint(buf, 100, "abc={},def={},ghi={},jkl={},mno={},xyz=0",
					 (u64)134, -33, 1.5, "abcdefghi", x),
			  snprintf(buf, 100, "abc=%llu,def=%i,ghi=%f,jkl=%s,mno=1,xyz=0",
					   (u64)134, -33, 1.5, "abcdefghi"));
}
