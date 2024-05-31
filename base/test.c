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

#include <base/config.h>
#include <base/rand.h>
#include <base/types.h>
#include <criterion/criterion.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

Test(base, rand)
{
    int v;
    i32 v32;
    rand_i32(&v32);
    printf("v=%d\n", v32);
    i64 v64;
    rand_i64(&v64);
    printf("v=%" PRId64 "\n", v64);
    u128 ru128;
    v = rand_u128(&ru128);
    printf("v=%i\n", v);
    u64 high = (ru128 >> 64);
    u64 low = ru128 & 0xFFFFFFFFFFFFFFFF;
    printf("x=%" PRIu64, high);
    printf("%" PRIu64 "\n", low);
    i128 r128;
    v = rand_i128(&r128);
    u64 ihigh = (r128 >> 64);
    uint64_t ilow = r128 & 0xFFFFFFFFFFFFFFFF;
    printf("d=%" PRId64, ihigh);
    printf("%" PRIu64 "\n", ilow);
    // 2^64 chance these are equal
    cr_assert_neq(ihigh, ilow);

    char buf[4];
    rand_bytes(&buf, 4);

    char buf2[4];
    rand_bytes(&buf2, 4);
    bool equal = true;
    for (int i = 0; i < 4; i++) {
        if (buf[i] != buf2[i])
            equal = false;
    }
    // this is about all we can do, 1 in several billion chance these are equal
    cr_assert_eq(equal, false);
    printf("%i %i %i %i\n", buf[0], buf[1], buf[2], buf[3]);

    uint64_t u64_1 = 0, u64_2 = 0;
    rand_u64(&u64_1);
    rand_u64(&u64_2);

    cr_assert_neq(u64_1, u64_2);

    uint32_t u32_1 = 0, u32_2 = 0;
    rand_u32(&u32_1);
    rand_u32(&u32_2);

    cr_assert_neq(u32_1, u32_2);

    i8 vi8, vi8_2, vi8_3, vi8_4;
    i16 vi16, vi16_2, vi16_3, vi16_4;
    u8 vu8, vu8_2, vu8_3, vu8_4;
    u16 vu16, vu16_2, vu16_3, vu16_4;

    rand_i8(&vi8);
    rand_u8(&vu8);
    rand_i16(&vi16);
    rand_u16(&vu16);

    rand_i8(&vi8_2);
    rand_u8(&vu8_2);
    rand_i16(&vi16_2);
    rand_u16(&vu16_2);

    rand_i8(&vi8_3);
    rand_u8(&vu8_3);
    rand_i16(&vi16_3);
    rand_u16(&vu16_3);

    rand_i8(&vi8_4);
    rand_u8(&vu8_4);
    rand_i16(&vi16_4);
    rand_u16(&vu16_4);

    bool i8_eq = vi8 == vi8_2 && vi8_2 == vi8_3 && vi8_4;
    bool u8_eq = vu8 == vu8_2 && vu8_2 == vu8_3 && vu8_4;
    bool i16_eq = vi16 == vi16_2 && vi16_2 == vi16_3 && vi16_4;
    bool u16_eq = vu16 == vu16_2 && vu16_2 == vu16_3 && vu16_4;

    // very unlikely to happen
    cr_assert_eq(i8_eq, false);
    cr_assert_eq(u8_eq, false);
    cr_assert_eq(i16_eq, false);
    cr_assert_eq(u16_eq, false);

    printf("vi8=%i,vu8=%i,vi16=%i,vu16=%i\n", vi8, vu8, vi16, vu16);
}

Test(base, config)
{
    u8 u8_1 = 1, u8_2 = 2;
    configure_u8(&u8_1, NULL, 3);
    cr_assert_eq(u8_1, 3);

    configure_u8(&u8_1, &u8_2, 3);
    cr_assert_eq(u8_1, 2);

    u16 u16_1 = 1, u16_2 = 2;
    configure_u16(&u16_1, NULL, 3);
    cr_assert_eq(u16_1, 3);

    configure_u16(&u16_1, &u16_2, 3);
    cr_assert_eq(u16_1, 2);

    u32 u32_1 = 1, u32_2 = 2;
    configure_u32(&u32_1, NULL, 3);
    cr_assert_eq(u32_1, 3);

    configure_u32(&u32_1, &u32_2, 3);
    cr_assert_eq(u32_1, 2);

    u64 u64_1 = 1, u64_2 = 2;
    configure_u64(&u64_1, NULL, 3);
    cr_assert_eq(u64_1, 3);

    configure_u64(&u64_1, &u64_2, 3);
    cr_assert_eq(u64_1, 2);

    u128 u128_1 = 1, u128_2 = 2;
    configure_u128(&u128_1, NULL, 3);
    cr_assert_eq(u128_1, 3);

    configure_u128(&u128_1, &u128_2, 3);
    cr_assert_eq(u128_1, 2);

    bool bool_1 = true, bool_2 = true;
    configure_bool(&bool_1, NULL, false);
    cr_assert_eq(bool_1, false);

    configure_bool(&bool_1, &bool_2, true);
    cr_assert_eq(bool_1, true);

    char s1[100], s2[100];
    strcpy(s1, "test1");
    strcpy(s2, "test2");
    configure_string(s1, NULL, "333", 100);
    printf("s1=%s\n", s1);
    cr_assert_eq(strcmp(s1, "333"), 0);

    configure_string(s1, s2, "333", 100);
    cr_assert_eq(strcmp(s1, "test2"), 0);
}
