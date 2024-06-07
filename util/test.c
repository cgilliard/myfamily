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

#include <criterion/criterion.h>
#include <util/slabio.h>
#include <util/slabs.h>
#include <util/slabs_test.h>
#include <util/option.h>
#include <base/rand.h>
#include <base/types.h>
#include <limits.h>
#include <log/log.h>

#define LOG_LEVEL Debug

Test(util, slab_resize) {
	SlabAllocatorConfig sc;
        slab_allocator_config_slab_data(&sc, 11, 512);
	SlabAllocatorConfig sc2;
	slab_allocator_config_slabs_per_resize(&sc2, 5);

	// slabs returned in order
        SlabAllocator sa;
        slab_init(&sa, 2, sc, sc2);
        u64 slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, 0);
        debug("==========>slab_id=%llu", slab);
        slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, 1);
        debug("==========>slab_id=%llu", slab);
        u64 to_free;
        slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, 2);
        to_free = slab;
        debug("==========>slab_id=%llu", slab);
        slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, 3);
        debug("==========>slab_id=%llu", slab);
        slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, 4);
        debug("==========>slab_id=%llu", slab);
	slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, 5);
        debug("==========>slab_id=%llu", slab);
	slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, 6);
        debug("==========>slab_id=%llu", slab);
	debug("==========>to_free=%llu", to_free);
	slab_free(&sa, to_free);
	slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, 2);
        debug("==========>slab_id=%llu", slab);
	slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, 7);
        debug("==========>slab_id=%llu", slab);
	slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, 8);
        debug("==========>slab_id=%llu", slab);
	slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, 9);
        debug("==========>slab_id=%llu", slab);
	slab = slab_allocate(&sa, 512);
	debug("v=%llu", slab);
	cr_assert_eq(slab, 10);
        debug("==========>slab_id=%llu", slab);
	slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, ULONG_MAX);
        debug("==========>slab_id=%llu", slab);
	slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, ULONG_MAX);
        debug("==========>slab_id=%llu", slab);
	slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, ULONG_MAX);
        debug("==========>slab_id=%llu", slab);
	slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, ULONG_MAX);
        debug("==========>slab_id=%llu", slab);
	debug("==========>to_free=%llu", to_free);
	slab_free(&sa, to_free);
	slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, to_free);
        debug("==========>slab_id=%llu", slab);
	slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, ULONG_MAX);
        debug("==========>slab_id=%llu", slab);

	Slab s1;
	slab_read(&sa, 1, &s1);

	((char*)s1.data)[0] = 100;

	Slab s2;
	slab_read(&sa, 1, &s2);

	cr_assert_eq(((char*)s2.data)[0], 100);

	Slab s3;
	s3.data = malloc(sizeof(char) * 512);
	((char*)s3.data)[0] = 101;
	s3.len = 512;

	slab_write(&sa, 4, &s3, 1);

	free(s3.data);

	Slab s4;
	slab_read(&sa, 4, &s4);
	for(int i=0; i<10; i++) {
		debug("s4.data[%d]=%d", i, ((char*)s4.data)[i]);
	}
	cr_assert_eq(((char*)s4.data)[1], 101);


}

Test(util, slab_index_for_size) {
	SlabAllocatorConfig sc1;
        slab_allocator_config_slab_data(&sc1, 10, 10);
	SlabAllocatorConfig sc2;
        slab_allocator_config_slab_data(&sc2, 10, 20);
	SlabAllocatorConfig sc3;
        slab_allocator_config_slab_data(&sc3, 10, 30);
	SlabAllocatorConfig sc4;
        slab_allocator_config_slab_data(&sc4, 10, 40);
	SlabAllocatorConfig sc5;
        slab_allocator_config_slab_data(&sc5, 10, 50);
	SlabAllocatorConfig sc6;
        slab_allocator_config_slab_data(&sc6, 10, 60);
	SlabAllocatorConfig sc7;
        slab_allocator_config_slab_data(&sc7, 10, 70);
	SlabAllocatorConfig sc8;
        slab_allocator_config_slab_data(&sc8, 10, 80);
	SlabAllocatorConfig sc9;
        slab_allocator_config_slab_data(&sc9, 10, 90);
	SlabAllocatorConfig sc10;
        slab_allocator_config_slab_data(&sc10, 10, 100);


        SlabAllocator sa;
        slab_init(&sa, 10, sc1, sc2, sc3, sc4, sc5, sc6, sc7, sc8, sc9, sc10);

	cr_assert_eq(slab_index_for_size(&sa, 10), 0);
	cr_assert_eq(slab_index_for_size(&sa, 20), 1);
	cr_assert_eq(slab_index_for_size(&sa, 30), 2);
	cr_assert_eq(slab_index_for_size(&sa, 40), 3);
	cr_assert_eq(slab_index_for_size(&sa, 50), 4);
	cr_assert_eq(slab_index_for_size(&sa, 60), 5);
	cr_assert_eq(slab_index_for_size(&sa, 70), 6);
	cr_assert_eq(slab_index_for_size(&sa, 80), 7);
	cr_assert_eq(slab_index_for_size(&sa, 90), 8);
	cr_assert_eq(slab_index_for_size(&sa, 100), 9);
	cr_assert_eq(slab_index_for_size(&sa, 101), -1);
	cr_assert_eq(slab_index_for_size(&sa, 1), -1);
	cr_assert_eq(slab_index_for_size(&sa, 11), -1);
	cr_assert_eq(slab_index_for_size(&sa, 21), -1);
	cr_assert_eq(slab_index_for_size(&sa, 31), -1);
	cr_assert_eq(slab_index_for_size(&sa, 41), -1);
	cr_assert_eq(slab_index_for_size(&sa, 51), -1);
	cr_assert_eq(slab_index_for_size(&sa, 61), -1);
	cr_assert_eq(slab_index_for_size(&sa, 71), -1);
	cr_assert_eq(slab_index_for_size(&sa, 81), -1);
	cr_assert_eq(slab_index_for_size(&sa, 91), -1);

	u64 slab;
	slab = slab_allocate(&sa, 10);
	debug("slab=%llu", slab);
	slab = slab_allocate(&sa, 10);
        debug("slab=%llu", slab);
	slab = slab_allocate(&sa, 10);
        debug("slab=%llu", slab);
	debug("slab free %llu", slab);
	slab_free(&sa, slab);
	slab = slab_allocate(&sa, 10);
        debug("slab=%llu", slab);
        slab = slab_allocate(&sa, 10);
        debug("slab=%llu", slab);

	slab = slab_allocate(&sa, 20);
        debug("slab=%llu", slab);
        slab = slab_allocate(&sa, 20);
        debug("slab=%llu", slab);
        slab = slab_allocate(&sa, 20);
        debug("slab=%llu", slab);
        debug("slab free %llu", slab);
        slab_free(&sa, slab);
        slab = slab_allocate(&sa, 20);
        debug("slab=%llu", slab);
        slab = slab_allocate(&sa, 20);
        debug("slab=%llu", slab);
}

Test(util, slabs) {
	SlabAllocatorConfig sc;
	slab_allocator_config_slab_data(&sc, 1000, 512);

	SlabAllocator sa;
	slab_init(&sa, 1, sc);
	u64 slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, 0);
	debug("slab_id=%llu", slab);
	slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, 1);
        debug("slab_id=%llu", slab);
	u64 to_free;
 	slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, 2);
	to_free = slab;
        debug("slab_id=%llu", slab);
	slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, 3);
        debug("slab_id=%llu", slab);
	slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, 4);
        debug("slab_id=%llu", slab);

	debug("to_free=%llu", to_free);
	slab_free(&sa, to_free);

	slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, to_free);
        debug("slab_id=%llu", slab);
	slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, 5);
        debug("slab_id=%llu", slab);
	slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, 6);
        debug("slab_id=%llu", slab);

	debug("to_free=1");
	slab_free(&sa, 1);

	slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, 1);
        debug("slab_id=%llu", slab);
        slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, 7);
        debug("slab_id=%llu", slab);
        slab = slab_allocate(&sa, 512);
	cr_assert_eq(slab, 8);
        debug("slab_id=%llu", slab);


}

Test(util, slab_allocator_config) {
	SlabAllocatorConfig sc1, sc2, sc3;

	slab_allocator_config_zeroed(&sc1, false);
	slab_allocator_config_slabs_per_resize(&sc2, 100);
	slab_allocator_config_slab_data(&sc3, 100, 512);

	cr_assert_eq(sc1.type, SlabAllocatorConfigImplTypeZeroed);
	cr_assert_eq(sc2.type, SlabAllocatorConfigImplTypeSlabsPerResize);
	cr_assert_eq(sc3.type, SlabAllocatorConfigImplTypeSlabData);

	cr_assert_eq(*((bool *)sc1.value), false);
	cr_assert_eq(*((u64 *)sc2.value), 100);
	u64 *test = sc3.value;
	cr_assert_eq(test[0], 100);
	cr_assert_eq(test[1], 512);
}

Test(util, slab_allocator_init) {
	SlabAllocatorConfig sc1;
	SlabAllocatorConfig sc2;
	SlabAllocatorConfig sc3;
	SlabAllocatorConfig sc4;
	SlabAllocator sa1, sa2, sa3, sa4;
	slab_allocator_config_zeroed(&sc1, false);
	slab_allocator_config_zeroed(&sc2, true);
	slab_allocator_config_slabs_per_resize(&sc3, 123);
	slab_allocator_config_slab_data(&sc4, 999, 888);

	slab_init(&sa1, 1, sc1);
	cr_assert_eq(sa1.zeroed, false);

	slab_init(&sa2, 1, sc2);
        cr_assert_eq(sa2.zeroed, true);
	cr_assert_eq(sa2.slabs_per_resize, 100);

	slab_init(&sa3, 1, sc3);
	cr_assert_eq(sa3.zeroed, true);
	cr_assert_eq(sa3.slabs_per_resize, 123);


	printf("sc4.type=%u,size=%llu\n", sc4.type, ((u64 *)sc4.value)[0]);
	slab_init(&sa4, 1, sc4);
	cr_assert_eq(sa4.zeroed, true);
        cr_assert_eq(sa4.slabs_per_resize, 100);
	printf("sizes=%llu\n", sa4.sizes[0]);
	cr_assert_eq(sa4.sizes[0], 888);
	printf("max=%llu\n", sa4.max_slabs[0]);
	cr_assert_eq(sa4.max_slabs[0], 999);
}

Test(util, slabio) {
	SlabAllocator sa;
	SlabReader sr;
	SlabWriter sw;

	SlabAllocatorConfig sc;
        slab_allocator_config_slab_data(&sc, 10, 52);

	slab_init(&sa, 1, sc);
	slab_reader_init(&sr, &sa);
	slab_writer_init(&sw, &sa);

        Reader reader = READER(slab_reader_read_fixed_bytes, &sr);
        Writer writer = WRITER(slab_writer_write_fixed_bytes, &sw);

	u64 root = slab_allocate(&sa, 52);
	debug("root=%llu", root);
	slab_reader_seek(&reader, root, 0);
	slab_writer_seek(&writer, root, 0);

	String s_in, s_out;
        string_set(&s_in, "012345678901234567890123456789012345678901234567890012345678901234567890123");

	u128 r_in = 0, r_out = 0;
        int v = rand_u128(&r_in);
	u64 low = (u64)r_in;
	char buf1[] = { "18446744073709551615" };
	sprintf(buf1, "llu%", low);
	u64 high = (u64)(r_in >> 64);
	char buf2[] = { "18446744073709551615" };
        sprintf(buf2, "llu%" , high);
	debug("r_in=%llu%llu", high, low);

	Serializable s1 = SER(s_in);
	Serializable s2 = SER(s_out);
	Serializable r1 = SER(r_in);
	Serializable r2 = SER(r_out);
	serialize(&s1, &writer);
	serialize(&r1, &writer);
	deserialize(&s2, &reader);
	deserialize(&r2, &reader);

	debug("s_in.ptr=%s", s_in.ptr);
	debug("s_out.ptr=%s", s_out.ptr);
	cr_assert_eq(strcmp(s_in.ptr, s_out.ptr), 0);
	cr_assert_eq(r_in, r_out);

}

Test(util, over_read) {
	SlabAllocator sa;
        SlabReader sr;
        SlabWriter sw;

        SlabAllocatorConfig sc;
        slab_allocator_config_slab_data(&sc, 10, 52);

        slab_init(&sa, 1, sc);
        slab_reader_init(&sr, &sa);
        slab_writer_init(&sw, &sa);

        Reader reader = READER(slab_reader_read_fixed_bytes, &sr);
        Writer writer = WRITER(slab_writer_write_fixed_bytes, &sw);

        u64 root = slab_allocate(&sa, 52);
        debug("root=%llu", root);
        slab_reader_seek(&reader, root, 0);
        slab_writer_seek(&writer, root, 0);

	String s_in, s_out;
        string_set(&s_in, "01234567890123456789012345678901234501234567890123456789012345678901234567890123");
	u64 dummy_in;

	Serializable s1 = SER(s_in);
        Serializable s2 = SER(s_out);
	Serializable dummy = SER(dummy_in);
        serialize(&s1, &writer);
        deserialize(&s2, &reader);

	debug("over_read s_in.ptr=%s", s_in.ptr);
        debug("over_read s_out.ptr=%s", s_out.ptr);
        cr_assert_eq(strcmp(s_in.ptr, s_out.ptr), 0);

	int v = deserialize(&dummy, &reader);
	cr_assert_neq(v, 0);

}

Test(util, option) {
	Option opt = None;
	cr_assert_eq(opt.is_some(), false);
	debug("opt.is_some=%d,ref=%i", opt.is_some(), opt.ref);
	int x = 2;
	Option opt2 = Some(&opt2, x);
	cr_assert_eq(opt2.is_some(), true);
	int *ptr = Unwrap(opt2);
	debug("ptr_value=%i", *ptr);
	String s;
	string_set(&s, "this is a test");
	Option opt3 = Some(&opt3, s);
	cr_assert_eq(opt3.is_some(), true);
	void *str = Unwrap(opt3);
	debug("str='%s'", ((String*)str)->ptr);
	cr_assert_eq(strcmp(((String*)str)->ptr, "this is a test"), 0);

	char *ptr1 = opt3.is_some() ? ((String*)Unwrap(opt3))->ptr : "my default str";
	cr_assert_eq(strcmp(ptr1, "this is a test"), 0);
	Option opt4 = None;
	char *ptr2 = opt4.is_some() ? ((String*)Unwrap(opt4))->ptr : "my default str";
	cr_assert_eq(strcmp(ptr2, "my default str"), 0);
}
