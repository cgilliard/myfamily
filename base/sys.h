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

#ifndef _BASE_SYS__
#define _BASE_SYS__

#include <base/types.h>

#define EVENT_READ 0x1
#define EVENT_WRITE (0x1 << 1)

typedef struct EvhEvent {
	uint32 events;
	int64 handle;
} EvhEvent;

int getpagesize();
#define PAGE_SIZE (getpagesize())

void *map(int64 pages);
void unmap(void *addr, int64 pages);
void *fmap(int64 id);
int flush();
int64 fsize();
int64 transmit(int64 handle, const byte *buf, uint64 len);
int64 receive(int64 handle, byte *buf, uint64 len);
int64 accept_conn(int64 handle);
void __attribute__((noreturn)) halt(int code);

int64 server(int port, const char *addr, bool udp, int backlog);
int64 client(int port, const char *addr, bool udp);
int64 establish(int64 handle);
int evh();
int evh_register(int evh, int64 conn, int op);
int evh_unregister(int evh, int64 conn);
int evh_wait(int evh, int64 timeout_millis, int64 max_events, EvhEvent *events);

void init_sys(const char *path);
void shutdown_sys();

#ifdef TEST
extern int64 _alloc_sum;
#endif	// TEST

#endif	// _BASE_SYS__
