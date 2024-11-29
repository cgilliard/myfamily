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

#define _XOPEN_SOURCE
#include <base/fam.h>
#include <base/print_util.h>
#include <ucontext.h>

// system calls:
void __attribute__((noreturn)) _exit(int code);

Object send(Channel channel, Object object) {
	Object ret = 0;
	return ret;
}
Channel run(Object (*task)(Channel channel)) {
	Channel channel;
	return channel;
}
Object recv(Channel channel, int timeout_millis) {
	Object ret = 0;
	return ret;
}
Object init(Object (*init)(Channel channel), int threads) {
	Object ret = 0;
	return ret;
}

void __attribute__((noreturn)) halt(int code) {
	_exit(code);
}
