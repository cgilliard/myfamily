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

#ifndef _UTIL_OBJECT__
#define _UTIL_OBJECT__

#include <util/channel.h>
#include <util/rc.h>

#define INIT_OBJECT {.impl = rc_null}

typedef struct ObjectNc {
	Rc impl;
	FatPtr flags;
} ObjectNc;

void object_cleanup(ObjectNc *ptr);

#define Object ObjectNc __attribute__((warn_unused_result, cleanup(object_cleanup)))

int object_move(Object *dst, Object *src);
int object_ref(Object *dst, Object *src);
int object_mut_ref(Object *dst, Object *src);
int object_create(Object *obj, FatPtr data, void (*cleanup)(void *), bool send);
int object_send(Object *obj, Channel *channel);

#endif // _UTIL_OBJECT__
