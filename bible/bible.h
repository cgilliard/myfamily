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

#ifndef _BIBLE_BIBLE__
#define _BIBLE_BIBLE__

#include <base/base.h>

#define VERSE_BUF_LEN 1024
#define BIBLE_CHECKSUM_SCAN_BUF_SIZE 4634230

typedef struct BibleBookIdMap {
	u8 book_id;
	char *name;
} BibleBookIdMap;

typedef struct BibleVerse {
	u8 book_id;
	u8 chapter_id;
	u8 verse_id;
	char *text;
} BibleVerse;

typedef struct Bible {
	BibleVerse *verses;
	u16 verse_count;
	BibleBookIdMap *book_ids;
	BibleBookIdMap *book_ids_sorted;
	u8 book_id_count;
} Bible;

int bible_build(Bible *bible, char *path);
void bible_cleanup(Bible *bible);
int bible_verse_to_string(Bible *bible, char *book, u8 chapter, u8 verse, char *buf, int buf_len,
						  bool colors);
int bible_random_verse_to_string(Bible *bible, char *buf, int buf_len, bool colors,
								 char *optional_book, u8 *optional_chapter, u8 *optional_verse);

#endif // _BIBLE_BIBLE__
