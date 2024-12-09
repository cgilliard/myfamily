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

#ifndef _BASE_LOCK__
#define _BASE_LOCK__

#include <base/types.h>

#define INIT_LOCK (0)

typedef i64 Lock;

#define lockr(l) lock_read(l)
#define lockw(l) lock_write(l)
#define locku(l) lock_upgrade(l)
#define lockd(l) lock_downgrade(l)
#define unlock(l) lock_unlock(l)

Lock lock_create();
void lock_read(Lock *lock);
void lock_write(Lock *lock);
void lock_unlock(Lock *lock);
void lock_upgrade(Lock *lock);
void lock_downgrade(Lock *lock);

#endif	// _BASE_LOCK__
