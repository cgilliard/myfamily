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

#include <core/backtrace.h>
#include <core/colors.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <stdlib.h>

GETTER(Backtrace, count);
SETTER(Backtrace, count);

#define BUFSIZE 10000

int get_file_line(char *bin, char *addr, char *line_num, char *fn_name,
		  int max_len) {
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

	char buffer[BUFSIZE] = {0};
	FILE *fp;

	if ((fp = popen(cmd, "r")) == NULL) {
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
		return -1;
	}

	return 0;
}

void BacktraceEntry_cleanup(BacktraceEntry *ptr) {}

bool BacktraceEntry_myclone(BacktraceEntry *dst, BacktraceEntry *src) {
	BacktraceEntry_set_backtrace_entry_values(
	    dst, src->_name, src->_bin_name, src->_address, src->_file_path);
	return true;
}

void BacktraceEntry_set_backtrace_entry_values(BacktraceEntry *ptr,
					       const char *name,
					       const char *bin_name,
					       const char *address,
					       const char *file_path) {
	BUILDPTR(ptr, BacktraceEntry);
	int file_path_len = strlen(file_path);
	if (file_path_len > BACKTRACE_CHAR_ARRAY_LEN)
		file_path =
		    file_path + (file_path_len - BACKTRACE_CHAR_ARRAY_LEN) + 1;
	int strlen_bin_name = strlen(bin_name);
	if (strlen_bin_name > BACKTRACE_CHAR_ARRAY_LEN)
		bin_name =
		    bin_name + (strlen_bin_name - BACKTRACE_CHAR_ARRAY_LEN) + 1;
	snprintf(ptr->_name, BACKTRACE_CHAR_ARRAY_LEN, "%s", name);
	snprintf(ptr->_bin_name, BACKTRACE_CHAR_ARRAY_LEN, "%s", bin_name);
	snprintf(ptr->_address, BACKTRACE_CHAR_ARRAY_LEN, "%s", address);
	snprintf(ptr->_file_path, BACKTRACE_CHAR_ARRAY_LEN, "%s", file_path);
}

#define GET_ROWS(bt) Backtrace_get_rows(bt)
#define SET_ROWS(bt, value) Backtrace_set_rows(bt, value)
#define GET_COUNT(bt) *Backtrace_get_count(bt)
#define SET_COUNT(bt, value) Backtrace_set_count(bt, value)

bool Backtrace_add_entry(Backtrace *ptr, const char *name, const char *bin_name,
			 const char *address, const char *file_path) {
	u64 count = GET_COUNT(ptr);
	if (count >= MAX_BACKTRACE_ROWS)
		return false;
	BacktraceEntry_set_backtrace_entry_values(&ptr->_rows[count], name,
						  bin_name, address, file_path);
	SET_COUNT(ptr, count + 1);
	return true;
}

Backtrace Backtrace_generate_bt(u64 max_depth) {
	BacktracePtr ret = EMPTY_BACKTRACE;
	Backtrace_generate(&ret, max_depth);

	return ret;
}

bool Backtrace_generate(Backtrace *ptr, u64 max_depth) {
	void *array[max_depth];
	int size = backtrace(array, max_depth);
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
		if (!Backtrace_add_entry(ptr, function_name, bin_name, address,
					 file_path))
			break;
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
		if (!Backtrace_add_entry(ptr, fn_name, buffer, address,
					 file_path))
			break;
#endif // OS Specific code
	}

	if (strings)
		free(strings);
	return true;
}

void Backtrace_cleanup(Backtrace *ptr) {}

bool Backtrace_myclone(Backtrace *dst, Backtrace *src) {
	bool ret = true;
	u64 src_count = GET_COUNT(src);
	SET_COUNT(dst, src_count);
	for (u64 i = 0; i < src_count; i++) {
		myclone(&dst->_rows[i], &src->_rows[i]);
	}

	return ret;
}

void Backtrace_print(Backtrace *ptr) {
	BacktraceEntryPtr *rows = ptr->_rows;
	u64 count = GET_COUNT(ptr);
	if (count == 0) {
		printf("-------------Backtrace not available-------------\n");
	} else {
		printf("Backtrace:\n");
		for (int i = 0; i < count; i++) {
			char *function_name = rows[i]._name;
			char *bin_name = rows[i]._bin_name;
			char *address = rows[i]._address;
			char *file_path = rows[i]._file_path;
			printf("#%i:\n\
	[%sfn=%s%s%s%s']\n\
	[%sbinary=%s'%s%s%s'] [%saddress=%s%s]\n\
	[%scode=%s'%s%s%s']\n",
			       i, DIMMED, RESET, GREEN, function_name, RESET,
			       DIMMED, RESET, MAGENTA, bin_name, RESET, DIMMED,
			       RESET, address, DIMMED, RESET, CYAN, file_path,
			       RESET);
		}
	}
}

bool Backtrace_fn_name(Backtrace *ptr, char *buffer, u64 len, u64 index) {
	u64 count = GET_COUNT(ptr);
	if (index >= count)
		return false;

	BacktraceEntryPtr *rows = ptr->_rows;

	strncpy(buffer, rows[index]._name, len);

	return true;
}

bool Backtrace_bin_name(Backtrace *ptr, char *buffer, u64 len, u64 index) {
	u64 count = GET_COUNT(ptr);
	if (index >= count)
		return false;

	BacktraceEntryPtr *rows = ptr->_rows;

	strncpy(buffer, rows[index]._bin_name, len);
	return true;
}

bool Backtrace_address(Backtrace *ptr, char *buffer, u64 len, u64 index) {
	u64 count = GET_COUNT(ptr);
	if (index >= count)
		return false;

	BacktraceEntryPtr *rows = ptr->_rows;

	strncpy(buffer, rows[index]._address, len);
	return true;
}

bool Backtrace_file_path(Backtrace *ptr, char *buffer, u64 len, u64 index) {
	u64 count = GET_COUNT(ptr);
	if (index >= count)
		return false;

	BacktraceEntryPtr *rows = ptr->_rows;

	strncpy(buffer, rows[index]._file_path, len);
	return true;
}
