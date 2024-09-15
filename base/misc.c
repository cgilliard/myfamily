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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *rstrstr(const char *s1, const char *s2)
{
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

#define BUFFER_SIZE 1024

int copy_file(const char *dst, const char *src)
{
	FILE *source_file, *dest_file;
	char buffer[BUFFER_SIZE];
	size_t bytes;

	// Open the source file in binary read mode
	source_file = fopen(src, "rb");
	if (source_file == NULL) {
		perror("Error opening source file");
		return -1;
	}

	// Open the destination file in binary write mode
	dest_file = fopen(dst, "wb");
	if (dest_file == NULL) {
		perror("Error opening destination file");
		fclose(source_file);
		return -1;
	}

	// Copy the file content
	while ((bytes = fread(buffer, 1, BUFFER_SIZE, source_file)) > 0) {
		if (fwrite(buffer, 1, bytes, dest_file) != bytes) {
			perror("Error writing to destination file");
			fclose(source_file);
			fclose(dest_file);
			return -1;
		}
	}

	// Close both files
	fclose(source_file);
	fclose(dest_file);

	return 0;
}
