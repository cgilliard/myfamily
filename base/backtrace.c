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
#include <base/colors.h>
#include <base/fam_err.h>
#include <base/resources.h>
#include <base/types.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_file_line(char *bin, char *addr, char *line_num, char *fn_name, int max_len) {
	pid_t process_id;
	int cmd_max_len = strlen(bin) + strlen(addr) + 100;
	char cmd[cmd_max_len];

	// hack to get line numbers on linux in criterion
	if (!strcmp(bin, "boxfort-worker")) {
		strcpy(bin, "./bin/test");
	}

#ifdef __APPLE__
	snprintf(cmd, cmd_max_len, "atos --fullPath -o %s %s", bin, addr);
#else  // LINUX/WIN for now
	snprintf(cmd, cmd_max_len, "addr2line -f -e %s %s", bin, addr);
#endif // OS Specific code

	char buffer[MAX_ENTRY_SIZE] = {0};
	FILE *fp;

	if ((fp = popen(cmd, "r")) == NULL) {
		return -1;
	}

	int counter = 0;
	while (fgets(buffer, MAX_ENTRY_SIZE, fp) != NULL) {
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
		return -1;
	}

	return 0;
}

void backtrace_set_entry_values(BacktraceEntry *ptr, const char *name, const char *bin_name,
								const char *address, const char *file_path) {
	u64 offset = 0;
	ptr->start_bin = MAX_ENTRY_SIZE;
	ptr->start_addr = MAX_ENTRY_SIZE;
	ptr->start_file_path = MAX_ENTRY_SIZE;
	for (int i = 0; i <= MAX_ENTRY_SIZE; i++)
		ptr->data[i] = 0;

	strncpy((char *)(ptr->data + offset), name, MAX_ENTRY_SIZE - offset);
	offset += strlen(name) + 1;

	if (offset >= MAX_ENTRY_SIZE)
		return;

	ptr->start_bin = offset;
	strncpy((char *)(ptr->data + offset), bin_name, MAX_ENTRY_SIZE - offset);
	offset += strlen(bin_name) + 1;

	if (offset >= MAX_ENTRY_SIZE)
		return;

	ptr->start_addr = offset;
	strncpy((char *)(ptr->data + offset), address, MAX_ENTRY_SIZE - offset);
	offset += strlen(address) + 1;

	if (offset >= MAX_ENTRY_SIZE)
		return;

	ptr->start_file_path = offset;
	strncpy((char *)(ptr->data + offset), file_path, MAX_ENTRY_SIZE - offset);
	offset += strlen(address) + 1;
}

bool backtrace_add_entry(Backtrace *ptr, const char *name, const char *bin_name,
						 const char *address, const char *file_path) {
	u64 count = ptr->cur_entries;
	if (count >= MAX_ENTRIES)
		return false;
	backtrace_set_entry_values(&ptr->entries[count], name, bin_name, address, file_path);
	ptr->cur_entries++;
	return true;
}

int backtrace_generate(Backtrace *ptr) {
	if (ptr == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}

	ptr->cur_entries = 0;
	void *array[MAX_ENTRIES];
	int size = backtrace(array, MAX_ENTRIES);
	if (size < 0) {
		SetErr(BackTraceErr);
		return -1;
	}
	char **strings = backtrace_symbols(array, size);

	if (strings == NULL)
		size = 0;

	for (int i = 0; i < size; i++) {
#ifdef __APPLE__
		char address[30];
		Dl_info info;
		dladdr(array[i], &info);
		u64 addr = 0x0000000100000000 + info.dli_saddr - info.dli_fbase;
		snprintf(address, 30, "0x%" PRIx64 "", addr);
		char function_name[strlen(info.dli_sname) + 1];
		char bin_name[strlen(info.dli_fname) + 1];
		strcpy(function_name, info.dli_sname);
		strcpy(bin_name, info.dli_fname);

		char file_path[513];
		char fn_name[513];
		strcpy(file_path, "");
		get_file_line(bin_name, address, file_path, fn_name, 512);
		if (!strcmp(file_path, "")) {
			strcpy(file_path, "Unknown");
		}
		if (!backtrace_add_entry(ptr, function_name, bin_name, address, file_path))
			break;
#else  // LINUX
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
		if (!strcmp(file_path, "") || file_path[0] == 63 || file_path[0] == -107) {
			strcpy(file_path, "Unknown");
		}
		if (fn_name[0] == 63 || fn_name[0] == -107)
			strcpy(fn_name, "Unknown");
		if (!backtrace_add_entry(ptr, fn_name, buffer, address, file_path))
			break;
#endif // OS Specific code
	}

	if (strings)
		free(strings);

	return 0;
}

void backtrace_print(const Backtrace *ptr) {
	const BacktraceEntry *rows = ptr->entries;
	u64 count = ptr->cur_entries;
	if (count == 0) {
		printf("-------------Backtrace not available-------------\n");
	} else {
		printf("Backtrace:\n");
		for (int i = 0; i < count; i++) {
			const u8 *function_name = rows[i].data;
			const u8 *bin_name;
			if (rows[i].start_bin < MAX_ENTRY_SIZE)
				bin_name = rows[i].data + rows[i].start_bin;
			else
				bin_name = (u8 *)"";
			const u8 *address;
			if (rows[i].start_bin < MAX_ENTRY_SIZE)
				address = rows[i].data + rows[i].start_addr;
			else
				address = (u8 *)"";

			const u8 *file_path;
			if (rows[i].start_file_path < MAX_ENTRY_SIZE)
				file_path = rows[i].data + rows[i].start_file_path;
			else
				file_path = (u8 *)"";
			printf("#%i:\n\
        [%sfn=%s%s%s%s']\n\
        [%sbinary=%s'%s%s%s'] [%saddress=%s%s]\n\
        [%scode=%s'%s%s%s']\n",
				   i, DIMMED, RESET, GREEN, function_name, RESET, DIMMED, RESET, MAGENTA, bin_name,
				   RESET, DIMMED, RESET, address, DIMMED, RESET, CYAN, file_path, RESET);
		}
	}
}
