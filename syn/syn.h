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

#include <parser/parser.h>

struct Fn {
	char *name;
	int is_implemented;
	char *ret_type;
};
typedef struct Fn Fn;

struct Class {
	char *name;
	char *impl_name;
	Fn *fn_array;
	int fn_count;
};
typedef struct Class Class;

enum State {
	StateExpectClassName = 1,
	StateAfterClassName = 2,
	StateExpectReturnTypeOrAttribute= 3,
	StateExpectAttributeBracket = 4,
	StateExpectClassBrace = 5,
	StateInAttribute = 6,
	StateExpectImplName = 7,
	StateExpectFnName = 8,
	StateExpectParameterList = 9,
	StateExpectFnBlockOrSemi = 10,
};
typedef enum State State;

struct StateMachine {
	State state;
	TokenStream strm;
	Class *class_array;
	int class_count;
	int scope;
	int in_param_list;
};
typedef struct StateMachine StateMachine;

int build_state(char *file, StateMachine *sm, int debug_flags);

