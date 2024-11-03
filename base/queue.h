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

#ifndef _BASE_QUEUE__
#define _BASE_QUEUE__

#include <base/macros.h>
#include <base/slabs.h>

Type(Queue);
#define Queue DefineType(Queue)

Queue queue_create();
int queue_enqueue(Ptr data);
Ptr queue_dequeue();

#endif	// _BASE_QUEUE__
