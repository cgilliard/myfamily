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

#include <base/sys.h>
#include <base/util.h>
#include <sys/mman.h>
#include <time.h>

#ifdef __APPLE__
#include <dlfcn.h>
#endif	// __APPLE__

#define MAX_BACKTRACE_ENTRIES 128

// system calls / std library functions
int backtrace(void **array, int capacity);
char **backtrace_symbols(void **array, int capacity);
void free(void *addr);
void *popen(const char *command, const char *rw);
char *fgets(char *str, int n, void *stream);
int pclose(void *fp);
int printf(const char *fmt, ...);

void *map(u64 pages) {
	if (pages == 0) return NULL;
	void *ret = mmap(NULL, pages * PAGE_SIZE, PROT_READ | PROT_WRITE,
					 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	return ret;
}

void unmap(void *addr, u64 pages) {
	if (pages) munmap(addr, pages * PAGE_SIZE);
}

__int128_t getnanos() {
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return (__int128_t)now.tv_sec * (__int128_t)1e9 + (__int128_t)now.tv_nsec;
}

const char *backtrace_full() {
	void *array[MAX_BACKTRACE_ENTRIES];
	int size = backtrace(array, MAX_BACKTRACE_ENTRIES);
	char **strings = backtrace_symbols(array, size);
	char *ret = map(4);
	bool term = false;
	int len_sum = 0;
	for (int i = 0; i < size; i++) {
		char address[256];
#ifdef __linux__
		int len = cstring_len(strings[i]);
		int last_plus = -1;

		while (len > 0) {
			if (strings[i][len] == '+') {
				last_plus = len;
				break;
			}
			len--;
		}
		if (last_plus > 0) {
			byte *addr = strings[i] + last_plus + 1;
			int itt = 0;
			while (addr[itt]) {
				if (addr[itt] == ')') {
					addr[itt] = 0;
					break;
				}
				itt++;
			}
			u64 address = cstring_strtoull(addr, 16);
			address -= 8;

			char command[256];
			snprintf(command, sizeof(command),
					 "addr2line -f -e ./.bin/test %llx", address);

			void *fp = popen(command, "r");
			char buffer[128];
			while (fgets(buffer, sizeof(buffer), fp) != NULL) {
				int len = cstring_len(buffer);
				if (cstring_strstr(buffer, ".c:")) {
					len_sum += len;
					if (len_sum >= 4 * PAGE_SIZE) break;
					cstring_cat_n(ret, buffer, cstring_len(buffer));
					if (term) {
						i = size;
						break;
					}
				} else if (cstring_is_alpha_numeric(buffer)) {
					if (len && buffer[len - 1] == '\n') {
						len--;
						buffer[len] = ' ';
					}

					len_sum += len;
					if (len_sum >= 4 * PAGE_SIZE) break;
					cstring_cat_n(ret, buffer, cstring_len(buffer));
					if (!cstring_compare(buffer, "main ")) {
						term = true;
					}
				}
			}

			pclose(fp);
		}
#elif defined(__APPLE__)
		Dl_info info;
		dladdr(array[i], &info);
		u64 addr = 0x0000000100000000 + info.dli_saddr - info.dli_fbase;
		u64 offset = (u64)array[i] - (u64)info.dli_saddr;
		addr += offset;
		addr -= 4;
		snprintf(address, sizeof(address), "0x%llx", addr);
		char command[256];
		snprintf(command, sizeof(command),
				 "atos -fullPath -o ./.bin/test -l 0x100000000 %s", address);
		void *fp = popen(command, "r");
		char buffer[128];

		while (fgets(buffer, sizeof(buffer), fp) != NULL) {
			int len = cstring_len(buffer);
			len_sum += len;
			if (len_sum >= 4 * PAGE_SIZE) break;
			cstring_cat_n(ret, buffer, cstring_len(buffer));
			if (cstring_strstr(buffer, "main ") == buffer) {
				i = size;
				break;
			}
		}
		pclose(fp);
#else
		println("WARN: Unsupported OS: cannot build backtraces");
#endif
	}

	if (strings && size) free(strings);
	return ret;
}
const char *__last_trace_impl__() {
	void *array[MAX_BACKTRACE_ENTRIES];
	int size = backtrace(array, MAX_BACKTRACE_ENTRIES);
	char **strings = backtrace_symbols(array, size);
	char output_last[PAGE_SIZE];
	char output_last_last[PAGE_SIZE];
	char output_last_last_last[PAGE_SIZE];
	char *ret = NULL;
	for (int i = 0; i < size; i++) {
		char address[256];
#ifdef __linux__
		int len = cstring_len(strings[i]);
		int last_plus = -1;

		while (len > 0) {
			if (strings[i][len] == '+') {
				last_plus = len;
				break;
			}
			len--;
		}
		if (last_plus > 0) {
			byte *addr = strings[i] + last_plus + 1;
			int itt = 0;
			while (addr[itt]) {
				if (addr[itt] == ')') {
					addr[itt] = 0;
					break;
				}
				itt++;
			}
			u64 address = cstring_strtoull(addr, 16);
			address -= 8;

			char command[256];
			snprintf(command, sizeof(command),
					 "addr2line -f -e ./.bin/test %llx", address);

			void *fp = popen(command, "r");
			char buffer[128];
			char output[1024 * 2];
			int n = sizeof(output);
			output[0] = 0;
			while (n && fgets(buffer, sizeof(buffer), fp) != NULL) {
				cstring_cat_n(output, buffer, n);
				int buf_len = cstring_len(buffer);
				if (buf_len < n)
					n -= buf_len;
				else
					n = 0;
			}

			pclose(fp);
			if (cstring_strstr(output_last_last_last, "__last_trace_impl__")) {
				ret = map(1);
				copy_bytes(ret, output, PAGE_SIZE);
			}

			copy_bytes(output_last_last_last, output_last_last, PAGE_SIZE);
			copy_bytes(output_last_last, output_last, PAGE_SIZE);
			copy_bytes(output_last, output, PAGE_SIZE);
		}
#elif defined(__APPLE__)
		Dl_info info;
		dladdr(array[i], &info);
		u64 addr = 0x0000000100000000 + info.dli_saddr - info.dli_fbase;
		u64 offset = (u64)array[i] - (u64)info.dli_saddr;
		addr += offset;
		addr -= 4;
		snprintf(address, sizeof(address), "0x%llx", addr);
		if (i > 2 && cstring_strstr(strings[i - 3], "__last_trace_impl__")) {
			char command[256];
			snprintf(command, sizeof(command),
					 "atos -fullPath -o ./.bin/test -l 0x100000000 %s",
					 address);
			void *fp = popen(command, "r");
			char buffer[128];
			ret = map(1);

			int len_sum = 0;
			while (fgets(buffer, sizeof(buffer), fp) != NULL) {
				int len = cstring_len(buffer);
				len_sum += len;
				if (len_sum > PAGE_SIZE) break;
				cstring_cat_n(ret, buffer, cstring_len(buffer));
			}
			pclose(fp);

			break;
		}
#else
		println("WARN: Unsupported OS: cannot build backtraces");
#endif	// End MACOS
	}

	if (strings && size) free(strings);
	return ret;
}

const char *last_trace() {
	return __last_trace_impl__();
}

static void check_arch(char *type, int actual, int expected) {
	char buf[30] = {};
	if (actual != expected) {
		write(2, "'", 1);
		write(2, type, cstring_len(type));
		write(2, "' must be ", 10);
		cstring_itoau64(expected, buf, 10, 30);
		write(2, buf, cstring_len(buf));
		write(2, " bytes. It is ", 14);
		cstring_itoau64(actual, buf, 10, 30);
		write(2, buf, cstring_len(buf));
		write(2, " bytes. Arch invalid!\n", 23);
		_exit(-1);
	}
}

#define arch(type, expected) check_arch(#type, sizeof(type), expected)

void __attribute__((constructor)) __check_sizes() {
	char buf[30] = {};
	arch(int, 4);
	arch(long long, 8);
	arch(unsigned long long, 8);
	arch(unsigned long, 8);
	arch(__uint128_t, 16);
	arch(char, 1);
	arch(unsigned char, 1);
	arch(float, 4);
	arch(double, 8);
	if (__SIZEOF_SIZE_T__ != 8) {
		write(2, "size_t must be 8 bytes. It is ", 30);
		cstring_itoau64(__SIZEOF_SIZE_T__, buf, 10, 30);
		write(2, buf, cstring_len(buf));
		write(2, " bytes. Arch invalid.\n", 22);
		_exit(-1);
	}

	// little endian check
	int test = 0x1;
	if (*(byte *)&test != 0x1) {
		write(2, "Big endian is not supported!\n", 29);
		_exit(-1);
	}
}
