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

#ifndef _BASE_OBJECT_IMPL__
#define _BASE_OBJECT_IMPL__

typedef struct ObjectImpl {
	union {
		unsigned char bytes[8];
		long long int_value;
		unsigned long long uint_value;
		double float_value;
		int code_value;
		void *ptr_value;
	} value;
	unsigned int type : 4;
	unsigned int no_cleanup : 1;
	unsigned int consumed : 1;
	unsigned int aux;
} ObjectImpl;

#endif	// _BASE_OBJECT_IMPL__
