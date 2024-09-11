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

#include <base/test.h>
#include <bible/bible.h>

MySuite(bible);

MyTest(bible, test_bible_basic) {
	Bible bible;
	char bible_path[100];
	strcpy(bible_path, "./resources/akjv.txt");
	int res = bible_build(&bible, bible_path);
	if (res)
		perror("bible build");
	cr_assert(!res);

	FILE *fp = fopen("./resources/akjv.txt", "r");
	char buf[1024];
	int line_num = 0;
	int last_chapter = 0;
	int last_verse = 0;
	char last_book[100];
	strcpy(last_book, "");

	while (true) {
		if (fgets(buf, 1024, fp) == NULL)
			break;
		int end_book = strstr(buf, "|") - buf;
		if (end_book < 0)
			continue; // 1 empty line in akjv.txt
		char bookname[end_book + 1];
		memcpy(bookname, buf, end_book);
		bookname[end_book] = 0;

		int start_chapter = end_book + 2;
		int end_chapter = strstr(buf + start_chapter, "|") - buf;
		cr_assert(end_chapter > start_chapter);

		char chapter[(end_chapter - start_chapter) + 1];
		memcpy(chapter, buf + start_chapter, end_chapter - start_chapter);
		chapter[end_chapter - start_chapter] = 0;

		int start_verse = end_chapter + 2;
		int end_verse = strstr(buf + start_verse, "|") - buf;

		cr_assert(end_verse > start_verse);

		char verse[(end_verse - start_verse) + 1];
		memcpy(verse, buf + start_verse, end_verse - start_verse);
		verse[end_verse - start_verse] = 0;

		int start_text = end_verse + 2;
		int end_text = strstr(buf, "\n") - buf;

		// deal with last line in file which has no newline
		if (end_text < 0)
			end_text = strlen(buf);
		cr_assert(end_text > start_text);
		char text[(end_text - start_text) + 1];
		memcpy(text, buf + start_text, end_text - start_text);
		text[end_text - start_text] = 0;

		int verse_int = atoi(verse);
		int chapter_int = atoi(chapter);

		char compare[1024];
		snprintf(compare, 1024, "[%s:%i:%i] %s", bookname, chapter_int, verse_int, text);

		char buf_out[1024];
		int res =
			bible_verse_to_string(&bible, bookname, chapter_int, verse_int, buf_out, 1024, false);

		cr_assert(res != -1);

		cr_assert(!strcmp(buf_out, compare));

		if (last_chapter != chapter_int) {
			// try to get invalid verse
			if (line_num != 0) {
				res = bible_verse_to_string(&bible, last_book, last_chapter, last_verse + 1,
											buf_out, 1024, false);
				cr_assert_eq(res, -1);
			}
		}
		if (strcmp(last_book, bookname) && strcmp(last_book, "")) {
			// new book try to get invalid chapter
			res =
				bible_verse_to_string(&bible, last_book, last_chapter + 1, 1, buf_out, 1024, false);
			cr_assert_eq(res, -1);
		}

		last_chapter = chapter_int;
		last_verse = verse_int;
		strcpy(last_book, bookname);
		line_num++;
	}

	// try invalid book
	char buf_out[1024];
	res = bible_verse_to_string(&bible, "", 1, 1, buf_out, 1024, false);
	cr_assert_eq(res, -1);

	fclose(fp);

	bible_cleanup(&bible);
}

MyTest(bible, load_basic) {
	Bible bible;
	char bible_path[100];
	strcpy(bible_path, "./resources/akjv.txt");
	int res = bible_build(&bible, bible_path);
	if (res)
		perror("bible build");
	cr_assert(!res);
	bible_cleanup(&bible);
}
