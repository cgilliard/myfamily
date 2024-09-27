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
#include <base/resources.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void path_cleanup(PathImpl *ptr) {
	if (ptr->ptr)
		myfree(ptr->ptr);
}

int path_for(Path *p, const char *path) {
	if (p == NULL) {
		errno = EINVAL;
		return -1;
	}
	if (path == NULL) {
		errno = EINVAL;
		p->ptr = NULL;
		return -1;
	}
	int len = strlen(path);
	if (len == 0) {
		errno = EINVAL;
		p->ptr = NULL;
		return -1;
	}
	p->ptr = mymalloc(len + 1);
	if (!p->ptr)
		return -1;
	p->len = len;
	strcpy(p->ptr, path);
	return 0;
}

int path_replace_home(Path *p) {
	const char *home_dir = getenv("HOME");
	if (home_dir == NULL) {
		errno = EINVAL;
		return -1;
	}
	if (((char *)(p->ptr))[0] == '~') {
		int nlen = strlen(home_dir) + strlen(PATH_SEPARATOR) + strlen(p->ptr);
		if (nlen >= PATH_MAX) {
			errno = E2BIG;
			return -1;
		}
		if (nlen >= p->len) {
			void *nptr = myrealloc(p->ptr, nlen + 1);
			if (nptr == NULL)
				return -1;
			p->ptr = nptr;
			p->len = nlen;
		}
		char buf[PATH_MAX + 1];
		if (strlen(p->ptr) > 1 && ((char *)(p->ptr))[1] == PATH_SEPARATOR_CHAR)
			snprintf(buf, PATH_MAX, "%s%s", home_dir, (char *)(p->ptr + 1));
		else
			snprintf(buf, PATH_MAX, "%s%s%s", home_dir, PATH_SEPARATOR, (char *)(p->ptr + 1));
		strcpy(p->ptr, buf);
	}
	return 0;
}

int path_canonicalize(Path *p) {
	if (path_replace_home(p))
		return -1;
	char buf[PATH_MAX];
	errno = 0;
	if (realpath(p->ptr, buf) == NULL) {
		return -1;
	}
	int nlen = strlen(buf);
	if (nlen == 0) {
		errno = EFAULT;
		return -1;
	}
	if (nlen >= p->len) {
		void *nptr = myrealloc(p->ptr, nlen + 1);
		if (nptr == NULL)
			return -1;
		p->ptr = nptr;
		p->len = nlen;
	}
	strcpy(p->ptr, buf);
	return 0;
}
int path_push(Path *p, const char *next) {
	if (p == NULL) {
		errno = EINVAL;
		return -1;
	}
	if (next == NULL) {
		errno = EINVAL;
		return -1;
	}
	int slen = strlen(p->ptr);
	if (slen <= 0 || strlen(next) == 0) {
		errno = EFAULT;
		return -1;
	}
	bool need_sep = false;
	if (((char *)p->ptr)[slen - 1] != PATH_SEPARATOR_CHAR) {
		need_sep = true;
	}

	int nlen = slen + strlen(next);
	if (need_sep)
		nlen += strlen(PATH_SEPARATOR);

	if (nlen >= p->len) {
		void *nptr = myrealloc(p->ptr, nlen + 1);
		if (nptr == NULL)
			return -1;
		p->ptr = nptr;
		p->len = nlen;
	}

	if (need_sep)
		strcat(p->ptr, PATH_SEPARATOR);
	strcat(p->ptr, next);
	return 0;
}
int path_pop(Path *p) {
	const char *res = rstrstr(p->ptr, PATH_SEPARATOR);
	if (res) {
		int index = res - (char *)p->ptr;
		((char *)(p->ptr))[index] = 0;
	} else {
		((char *)(p->ptr))[0] = '.';
		((char *)(p->ptr))[1] = 0;
	}
	return 0;
}

char *path_to_string(const Path *p) {
	return p->ptr;
}

const char *path_file_name(const Path *p) {
	const char *ret = rstrstr(p->ptr, PATH_SEPARATOR);
	if (ret != NULL && strlen(ret) > 0)
		return ret + 1;

	return ret;
}

bool path_exists(const Path *p) {
	if (p->ptr == NULL || p->len == 0) {
		errno = EINVAL;
		return false;
	}
	return access(p->ptr, F_OK) == 0;
}
bool path_is_dir(const Path *p) {
	if (p->ptr == NULL || p->len == 0) {
		errno = EINVAL;
		return false;
	}
	struct stat s;
	if (stat(p->ptr, &s) == 0) {
		return s.st_mode & S_IFDIR;
	}
	return false;
}

bool path_mkdir(Path *p, u64 mode, bool parent) {
	if (p->ptr == NULL || p->len == 0) {
		errno = EINVAL;
		return false;
	}

	struct stat s;

	// Check if the path exists
	if (stat(p->ptr, &s) == 0) {
		// If the path exists and it's a directory, return success
		if (s.st_mode & S_IFDIR) {
			return true;
		} else {
			// Path exists but it's not a directory
			errno = ENOTDIR;
			return false;
		}
	}

	// If parent is false, attempt to create only the target directory
	if (!parent) {
		if (mkdir(p->ptr, mode) != 0) {
			return false;
		}
		return true;
	}

	// Create the directory structure (parent is true)
	Path temp_path;
	if (path_copy(&temp_path, p) != 0) {
		return false;
	}

	int i = 0;
	char path_bufs[20][PATH_MAX];
	while (true) {
		if (path_mkdir(&temp_path, mode, false)) {
			for (int j = i - 1; j >= 0; j--) {
				path_push(&temp_path, path_bufs[j]);
				if (!path_mkdir(&temp_path, mode, false))
					return false;
			}
			return true;
		}
		const char *dir_part = path_file_name(&temp_path);
		path_pop(&temp_path);
		strcpy(path_bufs[i], dir_part);
		if (path_file_name(&temp_path) == NULL)
			break;
		i++;
		if (i == 20)
			break;
	}
	return false;
}

int path_copy(Path *dst, const Path *src) {
	return path_for(dst, path_to_string(src));
}

int path_file_stem(const Path *p, char *buf, u64 limit) {
	// Copy the file name into the buffer, ensuring proper null termination
	strncpy(buf, path_file_name(p), limit - 1);
	buf[limit - 1] = '\0'; // Manually ensure null-termination

	u64 buflen = strlen(buf); // Compute length once
	for (ssize_t i = buflen - 1; i >= 0; i--) {
		if (buf[i] == '.') {
			buf[i] = '\0'; // Null-terminate at the dot
			break;
		} else if (buf[i] == '/') {
			break; // Stop if we encounter a directory separator
		}
	}
	return 0;
}
