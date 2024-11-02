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

#ifndef _BASE_CONTEXT__
#define _BASE_CONTEXT__

#include <base/types.h>

// Topic data structure
typedef struct Topic {
	void *tbd;
} Topic;

// Message data structure
typedef struct Message {
	void *tbd;
} Message;

// Initialize the scheduler with main as the initial callback program
int init(void (*main)());

// Spawn a new program returning it's input topic.
// To send arguments:
// Topic new_prog = spawn(new_prog_fn);
// Topic args = topic_push(new_prog, "args");
// // build command_line_args
// send(args, command_line_args);
// Topic stdin = topic_push(new_prog, "stdin");
// build input_data
// send(stdin, input_data);
Topic spawn(void (*fn)());
// Create a general purpose topic to be used for IPC.
Topic create_topic();
// send the specified msg on the specified topic
int send(Topic topic, Message msg);

// subscribe to the specified topic
int subscribe(Topic topic);
// set a handler callback to handle messages
void on_message(void *(on_message)(Message msg));

#endif // _BASE_CONTEXT__
