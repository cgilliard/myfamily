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

#include <base/ekinds.h>
#include <base/option.h>
#include <base/tlmalloc.h>
#include <base/unit.h>
#include <parser/parser.h>
#include <stdlib.h>

GETTER(TokenStream, fp)
SETTER(TokenStream, fp)
GETTER(TokenStream, line)
SETTER(TokenStream, line)
GETTER(TokenStream, line_off)
SETTER(TokenStream, line_off)
GETTER(TokenStream, line_len)
SETTER(TokenStream, line_len)

void TokenStream_cleanup(TokenStream *strm) {
	FILE *fp = *TokenStream_get_fp(strm);
	if (fp)
		fclose(fp);
}

void Span_cleanup(Span *span) {}

Result Span_display(Span *span, ErrorLevel lvl, char *message) { todo() }

void Group_cleanup(Group *group) {}
void Ident_cleanup(Ident *ident) {}
void Literal_cleanup(Literal *literal) {}
void Punct_cleanup(Punct *punct) {}
void TokenTree_cleanup(TokenTree *tree) {}

Result get_next_line(TokenStream *strm) {
	char *line = *TokenStream_get_line(strm);
	u64 line_off = *TokenStream_get_line_off(strm);
	u64 line_len = *TokenStream_get_line_len(strm);
	if (line == NULL || line_off >= line_len) {
	}

	return Ok(None);
}

Result TokenStream_next(TokenStream *strm) {
	Result lres = get_next_line(strm);
	Option lopt = *(Option *)Try(lres);

	if (!lopt.is_some())
		return Ok(None);

	todo()
}

Result TokenStream_parse(char *path, u64 flags) {
	char *path_copy = tlmalloc(sizeof(char) * (1 + strlen(path)));
	strcpy(path_copy, path);
	TokenStream strm =
	    BUILD(TokenStream, NULL, 0, 0, path_copy, NULL, NULL, 0, 0, flags);
	FILE *fp = fopen(path, "r");

	TokenStream_set_fp(&strm, fp);
}
