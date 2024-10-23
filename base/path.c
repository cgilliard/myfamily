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

#include <base/deps.h>
#include <base/fam_err.h>
#include <base/limits.h>
#include <base/misc.h>
#include <base/path.h>
#include <base/print_util.h>
#include <base/resources.h>

bool __is_debug_path_homedir_null = false;

void path_cleanup(PathImpl *ptr) {
	if (ptr->ptr) {
		myfree(ptr->ptr);
		ptr->ptr = NULL;
	}
}

i32 path_for(Path *p, const u8 *path) {
	if (p == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}
	if (path == NULL) {
		SetErr(IllegalArgument);
		p->ptr = NULL;
		return -1;
	}
	i32 len = mystrlen(path);
	if (len == 0) {
		SetErr(IllegalArgument);
		p->ptr = NULL;
		return -1;
	}
	p->ptr = mymalloc(len + 1);
	if (!p->ptr)
		return -1;
	p->len = len;
	mystrcpy(p->ptr, path, len);
	return 0;
}

i32 path_replace_home(Path *p) {
	const u8 *home_dir = getenv("HOME");
	if (home_dir == NULL || __is_debug_path_homedir_null) {
		SetErr(IllegalArgument);
		return -1;
	}
	if (((u8 *)(p->ptr))[0] == '~') {
		i32 nlen = mystrlen(home_dir) + mystrlen(PATH_SEPARATOR) + mystrlen(p->ptr);
		if (nlen >= PATH_MAX) {
			SetErr(TooBig);
			return -1;
		}
		if (nlen >= p->len) {
			void *nptr = myrealloc(p->ptr, nlen + 1);
			if (nptr == NULL)
				return -1;
			p->ptr = nptr;
			p->len = nlen;
		}
		u8 buf[PATH_MAX + 1];
		u64 buf_len;
		if (mystrlen(p->ptr) > 1 && ((u8 *)(p->ptr))[1] == PATH_SEPARATOR_CHAR)
			buf_len = sprint(buf, PATH_MAX, "{}{}", home_dir, (u8 *)(p->ptr + 1));
		else
			buf_len = sprint(buf, PATH_MAX, "{}{}{}", home_dir, PATH_SEPARATOR, (u8 *)(p->ptr + 1));
		mystrcpy(p->ptr, buf, buf_len);
	}
	return 0;
}

u64 path_file_size(Path *p) {
	Stream strm = myfopen(p, O_RDONLY);
	if (strm.handle < 0) {
		SetErr(IO);
		return 0;
	}
	u64 ret = lseek(strm.handle, 0, SEEK_END);
	myfclose(&strm);
	return ret;
}

i32 path_canonicalize(Path *p) {
	u8 buf[PATH_MAX];
	i32 nlen;
	if (path_replace_home(p) || realpath(p->ptr, buf) == NULL || (nlen = mystrlen(buf)) == 0) {
		SetErr(IllegalArgument);
		return -1;
	}
	SetErr(NoErrors);
	if (nlen >= p->len) {
		void *nptr = myrealloc(p->ptr, nlen + 1);
		if (nptr == NULL)
			return -1;
		p->ptr = nptr;
		p->len = nlen;
	}
	mystrcpy(p->ptr, buf, nlen);
	return 0;
}
i32 path_push(Path *p, const u8 *next) {
	if (p == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}
	if (next == NULL) {
		SetErr(IllegalArgument);
		return -1;
	}
	i32 slen = mystrlen(p->ptr);
	if (slen <= 0 || mystrlen(next) == 0) {
		SetErr(IllegalState);
		return -1;
	}
	bool need_sep = false;
	if (((u8 *)p->ptr)[slen - 1] != PATH_SEPARATOR_CHAR) {
		need_sep = true;
	}

	i32 nlen = slen + mystrlen(next);
	if (need_sep)
		nlen += mystrlen(PATH_SEPARATOR);

	if (nlen >= p->len) {
		void *nptr = myrealloc(p->ptr, nlen + 1);
		if (nptr == NULL)
			return -1;
		p->ptr = nptr;
		p->len = nlen;
	}

	if (need_sep)
		mystrcat(p->ptr, PATH_SEPARATOR, PATH_MAX);
	mystrcat(p->ptr, next, PATH_MAX);
	return 0;
}
i32 path_pop(Path *p) {
	const u8 *res = rstrstr(p->ptr, PATH_SEPARATOR);
	if (res) {
		i32 index = res - (u8 *)p->ptr;
		((u8 *)(p->ptr))[index] = 0;
	} else {
		((u8 *)(p->ptr))[0] = '.';
		((u8 *)(p->ptr))[1] = 0;
	}
	return 0;
}

u8 *path_to_string(const Path *p) {
	return p->ptr;
}

const u8 *path_file_name(const Path *p) {
	const u8 *ret = rstrstr(p->ptr, PATH_SEPARATOR);
	if (ret != NULL && mystrlen(ret) > 0)
		return ret + 1;

	return ret;
}

bool path_exists(const Path *p) {
	if (p->ptr == NULL || p->len == 0) {
		SetErr(IllegalArgument);
		return false;
	}

	i32 fd = open(p->ptr, O_RDONLY);
	if (fd == -1) {
		if (errno == ENOENT) {
			return false; // File doesn't exist
		} else {
			// Some other error occurred
			fam_err = IO;
			return false;
		}
	} else {
		close(fd);
		return true; // File exists
	}
}
bool path_is_dir(const Path *p) {
	if (p->ptr == NULL || p->len == 0) {
		SetErr(IllegalArgument);
		return false;
	}

	i32 fd = open(p->ptr, O_RDONLY | O_DIRECTORY);
	if (fd == -1) {
		return false; // Not a directory
	} else {
		close(fd);
		return true; // It's a directory
	}
}

bool path_mkdir(Path *p, u64 mode, bool parent) {
	if (p->ptr == NULL || p->len == 0) {
		SetErr(IllegalArgument);
		return false;
	}
	if (path_exists(p)) {
		if (path_is_dir(p))
			return true;
		return false;
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

	i32 i = 0;
	u8 path_bufs[20][PATH_MAX];
	while (i < 20 && path_file_name(&temp_path) != NULL) {
		if (path_mkdir(&temp_path, mode, false)) {
			bool ret = true;
			for (i32 j = i - 1; ret && j >= 0; j--) {
				path_push(&temp_path, path_bufs[j]);
				ret = path_mkdir(&temp_path, mode, false);
			}
			return true;
		}
		const u8 *dir_part = path_file_name(&temp_path);
		path_pop(&temp_path);
		mystrcpy(path_bufs[i], dir_part, PATH_MAX);
		i++;
	}
	return false;
}

i32 path_copy(Path *dst, const Path *src) {
	return path_for(dst, path_to_string(src));
}

i32 path_file_stem(const Path *p, u8 *buf, u64 limit) {
	if (limit == 0 || !p) {
		SetErr(IllegalArgument);
		return -1;
	}
	// Copy the file name i32o the buffer, ensuring proper null termination
	mystrcpy(buf, path_file_name(p), limit - 1);
	buf[limit - 1] = '\0'; // Manually ensure null-termination

	u64 buflen = mystrlen(buf); // Compute length once
	for (u64 i = buflen - 1; i > 0; i--) {
		if (buf[i] == '.') {
			buf[i] = '\0'; // Null-terminate at the dot
			break;
		}
	}
	return 0;
}
