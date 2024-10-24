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

#include <base/fam_err.h>
#include <base/os.h>
#include <base/path.h>
#include <base/print_util.h>
#include <base/resources.h>
#include <base/string.h>
#include <base/types.h>

_Thread_local ResourceStats THREAD_LOCAL_RESOURCE_STATS = {0, 0, 0, 0, 0};
bool __is_debug_malloc = false;
bool __is_debug_realloc = false;
u64 __is_debug_malloc_counter_ = UINT64_MAX;
u64 __is_debug_realloc_counter_ = UINT64_MAX;
bool __is_debug_misc_ferror = false;
bool __is_debug_misc_fwrite = false;
bool __is_debug_misc_stat = false;
bool __is_debug_misc_remove_dir = false;
bool __is_debug_misc_unlink = false;
bool __is_debug_misc_no_exit = false;
bool __is_debug_misc_preserve = false;

void *mymalloc(u64 size) {
#ifdef TEST
	if (__is_debug_malloc || __is_debug_malloc_counter_ == 0) {
		__is_debug_malloc_counter_ = UINT64_MAX;
		return NULL;
	}
	__is_debug_malloc_counter_--;
#endif // TEST
	void *ret;
	ret = malloc(size);

	if (ret) {
		THREAD_LOCAL_RESOURCE_STATS.malloc_sum += 1;
	} else
		SetErr(AllocErr);
	return ret;
}
void *myrealloc(void *ptr, u64 size) {
#ifdef TEST
	if (__is_debug_realloc || __is_debug_realloc_counter_ == 0) {
		__is_debug_realloc_counter_ = UINT64_MAX;
		return NULL;
	}
	__is_debug_realloc_counter_--;
#endif // TEST
	void *ret;
	ret = realloc(ptr, size);

	if (ret) {
		THREAD_LOCAL_RESOURCE_STATS.realloc_sum += 1;
	} else
		SetErr(AllocErr);
	return ret;
}
void myfree(void *ptr) {
	THREAD_LOCAL_RESOURCE_STATS.free_sum += 1;
	free(ptr);
}

Stream myfopen(const Path *path, i32 flags, i32 mode) {
	const char *path_str = path_to_string(path);
	i32 ret = open(path_str, flags);
	if (ret > 0) {
		errno = 0;
		chmod(path_str, 0700);
		THREAD_LOCAL_RESOURCE_STATS.fopen_sum += 1;
	}
	return (Stream) {ret};
}

void myfclose(Stream *ptr) {
	THREAD_LOCAL_RESOURCE_STATS.fclose_sum += 1;
	close(ptr->handle);
}

u64 mymalloc_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.malloc_sum;
}
u64 myrealloc_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.realloc_sum;
}
u64 myfree_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.free_sum;
}
u64 myfopen_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.fopen_sum;
}
u64 myfclose_sum() {
	return THREAD_LOCAL_RESOURCE_STATS.fclose_sum;
}

i32 remove_directory(const Path *p, bool preserve_dir) {
	const char *path = path_to_string(p);
	struct dirent *entry;
	DIR *dir = opendir(path);

	if (dir == NULL) {
		return -1;
	}

	u64 max_path = pathconf(path, _PC_PATH_MAX);
	while ((entry = readdir(dir)) != NULL) {
		char full_path[max_path];

		// Skip the special entries "." and ".."
		if (mystrcmp(entry->d_name, ".") == 0 || mystrcmp(entry->d_name, "..") == 0) {
			continue;
		}

		// Construct the full path to the file/directory
		sprint(full_path, sizeof(full_path), "{}{}{}", path, PATH_SEPARATOR, entry->d_name);

		Path full_path_p;
		path_for(&full_path_p, full_path);
		if (!path_exists(&full_path_p)) {
			SetErr(FileNotFound);
			closedir(dir);
			return -1;
		}

		if (path_is_dir(&full_path_p)) {
			// It's a directory, recurse into it
			Path full_path_p;
			path_for(&full_path_p, full_path);
			if (remove_directory(&full_path_p, false) == -1 || __is_debug_misc_remove_dir) {
				closedir(dir);
				return -1;
			}
		} else {
			// It's a file, unlink (delete) it
			if (unlink(full_path) == -1 || __is_debug_misc_unlink) {
				SetErr(IO);
				closedir(dir);
				return -1;
			}
		}
	}

	closedir(dir);

	// Now the directory is empty, so we can remove it
	if (!preserve_dir) {
		if (rmdir(path) == -1 || __is_debug_misc_preserve) {
			SetErr(IO);
			return -1;
		}
	}
	return 0;
}
