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

#ifndef _UTIL_RC__
#define _UTIL_RC__

#include <base/slabs.h>

// Reference counter type
typedef struct RcNc {
	FatPtr impl;
} RcNc;

// Weak reference type
typedef struct WeakNc {
	FatPtr impl;
} WeakNc;

// internal cleanup function for Rcs
void rc_cleanup(RcNc *ptr);

#define Rc RcNc __attribute__((warn_unused_result, cleanup(rc_cleanup)))

// internal cleanup function for Weaks
void weak_cleanup(WeakNc *ptr);

#define Weak WeakNc __attribute__((warn_unused_result, cleanup(weak_cleanup)))

// Build a reference counted object around the specified memory location.
int rc_build(Rc *ptr, void *data, u64 size, bool atomic, void (*cleanup)(void *));

// Clone the src reference counter incrementing its internal counter.
int rc_clone(Rc *dst, const Rc *src);

void *rc_access(Rc *ptr);

// Make a weak reference to the reference counter object.
int rc_weak(Weak *dst, const Rc *src);

// Upgrade the weak reference to a reference counter object if the underlying reference
// counter still exists.
int rc_upgrade(Rc *upgraded, Weak *ptr);

#endif // _UTIL_RC__
