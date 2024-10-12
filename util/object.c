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

#include <base/bitflags.h>
#include <base/fam_alloc.h>
#include <base/fam_err.h>
#include <base/resources.h>
#include <stdio.h>
#include <util/object.h>
#include <util/rc.h>

#define OBJECT_FLAG_CONSUMED 1
#define OBJECT_FLAG_NO_CLEANUP 2
#define OBJECT_FLAG_SEND 3
#define OBJECT_FLAG_IMUT 4

typedef struct ObjectImpl {
	FatPtr self;
	FatPtr data;
	void (*cleanup)(void *);
	u8 flags;
} ObjectImpl;

typedef struct ObjectFlags {
	u8 flags;
} ObjectFlags;

void object_cleanup_rc(ObjectImpl *impl) {
	printf("rc obj cleanup\n");
	impl->cleanup($(impl->data));

	if (!nil(impl->self)) {
		fam_free(&impl->self);
		impl->self = null;
	}
	if (!nil(impl->data)) {
		fam_free(&impl->data);
		impl->data = null;
	}
}

void object_cleanup(ObjectNc *ptr) {
	if (ptr == NULL)
		return;
	printf("obj cleanup\n");
	if (!nil(ptr->flags)) {
		fam_free(&ptr->flags);
		ptr->flags = null;
	}

	if (nil(ptr->impl))
		return;
	ObjectImpl *impl = $(ptr->impl);
	if (!nil(ptr->impl)) {
		rc_cleanup(&ptr->impl);
		ptr->impl.impl = null;
	}
}

int object_move(Object *dst, Object *src) {
	dst->flags = src->flags;
	dst->impl = src->impl;
	src->impl = rc_null;
	src->flags = null;
	return 0;
}

int object_ref_impl(Object *dst, Object *src, bool imut) {
	FatPtr flags;
	bool send;
	bool src_imut;
	ObjectFlags *src_flags = $(src->flags);
	BitFlags bfsrc = {.flags = &src_flags->flags, .capacity = 1};
	send = bitflags_check(&bfsrc, OBJECT_FLAG_SEND);
	src_imut = bitflags_check(&bfsrc, OBJECT_FLAG_IMUT);
	if (src_imut && !imut) {
		fam_err = Permission;
		return -1;
	}
	{
		SendStateGuard _ = SetSend(send);
		if (fam_alloc(&flags, sizeof(ObjectFlags))) {
			return -1;
		}
	}
	ObjectFlags *ff = $(flags);
	ff->flags = 0;
	BitFlags bfdst = {.flags = &ff->flags, .capacity = 1};
	bitflags_set(&bfdst, OBJECT_FLAG_SEND, send);
	bitflags_set(&bfdst, OBJECT_FLAG_IMUT, imut);

	dst->flags = flags;
	int ret = rc_clone(&dst->impl, &src->impl);
	if (ret) {
		fam_free(&flags);
	}
	return ret;
}
int object_ref(Object *dst, Object *src) {
	return object_ref_impl(dst, src, true);
}
int object_mut_ref(Object *dst, Object *src) {
	return object_ref_impl(dst, src, false);
}
int object_create(Object *obj, FatPtr data, void (*cleanup)(void *), bool send) {
	if (obj == NULL || nil(data)) {
		fam_err = IllegalArgument;
		return -1;
	}

	FatPtr ptr;
	FatPtr flags;
	{
		SendStateGuard _ = SetSend(send);
		if (fam_alloc(&ptr, sizeof(ObjectImpl))) {
			return -1;
		}
		if (fam_alloc(&flags, sizeof(ObjectFlags))) {
			fam_free(&ptr);
			return -1;
		}
	}
	ObjectImpl *impl = $(ptr);
	ObjectFlags *obj_flags = $(flags);
	impl->cleanup = cleanup;
	impl->self = ptr;
	impl->data = data;
	obj->flags = flags;

	obj_flags->flags = 0;
	BitFlags bf = {.flags = &obj_flags->flags, .capacity = 1};
	bitflags_set(&bf, OBJECT_FLAG_SEND, send);

	if (rc_build(&obj->impl, impl, sizeof(ObjectImpl), false, (void (*)(void *))object_cleanup_rc))
		return -1;

	return 0;
}
int object_send(Object *obj, Channel *channel) {
	return 0;
}
