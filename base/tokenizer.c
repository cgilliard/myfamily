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
#include <base/tokenizer.h>
#include <base/unit.h>

GETTER(Tokenizer, pos)
SETTER(Tokenizer, pos)
GETTER(Tokenizer, ref)
SETTER(Tokenizer, ref)
GETTER(Tokenizer, in_comment)
SETTER(Tokenizer, in_comment)
SETTER(Token, ttype)
GETTER(Ident, value)
SETTER(Ident, value)

void Ident_cleanup(Ident *ptr) {
	void *value = *Ident_get_value(ptr);
	if (value) {
		tlfree(value);
		Ident_set_value(ptr, NULL);
	}
}
void Literal_cleanup(Literal *ptr) {}
void Punct_cleanup(Punct *ptr) {}

void Token_cleanup(Token *ptr) {
	if (ptr->_ttype == IdentType)
		cleanup(&ptr->_ident);
	// no cleanup for others so for now don't call them
}

void Tokenizer_cleanup(Tokenizer *ptr) {
	StringRefPtr *ref = (StringRef *)Tokenizer_get_ref(ptr);
	cleanup(ref);
}

Result try_parse_ident(Tokenizer *st, Token *ret) {
	char *s = "this";
	ret->_ident._value = tlmalloc(sizeof(char) * (strlen(s) + 1));
	strcpy(ret->_ident._value, s);
	return Ok(UNIT);
}
Result try_parse_punct(Tokenizer *st, Token *ret) { return Ok(UNIT); }
Result try_parse_literal(Tokenizer *st, Token *ret) { return Ok(UNIT); }

Result Tokenizer_next_token(Tokenizer *st) {
	u64 pos = *(u64 *)Tokenizer_get_pos(st);
	StringRefPtr s = *Tokenizer_get_ref(st);
	u64 slen = len(&s);

	if (pos >= slen) {
		return Ok(None);
	}

	Result r1 = char_at(&s, pos);
	signed char first = *(signed char *)Try(r1);

	Token ret = TOKEN_INIT;

	// we can determine what kind of token based on the first char.
	// A - Za - z_

	if ((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z') ||
	    first == '_') {
		// ident
		Token_set_ttype(&ret, IdentType);
		Result r = try_parse_ident(st, &ret);
		Try(r);
	} else if ((first >= '1' && first <= '9') || first == '\"') {
		// literal
		Token_set_ttype(&ret, LiteralType);
		Result r = try_parse_literal(st, &ret);
		Try(r);
	} else {
		// check for punct or comments
		Token_set_ttype(&ret, PunctType);
		Result r = try_parse_punct(st, &ret);
		Try(r);
	}

	TokenPtr *rcpy = tlmalloc(sizeof(Token));
	clone(rcpy, &ret);
	Rc rc = RC(rcpy);
	Option ropt = Some(rc);

	return Ok(ropt);
}

bool Token_clone(Token *dst, Token *src) {
	dst->_ttype = src->_ttype;
	dst->_ident.vdata.vtable = &IdentVtable;
	dst->_ident.vdata.name = "Ident";
	return true;
}

Result Tokenizer_parse(StringRef *s) {
	StringRefPtr scopy;
	copy(&scopy, s);
	Tokenizer ret = BUILD(Tokenizer, 0, scopy, false);
	return Ok(ret);
}

size_t Tokenizer_size(Tokenizer *ptr) { return sizeof(Tokenizer); }

bool Tokenizer_clone(Tokenizer *dst, Tokenizer *src) {
	u64 src_pos = *Tokenizer_get_pos(src);
	bool src_in_comment = *Tokenizer_get_in_comment(src);
	StringRefPtr *src_ref = (StringRefPtr *)Tokenizer_get_ref(src);
	StringRefPtr *dst_ref = (StringRefPtr *)Tokenizer_get_ref(dst);
	if (!copy(dst_ref, src_ref))
		return false;
	Tokenizer_set_pos(dst, src_pos);
	Tokenizer_set_ref(dst, *dst_ref);
	Tokenizer_set_in_comment(dst, src_in_comment);
	return true;
}
