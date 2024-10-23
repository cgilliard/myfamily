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

#include <base/fam_err.h>
#include <base/misc.h>
#include <base/types.h>

i32 mymemcmp(const u8 *X, const u8 *Y, u64 len) {
	if (!X || !Y) {
		SetErr(IllegalArgument);
		return -1;
	}
	if (len == 0)
		return 0;
	while (*X && *Y && len--) {
		if (*X > *Y)
			return 1;
		else if (*X < *Y)
			return -1;

		X++;
		Y++;
		if (len == 0)
			return 0;
	}
	if (*X != '\0')
		return -1;
	else
		return 1;
}

i32 mystrcmp(const u8 *X, const u8 *Y) {
	if (!X || !Y) {
		SetErr(IllegalArgument);
		return -1;
	}
	while (*X && *Y) {
		if (*X > *Y)
			return 1;
		else if (*X < *Y)
			return -1;

		X++;
		Y++;
	}

	if (*X != '\0')
		return 1;

	else if (*Y != '\0')
		return -1;
	else
		return 0;
}

i32 mystrlen(const u8 *Y) {
	if (!Y) {
		SetErr(IllegalArgument);
		return -1;
	}
	u64 ret = 0;
	while (*Y) {
		ret++;
		Y++;
		if (ret == INT32_MAX) {
			SetErr(Overflow);
			return -1;
		}
	}
	return ret;
}

const u8 *mystrstr(const u8 *X, const u8 *Y) {
	if (!X || !Y) {
		SetErr(IllegalArgument);
		return NULL;
	}
	u64 len = mystrlen(Y);
	while (*X != '\0') {

		if ((*X == *Y) && !mymemcmp(X, Y, len)) {
			return X;
		}

		X++;
	}

	return NULL;
}

u8 *mystrcpy(u8 *X, const u8 *Y, u64 limit) {
	if (!X || !Y) {
		SetErr(IllegalArgument);
		return NULL;
	}
	u8 *ret = X;

	if (limit == 0)
		return ret;
	limit--;

	while (*Y != '\0' && limit) {
		*X = *Y;
		X++;
		Y++;
		limit--;
	}
	*X = '\0';
	return ret;
}

void *mymemcpy(void *X, const void *Y, u64 limit) {
	if (!X || !Y) {
		SetErr(IllegalArgument);
		return NULL;
	}
	u8 *ret = X;
	if (limit == 0)
		return ret;

	while (limit) {
		*(u8 *)X = *(u8 *)Y;
		X++;
		Y++;
		limit--;
	}

	return ret;
}

// A utility function to reverse a string
void reverse(u8 str[], u64 length) {
	int start = 0;
	int end = length - 1;
	while (start < end) {
		char temp = str[start];
		str[start] = str[end];
		str[end] = temp;
		end--;
		start++;
	}
}

u8 *citoau64(u64 num, u8 *str, u64 base) {
	u64 i = 0;

	if (num == 0) {
		str[i++] = '0';
		str[i] = '\0';
		return str;
	}

	while (num != 0) {
		u64 rem = num % base;
		str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
		num = num / base;
	}

	str[i] = '\0';

	reverse(str, i);

	return str;
}

u8 *citoai64(i64 num, u8 *str, u64 base) {
	u64 i = 0;
	bool isNegative = false;

	if (num == 0) {
		str[i++] = '0';
		str[i] = '\0';
		return str;
	}

	if (num < 0 && base == 10) {
		isNegative = true;
		num = -num;
	}

	while (num != 0) {
		u64 rem = num % base;
		str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
		num = num / base;
	}

	if (isNegative)
		str[i++] = '-';

	str[i] = '\0';

	reverse(str, i);

	return str;
}
