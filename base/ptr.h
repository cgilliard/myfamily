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

#ifndef _BASE_PTR__
#define _BASE_PTR__

// Ptr data type
typedef struct Type *Ptr;

unsigned int ptr_len(const Ptr ptr);
unsigned int ptr_id(const Ptr ptr);
void *ptr_data(const Ptr ptr);
void *ptr_aux(const Ptr ptr);

// Direct alloc
Ptr ptr_direct_alloc(unsigned int size);
Ptr ptr_direct_resize(Ptr ptr, unsigned int size);
void ptr_direct_release(Ptr ptr);

#endif	// _BASE_PTR__