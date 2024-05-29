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

#ifndef _CONFIG_BASE__
#define _CONFIG_BASE__

#include <stdbool.h>
#include <base/types.h>


void configure_bool(bool *value, bool *configured, bool d);
void configure_u8(u8 *value, u8 *configured, u8 d);
void configure_u16(u16 *value, u16 *configured, u16 d);
void configure_u32(u32 *value, u32 *configured, u32 d);
void configure_u64(u64 *value, u64 *configured, u64 d);
void configure_u128(u128 *value, u128 *configured, u128 d);
void configure_string(char *value, char *configured, char *d, int len);


#endif /* _CONFIG_BASE__ */
