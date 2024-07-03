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

#include <base/option.h>
#include <base/tokenizer.h>
#include <base/unit.h>

SETTER(Token, ttype)
SETTER(Token, text)
GETTER(Tokenizer, pos)
SETTER(Tokenizer, pos)
GETTER(Tokenizer, ref)
SETTER(Tokenizer, ref)

Result Token_fmt(Token *obj, Formatter *formatter) {
	TokenType ttype = *Token_get_ttype(obj);
	StringRefPtr token = *Token_get_text(obj);

	if (ttype == IdentType) {
		Result r = WRITE(formatter, "Token[Ident[%s]]", to_str(&token));
		Try(Unit, r);
	} else if (ttype == LiteralType) {
		Result r =
		    WRITE(formatter, "Token[Literal[%s]]", to_str(&token));
		Try(Unit, r);
	} else if (ttype == PunctType) {
		Result r = WRITE(formatter, "Token[Punct[%s]]", to_str(&token));
		Try(Unit, r);
	} else if (ttype == DocType) {
		Result r = WRITE(formatter, "Token[Doc[%s]]", to_str(&token));
		Try(Unit, r);
	} else {
		Result r = WRITE(formatter, "Token[Unknown]");
		Try(Unit, r);
	}
	return Ok(UNIT);
}

Result Token_dbg(Token *obj, Formatter *f) {
	StringRefPtr token = *Token_get_text(obj);
	Result r = WRITE(f, "%s", to_str(&token));
	Try(Unit, r);
	return Ok(UNIT);
}

void Token_cleanup(Token *ptr) {
	TokenType ttype = *Token_get_ttype(ptr);
	if (ttype != NoType) {
		cleanup(&ptr->_text);
	}
	Token_set_ttype(ptr, NoType);
}
bool Token_clone(Token *dst, Token *src) {
	TokenType ttype = *Token_get_ttype(src);
	Token_set_ttype(dst, ttype);
	Result r = deep_copy(&dst->_text, &src->_text);
	return r.is_ok();
}

Token Token_build(TokenType ttype, char *text) {
	StringRefPtr ref = STRINGP(text);
	TokenPtr ret = BUILD(Token, ttype, ref);
	return ret;
}

Token Token_build_lit_num(i128 value) {
	char is_negative[2];
	if (value < 0) {
		value = value * -1;
		is_negative[0] = '-';
		is_negative[1] = 0;
	} else {
		is_negative[0] = 0;
	}

	uint64_t high = (uint64_t)(value >> 64);
	uint64_t low = (uint64_t)value;
	char s[100];
	if (high != 0)
		snprintf(s, 100, "%s%llu%llu", is_negative, high, low);
	else
		snprintf(s, 100, "%s%llu", is_negative, low);
	StringRefPtr ref = STRINGP(s);
	TokenPtr ret = BUILD(Token, LiteralType, ref);
	return ret;
}

void Tokenizer_cleanup(Tokenizer *ptr) {
	StringRefPtr *ref = (StringRef *)Tokenizer_get_ref(ptr);
	cleanup(ref);
}
bool Tokenizer_clone(Tokenizer *dst, Tokenizer *src) {
	Tokenizer_set_pos(dst, *Tokenizer_get_pos(src));
	Tokenizer_set_in_comment(dst, *Tokenizer_get_in_comment(src));
	StringRefPtr dc = BUILD(StringRef);
	StringRefPtr *src_ref = (StringRef *)Tokenizer_get_ref(src);
	Result r = deep_copy(&dc, src_ref);
	Tokenizer_set_ref(dst, dc);
	return true;
}

Result do_skip_white_space(Tokenizer *ptr) {
	StringRefPtr *ref = (StringRef *)Tokenizer_get_ref(ptr);
	u64 rlen = len(ref);
	u64 pos = *Tokenizer_get_pos(ptr);

	while (true) {
		if (pos >= rlen)
			break;

		Result r = char_at(ref, pos);
		char ch = *(signed char *)unwrap(&r);
		if (!(ch == ' ' || ch == '\r' || ch == '\t' || ch == '\n' ||
		      ch == '\v' || ch == '\f'))
			break;
		pos += 1;
	}

	Tokenizer_set_pos(ptr, pos);
	return Ok(UNIT);
}
Result do_skip_comments(Tokenizer *ptr) {
	StringRefPtr *ref = (StringRef *)Tokenizer_get_ref(ptr);
	u64 rlen = len(ref);
	u64 pos = *Tokenizer_get_pos(ptr);
	u64 start_doc_comment = 0;

	if (pos + 1 < rlen) {
		// look for comments
		Result r = char_at(ref, pos);
		char ch = *(signed char *)unwrap(&r);
		Result r2 = char_at(ref, pos + 1);
		char ch2 = *(signed char *)unwrap(&r2);
		if (ch == '/' && ch2 == '/') {
			pos += 2;
			if (pos < rlen) { // check for doc comment
				Result rc = char_at(ref, pos);
				char chc = *(signed char *)unwrap(&rc);
				if (chc == '/') {
					// this is a doc comment. we need to
					// return it
					start_doc_comment = pos + 1;
					pos += 1;
				}
			}
			bool in_start_comment = true;
			while (true) {
				if (pos >= rlen) {
					break;
				}
				Result r3 = char_at(ref, pos);
				char ch3 = *(signed char *)unwrap(&r3);
				if ((ch3 == ' ' || ch3 == '\t' || ch3 == '\v' ||
				     ch3 == '\f') &&
				    in_start_comment &&
				    start_doc_comment != 0) {
					start_doc_comment += 1;
				} else {
					in_start_comment = false;
				}
				if (ch3 == '\n')
					break;
				pos += 1;
			}
		}
		if (ch == '/' && ch2 == '*') {
			pos += 2;
			while (true) {
				if (pos >= rlen) {
					Tokenizer_set_in_comment(ptr, true);
					break;
				}
				Result r3 = char_at(ref, pos - 1);
				char close_ch1 = *(signed char *)unwrap(&r3);
				Result r4 = char_at(ref, pos);
				char close_ch2 = *(signed char *)unwrap(&r4);

				if (close_ch1 == '*' && close_ch2 == '/') {
					pos += 1;
					break;
				}

				pos += 1;
			}
		}
	}

	Tokenizer_set_pos(ptr, pos);

	if (start_doc_comment) {
		// a doc comment was found, return the substring.
		Result r = SUBSTRING(*ref, start_doc_comment, pos);
		StringRef comment = Try(StringRef, r);
		Option opt = Some(comment);
		return Ok(opt);
	} else {
		return Ok(None);
	}
}

Result Tokenizer_skip_in_comment(Tokenizer *ptr) {
	StringRefPtr *ref = (StringRef *)Tokenizer_get_ref(ptr);
	u64 rlen = len(ref);
	u64 pos = *Tokenizer_get_pos(ptr);
	while (true) {
		if (pos >= rlen)
			break;
		if (pos > 0) {
			Result r1 = char_at(ref, pos - 1);
			char ch1 = Try(signed char, r1);
			Result r2 = char_at(ref, pos);
			char ch2 = Try(signed char, r2);
			if (ch1 == '*' && ch2 == '/') {
				pos += 1;
				Tokenizer_set_in_comment(ptr, false);
				break;
			}
		}
		pos += 1;
	}
	Tokenizer_set_pos(ptr, pos);
	return Ok(UNIT);
}

Result Tokenizer_skip_to_token(Tokenizer *ptr, bool in_comment) {
	if (in_comment) {
		Result r0 = Tokenizer_skip_in_comment(ptr);
		Try(Unit, r0);
	}
	while (true) {
		Result r1 = do_skip_white_space(ptr);
		Try(Unit, r1);
		Result r2 = do_skip_comments(ptr);
		Option opt = Try(Option, r2);
		if (opt.is_some()) {
			StringRef sref = *(StringRef *)unwrap(&opt);
			Option ret = Some(sref);
			return Ok(ret);
		}
		StringRefPtr *ref = (StringRef *)Tokenizer_get_ref(ptr);
		u64 rlen = len(ref);
		u64 pos = *Tokenizer_get_pos(ptr);
		if (pos >= rlen)
			break;
		Result r = char_at(ref, pos);
		char ch = *(signed char *)unwrap(&r);
		if (!(ch == ' ' || ch == '\r' || ch == '\t' || ch == '\n' ||
		      ch == '\v' || ch == '\f'))
			break;
	}

	return Ok(None);
}

Result Tokenizer_proc_ident(Tokenizer *ptr) {
	StringRefPtr *ref = (StringRef *)Tokenizer_get_ref(ptr);
	u64 rlen = len(ref);
	u64 pos = *Tokenizer_get_pos(ptr);
	u64 start = pos;
	while (true) {
		if (pos >= rlen)
			break;
		Result r = char_at(ref, pos);
		char ch = *(signed char *)unwrap(&r);
		if (!((ch <= 'Z' && ch >= 'A') || (ch <= 'z' && ch >= 'a') ||
		      (ch <= '9' && ch >= '0') || ch == '_'))
			break;
		pos += 1;
	}
	u64 end = pos;
	Tokenizer_set_pos(ptr, end);
	Result ret = SUBSTRING(*ref, start, end);
	StringRef sptr = *(StringRef *)unwrap(&ret);
	Token token = TOKEN(IdentType, to_str(&sptr));
	Option opt = Some(token);
	return Ok(opt);
}

Result Tokenizer_proc_literal(Tokenizer *ptr) {
	StringRefPtr *ref = (StringRef *)Tokenizer_get_ref(ptr);
	u64 rlen = len(ref);
	u64 pos = *Tokenizer_get_pos(ptr);
	u64 start = pos;

	Result r1 = char_at(ref, pos);
	char ch1 = *(signed char *)unwrap(&r1);

	while (true) {
		if (pos >= rlen)
			break;
		Result r = char_at(ref, pos);
		char ch = *(signed char *)unwrap(&r);
		if (ch1 == '\"') {
			if (pos != start && ch == '\"') {
				pos += 1;
				break;
			}
		} else if (ch1 == '\'') {
			if (pos != start && ch == '\'') {
				pos += 1;
				break;
			}
		} else if ((ch > '9' || ch < '0') && ch != '.') {
			break;
		}
		pos += 1;
	}
	u64 end = pos;
	Tokenizer_set_pos(ptr, end);
	Result ret = SUBSTRING(*ref, start, end);
	StringRef sptr = *(StringRef *)unwrap(&ret);
	Token token = TOKEN(LiteralType, to_str(&sptr));
	Option opt = Some(token);
	return Ok(opt);
}

bool is_second_punct(char ch1, char ch2) {
	if (ch1 == '!' && ch2 == '=')
		return true;
	if (ch1 == '%' && ch2 == '%')
		return true;
	if (ch1 == '&' && (ch2 == '&' || ch2 == '='))
		return true;
	if (ch1 == '*' && ch2 == '=')
		return true;
	if (ch1 == '+' && ch2 == '=')
		return true;
	if (ch1 == '-' && (ch2 == '=' || ch2 == '>'))
		return true;
	if (ch1 == '/' && ch2 == '=')
		return true;
	if (ch1 == '^' && ch2 == '=')
		return true;
	if (ch1 == '|' && (ch2 == '=' || ch2 == '|'))
		return true;
	if (ch1 == '=' && (ch2 == '>' || ch2 == '='))
		return true;
	if (ch1 == ':' && ch2 == ':')
		return true;
	if (ch1 == '<' && (ch2 == '<' || ch2 == '='))
		return true;
	if (ch1 == '>' && (ch2 == '>' || ch2 == '='))
		return true;
	if (ch1 == '.' && ch2 == '.')
		return true;

	return false;
}
bool is_third_punct(char ch1, char ch2, char ch3) {
	if (ch1 == '<' && ch2 == '<' && ch3 == '=')
		return true;
	if (ch1 == '>' && ch2 == '>' && ch3 == '=')
		return true;
	if (ch1 == '.' && ch2 == '.' && ch3 == '.')
		return true;
	return false;
}

Result Tokenizer_proc_punct(Tokenizer *ptr) {
	StringRefPtr *ref = (StringRef *)Tokenizer_get_ref(ptr);
	u64 rlen = len(ref);
	u64 pos = *Tokenizer_get_pos(ptr);
	u64 start = pos;
	Result r1 = char_at(ref, pos);
	char ch = *(signed char *)unwrap(&r1);
	char ch2 = 0;
	char ch3 = 0;

	if (pos + 1 < rlen) {
		Result r2 = char_at(ref, pos + 1);
		ch2 = *(signed char *)unwrap(&r2);
	}
	if (pos + 2 < rlen) {
		Result r3 = char_at(ref, pos + 2);
		ch3 = *(signed char *)unwrap(&r3);
	}

	if (!is_second_punct(ch, ch2)) {
		ch2 = 0;
		ch3 = 0;
	}

	if (!is_third_punct(ch, ch2, ch3)) {
		ch3 = 0;
	}

	if (ch3)
		Tokenizer_set_pos(ptr, pos + 3);
	else if (ch2)
		Tokenizer_set_pos(ptr, pos + 2);
	else
		Tokenizer_set_pos(ptr, pos + 1);

	char punct[4];
	punct[0] = ch;
	punct[1] = ch2;
	punct[2] = ch3;
	punct[3] = 0;
	Token token = TOKEN(PunctType, punct);
	Option opt = Some(token);
	return Ok(opt);
}

Result Tokenizer_next_token(Tokenizer *ptr) {
	StringRefPtr *ref = (StringRef *)Tokenizer_get_ref(ptr);
	u64 rlen = len(ref);
	u64 pos = *Tokenizer_get_pos(ptr);
	bool in_comment = *Tokenizer_get_in_comment(ptr);

	if (pos >= rlen) {
		return Ok(None);
	}

	Result skip = Tokenizer_skip_to_token(ptr, in_comment);
	Option res = Try(Option, skip);
	if (res.is_some()) {
		StringRef comment = *(StringRef *)unwrap(&res);
		Token token = TOKEN(DocType, to_str(&comment));
		Option opt = Some(token);
		return Ok(opt);
	}
	pos = *Tokenizer_get_pos(ptr);
	if (pos >= rlen) {
		return Ok(None);
	}
	Result r = char_at(ref, pos);
	char ch = *(signed char *)unwrap(&r);

	if (ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' || ch == '_') {
		return Tokenizer_proc_ident(ptr);
	} else if (ch >= '0' && ch <= '9' || ch == '\"' || ch == '\'') {
		return Tokenizer_proc_literal(ptr);
	} else
		return Tokenizer_proc_punct(ptr);
}

Result Tokenizer_parse(StringRef *s) {
	Tokenizer ret = BUILD(Tokenizer, ._pos = 0, ._in_comment = false);
	StringRefPtr copy;
	Result r = deep_copy(&copy, s);
	Tokenizer_set_ref(&ret, copy);
	return Ok(ret);
}
