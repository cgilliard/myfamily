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

#ifndef _UNIT_BASE__
#define _UNIT_BASE__

#include <base/vtable.h>

typedef struct Unit {
	Vtable *vtable;
} Unit;

bool unit_copy(Unit *dst, Unit *src);
size_t unit_size(Unit *u);
void unit_free(Unit *u);

// vtable
static VtableEntry UnitVtableEntries[] = {
    {"copy", unit_copy}, {"size", unit_size}, {"cleanup", unit_free}};

DEFINE_VTABLE(UnitVtable, UnitVtableEntries)

static Unit UNIT = {&UnitVtable};

#endif // _UNIT_BASE__
