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

#include <base/macros.h>
#include <base/types.h>

Type(Object);
#define Object DefineType(Object)

Object object_create_int64(int64 value);
Object object_create_int(int value);
Object object_create_byte(byte value);
Object object_move(Object src);
Object object_ref(Object src);
int64 object_as_int64(const Object obj);
int object_as_int(const Object obj);
byte object_as_byte(const Object obj);
