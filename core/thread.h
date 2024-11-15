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

#ifndef _CORE_THREAD__
#define _CORE_THREAD__

#include <base/types.h>

#define CHANNEL_IMPL_SIZE 16
typedef struct Channel {
	byte impl[CHANNEL_IMPL_SIZE];
} Channel;

typedef struct ChannelConfig {
	unsigned int max_pending;
} ChannelConfig;

int channel_init(Channel *c, const char *name, ChannelConfig *config);

#define TASK_IMPL_SIZE 16
typedef struct Task {
	byte impl[TASK_IMPL_SIZE];
} Task;

int64 task_id(Task *task);
Task task_self();

#define MESSAGE_IMPL_SIZE 16
typedef struct Message {
	byte impl[MESSAGE_IMPL_SIZE];
} Message;

typedef struct SchedulerConfig {
	unsigned int os_threads;
} SchedulerConfig;

#define INIT_SCHEDULER_CONFIG {.os_threads = 4}

// start the secheduler and execute 'main' to kick off.
int init(void (*main)(void), SchedulerConfig *config);
// spawn a new task executing 'fn'
int spawn(Task *task, void (*fn)(void));
int join(Task *task, int64 timeout_millis);

// send a message
int send(Channel ch, Message msg);
// recive a message
int recv(Channel ch, void *(on_message)(Message msg));

// With this API, tasks can use their task_ids as channel names. For instance,
// the spawning task could use the returned task_id to initiate interprocess
// communication. The spawned task could listen for messages on this task id or
// vice-versa.

#endif	// _CORE_THREAD__
