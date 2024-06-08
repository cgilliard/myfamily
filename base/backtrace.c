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

#include <unistd.h>
#include <sys/types.h>

int backtrace_add_entry(Backtrace *ptr, BacktraceEntry *entry) {
	int ret = 0;
	if(ptr->count == 0) {
		ptr->rows = malloc(sizeof(BacktraceEntry));
	} else {
		BacktraceEntry *tmp = realloc(
			ptr->rows,
			sizeof(BacktraceEntry) * (ptr->count + 1)
		);
		if(tmp == NULL) {
			ret = -1;
		} else {
			ptr->rows = tmp;
		}
	}

	if(!ret) {
		ptr->rows[ptr->count].function_name = malloc(
			sizeof(char) * (strlen(entry->function_name) + 1)
		);
		strcpy(ptr->rows[ptr->count].function_name, entry->function_name);

		ptr->rows[ptr->count].bin_name = malloc(
                	sizeof(char) * (strlen(entry->bin_name) + 1)
        	);
        	strcpy(ptr->rows[ptr->count].bin_name, entry->bin_name);

		ptr->rows[ptr->count].address = malloc(
                	sizeof(char) * (strlen(entry->address) + 1)
        	);
        	strcpy(ptr->rows[ptr->count].address, entry->address);

		ptr->rows[ptr->count].file_path = malloc(
                	sizeof(char) * (strlen(entry->file_path) + 1)
        	);
        	strcpy(ptr->rows[ptr->count].file_path, entry->file_path);

		ptr->count++;
	}
	return ret;
}

void backtrace_free(BacktraceImpl *ptr) {
	for(int i=0; i<ptr->count; i++) {
		free(ptr->rows[i].function_name);
		free(ptr->rows[i].bin_name);
		free(ptr->rows[i].address);
		free(ptr->rows[i].file_path);
	}

	ptr->count = 0;
	free(ptr->rows);
}

void backtrace_print(Backtrace *ptr) {
	printf("Backtrace:\n");
	for(int i=0; i<ptr->count; i++) {
		printf(
			"#%i:\n\
	[function_name=%s]\n\
	[binary_path=%s]\n\
	[address=%s]\n\
       	[file_lineno=%s]\n",
			i,
			ptr->rows[i].function_name,
			ptr->rows[i].bin_name,
			ptr->rows[i].address,
			ptr->rows[i].file_path
		);
	}
}

#define BUFSIZE 10000

int get_file_line(char *bin, char *addr, char *line_num, char *fn_name, int max_len) {
    pid_t process_id;
    char cmd[strlen(bin) + strlen(addr) + 100];
    #ifdef __APPLE__
    	sprintf(cmd, "atos --fullPath -o %s %s", bin, addr);
    #else // LINUX/WIN for now
	sprintf(cmd, "addr2line -f -e %s %s", bin, addr);
    #endif // OS Specific code

    char buffer[BUFSIZE] = {0};
    FILE *fp;

    if ((fp = popen(cmd, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return -1;
    }

    int counter = 0;
    while (fgets(buffer, BUFSIZE, fp) != NULL) {
	#ifdef __APPLE__
	bool found_first_paren = false;
	bool found_second_paren = false;
	u64 len = strlen(buffer);
	int line_num_itt = 0;
	for(int i=0; i<len; i++) {
		if(!found_first_paren && buffer[i] == '(') {
			found_first_paren = true;
		} else if(!found_second_paren && buffer[i] == '(') {
			found_second_paren = true;
		} else if(found_second_paren && found_first_paren) {
			if(buffer[i] == ')') {
				break;
			}
			line_num[line_num_itt] = buffer[i];
			line_num_itt++;
			if(line_num_itt == max_len) break;
		}
	}
	line_num[line_num_itt] = 0;
	#else // Linux
	if(counter == 0) {
		int len = strlen(buffer);
		for(int i=0; i<len; i++) {
			if(buffer[i] == '\n')
				buffer[i] = 0;
		}
		buffer[max_len-1] = 0;
		strcpy(fn_name, buffer);
	} else if(counter == 1) {
		int len = strlen(buffer);
		for(int i=0; i<len; i++) {
			if(buffer[i] == '\n')
				buffer[i] = 0;
		}
		buffer[max_len-1] = 0;
		strcpy(line_num, buffer);
	}
	counter++;
	#endif // OS specific code
    }

    if (pclose(fp)) {
        printf("Command not found or exited with error status\n");
        return -1;
    }

    return 0;
}

int backtrace_generate(Backtrace *ptr, u64 max_depth) {
	int ret = 0;
	void *array[max_depth];
	int size = backtrace(array, max_depth);
	char **strings = backtrace_symbols(array, size);

	for(int i=0; i<size; i++) {
		#ifdef __APPLE__
			BacktraceEntry ent;
			char address[30];
			Dl_info info;
			dladdr(array[i], &info);
			u64 addr = 0x0000000100000000 + info.dli_saddr - info.dli_fbase;
			sprintf(address, "0x%" PRIx64 "", addr);
			ent.address = address;
			char function_name[strlen(info.dli_sname)+1];
			char bin_name[strlen(info.dli_fname)+1];
			strcpy(function_name, info.dli_sname);
			ent.function_name = function_name;
			strcpy(bin_name, info.dli_fname);
			ent.bin_name = bin_name;

			char file_path[513];
			char fn_name[513];
			strcpy(file_path, "");
			get_file_line(bin_name, address, file_path, fn_name, 512);
			ent.file_path = file_path;
			backtrace_add_entry(ptr, &ent);
		#else // LINUX/WIN for now
		      char buffer[2050];
		      char address[101];
		      int len = strlen(strings[i]);
		      bool in_bin_name = true;
		      bool in_address = false;
		      int buffer_itt = 0;
		      int address_itt = 0;
		      for(int j=0; j<len; j++) {
			      if(in_bin_name && strings[i][j] == '(') {
			      	 in_bin_name = false;
				 in_address = true;
			      } else if(in_address && strings[i][j] == ')') {
					in_address = false;
					if(address_itt < 100) {
						address[address_itt] = 0;
					}
			      } else if(in_bin_name) {
					if(buffer_itt < 2048) {
						buffer[buffer_itt] = strings[i][j];
						buffer_itt++;
					} else if(buffer_itt == 2048) {
						buffer[buffer_itt] = 0;
						buffer_itt = 0;
					}
			      } else if(in_address) {
					if(address_itt < 100) {
						address[address_itt] = strings[i][j];
						address_itt++;
					} else {
						address[100] = 0;
					}
			      }
		      }

		      if(address_itt < 100) {
                      	address[address_itt] = 0;
                      }
		      if(buffer_itt < 2048) {
			buffer[buffer_itt] = 0;
		      }

		      char file_path[513];
		      char fn_name[513];
                      strcpy(file_path, "");
                      get_file_line(buffer, address, file_path, fn_name, 512);

		      BacktraceEntry ent;
		      ent.address = address;
		      ent.function_name = fn_name;
		      ent.file_path = file_path;
		      ent.bin_name = buffer;
		      backtrace_add_entry(ptr, &ent);
		#endif // OS Specific code
	}

	if(strings != NULL)
                free(strings);

	return ret;
}

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
