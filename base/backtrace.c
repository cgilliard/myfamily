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
#include <base/tlmalloc.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

int backtrace_add_entry(Backtrace *ptr, BacktraceEntry *entry) {
	int ret = 0;
	if (!ptr->count) {
		ptr->rows = tlmalloc(sizeof(BacktraceEntry));
	} else {
		BacktraceEntry *tmp = tlrealloc(
		    ptr->rows, sizeof(BacktraceEntry) * (ptr->count + 1));
		if (tmp == NULL) {
			ret = -1;
		} else {
			ptr->rows = tmp;
		}
	}

	if (!ret) {
		ptr->rows[ptr->count].function_name =
		    tlmalloc(sizeof(char) * (strlen(entry->function_name) + 1));
		strcpy(ptr->rows[ptr->count].function_name,
		       entry->function_name);

		ptr->rows[ptr->count].bin_name =
		    tlmalloc(sizeof(char) * (strlen(entry->bin_name) + 1));
		strcpy(ptr->rows[ptr->count].bin_name, entry->bin_name);

		ptr->rows[ptr->count].address =
		    tlmalloc(sizeof(char) * (strlen(entry->address) + 1));
		strcpy(ptr->rows[ptr->count].address, entry->address);

		ptr->rows[ptr->count].file_path =
		    tlmalloc(sizeof(char) * (strlen(entry->file_path) + 1));
		strcpy(ptr->rows[ptr->count].file_path, entry->file_path);

		ptr->count++;
	}
	return ret;
}

void backtrace_copy(Backtrace *dst, Backtrace *src) {
	dst->count = 0;
	dst->rows = NULL;
	for (int i = 0; i < src->count; i++) {
		backtrace_add_entry(dst, &src->rows[i]);
	}
}

void backtrace_free(BacktracePtr *ptr) {
	for (int i = 0; i < ptr->count; i++) {
		tlfree(ptr->rows[i].function_name);
		tlfree(ptr->rows[i].bin_name);
		tlfree(ptr->rows[i].address);
		tlfree(ptr->rows[i].file_path);
	}

	ptr->count = 0;
	tlfree(ptr->rows);
}

#define ANSI_COLOR_DIMMED "\x1b[2m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_BRIGHT_RED "\x1b[91m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

void backtrace_print(Backtrace *ptr, int flags) {
	printf("Backtrace:\n");
	for (int i = 0; i < ptr->count; i++) {
		if ((flags & ERROR_PRINT_FLAG_NO_COLOR) == 0)
			printf("#%i:\n\
        [" ANSI_COLOR_DIMMED "fn=" ANSI_COLOR_RESET "'" ANSI_COLOR_GREEN
			       "%s" ANSI_COLOR_RESET "']\n\
        [" ANSI_COLOR_DIMMED "binary=" ANSI_COLOR_RESET "'" ANSI_COLOR_MAGENTA
			       "%s" ANSI_COLOR_RESET "'] [" ANSI_COLOR_DIMMED
			       "address=" ANSI_COLOR_RESET "%s]\n\
        [" ANSI_COLOR_DIMMED "code=" ANSI_COLOR_RESET "'" ANSI_COLOR_CYAN
			       "%s" ANSI_COLOR_RESET "']\n",
			       i, ptr->rows[i].function_name,
			       ptr->rows[i].bin_name, ptr->rows[i].address,
			       ptr->rows[i].file_path);
		else
			printf("#%i:\n\
	[fn='%s']\n\
	[binary='%s'] [address=%s]\n\
       	[code='%s']\n",
			       i, ptr->rows[i].function_name,
			       ptr->rows[i].bin_name, ptr->rows[i].address,
			       ptr->rows[i].file_path);
	}
}

#define BUFSIZE 10000

int get_file_line(char *bin, char *addr, char *line_num, char *fn_name,
		  int max_len) {
	pid_t process_id;
	char cmd[strlen(bin) + strlen(addr) + 100];
#ifdef __APPLE__
	sprintf(cmd, "atos --fullPath -o %s %s", bin, addr);
#else  // LINUX/WIN for now
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
		for (int i = 0; i < len; i++) {
			if (!found_first_paren && buffer[i] == '(') {
				found_first_paren = true;
			} else if (!found_second_paren && buffer[i] == '(') {
				found_second_paren = true;
			} else if (found_second_paren && found_first_paren) {
				if (buffer[i] == ')') {
					break;
				}
				line_num[line_num_itt] = buffer[i];
				line_num_itt++;
				if (line_num_itt == max_len)
					break;
			}
		}
		line_num[line_num_itt] = 0;
#else  // Linux
		if (counter == 0) {
			int len = strlen(buffer);
			for (int i = 0; i < len; i++) {
				if (buffer[i] == '\n')
					buffer[i] = 0;
			}
			buffer[max_len - 1] = 0;
			strcpy(fn_name, buffer);
		} else if (counter == 1) {
			int len = strlen(buffer);
			for (int i = 0; i < len; i++) {
				if (buffer[i] == '\n')
					buffer[i] = 0;
			}
			buffer[max_len - 1] = 0;
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

Backtrace backtrace_generate(u64 max_depth) {
	BacktracePtr ptr = EMPTY_BACKTRACE;
	void *array[max_depth];
	int size = backtrace(array, max_depth);
	char **strings = backtrace_symbols(array, size);

	for (int i = 0; i < size; i++) {
#ifdef __APPLE__
		BacktraceEntry ent;
		char address[30];
		Dl_info info;
		dladdr(array[i], &info);
		u64 addr = 0x0000000100000000 + info.dli_saddr - info.dli_fbase;
		sprintf(address, "0x%" PRIx64 "", addr);
		ent.address = address;
		char function_name[strlen(info.dli_sname) + 1];
		char bin_name[strlen(info.dli_fname) + 1];
		strcpy(function_name, info.dli_sname);
		ent.function_name = function_name;
		strcpy(bin_name, info.dli_fname);
		ent.bin_name = bin_name;

		char file_path[513];
		char fn_name[513];
		strcpy(file_path, "");
		get_file_line(bin_name, address, file_path, fn_name, 512);
		if (!strcmp(file_path, "")) {
			strcpy(file_path, "Unknown");
		}
		ent.file_path = file_path;
		backtrace_add_entry(&ptr, &ent);
#else  // LINUX/WIN for now
		char buffer[2050];
		char address[101];
		int len = strlen(strings[i]);
		bool in_bin_name = true;
		bool in_address = false;
		int buffer_itt = 0;
		int address_itt = 0;
		for (int j = 0; j < len; j++) {
			if (in_bin_name && strings[i][j] == '(') {
				in_bin_name = false;
				in_address = true;
			} else if (in_address && strings[i][j] == ')') {
				in_address = false;
				if (address_itt < 100) {
					address[address_itt] = 0;
				}
			} else if (in_bin_name) {
				if (buffer_itt < 2048) {
					buffer[buffer_itt] = strings[i][j];
					buffer_itt++;
				} else if (buffer_itt == 2048) {
					buffer[buffer_itt] = 0;
					buffer_itt = 0;
				}
			} else if (in_address) {
				if (address_itt < 100) {
					address[address_itt] = strings[i][j];
					address_itt++;
				} else {
					address[100] = 0;
				}
			}
		}

		if (address_itt < 100) {
			address[address_itt] = 0;
		}
		if (buffer_itt < 2048) {
			buffer[buffer_itt] = 0;
		}

		char file_path[513];
		char fn_name[513];
		strcpy(file_path, "");
		get_file_line(buffer, address, file_path, fn_name, 512);

		if (!strcmp(file_path, "")) {
			strcpy(file_path, "Unknown");
		}
		if (!strcmp(file_path, "") || file_path[0] == 63 ||
		    file_path[0] == -107) {
			strcpy(file_path, "Unknown");
		}
		if (fn_name[0] == 63 || fn_name[0] == -107)
			strcpy(fn_name, "Unknown");

		BacktraceEntry ent;
		ent.address = address;
		ent.function_name = fn_name;
		ent.file_path = file_path;
		ent.bin_name = buffer;
		backtrace_add_entry(&ptr, &ent);
#endif // OS Specific code
	}

	// use regular free because this is not allocated by tlmalloc
	if (strings != NULL)
		free(strings);

	return ptr;
}
