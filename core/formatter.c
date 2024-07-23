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

#include <core/ekinds.h>
#include <core/formatter.h>
#include <core/string.h>
#include <core/unit.h>
#include <errno.h>

#define TO_STRING_BUF_SIZE 10000

GETTER(Formatter, buf)
SETTER(Formatter, buf)
GETTER(Formatter, pos)
SETTER(Formatter, pos)
GETTER(Formatter, len)
GETTER(Formatter, cleanup)

void Formatter_cleanup(Formatter *ptr) {
	bool do_cleanup = GET(Formatter, ptr, cleanup);
	if (do_cleanup) {
		char *buf = GET(Formatter, ptr, buf);
		if (buf) {
			myfree(buf);
			SET(Formatter, ptr, buf, NULL);
		}
	}
}

Result Formatter_write(Formatter *ptr, char *fmt, ...) {
	char *buf = *Formatter_get_buf(ptr);
	u64 len = *Formatter_get_len(ptr);
	u64 pos = *Formatter_get_pos(ptr);
	u64 rem;
	if (pos > len)
		rem = 0;
	else
		rem = len - pos;

	va_list args;
	va_list args2;
	va_copy(args2, args);
	va_start(args, fmt);
	i32 r = vsnprintf(buf + pos, rem, fmt, args);
	if (r < 0) {
		char *msg = strerror(errno);
		Error err = ERR(WRITE_ERROR, "%s", msg);
		return Err(err);
	}
	va_end(args);

	if (r < rem) {
		Formatter_set_pos(ptr, pos + r);
		return Ok(UNIT);
	} else if (r >= rem) {
		va_start(args2, fmt);
		i32 r2 = vsnprintf(NULL, 0, fmt, args2);
		if (r2 > rem) {
			Error err = ERR(OVERFLOW,
					"formatted string would have been %i "
					"bytes. Only %llu bytes were available",
					r2, rem);
			return Err(err);
		} else {
			Formatter_set_pos(ptr, pos + r);
			return Ok(UNIT);
		}
	}
	return Ok(UNIT);
}

/*
Result Formatter_write2(Formatter *formatter, String fmt, ...) {
	int n = 0;

	u64 itt = 0;
	while (true) {
		Result r = String_index_of_s(&fmt, "{}", itt);
		i64 v = TRY(r, v);
		if (v < 0)
			break;
		itt = v + 2;
		n += 1;
	}

	va_list ptr;
	va_start(ptr, n);

	itt = 0;
	String fmt_str = STRING("");
	for (int i = 0; i < n; i++) {
		Result r = String_index_of_s(&fmt, "{}", itt);
		i64 v = TRY(r, v);
		if (v < 0)
			break;
		String sub = SUBSTRING(&fmt, itt, v);
		Result r2 = append(&fmt_str, &sub);
		Result r3 = WRITE(&formatter, unwrap(&sub));
		Object *next = va_arg(ptr, void *);
		if (CLASS_NAME(next) == NULL) {
			printf("null\n");
		}
		Result r4 = to_string(next);
		String s4 = TRY(r4, s4);
		Result r5 = WRITE(&formatter, unwrap(&s4));

		itt = v + 2;
	}
	String sub = SUBSTRING(&fmt, itt, len(&fmt));
	Result r5 = WRITE(&formatter, unwrap(&sub));
	va_end(ptr);
	return Ok(UNIT);
}
*/

Result Formatter_to_string(Formatter *ptr) {
	char *buf = *Formatter_get_buf(ptr);
	u64 pos = *Formatter_get_pos(ptr);
	buf[pos] = 0;
	return String_build(buf);
}

Result to_string(void *obj) { return to_string_buf(obj, TO_STRING_BUF_SIZE); }

Result to_string_buf(void *obj, usize buf_size) {
	ResultPtr (*do_fmt)(Object *obj, Formatter *formatter) =
	    find_fn((Object *)obj, "fmt");
	if (do_fmt == NULL) {
		char *class_name = CLASS_NAME(obj);
		u64 id = ((Object *)obj)->vdata.id;
		int sz = strlen(class_name) + 20;
		if (sz > buf_size)
			sz = buf_size;
		char buf[1 + sz];
		Formatter fmt = BUILD(Formatter, buf, buf_size, 0, false);
		snprintf(buf, 1 + sz, "%s@%llu", class_name, id);

		return String_build(buf);
	}

	char buf[buf_size];
	Formatter fmt = BUILD(Formatter, buf, buf_size, 0, false);

	Result r = do_fmt(obj, &fmt);
	TRYU(r);

	return Formatter_to_string(&fmt);
}

Result to_debug(void *obj) { return to_debug_buf(obj, TO_STRING_BUF_SIZE); }

Result to_debug_buf(void *obj, usize buf_size) {
	ResultPtr (*do_fmt)(Object *obj, Formatter *formatter) =
	    find_fn((Object *)obj, "dbg");
	if (do_fmt == NULL) {

		panic("debug trait not implemented for this type [%s]",
		      CLASS_NAME(obj));
	}

	char buf[buf_size];
	Formatter fmt = BUILD(Formatter, buf, buf_size, 0, false);

	Result r = do_fmt(obj, &fmt);
	Unit u = TRY(r, u);

	return Formatter_to_string(&fmt);
}

Result U8_fmt(U8 *v, Formatter *f) {
	return WRITE(f, "%" PRIu8, GET(U8, v, value));
}

Result U16_fmt(U16 *v, Formatter *f) {
	return WRITE(f, "%" PRIu16, GET(U16, v, value));
}

Result U32_fmt(U32 *v, Formatter *f) {
	return WRITE(f, "%" PRIu32, GET(U32, v, value));
}

Result U64_fmt(U64 *v, Formatter *f) {
	return WRITE(f, "%" PRIu64, GET(U64, v, value));
}

/*
Result U128_fmt(U128 *v, Formatter *f) {
	u128 value = GET(U128, v, value);
	return WRITE(f, "%i", value);
}
*/

void uint128_to_str(char *str, u128 value) {
	char buffer[128];
	int pos = 0;

	// Handle the base case for 0
	if (value == 0) {
		str[0] = '0';
		str[1] = '\0';
		return;
	}

	while (value > 0) {
		buffer[pos++] = '0' + (value % 10);
		value /= 10;
	}
	buffer[pos] = '\0';

	// Reverse the buffer to get the correct number
	int len = pos;
	for (int i = 0; i < len / 2; i++) {
		char temp = buffer[i];
		buffer[i] = buffer[len - 1 - i];
		buffer[len - 1 - i] = temp;
	}

	// Copy to the output string
	for (int i = 0; i < len; i++) {
		str[i] = buffer[i];
	}
	str[len] = '\0';
}

Result U128_fmt(U128 *v, Formatter *f) {
	u128 value = GET(U128, v, value);
	char str[128];
	uint128_to_str(str, value);
	return WRITE(f, "%s", str);
}

Result I8_fmt(I8 *v, Formatter *f) { return WRITE(f, "%d", GET(I8, v, value)); }

Result I16_fmt(I16 *v, Formatter *f) {
	return WRITE(f, "%" PRIi16, GET(I16, v, value));
}

Result I32_fmt(I32 *v, Formatter *f) {
	return WRITE(f, "%" PRIi32, GET(I32, v, value));
}

Result I64_fmt(I64 *v, Formatter *f) {
	return WRITE(f, "%" PRIi64, GET(I64, v, value));
}

/*
Result I128_fmt(I128 *v, Formatter *f) {
	return WRITE(f, "%i", GET(I128, v, value));
}
*/

void int128_to_str(char *str, i128 value) {
	char buffer[128];
	int pos = 0;
	int is_negative = value < 0;

	// Handle the base case for 0
	if (value == 0) {
		str[0] = '0';
		str[1] = '\0';
		return;
	}

	// If the value is negative, make it positive for easier handling
	if (is_negative) {
		value = -value;
	}

	while (value > 0) {
		buffer[pos++] = '0' + (value % 10);
		value /= 10;
	}
	buffer[pos] = '\0';

	// Reverse the buffer to get the correct number
	int len = pos;
	for (int i = 0; i < len / 2; i++) {
		char temp = buffer[i];
		buffer[i] = buffer[len - 1 - i];
		buffer[len - 1 - i] = temp;
	}

	// Add the negative sign if necessary
	if (is_negative) {
		str[0] = '-';
		strcpy(str + 1, buffer);
	} else {
		strcpy(str, buffer);
	}
}

Result I128_fmt(I128 *v, Formatter *f) {
	i128 value = GET(I128, v, value);
	char str[128];
	int128_to_str(str, value);
	return WRITE(f, "%s", str);
}

Result F32_fmt(F32 *v, Formatter *f) {
	return WRITE(f, "%f", GET(F32, v, value));
}

Result F64_fmt(F64 *v, Formatter *f) {
	return WRITE(f, "%f", GET(F64, v, value));
}

Result USize_fmt(USize *v, Formatter *f) {
	return WRITE(f, "%zu", GET(USize, v, value));
}

Result ISize_fmt(ISize *v, Formatter *f) {
	return WRITE(f, "%jd", GET(ISize, v, value));
}

Result Bool_fmt(Bool *v, Formatter *f) {
	bool boolv = GET(Bool, v, value);
	if (boolv)
		return WRITE(f, "true");
	else
		return WRITE(f, "false");
}

Result Unit_fmt(Unit *v, Formatter *f) { return WRITE(f, "()"); }
