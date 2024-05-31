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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void configure_u32(uint32_t* value, uint32_t* configured, uint32_t d)
{
    if (configured == NULL) {
        *value = d;
    } else {
        *value = *configured;
    }
}

void configure_u64(uint64_t* value, uint64_t* configured, uint64_t d)
{
    if (configured == NULL) {
        *value = d;
    } else {
        *value = *configured;
    }
}

void configure_u128(__uint128_t* value, __uint128_t* configured, __uint128_t d)
{
    if (configured == NULL) {
        *value = d;
    } else {
        *value = *configured;
    }
}

void configure_bool(bool* value, bool* configured, bool d)
{
    if (configured == NULL) {
        *value = d;
    } else {
        *value = *configured;
    }
}

void configure_u8(uint8_t* value, uint8_t* configured, uint8_t d)
{
    if (configured == NULL) {
        *value = d;
    } else {
        *value = *configured;
    }
}

void configure_u16(uint16_t* value, uint16_t* configured, uint16_t d)
{
    if (configured == NULL) {
        *value = d;
    } else {
        *value = *configured;
    }
}

void configure_string(char* value, char* configured, char* d, int max_len)
{
    if (configured == NULL) {
        int len = max_len - 1;
        int strlen_value = strlen(d);
        if (strlen_value < len) {
            len = strlen_value;
        }
        memcpy(value, d, len);
        value[len] = 0;
    } else {
        int len = max_len - 1;
        int strlen_value = strlen(configured);
        if (strlen_value < len) {
            len = strlen_value;
        }
        memcpy(value, configured, len);
        value[len] = 0;
    }
}
