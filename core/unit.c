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

#include <core/unit.h>

void Unit_cleanup(UnitPtr *u) {}
bool Unit_myclone(UnitPtr *dst, UnitPtr *src) { return true; }
bool Unit_equal(UnitPtr *obj1, UnitPtr *obj2) { return true; }
