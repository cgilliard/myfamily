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

bool errorkind_equal(ErrorKind *kind1, ErrorKind *kind2) {
	return strcmp(kind1->type_str, kind2->type_str) == 0;
}

void error_free(ErrorImpl *err) {}

bool error_equal(Error *e1, Error *e2) {
	// only compare kinds of errors, not message
	return errorkind_equal(&e1->kind, &e2->kind);
}

char *error_to_string(char *s, Error *e) {
	strcpy(s, e->kind.type_str);
	strcat(s, ": ");
	strcat(s, e->msg);
	return s;
}
