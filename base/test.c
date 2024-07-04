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

CLASS(MyClazz, FIELD(x) FIELD(y) FIELD(z), OFFSET(u64) OFFSET(u32) OFFSET(u64),
      TYPE(u64) TYPE(u32) TYPE(u64))
void MyClazz_cleanup(MyClazzPtr *ptr) {}

Test(base, test) {
	printf("sizeof=%i,obj=%i,obj1=%i\n", sizeof(Vdata), sizeof(Object),
	       sizeof(Object1));
}
