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

#include <base/class.h>
#include <criterion/criterion.h>

CLASS(MyClazz,
      FIELD(a) FIELD(b) FIELD(c) FIELD(d) FIELD(e) FIELD(f) FIELD(g) FIELD(h)
	  FIELD(i) FIELD(j) FIELD(k) FIELD(l),
      OFFSET(i8) OFFSET(i16) OFFSET(i32) OFFSET(i64) OFFSET(i128) OFFSET(u8)
	  OFFSET(u16) OFFSET(u32) OFFSET(u64) OFFSET(u128) OFFSET(usize)
	      OFFSET(bool),
      TYPE(i8) TYPE(i16) TYPE(i32) TYPE(i64) TYPE(i128) TYPE(u8) TYPE(u16)
	  TYPE(u32) TYPE(u64) TYPE(u128) TYPE(usize) TYPE(bool))

void MyClazz_cleanup(MyClazzPtr *ptr) {}

Test(base, test) {
	printf("sizeof=%i,obj=%i,obj1=%i\n", sizeof(Vdata), sizeof(Object),
	       sizeof(Object1));
}
