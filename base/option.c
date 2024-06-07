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
#include <stdlib.h>
#include <string.h>
#include <base/rand.h>
#include <base/panic.h>

bool option_is_some_false() {
        return false;
}

bool option_is_some_true() {
        return true;
}

Option None = { option_is_some_false, NULL };

void *option_unwrap(Option x) {
	if(!x.is_some()) {
		panic("attempt to unwrap on a None");
	} else {
		return x.ref;
	}
}

Option option_build(OptionImpl *opt, void *x, size_t size) {
	(*opt).is_some = option_is_some_true;
	(*opt).ref = malloc(size);
	memcpy((*opt).ref, x, size);
	return *opt;
}

void option_free(OptionImpl *ptr) {
	if(ptr->ref) {
		free(ptr->ref);
		ptr->ref = NULL;
	}
}
