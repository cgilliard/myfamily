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

#ifndef _BASE_ORBTREE__
#define _BASE_ORBTREE__

#include <base/macros.h>
#include <base/slabs.h>
#include <base/types.h>

Type(OrbTree);
#define OrbTree DefineType(OrbTree)

OrbTree orbtree_create();
const void *orbtree_get(const OrbTree tree, int64 namespace, int64 seqno, const char *name,
						int offset);
unsigned int orbtree_range_size(const OrbTree tree, int64 namespace);
int orbtree_remove(OrbTree tree, int64 namespace, int64 seqno, const char *name, int offset);
int orbtree_put(OrbTree tree, const char *name, int64 namespace, int64 seqno, Ptr value);

#endif // _BASE_ORBTREE__
