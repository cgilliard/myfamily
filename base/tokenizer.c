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

GETTER(Ident, value)
SETTER(Ident, value)
GETTER(Literal, value)
SETTER(Literal, value)
SETTER(Punct, ch)
SETTER(Punct, second_ch)
SETTER(Punct, third_ch)
SETTER(Token, ttype)
GETTER(Tokenizer, pos)
SETTER(Tokenizer, pos)
GETTER(Tokenizer, in_comment)
SETTER(Tokenizer, in_comment)
GETTER(Tokenizer, ref)
SETTER(Tokenizer, ref)

void Ident_cleanup(Ident *ptr) {
	StringRefPtr value = *Ident_get_value(ptr);
	cleanup(&value);
}

Result Ident_fmt(Ident *ptr, Formatter *formatter) {
	StringRefPtr value = *Ident_get_value(ptr);
	Result r = Formatter_write(formatter, "Ident[\"%s\"]", to_str(&value));
	Try(r);
	return Ok(UNIT);
}
bool Ident_clone(Ident *dst, Ident *src) {
	StringRefPtr value_src = *Ident_get_value(src);
	Result r = deep_copy(&dst->_value, &value_src);
	return r.is_ok();
}
usize Ident_size(Ident *obj) { return sizeof(Ident); }
char *Ident_to_str(Ident *obj) {
	StringRefPtr value = *Ident_get_value(obj);
	return to_str(&value);
}
Ident Ident_build(char *s) {
	StringRef ref = STRINGP(s);
	IdentPtr ret = BUILD(Ident);
	Result r = deep_copy(&ret._value, &ref);
	Expect(r);

	return ret;
}

Result Literal_fmt(Literal *ptr, Formatter *formatter) {
	StringRefPtr value = *Literal_get_value(ptr);
	Result r =
	    Formatter_write(formatter, "Literal[\"%s\"]", to_str(&value));
	Try(r);
	return Ok(UNIT);
}

void Literal_cleanup(Literal *ptr) {
	StringRefPtr value = *Literal_get_value(ptr);
	cleanup(&value);
}
bool Literal_clone(Literal *dst, Literal *src) {
	StringRefPtr value_src = *Literal_get_value(src);
	Result r = deep_copy(&dst->_value, &value_src);
	return r.is_ok();
}
usize Literal_size(Literal *obj) { return sizeof(Literal); }
char *Literal_to_str(Literal *obj) {
	StringRefPtr value = *Literal_get_value(obj);
	return to_str(&value);
}
Literal Literal_build(char *s) {
	StringRef ref = STRINGP(s);
	LiteralPtr ret = BUILD(Literal);
	Result r = deep_copy(&ret._value, &ref);
	Expect(r);

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

Result Punct_fmt(Punct *ptr, Formatter *formatter) {
	signed char p_ch = *Punct_get_ch(ptr);
	signed char p_second_ch = *Punct_get_second_ch(ptr);
	signed char p_third_ch = *Punct_get_third_ch(ptr);
	Result r;

	if (p_second_ch == 0)
		r = Formatter_write(formatter, "Punct[\"%c\"]", p_ch);
	else if (p_third_ch == 0)
		r = Formatter_write(formatter, "Punct[\"%c%c\"]", p_ch,
				    p_second_ch);
	else
		r = Formatter_write(formatter, "Punct[\"%c%c%c\"]", p_ch,
				    p_second_ch, p_third_ch);
	Try(r);
	return Ok(UNIT);
}

void Punct_cleanup(Punct *ptr) {}
bool Punct_clone(Punct *dst, Punct *src) {
	Punct_set_ch(dst, *Punct_get_ch(src));
	Punct_set_second_ch(dst, *Punct_get_second_ch(src));
	Punct_set_third_ch(dst, *Punct_get_third_ch(src));
	return true;
}
usize Punct_size(Punct *obj) { return sizeof(Punct); }

Result Token_fmt(Token *obj, Formatter *formatter) {
	u64 ttype = *Token_get_ttype(obj);
	if (ttype == IdentType) {
		IdentPtr *ident = *(Ident **)Token_get_ident(obj);
		Result r0 = to_string(ident);
		StringRef s = *(StringRef *)Try(r0);
		Result r = Formatter_write(formatter, "Token[%s]", to_str(&s));
		Try(r);
	} else if (ttype == LiteralType) {
		LiteralPtr *literal = *(Literal **)Token_get_literal(obj);
		Result r0 = to_string(literal);
		StringRef s = *(StringRef *)Try(r0);
		Result r = Formatter_write(formatter, "Token[%s]", to_str(&s));
		Try(r);
	} else if (ttype == PunctType) {
		PunctPtr *punct = *(Punct **)Token_get_punct(obj);
		Result r0 = to_string(punct);
		StringRef s = *(StringRef *)Try(r0);
		Result r = Formatter_write(formatter, "Token[%s]", to_str(&s));
		Try(r);
	} else {
		Result r = Formatter_write(formatter, "Token[]");
		Try(r);
	}
	return Ok(UNIT);
}

usize Token_size(Token *obj) { return sizeof(Token); }

void Token_cleanup(Token *ptr) {
	TokenType ttype = *Token_get_ttype(ptr);
	if (ttype == IdentType) {
		cleanup(ptr->_ident);
		tlfree(ptr->_ident);
	} else if (ttype == LiteralType) {
		cleanup(ptr->_literal);
		tlfree(ptr->_literal);
	} else if (ttype == PunctType) {
		cleanup(ptr->_punct);
		tlfree(ptr->_punct);
	}
	Token_set_ttype(ptr, NoType);
}
bool Token_clone(Token *dst, Token *src) {
	TokenType ttype = *Token_get_ttype(src);
	Token_set_ttype(dst, ttype);
	if (ttype == IdentType) {
		dst->_ident = tlmalloc(sizeof(Ident));
		return clone(dst->_ident, src->_ident);
	} else if (ttype == LiteralType) {
		dst->_literal = tlmalloc(sizeof(Literal));
		return clone(dst->_literal, src->_literal);
	} else if (ttype == PunctType) {
		dst->_punct = tlmalloc(sizeof(Punct));
		return clone(dst->_punct, src->_punct);
	} else {
		return true;
	}

	return true;
}
Token Build_token_ident(Ident ident) {
	TokenPtr ret = BUILD(Token, IdentType);
	ret._ident = tlmalloc(sizeof(Ident));
	clone(ret._ident, &ident);
	return ret;
}
Token Build_token_literal(Literal literal) {
	TokenPtr ret = BUILD(Token, LiteralType);
	ret._literal = tlmalloc(sizeof(Literal));
	clone(ret._literal, &literal);
	return ret;
}
Token Build_token_punct(Punct punct) {
	TokenPtr ret = BUILD(Token, PunctType);
	ret._punct = tlmalloc(sizeof(Punct));
	clone(ret._punct, &punct);
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

Result Tokenizer_skip_white_space(Tokenizer *ptr) {
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

Result Tokenizer_proc_ident(Tokenizer *ptr) {
	StringRefPtr *ref = (StringRef *)Tokenizer_get_ref(ptr);
	u64 rlen = len(ref);
	u64 pos = *Tokenizer_get_pos(ptr);
	u64 start = pos;
	while (true) {
		if (pos >= rlen)
			break;
		Result r = char_at(ref, pos);
		if (*(signed char *)unwrap(&r) == ' ')
			break;
		pos += 1;
	}
	u64 end = pos;
	Tokenizer_set_pos(ptr, end);
	Result ret = SUBSTRING(ref, start, end);
	StringRef sptr = *(StringRef *)unwrap(&ret);
	Ident ident = IDENT(to_str(&sptr));
	Token token = TOKEN(ident);
	Result rtoken = to_string(&token);
	StringRef sreftoken = *(StringRef *)unwrap(&rtoken);
	Option opt = Some(token);
	return Ok(opt);
}

Result Tokenizer_proc_literal(Tokenizer *ptr) { return Ok(None); }

Result Tokenizer_proc_punct(Tokenizer *ptr) { return Ok(None); }

Result Tokenizer_next_token(Tokenizer *ptr) {
	StringRefPtr *ref = (StringRef *)Tokenizer_get_ref(ptr);
	u64 rlen = len(ref);
	u64 pos = *Tokenizer_get_pos(ptr);
	printf("pos=%i,len=%i\n", pos, rlen);
	if (pos >= rlen) {
		printf("ret none\n");
		return Ok(None);
	}

	Result skip = Tokenizer_skip_white_space(ptr);
	Try(skip);

	pos = *Tokenizer_get_pos(ptr);
	Result r = char_at(ref, pos);
	char ch = *(signed char *)unwrap(&r);

	if (ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' || ch == '_') {
		return Tokenizer_proc_ident(ptr);
	} else if (ch >= '0' && ch <= '9' || ch == '\"' || ch == '\'') {
		return Tokenizer_proc_literal(ptr);
	} else
		return Tokenizer_proc_punct(ptr);

	/*
	u64 start = pos;
	while (true) {
		if (pos >= rlen)
			break;
		Result r = char_at(ref, pos);
		if (*(signed char *)unwrap(&r) == ' ')
			break;
		pos += 1;
	}
	u64 end = pos;
	Tokenizer_set_pos(ptr, end + 1);
	Result ret = SUBSTRING(ref, start, end);
	StringRef sptr = *(StringRef *)unwrap(&ret);
	Ident ident = IDENT(to_str(&sptr));
	Token token = TOKEN(ident);
	Option opt = Some(token);
	return Ok(opt);
	*/
}

Result Tokenizer_parse(StringRef *s) {
	Tokenizer ret = BUILD(Tokenizer, ._pos = 0, ._in_comment = false);
	StringRefPtr copy;
	Result r = deep_copy(&copy, s);
	Tokenizer_set_ref(&ret, copy);
	return Ok(ret);
}

usize Tokenizer_size(Tokenizer *obj) { return sizeof(Tokenizer); }
