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
#include <faml/faml.h>

MySuite(faml);

MyTest(faml, test_faml_objs) {
	FamlObj obj1, obj2;
	FamlPrototype proto1, proto2;
	cr_assert(!faml_prototype_create(&proto1, false, false, true));
	cr_assert(!faml_prototype_create(&proto2, false, false, true));

	faml_prototype_set_u8(&proto1, 21);

	cr_assert(!faml_build_obj(&obj1, NULL, false, false));
	cr_assert(!faml_build_obj(&obj2, NULL, false, false));

	faml_put(&obj1, "myval", 100);
	faml_put(&obj1, "myval2", 101);
	faml_put(&obj1, "test", 100LL);
	faml_put(&obj1, "obj", &obj2);

	cleanup_faml_data();
}

MyTest(faml, test_faml_objs2) {
}
