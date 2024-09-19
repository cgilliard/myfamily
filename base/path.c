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

#include <base/misc.h>
#include <base/path.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void path_cleanup(PathImpl *ptr)
{
	chain_free(&ptr->ptr);
}

int path_for(Path *p, const char *path)
{
	if (path == NULL) {
		errno = EINVAL;
		return -1;
	}
	int len = strlen(path);
	if (len == 0) {
		errno = EINVAL;
		return -1;
	}
	if (chain_malloc(&p->ptr, len + 1))
		return -1;
	strcpy(p->ptr.data, path);
	return 0;
}

int path_replace_home(Path *p)
{
	const char *home_dir = getenv("HOME");
	if (home_dir == NULL) {
		errno = EINVAL;
		return -1;
	}
	if (((char *)(p->ptr.data))[0] == '~') {
		int nlen = strlen(home_dir) + strlen(PATH_SEPARATOR) + strlen(p->ptr.data);
		if (nlen >= PATH_MAX) {
			errno = E2BIG;
			return -1;
		}
		if (nlen >= p->ptr.len) {
			FatPtr nptr;
			if (chain_realloc(&nptr, &p->ptr, nlen + 1))
				return -1;
			p->ptr = nptr;
		}
		char buf[PATH_MAX + 1];
		snprintf(buf, PATH_MAX, "%s%s%s", home_dir, PATH_SEPARATOR, (char *)(p->ptr.data + 1));
		strcpy(p->ptr.data, buf);
	}
	return 0;
}

int path_canonicalize(Path *p)
{
	if (path_replace_home(p))
		return -1;
	char buf[PATH_MAX];
	errno = 0;
	if (realpath(p->ptr.data, buf) == NULL) {
		return -1;
	}
	int nlen = strlen(buf);
	if (nlen == 0) {
		errno = EFAULT;
		return -1;
	}
	if (nlen >= p->ptr.len) {
		FatPtr nptr;
		if (chain_realloc(&nptr, &p->ptr, nlen + 1))
			return -1;
		p->ptr = nptr;
	}
	strcpy(p->ptr.data, buf);
	return 0;
}
int path_push(Path *p, const char *next)
{
	if (next == NULL) {
		errno = EINVAL;
		return -1;
	}
	int slen = strlen(p->ptr.data);
	if (slen <= 0) {
		errno = EFAULT;
		return -1;
	}
	bool need_sep = false;
	if (((char *)p->ptr.data)[slen - 1] != PATH_SEPARATOR_CHAR) {
		need_sep = true;
	}

	int nlen = slen + strlen(next);
	if (need_sep)
		nlen += strlen(PATH_SEPARATOR);

	if (nlen >= p->ptr.len) {
		FatPtr nptr;
		if (chain_realloc(&nptr, &p->ptr, nlen + 1))
			return -1;
		p->ptr = nptr;
	}

	if (need_sep)
		strcat(p->ptr.data, PATH_SEPARATOR);
	strcat(p->ptr.data, next);
	return 0;
}
int path_pop(Path *p)
{
	char *res = rstrstr(p->ptr.data, PATH_SEPARATOR);
	if (res) {
		int index = res - (char *)p->ptr.data;
		((char *)(p->ptr.data))[index] = 0;
	} else {
		((char *)(p->ptr.data))[0] = '.';
		((char *)(p->ptr.data))[1] = 0;
	}
	return 0;
}

char *path_to_string(const Path *p)
{
	return p->ptr.data;
}

char *path_file_name(const Path *p)
{
	char *ret = rstrstr(p->ptr.data, PATH_SEPARATOR);
	if (ret != NULL && strlen(ret) > 0)
		return ret + 1;

	return ret;
}

bool path_exists(const Path *p)
{
	if (p->ptr.data == NULL && p->ptr.len == 0) {
		errno = EINVAL;
		return false;
	}
	return access(p->ptr.data, F_OK) == 0;
}
bool path_is_dir(const Path *p)
{
	if (p->ptr.data == NULL && p->ptr.len == 0) {
		errno = EINVAL;
		return false;
	}
	struct stat s;
	if (stat(p->ptr.data, &s) == 0) {
		return s.st_mode & S_IFDIR;
	}
	return false;
}

bool path_mkdir(Path *p, mode_t mode, bool parent)
{
	if (p->ptr.data == NULL && p->ptr.len == 0) {
		errno = EINVAL;
		printf("einval\n");
		return false;
	}

	struct stat s;

	// Check if the path exists
	if (stat(p->ptr.data, &s) == 0) {
		// If the path exists and it's a directory, return success
		if (s.st_mode & S_IFDIR) {
			return true;
		} else {
			// Path exists but it's not a directory
			printf("exists");
			errno = ENOTDIR;
			return false;
		}
	}

	// If parent is false, attempt to create only the target directory
	if (!parent) {
		printf("!parent\n");
		if (mkdir(p->ptr.data, mode) != 0) {
			printf("parent false return false\n");
			return false;
		}
		return true;
	}

	// Create the directory structure (parent is true)
	Path temp_path;
	if (path_copy(&temp_path, p) != 0) {
		return false;
	}

	char *dir_part = strtok(temp_path.ptr.data, PATH_SEPARATOR);
	Path current_path;
	path_for(&current_path, PATH_SEPARATOR);

	// Iterate through each part of the path and create directories as needed
	while (dir_part != NULL) {
		if (path_push(&current_path, dir_part) != 0) {
			return false;
		}

		// Check if the current part of the path exists, and if not, create it
		if (stat(current_path.ptr.data, &s) != 0) {
			// Directory does not exist, so create it
			if (mkdir(current_path.ptr.data, mode) != 0) {
				return false;
			}
		} else if (!(s.st_mode & S_IFDIR)) {
			// Path exists but is not a directory
			errno = ENOTDIR;
			return false;
		}

		dir_part = strtok(NULL, PATH_SEPARATOR);
	}

	return true;
}

int path_copy(Path *dst, const Path *src)
{
	return path_for(dst, path_to_string(src));
}
