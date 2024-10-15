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

#define $rc_impl(v)                                                                                \
	_Generic((v), FatPtr: fat_ptr_data((FatPtr *)&v), Rc: rc_access((Rc *)&v), default: ({}))

#ifndef $object_impl
#undef $
#define $(v) $rc_impl(v)
#endif // $object_impl

#ifndef obj_nil
#undef nil
#define nil(v)                                                                                     \
	_Generic((v),                                                                                  \
		FatPtr: fat_ptr_flag_get((FatPtr *)&v, FAT_PTR_FLAG_NIL),                                  \
		Rc: fat_ptr_flag_get(&((Rc *)&v)->impl, FAT_PTR_FLAG_NIL))
#endif // obj_nil

// Reference counter type
typedef struct RcNc {
	FatPtr impl;
} RcNc;

// Weak reference type
typedef struct WeakNc {
	FatPtr impl;
} WeakNc;

// meta data associated with the Rc.
typedef struct RcMetaV {
	u32 weak_count;			 // number of weak references
	u32 strong_count;		 // number of strong references
	bool atomic;			 // whether or not this is an atomic reference counter
	void (*cleanup)(void *); // cleanup function to call when last Rc goes out of scope
	u64 size;				 // size of data
} RcMetaV;

typedef struct RcNilImpl {
	RcMetaV meta; // meta data for this Rc
	u8 data[];	  // data held in the Rc
} RcNilImpl;

// Global constant that points to a all ones implementation of FatPtrImpl (so all bit flags are set)
static const RcNilImpl rc_impl_null = {};

#define rc_null                                                                                    \
	(const Rc) {                                                                                   \
		.impl = null                                                                               \
	}

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
