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

#include <base/resources.h>
#include <stdio.h>
#include <util/replace.h>
#include <util/trie.h>

int replace_file(Path *in_path, Path *out_path, const char *patterns_in[],
	const bool is_case_sensisitive[], const char *replace[], int count)
{
	FILE *file = myfopen(path_to_string(in_path), "rb"); // Open the file in binary mode
	if (!file) {
		perror("File opening failed");
		return -1;
	}

	// Move the file pointer to the end of the file to get its size
	fseek(file, 0, SEEK_END);
	long file_size = ftell(file); // Get the size of the file
	fseek(file, 0, SEEK_SET); // Move file pointer back to the beginning

	// Allocate memory to store the contents (+1 for null terminator)
	char buffer[file_size + 1];

	// Read the contents of the file into the buffer
	if (fread(buffer, 1, file_size, file) < file_size) {
		perror("fread");
		myfclose(file);
		return -1;
	}
	buffer[file_size] = '\0'; // Null-terminate the buffer

	myfclose(file); // Close the file

	TrieMatch tm[count];
	Trie t;
	trie_build(&t, patterns_in, is_case_sensisitive, count);
	int matches = trie_match(&t, buffer, tm, count);

	file = myfopen(path_to_string(out_path), "wb"); // Open file for writing
	if (!file) {
		perror("File opening failed");
		return -1;
	}

	int itt = 0;

	for (int i = 0; i < matches; i++) {
		if (tm[i].offset > itt) {
			int v = fwrite(buffer + itt, sizeof(char), tm[i].offset - itt, file);
			if (v < tm[i].offset - itt) {
				perror("fwrite");
				myfclose(file);
				return -1;
			}
			itt = tm[i].offset;
		}
		fprintf(file, "%s", replace[tm[i].pattern_id]);
		itt += tm[i].len;
	}
	if (itt < file_size) {
		if (fwrite(buffer + itt, sizeof(char), file_size - itt, file) < file_size - itt) {
			perror("fwrite");
			myfclose(file);
			return -1;
		}
	}

	myfclose(file);

	return 0;
}
