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

#define PATH_SEPARATOR "/"
#define PATH_SEPARATOR_CHAR '/'

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
	}
	return 0;
}

char *path_to_string(Path *p)
{
	return p->ptr.data;
}

char *path_file_name(Path *p)
{
	return rstrstr(p->ptr.data, PATH_SEPARATOR);
}

bool path_exists(Path *p)
{
	if (p->ptr.data == NULL && p->ptr.len == 0) {
		errno = EINVAL;
		return false;
	}
	return access(p->ptr.data, F_OK) == 0;
}
bool path_is_dir(Path *p)
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

bool path_mkdir(Path *p, mode_t mode)
{
	if (p->ptr.data == NULL && p->ptr.len == 0) {
		errno = EINVAL;
		return false;
	}
	return mkdir(p->ptr.data, 0700) != 0;
}
