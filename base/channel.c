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

#include <base/channel.h>
#include <base/cpsrng.h>

Channel channel() {
	Channel ch;
	cpsrng_rand_bytes(&ch.value, 32);
	return ch;
}

bool channel_equal(Channel *ch1, Channel *ch2) {
	int i = 0;
	while (i < 32 && ch1->value[i] == ch2->value[i]) i++;
	return i == 32;
}
