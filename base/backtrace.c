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
#include <base/os.h>
#include <base/print_util.h>
#include <base/stream.h>
#include <base/string.h>
#include <base/types.h>

#ifdef __APPLE__
#include <dlfcn.h>
#endif // __APPLE__
#include <unistd.h>

i32 get_file_line(const u8 *bin, const u8 *addr, u8 *line_num, u8 *file_path, i32 max_len) {
	u64 cmd_max_len = strlen(bin) + strlen(addr) + 100;
	u8 cmd[cmd_max_len];
	strcpy(cmd, "");
	sprint(cmd, cmd_max_len, "atos --fullPath -o {} {}", bin, addr);
	Stream strm = {};
	if (strm_open(&strm, cmd, "r")) {
		SetErr(POpenErr);
		return -1;
	}
	u8 buffer[MAX_ENTRY_SIZE + 1];
	while (sgets(buffer, MAX_ENTRY_SIZE + 1, &strm)) {
		bool found_first_paren = false;
		bool found_second_paren = false;
		bool found_colon = false;
		u64 len = strlen(buffer);
		i32 line_num_itt = 0;
		i32 line_count_itt = 0;
		for (i32 i = 0; i < len; i++) {
			if (!found_first_paren && buffer[i] == '(') {
				found_first_paren = true;
			} else if (!found_second_paren && buffer[i] == '(') {
				found_second_paren = true;
			} else if (found_second_paren && found_first_paren) {
				if (found_colon) {
					if (buffer[i] == ')') {
						break;
					} else {
						line_num[line_count_itt] = buffer[i];
						line_num_itt++;
						line_count_itt++;
					}
				} else if (buffer[i] == ':') {
					found_colon = true;
					file_path[line_num_itt] = 0;
					line_num_itt++;
				} else if (buffer[i] == ')') {
					break;
				} else {
					file_path[line_num_itt] = buffer[i];
					line_num_itt++;
					if (line_num_itt == max_len)
						break;
				}
			}
		}
		file_path[line_num_itt] = 0;
		line_num[line_count_itt] = 0;
	}
	strm_close(&strm);
	return 0;
}

void backtrace_set_entry_values(BacktraceEntry *ptr, const u8 *name, const u8 *bin_name,
								const u8 *address, const u8 *file_path) {
	u64 offset = 0;
	ptr->start_bin = MAX_ENTRY_SIZE;
	ptr->start_addr = MAX_ENTRY_SIZE;
	ptr->start_file_path = MAX_ENTRY_SIZE;
	memset(ptr->data, '\0', MAX_ENTRY_SIZE);

	strncpy((u8 *)(ptr->data + offset), name, MAX_ENTRY_SIZE - offset);
	offset += strlen(name) + 1;

	if (offset >= MAX_ENTRY_SIZE)
		return;

	ptr->start_bin = offset;
	strncpy((u8 *)(ptr->data + offset), bin_name, MAX_ENTRY_SIZE - offset);
	offset += strlen(bin_name) + 1;

	if (offset >= MAX_ENTRY_SIZE)
		return;

	ptr->start_addr = offset;
	strncpy((u8 *)(ptr->data + offset), address, MAX_ENTRY_SIZE - offset);
	offset += strlen(address) + 1;

	if (offset >= MAX_ENTRY_SIZE)
		return;

	ptr->start_file_path = offset;
	strncpy((u8 *)(ptr->data + offset), file_path, MAX_ENTRY_SIZE - offset);
	offset += strlen(address) + 1;
}

bool backtrace_add_entry(Backtrace *ptr, const u8 *name, const u8 *bin_name, const u8 *address,
						 const u8 *file_path) {
	u64 count = ptr->cur_entries;
	if (count >= MAX_ENTRIES)
		return false;
	backtrace_set_entry_values(&ptr->entries[count], name, bin_name, address, file_path);
	ptr->cur_entries++;
	return true;
}

i32 backtrace_generate(Backtrace *ptr) {
	if (ptr == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}

	ptr->cur_entries = 0;
	void *array[MAX_ENTRIES];
	i32 size = backtrace(array, MAX_ENTRIES);

	if (size < 0) {
		SetErr(BacktraceErr);
		return -1;
	}

	u8 **strings = backtrace_symbols(array, size);

	if (strings == NULL)
		size = 0;

	for (i32 i = 0; i < size; i++) {
#ifdef __APPLE__
		u64 path_max = pathconf("/", _PC_PATH_MAX);
		u8 address[30];
		strcpy(address, "");
		Dl_info info;
		dladdr(array[i], &info);
		u64 addr = 0x0000000100000000 + info.dli_saddr - info.dli_fbase;
		sprint(address, 30, "0x{hex}", addr);
		u64 snamelen = strlen(info.dli_sname) + 1;
		u64 fnamelen = strlen(info.dli_fname) + 1;
		u8 fn_name[snamelen];
		u8 bin_name[fnamelen];
		strncpy(fn_name, info.dli_sname, snamelen);
		strncpy(bin_name, info.dli_fname, fnamelen);

		u8 file_path[path_max + 101];
		u8 line_num[path_max + 101];
		strcpy(file_path, "");
		strcpy(line_num, "");

		get_file_line(bin_name, address, line_num, file_path, path_max + 100);

		u8 real_bin_name[path_max + 1];
		if (strlen(bin_name) > 0) {
			realpath(bin_name, real_bin_name);
		} else
			strncpy(real_bin_name, bin_name, 1 + strlen(bin_name));

		u8 real_file_path[path_max + 1];
		bool has_file = true;
		bool has_line_no = true;
		if (!strncmp(file_path, "", 1)) {
			strcpy(real_file_path, "Unknown");
			has_file = false;
		} else
			realpath(file_path, real_file_path);
		if (!strncmp(line_num, "", 1)) {
			has_line_no = false;
			strcpy(line_num, "Unknown");
		}

		if (has_line_no && has_file) {
			strncat(file_path, ":", 2);
			strncat(file_path, line_num, 1 + strlen(line_num));
		}

		if (!backtrace_add_entry(ptr, fn_name, real_bin_name, address, file_path))
			break;
#else  // Linux
#endif // OS Specific code
	}

	if (strings)
		release_no_stat(strings);

	return 0;
}

void backtrace_print(const Backtrace *ptr) {
	if (env("CBACKTRACE") != NULL) {
		const BacktraceEntry *rows = ptr->entries;
		u64 count = ptr->cur_entries;
		if (count == 0) {
			println("-------------Backtrace not available-------------");
		} else {
			println("Backtrace:");
			for (i32 i = 0; i < count; i++) {
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
				println("#{}:\n\
	[{}fn={}{}{}{}']\n\
	[{}binary={}'{}{}{}'] [{}address={}{}]\n\
	[{}code={}'{}{}{}']",
						i, DIMMED, RESET, GREEN, function_name, RESET, DIMMED, RESET, CYAN,
						bin_name, RESET, DIMMED, RESET, address, DIMMED, RESET, YELLOW, file_path,
						RESET);
			}
		}
	} else {
		println("Backtrace currently disabled set env variable CBACKTRACE to enable");
	}
}
