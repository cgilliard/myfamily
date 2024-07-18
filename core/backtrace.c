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
#include <core/resources.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <stdlib.h>

GETTER(BacktraceEntry, name);
SETTER(BacktraceEntry, name);
GETTER(BacktraceEntry, bin_name);
SETTER(BacktraceEntry, bin_name);
GETTER(BacktraceEntry, address);
SETTER(BacktraceEntry, address);
GETTER(BacktraceEntry, file_path);
SETTER(BacktraceEntry, file_path);

GETTER(Backtrace, rows);
SETTER(Backtrace, rows);
GETTER(Backtrace, count);
SETTER(Backtrace, count);

#define GET_NAME(bte) BacktraceEntry_get_name(bte)
#define SET_NAME(bte, value) BacktraceEntry_set_name(bte, value)

#define GET_BIN_NAME(bte) BacktraceEntry_get_bin_name(bte)
#define SET_BIN_NAME(bte, value) BacktraceEntry_set_bin_name(bte, value)

#define GET_ADDRESS(bte) BacktraceEntry_get_address(bte)
#define SET_ADDRESS(bte, value) BacktraceEntry_set_address(bte, value)

#define GET_FILE_PATH(bte) BacktraceEntry_get_file_path(bte)
#define SET_FILE_PATH(bte, value) BacktraceEntry_set_file_path(bte, value)

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

void BacktraceEntry_cleanup(BacktraceEntry *ptr) {
	char *name = *GET_NAME(ptr);
	if (name) {
		myfree(name);
		SET_NAME(ptr, NULL);
	}

	char *bin_name = *GET_BIN_NAME(ptr);
	if (bin_name) {
		myfree(bin_name);
		SET_BIN_NAME(ptr, NULL);
	}

	char *file_path = *GET_FILE_PATH(ptr);
	if (file_path) {
		myfree(file_path);
		SET_FILE_PATH(ptr, NULL);
	}

	char *address = *GET_ADDRESS(ptr);
	if (address) {
		myfree(address);
		SET_ADDRESS(ptr, NULL);
	}
}
bool BacktraceEntry_clone(BacktraceEntry *dst, BacktraceEntry *src) {
	return BacktraceEntry_set_backtrace_entry_values(
	    dst, *GET_NAME(src), *GET_BIN_NAME(src), *GET_ADDRESS(src),
	    *GET_FILE_PATH(src));
}

bool BacktraceEntry_set_backtrace_entry_values(BacktraceEntry *ptr,
					       const char *name,
					       const char *bin_name,
					       const char *address,
					       const char *file_path) {

	bool ret = true;

	char *vname = mymalloc(sizeof(char) * (1 + strlen(name)));
	if (vname) {
		strcpy(vname, name);
		SET_NAME(ptr, vname);
	} else {
		ret = false;
	}

	if (ret) {
		char *vbin_name =
		    mymalloc(sizeof(char) * (1 + strlen(bin_name)));
		if (vbin_name) {
			strcpy(vbin_name, bin_name);
			SET_BIN_NAME(ptr, vbin_name);
		} else {
			ret = false;
		}
	}

	if (ret) {
		char *vaddress = mymalloc(sizeof(char) * (1 + strlen(address)));
		if (vaddress) {
			strcpy(vaddress, address);
			SET_ADDRESS(ptr, vaddress);
		} else {
			ret = false;
		}
	}

	if (ret) {
		char *vfile_path =
		    mymalloc(sizeof(char) * (1 + strlen(file_path)));
		if (vfile_path) {
			strcpy(vfile_path, file_path);
			SET_FILE_PATH(ptr, vfile_path);
		} else {
			ret = false;
		}
	}

	if (!ret) {
		BacktraceEntry_cleanup(ptr);
	}

	return ret;
}

#define GET_ROWS(bt) Backtrace_get_rows(bt)
#define SET_ROWS(bt, value) Backtrace_set_rows(bt, value)
#define GET_COUNT(bt) *Backtrace_get_count(bt)
#define SET_COUNT(bt, value) Backtrace_set_count(bt, value)

bool Backtrace_add_entry(Backtrace *ptr, const char *name, const char *bin_name,
			 const char *address, const char *file_path) {
	bool ret = true;
	u64 count = GET_COUNT(ptr);
	BacktraceEntryPtr *rows;
	rows = *GET_ROWS(ptr);
	if (count == 0) {
		rows = mymalloc(sizeof(BacktraceEntry));
		SET_ROWS(ptr, rows);
	} else {
		rows = myrealloc(rows, sizeof(BacktraceEntry) * (count + 1));
		SET_ROWS(ptr, rows);
	}
	if (rows == NULL)
		ret = false;

	if (ret) {
		BacktraceEntry ent =
		    BUILD(BacktraceEntry, NULL, NULL, NULL, NULL);
		BacktraceEntry_set_backtrace_entry_values(&ent, name, bin_name,
							  address, file_path);
		BacktraceEntryPtr *entry_arr;
		copy(&rows[count], &ent);

		SET_COUNT(ptr, count + 1);
	}

	return ret;
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

	// not generated by mymalloc so we use free
	if (strings)
		free(strings);
	return true;
}

void Backtrace_cleanup(Backtrace *ptr) {
	u64 count = GET_COUNT(ptr);
	BacktraceEntryPtr *rows = *GET_ROWS(ptr);
	for (u64 i = 0; i < count; i++) {
		BacktraceEntry_cleanup(&rows[i]);
	}
	SET_COUNT(ptr, 0);
	if (rows) {
		myfree(rows);
		SET_ROWS(ptr, NULL);
	}
}
bool Backtrace_clone(Backtrace *dst, Backtrace *src) {
	bool ret = true;
	u64 src_count = GET_COUNT(src);
	SET_COUNT(dst, src_count);

	if (src_count) {
		BacktraceEntryPtr *rows =
		    mymalloc(sizeof(BacktraceEntry) * src_count);
		BacktraceEntryPtr *src_rows = *GET_ROWS(src);
		for (u64 i = 0; i < src_count; i++) {
			if (!copy(&rows[i], &src_rows[i])) {
				ret = false;
				break;
			}
		}
		SET_ROWS(dst, rows);
	}

	return ret;
}

void Backtrace_print(Backtrace *ptr) {
	printf("Backtrace:\n");
	BacktraceEntryPtr *rows;
	rows = *GET_ROWS(ptr);
	u64 count = GET_COUNT(ptr);
	for (int i = 0; i < count; i++) {
		char *function_name = *GET_NAME(&rows[i]);
		char *bin_name = *GET_BIN_NAME(&rows[i]);
		char *address = *GET_ADDRESS(&rows[i]);
		char *file_path = *GET_FILE_PATH(&rows[i]);
		printf("#%i:\n\
	[%sfn=%s%s%s%s']\n\
	[%sbinary=%s'%s%s%s'] [%saddress=%s%s]\n\
	[%scode=%s'%s%s%s']\n",
		       i, DIMMED, RESET, GREEN, function_name, RESET, DIMMED,
		       RESET, MAGENTA, bin_name, RESET, DIMMED, RESET, address,
		       DIMMED, RESET, CYAN, file_path, RESET);
	}
}

bool Backtrace_fn_name(Backtrace *ptr, char *buffer, usize len, u64 index) {
	u64 count = GET_COUNT(ptr);
	if (index >= count)
		return false;

	BacktraceEntryPtr *rows;
	rows = *GET_ROWS(ptr);

	strncpy(buffer, *GET_NAME(&rows[index]), len);

	return true;
}

bool Backtrace_bin_name(Backtrace *ptr, char *buffer, usize len, u64 index) {
	u64 count = GET_COUNT(ptr);
	if (index >= count)
		return false;

	BacktraceEntryPtr *rows;
	rows = *GET_ROWS(ptr);

	strncpy(buffer, *GET_BIN_NAME(&rows[index]), len);
	return true;
}

bool Backtrace_address(Backtrace *ptr, char *buffer, usize len, u64 index) {
	u64 count = GET_COUNT(ptr);
	if (index >= count)
		return false;

	BacktraceEntryPtr *rows;
	rows = *GET_ROWS(ptr);

	strncpy(buffer, *GET_ADDRESS(&rows[index]), len);
	return true;
}

bool Backtrace_file_path(Backtrace *ptr, char *buffer, usize len, u64 index) {
	u64 count = GET_COUNT(ptr);
	if (index >= count)
		return false;

	BacktraceEntryPtr *rows;
	rows = *GET_ROWS(ptr);

	strncpy(buffer, *GET_FILE_PATH(&rows[index]), len);
	return true;
}
