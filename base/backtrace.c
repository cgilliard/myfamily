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

#include <base/backtrace.h>
#include <execinfo.h>
#include <stdlib.h>
#include <string.h>

String backtrace_to_string(String *s)
{
	bool response_ok = true;
	void *array[BACKTRACE_MAX_DEPTH];
	char **strings;
	int size, i;

	size = backtrace (array, BACKTRACE_MAX_DEPTH);
	strings = backtrace_symbols (array, size);

	int total_str_len = 0;
	if(strings != NULL) {
		for(int i=0; i<size; i++) {
			int len = strlen(strings[i]);
			if(len < 0) {
				response_ok = false;
				break;
			}
			total_str_len += len;
		}
	}

	if(response_ok) {
		char *responseline = malloc(sizeof(char) * (200 + total_str_len));
		strcpy(responseline, "Backtrace:\n");
		for(int i=0; i<size; i++) {
			strcat(responseline, strings[i]);
			strcat(responseline, "\n");
		}
		s->ptr = responseline;
	}

	if(strings != NULL)
		free(strings);

	return *s;
}
