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

// This utility is used to build a header for resource directories which can be included
// in binaries.

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int file_count = 0;
#define MAX_FILES 4096
int file_sizes[MAX_FILES];

void print_hex(
	const unsigned char *data, size_t size, FILE *out, const char *file, const char *namespace)
{
	char buf[strlen(file) + 100];
	snprintf(
		buf, strlen(file) + 100, "unsigned char %sxxdir_file_%i[] = {\n", namespace, file_count);
	for (int i = 0; i < strlen(buf); i++)
		if (buf[i] == '.')
			buf[i] = '_';
	fprintf(out, "%s", buf);
	for (size_t i = 0; i < size; i++) {
		fprintf(out, "0x%02x, ", data[i]);
		if ((i + 1) % 16 == 0) {
			fprintf(out, "\n");
		} else {
			fprintf(out, " ");
		}
	}
	file_sizes[file_count] = size;
	fprintf(out, "0x00};\nsize_t %sxxdir_file_size_%i = %zu;\n", namespace, file_count, size);
}

void proc_file(const char *file_path, const char *output_header, FILE *out, const char *namespace)
{
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

int main(int argc, char **argv)
{
	if (argc != 3 && argc != 4) {
		fprintf(stderr, "Usage: xxdir <resource_directory> <header name> <optional namespace>");
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

	DIR *dir = opendir(dir_path);
	if (dir == NULL) {
		perror("Error opening directory");
		exit(-1);
	}

	char initial_text[1024];
	snprintf(initial_text, 1024, "char *%sxxdir_file_names[] = {", namespace);
	// add three bytes for the last strcat
	char *buf = malloc(sizeof(char) * (strlen(initial_text) + 7));
	int cur_alloc = strlen(initial_text);
	strcpy(buf, initial_text);

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL) {
		// Skip the "." and ".." entries
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}

		// Print the name of the file or directory
		printf("Including: %s\n", entry->d_name);
		char full_path[strlen(entry->d_name) + strlen(dir_path) + 100];
		strcpy(full_path, dir_path);
		strcat(full_path, "/");
		strcat(full_path, entry->d_name);

		proc_file(full_path, output_header, out, namespace);

		char *tmp = realloc(buf, cur_alloc + strlen(entry->d_name) + 3);
		if (tmp == NULL) {
			fprintf(stderr, "Alloc error!\n");
			exit(-1);
		}
		cur_alloc += strlen(entry->d_name) + 4;
		buf = tmp;
		strcat(buf, "\"");
		strcat(buf, entry->d_name);
		strcat(buf, "\", ");
	}

	// 7 bytes preallocated
	strcat(buf, "NULL};");
	// write the names to the file
	// fprintf(out, "%s", namespace);
	fprintf(out, "%s", buf);

	fprintf(out, "\nint %sxxdir_file_count=%i;\n", namespace, file_count);

	fprintf(out, "unsigned char *%sxxdir_files[] = {", namespace);
	for (int i = 0; i < file_count; i++)
		fprintf(out, "%sxxdir_file_%i,", namespace, i);
	fprintf(out, "NULL};\n");
	fprintf(out, "size_t %sxxdir_file_sizes[] = { ", namespace);
	for (int i = 0; i < file_count; i++)
		fprintf(out, "%i, ", file_sizes[i]);
	fprintf(out, "0};\n");

	free(buf);

	fclose(out);

	return 0;
}