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

#include <base/fam.h>

Object init(Object (*task)(Channel *channel), int threads) {
	return $(0);
}
Channel run(Object (*task)(Channel *channel)) {
	Channel ch;
	return ch;
}
Object send(Channel channel, Object *object) {
	return $(0);
}
Object recv(Channel channel, int timeout_millis) {
	return $(0);
}
void halt(int code) {
}
