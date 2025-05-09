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

// This utility is used to build a header for resource directories which can be
// included in binaries.

#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int file_count = 0;
#define MAX_FILES 4096
int file_sizes[MAX_FILES];

void print_hex(const unsigned char *data, unsigned long long size, FILE *out,
			   const char *file, const char *namespace) {
	char buf[strlen(namespace) + 100];
	snprintf(buf, strlen(namespace) + 100,
			 "static unsigned char %sxxdir_file_%i[] = {\n", namespace,
			 file_count);
	for (int i = 0; i < strlen(buf); i++)
		if (buf[i] == '.') buf[i] = '_';
	fprintf(out, "%s", buf);
	for (unsigned long long i = 0; i < size; i++) {
		fprintf(out, "0x%02x, ", data[i]);
		if ((i + 1) % 16 == 0) {
			fprintf(out, "\n");
		} else {
			fprintf(out, " ");
		}
	}
	file_sizes[file_count] = size;
	fprintf(out,
			"0x00};\nstatic unsigned long long %sxxdir_file_size_%i = %llu;\n",
			namespace, file_count, size);
}

void proc_file(const char *file_path, const char *output_header, FILE *out,
			   const char *namespace) {
	FILE *file = fopen(file_path, "rb");
	if (file == NULL) {
		perror("Failed to open file");
		exit(-1);
	}

	// Determine the file size
	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	// Read the file into memory
	unsigned char *data = malloc(file_size);
	if (data == NULL) {
		perror("Failed to allocate memory");
		fclose(file);
		exit(-1);
	}
	fread(data, 1, file_size, file);
	fclose(file);

	// Print the hexadecimal representation
	print_hex(data, file_size, out, file_path, namespace);
	file_count++;

	// Clean up
	free(data);
}

void include_dir(const char *dir_path, const char *base, char *buf,
				 const char *output_header, FILE *out, const char *namespace) {
	DIR *dir = opendir(dir_path);
	if (dir == NULL) {
		perror("Error opening directory");
		exit(-1);
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		// Skip the "." and ".." entries
		if (strcmp(entry->d_name, ".") == 0 ||
			strcmp(entry->d_name, "..") == 0) {
			continue;
		}

		char full_path[strlen(entry->d_name) + strlen(dir_path) + 100];
		strcpy(full_path, dir_path);
		strcat(full_path, "/");
		strcat(full_path, entry->d_name);

		struct stat s;
		if (stat(full_path, &s) == 0 && s.st_mode & S_IFDIR) {
			char next_level[PATH_MAX];
			strcpy(next_level, base);
			if (strlen(base) > 0) strcat(next_level, "/");
			strcat(next_level, entry->d_name);
			include_dir(full_path, next_level, buf, output_header, out,
						namespace);
		} else {
			proc_file(full_path, output_header, out, namespace);

			strcat(buf, "\"");
			if (strlen(base) > 0) {
				// it's in a subdirectory
				strcat(buf, base);
				strcat(buf, "/");
			}
			strcat(buf, entry->d_name);
			strcat(buf, "\", ");
		}
	}
}

int main(int argc, char **argv) {
	if (argc != 3 && argc != 4) {
		fprintf(stderr,
				"Usage: xxdir <resource_directory> <header name> <optional "
				"namespace>");
		exit(-1);
	}

	const char *dir_path = argv[1];
	const char *output_header = argv[2];

	char namespace[1000];
	if (argc == 4) {
		strcpy(namespace, argv[3]);
		strcat(namespace, "_");
	} else {
		strcpy(namespace, "");
	}

	FILE *out = fopen(output_header, "w");
	if (out == NULL) {
		fprintf(stderr, "Could not open output file");
		exit(-1);
	}

	char initial_text[1024];
	if (strlen(namespace) > 128) {
		fprintf(stderr, "namespace is too long. Max len = 128 bytes.");
		exit(-1);
	}
	strcpy(initial_text, "static char *");
	strcat(initial_text, namespace);
	strcat(initial_text, "xxdir_file_names[] = {");
	char buf[1024 * 1024];
	strcpy(buf, initial_text);

	include_dir(dir_path, "", buf, output_header, out, namespace);

	strcat(buf, "(void*)0};");
	fprintf(out, "%s", buf);

	fprintf(out, "\nstatic int %sxxdir_file_count=%i;\n", namespace,
			file_count);

	fprintf(out, "static unsigned char *%sxxdir_files[] = {", namespace);
	for (int i = 0; i < file_count; i++)
		fprintf(out, "%sxxdir_file_%i,", namespace, i);
	fprintf(out, "(void*)0};\n");
	fprintf(out, "static unsigned long long %sxxdir_file_sizes[] = { ",
			namespace);
	for (int i = 0; i < file_count; i++) fprintf(out, "%i, ", file_sizes[i]);
	fprintf(out, "0};\n");

	fclose(out);

	return 0;
}
