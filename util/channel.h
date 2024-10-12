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

#ifndef _UTIL_CHANNEL__
#define _UTIL_CHANNEL__

#include <util/rc.h>

typedef struct ChannelNc {
	Rc impl;
} ChannelNc;

void channel_cleanup(ChannelNc *ptr);

#define Channel ChannelNc __attribute__((warn_unused_result, cleanup(channel_cleanup)))

#endif // _UTIL_CHANNEL__
