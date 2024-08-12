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

#include <core/option.h>
#include <core/string.h>

bool Option_myclone(Option *dst, Option *src) {
	if (IS_NONE(*src)) {
		*dst = None;
	} else {
		void *data = src->slab.data;
		Rc sv = *(Rc *)unwrap(src->slab.data);
		String sv2 = *(String *)unwrap(&sv);
		RcPtr rc = BUILD(Rc);
		deep_copy(&rc, &sv);
		*dst = BUILD_ENUM(Option, SOME, rc);
		StringPtr out = ENUM_VALUE(out, String, *dst);
	}
	return true;
}
