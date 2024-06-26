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

#include <base/tokenizer.h>

GETTER(Ident, value)
SETTER(Ident, value)
GETTER(Literal, value)
SETTER(Literal, value)
SETTER(Punct, ch)
SETTER(Punct, second_ch)
SETTER(Punct, third_ch)
SETTER(Token, ttype)

void Ident_cleanup(Ident *ptr) {
	StringRefPtr value = *Ident_get_value(ptr);
	cleanup(&value);
}
bool Ident_clone(Ident *dst, Ident *src) {
	StringRefPtr value_src = *Ident_get_value(src);
	StringRefPtr value_copy;

	Result cr = deep_copy(&value_copy, &value_src);
	if (!cr.is_ok())
		return false;
	Ident_set_value(dst, value_copy);
	return true;
}
usize Ident_size(Ident *obj) { return sizeof(Ident); }
char *Ident_to_str(Ident *obj) {
	StringRefPtr value = *Ident_get_value(obj);
	return to_str(&value);
}
Ident Ident_build(char *s) {
	StringRefPtr ref = STRINGP(s);
	IdentPtr ret = BUILD(Ident, ref);
	return ret;
}

void Literal_cleanup(Literal *ptr) {
	StringRefPtr value = *Literal_get_value(ptr);
	cleanup(&value);
}
bool Literal_clone(Literal *dst, Literal *src) {
	StringRefPtr value_src = *Literal_get_value(src);
	StringRefPtr value_copy;
	if (!clone(&value_copy, &value_src))
		return false;
	Literal_set_value(dst, value_copy);
	return true;
}
usize Literal_size(Literal *obj) { return sizeof(Literal); }
char *Literal_to_str(Literal *obj) {
	StringRefPtr value = *Literal_get_value(obj);
	return to_str(&value);
}
Literal Literal_build(char *s) {
	StringRefPtr ref = STRINGP(s);
	LiteralPtr ret = BUILD(Literal, ref);
	return ret;
}
Literal Literal_build_num(i128 value) {
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
	StringRefPtr sref = STRINGP(s);
	LiteralPtr ret = BUILD(Literal, sref);
	return ret;
}
void Punct_cleanup(Punct *ptr) {}
bool Punct_clone(Punct *dst, Punct *src) {
	Punct_set_ch(dst, *Punct_get_ch(src));
	Punct_set_second_ch(dst, *Punct_get_second_ch(src));
	Punct_set_third_ch(dst, *Punct_get_third_ch(src));
	return true;
}
usize Punct_size(Punct *obj) { return sizeof(Punct); }

void Token_cleanup(Token *ptr) {
	TokenType ttype = *Token_get_ttype(ptr);
	if (ttype == IdentType) {
		cleanup(&ptr->_ident);
	} else if (ttype == LiteralType) {
		cleanup(&ptr->_literal);
	} else if (ttype == PunctType) {
		cleanup(&ptr->_punct);
	}
	Token_set_ttype(ptr, NoType);
}
bool Token_clone(Token *dst, Token *src) {
	TokenType ttype = *Token_get_ttype(src);
	Token_set_ttype(dst, ttype);
	if (ttype == IdentType) {
		return clone(&dst->_ident, &src->_ident);
	} else if (ttype == LiteralType) {
		return clone(&dst->_literal, &src->_literal);
	} else if (ttype == PunctType) {
		return clone(&dst->_punct, &src->_punct);
	} else {
		return true;
	}
}
Token Build_token_ident(Ident ident) {
	TokenPtr ret = BUILD(Token, IdentType, ._ident = ident);
	return ret;
}
Token Build_token_literal(Literal literal) {
	TokenPtr ret = BUILD(Token, LiteralType, ._literal = literal);
	return ret;
}
Token Build_token_punct(Punct punct) {
	TokenPtr ret = BUILD(Token, PunctType, ._punct = punct);
	return ret;
}

void Tokenizer_cleanup(Tokenizer *ptr) {}
bool Tokenizer_clone(Tokenizer *dst, Tokenizer *src) { return false; }
Result Tokenizer_next_token(Tokenizer *ptr) { todo(); }
Result Tokenizer_parse(StringRef *s) { todo(); }
usize Tokenizer_size(Tokenizer *obj) { return sizeof(Tokenizer); }
