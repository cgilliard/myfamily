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

#include <bible/bible.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int bible_checksum(char *data) {
	char *sha = SHA256(data);

	// sha 256 checksum of the akjv bible from
	// https://github.com/bible-hub/Bibles/blob/master/English__American_King_James_Version__akjv__LTR.txt
	if (strcmp(sha, "9fac349223681483dd1d225cd04a298cef93f3efeb21b562469a1a"
					"8efa7069ff")) {
		errno = EPERM;
		return -1;
	}
	return 0;
}

int bible_parse_verse(Bible *bible, u64 index, char *buf) {
	const char *strstr_res = strstr(buf, "|");
	if (strstr_res == NULL) {
		fprintf(stderr, "invalid line %s, index=%" PRIu64 "\n", buf, index);
		errno = EINVAL;
		return -1;
	}
	int end_book = strstr_res - buf;
	if (end_book < 0) {
		fprintf(stderr, "invalid line %s, index=%" PRIu64 "\n", buf, index);
		errno = EINVAL;
		return -1;
	}
	char bookname[end_book + 1];
	memcpy(bookname, buf, end_book);
	bookname[end_book] = 0;

	int start_chapter = end_book + 2;
	int end_chapter = strstr(buf + start_chapter, "|") - buf;
	if (end_chapter < start_chapter) {
		fprintf(stderr, "invalid line %s, index=%" PRIu64 ", end_chapter=%i,start_chap=%i\n", buf,
				index, end_chapter, start_chapter);
		errno = EINVAL;
		return -1;
	}

	char chapter[(end_chapter - start_chapter) + 1];
	memcpy(chapter, buf + start_chapter, end_chapter - start_chapter);
	chapter[end_chapter - start_chapter] = 0;

	int start_verse = end_chapter + 2;
	int end_verse = strstr(buf + start_verse, "|") - buf;

	if (end_verse < start_verse) {
		fprintf(stderr, "invalid line %s, index=%" PRIu64 ", end_chapter=%i,start_chap=%i\n", buf,
				index, end_verse, start_verse);
		errno = EINVAL;
		return -1;
	}

	char verse[(end_verse - start_verse) + 1];
	memcpy(verse, buf + start_verse, end_verse - start_verse);
	verse[end_verse - start_verse] = 0;

	int start_text = end_verse + 2;
	int end_text = strstr(buf, "\n") - buf;

	// deal with last line in file which has no newline
	if (end_text < 0)
		end_text = strlen(buf);
	if (end_text < start_text) {
		fprintf(stderr, "invalid line %s, index=%" PRIu64 ", end_chapter=%i,start_chap=%i\n", buf,
				index, end_verse, start_verse);
		errno = EINVAL;
		return -1;
	}
	char text[(end_text - start_text) + 1];
	memcpy(text, buf + start_text, end_text - start_text);
	text[end_text - start_text] = 0;

	bible->verses[index].text = malloc(sizeof(char) * (end_text - start_text) + 1);
	strcpy(bible->verses[index].text, text);
	bible->verses[index].chapter_id = strtol(chapter, NULL, 10);
	bible->verses[index].verse_id = strtol(verse, NULL, 10);

	// find book
	if (bible->book_id_count == 0) {
		// first book
		bible->book_ids = malloc(sizeof(BibleBookIdMap));
		bible->book_ids[0].book_id = 0;
		bible->book_ids[0].name = malloc(sizeof(char) * (strlen(bookname) + 1));
		strcpy(bible->book_ids[0].name, bookname);
		bible->book_id_count += 1;
		bible->verses[index].book_id = 0;
	} else {
		char *cur_book = bible->book_ids[bible->book_id_count - 1].name;
		// check for new book
		if (!strcmp(cur_book, bookname)) {
			// we're still in the same book
			bible->verses[index].book_id = bible->book_ids[bible->book_id_count - 1].book_id;
		} else {
			// new book, update book ids
			bible->verses[index].book_id = bible->book_ids[bible->book_id_count - 1].book_id + 1;
			void *tmp =
				realloc(bible->book_ids, sizeof(BibleBookIdMap) * (bible->book_id_count + 1));
			if (!tmp)
				return -1;
			bible->book_ids = tmp;
			// next higher book id
			bible->book_ids[bible->book_id_count].book_id =
				bible->book_ids[bible->book_id_count - 1].book_id + 1;
			bible->book_ids[bible->book_id_count].name =
				malloc(sizeof(char) * (strlen(bookname) + 1));
			strcpy(bible->book_ids[bible->book_id_count].name, bookname);
			bible->verses[index].book_id = bible->book_ids[bible->book_id_count].book_id;
			bible->book_id_count += 1;
		}
	}

	return 0;
}

int bible_book_id_map_compare(const void *a, const void *b) {
	return strcmp(((BibleBookIdMap *)a)->name, ((BibleBookIdMap *)b)->name);
}

int bible_build(Bible *bible, char *path) {
	FILE *fp = fopen(path, "r");
	if (fp == NULL) {
		return -1;
	}

	char bible_check[BIBLE_CHECKSUM_SCAN_BUF_SIZE];
	int len = fread(bible_check, sizeof(*bible_check), BIBLE_CHECKSUM_SCAN_BUF_SIZE, fp);
	bible_check[len] = 0;

	if (bible_checksum(bible_check))
		return -1;

	fseek(fp, 0, SEEK_SET);

	char buf[VERSE_BUF_LEN];

	bible->verses = NULL;
	bible->verse_count = 0;
	bible->book_ids = NULL;
	bible->book_id_count = 0;

	int max_len = 0;
	while (fgets(buf, VERSE_BUF_LEN, fp)) {
		if (strlen(buf) > max_len)
			max_len = strlen(buf);
		if (bible->verse_count == 0) {
			bible->verses = malloc(sizeof(BibleVerse));
			if (bible->verses == NULL)
				return -1;
		} else {
			void *tmp = realloc(bible->verses, sizeof(BibleVerse) * (bible->verse_count + 1));
			if (tmp == NULL)
				return -1;
			bible->verses = tmp;
		}
		int end_book = strstr(buf, "|") - buf;
		if (end_book < 0) {
			// there is at least one empty line in the text
			continue;
		}
		if (bible_parse_verse(bible, bible->verse_count, buf))
			return -1;
		bible->verse_count += 1;
	}

	// sort the books for quicker lookups
	bible->book_ids_sorted = malloc(sizeof(BibleVerse) * (bible->verse_count));
	if (bible->book_ids_sorted == NULL)
		return -1;
	memcpy(bible->book_ids_sorted, bible->book_ids, bible->book_id_count * sizeof(BibleBookIdMap));
	qsort(bible->book_ids_sorted, bible->book_id_count, sizeof(BibleBookIdMap),
		  bible_book_id_map_compare);

	fclose(fp);
	return 0;
}

void bible_cleanup(Bible *bible) {
	for (int i = 0; i < bible->verse_count; i++)
		free(bible->verses[i].text);
	for (int i = 0; i < bible->book_id_count; i++)
		free(bible->book_ids[i].name);
	free(bible->verses);
	free(bible->book_ids);
	free(bible->book_ids_sorted);
}

int format_verse(Bible *bible, int index, char *buf, int buf_len, bool colors) {

	if (colors)
		return snprintf(buf, buf_len, "[%s%s%s:%s%i:%i%s] %s%s%s", GREEN,
						bible->book_ids[bible->verses[index].book_id].name, RESET, CYAN,
						bible->verses[index].chapter_id, bible->verses[index].verse_id, RESET,
						YELLOW, bible->verses[index].text, RESET);
	else
		return snprintf(buf, buf_len, "[%s:%i:%i] %s",
						bible->book_ids[bible->verses[index].book_id].name,
						bible->verses[index].chapter_id, bible->verses[index].verse_id,
						bible->verses[index].text);
}

int bible_book_index(Bible *bible, char *book) {
	// use binary search to find the book
	int index;
	int min = 0;
	int max = bible->book_id_count - 1;
	int book_id;
	while (true) {
		if (max < min) {
			errno = EINVAL;
			return -1;
		}
		index = min + ((max - min) / 2);
		int strcmp_res = strcmp(book, bible->book_ids_sorted[index].name);
		if (strcmp_res == 0) {
			book_id = bible->book_ids_sorted[index].book_id;
			break;
		} else if (strcmp_res < 0) {
			max = index - 1;
		} else {
			min = index + 1;
		}
	}
	return book_id;
}

int bible_verse_to_string(Bible *bible, char *book, u8 chapter, u8 verse, char *buf, int buf_len,
						  bool colors) {
	if (book == NULL || strlen(book) == 0) {
		errno = EINVAL;
		return -1;
	}
	return bible_random_verse_to_string(bible, buf, buf_len, colors, book, &chapter, &verse);
}

int bible_random_verse_to_string(Bible *bible, char *buf, int buf_len, bool colors,
								 char *optional_book, u8 *optional_chapter, u8 *optional_verse) {
	u16 r;
	if (rand_u16(&r))
		return -1;

	if (optional_book && strlen(optional_book) > 0) {
		int book_id = bible_book_index(bible, optional_book);
		if (book_id < 0)
			return -1;

		int index;
		int min = 0;
		int max = bible->verse_count;

		// use binary search to find the verse

		while (true) {
			if (max < min) {
				errno = EINVAL;
				return -1;
			}
			index = min + ((max - min) / 2);
			if (book_id == bible->verses[index].book_id) {
				min = index;
				max = index;
				while (min - 1 >= 0 && bible->verses[min - 1].book_id == book_id)
					min--;
				while (max + 1 < bible->verse_count && bible->verses[max + 1].book_id == book_id)
					max++;
				break;
			} else if (book_id < bible->verses[index].book_id) {
				max = index - 1;
			} else {
				min = index + 1;
			}
		}

		if (optional_chapter) {
			u8 target_chapter = *optional_chapter;
			while (true) {
				if (max < min) {
					errno = EINVAL;
					return -1;
				}
				index = min + ((max - min) / 2);
				if (target_chapter == bible->verses[index].chapter_id) {
					min = index;
					max = index;
					while (min - 1 >= 0 && bible->verses[min - 1].chapter_id == target_chapter &&
						   bible->verses[min - 1].book_id == book_id)
						min--;
					while (max + 1 < bible->verse_count &&
						   bible->verses[max + 1].chapter_id == target_chapter &&
						   bible->verses[max + 1].book_id == book_id)
						max++;

					if (optional_verse) {
						u8 target_verse = *optional_verse;
						if (target_verse > (1 + max) - min) {
							errno = EINVAL;
							return -1;
						}
						min += target_verse - 1;
						max = min + 2;
					}
					break;
				} else if (target_chapter < bible->verses[index].chapter_id) {
					max = index - 1;
				} else {
					min = index + 1;
				}
			}
		}

		r %= ((max - 1) - min);
		r += (min);
	} else
		r %= bible->verse_count;
	return format_verse(bible, r, buf, buf_len, colors);
}
