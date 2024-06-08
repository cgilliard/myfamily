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
#include <dlfcn.h>
#include <execinfo.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
			#ifdef __APPLE__
				Dl_info info;
				char buf[100];
				dladdr(array[i], &info);
				sprintf(buf, "#%i", i);
				strcat(responseline, buf);
				strcat(responseline, ": [");
				strcat(responseline, info.dli_sname);
				strcat(responseline, "] [");
                        	strcat(responseline, info.dli_fname);
				strcat(responseline, "] [");
                                sprintf(buf, "%p", 0x0000000100000000 + info.dli_saddr - info.dli_fbase);
                                strcat(responseline, buf);
				strcat(responseline, "] [");
				strcat(responseline, strings[i]);
				strcat(responseline, "]\n");
			#else // LINUX/WIN for now
				strcat(responseline, strings[i]);
				strcat(responseline, "\n");
			#endif // OS Specific code
		}
		s->ptr = responseline;
	}

	if(strings != NULL)
		free(strings);

	return *s;
}
