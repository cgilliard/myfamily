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

#include <base/colors.h>
#include <base/resources.h>
#include <base/types.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

const char *rstrstr(const char *s1, const char *s2) {
	size_t s1len = strlen(s1);
	size_t s2len = strlen(s2);
	const char *s;

	if (s2len > s1len)
		return NULL;
	for (s = s1 + s1len - s2len; s >= s1; --s)
		if (strncmp(s, s2, s2len) == 0)
			return s;
	return NULL;
}

u64 read_all(void *buffer, size_t size, size_t count, FILE *stream) {
	size_t total_read = 0;
	size_t bytes_to_read = size * count;
	size_t bytes_read;

	while (total_read < bytes_to_read) {
		bytes_read = fread((char *)buffer + total_read, 1, bytes_to_read - total_read, stream);
		if (bytes_read == 0) {
			// Check for EOF or error
			if (feof(stream)) {
				break; // End of file reached
			} else if (ferror(stream)) {
				perror("Read error");
				break; // Error occurred
			}
		}
		total_read += bytes_read;
	}

	return total_read;
}

int copy_file(const Path *dst_path, const Path *src_path) {
	if (dst_path == NULL || src_path == NULL) {
		errno = EINVAL;
		return -1;
	}
	FILE *source_file, *dest_file;
	size_t bytes;

	// Open the source file in binary read mode
	source_file = (FILE *)myfopen(src_path, "rb");
	if (source_file == NULL) {
		errno = EINVAL;
		return -1;
	}

	// Determine the file size
	fseek(source_file, 0, SEEK_END);
	long file_size = ftell(source_file);
	fseek(source_file, 0, SEEK_SET);
	char buffer[file_size];

	// Open the destination file in binary write mode
	dest_file = (FILE *)myfopen(dst_path, "wb");
	if (dest_file == NULL) {
		errno = ENOENT;
		myfclose((MYFILE *)source_file);
		return -1;
	}

	// Copy the file content
	while ((bytes = fread(buffer, 1, file_size, source_file)) > 0) {
		if (fwrite(buffer, 1, bytes, dest_file) != bytes) {
			errno = EIO;
			myfclose((MYFILE *)source_file);
			myfclose((MYFILE *)dest_file);
			return -1;
		}
	}

	// Close both files
	myfclose((MYFILE *)source_file);
	myfclose((MYFILE *)dest_file);

	return 0;
}

// Function to recursively remove a directory and its contents
int remove_directory(const Path *p, bool preserve_dir) {
	const char *path = path_to_string(p);
	struct dirent *entry;
	DIR *dir = opendir(path);

	if (dir == NULL) {
		return -1;
	}

	while ((entry = readdir(dir)) != NULL) {
		char full_path[PATH_MAX];

		// Skip the special entries "." and ".."
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}

		// Construct the full path to the file/directory
		snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

		struct stat statbuf;
		if (stat(full_path, &statbuf) == -1) {
			perror("stat");
			closedir(dir);
			return -1;
		}

		if (S_ISDIR(statbuf.st_mode)) {
			// It's a directory, recurse into it
			Path full_path_p;
			path_for(&full_path_p, full_path);
			if (remove_directory(&full_path_p, false) == -1) {
				closedir(dir);
				return -1;
			}
		} else {
			// It's a file, unlink (delete) it
			if (unlink(full_path) == -1) {
				perror("unlink");
				closedir(dir);
				return -1;
			}
		}
	}

	closedir(dir);

	// Now the directory is empty, so we can remove it
	if (!preserve_dir) {
		if (rmdir(path) == -1) {
			perror("rmdir");
			return -1;
		}
	}

	return 0;
}

void exit_error(char *format, ...) {
	va_list va_args;
	va_start(va_args, format);
	fprintf(stderr, "%sError%s: ", BRIGHT_RED, RESET);
	vfprintf(stderr, format, va_args);
	fprintf(stderr, "\n");
	va_end(va_args);
	exit(-1);
}
