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
#include <pthread.h>
#include <ucontext.h>

// system calls:
void __attribute__((noreturn)) _exit(int code);
int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
				   void *(*start_routine)(void *), void *arg);

void *fam_start_thread() {
	println("start fam thread");
	return 0;
}

Object send(Channel channel, Object object) {
	Object ret = $(0);
	return ret;
}
Channel run(Object (*task)(Channel channel)) {
	Channel channel;
	return channel;
}
Object recv(Channel channel, int timeout_millis) {
	Object ret = $(0);
	return ret;
}

Object init(Object (*task)(Channel channel), int threads) {
	for (int i = 0; i < threads; i++) {
		pthread_t th;
		pthread_create(&th, 0, fam_start_thread, 0);
	}
	Object ret = $(0);
	return ret;
}

void __attribute__((noreturn)) halt(int code) {
	_exit(code);
}
